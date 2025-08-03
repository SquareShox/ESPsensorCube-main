#include "i2c_sensors.h"
#include <sensors.h>
#include "sps30_sensor.h"
#include "network_config.h"
#include <Wire.h>
#include <MCP342x.h>
#include <Adafruit_INA219.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <SensirionI2cScd4x.h>

// macro definitions
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

// Forward declarations for sensor functions
bool initializeSHT40();
bool readSHT40();
bool isSHT40Valid();
void resetSHT40();
void printSHT40Data();

// I2C semaphore and task management
SemaphoreHandle_t i2c_semaphore = NULL;
TaskHandle_t mcp3424_task_handle = NULL;
TaskHandle_t scd41_task_handle = NULL;
volatile bool mcp3424_conversion_in_progress = false;
volatile unsigned long mcp3424_conversion_start_time = 0;
volatile uint8_t mcp3424_current_channel = 0; // Aktualny kanał do konwersji (0-3)
volatile unsigned long mcp3424_cycle_start_time = 0; // Czas rozpoczęcia cyklu 4 kanałów

// Debug control flag
bool mcp3424_debug_enabled = false; // Można zmienić na false żeby wyłączyć debug

// Global sensor data
extern I2CSensorData i2cSensorData;
extern FeatureConfig config;

// Global status flags - extern declarations (defined in main sensors.cpp)
extern bool i2cSensorStatus;
extern bool sht30SensorStatus;      // Osobny status SHT30
extern bool bme280SensorStatus;     // Osobny status BME280  
extern bool scd41SensorStatus;      // Osobny status SCD41
extern bool sht40SensorStatus;      // Osobny status SHT40
extern bool sps30SensorStatus;      // Status SPS30
extern bool mcp3424SensorStatus;    // Status MCP3424
extern bool ads1110SensorStatus;    // Status ADS1110
extern bool ina219SensorStatus;     // Status INA219

// Global ADC data - extern declarations (defined in main sensors.cpp)
extern MCP3424Data mcp3424Data;
extern ADS1110Data ads1110Data;
extern INA219Data ina219Data;
extern SPS30Data sps30Data;
extern SHT40Data sht40Data;

// SCD41 task control
static volatile bool scd41_task_running = false;
static volatile bool scd41_data_ready = false;
static volatile unsigned long scd41_last_read_time = 0;

// ADC sensor instances
MCP342x* mcp3424_devices[MAX_MCP3424_DEVICES] = {nullptr}; // Array of pointers to MCP342x instances
Adafruit_INA219 ina219(INA219_DEFAULT_ADDR);

// SCD41 sensor instance using Sensirion library
SensirionI2cScd4x scd41;

// Auto-retry timers for failed sensors (2 minutes = 120000ms)
#define SENSOR_RETRY_INTERVAL_MS 120000
static unsigned long lastSHT30RetryTime = 0;
static unsigned long lastBME280RetryTime = 0;
static unsigned long lastSCD41RetryTime = 0;
static unsigned long lastSHT40RetryTime = 0;
static unsigned long lastSPS30RetryTime = 0;
static unsigned long lastMCP3424RetryTime = 0;
static unsigned long lastADS1110RetryTime = 0;
static unsigned long lastINA219RetryTime = 0;

void initializeI2C() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    // Set pull-up resistors
    //pinMode(I2C_SDA_PIN, INPUT_PULLUP);
   // pinMode(I2C_SCL_PIN, INPUT_PULLUP);
    
    // Create I2C semaphore for thread safety
    if (i2c_semaphore == NULL) {
        i2c_semaphore = xSemaphoreCreateMutex();
        if (i2c_semaphore != NULL) {
            safePrintln("I2C semaphore created successfully");
        } else {
            safePrintln("Failed to create I2C semaphore");
        }
    }
    
    safePrintln("I2C initialized with pull-up resistors");
    
    // Try to detect and initialize I2C sensors based on configuration
    if (config.enableSHT30 && initializeI2CSensor(SENSOR_SHT30)) {
        sht30SensorStatus = true;
        i2cSensorData.type = SENSOR_SHT30;
        safePrintln("SHT30 sensor detected and enabled");
    }
    
    if (config.enableBME280 && initializeI2CSensor(SENSOR_BME280)) {
        bme280SensorStatus = true;
        if (!sht30SensorStatus) { // Use BME280 as primary if SHT30 not available
            i2cSensorData.type = SENSOR_BME280;
        }
        safePrintln("BME280 sensor detected and enabled");
    }
    
    if (config.enableSCD41 && initializeI2CSensor(SENSOR_SCD41)) {
        safePrintln("SCD41: Sensor detected at address 0x62, starting initialization...");
        if (initializeSCD41()) {
            scd41SensorStatus = true;
            if (!sht30SensorStatus && !bme280SensorStatus) { // Use SCD41 as primary if others not available
                i2cSensorData.type = SENSOR_SCD41;
            }
            safePrintln("SCD41 sensor detected and enabled");
        } else {
            safePrintln("SCD41 initialization failed");
        }
    } else if (config.enableSCD41) {
        safePrintln("SCD41: Sensor not detected at address 0x62");
    }
    
    if (config.enableSHT40 && initializeI2CSensor(SENSOR_SHT40)) {
        if (initializeSHT40()) {
            sht40SensorStatus = true;
            safePrintln("SHT40 sensor detected and enabled");
        } else {
            safePrintln("SHT40 initialization failed");
        }
    }
    
    // Initialize SPS30 particle sensor
    if (config.enableSPS30 && initializeI2CSensor(SENSOR_SPS30)) {
        if (initializeSPS30()) {
            sps30SensorStatus = true;
            safePrintln("SPS30 particle sensor initialized and enabled");
        } else {
            safePrintln("SPS30 initialization failed");
        }
    }
    
    // Initialize ADC sensors
    // Initialize MCP3424 using new address mapping system
    if (config.enableMCP3424) {
        // Load MCP3424 configuration, if not found, use defaults
        extern MCP3424Config mcp3424Config;
        if (!loadMCP3424Config(mcp3424Config) || !mcp3424Config.configValid) {
            safePrintln("Loading default MCP3424 mapping...");
            initializeDefaultMCP3424Mapping();
        }
        
        // Scan and map devices using the new system with retry mechanism
        safePrintln("=== Starting MCP3424 scan with retry mechanism ===");
        bool scanSuccess = false;
        for (int attempt = 1; attempt <= 5; attempt++) {
            safePrintln("MCP3424 scan attempt " + String(attempt) + "/5");
            scanAndMapMCP3424Devices();
            
            // Count detected devices from ALL 8 devices (not just deviceCount)
            uint8_t detectedCount = 0;
            for (uint8_t i = 0; i < 8; i++) {
                if (mcp3424Config.devices[i].autoDetected && mcp3424Config.devices[i].enabled) {
                    safePrint("Count: Device ");
                    safePrint(String(i));
                    safePrint(" detected & enabled -> ");
                    safePrintln(mcp3424Config.devices[i].gasType);
                    detectedCount++;
                }
            }
            
            safePrintln("Detected " + String(detectedCount) + " MCP3424 devices");
            if (detectedCount > 0) {
                scanSuccess = true;
                safePrintln("MCP3424 scan successful on attempt " + String(attempt));
                break;
            }
            
            if (attempt < 5) {
                safePrintln("No devices found, waiting 500ms before retry...");
                delay(500);
            }
        }
        
        if (!scanSuccess) {
            safePrintln("WARNING: No MCP3424 devices found after 5 attempts");
        }
        
        // Initialize MCP3424 devices based on detected addresses
        mcp3424Data.deviceCount = 0;
        
        for (uint8_t i = 0; i < 8; i++) {
            if (mcp3424Config.devices[i].autoDetected && mcp3424Config.devices[i].enabled) {
                uint8_t addr = mcp3424Config.devices[i].i2cAddress;
                uint8_t deviceIndex = mcp3424Config.devices[i].deviceIndex;
                
                safePrint("Init: Device ");
                safePrint(String(i));
                safePrint(" detected & enabled -> ");
                safePrint(mcp3424Config.devices[i].gasType);
                safePrint(" @ 0x");
                safePrintln(String(addr, HEX));
                
                if (mcp3424Data.deviceCount < MAX_MCP3424_DEVICES) {
                    safePrint("Initializing MCP3424 device ");
                    safePrint(String(deviceIndex));
                    safePrint(" at address 0x");
                    safePrint(String(addr, HEX));
                    safePrint(" for ");
                    safePrintln(mcp3424Config.devices[i].gasType);
                    
                    // Create new instance for this address
                    mcp3424_devices[mcp3424Data.deviceCount] = new MCP342x(addr);
                    mcp3424Data.addresses[mcp3424Data.deviceCount] = addr;
                    mcp3424Data.valid[mcp3424Data.deviceCount] = false;
                    
                    // Initialize all channels to 0
                    for (int ch = 0; ch < 4; ch++) {
                        mcp3424Data.channels[mcp3424Data.deviceCount][ch] = 0.0;
                    }
                    
                    mcp3424Data.deviceCount++;
                }
            }
        }
        
        safePrintln("Final mcp3424Data.deviceCount: " + String(mcp3424Data.deviceCount));
        
        // Save MCP3424 config after successful scan to preserve autoDetected status
        if (saveMCP3424Config(mcp3424Config)) {
            safePrintln("MCP3424 config saved with detection status");
        } else {
            safePrintln("WARNING: Failed to save MCP3424 config");
        }
        
        if (mcp3424Data.deviceCount > 0) {
            mcp3424SensorStatus = true;
            mcp3424Data.resolution = 18;  // Use 18-bit for maximum precision (~266ms per channel)
            mcp3424Data.gain = 1;         // Default 1x gain
            mcp3424_current_channel = 0;  // Start from channel 1 (index 0)
            mcp3424_cycle_start_time = 0; // Initialize cycle timer
            
            // Create MCP3424 async conversion task
            if (mcp3424_task_handle == NULL) {
                xTaskCreate(
                    mcp3424ConversionTask,   // Task function
                    "MCP3424_Task",          // Task name
                    4096,                    // Stack size
                    NULL,                    // Task parameters
                    1,                       // Priority
                    &mcp3424_task_handle     // Task handle
                );
                safePrintln("MCP3424 async task created");
            }
            
            safePrint("Initialized ");
            safePrint(String(mcp3424Data.deviceCount));
            safePrint(" MCP3424 devices with 18-bit resolution using address mapping");
            if (mcp3424_debug_enabled) {
                safePrint(" - DEBUG ON");
            } else {
                safePrint(" - DEBUG OFF");
            }
            safePrintln("");
        } else {
            safePrintln("No MCP3424 devices found or enabled");
        }
    }
    
    if (config.enableADS1110 && initializeI2CSensor(SENSOR_ADS1110)) {
        ads1110SensorStatus = true;
        ads1110Data.dataRate = 15;    // Default 15 SPS (00 = 15 SPS)
        ads1110Data.gain = 1;         // Default 1x gain (00 = 1x)
        safePrintln("ADS1110 sensor detected and enabled");
    }
    
    if (config.enableINA219 && initializeI2CSensor(SENSOR_INA219)) {
        ina219SensorStatus = true;
        ina219.begin();
        ina219.setCalibration_32V_2A();  // Default calibration
        safePrintln("INA219 sensor detected and enabled");
    }
    
    // Update overall I2C status
    i2cSensorStatus = sht30SensorStatus || bme280SensorStatus || scd41SensorStatus ||
                     sht40SensorStatus || sps30SensorStatus || mcp3424SensorStatus || ads1110SensorStatus || ina219SensorStatus;
    
    if (!i2cSensorStatus) {
        safePrintln("No I2C sensors detected or enabled");
    }
}

