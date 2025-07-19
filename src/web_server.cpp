#include <web_server.h>
#include <html.h>
#include <chart.h>
#include <sensors.h>
#include <calib.h>
#include <config.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <time.h>
#include <history.h>

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

// Global objects
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Global variables
unsigned long lastConnectionTime = 0;
bool timeInitialized = false;

// External configuration
extern FeatureConfig config;

// External sensor data and status flags (for events)
extern SolarData solarData;
extern HistogramData opcn3Data;
extern I2CSensorData i2cSensorData;
extern MCP3424Data mcp3424Data;
extern ADS1110Data ads1110Data;
extern INA219Data ina219Data;
extern SPS30Data sps30Data;
extern HCHOData hchoData;
extern IPSSensorData ipsSensorData;

extern bool solarSensorStatus;
extern bool opcn3SensorStatus;
extern bool i2cSensorStatus;
extern bool sps30SensorStatus;
extern bool mcp3424SensorStatus;
extern bool ads1110SensorStatus;
extern bool ina219SensorStatus;
extern bool hchoSensorStatus;
extern bool ipsSensorStatus;

// Calibration data
extern CalibratedSensorData calibratedData;
extern CalibrationConfig calibConfig;

// Time helper functions
String getFormattedTime() {
    if (!timeInitialized) return "00:00:00";
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "00:00:00";
    }
    
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    return String(timeStr);
}

String getFormattedDate() {
    if (!timeInitialized) return "01/01/2024";
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "01/01/2024";
    }
    
    char dateStr[20];
    strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
    return String(dateStr);
}

time_t getEpochTime() {
    if (!timeInitialized) return 0;
    return time(nullptr);
}

bool isTimeSet() {
    return timeInitialized && (time(nullptr) > 8 * 3600 * 2); // > year 1970
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        safePrintln("WebSocket client connected");
    } else if (type == WS_EVT_DISCONNECT) {
        safePrintln("WebSocket client disconnected");
    }
}

