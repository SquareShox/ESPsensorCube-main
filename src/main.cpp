#include <Arduino.h>
#include <esp_task_wdt.h>
#include <Adafruit_NeoPixel.h>
#include <led.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
// ESP-IDF heap debugging includes (Arduino compatible)
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Project includes
#include <config.h>
#include <sensors.h>
#include <i2c_sensors.h>
#include <ips_sensor.h>
#include <modbus_handler.h>
#include <web_server.h>
#include <web_socket.h>
#include <mean.h>
#include <calib.h>
#include <calib_constants.h>
#include <network_config.h>
#include <history.h>
#include <fan.h>

// #include <html.h>

// Global configuration
FeatureConfig config;
SerialSensorConfig serialSensorConfigs[MAX_SERIAL_SENSORS];

// Global battery data
BatteryData batteryData;

// Simplified memory monitoring
static unsigned long last_heap_report = 0;

// Hardware objects
HardwareSerial MySerial(2); // Solar sensor serial
Adafruit_NeoPixel pixels(1, WS2812_PIN, NEO_GRB + NEO_KHZ800);
extern Adafruit_NeoPixel& getExtPixels();

// Global variables
bool sendDataFlag = false;
unsigned long lastSensorCheck = 0;
unsigned long lastMovingAverageUpdate = 0;

// Network flag for display
bool turnOnNetwork = false;

// Heap debugging function declarations (Arduino compatible)
void initializeHeapDebugging();
void printHeapInfo();
void printDetailedHeapInfo();
void takeHeapSnapshot();
void printHeapHistory();
void analyzeMemoryLeaks();
void performHeapAnalysis();
void startHeapMonitoring();
void stopHeapMonitoring();
void printTaskMemoryStats();
void printAllTasksMemoryStats();
void printSingleTaskMemoryStats(const char *taskName);

// Function declarations
void setup();
void loop();
void watchDogTask(void *parameter);
void timeSyncTask(void *parameter);
void initializeHardware();
void processSerialCommands();
void updateSystemStatus();

// Safe Serial printing functions
void safePrint(const String &message)
{
    if (Serial && Serial.availableForWrite())
    {
        Serial.print(message);
    }
}

void safePrintln(const String &message)
{
    if (Serial && Serial.availableForWrite())
    {
        Serial.println(message);
    }
}

bool isSerialAvailable()
{
    return Serial && Serial.availableForWrite();
}

void setup()
{
    // Initialize serial communication with timeout - non-blocking
    Serial.begin(SERIAL_BAUD);
    unsigned long startMillis = millis();
    while (!Serial && millis() - startMillis < 2000)
    {
        ; // Wait for serial port to connect with short timeout
    }
    Serial.setTimeout(1000);
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    // Initialize LittleFS and load system configuration
    if (initLittleFS())
    {
        // Check and repair LittleFS if needed
        if (checkAndRepairLittleFS())
        {
            if (loadSystemConfig(config))
            {
                Serial.println("System configuration loaded from file");
            }
            else
            {
                Serial.println("Using default system configuration");
            }
        }
        else
        {
            Serial.println("LittleFS check failed, attempting cleanup...");
            if (cleanLittleFS() && checkAndRepairLittleFS())
            {
                Serial.println("LittleFS cleaned and repaired successfully");
                if (loadSystemConfig(config))
                {
                    Serial.println("System configuration loaded after cleanup");
                }
                else
                {
                    Serial.println("Using default system configuration after cleanup");
                }
            }
            else
            {
                Serial.println("LittleFS cleanup failed, using defaults");
            }
        }
    }
    else
    {
        Serial.println("Failed to initialize LittleFS, using defaults");
    }

    // Don't wait for Serial - system should work without it
    if (Serial)
    {
        Serial.println("=== ESP32 Multi-Sensor System Starting ===");
        //  pinMode(IPS_POWER_PIN, OUTPUT);
        //  digitalWrite(IPS_POWER_PIN, LOW);  // Set to GND
        delay(1000); // Wait for power stabilization

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
        if (config.enableSolarSensor && config.enableIPS)
        {
            Serial.println("WARNING: Solar and IPS sensors share the same UART - only one can work!");
        }
    }
    else
    {
        // System starting without Serial - just configure IPS power
        //  pinMode(IPS_POWER_PIN, OUTPUT);
        //  digitalWrite(IPS_POWER_PIN, LOW);  // Set to GND
        delay(1000); // Wait for power stabilization
    }

    if (config.enableWiFi)
    {
        initializeWiFi();
    }
    if (config.enableWebServer && config.enableWiFi)
    {
        initializeWebServer();
    }

    // Initialize hardware
    initializeHardware();
    // Initialize external LED strip
    initExtLeds();
    // Optional: uncomment to force debug white
    // ledSetDebugWhite(true);

    // Initialize sensors
    initializeSensors();

    // Initialize MCP3424 mapping
    safePrintln("Initializing MCP3424 mapping...");
    if (!loadMCP3424Config(mcp3424Config))
    {
        safePrintln("No MCP3424 config found, creating default...");
        initializeDefaultMCP3424Mapping();
        saveMCP3424Config(mcp3424Config);
        safePrintln("Default MCP3424 config saved");
    }
    else
    {
        safePrintln("MCP3424 config loaded: " + String(mcp3424Config.deviceCount) + " devices");
    }

    // Initialize calibration constants
    safePrintln("Loading calibration constants...");
    if (loadCalibrationConstants(calibConstants))
    {
        safePrintln("Calibration constants loaded from file");
    }
    else
    {
        safePrintln("Using default calibration constants");
    }

    // Initialize moving averages system
    initializeMovingAverages();

    // Initialize sensor history in PSRAM
    initializeHistory();

    // Initialize communication protocols
    if (config.enableModbus)
    {
        lastModbusActivity = millis();
        initializeModbus();
    }

    // Create system tasks
    xTaskCreate(
        watchDogTask, // Task function
        "Watchdog",   // Task name
        10000,        // Stack size
        NULL,         // Parameters
        5,            // Priority
        NULL          // Task handle
    );

    if (config.enableWiFi)
    {
        xTaskCreate(
            WiFiReconnectTask, // Task function
            "WiFi Reconnect",  // Task name
            10000,             // Stack size
            NULL,              // Parameters
            1,                 // Priority
            NULL               // Task handle
        );
    }

    if (Serial)
    {
        Serial.println("=== System initialization complete ===");
    }
    delay(1000);
    // if (config.enablePushbullet && strlen(config.pushbulletToken) > 0) {
    //     // Wait a bit for WiFi to stabilize
    //    // delay(5000);
    //     sendSystemStartupNotification();
    // }
}