bool initializeI2CSensor(I2CSensorType sensorType) {
    uint8_t address = 0;
    switch (sensorType) {
        case SENSOR_SHT30: address = 0x44; break;
        case SENSOR_BME280: address = 0x76; break;
        case SENSOR_SCD41: address = 0x62; break;
        case SENSOR_SHT40: address = SHT40_DEFAULT_ADDR; break;
        case SENSOR_SPS30: address = SPS30_I2C_ADDRESS; break;
        case SENSOR_MCP3424: address = MCP3424_DEFAULT_ADDR; break;
        case SENSOR_ADS1110: address = ADS1110_DEFAULT_ADDR; break;
        case SENSOR_INA219: address = INA219_DEFAULT_ADDR; break;
        default: return false;
    }
    
    // Try communication with timeout
    unsigned long startTime = millis();
    Wire.beginTransmission(address);
    uint8_t result = Wire.endTransmission();
    
    // Check if communication took too long (timeout protection)
    if (millis() - startTime > I2C_TIMEOUT_MS) {
        safePrintln("I2C timeout detected, skipping sensor");
        return false;
    }
    
    return (result == 0);
}

// Function to retry initialization of failed I2C sensors
void retryFailedI2CSensors() {
    unsigned long currentTime = millis();
    
    // Check and retry SHT30 if failed and 2 minutes passed
    if (config.enableSHT30 && !sht30SensorStatus && 
        (currentTime - lastSHT30RetryTime >= SENSOR_RETRY_INTERVAL_MS)) {
        safePrintln("Retrying SHT30 initialization...");
        if (initializeI2CSensor(SENSOR_SHT30)) {
            sht30SensorStatus = true;
            i2cSensorData.type = SENSOR_SHT30;
            safePrintln("SHT30 retry successful!");
        } else {
            safePrintln("SHT30 retry failed");
        }
        lastSHT30RetryTime = currentTime;
    }
    
    // Check and retry BME280 if failed and 2 minutes passed
    if (config.enableBME280 && !bme280SensorStatus && 
        (currentTime - lastBME280RetryTime >= SENSOR_RETRY_INTERVAL_MS)) {
        safePrintln("Retrying BME280 initialization...");
        if (initializeI2CSensor(SENSOR_BME280)) {
            bme280SensorStatus = true;
            if (!sht30SensorStatus) {
                i2cSensorData.type = SENSOR_BME280;
            }
            safePrintln("BME280 retry successful!");
        } else {
            safePrintln("BME280 retry failed");
        }
        lastBME280RetryTime = currentTime;
    }
    
    // Check and retry SCD41 if failed and 2 minutes passed
    if (config.enableSCD41 && !scd41SensorStatus && 
        (currentTime - lastSCD41RetryTime >= SENSOR_RETRY_INTERVAL_MS)) {
        safePrintln("Retrying SCD41 initialization...");
        if (initializeI2CSensor(SENSOR_SCD41) && initializeSCD41()) {
            scd41SensorStatus = true;
            if (!sht30SensorStatus && !bme280SensorStatus) {
                i2cSensorData.type = SENSOR_SCD41;
            }
            safePrintln("SCD41 retry successful!");
        } else {
            safePrintln("SCD41 retry failed");
        }
        lastSCD41RetryTime = currentTime;
    }
    
    // Check and retry SHT40 if failed and 2 minutes passed
    if (config.enableSHT40 && !sht40SensorStatus && 
        (currentTime - lastSHT40RetryTime >= SENSOR_RETRY_INTERVAL_MS)) {
        safePrintln("Retrying SHT40 initialization...");
        if (initializeI2CSensor(SENSOR_SHT40) && initializeSHT40()) {
            sht40SensorStatus = true;
            safePrintln("SHT40 retry successful!");
        } else {
            safePrintln("SHT40 retry failed");
        }
        lastSHT40RetryTime = currentTime;
    }
    
    // Check and retry SPS30 if failed and 2 minutes passed
    if (config.enableSPS30 && !sps30SensorStatus && 
        (currentTime - lastSPS30RetryTime >= SENSOR_RETRY_INTERVAL_MS)) {
        safePrintln("Retrying SPS30 initialization...");
        if (initializeI2CSensor(SENSOR_SPS30) && initializeSPS30()) {
            sps30SensorStatus = true;
            safePrintln("SPS30 retry successful!");
        } else {
            safePrintln("SPS30 retry failed");
        }
        lastSPS30RetryTime = currentTime;
    }
    
    // Check and retry MCP3424 if failed and 2 minutes passed
    if (config.enableMCP3424 && !mcp3424SensorStatus && 
        (currentTime - lastMCP3424RetryTime >= SENSOR_RETRY_INTERVAL_MS)) {
        safePrintln("Retrying MCP3424 initialization...");
        
        // Reload MCP3424 config and scan for devices
        extern MCP3424Config mcp3424Config;
        if (!loadMCP3424Config(mcp3424Config) || !mcp3424Config.configValid) {
            safePrintln("Loading default MCP3424 mapping for retry...");
            initializeDefaultMCP3424Mapping();
        }
        
        scanAndMapMCP3424Devices();
        
        // Reinitialize devices based on scan results
        mcp3424Data.deviceCount = 0;
        for (uint8_t i = 0; i < 8; i++) {
            if (mcp3424Config.devices[i].autoDetected && mcp3424Config.devices[i].enabled) {
                uint8_t addr = mcp3424Config.devices[i].i2cAddress;
                if (mcp3424Data.deviceCount < MAX_MCP3424_DEVICES) {
                    mcp3424_devices[mcp3424Data.deviceCount] = new MCP342x(addr);
                    mcp3424Data.addresses[mcp3424Data.deviceCount] = addr;
                    mcp3424Data.valid[mcp3424Data.deviceCount] = false;
                    mcp3424Data.deviceCount++;
                }
            }
        }
        
        safePrintln("Retry final mcp3424Data.deviceCount: " + String(mcp3424Data.deviceCount));
        
        // Save MCP3424 config after retry scan to preserve autoDetected status
        if (saveMCP3424Config(mcp3424Config)) {
            safePrintln("MCP3424 config saved after retry");
        } else {
            safePrintln("WARNING: Failed to save MCP3424 config after retry");
        }
        
        if (mcp3424Data.deviceCount > 0) {
            mcp3424SensorStatus = true;
            safePrintln("MCP3424 retry successful!");
        } else {
            safePrintln("MCP3424 retry failed - no devices found");
        }
        lastMCP3424RetryTime = currentTime;
    }
    
    // Check and retry ADS1110 if failed and 2 minutes passed
    if (config.enableADS1110 && !ads1110SensorStatus && 
        (currentTime - lastADS1110RetryTime >= SENSOR_RETRY_INTERVAL_MS)) {
        safePrintln("Retrying ADS1110 initialization...");
        if (initializeI2CSensor(SENSOR_ADS1110)) {
            ads1110SensorStatus = true;
            ads1110Data.dataRate = 15;
            ads1110Data.gain = 1;
            safePrintln("ADS1110 retry successful!");
        } else {
            safePrintln("ADS1110 retry failed");
        }
        lastADS1110RetryTime = currentTime;
    }
    
    // Check and retry INA219 if failed and 2 minutes passed
    if (config.enableINA219 && !ina219SensorStatus && 
        (currentTime - lastINA219RetryTime >= SENSOR_RETRY_INTERVAL_MS)) {
        safePrintln("Retrying INA219 initialization...");
        if (initializeI2CSensor(SENSOR_INA219)) {
            ina219SensorStatus = true;
            ina219.begin();
            ina219.setCalibration_32V_2A();
            safePrintln("INA219 retry successful!");
        } else {
            safePrintln("INA219 retry failed");
        }
        lastINA219RetryTime = currentTime;
    }
    
    // Update overall I2C status
    i2cSensorStatus = sht30SensorStatus || bme280SensorStatus || scd41SensorStatus ||
                     sht40SensorStatus || sps30SensorStatus || mcp3424SensorStatus || 
                     ads1110SensorStatus || ina219SensorStatus;
}