String getAllSensorJson() {
    // Check memory before building JSON
    if (ESP.getFreeHeap() < 15000) {
        return "{\"error\":\"Low memory\",\"freeHeap\":" + String(ESP.getFreeHeap()) + "}";
    }
    
    String json;
    json.reserve(3072); // Reserve 3KB for sensor JSON
    json = "{";
    json += "\"t\":" + String(millis() / 1000) + ",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"wifiSignal\":" + String(WiFi.RSSI()) + ",";
    json += "\"ntpTime\":\"" + getFormattedTime() + "\",";
    json += "\"ntpEpoch\":" + String(getEpochTime()) + ",";
    json += "\"ntpDate\":\"" + getFormattedDate() + "\",";
    json += "\"ntpValid\":" + String(isTimeSet() ? "true" : "false") + ",";
    
    // History status
    extern HistoryManager historyManager;
    json += "\"history\":{";
    json += "\"configEnabled\":" + String(config.enableHistory ? "true" : "false") + ",";
    json += "\"enabled\":" + String(config.enableHistory && historyManager.isInitialized() ? "true" : "false") + ",";
    json += "\"memoryUsed\":" + String(config.enableHistory ? historyManager.getTotalMemoryUsed() : 0) + ",";
    json += "\"memoryBudget\":" + String(TARGET_MEMORY_BYTES) + "";
    json += "},";
    json += "\"psramSize\":" + String(ESP.getPsramSize()) + ",";
    json += "\"freePsram\":" + String(ESP.getFreePsram()) + ",";
    json += "\"sensorsEnabled\":{";
    json += "\"solar\":" + String(solarSensorStatus ? "true" : "false") + ",";
    json += "\"opcn3\":" + String(opcn3SensorStatus ? "true" : "false") + ",";
    json += "\"i2c\":" + String(i2cSensorStatus ? "true" : "false") + ",";
    json += "\"sps30\":" + String(sps30SensorStatus ? "true" : "false") + ",";
    json += "\"ads1110\":" + String(ads1110SensorStatus ? "true" : "false") + ",";
    json += "\"ina219\":" + String(ina219SensorStatus ? "true" : "false") + ",";
    json += "\"hcho\":" + String(hchoSensorStatus ? "true" : "false") + ",";
    json += "\"ips\":" + String(ipsSensorStatus ? "true" : "false");
    json += "},";
    // Solar
    json += "\"solar\":{\"valid\":" + String(solarSensorStatus && solarData.valid ? "true" : "false");
    if (solarSensorStatus && solarData.valid) {
        json += ",\"V\":\"" + solarData.V + "\",\"I\":\"" + solarData.I + "\",\"VPV\":\"" + solarData.VPV + "\",\"PPV\":\"" + solarData.PPV + "\"";
    }
    json += "},";
    // OPCN3
    json += "\"opcn3\":{\"valid\":" + String(opcn3SensorStatus && opcn3Data.valid ? "true" : "false");
    if (opcn3SensorStatus && opcn3Data.valid) {
        json += ",\"PM1\":" + String(opcn3Data.pm1) + ",\"PM25\":" + String(opcn3Data.pm2_5) + ",\"PM10\":" + String(opcn3Data.pm10) + ",\"temperature\":" + String(opcn3Data.getTempC()) + ",\"humidity\":" + String(opcn3Data.getHumidity());
    }
    json += "},";
    // SPS30
    json += "\"sps30\":{\"valid\":" + String(sps30SensorStatus && sps30Data.valid ? "true" : "false");
    if (sps30SensorStatus && sps30Data.valid) {
        json += ",\"PM1\":" + String(sps30Data.pm1_0, 1) + ",\"PM25\":" + String(sps30Data.pm2_5, 1) + ",\"PM4\":" + String(sps30Data.pm4_0, 1) + ",\"PM10\":" + String(sps30Data.pm10, 1) + ",\"NC05\":" + String(sps30Data.nc0_5, 1) + ",\"NC1\":" + String(sps30Data.nc1_0, 1) + ",\"NC25\":" + String(sps30Data.nc2_5, 1) + ",\"NC4\":" + String(sps30Data.nc4_0, 1) + ",\"NC10\":" + String(sps30Data.nc10, 1) + ",\"TPS\":" + String(sps30Data.typical_particle_size, 1);
    }
    json += "},";
    // I2C
    json += "\"i2c\":{\"valid\":" + String(i2cSensorStatus && i2cSensorData.valid ? "true" : "false");
    if (i2cSensorStatus && i2cSensorData.valid) {
        json += ",\"temperature\":" + String(i2cSensorData.temperature) + ",\"humidity\":" + String(i2cSensorData.humidity) + ",\"pressure\":" + String(i2cSensorData.pressure) + ",\"CO2\":" + String(i2cSensorData.co2);
    }
    json += "},";
    // ADC data structure
    json += "\"adc\":{";
    // MCP3424
    json += "\"mcp3424\":{\"deviceCount\":" + String(mcp3424Data.deviceCount) + ",\"enabled\":" + String(mcp3424SensorStatus ? "true" : "false") + ",\"devices\":[";
    for (uint8_t device = 0; device < mcp3424Data.deviceCount; device++) {
        json += "{\"address\":\"0x" + String(mcp3424Data.addresses[device], HEX) + "\",\"valid\":" + String(mcp3424Data.valid[device] ? "true" : "false") + ",\"channels\":{\"ch1\":" + String(mcp3424Data.channels[device][0], 6) + ",\"ch2\":" + String(mcp3424Data.channels[device][1], 6) + ",\"ch3\":" + String(mcp3424Data.channels[device][2], 6) + ",\"ch4\":" + String(mcp3424Data.channels[device][3], 6) + "}}";
        if (device < mcp3424Data.deviceCount - 1) json += ",";
    }
    json += "]},";
    // ADS1110
    json += "\"ads1110\":{\"enabled\":" + String(ads1110SensorStatus ? "true" : "false") + ",\"valid\":" + String(ads1110SensorStatus && ads1110Data.valid ? "true" : "false") + ",\"voltage\":" + String(ads1110Data.voltage, 6) + ",\"dataRate\":" + String(ads1110Data.dataRate) + ",\"gain\":" + String(ads1110Data.gain) + "}";
    json += "},";
    // Power
    json += "\"power\":{";
    if (ina219SensorStatus && ina219Data.valid) {
        json += "\"valid\":true,\"busVoltage\":" + String(ina219Data.busVoltage, 3) + ",\"shuntVoltage\":" + String(ina219Data.shuntVoltage, 2) + ",\"current\":" + String(ina219Data.current, 2) + ",\"power\":" + String(ina219Data.power, 2);
    } else {
        json += "\"valid\":false";
    }
    json += "},";
    // HCHO
    json += "\"hcho\":{\"valid\":" + String(hchoSensorStatus && hchoData.valid ? "true" : "false");
    if (hchoSensorStatus && hchoData.valid) {
        json += ",\"HCHO\":" + String(hchoData.hcho, 3) + ",\"VOC\":" + String(hchoData.voc, 3) + ",\"temperature\":" + String(hchoData.temperature, 1) + ",\"humidity\":" + String(hchoData.humidity, 1) + ",\"TVOC\":" + String(hchoData.tvoc, 3) + ",\"sensorStatus\":" + String(hchoData.sensorStatus) + ",\"autoCalibration\":" + String(hchoData.autoCalibration);
    }
    json += "},";
    // IPS
    json += "\"ips\":{\"valid\":" + String(ipsSensorStatus ? "true" : "false");
    if (ipsSensorStatus) {
        json += ",\"PC\":[";
        for (int i = 0; i < 7; i++) {
            json += String(ipsSensorData.pc_values[i]);
            if (i < 6) json += ",";
        }
        json += "],\"PM\":[";
        for (int i = 0; i < 7; i++) {
            json += String(ipsSensorData.pm_values[i]);
            if (i < 6) json += ",";
        }
        json += "]";
    }
    json += "},";
    
    // Calibration data
    json += "\"calibration\":{\"enabled\":" + String(calibConfig.enableCalibration ? "true" : "false") + ",\"valid\":" + String(calibratedData.valid ? "true" : "false");
    if (calibConfig.enableCalibration && calibratedData.valid) {
        // Temperatures
        json += ",\"temperatures\":{";
        json += "\"K1\":" + (isnan(calibratedData.K1_temp) ? "null" : String(calibratedData.K1_temp, 1)) + ",\"K2\":" + (isnan(calibratedData.K2_temp) ? "null" : String(calibratedData.K2_temp, 1)) + ",\"K3\":" + (isnan(calibratedData.K3_temp) ? "null" : String(calibratedData.K3_temp, 1)) + ",\"K4\":" + (isnan(calibratedData.K4_temp) ? "null" : String(calibratedData.K4_temp, 1)) + ",\"K5\":" + (isnan(calibratedData.K5_temp) ? "null" : String(calibratedData.K5_temp, 1));
        json += ",\"K6\":" + (isnan(calibratedData.K6_temp) ? "null" : String(calibratedData.K6_temp, 1)) + ",\"K7\":" + (isnan(calibratedData.K7_temp) ? "null" : String(calibratedData.K7_temp, 1)) + ",\"K8\":" + (isnan(calibratedData.K8_temp) ? "null" : String(calibratedData.K8_temp, 1)) + ",\"K9\":" + (isnan(calibratedData.K9_temp) ? "null" : String(calibratedData.K9_temp, 1)) + ",\"K12\":" + (isnan(calibratedData.K12_temp) ? "null" : String(calibratedData.K12_temp, 1));
        json += "},";
        
        // Voltages
        json += "\"voltages\":{";
        json += "\"K1\":" + (isnan(calibratedData.K1_voltage) ? "null" : String(calibratedData.K1_voltage, 2)) + ",\"K2\":" + (isnan(calibratedData.K2_voltage) ? "null" : String(calibratedData.K2_voltage, 2)) + ",\"K3\":" + (isnan(calibratedData.K3_voltage) ? "null" : String(calibratedData.K3_voltage, 2)) + ",\"K4\":" + (isnan(calibratedData.K4_voltage) ? "null" : String(calibratedData.K4_voltage, 2)) + ",\"K5\":" + (isnan(calibratedData.K5_voltage) ? "null" : String(calibratedData.K5_voltage, 2));
        json += ",\"K6\":" + (isnan(calibratedData.K6_voltage) ? "null" : String(calibratedData.K6_voltage, 2)) + ",\"K7\":" + (isnan(calibratedData.K7_voltage) ? "null" : String(calibratedData.K7_voltage, 2)) + ",\"K8\":" + (isnan(calibratedData.K8_voltage) ? "null" : String(calibratedData.K8_voltage, 2)) + ",\"K9\":" + (isnan(calibratedData.K9_voltage) ? "null" : String(calibratedData.K9_voltage, 2)) + ",\"K12\":" + (isnan(calibratedData.K12_voltage) ? "null" : String(calibratedData.K12_voltage, 2));
        json += "},";
        
        // Gases (ug/m3)
        json += "\"gases_ugm3\":{";
        json += "\"CO\":" + (isnan(calibratedData.CO) ? "null" : String(calibratedData.CO, 1)) + ",\"NO\":" + (isnan(calibratedData.NO) ? "null" : String(calibratedData.NO, 1)) + ",\"NO2\":" + (isnan(calibratedData.NO2) ? "null" : String(calibratedData.NO2, 1)) + ",\"O3\":" + (isnan(calibratedData.O3) ? "null" : String(calibratedData.O3, 1)) + ",\"SO2\":" + (isnan(calibratedData.SO2) ? "null" : String(calibratedData.SO2, 1)) + ",\"H2S\":" + (isnan(calibratedData.H2S) ? "null" : String(calibratedData.H2S, 1)) + ",\"NH3\":" + (isnan(calibratedData.NH3) ? "null" : String(calibratedData.NH3, 1));
        json += "},";
        
        // Gases (ppb)
        json += "\"gases_ppb\":{";
        json += "\"CO\":" + (isnan(calibratedData.CO_ppb) ? "null" : String(calibratedData.CO_ppb, 1)) + ",\"NO\":" + (isnan(calibratedData.NO_ppb) ? "null" : String(calibratedData.NO_ppb, 1)) + ",\"NO2\":" + (isnan(calibratedData.NO2_ppb) ? "null" : String(calibratedData.NO2_ppb, 1)) + ",\"O3\":" + (isnan(calibratedData.O3_ppb) ? "null" : String(calibratedData.O3_ppb, 1)) + ",\"SO2\":" + (isnan(calibratedData.SO2_ppb) ? "null" : String(calibratedData.SO2_ppb, 1)) + ",\"H2S\":" + (isnan(calibratedData.H2S_ppb) ? "null" : String(calibratedData.H2S_ppb, 1)) + ",\"NH3\":" + (isnan(calibratedData.NH3_ppb) ? "null" : String(calibratedData.NH3_ppb, 1));
        json += "},";
        
        // TGS sensors
        json += "\"tgs\":{";
        json += "\"TGS02\":" + (isnan(calibratedData.TGS02) ? "null" : String(calibratedData.TGS02, 3)) + ",\"TGS03\":" + (isnan(calibratedData.TGS03) ? "null" : String(calibratedData.TGS03, 3)) + ",\"TGS12\":" + (isnan(calibratedData.TGS12) ? "null" : String(calibratedData.TGS12, 3));
        json += ",\"TGS02_ohm\":" + (isnan(calibratedData.TGS02_ohm) ? "null" : String(calibratedData.TGS02_ohm, 0)) + ",\"TGS03_ohm\":" + (isnan(calibratedData.TGS03_ohm) ? "null" : String(calibratedData.TGS03_ohm, 0)) + ",\"TGS12_ohm\":" + (isnan(calibratedData.TGS12_ohm) ? "null" : String(calibratedData.TGS12_ohm, 0));
        json += "},";
        
        // Special sensors
        json += "\"special\":{";
        json += "\"HCHO_ppb\":" + (isnan(calibratedData.HCHO) ? "null" : String(calibratedData.HCHO, 1)) + ",\"PID\":" + (isnan(calibratedData.PID) ? "null" : String(calibratedData.PID, 3)) + ",\"PID_mV\":" + (isnan(calibratedData.PID_mV) ? "null" : String(calibratedData.PID_mV, 2));
        json += "}";
    }
    json += "}";
    json += "}";
    return json;
}

