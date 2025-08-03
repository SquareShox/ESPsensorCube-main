#include <Wire.h>
#include <sensors.h>
#include <config.h>
#include <Arduino.h>
#include <WSEN_PADS.h>
#include <i2c_sensors.h>  // For I2C semaphore

Sensor_PADS sensor;

// SHT40 I2C commands
#define SHT40_MEASURE_HIGHREP_NOHEAT   0xFD  // High precision, no heater
#define SHT40_MEASURE_MEDREP_NOHEAT    0xF6  // Medium precision, no heater
#define SHT40_MEASURE_LOWREP_NOHEAT    0xE0  // Low precision, no heater
#define SHT40_SOFT_RESET               0x94  // Soft reset
#define SHT40_SERIAL_NUMBER            0x89  // Read serial number

// Timing constants
#define SHT40_MEASURE_DELAY_HIGHREP    10    // ms for high precision
#define SHT40_MEASURE_DELAY_MEDREP     5     // ms for medium precision
#define SHT40_MEASURE_DELAY_LOWREP     2     // ms for low precision
#define SHT40_RESET_DELAY              1     // ms for soft reset

// CRC-8 polynomial for data validation
#define SHT40_CRC8_POLYNOMIAL 0x31

// Auto-retry timer for SHT40 (2 minutes = 120000ms)
#define SHT40_RETRY_INTERVAL_MS 120000
static unsigned long lastSHT40RetryTime = 0;

// Forward declarations
static void safePrint(const String& message);
static void safePrintln(const String& message);

// Safe printing functions
static void safePrint(const String& message) {
    if (Serial) {
        Serial.print(message);
    }
}

static void safePrintln(const String& message) {
    if (Serial) {
        Serial.println(message);
    }
}

// CRC-8 calculation for SHT40 data validation
static uint8_t calculateCRC8(const uint8_t* data, size_t len) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 8; j > 0; j--) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ SHT40_CRC8_POLYNOMIAL;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