void readI2CSensors() {
    if (!config.enableI2CSensors) return;
    
    static unsigned long lastReadTime = 0;
    static unsigned long lastRetryTime = 0;
    unsigned long currentTime = millis();
    
    // Try to retry failed sensors every 10 seconds (check if 2 minutes passed since last retry)
    if (currentTime - lastRetryTime >= 10000) { // Check every 10 seconds
        retryFailedI2CSensors();
        lastRetryTime = currentTime;
    }
    
    // Early exit if no sensors are working at all
    if (!i2cSensorStatus) return;

    if (config.enableMCP3424) {
       // Always call readMCP3424 to trigger conversions
       readMCP3424(mcp3424Data);
       
       // Check if MCP3424 is actually working by checking:
       // 1. Device count > 0 (devices detected)
       // 2. At least one device has valid data
       // 3. Data is reasonably fresh (less than 10 seconds old)
       bool hasValidData = false;
       if (mcp3424Data.deviceCount > 0) {
           for (uint8_t device = 0; device < mcp3424Data.deviceCount; device++) {
               if (mcp3424Data.valid[device]) {
                   hasValidData = true;
                   break;
               }
           }
       }
       
       // Check if data is fresh (less than 10 seconds old)
       bool dataIsFresh = (millis() - mcp3424Data.lastUpdate) < 10000;
       
       // Set status based on valid data and freshness
       mcp3424SensorStatus = hasValidData && dataIsFresh;
       
       // Debug output every 30 seconds
       static unsigned long lastMCP3424ReadDebug = 0;
       if (millis() - lastMCP3424ReadDebug > 30000) {
           lastMCP3424ReadDebug = millis();
           safePrint("MCP3424 Status Check - DeviceCount: ");
           safePrint(String(mcp3424Data.deviceCount));
           safePrint(", HasValidData: ");
           safePrint(hasValidData ? "true" : "false");
           safePrint(", DataAge: ");
           safePrint(String((millis() - mcp3424Data.lastUpdate) / 1000));
           safePrint("s, Status: ");
           safePrintln(mcp3424SensorStatus ? "OK" : "ERROR");
       }
    }
    
    // Read SPS30 particle sensor every second
    if (config.enableSPS30) {
        // Always call readSPS30 to trigger measurements
        readSPS30(sps30Data);
        
        // Check if SPS30 is actually working by checking:
        // 1. Data is valid (successful read)
        // 2. Data is reasonably fresh (less than 5 seconds old)
        bool dataIsFresh = (millis() - sps30Data.lastUpdate) < 5000;
        
        // Set status based on valid data and freshness
        sps30SensorStatus = sps30Data.valid && dataIsFresh;
        
        // Debug output every 60 seconds
        static unsigned long lastSPS30Debug = 0;
        if (millis() - lastSPS30Debug > 60000) {
            lastSPS30Debug = millis();
            safePrint("SPS30 Status Check - Valid: ");
            safePrint(sps30Data.valid ? "true" : "false");
            safePrint(", DataAge: ");
            safePrint(String((millis() - sps30Data.lastUpdate) / 1000));
            safePrint("s, Status: ");
            safePrintln(sps30SensorStatus ? "OK" : "ERROR");
        }
    }
    
    if (currentTime - lastReadTime >= 1000) { // Read every 1 second
        lastReadTime = currentTime;
        
        // Read environmental sensors
        switch (i2cSensorData.type) {
            case SENSOR_SHT30:
                sht30SensorStatus = readSHT30(i2cSensorData);
                break;
            case SENSOR_BME280:
                bme280SensorStatus = readBME280(i2cSensorData);
                break;  
            case SENSOR_SCD41:
                scd41SensorStatus = readSCD41(i2cSensorData);
                break;
            default:
                break;
        }
        
        // Read SHT40 for temperature, humidity and pressure (main environmental data)
        if (config.enableSHT40 && sht40SensorStatus) {
            sht40SensorStatus = readSHT40(sht40Data);
            if (sht40Data.valid) {
                // Use SHT40 data as main environmental data
                i2cSensorData.temperature = sht40Data.temperature;
                i2cSensorData.humidity = sht40Data.humidity;
                i2cSensorData.pressure = sht40Data.pressure;
                i2cSensorData.valid = true;
                i2cSensorData.lastUpdate = currentTime;
                i2cSensorData.type = SENSOR_SHT40;
                
                // safePrint("SHT40 - Temp: ");
                // safePrint(String(sht40Data.temperature, 2));
                // safePrint("°C, Humidity: ");
                // safePrint(String(sht40Data.humidity, 2));
                // safePrint("%, Pressure: ");
                // safePrint(String(sht40Data.pressure, 2));
                // safePrintln(" hPa");
            }
        }
        
        // SCD41 data is updated by background task, just check if it's valid
        if (config.enableSCD41 && scd41SensorStatus) {
            // SCD41 task updates i2cSensorData automatically
            // Just ensure temperature and humidity come from SHT40 if available
            if (config.enableSHT40 && sht40Data.valid && i2cSensorData.valid) {
                i2cSensorData.temperature = sht40Data.temperature;
                i2cSensorData.humidity = sht40Data.humidity;
                i2cSensorData.pressure = sht40Data.pressure;
            }
        } else if (config.enableSCD41 && !scd41SensorStatus) {
            safePrintln("SCD41: Sensor enabled but not initialized");
        }
        
        // Read ADC sensors
       
        
       
    }
}