void loop()
{
    unsigned long currentTime = millis();

    esp_task_wdt_reset();

    // Read sensors
    if (config.enableSolarSensor)
    {
        readSolarSensor();
    }

    if (config.enableOPCN3Sensor)
    {
        readOPCN3Sensor();
    }

    if (config.enableI2CSensors)
    {
        readI2CSensors();

        // Wykonaj kalibrację po odczycie danych z MCP3424
        if (config.enableMCP3424 && mcp3424SensorStatus)
        {
            performCalibration();
        }
        ledSetAirQualityColorFromPM25(calibratedData.PM25);
        // Update LED color from calibrated PM2.5 if available
       
    }

    if (config.enableIPS)
    {
        readIPSSensor();
    }

    if (config.enableHCHO)
    {
        readHCHO();
    }

    readBatteryVoltage();
    readSerialSensors();

    // Update battery status every second (not every loop iteration)
    static unsigned long lastBatteryUpdate = 0;
    if (currentTime - lastBatteryUpdate >= 1000) {
        lastBatteryUpdate = currentTime;
        updateBatteryStatus();
        checkBatteryShutdown();
    }

    // Update fan RPM measurement if enabled
    if (config.enableFan)
    {
        updateFanRPM();
    }

    // Update Modbus registers
    if (config.enableModbus)
    {
        processModbusTask();

        if (solarSensorStatus && currentTime - lastSensorCheck >= 1000)
        {
            updateModbusSolarRegisters();
        }

        if (opcn3SensorStatus)
        {
            updateModbusOPCN3Registers();
        }

        if (i2cSensorStatus)
        {
            updateModbusI2CRegisters();
        }

        if (mcp3424SensorStatus)
        {
            updateModbusMCP3424Registers();
        }

        if (ads1110SensorStatus)
        {
            updateModbusADS1110Registers();
        }

        if (ina219SensorStatus)
        {
            updateModbusINA219Registers();
        }

        if (sht40SensorStatus)
        {
            updateModbusSHT40Registers();
        }

        if (ipsSensorStatus)
        {
            updateModbusIPSRegisters();
        }

        if (hchoSensorStatus)
        {
            updateModbusHCHORegisters();
        }

        // Debug SPS30 status
        static unsigned long lastSPS30Debug = 0;
        if (currentTime - lastSPS30Debug > 15000)
        { // Every 15 seconds
            lastSPS30Debug = currentTime;
        }

        if (sps30SensorStatus)
        {
            updateModbusSPS30Registers();
        }

        // Update calibrated data registers if calibration is enabled
        if (calibConfig.enableCalibration && calibratedData.valid)
        {
            updateModbusCalibrationRegisters();
        }

        lastSensorCheck = currentTime;
    }

    // Process serial commands
    processSerialCommands();

    // Update system status indicators
    updateSystemStatus();

    // Update moving averages every 5 seconds
    if (currentTime - lastMovingAverageUpdate >= 5000)
    {
        updateMovingAverages();
        lastMovingAverageUpdate = currentTime;
    }

    // Update sensor history
    updateSensorHistory();

    // Periodic heap monitoring (every 2 minutes)
    // if (currentTime - last_heap_report >= 120000) {
    //     last_heap_report = currentTime;

    //     // Take snapshot for history
    //     takeHeapSnapshot();

    //     safePrintln("\n=== PERIODIC HEAP MONITOR ===");
    //     printHeapInfo();

    //     // Auto-analyze if memory usage is concerning
    //     uint32_t current_free = ESP.getFreeHeap();
    //     if (heap_baseline > 0) {
    //         int32_t memory_change = (int32_t)current_free - (int32_t)heap_baseline;
    //         if (memory_change < -10000) { // More than 10KB lost
    //             safePrintln("ALERT: Significant memory loss detected! Starting detailed analysis...");
    //             analyzeMemoryLeaks();
    //         }
    //     }
    // }

    // Auto reset check
    if (config.autoReset)
    {
        static unsigned long lastValidData = millis();
        static unsigned long lastStatusPrint = 0;

        // Check if any enabled sensors have valid data
        bool hasValidData = false;
        if (config.enableSolarSensor && solarSensorStatus)
            hasValidData = true;
        if (config.enableOPCN3Sensor && opcn3SensorStatus)
            hasValidData = true;
        if (config.enableI2CSensors && i2cSensorStatus)
            hasValidData = true;
        if (config.enableSHT40 && sht40SensorStatus)
            hasValidData = true;
        if (config.enableIPS && ipsSensorStatus)
            hasValidData = true;
        if (config.enableMCP3424 && mcp3424SensorStatus)
            hasValidData = true;

        if (hasValidData)
        {
            lastValidData = currentTime;
        }

        // Check for sensor timeout - only if any sensors are enabled
        bool anySensorEnabled = config.enableSolarSensor || config.enableOPCN3Sensor ||
                                config.enableI2CSensors || config.enableSHT40 || config.enableIPS;

        if (anySensorEnabled && (currentTime - lastValidData > SENSOR_TIMEOUT))
        {
            safePrintln("No data from enabled sensors for 2 minutes. Restarting...");
            delay(1000);
            ESP.restart();
        }

        // Check for Modbus timeout - only if there was previous activity and system running >2 minutes
        if (config.enableModbus && hasHadModbusActivity && (currentTime > 120000) && (currentTime + 1000 - lastModbusActivity > MODBUS_TIMEOUT))
        {
            safePrint("No Modbus activity for 5 minutes. Last activity: ");
            safePrint(String((currentTime - lastModbusActivity) / 1000));
            safePrint("s ago. Restarting...");
            safePrintln("");
            delay(1000);
            ESP.restart();
        }

        // Print status every 60 seconds for debugging
        if (currentTime - lastStatusPrint >= 60000)
        {
            lastStatusPrint = currentTime;
            if (isSerialAvailable())
            {
                safePrint("Auto-reset status - Last valid data: ");
                safePrint(String((currentTime - lastValidData) / 1000));
                safePrint("s ago");
                if (config.enableModbus)
                {
                    safePrint(", Modbus: ");
                    if (hasHadModbusActivity)
                    {
                        safePrint(String((currentTime - lastModbusActivity) / 1000));
                        safePrint("s ago");
                    }
                    else
                    {
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
    updateExtBreathing();
    delay(10);
}

void initializeHardware()
{
    // Initialize LED strip only if not in low power mode
    if (!config.lowPowerMode)
    {
        pixels.begin();
        pixels.setBrightness(60);
        pixels.setPixelColor(0, pixels.Color(255, 255, 255)); // White during initialization

        pixels.show();
    }
    else
    {
        safePrintln("Low power mode: LED disabled");
    }

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
    if (config.enableFan)
    {
        initializeFan();
    }

    // Initialize battery monitoring
    initializeBatteryMonitoring();

    // Send startup notification (after WiFi is connected)

    safePrintln("Hardware initialization complete");

    // Start time sync task
    xTaskCreate(
        timeSyncTask,        // Task function
        "TimeSync",          // Task name
        8096,                // Stack size
        NULL,                // Task parameters
        3,                   // Priority
        NULL                 // Task handle
    );
}

void processSerialCommands()
{
    // Process commands from main serial - only if connected
    if (Serial && Serial.available() > 0)
    {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command.equals("SEND"))
        {
            sendDataFlag = true;
        }
        else if (command.equals("STATUS"))
        {
            if (isSerialAvailable())
            {
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
                if (config.enableFan)
                {
                    safePrintln("Fan Control: " + String(isFanEnabled() ? "ON" : "OFF") + " (" + String(getFanDutyCycle()) + "%, " + String(getFanRPM()) + " RPM)");
                    safePrintln("GLine Router: " + String(isGLineEnabled() ? "ENABLED" : "DISABLED"));
                    if (isSleepModeActive())
                    {
                        FanStatus status = getFanStatus();
                        safePrintln("Sleep Mode: ACTIVE (End: " + String(status.sleepEndTime) + ")");
                    }
                }
                else
                {
                    safePrintln("Fan Control: DISABLED");
                }

                safePrintln("WiFi: " + String(WiFi.status() == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED"));
                // ip address
                safePrintln("IP Address: " + String(WiFi.localIP().toString()));
                safePrintln("Auto Reset: " + String(config.autoReset ? "ENABLED" : "DISABLED"));
                safePrintln("Low Power Mode: " + String(config.lowPowerMode ? "ENABLED" : "DISABLED"));
                safePrintln("Pushbullet: " + String(config.enablePushbullet ? "ENABLED" : "DISABLED"));
                if (config.enablePushbullet)
                {
                    safePrintln("Pushbullet Token: " + String(strlen(config.pushbulletToken) > 0 ? "SET" : "NOT SET"));
                }
                if (config.enableModbus)
                {
                    safePrint("Modbus Activity: ");
                    if (hasHadModbusActivity)
                    {
                        safePrintln("Last " + String((millis() - lastModbusActivity) / 1000) + " seconds ago");
                    }
                    else
                    {
                        safePrintln("Never detected");
                    }
                }
                safePrintln("Free Heap: " + String(ESP.getFreeHeap()));
                safePrintln("PSRAM Size: " + String(ESP.getPsramSize() / 1024) + " KB");
                safePrintln("Free PSRAM: " + String(ESP.getFreePsram() / 1024) + " KB");
                safePrintln("Uptime: " + String(millis() / 1000) + " seconds");

                // Battery status
                if (batteryData.valid)
                {
                    safePrintln("=== Battery Status ===");
                    safePrintln("Voltage: " + String(batteryData.voltage, 3) + "V");
                    safePrintln("Current: " + String(batteryData.current, 2) + "mA");
                    safePrintln("Power: " + String(batteryData.power, 1) + "mW");
                    safePrintln("Charge: " + String(batteryData.chargePercent) + "%");
                    safePrintln("Source: " + String(batteryData.isBatteryPowered ? "External" : "Battery"));
                    safePrintln("Low Battery: " + String(batteryData.lowBattery ? "YES" : "NO"));
                    safePrintln("Critical: " + String(batteryData.criticalBattery ? "YES" : "NO"));
                }
                else
                {
                    safePrintln("Battery Status: No valid data");
                }

                // Time info from ESP32 built-in time functions

                if (isTimeSet())
                {
                    safePrintln("Time: " + getFormattedTime());
                    safePrintln("Date: " + getFormattedDate());
                    safePrintln("Epoch: " + String(getEpochTime()));

                    struct tm timeinfo;
                    if (getLocalTime(&timeinfo))
                    {
                        safePrintln("Day of Week: " + String(timeinfo.tm_wday));
                        safePrintln("Timezone: Poland (UTC+1/UTC+2 with DST)");
                    }
                }
                else
                {
                    safePrintln("Time: Not synchronized");
                }

                // Calibration status
                // safePrintln("Calibration: " + String(calibConfig.enableCalibration ? "ENABLED" : "DISABLED"));
                // if (calibConfig.enableCalibration) {
                //     safePrintln("Calibration Valid: " + String(calibratedData.valid ? "YES" : "NO"));
                //     safePrintln("Last Calibration: " + String((millis() - calibratedData.lastUpdate) / 1000) + " seconds ago");
                // }

                // safePrintln("=== Moving Averages Status ===");
                // printMovingAverageStatus();

                safePrintln("=== Fan Control Commands ===");
                safePrintln("FAN_ON - Turn fan ON at 50% speed");
                safePrintln("FAN_OFF - Turn fan OFF");
                safePrintln("FAN_SPEED_[0-100] - Set fan speed (e.g., FAN_SPEED_75)");
                safePrintln("GLINE_ON - Enable GLine router");
                safePrintln("GLINE_OFF - Disable GLine router");
                safePrintln("SLEEP_[delay]_[duration] - Schedule sleep mode (e.g., SLEEP_60_300)");
                safePrintln("SLEEP_STOP - Stop sleep mode immediately");
                safePrintln("FAN_STATUS - Show detailed fan status");
                
                safePrintln("=== MCP3424 Mapping Commands ===");
                safePrintln("MCP3424_MAPPING - Show detailed device mapping info");
                safePrintln("MCP3424_SCAN - Rescan I2C bus and update device detection");
                safePrintln("MCP3424_RESET - Reset mapping to defaults and rescan");
                safePrintln("MCP3424_GET_[GAS] - Get device info for gas type (e.g., MCP3424_GET_SO2)");
                safePrintln("MCP3424_ADDR_[HEX] - Get device info for I2C address (e.g., MCP3424_ADDR_6A)");
                safePrintln("Available gas types: NO, O3, NO2, CO, SO2, TGS1, TGS2, TGS3");
                
                safePrintln("=== Memory Management Commands ===");
                safePrintln("MEMORY_EMERGENCY - Full aggressive memory cleanup & analysis");
                safePrintln("MEMORY_FORCE_GC - Force defragmentation garbage collection");
                safePrintln("MEMORY_SMART - Intelligent adaptive memory cleanup");
                safePrintln("Current memory status:");
                safePrintln("- Free heap: " + String(ESP.getFreeHeap()) + " bytes");
                safePrintln("- Min free ever: " + String(ESP.getMinFreeHeap()) + " bytes");
                safePrintln("- Largest block: " + String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)) + " bytes");

                extern int wsClientCount;
                extern AsyncWebSocket ws;
                safePrintln("WebSocket status: " + String(wsClientCount) + " tracked, " + String(ws.count()) + " active");
                if (wsClientCount != ws.count())
                {
                    safePrintln("WARNING: WebSocket client count mismatch - possible memory leak!");
                }
            }
        }
        else if (command.equals("AVGSTATUS"))
        {
            if (isSerialAvailable())
            {
                safePrintln("=== Moving Averages Buffer Status ===");
                printMovingAverageStatus();
            }
        }
        else if (command.startsWith("CONFIG_"))
        {
            // Configuration commands
            if (command.equals("CONFIG_SOLAR_ON"))
            {
                config.enableSolarSensor = true;
                safePrintln("Solar sensor enabled");
            }
            else if (command.equals("CONFIG_SOLAR_OFF"))
            {
                config.enableSolarSensor = false;
                safePrintln("Solar sensor disabled");
            }
            else if (command.equals("CONFIG_OPCN3_ON"))
            {
                config.enableOPCN3Sensor = true;
                safePrintln("OPCN3 sensor enabled");
            }
            else if (command.equals("CONFIG_OPCN3_OFF"))
            {
                config.enableOPCN3Sensor = false;
                safePrintln("OPCN3 sensor disabled");
            }
            else if (command.equals("CONFIG_I2C_ON"))
            {
                config.enableI2CSensors = true;
                initializeI2C();
                safePrintln("I2C sensors enabled");
            }
            else if (command.equals("CONFIG_I2C_OFF"))
            {
                config.enableI2CSensors = false;
                safePrintln("I2C sensors disabled");
            }
            else if (command.equals("CONFIG_SHT30_ON"))
            {
                enableI2CSensor(SENSOR_SHT30);
            }
            else if (command.equals("CONFIG_SHT30_OFF"))
            {
                disableI2CSensor(SENSOR_SHT30);
            }
            else if (command.equals("CONFIG_BME280_ON"))
            {
                enableI2CSensor(SENSOR_BME280);
            }
            else if (command.equals("CONFIG_BME280_OFF"))
            {
                disableI2CSensor(SENSOR_BME280);
            }
            else if (command.equals("CONFIG_SCD41_ON"))
            {
                enableI2CSensor(SENSOR_SCD41);
            }
            else if (command.equals("CONFIG_SCD41_OFF"))
            {
                disableI2CSensor(SENSOR_SCD41);
            }
            else if (command.equals("RESET_SCD41"))
            {
                resetSCD41();
            }
            else if (command.equals("CONFIG_SHT40_ON"))
            {
                enableI2CSensor(SENSOR_SHT40);
            }
            else if (command.equals("CONFIG_SHT40_OFF"))
            {
                disableI2CSensor(SENSOR_SHT40);
            }
            else if (command.equals("CONFIG_MCP3424_ON"))
            {
                enableI2CSensor(SENSOR_MCP3424);
            }
            else if (command.equals("CONFIG_MCP3424_OFF"))
            {
                disableI2CSensor(SENSOR_MCP3424);
            }
            else if (command.equals("CONFIG_ADS1110_ON"))
            {
                enableI2CSensor(SENSOR_ADS1110);
            }
            else if (command.equals("CONFIG_ADS1110_OFF"))
            {
                disableI2CSensor(SENSOR_ADS1110);
            }
            else if (command.equals("CONFIG_INA219_ON"))
            {
                enableI2CSensor(SENSOR_INA219);
            }
            else if (command.equals("CONFIG_INA219_OFF"))
            {
                disableI2CSensor(SENSOR_INA219);
            }
            else if (command.equals("CONFIG_SPS30_ON"))
            {
                enableI2CSensor(SENSOR_SPS30);
            }
            else if (command.equals("CONFIG_SPS30_OFF"))
            {
                disableI2CSensor(SENSOR_SPS30);
            }
            else if (command.equals("CONFIG_ADS1110_"))
            {
                // Parse ADS1110 configuration: CONFIG_ADS1110_RATE_GAIN (e.g., CONFIG_ADS1110_60_4)
                String params = command.substring(15); // Remove "CONFIG_ADS1110_"
                int separatorIndex = params.indexOf('_');
                if (separatorIndex > 0)
                {
                    uint8_t dataRate = params.substring(0, separatorIndex).toInt();
                    uint8_t gain = params.substring(separatorIndex + 1).toInt();
                    configureADS1110(dataRate, gain);
                }
                else
                {
                    safePrintln("Invalid ADS1110 config format. Use: CONFIG_ADS1110_RATE_GAIN (e.g., CONFIG_ADS1110_60_4)");
                }
            }
            else if (command.equals("CONFIG_IPS_ON"))
            {
                enableI2CSensor(SENSOR_IPS);
            }
            else if (command.equals("CONFIG_IPS_OFF"))
            {
                disableI2CSensor(SENSOR_IPS);
            }
            else if (command.equals("CONFIG_MODBUS_ON"))
            {
                config.enableModbus = true;
                initializeModbus();
                safePrintln("Modbus enabled");
            }
            else if (command.equals("CONFIG_MODBUS_OFF"))
            {
                config.enableModbus = false;
                safePrintln("Modbus disabled");
            }
            else if (command.equals("CONFIG_AUTO_RESET_ON"))
            {
                config.autoReset = true;
                safePrintln("Auto reset enabled");
            }
            else if (command.equals("CONFIG_AUTO_RESET_OFF"))
            {
                config.autoReset = false;
                safePrintln("Auto reset disabled");
            }
            else if (command.equals("CONFIG_LOW_POWER_ON"))
            {
                config.lowPowerMode = true;
                safePrintln("Low power mode enabled - LED disabled");
            }
            else if (command.equals("CONFIG_LOW_POWER_OFF"))
            {
                config.lowPowerMode = false;
                safePrintln("Low power mode disabled - LED enabled");
            }
            else if (command.equals("PUSHBULLET_TEST"))
            {
                if (config.enablePushbullet && strlen(config.pushbulletToken) > 0)
                {
                    sendPushbulletNotification("🧪 Test Notification", "This is a test notification from ESP32 Sensor Cube\nTime: " + getFormattedTime() + "\nUptime: " + getUptimeString());
                    safePrintln("Test notification sent");
                }
                else
                {
                    safePrintln("Pushbullet not configured - enable and set token first");
                }
            }
            else if (command.equals("PUSHBULLET_BATTERY_TEST"))
            {
                if (config.enablePushbullet && strlen(config.pushbulletToken) > 0)
                {
                    sendBatteryCriticalNotification();
                    safePrintln("Battery critical test notification sent");
                }
                else
                {
                    safePrintln("Pushbullet not configured - enable and set token first");
                }
            }
            else if (command.equals("CONFIG_CALIB_ON"))
            {
                extern CalibrationConfig calibConfig;
                calibConfig.enableCalibration = true;
                safePrintln("Calibration enabled");
            }
            else if (command.equals("CONFIG_CALIB_OFF"))
            {
                extern CalibrationConfig calibConfig;
                calibConfig.enableCalibration = false;
                safePrintln("Calibration disabled");
            }
            else if (command.equals("CONFIG_TGS_ON"))
            {
                extern CalibrationConfig calibConfig;
                calibConfig.enableTGSSensors = true;
                safePrintln("TGS sensors enabled");
            }
            else if (command.equals("CONFIG_TGS_OFF"))
            {
                extern CalibrationConfig calibConfig;
                calibConfig.enableTGSSensors = false;
                safePrintln("TGS sensors disabled");
            }
            else if (command.equals("CONFIG_GASES_ON"))
            {
                extern CalibrationConfig calibConfig;
                calibConfig.enableGasSensors = true;
                safePrintln("Gas sensors enabled");
            }
            else if (command.equals("CONFIG_GASES_OFF"))
            {
                extern CalibrationConfig calibConfig;
                calibConfig.enableGasSensors = false;
                safePrintln("Gas sensors disabled");
            }
            else if (command.startsWith("FAN_"))
            {
                // Fan control commands
                if (!config.enableFan)
                {
                    safePrintln("Fan control is DISABLED in configuration");
                }
                else
                {
                    if (command.equals("FAN_ON"))
                    {
                        setFanSpeed(50); // Default 50% speed
                        safePrintln("Fan turned ON at 50% speed");
                    }
                    else if (command.equals("FAN_OFF"))
                    {
                        setFanSpeed(0);
                        safePrintln("Fan turned OFF");
                    }
                    else if (command.startsWith("FAN_SPEED_"))
                    {
                        int speed = command.substring(10).toInt(); // Remove "FAN_SPEED_"
                        if (speed >= 0 && speed <= 100)
                        {
                            setFanSpeed(speed);
                            safePrintln("Fan speed set to " + String(speed) + "%");
                        }
                        else
                        {
                            safePrintln("Invalid fan speed (0-100): " + String(speed));
                        }
                    }
                    else if (command.equals("GLINE_ON"))
                    {
                        setGLine(true);
                        safePrintln("GLine router ENABLED");
                    }
                    else if (command.equals("GLINE_OFF"))
                    {
                        setGLine(false);
                        safePrintln("GLine router DISABLED");
                    }
                    else if (command.startsWith("SLEEP_"))
                    {
                        // Sleep mode commands
                        if (command.equals("SLEEP_STOP") || command.equals("SLEEP_WAKE"))
                        {
                            stopSleepMode();
                            safePrintln("Sleep mode stopped");
                        }
                        else if (command.startsWith("SLEEP_"))
                        {
                            // Format: SLEEP_[delay]_[duration] (e.g., SLEEP_60_300 for 60s delay, 300s duration)
                            String params = command.substring(6); // Remove "SLEEP_"
                            int separatorIndex = params.indexOf('_');
                            if (separatorIndex > 0)
                            {
                                unsigned long delaySec = params.substring(0, separatorIndex).toInt();
                                unsigned long durationSec = params.substring(separatorIndex + 1).toInt();
                                if (delaySec > 0 && durationSec > 0)
                                {
                                    startSleepMode(delaySec, durationSec);
                                    safePrintln("Sleep mode scheduled: " + String(delaySec) + "s delay, " + String(durationSec) + "s duration");
                                }
                                else
                                {
                                    safePrintln("Invalid sleep parameters: delay=" + String(delaySec) + ", duration=" + String(durationSec));
                                }
                            }
                            else
                            {
                                safePrintln("Invalid sleep format. Use: SLEEP_[delay]_[duration] (e.g., SLEEP_60_300)");
                            }
                        }
                    }
                    else if (command.equals("FAN_STATUS"))
                    {
                        FanStatus status = getFanStatus();
                        safePrintln("=== Fan Status ===");
                        safePrintln("Enabled: " + String(status.enabled ? "YES" : "NO"));
                        safePrintln("Duty Cycle: " + String(status.dutyCycle) + "%");
                        safePrintln("RPM: " + String(status.rpm));
                        safePrintln("GLine: " + String(status.glineEnabled ? "ON" : "OFF"));
                        safePrintln("Sleep Mode: " + String(status.sleepMode ? "ACTIVE" : "INACTIVE"));
                        if (status.sleepMode)
                        {
                            safePrintln("- Sleep Start: " + String(status.sleepStartTime));
                            safePrintln("- Sleep Duration: " + String(status.sleepDuration) + "ms");
                            safePrintln("- Sleep End: " + String(status.sleepEndTime));
                        }
                    }
                }
            }
        }
        else if (command.equals("DATATYPE_CURRENT"))
        {
            if (config.enableModbus)
            {
                setCurrentDataType(DATA_CURRENT);
                safePrintln("Switched to current data mode");
            }
            else
            {
                safePrintln("Modbus not enabled");
            }
        }
        else if (command.equals("DATATYPE_FAST"))
        {
            if (config.enableModbus)
            {
                setCurrentDataType(DATA_FAST_AVG);
                safePrintln("Switched to fast average mode (10s)");
            }
            else
            {
                safePrintln("Modbus not enabled");
            }
        }
        else if (command.equals("DATATYPE_SLOW"))
        {
            if (config.enableModbus)
            {
                setCurrentDataType(DATA_SLOW_AVG);
                safePrintln("Switched to slow average mode (5min)");
            }
            else
            {
                safePrintln("Modbus not enabled");
            }
        }
        else if (command.equals("DATATYPE_CYCLE"))
        {
            if (config.enableModbus)
            {
                cycleDataType();
                safePrint("Current data type: ");
                safePrintln(getCurrentDataTypeName());
            }
            else
            {
                safePrintln("Modbus not enabled");
            }
        }
        else if (command.equals("DATATYPE_STATUS"))
        {
            if (config.enableModbus)
            {
                safePrint("Current data type: ");
                safePrintln(getCurrentDataTypeName());
            }
            else
            {
                safePrintln("Modbus not enabled");
            }
        }
        else if (command.equals("MODBUS_STATUS"))
        {
            if (isSerialAvailable())
            {
                safePrintln("=== Modbus Status ===");
                safePrintln("Enabled: " + String(config.enableModbus ? "YES" : "NO"));
                safePrintln("Had Activity: " + String(hasHadModbusActivity ? "YES" : "NO"));
                safePrintln("Last Activity: " + String((millis() - lastModbusActivity) / 1000) + " seconds ago");
                safePrintln("Timeout: " + String(MODBUS_TIMEOUT / 1000) + " seconds");
                safePrintln("System Uptime: " + String(millis() / 1000) + " seconds");
            }
        }
        else if (command.equals("CALIB_DATA"))
        {
            if (isSerialAvailable())
            {
                extern CalibratedSensorData calibratedData;
                extern CalibrationConfig calibConfig;

                safePrintln("=== Calibration Data ===");
                safePrintln("Enabled: " + String(calibConfig.enableCalibration ? "YES" : "NO"));
                safePrintln("Valid: " + String(calibratedData.valid ? "YES" : "NO"));

                if (calibConfig.enableCalibration && calibratedData.valid)
                {
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
                }
                else
                {
                    safePrintln("No valid calibration data available");
                }
            }
        }
        else if (command.equals("TIME_INFO"))
        {
            if (isSerialAvailable())
            {
                safePrintln("=== Time Information ===");
                safePrintln("Time Synchronized: " + String(isTimeSet() ? "YES" : "NO"));
                if (isTimeSet())
                {
                    safePrintln("Current Time: " + getFormattedTime());
                    safePrintln("Current Date: " + getFormattedDate());
                    safePrintln("Epoch Time: " + String(getEpochTime()));

                    struct tm timeinfo;
                    if (getLocalTime(&timeinfo))
                    {
                        safePrintln("Day of Week: " + String(timeinfo.tm_wday) + " (0=Sunday)");
                        safePrintln("Hours: " + String(timeinfo.tm_hour));
                        safePrintln("Minutes: " + String(timeinfo.tm_min));
                        safePrintln("Seconds: " + String(timeinfo.tm_sec));
                        safePrintln("Day: " + String(timeinfo.tm_mday));
                        safePrintln("Month: " + String(timeinfo.tm_mon + 1));
                        safePrintln("Year: " + String(timeinfo.tm_year + 1900));
                        safePrintln("Timezone: Poland (UTC+1/UTC+2 with automatic DST)");
                    }
                }
                else
                {
                    safePrintln("Time not synchronized - check WiFi connection");
                }
                safePrintln("System Uptime: " + String(millis() / 1000) + " seconds");
            }
        }
        else if (command.equals("HISTORY"))
        {
            if (isSerialAvailable())
            {
                safePrintln("=== Sensor History Status ===");
                printHistoryMemoryUsage();
                printHistoryStatus();
            }
        }
        else if (command.equals("BATTERY"))
        {
            if (isSerialAvailable())
            {
                safePrintln("=== Battery Status ===");
                if (batteryData.valid)
                {
                    safePrintln("Voltage: " + String(batteryData.voltage, 3) + "V");
                    safePrintln("Current: " + String(batteryData.current, 2) + "mA");
                    safePrintln("Power: " + String(batteryData.power, 1) + "mW");
                    safePrintln("Charge: " + String(batteryData.chargePercent) + "%");
                    safePrintln("Source: " + String(batteryData.isBatteryPowered ? "External" : "Battery"));
                    safePrintln("Low Battery: " + String(batteryData.lowBattery ? "YES" : "NO"));
                    safePrintln("Critical: " + String(batteryData.criticalBattery ? "YES" : "NO"));
                    safePrintln("OFF Pin: " + String(digitalRead(OFF_PIN) ? "HIGH" : "LOW"));
                }
                else
                {
                    safePrintln("No valid battery data available");
                }
            }
        }
        else if (command.equals("RESTART"))
        {
            safePrintln("Restarting system...");
            delay(1000);
            ESP.restart();
        }
        else if (command.equals("MEMORY_EMERGENCY"))
        {
            forceWebSocketReset("Emergency reset from Serial command");
        }
        else if (command.equals("MEMORY_SMART"))
        {
            // Użyj nowego systemu resetowania WebSocket
        forceWebSocketReset("Manual reset from Serial command");
        }
        else if (command.equals("MCP3424_MAPPING"))
        {
            if (isSerialAvailable())
            {
                displayMCP3424MappingInfo();
            }
        }
        else if (command.equals("MCP3424_SCAN"))
        {
            if (isSerialAvailable())
            {
                safePrintln("=== Rescanning MCP3424 devices ===");
                scanAndMapMCP3424Devices();
            }
        }
        else if (command.equals("MCP3424_RESET"))
        {
            if (isSerialAvailable())
            {
                safePrintln("=== Resetting MCP3424 mapping to defaults ===");
                initializeDefaultMCP3424Mapping();
                scanAndMapMCP3424Devices();
                saveMCP3424Config(mcp3424Config);
                safePrintln("MCP3424 mapping reset and saved");
            }
        }
        else if (command.startsWith("MCP3424_GET_"))
        {
            if (isSerialAvailable())
            {
                String gasType = command.substring(12); // Remove "MCP3424_GET_"
                int8_t deviceIndex = getMCP3424DeviceByGasType(gasType.c_str());
                
                if (deviceIndex >= 0) {
                    uint8_t i2cAddr = getMCP3424I2CAddressByDeviceIndex(deviceIndex);
                    safePrint("Gas type '");
                    safePrint(gasType);
                    safePrint("' -> Device ");
                    safePrint(String(deviceIndex));
                    safePrint(" at I2C address 0x");
                    safePrintln(String(i2cAddr, HEX));
                    
                    // Show channel values
                    safePrintln("Channel values:");
                    for (uint8_t ch = 0; ch < 4; ch++) {
                        float value = getMCP3424Value(deviceIndex, ch);
                        safePrint("  CH");
                        safePrint(String(ch));
                        safePrint(": ");
                        safePrint(String(value, 6));
                        safePrintln("V");
                    }
                } else {
                    safePrint("Gas type '");
                    safePrint(gasType);
                    safePrintln("' not found, not enabled, or not detected");
                }
            }
        }
        else if (command.startsWith("MCP3424_ADDR_"))
        {
            if (isSerialAvailable())
            {
                String addrStr = command.substring(13); // Remove "MCP3424_ADDR_"
                uint8_t addr = strtol(addrStr.c_str(), NULL, 16); // Parse hex
                
                int8_t deviceIndex = getMCP3424DeviceByI2CAddress(addr);
                String gasType = getMCP3424GasTypeByI2CAddress(addr);
                
                if (deviceIndex >= 0) {
                    safePrint("I2C address 0x");
                    safePrint(String(addr, HEX));
                    safePrint(" -> Device ");
                    safePrint(String(deviceIndex));
                    safePrint(" (");
                    safePrint(gasType);
                    safePrintln(")");
                } else {
                    safePrint("I2C address 0x");
                    safePrint(String(addr, HEX));
                    safePrintln(" not found, not enabled, or not detected");
                }
            }
        }
    }

    // Process Serial1 commands if needed
    if (Serial1.available() > 0)
    {
        String command = Serial1.readStringUntil('\n');
        command.trim();

        if (command.equals("SEND"))
        {
            sendDataFlag = true;
        }
    }

    // Send data if flag is set - only if Serial is connected
    if (sendDataFlag && isSerialAvailable())
    {
        safePrintln("=== Sensor Data ===");

        if (config.enableSolarSensor && solarSensorStatus)
        {
            printSolarData();
        }

        if (config.enableOPCN3Sensor && opcn3SensorStatus)
        {
            safePrint("OPCN3: ");
            safePrint("Temp=");
            safePrint(String(opcn3Data.getTempC()));
            safePrint("°C, Hum=");
            safePrint(String(opcn3Data.getHumidity()));
            safePrint(", PM1=");
            safePrint(String(opcn3Data.pm1));
            safePrint(", PM2.5=");
            safePrint(String(opcn3Data.pm2_5));
            safePrint(", PM10=");
            safePrintln(String(opcn3Data.pm10));
        }

        if (config.enableI2CSensors && i2cSensorStatus)
        {
            safePrint("I2C Sensor: ");
            safePrint("Temp=");
            safePrint(String(i2cSensorData.temperature));
            safePrint("°C, Hum=");
            safePrint(String(i2cSensorData.humidity));
            safePrintln("%");
        }

        if (config.enableSCD41 && scd41SensorStatus && i2cSensorData.valid)
        {
            safePrint("SCD41: ");
            safePrint("CO2=");
            safePrint(String(i2cSensorData.co2, 0));
            safePrint(" ppm, Temp=");
            safePrint(String(i2cSensorData.temperature, 2));
            safePrint("°C, Hum=");
            safePrint(String(i2cSensorData.humidity, 2));
            safePrintln("%");
        }

        if (config.enableMCP3424 && mcp3424SensorStatus)
        {
            safePrint("MCP3424 Devices: ");
            safePrintln(String(mcp3424Data.deviceCount));

            for (uint8_t device = 0; device < mcp3424Data.deviceCount; device++)
            {
                safePrint("  Device ");
                safePrint(String(device));
                safePrint(" (0x");
                safePrint(String(mcp3424Data.addresses[device], HEX));
                safePrint("): ");

                if (mcp3424Data.valid[device])
                {
                    for (int ch = 0; ch < 4; ch++)
                    {
                        safePrint("Ch");
                        safePrint(String(ch));
                        safePrint("=");
                        safePrint(String(mcp3424Data.channels[device][ch], 4));
                        safePrint("V");
                        if (ch < 3)
                            safePrint(", ");
                    }
                    safePrintln("");
                }
                else
                {
                    safePrintln("Invalid");
                }
            }
        }

        if (config.enableADS1110 && ads1110SensorStatus)
        {
            safePrint("ADS1110: Voltage=");
            safePrint(String(ads1110Data.voltage, 6));
            safePrintln("V");
        }

        if (config.enableINA219 && ina219SensorStatus)
        {
            safePrint("INA219: Bus=");
            safePrint(String(ina219Data.busVoltage, 3));
            safePrint("V, Current=");
            safePrint(String(ina219Data.current, 2));
            safePrint("mA, Power=");
            safePrint(String(ina219Data.power, 2));
            safePrintln("mW");
        }

        if (config.enableIPS && ipsSensorStatus)
        {
            safePrint("IPS Sensor PC: ");
            for (int i = 0; i < 7; i++)
            {
                safePrint(String(ipsSensorData.pc_values[i]));
                if (i < 6)
                    safePrint(",");
            }
            safePrint(" PM: ");
            for (int i = 0; i < 7; i++)
            {
                safePrint(String(ipsSensorData.pm_values[i]));
                if (i < 6)
                    safePrint(",");
            }
            safePrintln("");
        }

        sendDataFlag = false;
    }
    else if (sendDataFlag)
    {
        // Clear flag even if Serial not available
        sendDataFlag = false;
    }
}

void updateSystemStatus()
{
    static unsigned long lastStatusUpdate = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastStatusUpdate >= 10000)
    { // Update every 5 seconds
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

        if (config.enableSHT30)
        {
            enabledSensors++;
            if (sht30SensorStatus)
                workingSensors++;
        }
        if (config.enableBME280)
        {
            enabledSensors++;
            if (bme280SensorStatus)
                workingSensors++;
        }
        if (config.enableSCD41)
        {
            enabledSensors++;
            if (scd41SensorStatus)
                workingSensors++;
        }
        if (config.enableMCP3424)
        {
            enabledSensors++;
            if (mcp3424SensorStatus)
                workingSensors++;
        }
        if (config.enableADS1110)
        {
            enabledSensors++;
            if (ads1110SensorStatus)
                workingSensors++;
        }
        if (config.enableINA219)
        {
            enabledSensors++;
            if (ina219SensorStatus)
                workingSensors++;
        }
        if (config.enableSolarSensor)
        {
            enabledSensors++;
            if (solarSensorStatus)
                workingSensors++;
        }
        if (config.enableIPS)
        {
            enabledSensors++;
            if (ipsSensorStatus)
                workingSensors++;
        }
        if (config.enableOPCN3Sensor)
        {
            enabledSensors++;
            if (opcn3SensorStatus)
                workingSensors++;
        }

        // Ustaw kolor LED na podstawie statusu wszystkich wlaczonych czujnikow (tylko jeśli nie w trybie niskiego poboru mocy)
        if (!config.lowPowerMode)
        {
            if (enabledSensors > 0)
            {
                if (workingSensors == enabledSensors)
                {
                    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Zielony - wszystkie wlaczone dzialaja
                }
                else if (workingSensors >= 2)
                {
                    pixels.setPixelColor(0, pixels.Color(0, 255, 255)); // Cyjan - wiekszosc dziala
                }
                else if (workingSensors == 1)
                {
                    pixels.setPixelColor(0, pixels.Color(255, 255, 0)); // Zolty - tylko 1 dziala
                }
                else
                {
                    pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Czerwony - zadne nie dzialaja
                }
            }
            else
            {
                pixels.setPixelColor(0, pixels.Color(128, 128, 128)); // Szary - brak wlaczonych czujnikow
            }
            pixels.show();
        }
    }
}
void watchDogTask(void *parameter)
{
    // Initialize task watchdog timer (18 seconds timeout)
    ESP_ERROR_CHECK(esp_task_wdt_init(18, true));
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    for (;;)
    {
        ESP_ERROR_CHECK(esp_task_wdt_reset());
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }
}

// Battery monitoring functions
void initializeBatteryMonitoring()
{
    // Initialize OFF pin - default LOW (safe state)
    pinMode(OFF_PIN, OUTPUT);
    digitalWrite(OFF_PIN, LOW);
    safePrintln("Battery monitoring initialized - OFF pin set to LOW");
}

void setOffPin(bool state)
{
    digitalWrite(OFF_PIN, state ? HIGH : LOW);
    safePrintln("OFF pin set to: " + String(state ? "HIGH" : "LOW"));
}

float calculateBatteryPercentage(float voltage)
{
    // Li-ion 2S battery: 7.25V = 0%, 8.35V = 100%
    const float MIN_VOLTAGE = 7.15;
    const float MAX_VOLTAGE = 8.25;

    if (voltage <= MIN_VOLTAGE)
        return 0.0;
    if (voltage >= MAX_VOLTAGE)
        return 100.0;

    return ((voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100.0;
}

bool isBatteryPowered(float current)
{
    // If current > -20mA, we're powered by external source (charging or external power)
    // If current < -20mA, we're running on battery (discharging)
    return current < -20.0;
}

void updateBatteryStatus()
{
    if (!config.enableINA219 || !ina219SensorStatus || !ina219Data.valid)
    {
        return;
    }

    // Update voltage history (moving average)
    batteryData.voltageHistory[batteryData.voltageIndex] = ina219Data.busVoltage;
    batteryData.voltageIndex = (batteryData.voltageIndex + 1) % BatteryData::VOLTAGE_AVERAGE_SIZE;

    if (batteryData.voltageIndex == 0)
    {
        batteryData.voltageHistoryFull = true;
    }

    // Calculate average voltage
    float avgVoltage = 0.0;
    int count = batteryData.voltageHistoryFull ? BatteryData::VOLTAGE_AVERAGE_SIZE : batteryData.voltageIndex;

    for (int i = 0; i < count; i++)
    {
        avgVoltage += batteryData.voltageHistory[i];
    }
    avgVoltage /= count;

    // Update battery data
    batteryData.voltage = avgVoltage;
    batteryData.current = ina219Data.current;
    batteryData.power = ina219Data.power;
    batteryData.chargePercent = (uint8_t)calculateBatteryPercentage(avgVoltage);
    batteryData.isBatteryPowered = isBatteryPowered(ina219Data.current);
    batteryData.lowBattery = (avgVoltage < 7.2);
    batteryData.criticalBattery = (avgVoltage < 7.09);
    batteryData.valid = true;
    batteryData.lastUpdate = millis();

    // Debug output every 30 seconds for monitoring 10-second average
    static unsigned long lastBatteryDebug = 0;
    if (millis() - lastBatteryDebug > 30000) {
        lastBatteryDebug = millis();
        safePrintln("=== Battery Status (10s average) ===");
        safePrint("Voltage: "); safePrint(String(avgVoltage, 3)); safePrintln("V");
        safePrint("Samples: "); safePrint(String(count)); safePrint("/"); safePrintln(String(BatteryData::VOLTAGE_AVERAGE_SIZE));
        safePrint("Current: "); safePrint(String(ina219Data.current, 2)); safePrintln("mA");
        safePrint("Power: "); safePrint(String(ina219Data.power, 1)); safePrintln("mW");
        safePrint("Charge: "); safePrint(String(batteryData.chargePercent)); safePrintln("%");
        safePrint("Source: "); safePrintln(batteryData.isBatteryPowered ? "Battery" : "External");
        safePrint("Low Battery (7.2V): "); safePrintln(batteryData.lowBattery ? "YES" : "NO");
        safePrint("Critical (7.09V): "); safePrintln(batteryData.criticalBattery ? "YES" : "NO");
    }
}

void checkBatteryShutdown()
{
    if (!batteryData.valid)
        return;

    // If 10-second average battery voltage drops below 7.09V (critical), activate OFF pin
    if (batteryData.criticalBattery && batteryData.isBatteryPowered)
    {
        safePrintln("CRITICAL: 10-second average battery voltage below 7.09V! Activating OFF pin...");

        // Send critical battery notification
        sendBatteryCriticalNotification();

        delay(2000);

        setOffPin(HIGH);

        // Wait a moment for devices to shut down
        delay(2000);

        // Keep OFF pin HIGH for 5 seconds to ensure shutdown
        unsigned long shutdownStart = millis();
        while (millis() - shutdownStart < 5000)
        {
            safePrintln("Shutdown in progress... " + String((5000 - (millis() - shutdownStart)) / 1000) + "s");
            delay(1000);
        }

        // Reset OFF pin to LOW and restart system
        setOffPin(LOW);
        safePrintln("System restarting after critical battery shutdown...");
        delay(1000);
        ESP.restart();
    }
}

// Pushbullet notification functions
String getUptimeString()
{
    unsigned long uptime = millis() / 1000;
    unsigned long days = uptime / 86400;
    unsigned long hours = (uptime % 86400) / 3600;
    unsigned long minutes = (uptime % 3600) / 60;

    String result = "";
    if (days > 0)
        result += String(days) + "d ";
    if (hours > 0 || days > 0)
        result += String(hours) + "h ";
    result += String(minutes) + "m";

    return result;
}

void sendPushbulletNotification(const String &title, const String &message)
{
    if (!config.enablePushbullet || strlen(config.pushbulletToken) == 0)
    {
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        safePrintln("Pushbullet: No WiFi connection");
        return;
    }

    HTTPClient http;
    http.begin("https://api.pushbullet.com/v2/pushes");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Access-Token", config.pushbulletToken);

    // Use ArduinoJson to properly format JSON
    DynamicJsonDocument doc(1024);
    doc["type"] = "note";
    doc["title"] = title;
    doc["body"] = message;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode == 200)
    {
        safePrintln("Pushbullet notification sent successfully");
    }
    else
    {
        safePrintln("Pushbullet notification failed: " + String(httpResponseCode));
        safePrintln("Response: " + http.getString());
        safePrintln("Sent JSON: " + jsonPayload);
    }

    http.end();
}

void sendBatteryCriticalNotification()
{
    // if (!batteryData.valid)
    //     return;

    String title = "🔋 CRITICAL BATTERY - Device Shutting Down";
    String message = "Device: ESP32 Sensor Cube\n";
    message += "Voltage: " + String(batteryData.voltage, 3) + "V\n";
    message += "Charge: " + String(batteryData.chargePercent) + "%\n";
    message += "Current: " + String(batteryData.current, 2) + "mA\n";
    message += "Uptime: " + getUptimeString() + "\n";
    message += "Time: " + getFormattedTime() + " " + getFormattedDate() + "\n";
    message += "IP: " + WiFi.localIP().toString()+"\n";
    //add vrsion and device id
    message += "Version: " + String(FIRMWARE_VERSION) + "\n";
    message += "Device ID: " + String(config.DeviceID) + "\n";

    sendPushbulletNotification(title, message);
    delay(1000);
    digitalWrite(OFF_PIN, HIGH);
}

void sendSystemStartupNotification()
{
    String title = "🚀 ESP32 Sensor Cube Started";
    String message = "Device: ESP32 Sensor Cube\n";
    message += "Version: " + String(FIRMWARE_VERSION) + "\n";
    message += "IP: " + WiFi.localIP().toString() + "\n";
    message += "WiFi Signal: " + String(WiFi.RSSI()) + " dBm\n";
    message += "Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB\n";
    message += "Time: " + getFormattedTime() + " " + getFormattedDate()+"\n";
   
    message += "Device ID: " + String(config.DeviceID) + "\n";

    sendPushbulletNotification(title, message);
}

// Nowa funkcja z timeoutem dla powiadomien - bezpieczniejsza wersja
void sendSystemStartupNotificationWithTimeout() {
    if (!config.enablePushbullet || strlen(config.pushbulletToken) == 0) {
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        safePrintln("Pushbullet: No WiFi connection");
        return;
    }

    HTTPClient http;
    http.setTimeout(10000); // 10 sekund timeout
    
    http.begin("https://api.pushbullet.com/v2/pushes");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Access-Token", config.pushbulletToken);

    // Uzyj mniejszego bufora JSON dla bezpieczenstwa
    DynamicJsonDocument doc(512); // Zmniejszone z 1024
    doc["type"] = "note";
    doc["title"] = "🚀 ESP32 Sensor Cube Started";
    
    String message = "Device: ESP32 Sensor Cube\n";
    message += "Version: " + String(FIRMWARE_VERSION) + "\n";
    message += "IP: " + WiFi.localIP().toString() + "\n";
    message += "WiFi Signal: " + String(WiFi.RSSI()) + " dBm\n";
    message += "Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB\n";
    message += "Time: " + getFormattedTime() + " " + getFormattedDate() + "\n";
    message += "Device ID: " + String(config.DeviceID) + "\n";
    
    doc["body"] = message;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode == 200) {
        safePrintln("Pushbullet notification sent successfully");
    } else {
        safePrintln("Pushbullet notification failed: " + String(httpResponseCode));
        // Nie loguj pelnej odpowiedzi dla bezpieczenstwa
    }

    http.end();
}

// ===========================================
// HEAP DEBUGGING FUNCTIONS
// ===========================================

// void initializeHeapDebugging() {
//     // Set baseline heap size
//     heap_baseline = ESP.getFreeHeap();
//     heap_low_watermark = heap_baseline;

//     // Initialize heap history
//     for (int i = 0; i < MAX_HEAP_SNAPSHOTS; i++) {
//         heap_history[i].timestamp = 0;
//     }

//     safePrintln("=== Heap Debugging Initialized ===");
//     safePrintln("Baseline heap: " + String(heap_baseline) + " bytes");
//     safePrintln("Monitoring enabled: " + String(heap_monitoring_enabled ? "YES" : "NO"));

//     // Take initial snapshot
//     takeHeapSnapshot();

//     // Print initial heap info
//     printHeapInfo();
// }

// void printHeapInfo() {
//     // Basic heap information
//     uint32_t free_heap = ESP.getFreeHeap();
//     uint32_t total_heap = ESP.getHeapSize();
//     uint32_t min_free = ESP.getMinFreeHeap();

//     // Advanced heap caps info
//     uint32_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
//     uint32_t free_spiram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
//     uint32_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
//     uint32_t min_free_internal = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);

//     safePrintln("=== HEAP STATUS ===");
//     safePrintln("Free: " + String(free_heap) + " bytes (" + String((free_heap * 100) / total_heap) + "%)");
//     safePrintln("Total: " + String(total_heap) + " bytes");
//     safePrintln("Min Free Ever: " + String(min_free) + " bytes");
//     safePrintln("Largest Block: " + String(largest_block) + " bytes");
//     safePrintln("Internal Free: " + String(free_internal) + " bytes");
//     safePrintln("SPIRAM Free: " + String(free_spiram) + " bytes");
//     safePrintln("Min Internal: " + String(min_free_internal) + " bytes");

//     if (heap_baseline > 0) {
//         int32_t memory_delta = (int32_t)free_heap - (int32_t)heap_baseline;
//         safePrintln("Memory Change: " + String(memory_delta) + " bytes");
//         if (memory_delta < -1000) {
//             safePrintln("WARNING: Possible memory leak detected!");
//         }
//     }

//     // Memory fragmentation analysis
//     float fragmentation = 100.0 - ((float)largest_block * 100.0 / (float)free_heap);
//     safePrintln("Fragmentation: " + String(fragmentation, 1) + "%");
// }

// void printDetailedHeapInfo() {
//     safePrintln("=== DETAILED HEAP ANALYSIS ===");

//     // Get detailed heap information
//     multi_heap_info_t info;
//     heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);

//     safePrintln("Internal Heap Details:");
//     safePrintln("  Total Free: " + String(info.total_free_bytes) + " bytes");
//     safePrintln("  Total Allocated: " + String(info.total_allocated_bytes) + " bytes");
//     safePrintln("  Largest Free Block: " + String(info.largest_free_block) + " bytes");
//     safePrintln("  Min Free Ever: " + String(info.minimum_free_bytes) + " bytes");
//     safePrintln("  Allocated Blocks: " + String(info.allocated_blocks));
//     safePrintln("  Free Blocks: " + String(info.free_blocks));
//     safePrintln("  Total Blocks: " + String(info.total_blocks));

//     // SPIRAM info if available
//     if (ESP.getPsramSize() > 0) {
//         heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
//         safePrintln("SPIRAM Heap Details:");
//         safePrintln("  Total Free: " + String(info.total_free_bytes) + " bytes");
//         safePrintln("  Total Allocated: " + String(info.total_allocated_bytes) + " bytes");
//         safePrintln("  Largest Free Block: " + String(info.largest_free_block) + " bytes");
//         safePrintln("  Min Free Ever: " + String(info.minimum_free_bytes) + " bytes");
//         safePrintln("  Allocated Blocks: " + String(info.allocated_blocks));
//         safePrintln("  Free Blocks: " + String(info.free_blocks));
//     }
// }

// void takeHeapSnapshot() {
//     if (!heap_monitoring_enabled) return;

//     HeapSnapshot& snapshot = heap_history[heap_history_index];
//     snapshot.free_heap = ESP.getFreeHeap();
//     snapshot.free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
//     snapshot.free_spiram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
//     snapshot.largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
//     snapshot.min_free_heap = ESP.getMinFreeHeap();
//     snapshot.timestamp = millis();

//     // Update watermark
//     if (snapshot.free_heap < heap_low_watermark) {
//         heap_low_watermark = snapshot.free_heap;
//     }

//     heap_history_index = (heap_history_index + 1) % MAX_HEAP_SNAPSHOTS;
//     if (heap_history_index == 0) {
//         heap_history_full = true;
//     }
// }

// void startHeapMonitoring() {
//     heap_monitoring_enabled = true;
//     takeHeapSnapshot();
//     safePrintln("Heap monitoring started - taking periodic snapshots");
// }

// void stopHeapMonitoring() {
//     heap_monitoring_enabled = false;
//     safePrintln("Heap monitoring stopped");
// }

// void printHeapHistory() {
//     safePrintln("=== HEAP HISTORY ===");

//     int start_idx = heap_history_full ? heap_history_index : 0;
//     int count = heap_history_full ? MAX_HEAP_SNAPSHOTS : heap_history_index;

//     safePrintln("Showing last " + String(count) + " snapshots:");
//     safePrintln("Time(s)\tFree\tInternal\tSPIRAM\tLargest\tMinEver");

//     for (int i = 0; i < count; i++) {
//         int idx = heap_history_full ? (start_idx + i) % MAX_HEAP_SNAPSHOTS : i;
//         HeapSnapshot& snap = heap_history[idx];

//         if (snap.timestamp > 0) {
//             safePrint(String(snap.timestamp / 1000) + "\t");
//             safePrint(String(snap.free_heap) + "\t");
//             safePrint(String(snap.free_internal) + "\t");
//             safePrint(String(snap.free_spiram) + "\t");
//             safePrint(String(snap.largest_block) + "\t");
//             safePrintln(String(snap.min_free_heap));
//         }
//     }

//     // Analyze trend
//     if (count >= 3) {
//         int first_idx = heap_history_full ? start_idx : 0;
//         int last_idx = heap_history_full ?
//             (start_idx + count - 1) % MAX_HEAP_SNAPSHOTS : count - 1;

//         int32_t trend = (int32_t)heap_history[last_idx].free_heap -
//                        (int32_t)heap_history[first_idx].free_heap;

//         safePrintln("Memory trend: " + String(trend) + " bytes");
//         if (trend < -1000) {
//             safePrintln("WARNING: Downward memory trend detected!");
//         }
//     }
// }

// void analyzeMemoryLeaks() {
//     safePrintln("=== MEMORY LEAK ANALYSIS ===");

//     // Check heap integrity
//     bool integrity_ok = heap_caps_check_integrity_all(true);
//     safePrintln("Heap integrity: " + String(integrity_ok ? "OK" : "CORRUPTED"));

//     if (!integrity_ok) {
//         safePrintln("ERROR: Heap corruption detected!");
//         // Try to get more info about corruption
//         heap_caps_check_integrity(MALLOC_CAP_INTERNAL, true);
//         if (ESP.getPsramSize() > 0) {
//             heap_caps_check_integrity(MALLOC_CAP_SPIRAM, true);
//         }
//     }

//     // Compare current vs baseline
//     uint32_t current_free = ESP.getFreeHeap();
//     if (heap_baseline > 0) {
//         int32_t memory_change = (int32_t)current_free - (int32_t)heap_baseline;

//         safePrintln("Memory analysis:");
//         safePrintln("  Baseline: " + String(heap_baseline) + " bytes");
//         safePrintln("  Current: " + String(current_free) + " bytes");
//         safePrintln("  Change: " + String(memory_change) + " bytes");

//         if (memory_change < -5000) {
//             safePrintln("  Status: MEMORY LEAK DETECTED!");
//         } else if (memory_change < -1000) {
//             safePrintln("  Status: Possible memory leak");
//         } else {
//             safePrintln("  Status: Memory usage normal");
//         }
//     }

//     // WebSocket specific analysis
//     extern int wsClientCount;
//     extern AsyncWebSocket ws;
//     safePrintln("WebSocket analysis:");
//     safePrintln("  Tracked clients: " + String(wsClientCount));
//     safePrintln("  Active connections: " + String(ws.count()));
//     safePrintln("  WebSocket memory usage: ~" + String((wsClientCount * 100) + (ws.count() * 200)) + " bytes estimated");
// }

// void performHeapAnalysis() {
//     safePrintln("\n===========================================");
//     safePrintln("    COMPREHENSIVE HEAP ANALYSIS");
//     safePrintln("===========================================");

//     printHeapInfo();
//     safePrintln("");
//     printDetailedHeapInfo();
//     safePrintln("");
//     analyzeMemoryLeaks();

//     safePrintln("===========================================");
// }

// void printTaskMemoryStats() {
//     safePrintln("=== TASK MEMORY OVERVIEW ===");

//     // Get basic FreeRTOS task info (Arduino compatible)
//     UBaseType_t taskCount = uxTaskGetNumberOfTasks();
//     safePrintln("Total active tasks: " + String(taskCount));

//     safePrintln("");
//     safePrintln("┌────────────────────┬─────────────────────┬─────────────────┐");
//     safePrintln("│ TASK ANALYSIS      │ CURRENT STATUS      │ NOTES           │");
//     safePrintln("├────────────────────┼─────────────────────┼─────────────────┤");
//     safePrintln("│ Total Tasks        │ " + String(taskCount) + "                   │ Active FreeRTOS │");
//     safePrintln("│ Current Task       │ " + String(pcTaskGetTaskName(NULL)) + "            │ Running now     │");

//     // Check specific known tasks
//     TaskHandle_t mainTask = xTaskGetHandle("loopTask");
//     if (mainTask != NULL) {
//         UBaseType_t mainStack = uxTaskGetStackHighWaterMark(mainTask);
//         safePrintln("│ Main Loop Task     │ Stack: " + String(mainStack) + " bytes     │ Arduino loop    │");
//     } else {
//         safePrintln("│ Main Loop Task     │ Not found           │ Check task name │");
//     }

//     TaskHandle_t wifiTask = xTaskGetHandle("wifi");
//     if (wifiTask != NULL) {
//         UBaseType_t wifiStack = uxTaskGetStackHighWaterMark(wifiTask);
//         safePrintln("│ WiFi Task          │ Stack: " + String(wifiStack) + " bytes     │ WiFi management │");
//     } else {
//         safePrintln("│ WiFi Task          │ Not found           │ May use diff name│");
//     }

//     // Check our custom tasks
//     TaskHandle_t wsTask = xTaskGetHandle("wsBroadcastTask");
//     if (wsTask != NULL) {
//         UBaseType_t wsStack = uxTaskGetStackHighWaterMark(wsTask);
//         safePrintln("│ WebSocket Task     │ Stack: " + String(wsStack) + " bytes     │ WS broadcast    │");
//     } else {
//         safePrintln("│ WebSocket Task     │ Not found           │ May not be active│");
//     }

//     safePrintln("└────────────────────┴─────────────────────┴─────────────────┘");

//     safePrintln("");
//     safePrintln("Stack analysis:");
//     safePrintln("- Values show remaining stack (lower = more used)");
//     safePrintln("- WARNING if <500 bytes remaining");
//     safePrintln("- Stack overflow protection active");

//     safePrintln("");
//     safePrintln("Key tasks to monitor:");
//     safePrintln("- loopTask: Main Arduino loop");
//     safePrintln("- wifi: WiFi stack memory usage");
//     safePrintln("- async_tcp: AsyncWebServer connections");
//     safePrintln("- wsBroadcastTask: WebSocket broadcast (custom)");
// }

// void printAllTasksMemoryStats() {
//     safePrintln("=== ENHANCED TASK ANALYSIS ===");

//     // Print basic task info first
//     printTaskMemoryStats();

//     safePrintln("");
//     safePrintln("=== MEMORY ANALYSIS BY COMPONENT ===");

//     // Analyze memory by major components
//     uint32_t free_heap = ESP.getFreeHeap();
//     uint32_t total_heap = ESP.getHeapSize();
//     uint32_t used_heap = total_heap - free_heap;

//     safePrintln("Total heap usage breakdown:");
//     safePrintln("- Total heap: " + String(total_heap) + " bytes");
//     safePrintln("- Used heap: " + String(used_heap) + " bytes (" + String((used_heap * 100) / total_heap) + "%)");
//     safePrintln("- Free heap: " + String(free_heap) + " bytes (" + String((free_heap * 100) / total_heap) + "%)");

//     // Estimate memory usage by major components
//     safePrintln("");
//     safePrintln("Estimated memory usage by component:");
//     safePrintln("- WiFi stack: ~40-60KB (when connected)");
//     safePrintln("- WebSocket connections: ~2-5KB per connection");
//     safePrintln("- Sensor buffers: ~10-20KB");
//     safePrintln("- History buffer (PSRAM): External memory");
//     safePrintln("- ArduinoJson documents: Variable (check JSON usage)");

//     // Check for potential issues
//     safePrintln("");
//     safePrintln("=== POTENTIAL MEMORY ISSUES ===");
//     extern int wsClientCount;
//     extern AsyncWebSocket ws;

//     uint32_t estimated_ws_memory = (wsClientCount * 1000) + (ws.count() * 2000);
//     safePrintln("WebSocket estimated usage: " + String(estimated_ws_memory) + " bytes");
//     safePrintln("- Tracked clients: " + String(wsClientCount));
//     safePrintln("- Active connections: " + String(ws.count()));

//     if (wsClientCount != ws.count()) {
//         safePrintln("WARNING: Mismatch between tracked and actual WebSocket clients!");
//         safePrintln("This may indicate a memory leak in WebSocket client tracking.");
//     }
// }

void timeSyncTask(void *parameter) {
    safePrintln("TimeSync task started - waiting for time synchronization...");
    
    int proby = 0;
    const int max_proby = 30; // Zwiekszam do 30 prob (30 sekund)
    const int check_interval = 500; // Sprawdzaj co 500ms zamiast 1000ms dla lepszej responsywnosci
    
    while (!isTimeSet() && proby < max_proby) {
        vTaskDelay(pdMS_TO_TICKS(check_interval)); // Krotsze opoznienie
        proby++;
        
        // Reset watchdog co 10 prob (co 5 sekund)
        if (proby % 10 == 0) { // Co 10 prob pokazuj status
            safePrintln("TimeSync: proba " + String(proby) + "/" + String(max_proby));
            esp_task_wdt_reset();
        }
        
        // Sprawdz czy WiFi jest nadal polaczony
        if (WiFi.status() != WL_CONNECTED) {
            safePrintln("TimeSync: WiFi disconnected, waiting for reconnection...");
            // Czekaj na ponowne polaczenie WiFi z timeoutem
            int wifi_retry = 0;
            while (WiFi.status() != WL_CONNECTED && wifi_retry < 20) {
                vTaskDelay(pdMS_TO_TICKS(1000));
                wifi_retry++;
                esp_task_wdt_reset();
            }
            if (WiFi.status() != WL_CONNECTED) {
                safePrintln("TimeSync: WiFi reconnection failed, exiting task");
                vTaskDelete(NULL);
                return;
            }
        }
    }
    
    if (isTimeSet()) {
        safePrintln("✓ Czas zsynchronizowany: " + getFormattedTime() + " " + getFormattedDate());
        if (config.enablePushbullet && strlen(config.pushbulletToken) > 0 && WiFi.status() == WL_CONNECTED) {
            // Ustaw timeout dla operacji HTTP
            esp_task_wdt_reset();
            
            // Wyslij powiadomienie z timeoutem
            sendSystemStartupNotificationWithTimeout();
        }
    } else {
        safePrintln("✗ Blad: nie udalo sie zsynchronizowac czasu po " + String(max_proby) + " probach");
    }
    
    safePrintln("TimeSync task completed and will be deleted");
    vTaskDelete(NULL); // Usun task po zakonczeniu
}
// void printSingleTaskMemoryStats(const char* taskName) {
//     safePrintln("=== SINGLE TASK ANALYSIS ===");
//     safePrintln("Searching for task: " + String(taskName));

//     // Find specific task (Arduino compatible)
//     TaskHandle_t taskHandle = xTaskGetHandle(taskName);
//     if (taskHandle != NULL) {
//         safePrintln("Task found!");

//         // Get basic info available in Arduino Core
//         UBaseType_t stackWaterMark = uxTaskGetStackHighWaterMark(taskHandle);
//         UBaseType_t priority = uxTaskPriorityGet(taskHandle);

//         safePrintln("Task details:");
//         safePrintln("- Name: " + String(taskName));
//         safePrintln("- Current Priority: " + String(priority));
//         safePrintln("- Stack High Water Mark: " + String(stackWaterMark) + " bytes");

//         // Stack usage analysis
//         safePrintln("- Stack remaining: " + String(stackWaterMark) + " bytes");

//         if (stackWaterMark < 500) {
//             safePrintln("WARNING: Low stack remaining! Risk of stack overflow!");
//         } else if (stackWaterMark < 1000) {
//             safePrintln("CAUTION: Stack getting low, monitor closely");
//         } else {
//             safePrintln("Stack usage: OK");
//         }

//         // Additional analysis for known tasks
//         if (String(taskName) == "loopTask") {
//             safePrintln("- Type: Main Arduino loop task");
//             safePrintln("- Function: Runs setup() once, then loop() repeatedly");
//         } else if (String(taskName).indexOf("wifi") >= 0) {
//             safePrintln("- Type: WiFi management task");
//             safePrintln("- Function: Handles WiFi connections and protocols");
//         } else if (String(taskName).indexOf("ws") >= 0 || String(taskName).indexOf("WebSocket") >= 0) {
//             safePrintln("- Type: WebSocket related task");
//             safePrintln("- Function: Handles WebSocket connections and data");
//         }

//     } else {
//         safePrintln("Task '" + String(taskName) + "' not found!");
//         safePrintln("");
//         safePrintln("Common task names to try:");
//         safePrintln("- loopTask (Arduino main loop)");
//         safePrintln("- wifi (WiFi management)");
//         safePrintln("- async_tcp (AsyncWebServer)");
//         safePrintln("- Watchdog (System watchdog)");
//         safePrintln("- WiFi Reconnect (Custom WiFi task)");
//         safePrintln("");
//         safePrintln("Current task overview:");
//         printTaskMemoryStats();
//     }
// }