void wsBroadcastTask(void *parameter) {
    for (;;) {
        // Skip broadcast if memory is low
        if (ESP.getFreeHeap() > 20000) {
            String json = getAllSensorJson();
            if (json.indexOf("error") == -1) { // Only send if no error
                ws.textAll(json);
            }
        } else {
            safePrintln("Skipping WebSocket broadcast - low memory: " + String(ESP.getFreeHeap()));
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS); // 10s
    }
}

void timeCheckTask(void *parameter) {
    for (;;) {
        if (WiFi.status() == WL_CONNECTED && !timeInitialized) {
            // Sprawdz czy czas zostal zsynchronizowany
            time_t now = time(nullptr);
            if (now > 8 * 3600 * 2) { // > year 1970
                timeInitialized = true;
                safePrintln("Time synchronized: " + getFormattedTime() + " " + getFormattedDate());
            }
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS); // Check every 10 seconds
    }
}

void WiFiReconnectTask(void *parameter) {
    for (;;) {
        if (WiFi.status() != WL_CONNECTED) {
            safePrintln("WiFi disconnected, attempting to reconnect...");
            WiFi.disconnect();
            delay(1000);
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            
            int retryCount = 0;
            while (WiFi.status() != WL_CONNECTED && retryCount < 20) {
                retryCount++;
                delay(1000);
                safePrint("Reconnecting to WiFi... attempt ");
                safePrintln(String(retryCount));
            }
            
            if (WiFi.status() == WL_CONNECTED) {
                safePrintln("WiFi reconnected successfully");
                safePrint("New IP Address: ");
                safePrintln(WiFi.localIP().toString());
                lastConnectionTime = millis();
                
                // Restart time sync after reconnection
                configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");
                timeInitialized = false;
            } else {
                safePrintln("WiFi reconnection failed");
            }
        }
        vTaskDelay(30000 / portTICK_PERIOD_MS); // Check every 30 seconds
    }
}