void readBatteryVoltage() {
    if (!config.enableI2CSensors || !i2cSensorStatus) return;
    
    static unsigned long lastReadTimeBat = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastReadTimeBat >= 10000) { // Read every 10 seconds
        lastReadTimeBat = currentTime;
        
       // safePrintln("=== Battery Voltage Reading ===");
        
        if (ads1110SensorStatus) {
            ads1110SensorStatus = readADS1110(ads1110Data);
            if (ads1110Data.valid) {
                // Assume battery divider ratio if needed (e.g., 2:1 divider)
                float batteryVoltage = ads1110Data.voltage * 2.0; // Adjust ratio as needed
                
                safePrint("Battery Voltage (ADS1110): ");
                safePrint(String(batteryVoltage, 3));
                safePrint("V (Raw ADC: ");
                safePrint(String(ads1110Data.voltage, 6));
                safePrintln("V)");
            } else {
                safePrintln("ADS1110 battery reading invalid");
            }
        }
        
        if (ina219SensorStatus) {
            ina219SensorStatus = readINA219(ina219Data);
            if (ina219Data.valid) {
                safePrint("Power Monitor (INA219): Bus=");
                safePrint(String(ina219Data.busVoltage, 3));
                safePrint("V, Current=");
                safePrint(String(ina219Data.current, 2));
                safePrint("mA, Power=");
                safePrint(String(ina219Data.power, 2));
                safePrintln("mW");
            } else {
                safePrintln("INA219 power reading invalid");
            }
        }
        
        
        
       // safePrintln("=== End Battery Reading ===");
    }
}

bool readSHT30(I2CSensorData& data) {
    // Take I2C semaphore for thread safety
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("SHT30: Failed to take I2C semaphore");
        data.valid = false;
        return false;
    }

    uint32_t start_time = millis();
    
    Wire.beginTransmission(0x44);
    Wire.write(0x2C);
    Wire.write(0x06);
    if (Wire.endTransmission() != 0) {
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    delay(500);
    
    // Timeout protection
    Wire.requestFrom(0x44, 6);
    while (Wire.available() < 6 && (millis() - start_time) < I2C_TIMEOUT_MS) {
        delay(1);
    }
    
    if (Wire.available() == 6) {
        uint8_t rawData[6];
        for (int i = 0; i < 6; i++) {
            rawData[i] = Wire.read();
        }
        
        uint16_t tempRaw = (rawData[0] << 8) | rawData[1];
        uint16_t humRaw = (rawData[3] << 8) | rawData[4];
        
        data.temperature = -45 + 175 * ((float)tempRaw / 65535.0);
        data.humidity = 100 * ((float)humRaw / 65535.0);
        data.valid = true;
        
        // Release I2C semaphore
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return true;
    }
    
    // Release I2C semaphore on failure
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
    return false;
}

bool readBME280(I2CSensorData& data) {
    // Simplified BME280 reading - would need proper library implementation
    data.temperature = 25.0; // Placeholder
    data.humidity = 50.0;
    data.pressure = 1013.25;
    data.valid = true;
    return true;
}

bool initializeSCD41() {
    extern FeatureConfig config;
    
    if (!config.enableSCD41) {
        safePrintln("SCD41 sensor disabled in config");
        return false;
    }

    // Take I2C semaphore for initialization
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("SCD41: Failed to take I2C semaphore for initialization");
        return false;
    }
    
    uint32_t start_time = millis();
    
    // Initialize SCD41 using Sensirion library
    uint16_t error;
    char errorMessage[256];
    
    // Initialize sensor with Wire and address 0x62
    scd41.begin(Wire, 0x62);
    
    // Wait for sensor to be ready
    delay(30);
    
    // Ensure sensor is in clean state - follow Sensirion example
    error = scd41.wakeUp();
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, 256);
        safePrint("SCD41: Error trying to execute wakeUp(): ");
        safePrintln(errorMessage);
    }
    
    error = scd41.stopPeriodicMeasurement();
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, 256);
        safePrint("SCD41: Error trying to execute stopPeriodicMeasurement(): ");
        safePrintln(errorMessage);
    }
    
    error = scd41.reinit();
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, 256);
        safePrint("SCD41: Error trying to execute reinit(): ");
        safePrintln(errorMessage);
    }
    
    // Read out information about the sensor (optional)
    uint64_t serialNumber = 0;
    error = scd41.getSerialNumber(serialNumber);
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, 256);
        safePrint("SCD41: Error trying to execute getSerialNumber(): ");
        safePrintln(errorMessage);
    } else {
        safePrint("SCD41: Serial number: 0x");
        safePrint(String((uint32_t)(serialNumber >> 32), HEX));
        safePrintln(String((uint32_t)(serialNumber & 0xFFFFFFFF), HEX));
    }
    
    // Start periodic measurements (5sec interval)
    error = scd41.startPeriodicMeasurement();
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, 256);
        safePrint("SCD41: Error trying to execute startPeriodicMeasurement(): ");
        safePrintln(errorMessage);
        if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
        return false;
    }
    

    
    safePrintln("SCD41 sensor initialized successfully with periodic measurement");
    
    // Release I2C semaphore
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
    
    // Start SCD41 task
    scd41_task_running = true;
    scd41_data_ready = false;
    scd41_last_read_time = 0;
    
    BaseType_t taskCreated = xTaskCreatePinnedToCore(
        scd41Task,
        "SCD41_Task",
        4096,  // Stack size
        NULL,  // Parameters
        2,     // Priority (higher than main loop)
        &scd41_task_handle,
        1      // Core 1 (same as main loop)
    );
    
    if (taskCreated == pdPASS) {
        safePrintln("SCD41 task created successfully");
    } else {
        safePrintln("Failed to create SCD41 task");
        return false;
    }
    
    return true;
}