// Initialize SHT40 sensor
bool initializeSHT40() {
    extern FeatureConfig config;
    
    if (!config.enableSHT40) {
        safePrintln("SHT40 sensor disabled in config");
        return false;
    }

    // Take I2C semaphore for initialization
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("SHT40: Failed to take I2C semaphore for initialization");
        return false;
    }
    
    // Initialize I2C if not already done
    if (!Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN)) {
        safePrintln("Failed to initialize I2C for SHT40");
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    // Check if SHT40 is present
    Wire.beginTransmission(SHT40_DEFAULT_ADDR);
    uint8_t error = Wire.endTransmission();
    int retry = 0;
    bool found = false;
    for (retry = 0; retry < 5; retry++) {
        if (error == 0) {
            found = true;
            break;
        }
        safePrint("SHT40 nie znaleziony, proba ");
        safePrintln(String(retry + 1));
        delay(50);
        // Ponowne sprawdzenie obecnosci
        Wire.beginTransmission(SHT40_DEFAULT_ADDR);
        error = Wire.endTransmission();
    }
    if (!found) {
        safePrint("SHT40 nie znaleziony pod adresem 0x");
        safePrintln(String(SHT40_DEFAULT_ADDR, HEX));
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    // Perform soft reset
    Wire.beginTransmission(SHT40_DEFAULT_ADDR);
    Wire.write(SHT40_SOFT_RESET);
    Wire.endTransmission();
    delay(SHT40_RESET_DELAY);
    
    safePrint("SHT40 sensor initialized at address 0x");
    safePrintln(String(SHT40_DEFAULT_ADDR, HEX));

    // Initialize PADS pressure sensor
    if (sensor.init(PADS_ADDRESS_I2C_0) != WE_SUCCESS) {
        safePrintln("PADS pressure sensor initialization failed");
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }

    // Set the free run mode with given ODR
    if (sensor.set_continuous_mode(1) != WE_SUCCESS) {
        safePrintln("Error: PADS set_continuous_mode() failed");
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }

    // Enable low-noise configuration
    if (sensor.set_low_noise_mode() != WE_SUCCESS) {
        safePrintln("Error: PADS set_low_noise_mode() failed");
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }

    // Enable the additional low pass filter
    if (sensor.set_low_pass_configuration() != WE_SUCCESS) {
        safePrintln("Error: PADS set_low_pass_configuration() failed");
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }

    safePrintln("PADS pressure sensor initialized successfully");
    
    // Release I2C semaphore
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
    
    return true;
}

// Function to retry SHT40 initialization if failed
void retrySHT40IfFailed() {
    extern FeatureConfig config;
    extern bool sht40SensorStatus;
    
    if (!config.enableSHT40 || sht40SensorStatus) {
        return; // SHT40 disabled or already working
    }
    
    unsigned long currentTime = millis();
    if (currentTime - lastSHT40RetryTime >= SHT40_RETRY_INTERVAL_MS) {
        safePrintln("Retrying SHT40 initialization...");
        if (initializeSHT40()) {
            safePrintln("SHT40 retry successful!");
        } else {
            safePrintln("SHT40 retry failed");
        }
        lastSHT40RetryTime = currentTime;
    }
}

// Read sensor data from SHT40
bool readSHT40() {
    extern FeatureConfig config;
    extern bool sht40SensorStatus;
    
    if (!config.enableSHT40) {
        return false;
    }
    
    // Try to retry initialization if sensor failed
    static unsigned long lastRetryCheck = 0;
    if (millis() - lastRetryCheck >= 10000) { // Check every 10 seconds
        retrySHT40IfFailed();
        lastRetryCheck = millis();
    }
    
    if (!sht40SensorStatus) {
        return false; // Sensor not working, don't try to read
    }

    // Take I2C semaphore
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("SHT40: Failed to take I2C semaphore for reading");
        sht40SensorStatus = false;
        sht40Data.valid = false;
        return false;
    }
    
    // Send measurement command (high precision, no heater)
    Wire.beginTransmission(SHT40_DEFAULT_ADDR);
    Wire.write(SHT40_MEASURE_HIGHREP_NOHEAT);
    uint8_t error = Wire.endTransmission();
    
    if (error != 0) {
        safePrintln("Failed to send measurement command to SHT40");
        sht40SensorStatus = false;
        sht40Data.valid = false;
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    // Wait for measurement
    delay(SHT40_MEASURE_DELAY_HIGHREP);
    
    // Read 6 bytes (T_MSB, T_LSB, T_CRC, RH_MSB, RH_LSB, RH_CRC)
    Wire.requestFrom(SHT40_DEFAULT_ADDR, 6);
    
    if (Wire.available() != 6) {
        safePrintln("Failed to read data from SHT40");
        sht40SensorStatus = false;
        sht40Data.valid = false;
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    uint8_t data[6];
    for (int i = 0; i < 6; i++) {
        data[i] = Wire.read();
    }
    
    // Validate CRC for temperature
    uint8_t tempCRC = calculateCRC8(&data[0], 2);
    if (tempCRC != data[2]) {
        safePrintln("SHT40 temperature CRC mismatch");
        sht40SensorStatus = false;
        sht40Data.valid = false;
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    // Validate CRC for humidity
    uint8_t humCRC = calculateCRC8(&data[3], 2);
    if (humCRC != data[5]) {
        safePrintln("SHT40 humidity CRC mismatch");
        sht40SensorStatus = false;
        sht40Data.valid = false;
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    // Convert raw data to temperature and humidity
    uint16_t rawTemp = (data[0] << 8) | data[1];
    uint16_t rawHum = (data[3] << 8) | data[4];
    
    // Convert to physical values
    float temperature = -45.0 + 175.0 * (rawTemp / 65535.0);
    float humidity = -6.0 + 125.0 * (rawHum / 65535.0);
    
    // Clamp humidity to valid range
    if (humidity > 100.0) humidity = 100.0;
    if (humidity < 0.0) humidity = 0.0;

    // Read pressure from PADS sensor
    float pressure = 0.0;
    PADS_state_t stateTemperature;
    PADS_state_t statePressure;
    
    int status = sensor.ready_to_read(&stateTemperature, &statePressure);
    if (status == WE_SUCCESS && statePressure != 0) {
        if (sensor.read_pressure(&pressure) != WE_SUCCESS) {
            safePrintln("Error reading pressure from PADS sensor");
            pressure = 0.0; // Set to 0 but don't fail the whole reading
        }
    } else {
        safePrintln("PADS sensor not ready to read pressure");
        pressure = 0.0; // Set to 0 but don't fail the whole reading
    }
    pressure = pressure*10;
    // Update global data
    sht40Data.temperature = temperature;
    sht40Data.humidity = humidity;
    sht40Data.pressure = pressure;
    sht40Data.valid = true;
    sht40Data.lastUpdate = millis();
    sht40SensorStatus = true;

    // Release I2C semaphore
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);

    // Debug output every 30 seconds
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 30000) {
        lastDebug = millis();
        safePrint("SHT40 - Temperature: ");
        safePrint(String(temperature, 2));
        safePrint("°C, Humidity: ");
        safePrint(String(humidity, 2));
        safePrint("%, Pressure: ");
        safePrint(String(pressure, 2));
        safePrintln(" hPa");
    }
    
    return true;
}

// Check if SHT40 data is valid
bool isSHT40Valid() {
    return sht40Data.valid && sht40SensorStatus;
}

// Reset SHT40 sensor
void resetSHT40() {
    // Take I2C semaphore
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("SHT40: Failed to take I2C semaphore for reset");
        return;
    }

    Wire.beginTransmission(SHT40_DEFAULT_ADDR);
    Wire.write(SHT40_SOFT_RESET);
    Wire.endTransmission();
    delay(SHT40_RESET_DELAY);
    
    sht40Data.valid = false;
    sht40Data.lastUpdate = 0;
    sht40Data.pressure = 0.0;
    sht40SensorStatus = false;
    
    // Release I2C semaphore
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
    
    safePrintln("SHT40 sensor reset");
}

// Print SHT40 sensor data
void printSHT40Data() {
    if (!isSHT40Valid()) {
        safePrintln("SHT40 data not valid");
        return;
    }
    
    safePrint("SHT40 Sensor Data:");
    safePrint("  Temperature: ");
    safePrint(String(sht40Data.temperature, 2));
    safePrint("°C");
    safePrint("  Humidity: ");
    safePrint(String(sht40Data.humidity, 2));
    safePrint("%");
    safePrint("  Pressure: ");
    safePrint(String(sht40Data.pressure, 2));
    safePrint("hPa");
    safePrint("  Age: ");
    safePrint(String((millis() - sht40Data.lastUpdate) / 1000));
    safePrintln("s");
}
