#include <Arduino.h>
#include <esp_task_wdt.h>
#include <Adafruit_NeoPixel.h>
#include <time.h>

// Project includes
#include <config.h>
#include <sensors.h>
#include <i2c_sensors.h>
#include <ips_sensor.h>
#include <modbus_handler.h>
#include <web_server.h>
#include <mean.h>
#include <calib.h>
#include <network_config.h>
#include <history.h>
#include <fan.h>

//#include <html.h>

// Global configuration
FeatureConfig config;
SerialSensorConfig serialSensorConfigs[MAX_SERIAL_SENSORS];

// Hardware objects
HardwareSerial MySerial(2); // Solar sensor serial
Adafruit_NeoPixel pixels(1, WS2812_PIN, NEO_GRB + NEO_KHZ800);

// Global variables
bool sendDataFlag = false;
unsigned long lastSensorCheck = 0;
unsigned long lastMovingAverageUpdate = 0;
// Function declarations
void setup();
void loop();
void watchDogTask(void *parameter);
void initializeHardware();
void processSerialCommands();
void updateSystemStatus();



// Safe Serial printing functions
void safePrint(const String& message) {
    if (Serial && Serial.availableForWrite()) {
        Serial.print(message);
    }
}

void safePrintln(const String& message) {
    if (Serial && Serial.availableForWrite()) {
        Serial.println(message);
    }
}

bool isSerialAvailable() {
    return Serial && Serial.availableForWrite();
}

void setup() {
    // Initialize serial communication with timeout - non-blocking
    Serial.begin(SERIAL_BAUD);
    unsigned long startMillis = millis();
    while (!Serial && millis() - startMillis < 2000) {
        ; // Wait for serial port to connect with short timeout
    }
    Serial.setTimeout(1000);
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    // Don't wait for Serial - system should work without it
    if (Serial) {
        Serial.println("=== ESP32 Multi-Sensor System Starting ===");
      //  pinMode(IPS_POWER_PIN, OUTPUT);
      //  digitalWrite(IPS_POWER_PIN, LOW);  // Set to GND
        delay(1000);  // Wait for power stabilization
        
        // Print configuration
        Serial.println("Configuration:");
        Serial.println("- Solar Sensor: " + String(config.enableSolarSensor ? "ENABLED" : "DISABLED"));
        Serial.println("- OPCN3 Sensor: " + String(config.enableOPCN3Sensor ? "ENABLED" : "DISABLED"));
        Serial.println("- I2C Sensors: " + String(config.enableI2CSensors ? "ENABLED" : "DISABLED"));
        Serial.println("- IPS Sensor: " + String(config.enableIPS ? "ENABLED" : "DISABLED"));
        Serial.println("- Modbus: " + String(config.enableModbus ? "ENABLED" : "DISABLED"));
        Serial.println("- Web Server: " + String(config.enableWebServer ? "ENABLED" : "DISABLED"));
        Serial.println("- WiFi: " + String(config.enableWiFi ? "ENABLED" : "DISABLED"));
        
        // Warning if both Solar and IPS are enabled (they share UART)
        if (config.enableSolarSensor && config.enableIPS) {
            Serial.println("WARNING: Solar and IPS sensors share the same UART - only one can work!");
        }
    } else {
        // System starting without Serial - just configure IPS power
      //  pinMode(IPS_POWER_PIN, OUTPUT);
      //  digitalWrite(IPS_POWER_PIN, LOW);  // Set to GND
        delay(1000);  // Wait for power stabilization
    }

     if (config.enableWiFi) {
        initializeWiFi();
    }
    if (config.enableWebServer && config.enableWiFi) {
        initializeWebServer();
    }
    
    // Initialize hardware
    initializeHardware();
    
    // Initialize sensors
    initializeSensors();
    
    // Initialize MCP3424 mapping
    safePrintln("Initializing MCP3424 mapping...");
    if (!loadMCP3424Config(mcp3424Config)) {
        safePrintln("No MCP3424 config found, creating default...");
        initializeDefaultMCP3424Mapping();
        saveMCP3424Config(mcp3424Config);
        safePrintln("Default MCP3424 config saved");
    } else {
        safePrintln("MCP3424 config loaded: " + String(mcp3424Config.deviceCount) + " devices");
    }
    
    // Initialize moving averages system
    initializeMovingAverages();
    
    // Initialize sensor history in PSRAM
    initializeHistory();
    
    // Initialize communication protocols
    if (config.enableModbus) {
        lastModbusActivity = millis();
        initializeModbus();
    }
    
   
    
    
    // Create system tasks
    xTaskCreate(
        watchDogTask,    // Task function
        "Watchdog",      // Task name
        10000,           // Stack size
        NULL,            // Parameters
        5,               // Priority
        NULL             // Task handle
    );
    
    if (config.enableWiFi) {
        xTaskCreate(
            WiFiReconnectTask, // Task function
            "WiFi Reconnect",  // Task name
            10000,             // Stack size
            NULL,              // Parameters
            1,                 // Priority
            NULL               // Task handle
        );
    }
    
    if (Serial) {
        Serial.println("=== System initialization complete ===");
    }
    delay(1000);
}