bool readSCD41(I2CSensorData& data) {
    // Check if task is running and data is ready
    if (!scd41_task_running || !scd41_data_ready) {
        data.valid = false;
        return false;
    }
    
    // Check if data is fresh (less than 10 seconds old)
    if (millis() - scd41_last_read_time > 10000) {
        data.valid = false;
        return false;
    }
    
    // Copy data from global structure (updated by task)
    extern I2CSensorData i2cSensorData;
    extern SHT40Data sht40Data;
    extern FeatureConfig config;
    
    if (i2cSensorData.valid && i2cSensorData.type == SENSOR_SCD41) {
        // Use temperature and humidity from SHT40 if available, otherwise from SCD41
        if (config.enableSHT40 && sht40Data.valid) {
            data.temperature = sht40Data.temperature;
            data.humidity = sht40Data.humidity;
            data.pressure = sht40Data.pressure;
        } else {
            data.temperature = i2cSensorData.temperature;
            data.humidity = i2cSensorData.humidity;
            data.pressure = i2cSensorData.pressure;
        }
        data.co2 = i2cSensorData.co2; // CO2 always from SCD41
        data.valid = true;
        data.lastUpdate = i2cSensorData.lastUpdate;
        data.type = SENSOR_SCD41;
        return true;
    }
    
    data.valid = false;
    return false;
}

bool readSHT40(SHT40Data& data) {
    // Call the actual SHT40 implementation from sht40.cpp
    return readSHT40();
}

void enableI2CSensor(I2CSensorType sensorType) {
    switch (sensorType) {
        case SENSOR_SHT30:
            config.enableSHT30 = true;
            if (initializeI2CSensor(SENSOR_SHT30)) {
                sht30SensorStatus = true;
                        safePrintln("SHT30 sensor enabled");
            }
            break;
        case SENSOR_BME280:
            config.enableBME280 = true;
            if (initializeI2CSensor(SENSOR_BME280)) {
                bme280SensorStatus = true;
                        safePrintln("BME280 sensor enabled");

            }
            break;
        case SENSOR_SCD41:
            config.enableSCD41 = true;
            if (initializeI2CSensor(SENSOR_SCD41)) {
                if (initializeSCD41()) {
                    scd41SensorStatus = true;
                            safePrintln("SCD41 sensor enabled");
                } else {
                                safePrintln("SCD41 sensor initialization failed");
                }
            }
            break;
        case SENSOR_SHT40:
            config.enableSHT40 = true;
            if (initializeI2CSensor(SENSOR_SHT40)) {
                if (initializeSHT40()) {
                    sht40SensorStatus = true;
                            safePrintln("SHT40 sensor enabled");
                } else {
                                safePrintln("SHT40 sensor initialization failed");
                }
            }
            break;
        case SENSOR_SPS30:
            config.enableSPS30 = true;
            if (initializeI2CSensor(SENSOR_SPS30)) {
                if (initializeSPS30()) {
                    sps30SensorStatus = true;
                            safePrintln("SPS30 sensor enabled");
                } else {
                                safePrintln("SPS30 sensor initialization failed");
                }
            }
            break;
        case SENSOR_MCP3424:
            config.enableMCP3424 = true;
            if (initializeI2CSensor(SENSOR_MCP3424)) {
                mcp3424SensorStatus = true;
                        safePrintln("MCP3424 sensor enabled");
            }
            break;
        case SENSOR_ADS1110:
            config.enableADS1110 = true;
            if (initializeI2CSensor(SENSOR_ADS1110)) {
                ads1110SensorStatus = true;
                        safePrintln("ADS1110 sensor enabled");
            }
            break;
        case SENSOR_INA219:
            config.enableINA219 = true;
            if (initializeI2CSensor(SENSOR_INA219)) {
                ina219SensorStatus = true;
                ina219.begin();
                ina219.setCalibration_32V_2A();
                        safePrintln("INA219 sensor enabled");
            }
            break;
        default:
            break;
    }
    
    // Update overall I2C status
    i2cSensorStatus = sht30SensorStatus || bme280SensorStatus || scd41SensorStatus ||
                     sht40SensorStatus || sps30SensorStatus || mcp3424SensorStatus || ads1110SensorStatus || ina219SensorStatus;
}

void disableI2CSensor(I2CSensorType sensorType) {
    switch (sensorType) {
        case SENSOR_SHT30:
            config.enableSHT30 = false;
            sht30SensorStatus = false;
                    safePrintln("SHT30 sensor disabled");
            break;
        case SENSOR_BME280:
            config.enableBME280 = false;
            bme280SensorStatus = false;
                    safePrintln("BME280 sensor disabled");
            break;
        case SENSOR_SCD41:
            config.enableSCD41 = false;
            scd41SensorStatus = false;
            
            // Stop SCD41 task
            if (scd41_task_running) {
                scd41_task_running = false;
                if (scd41_task_handle != NULL) {
                    vTaskDelay(pdMS_TO_TICKS(100)); // Give task time to stop
                    scd41_task_handle = NULL;
                }
                safePrintln("SCD41 task stopped");
            }
            
                    safePrintln("SCD41 sensor disabled");
            break;
        case SENSOR_SHT40:
            config.enableSHT40 = false;
            sht40SensorStatus = false;
            sht40Data.valid = false;
                    safePrintln("SHT40 sensor disabled");
            break;
        case SENSOR_SPS30:
            config.enableSPS30 = false;
            if (sps30SensorStatus) {
                stopSPS30Measurement();
                sps30SensorStatus = false;
                sps30Data.valid = false;
                        safePrintln("SPS30 sensor disabled");
            }
            break;
        case SENSOR_MCP3424:
            config.enableMCP3424 = false;
            mcp3424SensorStatus = false;
                    safePrintln("MCP3424 sensor disabled");
            break;
        case SENSOR_ADS1110:
            config.enableADS1110 = false;
            ads1110SensorStatus = false;
                    safePrintln("ADS1110 sensor disabled");
            break;
        case SENSOR_INA219:
            config.enableINA219 = false;
            ina219SensorStatus = false;
                    safePrintln("INA219 sensor disabled");
            break;
        default:
            break;
    }
    
    // Update overall I2C status
    i2cSensorStatus = sht30SensorStatus || bme280SensorStatus || scd41SensorStatus ||
                     sht40SensorStatus || sps30SensorStatus || mcp3424SensorStatus || ads1110SensorStatus || ina219SensorStatus;
}

// Start conversion on current channel for all MCP3424 devices
bool startMCP3424Conversion(MCP3424Data& data) {
    if (xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(50)) != pdTRUE) {
        if (mcp3424_debug_enabled) safePrintln("MCP3424: Failed to take I2C semaphore for conversion start");
        return false;
    }
    
    bool success = true;
    MCP342x::Channel channels[] = {MCP342x::channel1, MCP342x::channel2, MCP342x::channel3, MCP342x::channel4};
    
    // Jeśli zaczynamy kanał 1, zapisz czas rozpoczęcia cyklu
    if (mcp3424_current_channel == 0) {
        mcp3424_cycle_start_time = millis();
    }
    
    // Start conversion on current channel for all devices
    for (uint8_t device = 0; device < data.deviceCount; device++) {
        if (mcp3424_devices[device] == nullptr) continue;
        
        MCP342x::Config config(channels[mcp3424_current_channel], MCP342x::oneShot, MCP342x::resolution18, MCP342x::gain1);
        
        uint8_t err = mcp3424_devices[device]->convert(config);
        
        if (err != 0) {
            if (mcp3424_debug_enabled) {
                safePrint("MCP3424 device ");
                safePrint(String(device));
                safePrint(" ch ");
                safePrint(String(mcp3424_current_channel + 1));
                safePrint(" convert failed with error ");
                safePrintln(String(err));
            }
            success = false;
        }
    }
    
    if (success) {
        mcp3424_conversion_in_progress = true;
        mcp3424_conversion_start_time = millis();
    }
    
    xSemaphoreGive(i2c_semaphore);
    return success;
}

