#include "sps30_sensor.h"
#include <sensors.h>
#include <Wire.h>
#include <sps30.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

// External I2C semaphore for thread safety
extern SemaphoreHandle_t i2c_semaphore;

// Global sensor data and status - extern declarations (defined in sensors.cpp)
extern SPS30Data sps30Data;
extern bool sps30SensorStatus;

// Reading control variables
static unsigned long lastReadTime = 0;
static bool measurementStarted = false;
static unsigned long measurementStartTime = 0;

// Auto-retry timer for SPS30 (2 minutes = 120000ms)
#define SPS30_RETRY_INTERVAL_MS 120000
static unsigned long lastSPS30RetryTime = 0;

bool initializeSPS30() {
    safePrintln("=== Inicjalizacja SPS30 ===");
//    WebSerial.println("=== Inicjalizacja SPS30 ===");
    
    // Initialize status
    sps30SensorStatus = false;
    
    // Take I2C semaphore for thread safety
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("SPS30: Failed to take I2C semaphore for init");
        return false;
    }
    
    uint32_t start_time = millis();
    sensirion_i2c_init(Wire);
    // Initialize SPS30 using library functions
    int16_t ret = sps30_wake_up();
     while (sps30_probe() != 0) {
    Serial.print("SPS sensor probing failed\n");
    delay(500);
  }
    
    // if (millis() - start_time > I2C_TIMEOUT_MS) {
    //     safePrintln("SPS30: Initialization timeout");
    //     if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
    //     return false;
    // }
    
    // if (ret != 0) {
    //     safePrint("SPS30: Probe failed with error ");
    //     safePrintln(String(ret));
    //     if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
    //     return false;
    // }
    
    // Reset the sensor
    ret = sps30_reset();
    if (ret != 0) {
        safePrint("SPS30: Reset failed with error ");
        safePrintln(String(ret));
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    // Wait for reset to complete
    delay(500);
    
    // // Check timeout after reset
    // if (millis() - start_time > I2C_TIMEOUT_MS * 2) {
    //     safePrintln("SPS30: Reset timeout");
    //     if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
    //     return false;
    // }
    
    // Start measurement
    ret = sps30_start_measurement();
    if (ret != 0) {
        safePrint("SPS30: Start measurement failed with error ");
        safePrintln(String(ret));
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    measurementStarted = true;
    measurementStartTime = millis();
    
    // Initialize data structure
    sps30Data.pm1_0 = 0.0;
    sps30Data.pm2_5 = 0.0;
    sps30Data.pm4_0 = 0.0;
    sps30Data.pm10 = 0.0;
    sps30Data.nc0_5 = 0.0;
    sps30Data.nc1_0 = 0.0;
    sps30Data.nc2_5 = 0.0;
    sps30Data.nc4_0 = 0.0;
    sps30Data.nc10 = 0.0;
    sps30Data.typical_particle_size = 0.0;
    sps30Data.valid = false;
    sps30Data.lastUpdate = 0;
    
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
    
    sps30SensorStatus = true;
    safePrintln("SPS30: Sensor initialized successfully");
    safePrintln("SPS30: Measurement started - data available in ~1 second");
    
    return true;
}

// Function to retry SPS30 initialization if failed
void retrySPS30IfFailed() {
    extern FeatureConfig config;
    
    if (!config.enableSPS30 || sps30SensorStatus) {
        return; // SPS30 disabled or already working
    }
    
    unsigned long currentTime = millis();
    if (currentTime - lastSPS30RetryTime >= SPS30_RETRY_INTERVAL_MS) {
        safePrintln("Retrying SPS30 initialization...");
        if (initializeSPS30()) {
            safePrintln("SPS30 retry successful!");
        } else {
            safePrintln("SPS30 retry failed");
        }
        lastSPS30RetryTime = currentTime;
    }
}

bool readSPS30(SPS30Data& data) {
    static unsigned long lastReadTime = 0;
    static unsigned long lastRetryCheck = 0;
    unsigned long currentTime = millis();
    
    // Try to retry initialization if sensor failed (check every 10 seconds)
    if (currentTime - lastRetryCheck >= 10000) {
        retrySPS30IfFailed();
        lastRetryCheck = currentTime;
    }
    
    // Read once per second as requested
    if (currentTime - lastReadTime < 1000) {
        return false; // Too soon for next reading
    }
    
    if (!sps30SensorStatus) {
        return false; // Sensor not working, don't try to read
    }
    
    // Check if measurement was started and enough time passed (1 second)
    if (!measurementStarted) {
        safePrintln("SPS30: Measurement not started");
        return false;
    }
    
    if (currentTime - measurementStartTime < 1000) {
        return false; // Need to wait at least 1 second after start
    }
    
    // Take I2C semaphore for thread safety  
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("SPS30: Failed to take I2C semaphore for reading");
        return false;
    }
    
    uint32_t start_time = millis();
    
    // Check if data is ready using simple library function
    uint16_t data_ready = 0;
    int16_t ret = sps30_read_data_ready(&data_ready);
    
    if (ret != 0) {
        safePrint("SPS30: Read data ready failed with error ");
        safePrintln(String(ret));
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    if (data_ready == 0) {
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false; // Data not ready yet
    }
    
    // Read measurements with timeout protection
    struct sps30_measurement m;
    
    ret = sps30_read_measurement(&m);
    
    // Check timeout protection
    if (millis() - start_time > I2C_TIMEOUT_MS) {
        safePrintln("SPS30: Read timeout detected");
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    if (ret != 0) {
        safePrint("SPS30: Read measurement failed with error ");
        safePrintln(String(ret));
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    // Validate readings (check for reasonable values)
    if (m.mc_1p0 < 0 || m.mc_1p0 > 1000 || m.mc_2p5 < 0 || m.mc_2p5 > 1000 ||
        m.mc_4p0 < 0 || m.mc_4p0 > 1000 || m.mc_10p0 < 0 || m.mc_10p0 > 1000) {
        safePrintln("SPS30: Invalid PM readings detected");
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    // Update data structure
    data.pm1_0 = m.mc_1p0;
    data.pm2_5 = m.mc_2p5;
    data.pm4_0 = m.mc_4p0;
    data.pm10 = m.mc_10p0;
    data.nc0_5 = m.nc_0p5;
    data.nc1_0 = m.nc_1p0;
    data.nc2_5 = m.nc_2p5;
    data.nc4_0 = m.nc_4p0;
    data.nc10 = m.nc_10p0;
    data.typical_particle_size = m.typical_particle_size;
    data.valid = true;
    data.lastUpdate = currentTime;
   // Serial.println("SPS30: Data read successfully");
    lastReadTime = currentTime;
    //set valid to true
    sps30Data.valid = true;
    sps30SensorStatus = true;
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
    
    // Debug output
    
    // safePrint("SPS30: PM1.0=");
    // safePrint(String(m.mc_1p0, 1));
    // safePrint(" PM2.5=");
    // safePrint(String(m.mc_2p5, 1));
    // safePrint(" PM4.0=");
    // safePrint(String(m.mc_4p0, 1));
    // safePrint(" PM10=");
    // safePrint(String(m.mc_10p0, 1));
    // safePrint(" µg/m³, TPS=");
    // safePrint(String(m.typical_particle_size, 1));
    // safePrintln("µm");
    
    return true;
}

void cleanSPS30() {
    if (!sps30SensorStatus) {
        safePrintln("SPS30: Sensor not initialized for cleaning");
        return;
    }
    
    // Take I2C semaphore for thread safety
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("SPS30: Failed to take I2C semaphore for cleaning");
        return;
    }
    
    safePrintln("SPS30: Starting fan cleaning cycle...");
    
    // Note: Function name may vary between library versions
    // int16_t ret = sps30_start_fan_cleaning();
    // For now, we'll skip this feature until we confirm the correct function name
    safePrintln("SPS30: Fan cleaning function not available in this library version");
    
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
}

void resetSPS30() {
    safePrintln("SPS30: Resetting sensor...");
    
    // Take I2C semaphore for thread safety
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("SPS30: Failed to take I2C semaphore for reset");
        return;
    }
    
    int16_t ret = sps30_reset();
    if (ret != 0) {
        safePrint("SPS30: Reset failed with error ");
        safePrintln(String(ret));
    } else {
        safePrintln("SPS30: Reset successful");
        measurementStarted = false;
        delay(500); // Wait for reset to complete
    }
    
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
}

void startSPS30Measurement() {
    if (!sps30SensorStatus) return;
    
    // Take I2C semaphore for thread safety
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("SPS30: Failed to take I2C semaphore for start measurement");
        return;
    }
    
    int16_t ret = sps30_start_measurement();
    if (ret != 0) {
        safePrint("SPS30: Start measurement failed with error ");
        safePrintln(String(ret));
    } else {
        measurementStarted = true;
        measurementStartTime = millis();
        safePrintln("SPS30: Measurement started");
    }
    
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
}

void stopSPS30Measurement() {
    if (!sps30SensorStatus) return;
    
    // Take I2C semaphore for thread safety
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("SPS30: Failed to take I2C semaphore for stop measurement");
        return;
    }
    
    int16_t ret = sps30_stop_measurement();
    if (ret != 0) {
        safePrint("SPS30: Stop measurement failed with error ");
        safePrintln(String(ret));
    } else {
        measurementStarted = false;
        safePrintln("SPS30: Measurement stopped");
    }
    
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
}

bool isSPS30DataReady() {
    if (!sps30SensorStatus || !measurementStarted) {
        return false;
    }
    
    // Take I2C semaphore for thread safety
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(50)) != pdTRUE) {
        return false;
    }
    
    uint16_t data_ready = 0;
    int16_t ret = sps30_read_data_ready(&data_ready);
    
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
    
    return (ret == 0) && (data_ready != 0);
}

void enableSPS30Sensor() {
    extern FeatureConfig config;
    config.enableSPS30 = true;
    
    if (initializeSPS30()) {
        sps30SensorStatus = true;
        safePrintln("SPS30: Sensor enabled successfully");
    } else {
        safePrintln("SPS30: Failed to enable sensor");
    }
}

void disableSPS30Sensor() {
    extern FeatureConfig config;
    config.enableSPS30 = false;
    
    if (sps30SensorStatus) {
        stopSPS30Measurement();
        sps30SensorStatus = false;
        sps30Data.valid = false;
        safePrintln("SPS30: Sensor disabled");
    }
} 