void loop() {
    unsigned long currentTime = millis();

    esp_task_wdt_reset();
    
    // Read sensors
    if (config.enableSolarSensor) {
        readSolarSensor();
    }
    
    if (config.enableOPCN3Sensor) {
        readOPCN3Sensor();
    }
    
    if (config.enableI2CSensors) {
        readI2CSensors();
        
        // Wykonaj kalibrację po odczycie danych z MCP3424
        if (config.enableMCP3424 && mcp3424SensorStatus) {
            performCalibration();
        }
    }
    
    if (config.enableIPS) {
        readIPSSensor();
    }
    
    if (config.enableHCHO) {
        readHCHO();
    }
    
    readBatteryVoltage();
    readSerialSensors();
    
    // Update fan RPM measurement if enabled
    if (config.enableFan) {
        updateFanRPM();
    }
    
    // Update Modbus registers
    if (config.enableModbus) {
        processModbusTask();
        
        if (solarSensorStatus && currentTime - lastSensorCheck >= 1000) {
            updateModbusSolarRegisters();
        }
        
        if (opcn3SensorStatus) {
            updateModbusOPCN3Registers();
        }
        
        if (i2cSensorStatus) {
            updateModbusI2CRegisters();
        }
        
        if (mcp3424SensorStatus) {
            updateModbusMCP3424Registers();
        }
        
        if (ads1110SensorStatus) {
            updateModbusADS1110Registers();
        }
        
        if (ina219SensorStatus) {
            updateModbusINA219Registers();
        }
        
        if (sht40SensorStatus) {
            updateModbusSHT40Registers();
        }
        
        if (ipsSensorStatus) {
            updateModbusIPSRegisters();
        }
        
        if (hchoSensorStatus) {
            updateModbusHCHORegisters();
        }
        
        // Debug SPS30 status
        static unsigned long lastSPS30Debug = 0;
        if (currentTime - lastSPS30Debug > 15000) { // Every 15 seconds
            lastSPS30Debug = currentTime;

        }
        
        if (sps30SensorStatus) {
            updateModbusSPS30Registers();
        }
        
        // Update calibrated data registers if calibration is enabled
        if (calibConfig.enableCalibration && calibratedData.valid) {
            updateModbusCalibrationRegisters();
        }
        
        lastSensorCheck = currentTime;
    }
    
    // Process serial commands
    processSerialCommands();
    
    // Update system status indicators
    updateSystemStatus();

    // Update moving averages every 5 seconds
    if (currentTime - lastMovingAverageUpdate >= 5000) {
        updateMovingAverages();
        lastMovingAverageUpdate = currentTime;
    }
    
    // Update sensor history
    updateSensorHistory();
    
    // Auto reset check
    if (config.autoReset) {
        static unsigned long lastValidData = millis();
        static unsigned long lastStatusPrint = 0;
        
        // Check if any enabled sensors have valid data
        bool hasValidData = false;
        if (config.enableSolarSensor && solarSensorStatus) hasValidData = true;
        if (config.enableOPCN3Sensor && opcn3SensorStatus) hasValidData = true;
        if (config.enableI2CSensors && i2cSensorStatus) hasValidData = true;
        if (config.enableSHT40 && sht40SensorStatus) hasValidData = true;
        if (config.enableIPS && ipsSensorStatus) hasValidData = true;
        if (config.enableMCP3424 && mcp3424SensorStatus) hasValidData = true;
        
        if (hasValidData) {
            lastValidData = currentTime;
        }
        
        // Check for sensor timeout - only if any sensors are enabled
        bool anySensorEnabled = config.enableSolarSensor || config.enableOPCN3Sensor || 
                               config.enableI2CSensors || config.enableSHT40 || config.enableIPS;
        
        if (anySensorEnabled && (currentTime - lastValidData > SENSOR_TIMEOUT)) {
            safePrintln("No data from enabled sensors for 2 minutes. Restarting...");
            delay(1000);
            ESP.restart();
        }
        
        // Check for Modbus timeout - only if there was previous activity and system running >2 minutes
        if (config.enableModbus && hasHadModbusActivity && (currentTime > 120000) && (currentTime+1000 - lastModbusActivity > MODBUS_TIMEOUT)) {
            safePrint("No Modbus activity for 5 minutes. Last activity: ");
            safePrint(String((currentTime - lastModbusActivity) / 1000));
            safePrint("s ago. Restarting...");
            safePrintln("");
            delay(1000);
            ESP.restart();
        }
        
        // Print status every 60 seconds for debugging
        if (currentTime - lastStatusPrint >= 60000) {
            lastStatusPrint = currentTime;
            if (isSerialAvailable()) {
                safePrint("Auto-reset status - Last valid data: ");
                safePrint(String((currentTime - lastValidData) / 1000));
                safePrint("s ago");
                if (config.enableModbus) {
                    safePrint(", Modbus: ");
                    if (hasHadModbusActivity) {
                        safePrint(String((currentTime - lastModbusActivity) / 1000));
                        safePrint("s ago");
                    } else {
                        safePrint("never (uptime: ");
                        safePrint(String(currentTime / 1000));
                        safePrint("s)");
                    }
                }
                safePrintln("");
                
                // Print moving averages buffer status
                printMovingAverageStatus();
                
                // Print history status
                printHistoryMemoryUsage();
                printHistoryStatus();
            }
        }
    }
    
    // Small delay to prevent overwhelming the system
    delay(10);
}