// Read results from current channel for all MCP3424 devices
bool readMCP3424Results(MCP3424Data& data) {
    if (xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(50)) != pdTRUE) {
        if (mcp3424_debug_enabled) safePrintln("MCP3424: Failed to take I2C semaphore for results read");
        return false;
    }
    
    bool overallSuccess = false;
    uint32_t read_start = millis();
    
    // Only show data if debug is enabled
    if (mcp3424_debug_enabled) {
        // Read results from current channel for all devices
        for (uint8_t device = 0; device < data.deviceCount; device++) {
            if (mcp3424_devices[device] == nullptr) continue;
            
            long adcValue = 0;
            MCP342x::Config status;
            
            // Read the conversion result for current channel
            uint8_t err = mcp3424_devices[device]->read(adcValue, status);
            
            if (err == 0) {
                // Convert to voltage (2.048V reference, 18-bit, gain=1)
                data.channels[device][mcp3424_current_channel] = (adcValue*4) / (data.gain);

              //raw adc /gain / 131072.0
             // data.channels[device][mcp3424_current_channel] = adcValue / (data.gain );
              overallSuccess = true;
                
                // Show raw ADC value and voltage
                safePrint("Dev");
                safePrint(String(device));
                safePrint("Ch");
                safePrint(String(mcp3424_current_channel + 1));
                safePrint("=");
                safePrint(String(adcValue));
                safePrint("(");
                safePrint(String(data.channels[device][mcp3424_current_channel], 6));
                safePrint("V) ");
            } else {
                data.channels[device][mcp3424_current_channel] = 0.0;
                safePrint("Dev");
                safePrint(String(device));
                safePrint("Ch");
                safePrint(String(mcp3424_current_channel + 1));
                safePrint("=ERR");
                safePrint(String(err));
                safePrint(" ");
            }
        }
    } else {
        // Silent mode - just read data without printing
        for (uint8_t device = 0; device < data.deviceCount; device++) {
            if (mcp3424_devices[device] == nullptr) continue;
            
            long adcValue = 0;
            MCP342x::Config status;
            
            uint8_t err = mcp3424_devices[device]->read(adcValue, status);
            
            if (err == 0) {
                data.channels[device][mcp3424_current_channel] = (adcValue*4) / (data.gain);
                overallSuccess = true;
            } else {
                data.channels[device][mcp3424_current_channel] = 0.0;
            }
        }
    }
    
    if (overallSuccess && mcp3424_debug_enabled) {
        // Oblicz czas konwersji dla tego kanału
        unsigned long channel_conversion_time = millis() - mcp3424_conversion_start_time;
        unsigned long channel_read_time = millis() - read_start;
        
        safePrint(" -> Channel ");
        safePrint(String(mcp3424_current_channel + 1));
        safePrint(" read OK: konwersja=");
        safePrint(String(channel_conversion_time));
        safePrint("ms, odczyt=");
        safePrint(String(channel_read_time));
        safePrint("ms, total=");
        safePrint(String(channel_conversion_time + channel_read_time));
        safePrintln("ms");
    }
    
    if (overallSuccess) {
        // Update device validity (simplified - all devices or none)
        for (uint8_t device = 0; device < data.deviceCount; device++) {
            data.valid[device] = true;
        }
        data.lastUpdate = millis();
    }
    
    // Move to next channel (0-3 cycle)
    mcp3424_current_channel = (mcp3424_current_channel + 1) % 4;
    
    // Show complete cycle info when we complete all 4 channels
    if (mcp3424_current_channel == 0 && mcp3424_debug_enabled) {
        unsigned long total_cycle_time = millis() - mcp3424_cycle_start_time;
        safePrint("=== KOMPLETNY CYKL 4 KANALOW ZAKONCZONY: ");
        safePrint(String(total_cycle_time));
        safePrint("ms dla ");
        safePrint(String(data.deviceCount));
        safePrint(" urzadzen (");
        safePrint(String(total_cycle_time / (data.deviceCount * 4)));
        safePrint("ms/kanal/urzadzenie) ===");
        safePrintln("");
    }
    
    mcp3424_conversion_in_progress = false;
    xSemaphoreGive(i2c_semaphore);
    return overallSuccess;
}