void initializeWiFi() {
    if (!config.enableWiFi) return;
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int retryCount = 0;
    while (WiFi.status() != WL_CONNECTED && retryCount < 10) {
        retryCount++;
        delay(1000);
        safePrintln("Connecting to WiFi...");
    }
    if (WiFi.status() == WL_CONNECTED) {
        safePrintln("Connected to WiFi");
        safePrint("IP Address: ");
        safePrintln(WiFi.localIP().toString());
        lastConnectionTime = millis();
        
        // Initialize time with Poland timezone (UTC+1/UTC+2 with DST)
        configTime(3600, 3600, "pool.ntp.org", "time.nist.gov"); // UTC+1 base, 1h DST offset
        safePrintln("NTP time synchronization started...");
    } else {
        safePrintln("WiFi connection failed");
    }
}

void initializeWebServer() {
    if (!config.enableWebServer || !config.enableWiFi) return;
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", update_html);
    });
    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", dashboard_html);
    });
    server.on("/charts", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", charts_html);
    });
    server.on("/api/history", HTTP_GET, [](AsyncWebServerRequest *request) {
        String sensor = "i2c";  // Default to i2c instead of "all"
        String timeRange = "1h";
        
        if (request->hasParam("sensor")) {
            sensor = request->getParam("sensor")->value();
        }
        if (request->hasParam("timeRange")) {
            timeRange = request->getParam("timeRange")->value();
        }
        
        // Get current time for range calculation
        unsigned long currentTime = millis();
        unsigned long fromTime = 0;
        
        if (timeRange == "1h") {
            unsigned long range = 60 * 60 * 1000; // 1 hour
            fromTime = (currentTime > range) ? (currentTime - range) : 0;
        } else if (timeRange == "6h") {
            unsigned long range = 6 * 60 * 60 * 1000; // 6 hours
            fromTime = (currentTime > range) ? (currentTime - range) : 0;
        } else if (timeRange == "24h") {
            unsigned long range = 24 * 60 * 60 * 1000; // 24 hours
            fromTime = (currentTime > range) ? (currentTime - range) : 0;
        }
        
        // Ensure fromTime is never before system start
        if (fromTime == 0 || fromTime > currentTime) {
            fromTime = 0; // Get all data from system start
        }
        
        String jsonResponse;
        size_t dataCount = getHistoricalData(sensor, timeRange, jsonResponse, fromTime, currentTime);
        
        request->send(200, "application/json", jsonResponse);
    });
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            safePrint("Update Start: " + filename);
            if (!Update.begin()) {
                Update.printError(Serial);
            }
        }
        if (!Update.hasError()) {
            if (Update.write(data, len) != len) {
                Update.printError(Serial);
            }
        }
        if (final) {
            if (Update.end(true)) {
                safePrint("Update Success: " + String(index + len) + "B");
                safePrintln("Update complete! Rebooting...");
                request->send(200, "text/plain", "Update complete! Rebooting...\n");
                delay(100);
                ESP.restart();
            } else {
                Update.printError(Serial);
            }
        }
    });
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.begin();
    safePrintln("Web server started");
    xTaskCreatePinnedToCore(wsBroadcastTask, "wsBroadcastTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(WiFiReconnectTask, "wifiReconnectTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(timeCheckTask, "timeCheckTask", 2048, NULL, 1, NULL, 0);
}

 