void initializeHardware() {
    // Initialize LED strip
    pixels.begin();
    pixels.setPixelColor(0, pixels.Color(255, 255, 255)); // White during initialization
    pixels.show();
    
    // Initialize solar sensor serial if enabled
    // if (config.enableSolarSensor) {
    //     MySerial.begin(SOLAR_SERIAL_BAUD, SERIAL_8N1, SOLAR_RX_PIN, SOLAR_TX_PIN);
    //     while (!MySerial) {
    //         delay(100);
    //     }
    //     safePrintln("Solar sensor serial initialized");
    // }
    
    // Configure additional serial sensors
    // Example configuration - can be modified as needed
   // configureSerialSensor(0, 16, 17, 9600, "GPS_Module", "NMEA");
   // configureSerialSensor(1, 18, 19, 115200, "Additional_Sensor", "JSON");
    
    // Initialize fan control system if enabled
    if (config.enableFan) {
        initializeFan();
    }
    
    safePrintln("Hardware initialization complete");
}

void processSerialCommands() {
    // Process commands from main serial - only if connected
    if (Serial && Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.equals("SEND")) {
            sendDataFlag = true;
        }
        else if (command.equals("STATUS")) {
            if (isSerialAvailable()) {
                safePrintln("=== System Status ===");
                safePrintln("Solar Sensor: " + String(solarSensorStatus ? "OK" : "ERROR"));
                safePrintln("OPCN3 Sensor: " + String(opcn3SensorStatus ? "OK" : "ERROR"));
                safePrintln("I2C Overall: " + String(i2cSensorStatus ? "OK" : "ERROR"));
                safePrintln("  - SHT30: " + String(sht30SensorStatus ? "OK" : (config.enableSHT30 ? "ERROR" : "DISABLED")));
                safePrintln("  - BME280: " + String(bme280SensorStatus ? "OK" : (config.enableBME280 ? "ERROR" : "DISABLED")));
                safePrintln("  - SCD41: " + String(scd41SensorStatus ? "OK" : (config.enableSCD41 ? "ERROR" : "DISABLED")));
                safePrintln("  - SHT40: " + String(sht40SensorStatus ? "OK" : (config.enableSHT40 ? "ERROR" : "DISABLED")));
                safePrintln("  - MCP3424: " + String(mcp3424SensorStatus ? "OK" : (config.enableMCP3424 ? "ERROR" : "DISABLED")));
                safePrintln("  - ADS1110: " + String(ads1110SensorStatus ? "OK" : (config.enableADS1110 ? "ERROR" : "DISABLED")));
                safePrintln("  - INA219: " + String(ina219SensorStatus ? "OK" : (config.enableINA219 ? "ERROR" : "DISABLED")));
                safePrintln("  - SPS30: " + String(sps30SensorStatus ? "OK" : (config.enableSPS30 ? "ERROR" : "DISABLED")));
                safePrintln("IPS Sensor: " + String(ipsSensorStatus ? "OK" : (config.enableIPS ? "ERROR" : "DISABLED")));
                safePrintln("HCHO Sensor: " + String(hchoSensorStatus ? "OK" : (config.enableHCHO ? "ERROR" : "DISABLED")));
                if (config.enableFan) {
                    safePrintln("Fan Control: " + String(isFanEnabled() ? "ON" : "OFF") + " (" + String(getFanDutyCycle()) + "%, " + String(getFanRPM()) + " RPM)");
                    safePrintln("GLine Router: " + String(isGLineEnabled() ? "ENABLED" : "DISABLED"));
                } else {
                    safePrintln("Fan Control: DISABLED");
                }
                
                safePrintln("WiFi: " + String(WiFi.status() == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED"));
                //ip address
                safePrintln("IP Address: " + String(WiFi.localIP().toString()));    
                safePrintln("Auto Reset: " + String(config.autoReset ? "ENABLED" : "DISABLED"));
                if (config.enableModbus) {
                    safePrint("Modbus Activity: ");
                    if (hasHadModbusActivity) {
                        safePrintln("Last " + String((millis() - lastModbusActivity) / 1000) + " seconds ago");
                    } else {
                        safePrintln("Never detected");
                    }
                }
                safePrintln("Free Heap: " + String(ESP.getFreeHeap()));
                safePrintln("PSRAM Size: " + String(ESP.getPsramSize() / 1024) + " KB");
                safePrintln("Free PSRAM: " + String(ESP.getFreePsram() / 1024) + " KB");
                safePrintln("Uptime: " + String(millis() / 1000) + " seconds");
                
                // Time info from ESP32 built-in time functions
                extern bool timeInitialized;
                extern String getFormattedTime();
                extern String getFormattedDate();
                extern time_t getEpochTime();
                extern bool isTimeSet();
                
                if (isTimeSet()) {
                    safePrintln("Time: " + getFormattedTime());
                    safePrintln("Date: " + getFormattedDate());
                    safePrintln("Epoch: " + String(getEpochTime()));
                    
                    struct tm timeinfo;
                    if (getLocalTime(&timeinfo)) {
                        safePrintln("Day of Week: " + String(timeinfo.tm_wday));
                        safePrintln("Timezone: Poland (UTC+1/UTC+2 with DST)");
                    }
                } else {
                    safePrintln("Time: Not synchronized");
                }
                
                // Calibration status
                extern CalibrationConfig calibConfig;
                extern CalibratedSensorData calibratedData;
                safePrintln("Calibration: " + String(calibConfig.enableCalibration ? "ENABLED" : "DISABLED"));
                if (calibConfig.enableCalibration) {
                    safePrintln("Calibration Valid: " + String(calibratedData.valid ? "YES" : "NO"));
                    safePrintln("Last Calibration: " + String((millis() - calibratedData.lastUpdate) / 1000) + " seconds ago");
                }
                
                safePrintln("=== Moving Averages Status ===");
                printMovingAverageStatus();
            }
        }
        else if (command.equals("AVGSTATUS")) {
            if (isSerialAvailable()) {
                safePrintln("=== Moving Averages Buffer Status ===");
                printMovingAverageStatus();
            }
        }
        else if (command.startsWith("CONFIG_")) {
            // Configuration commands
            if (command.equals("CONFIG_SOLAR_ON")) {
                config.enableSolarSensor = true;
                safePrintln("Solar sensor enabled");
            }
            else if (command.equals("CONFIG_SOLAR_OFF")) {
                config.enableSolarSensor = false;
                safePrintln("Solar sensor disabled");
            }
            else if (command.equals("CONFIG_OPCN3_ON")) {
                config.enableOPCN3Sensor = true;
                safePrintln("OPCN3 sensor enabled");
            }
            else if (command.equals("CONFIG_OPCN3_OFF")) {
                config.enableOPCN3Sensor = false;
                safePrintln("OPCN3 sensor disabled");
            }
            else if (command.equals("CONFIG_I2C_ON")) {
                config.enableI2CSensors = true;
                initializeI2C();
                safePrintln("I2C sensors enabled");
            }
            else if (command.equals("CONFIG_I2C_OFF")) {
                config.enableI2CSensors = false;
                safePrintln("I2C sensors disabled");
            }
            else if (command.equals("CONFIG_SHT30_ON")) {
                enableI2CSensor(SENSOR_SHT30);
            }
            else if (command.equals("CONFIG_SHT30_OFF")) {
                disableI2CSensor(SENSOR_SHT30);
            }
            else if (command.equals("CONFIG_BME280_ON")) {
                enableI2CSensor(SENSOR_BME280);
            }
            else if (command.equals("CONFIG_BME280_OFF")) {
                disableI2CSensor(SENSOR_BME280);
            }
            else if (command.equals("CONFIG_SCD41_ON")) {
                enableI2CSensor(SENSOR_SCD41);
            }
            else if (command.equals("CONFIG_SCD41_OFF")) {
                disableI2CSensor(SENSOR_SCD41);
            }
            else if (command.equals("RESET_SCD41")) {
                resetSCD41();
            }
            else if (command.equals("CONFIG_SHT40_ON")) {
                enableI2CSensor(SENSOR_SHT40);
            }
            else if (command.equals("CONFIG_SHT40_OFF")) {
                disableI2CSensor(SENSOR_SHT40);
            }
            else if (command.equals("CONFIG_MCP3424_ON")) {
                enableI2CSensor(SENSOR_MCP3424);
            }
            else if (command.equals("CONFIG_MCP3424_OFF")) {
                disableI2CSensor(SENSOR_MCP3424);
            }
            else if (command.equals("CONFIG_ADS1110_ON")) {
                enableI2CSensor(SENSOR_ADS1110);
            }
            else if (command.equals("CONFIG_ADS1110_OFF")) {
                disableI2CSensor(SENSOR_ADS1110);
            }
            else if (command.equals("CONFIG_INA219_ON")) {
                enableI2CSensor(SENSOR_INA219);
            }
            else if (command.equals("CONFIG_INA219_OFF")) {
                disableI2CSensor(SENSOR_INA219);
            }
            else if (command.equals("CONFIG_SPS30_ON")) {
                enableI2CSensor(SENSOR_SPS30);
            }
            else if (command.equals("CONFIG_SPS30_OFF")) {
                disableI2CSensor(SENSOR_SPS30);
            }
            else if (command.equals("CONFIG_ADS1110_")) {
                // Parse ADS1110 configuration: CONFIG_ADS1110_RATE_GAIN (e.g., CONFIG_ADS1110_60_4)
                String params = command.substring(15); // Remove "CONFIG_ADS1110_"
                int separatorIndex = params.indexOf('_');
                if (separatorIndex > 0) {
                    uint8_t dataRate = params.substring(0, separatorIndex).toInt();
                    uint8_t gain = params.substring(separatorIndex + 1).toInt();
                    configureADS1110(dataRate, gain);
                } else {
                    safePrintln("Invalid ADS1110 config format. Use: CONFIG_ADS1110_RATE_GAIN (e.g., CONFIG_ADS1110_60_4)");
                }
            }
            else if (command.equals("CONFIG_IPS_ON")) {
                enableI2CSensor(SENSOR_IPS);
            }
            else if (command.equals("CONFIG_IPS_OFF")) {
                disableI2CSensor(SENSOR_IPS);
            }
            else if (command.equals("CONFIG_MODBUS_ON")) {
                config.enableModbus = true;
                initializeModbus();
                safePrintln("Modbus enabled");
            }
            else if (command.equals("CONFIG_MODBUS_OFF")) {
                config.enableModbus = false;
                safePrintln("Modbus disabled");
            }
            else if (command.equals("CONFIG_AUTO_RESET_ON")) {
                config.autoReset = true;
                safePrintln("Auto reset enabled");
            }
            else if (command.equals("CONFIG_AUTO_RESET_OFF")) {
                config.autoReset = false;
                safePrintln("Auto reset disabled");
            }
            else if (command.equals("CONFIG_CALIB_ON")) {
                extern CalibrationConfig calibConfig;
                calibConfig.enableCalibration = true;
                safePrintln("Calibration enabled");
            }
            else if (command.equals("CONFIG_CALIB_OFF")) {
                extern CalibrationConfig calibConfig;
                calibConfig.enableCalibration = false;
                safePrintln("Calibration disabled");
            }
            else if (command.equals("CONFIG_TGS_ON")) {
                extern CalibrationConfig calibConfig;
                calibConfig.enableTGSSensors = true;
                safePrintln("TGS sensors enabled");
            }
            else if (command.equals("CONFIG_TGS_OFF")) {
                extern CalibrationConfig calibConfig;
                calibConfig.enableTGSSensors = false;
                safePrintln("TGS sensors disabled");
            }
            else if (command.equals("CONFIG_GASES_ON")) {
                extern CalibrationConfig calibConfig;
                calibConfig.enableGasSensors = true;
                safePrintln("Gas sensors enabled");
            }
            else if (command.equals("CONFIG_GASES_OFF")) {
                extern CalibrationConfig calibConfig;
                calibConfig.enableGasSensors = false;
                safePrintln("Gas sensors disabled");
            }
            else if (command.startsWith("FAN_")) {
                // Fan control commands
                if (!config.enableFan) {
                    safePrintln("Fan control is DISABLED in configuration");
                } else {
                    if (command.equals("FAN_ON")) {
                        setFanSpeed(50); // Default 50% speed
                        safePrintln("Fan turned ON at 50% speed");
                    }
                    else if (command.equals("FAN_OFF")) {
                        setFanSpeed(0);
                        safePrintln("Fan turned OFF");
                    }
                    else if (command.startsWith("FAN_SPEED_")) {
                        int speed = command.substring(10).toInt(); // Remove "FAN_SPEED_"
                        if (speed >= 0 && speed <= 100) {
                            setFanSpeed(speed);
                            safePrintln("Fan speed set to " + String(speed) + "%");
                        } else {
                            safePrintln("Invalid fan speed (0-100): " + String(speed));
                        }
                    }
                    else if (command.equals("GLINE_ON")) {
                        setGLine(true);
                        safePrintln("GLine router ENABLED");
                    }
                    else if (command.equals("GLINE_OFF")) {
                        setGLine(false);
                        safePrintln("GLine router DISABLED");
                    }
                }
            }
        }
        else if (command.equals("DATATYPE_CURRENT")) {
            if (config.enableModbus) {
                setCurrentDataType(DATA_CURRENT);
                safePrintln("Switched to current data mode");
            } else {
                safePrintln("Modbus not enabled");
            }
        }
        else if (command.equals("DATATYPE_FAST")) {
            if (config.enableModbus) {
                setCurrentDataType(DATA_FAST_AVG);
                safePrintln("Switched to fast average mode (10s)");
            } else {
                safePrintln("Modbus not enabled");
            }
        }
        else if (command.equals("DATATYPE_SLOW")) {
            if (config.enableModbus) {
                setCurrentDataType(DATA_SLOW_AVG);
                safePrintln("Switched to slow average mode (5min)");
            } else {
                safePrintln("Modbus not enabled");
            }
        }
        else if (command.equals("DATATYPE_CYCLE")) {
            if (config.enableModbus) {
                cycleDataType();
                safePrint("Current data type: ");
                safePrintln(getCurrentDataTypeName());
            } else {
                safePrintln("Modbus not enabled");
            }
        }
        else if (command.equals("DATATYPE_STATUS")) {
            if (config.enableModbus) {
                safePrint("Current data type: ");
                safePrintln(getCurrentDataTypeName());
            } else {
                safePrintln("Modbus not enabled");
            }
        }
        else if (command.equals("MODBUS_STATUS")) {
            if (isSerialAvailable()) {
                safePrintln("=== Modbus Status ===");
                safePrintln("Enabled: " + String(config.enableModbus ? "YES" : "NO"));
                safePrintln("Had Activity: " + String(hasHadModbusActivity ? "YES" : "NO"));
                safePrintln("Last Activity: " + String((millis() - lastModbusActivity) / 1000) + " seconds ago");
                safePrintln("Timeout: " + String(MODBUS_TIMEOUT / 1000) + " seconds");
                safePrintln("System Uptime: " + String(millis() / 1000) + " seconds");
            }
        }
        else if (command.equals("CALIB_DATA")) {
            if (isSerialAvailable()) {
                extern CalibratedSensorData calibratedData;
                extern CalibrationConfig calibConfig;
                
                safePrintln("=== Calibration Data ===");
                safePrintln("Enabled: " + String(calibConfig.enableCalibration ? "YES" : "NO"));
                safePrintln("Valid: " + String(calibratedData.valid ? "YES" : "NO"));
                
                if (calibConfig.enableCalibration && calibratedData.valid) {
                    safePrintln("--- Temperatures ---");
                    safePrintln("K1: " + String(calibratedData.K1_temp, 1) + "°C");
                    safePrintln("K2: " + String(calibratedData.K2_temp, 1) + "°C");
                    safePrintln("K3: " + String(calibratedData.K3_temp, 1) + "°C");
                    safePrintln("K4: " + String(calibratedData.K4_temp, 1) + "°C");
                    safePrintln("K5: " + String(calibratedData.K5_temp, 1) + "°C");
                    
                    safePrintln("--- Gases (ppb) ---");
                    safePrintln("CO: " + String(calibratedData.CO_ppb, 1) + " ppb");
                    safePrintln("NO: " + String(calibratedData.NO_ppb, 1) + " ppb");
                    safePrintln("NO2: " + String(calibratedData.NO2_ppb, 1) + " ppb");
                    safePrintln("O3: " + String(calibratedData.O3_ppb, 1) + " ppb");
                    safePrintln("SO2: " + String(calibratedData.SO2_ppb, 1) + " ppb");
                    safePrintln("H2S: " + String(calibratedData.H2S_ppb, 1) + " ppb");
                    safePrintln("NH3: " + String(calibratedData.NH3_ppb, 1) + " ppb");
                    
                    safePrintln("--- TGS Sensors ---");
                    safePrintln("TGS02: " + String(calibratedData.TGS02, 3) + " ppm");
                    safePrintln("TGS03: " + String(calibratedData.TGS03, 3) + " ppm");
                    safePrintln("TGS12: " + String(calibratedData.TGS12, 3) + " ppm");
                    
                    safePrintln("--- Special Sensors ---");
                    safePrintln("HCHO: " + String(calibratedData.HCHO, 1) + " ppb");
                    safePrintln("PID: " + String(calibratedData.PID, 3) + " ppm");
                } else {
                    safePrintln("No valid calibration data available");
                }
            }
        }
        else if (command.equals("TIME_INFO")) {
            if (isSerialAvailable()) {
                extern bool isTimeSet();
                extern String getFormattedTime();
                extern String getFormattedDate();
                extern time_t getEpochTime();
                
                safePrintln("=== Time Information ===");
                safePrintln("Time Synchronized: " + String(isTimeSet() ? "YES" : "NO"));
                if (isTimeSet()) {
                    safePrintln("Current Time: " + getFormattedTime());
                    safePrintln("Current Date: " + getFormattedDate());
                    safePrintln("Epoch Time: " + String(getEpochTime()));
                    
                    struct tm timeinfo;
                    if (getLocalTime(&timeinfo)) {
                        safePrintln("Day of Week: " + String(timeinfo.tm_wday) + " (0=Sunday)");
                        safePrintln("Hours: " + String(timeinfo.tm_hour));
                        safePrintln("Minutes: " + String(timeinfo.tm_min));
                        safePrintln("Seconds: " + String(timeinfo.tm_sec));
                        safePrintln("Day: " + String(timeinfo.tm_mday));
                        safePrintln("Month: " + String(timeinfo.tm_mon + 1));
                        safePrintln("Year: " + String(timeinfo.tm_year + 1900));
                        safePrintln("Timezone: Poland (UTC+1/UTC+2 with automatic DST)");
                    }
                } else {
                    safePrintln("Time not synchronized - check WiFi connection");
                }
                safePrintln("System Uptime: " + String(millis() / 1000) + " seconds");
            }
        }
        else if (command.equals("HISTORY")) {
            if (isSerialAvailable()) {
                safePrintln("=== Sensor History Status ===");
                printHistoryMemoryUsage();
                printHistoryStatus();
            }
        }
        else if (command.equals("RESTART")) {
            safePrintln("Restarting system...");
            delay(1000);
            ESP.restart();
        }
    }
    
    // Process Serial1 commands if needed
    if (Serial1.available() > 0) {
        String command = Serial1.readStringUntil('\n');
        command.trim();
        
        if (command.equals("SEND")) {
            sendDataFlag = true;
        }
    }
    
    // Send data if flag is set - only if Serial is connected
    if (sendDataFlag && isSerialAvailable()) {
        safePrintln("=== Sensor Data ===");
        
        if (config.enableSolarSensor && solarSensorStatus) {
            printSolarData();
        }
        
        if (config.enableOPCN3Sensor && opcn3SensorStatus) {
            safePrint("OPCN3: ");
            safePrint("Temp="); safePrint(String(opcn3Data.getTempC()));
            safePrint("°C, Hum="); safePrint(String(opcn3Data.getHumidity()));
            safePrint(", PM1="); safePrint(String(opcn3Data.pm1));
            safePrint(", PM2.5="); safePrint(String(opcn3Data.pm2_5));
            safePrint(", PM10="); safePrintln(String(opcn3Data.pm10));
        }
        
        if (config.enableI2CSensors && i2cSensorStatus) {
            safePrint("I2C Sensor: ");
            safePrint("Temp="); safePrint(String(i2cSensorData.temperature));
            safePrint("°C, Hum="); safePrint(String(i2cSensorData.humidity));
            safePrintln("%");
        }
        
        if (config.enableSCD41 && scd41SensorStatus && i2cSensorData.valid) {
            safePrint("SCD41: ");
            safePrint("CO2="); safePrint(String(i2cSensorData.co2, 0));
            safePrint(" ppm, Temp="); safePrint(String(i2cSensorData.temperature, 2));
            safePrint("°C, Hum="); safePrint(String(i2cSensorData.humidity, 2));
            safePrintln("%");
        }
        
        if (config.enableMCP3424 && mcp3424SensorStatus) {
            safePrint("MCP3424 Devices: ");
            safePrintln(String(mcp3424Data.deviceCount));
            
            for (uint8_t device = 0; device < mcp3424Data.deviceCount; device++) {
                safePrint("  Device ");
                safePrint(String(device));
                safePrint(" (0x");
                safePrint(String(mcp3424Data.addresses[device], HEX));
                safePrint("): ");
                
                if (mcp3424Data.valid[device]) {
                    for (int ch = 0; ch < 4; ch++) {
                        safePrint("Ch"); safePrint(String(ch)); safePrint("=");
                        safePrint(String(mcp3424Data.channels[device][ch], 4)); safePrint("V");
                        if (ch < 3) safePrint(", ");
                    }
                    safePrintln("");
                } else {
                    safePrintln("Invalid");
                }
            }
        }
        
        if (config.enableADS1110 && ads1110SensorStatus) {
            safePrint("ADS1110: Voltage=");
            safePrint(String(ads1110Data.voltage, 6));
            safePrintln("V");
        }
        
        if (config.enableINA219 && ina219SensorStatus) {
            safePrint("INA219: Bus=");
            safePrint(String(ina219Data.busVoltage, 3));
            safePrint("V, Current=");
            safePrint(String(ina219Data.current, 2));
            safePrint("mA, Power=");
            safePrint(String(ina219Data.power, 2));
            safePrintln("mW");
        }
        
        if (config.enableIPS && ipsSensorStatus) {
            safePrint("IPS Sensor PC: ");
            for (int i = 0; i < 7; i++) {
                safePrint(String(ipsSensorData.pc_values[i]));
                if (i < 6) safePrint(",");
            }
            safePrint(" PM: ");
            for (int i = 0; i < 7; i++) {
                safePrint(String(ipsSensorData.pm_values[i]));
                if (i < 6) safePrint(",");
            }
            safePrintln("");
        }
        
        sendDataFlag = false;
    } else if (sendDataFlag) {
        // Clear flag even if Serial not available
        sendDataFlag = false;
    }
}

void updateSystemStatus() {
    static unsigned long lastStatusUpdate = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastStatusUpdate >= 10000) { // Update every 5 seconds
        lastStatusUpdate = currentTime;
        
        // Count active sensors
      //  int activeSensors = 0;
        // Sprawdz status czujnikow tylko jesli sa wlaczone w konfiguracji
        // if (config.enableSHT30 && sht30SensorStatus) activeSensors++;
        // if (config.enableBME280 && bme280SensorStatus) activeSensors++;
        // if (config.enableSCD41 && scd41SensorStatus) activeSensors++;
        // if (config.enableMCP3424 && mcp3424SensorStatus) activeSensors++;
        // if (config.enableADS1110 && ads1110SensorStatus) activeSensors++;
        // if (config.enableINA219 && ina219SensorStatus) activeSensors++;
        // if (config.enableSolarSensor && solarSensorStatus) activeSensors++;
        // if (config.enableIPS && ipsSensorStatus) activeSensors++;
        // if (config.enableOPCN3Sensor && opcn3SensorStatus) activeSensors++;
        
        // Sprawdz czy wszystkie wlaczone czujniki dzialaja
        int enabledSensors = 0;
        int workingSensors = 0;
        
        if (config.enableSHT30) {
            enabledSensors++;
            if (sht30SensorStatus) workingSensors++;
        }
        if (config.enableBME280) {
            enabledSensors++;
            if (bme280SensorStatus) workingSensors++;
        }
        if (config.enableSCD41) {
            enabledSensors++;
            if (scd41SensorStatus) workingSensors++;
        }
        if (config.enableMCP3424) {
            enabledSensors++;
            if (mcp3424SensorStatus) workingSensors++;
        }
        if (config.enableADS1110) {
            enabledSensors++;
            if (ads1110SensorStatus) workingSensors++;
        }
        if (config.enableINA219) {
            enabledSensors++;
            if (ina219SensorStatus) workingSensors++;
        }
        if (config.enableSolarSensor) {
            enabledSensors++;
            if (solarSensorStatus) workingSensors++;
        }
        if (config.enableIPS) {
            enabledSensors++;
            if (ipsSensorStatus) workingSensors++;
        }
        if (config.enableOPCN3Sensor) {
            enabledSensors++;
            if (opcn3SensorStatus) workingSensors++;
        }
        
        // Ustaw kolor LED na podstawie statusu wszystkich wlaczonych czujnikow
        if (enabledSensors > 0) {
            if (workingSensors == enabledSensors) {
                pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Zielony - wszystkie wlaczone dzialaja
            } else if (workingSensors >= 2) {
                pixels.setPixelColor(0, pixels.Color(0, 255, 255)); // Cyjan - wiekszosc dziala
            } else if (workingSensors == 1) {
                pixels.setPixelColor(0, pixels.Color(255, 255, 0)); // Zolty - tylko 1 dziala
            } else {
                pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Czerwony - zadne nie dzialaja
            }
        } else {
            pixels.setPixelColor(0, pixels.Color(128, 128, 128)); // Szary - brak wlaczonych czujnikow
        }
        pixels.show();
    }
}
void watchDogTask(void *parameter) {
    // Initialize task watchdog timer (18 seconds timeout)
    ESP_ERROR_CHECK(esp_task_wdt_init(18, true));
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    
    for (;;) {
        ESP_ERROR_CHECK(esp_task_wdt_reset());
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }
}