// FreeRTOS task for async MCP3424 conversion
void mcp3424ConversionTask(void* parameters) {
    const TickType_t conversionDelay = pdMS_TO_TICKS(300); // 300ms for 18-bit conversion
    
    for (;;) {
        if (mcp3424_conversion_in_progress) {
            // Wait for conversion to complete (18-bit needs ~266ms per channel)
            vTaskDelay(conversionDelay);
            
            // Check if conversion is still in progress and enough time has passed
            if (mcp3424_conversion_in_progress && 
                (millis() - mcp3424_conversion_start_time) >= 300) {
                
                // Read the results from current channel and move to next
                readMCP3424Results(mcp3424Data);
            }
        }
        
        // Small delay to prevent busy waiting
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

bool readMCP3424(MCP3424Data& data) {
    // Check if conversion is in progress
    if (mcp3424_conversion_in_progress) {
        // Check if enough time has passed for 18-bit conversion (300ms per channel)
        unsigned long elapsed = millis() - mcp3424_conversion_start_time;
        if (elapsed >= 300) {
            // Conversion should be ready, results will be read by task
            return false; // Task will handle reading and channel switching
        } else {
            // Still waiting for conversion
            return false;
        }
    }
    
    // No conversion in progress, start a new one
    static unsigned long lastConversionStart = 0;
    unsigned long currentTime = millis();
    
    // Minimum interval between channel conversions (400ms per channel cycle)
    if (currentTime - lastConversionStart < 400) {
        return false; // Too soon for next channel conversion
    }
    
    bool success = startMCP3424Conversion(data);
    
    if (success) {
        lastConversionStart = currentTime;
    } else if (mcp3424_debug_enabled) {
        safePrint("MCP3424: Failed to start channel ");
        safePrint(String(mcp3424_current_channel + 1));
        safePrintln(" conversion");
    }
    
    return false; // Results will be available after task completes
}

bool readADS1110(ADS1110Data& data) {
    // Take I2C semaphore for thread safety
    if (xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("ADS1110: Failed to take I2C semaphore");
        data.valid = false;
        return false;
    }
    
    uint32_t start_time = millis();
    
    // Uśrednianie 5 próbek - statyczne tablice dla zachowania stanu między wywołaniami
    static float voltage_samples[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    static int16_t raw_samples[5] = {0, 0, 0, 0, 0};
    static uint8_t sample_index = 0;
    static uint8_t valid_samples = 0;
    
    // ADS1110 Configuration byte:
    // bit 7: ST (start conversion) = 1
    // bit 6-5: SC (single/continuous) = 00 for single shot
    // bit 4-2: DR (data rate): 00=15 SPS, 01=30 SPS, 10=60 SPS, 11=240 SPS
    // bit 1-0: PGA gain: 00=1x, 01=2x, 10=4x, 11=8x
    
    uint8_t configByte = 0x80;  // Start conversion (bit 7 = 1)
    // Add data rate bits (bits 4-2)
    if (data.dataRate == 30) configByte |= (1 << 2);
    else if (data.dataRate == 60) configByte |= (2 << 2);  
    else if (data.dataRate == 240) configByte |= (3 << 2);
    // Add gain bits (bits 1-0)
    if (data.gain == 2) configByte |= 1;
    else if (data.gain == 4) configByte |= 2;
    else if (data.gain == 8) configByte |= 3;
    
    // Start conversion with retry mechanism
    int retry_count = 0;
    const int max_retries = 3;
    bool conversion_started = false;
    
    while (retry_count < max_retries && !conversion_started) {
        Wire.beginTransmission(ADS1110_DEFAULT_ADDR);
        Wire.write(configByte);
        if (Wire.endTransmission() == 0) {
            conversion_started = true;
        } else {
            retry_count++;
            delay(5); // Short delay before retry
        }
    }
    
    if (!conversion_started) {
        safePrintln("ADS1110: Failed to start conversion after retries");
        data.valid = false;
        xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    // Wait for conversion based on data rate with timeout protection
    uint32_t conversionTime = 1000 / data.dataRate + 50; // Reduced margin for speed
    if (conversionTime > I2C_TIMEOUT_MS) conversionTime = I2C_TIMEOUT_MS;
    
    delay(conversionTime);
    
    // Check for timeout
    if (millis() - start_time > I2C_TIMEOUT_MS) {
        safePrintln("ADS1110: Conversion timeout detected");
        data.valid = false;
        xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    // Read 2 bytes of conversion result with retry
    retry_count = 0;
    bool read_success = false;
    int16_t current_raw = 0;
    
    while (retry_count < max_retries && !read_success) {
        Wire.requestFrom(ADS1110_DEFAULT_ADDR, 2);
        
        uint32_t readStart = millis();
        while (Wire.available() < 2 && (millis() - readStart) < 50) { // Shorter timeout
            delay(1);
        }
        
        if (Wire.available() >= 2) {
            uint8_t msb = Wire.read();
            uint8_t lsb = Wire.read();
            
            // Combine bytes into 16-bit signed value - bieżąca próbka
            current_raw = (msb << 8) | lsb;
            
            // Sprawdź poprawność danych
            if ((msb == 0xFF && lsb == 0xFF) || (msb == 0x00 && lsb == 0x00)) {
                safePrint("ADS1110: Invalid data received (0x");
                safePrint(String(msb, HEX));
                safePrint(String(lsb, HEX));
                safePrintln(")");
                retry_count++;
                delay(10);
                continue;
            }
            
            read_success = true;
        } else {
            retry_count++;
            safePrint("ADS1110: Read timeout (attempt ");
            safePrint(String(retry_count));
            safePrintln(")");
            delay(10);
        }
    }
    
    if (!read_success) {
        safePrintln("ADS1110: Failed to read data after retries");
        data.valid = false;
        xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    // Convert to voltage: ADS1110 has 2.048V reference, 16-bit resolution - bieżąca próbka
    float current_voltage = ((float)current_raw / 32768.0) * (2.048 / data.gain);
    
    // === UŚREDNIANIE 5 PRÓBEK ===
    
    // Dodaj bieżącą próbkę do tablicy
    voltage_samples[sample_index] = current_voltage;
    raw_samples[sample_index] = current_raw;
    
    // Zwiększ licznik próbek (do maksymalnie 5)
    if (valid_samples < 5) {
        valid_samples++;
    }
    
    // Przesuń indeks (cyklicznie 0-4)
    sample_index = (sample_index + 1) % 5;
    
    // Oblicz uśrednione wartości
    float avg_voltage = 0.0;
    float avg_raw = 0.0;
    
    for (uint8_t i = 0; i < valid_samples; i++) {
        avg_voltage += voltage_samples[i];
        avg_raw += raw_samples[i];
    }
    avg_voltage /= valid_samples;
    avg_raw /= valid_samples;
    
    // Zwróć uśrednione wartości
    data.voltage = avg_voltage;
    data.valid = true;
    data.lastUpdate = millis();
    
    // Debug info (tylko co 10 próbek żeby nie spamować)
    static uint8_t debug_counter = 0;
    if (++debug_counter >= 10) {
            safePrint("ADS1110: Uśredniono z ");
    safePrint(String(valid_samples));
    safePrint(" próbek: ");
    safePrint(String(avg_voltage, 6));
    safePrint("V (bieżąca: ");
    safePrint(String(current_voltage, 6));
    safePrint("V, Gain: ");
    safePrint(String(data.gain));
    safePrint("x, Rate: ");
    safePrint(String(data.dataRate));
    safePrintln(" SPS)");
    debug_counter = 0;
}

xSemaphoreGive(i2c_semaphore);
return true;
}

bool readINA219(INA219Data& data) {
    // Take I2C semaphore for thread safety
    if (xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("INA219: Failed to take I2C semaphore");
        data.valid = false;
        return false;
    }
    
    uint32_t start_time = millis();
    
    // Uśrednianie 5 próbek - statyczne tablice dla zachowania stanu między wywołaniami
    static float bus_voltage_samples[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    static float shunt_voltage_samples[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    static float current_samples[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    static float power_samples[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    static uint8_t sample_index = 0;
    static uint8_t valid_samples = 0;
    
    // Read all values with timeout protection and retry mechanism
    int retry_count = 0;
    const int max_retries = 3;
    bool read_success = false;
    
    float current_bus_voltage = 0.0;
    float current_shunt_voltage = 0.0;
    float current_current = 0.0;
    float current_power = 0.0;
    
    while (retry_count < max_retries && !read_success) {
        bool single_read_success = true;
        
        // Read bus voltage with timeout check
        current_bus_voltage = ina219.getBusVoltage_V();
        if (millis() - start_time > I2C_TIMEOUT_MS) {
            single_read_success = false;
        }
        
        if (!single_read_success) {
            retry_count++;
            delay(10);
            continue;
        }
        
        // Read shunt voltage with timeout check
        current_shunt_voltage = ina219.getShuntVoltage_mV();
        if (millis() - start_time > I2C_TIMEOUT_MS) {
            single_read_success = false;
        }
        
        if (!single_read_success) {
            retry_count++;
            delay(10);
            continue;
        }
        
        // Read current with timeout check
        current_current = ina219.getCurrent_mA();
        if (millis() - start_time > I2C_TIMEOUT_MS) {
            single_read_success = false;
        }
        
        if (!single_read_success) {
            retry_count++;
            delay(10);
            continue;
        }
        
        // Read power with timeout check
        current_power = ina219.getPower_mW();
        if (millis() - start_time > I2C_TIMEOUT_MS) {
            single_read_success = false;
        }
        
        if (single_read_success) {
            // Validate readings
            if (current_bus_voltage < 0.0 || current_bus_voltage > 32.0 ||
                current_shunt_voltage < -320.0 || current_shunt_voltage > 320.0 ||
                abs(current_current) > 3200.0 || current_power < 0.0) {
                
                safePrint("INA219: Invalid readings - Bus: ");
                safePrint(String(current_bus_voltage, 3));
                safePrint("V, Shunt: ");
                safePrint(String(current_shunt_voltage, 2));
                safePrint("mV, Current: ");
                safePrint(String(current_current, 2));
                safePrint("mA, Power: ");
                safePrint(String(current_power, 2));
                safePrintln("mW");
                
                retry_count++;
                delay(10);
                continue;
            }
            
            read_success = true;
        } else {
            retry_count++;
            delay(10);
        }
    }
    
    if (!read_success) {
        safePrint("INA219: Failed to read data after ");
        safePrint(String(max_retries));
        safePrintln(" retries");
        data.valid = false;
        xSemaphoreGive(i2c_semaphore);
        return false;
    }
    
    // === UŚREDNIANIE 5 PRÓBEK ===
    
    // Dodaj bieżące próbki do tablic
    bus_voltage_samples[sample_index] = current_bus_voltage;
    shunt_voltage_samples[sample_index] = current_shunt_voltage;
    current_samples[sample_index] = current_current;
    power_samples[sample_index] = current_power;
    
    // Zwiększ licznik próbek (do maksymalnie 5)
    if (valid_samples < 5) {
        valid_samples++;
    }
    
    // Przesuń indeks (cyklicznie 0-4)
    sample_index = (sample_index + 1) % 5;
    
    // Oblicz uśrednione wartości
    float avg_bus_voltage = 0.0;
    float avg_shunt_voltage = 0.0;
    float avg_current = 0.0;
    float avg_power = 0.0;
    
    for (uint8_t i = 0; i < valid_samples; i++) {
        avg_bus_voltage += bus_voltage_samples[i];
        avg_shunt_voltage += shunt_voltage_samples[i];
        avg_current += current_samples[i];
        avg_power += power_samples[i];
    }
    avg_bus_voltage /= valid_samples;
    avg_shunt_voltage /= valid_samples;
    avg_current /= valid_samples;
    avg_power /= valid_samples;
    
    // Zwróć uśrednione wartości
    data.busVoltage = avg_bus_voltage;
    data.shuntVoltage = avg_shunt_voltage;
    data.current = avg_current;
    data.power = avg_power;
    data.valid = true;
    data.lastUpdate = millis();
    
    // Debug info (tylko co 10 próbek żeby nie spamować)
    static uint8_t debug_counter = 0;
    if (++debug_counter >= 10) {
            safePrint("INA219: Uśredniono z ");
    safePrint(String(valid_samples));
    safePrint(" próbek: ");
    safePrint(String(avg_current, 2));
    safePrint("mA (bieżąca: ");
    safePrint(String(current_current, 2));
    safePrint("mA), Bus: ");
    safePrint(String(avg_bus_voltage, 3));
    safePrint("V, Power: ");
    safePrint(String(avg_power, 2));
    safePrintln("mW");
    debug_counter = 0;
}

xSemaphoreGive(i2c_semaphore);
return true;
}

void configureADS1110(uint8_t dataRate, uint8_t gain) {
    if (!ads1110SensorStatus) {
        safePrintln("ADS1110 not initialized");
        return;
    }
    
    // Validate data rate (15, 30, 60, 240 SPS)
    if (dataRate != 15 && dataRate != 30 && dataRate != 60 && dataRate != 240) {
        safePrintln("Invalid ADS1110 data rate. Valid: 15, 30, 60, 240 SPS");
        return;
    }
    
    // Validate gain (1, 2, 4, 8)
    if (gain != 1 && gain != 2 && gain != 4 && gain != 8) {
        safePrintln("Invalid ADS1110 gain. Valid: 1, 2, 4, 8");
        return;
    }
    
    ads1110Data.dataRate = dataRate;
    ads1110Data.gain = gain;
    
    safePrint("ADS1110 configured - Data Rate: ");
    safePrint(String(dataRate));
    safePrint(" SPS, Gain: ");
    safePrint(String(gain));
    safePrintln("x");
}

void setMCP3424Debug(bool enabled) {
    mcp3424_debug_enabled = enabled;
    safePrint("MCP3424 debug ");
    if (enabled) {
        safePrintln("ENABLED - będą wypisywane dane i czasy");
    } else {
        safePrintln("DISABLED - tylko cichy odczyt danych");
    }
}

void scd41Task(void* parameters) {
    safePrintln("SCD41 task started");
    
    while (scd41_task_running) {
        // Check if data is ready
        bool dataReady = false;
        uint16_t error;
        char errorMessage[256];
        
        // Take I2C semaphore
        if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
            error = scd41.getDataReadyStatus(dataReady);
            if (error != NO_ERROR) {
                errorToString(error, errorMessage, 256);
                safePrint("SCD41: Error trying to execute getDataReadyStatus(): ");
                safePrintln(errorMessage);
                xSemaphoreGive(i2c_semaphore);
                vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second before retry
                continue;
            }
            
            // If data is not ready, wait and try again
            if (!dataReady) {
                xSemaphoreGive(i2c_semaphore);
                vTaskDelay(pdMS_TO_TICKS(100)); // Wait 100ms before checking again
                continue;
            }
            
            // Data is ready, read measurement
            uint16_t co2Concentration = 0;
            float temperature = 0.0;
            float relativeHumidity = 0.0;
            
            error = scd41.readMeasurement(co2Concentration, temperature, relativeHumidity);
            if (error != NO_ERROR) {
                errorToString(error, errorMessage, 256);
                safePrint("SCD41: Error trying to execute readMeasurement(): ");
                safePrintln(errorMessage);
                xSemaphoreGive(i2c_semaphore);
                vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second before retry
                continue;
            }
            
            // Validate readings
            if (co2Concentration >= 400 && co2Concentration <= 5000 &&
                temperature >= -40.0 && temperature <= 85.0 &&
                relativeHumidity >= 0.0 && relativeHumidity <= 100.0) {
                
                // Update global data - only CO2, preserve temperature and humidity from SHT40
                extern I2CSensorData i2cSensorData;
                extern SHT40Data sht40Data;
                extern FeatureConfig config;
                
                // Only update CO2 from SCD41, keep temperature and humidity from SHT40 if available
                if (config.enableSHT40 && sht40Data.valid) {
                    i2cSensorData.temperature = sht40Data.temperature;
                    i2cSensorData.humidity = sht40Data.humidity;
                    i2cSensorData.pressure = sht40Data.pressure;
                } else {
                    // Fallback to SCD41 temperature and humidity if SHT40 not available
                    i2cSensorData.temperature = temperature;
                    i2cSensorData.humidity = relativeHumidity;
                    i2cSensorData.pressure = 0.0; // SCD41 doesn't have pressure
                }
                i2cSensorData.co2 = (float)co2Concentration;
                i2cSensorData.valid = true;
                i2cSensorData.lastUpdate = millis();
                i2cSensorData.type = SENSOR_SCD41;
                
                scd41_data_ready = true;
                scd41_last_read_time = millis();
                
                // Debug output every 30 seconds
                static unsigned long lastSCD41Debug = 0;
                if (millis() - lastSCD41Debug > 30000) {
                    lastSCD41Debug = millis();
                    safePrint("SCD41 Task - CO2: ");
                    safePrint(String(co2Concentration));
                    safePrint(" ppm, Temp: ");
                    safePrint(String(temperature, 2));
                    safePrint("°C, Humidity: ");
                    safePrint(String(relativeHumidity, 2));
                    safePrintln("%");
                }
            } else {
                safePrintln("SCD41: Invalid readings detected");
            }
            
            xSemaphoreGive(i2c_semaphore);
        }
        
        // Wait before next measurement cycle
        vTaskDelay(pdMS_TO_TICKS(5000)); // 5 second interval
    }
    
    safePrintln("SCD41 task stopped");
    vTaskDelete(NULL);
}

void resetSCD41() {
    // Take I2C semaphore
    if (i2c_semaphore && xSemaphoreTake(i2c_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
        safePrintln("SCD41: Failed to take I2C semaphore for reset");
        return;
    }

    uint16_t error;
    char errorMessage[256];
    
    // Stop periodic measurement
    error = scd41.stopPeriodicMeasurement();
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, 256);
        safePrint("SCD41: Error stopping measurement during reset: ");
        safePrintln(errorMessage);
    }
    
    // Wait a bit after stopping measurement
    delay(1000);
    
    // Perform soft reset using Sensirion library
    error = scd41.performFactoryReset();
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, 256);
        safePrint("SCD41: Error during factory reset: ");
        safePrintln(errorMessage);
    }
    
    // Wait for reset to complete
    delay(1000);
    
    // Restart periodic measurement
    error = scd41.startPeriodicMeasurement();
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, 256);
        safePrint("SCD41: Error restarting measurement after reset: ");
        safePrintln(errorMessage);
    }
    
    // Reset global data - only CO2, preserve temperature and humidity from SHT40
    extern I2CSensorData i2cSensorData;
    extern SHT40Data sht40Data;
    extern FeatureConfig config;
    
    // Only reset CO2, preserve temperature and humidity from SHT40 if available
    if (config.enableSHT40 && sht40Data.valid) {
        i2cSensorData.temperature = sht40Data.temperature;
        i2cSensorData.humidity = sht40Data.humidity;
        i2cSensorData.pressure = sht40Data.pressure;
        i2cSensorData.valid = true; // Keep valid if SHT40 data is available
    } else {
        i2cSensorData.valid = false;
        i2cSensorData.temperature = 0.0;
        i2cSensorData.humidity = 0.0;
        i2cSensorData.pressure = 0.0;
    }
    i2cSensorData.co2 = 0.0; // Reset CO2
    i2cSensorData.lastUpdate = 0;
    scd41SensorStatus = false;
    
    // Reset task variables
    scd41_data_ready = false;
    scd41_last_read_time = 0;
    
    // Release I2C semaphore
    if (i2c_semaphore) xSemaphoreGive(i2c_semaphore);
    
    safePrintln("SCD41 sensor reset completed");
} 