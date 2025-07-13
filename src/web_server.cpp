#include <web_server.h>
#include <html.h>
#include <sensors.h>
#include <modbus_handler.h>
#include <i2c_sensors.h>

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

// Forward declarations for sensor functions
void setMCP3424Debug(bool enabled);

// Global objects
AsyncWebServer server(80);
AsyncEventSource events("/events");

// Global variables
unsigned long lastConnectionTime = 0;
bool autoReset = true;

// External configuration
extern FeatureConfig config;

// External sensor data and status flags
extern SolarData solarData;
extern OPCN3 myOPCN3;
extern HistogramData opcn3Data;
extern I2CSensorData i2cSensorData;
extern MCP3424Data mcp3424Data;
extern ADS1110Data ads1110Data;
extern INA219Data ina219Data;
extern SPS30Data sps30Data;
extern IPSSensorData ipsSensorData;

extern bool solarSensorStatus;
extern bool opcn3SensorStatus;
extern bool i2cSensorStatus;
extern bool sps30SensorStatus;
extern bool mcp3424SensorStatus;
extern bool ads1110SensorStatus;
extern bool ina219SensorStatus;
extern bool ipsSensorStatus;

void initializeWiFi() {
    if (!config.enableWiFi) return;
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // DHCP enabled by default - no manual IP configuration needed

    
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
    } else {
        safePrintln("WiFi connection failed");
    }
}

void initializeWebServer() {
    if (!config.enableWebServer || !config.enableWiFi) return;
    
    // Initialize WebSerial
    WebSerial.begin(&server);
    WebSerial.msgCallback(recvMsg);
    
    // Main page - Update firmware
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", update_html);
    });
    
    // Dashboard page - Live sensor data
    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", dashboard_html);
    });
    
    // API endpoints
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"t\":" + String(millis() / 1000) + ",";
        json += "\"solarSensor\":" + String(solarSensorStatus ? "true" : "false") + ",";
        json += "\"opcn3Sensor\":" + String(opcn3SensorStatus ? "true" : "false") + ",";
        json += "\"i2cSensor\":" + String(i2cSensorStatus ? "true" : "false") + ",";
        json += "\"autoReset\":" + String(config.autoReset ? "true" : "false") + ",";
        json += "\"uptime\":" + String(millis() / 1000) + ",";
        json += "\"freeHeap\":" + String(ESP.getFreeHeap());
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // Solar sensor endpoint with timestamp
    server.on("/api/data/solar", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"t\":" + String(millis() / 1000) + ",";
        json += "\"valid\":" + String(solarSensorStatus && solarData.valid ? "true" : "false");
        if (solarSensorStatus && solarData.valid) {
            json += ",\"PID\":\"" + solarData.PID + "\"";
            json += ",\"V\":\"" + solarData.V + "\"";
            json += ",\"I\":\"" + solarData.I + "\"";
            json += ",\"VPV\":\"" + solarData.VPV + "\"";
            json += ",\"PPV\":\"" + solarData.PPV + "\"";
            json += ",\"CS\":\"" + solarData.CS + "\"";
            json += ",\"LOAD\":\"" + solarData.LOAD + "\"";
        }
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // OPCN3 particle sensor endpoint with timestamp
    server.on("/api/data/opcn3", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"t\":" + String(millis() / 1000) + ",";
        json += "\"valid\":" + String(opcn3SensorStatus && opcn3Data.valid ? "true" : "false");
        if (opcn3SensorStatus && opcn3Data.valid) {
            json += ",\"temperature\":\"" + String(opcn3Data.getTempC()) + "\"";
            json += ",\"humidity\":\"" + String(opcn3Data.getHumidity()) + "\"";
            json += ",\"PM1\":\"" + String(opcn3Data.pm1) + "\"";
            json += ",\"PM25\":\"" + String(opcn3Data.pm2_5) + "\"";
            json += ",\"PM10\":\"" + String(opcn3Data.pm10) + "\"";
        }
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // SPS30 particle sensor endpoint with timestamp
    server.on("/api/data/sps30", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Debug output
        safePrint("API SPS30 - Status: ");
        safePrint(sps30SensorStatus ? "true" : "false");
        safePrint(", Valid: ");
        safePrint(sps30Data.valid ? "true" : "false");
        safePrint(", Config enabled: ");
        safePrintln(config.enableSPS30 ? "true" : "false");
        
        String json = "{";
        json += "\"t\":" + String(millis() / 1000) + ",";
        json += "\"valid\":" + String(sps30SensorStatus && sps30Data.valid ? "true" : "false") + ",";
        json += "\"debug\":{";
        json += "\"status\":" + String(sps30SensorStatus ? "true" : "false") + ",";
        json += "\"dataValid\":" + String(sps30Data.valid ? "true" : "false") + ",";
        json += "\"configEnabled\":" + String(config.enableSPS30 ? "true" : "false") + ",";
        json += "\"dataAge\":" + String((millis() - sps30Data.lastUpdate) / 1000);
        json += "}";
        if (sps30SensorStatus && sps30Data.valid) {
            json += ",\"PM1\":\"" + String(sps30Data.pm1_0, 1) + "\"";
            json += ",\"PM25\":\"" + String(sps30Data.pm2_5, 1) + "\"";
            json += ",\"PM4\":\"" + String(sps30Data.pm4_0, 1) + "\"";
            json += ",\"PM10\":\"" + String(sps30Data.pm10, 1) + "\"";
            json += ",\"NC05\":\"" + String(sps30Data.nc0_5, 1) + "\"";
            json += ",\"NC1\":\"" + String(sps30Data.nc1_0, 1) + "\"";
            json += ",\"NC25\":\"" + String(sps30Data.nc2_5, 1) + "\"";
            json += ",\"NC4\":\"" + String(sps30Data.nc4_0, 1) + "\"";
            json += ",\"NC10\":\"" + String(sps30Data.nc10, 1) + "\"";
            json += ",\"TPS\":\"" + String(sps30Data.typical_particle_size, 1) + "\"";
        }
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // Environmental I2C sensors endpoint with timestamp
    server.on("/api/data/env", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"t\":" + String(millis() / 1000) + ",";
        json += "\"valid\":" + String(i2cSensorStatus && i2cSensorData.valid ? "true" : "false");
        if (i2cSensorStatus && i2cSensorData.valid) {
            json += ",\"sensorType\":" + String((int)i2cSensorData.type);
            json += ",\"temperature\":\"" + String(i2cSensorData.temperature) + "\"";
            json += ",\"humidity\":\"" + String(i2cSensorData.humidity) + "\"";
            json += ",\"pressure\":\"" + String(i2cSensorData.pressure) + "\"";
            json += ",\"CO2\":\"" + String(i2cSensorData.co2) + "\"";
        }
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // MCP3424 ADC endpoint with timestamp
    server.on("/api/data/mcp3424", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Debug output
        safePrint("API MCP3424 - Status: ");
        safePrint(mcp3424SensorStatus ? "true" : "false");
        safePrint(", DeviceCount: ");
        safePrint(String(mcp3424Data.deviceCount));
        safePrint(", Config enabled: ");
        safePrintln(config.enableMCP3424 ? "true" : "false");
        
        String json = "{";
        json += "\"t\":" + String(millis() / 1000) + ",";
        json += "\"valid\":" + String(mcp3424SensorStatus ? "true" : "false") + ",";
        json += "\"deviceCount\":" + String(mcp3424Data.deviceCount) + ",";
        json += "\"debug\":{";
        json += "\"status\":" + String(mcp3424SensorStatus ? "true" : "false") + ",";
        json += "\"configEnabled\":" + String(config.enableMCP3424 ? "true" : "false") + ",";
        json += "\"i2cStatus\":" + String(i2cSensorStatus ? "true" : "false");
        json += "},";
        json += "\"devices\":[";
        for (uint8_t device = 0; device < mcp3424Data.deviceCount; device++) {
            json += "{";
            json += "\"address\":\"0x" + String(mcp3424Data.addresses[device], HEX) + "\",";
            json += "\"valid\":" + String(mcp3424Data.valid[device] ? "true" : "false") + ",";
            json += "\"CH1\":\"" + String(mcp3424Data.channels[device][0], 6) + "\",";
            json += "\"CH2\":\"" + String(mcp3424Data.channels[device][1], 6) + "\",";
            json += "\"CH3\":\"" + String(mcp3424Data.channels[device][2], 6) + "\",";
            json += "\"CH4\":\"" + String(mcp3424Data.channels[device][3], 6) + "\"";
            json += "}";
            if (device < mcp3424Data.deviceCount - 1) json += ",";
        }
        json += "]";
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // ADS1110 ADC endpoint with timestamp
    server.on("/api/data/ads1110", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"t\":" + String(millis() / 1000) + ",";
        json += "\"valid\":" + String(ads1110SensorStatus && ads1110Data.valid ? "true" : "false");
        if (ads1110SensorStatus && ads1110Data.valid) {
            json += ",\"voltage\":\"" + String(ads1110Data.voltage, 6) + "\"";
            json += ",\"dataRate\":" + String(ads1110Data.dataRate);
            json += ",\"gain\":" + String(ads1110Data.gain);
        }
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // INA219 power monitor endpoint with timestamp
    server.on("/api/data/power", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"t\":" + String(millis() / 1000) + ",";
        json += "\"valid\":" + String(ina219SensorStatus && ina219Data.valid ? "true" : "false");
        if (ina219SensorStatus && ina219Data.valid) {
            json += ",\"busVoltage\":\"" + String(ina219Data.busVoltage, 3) + "\"";
            json += ",\"shuntVoltage\":\"" + String(ina219Data.shuntVoltage, 2) + "\"";
            json += ",\"current\":\"" + String(ina219Data.current, 2) + "\"";
            json += ",\"power\":\"" + String(ina219Data.power, 2) + "\"";
        }
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // IPS sensor endpoint with timestamp
    server.on("/api/data/ips", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"t\":" + String(millis() / 1000) + ",";
        json += "\"valid\":" + String(ipsSensorStatus ? "true" : "false");
        if (ipsSensorStatus) {
            json += ",\"PC\":[";
            for (int i = 0; i < 7; i++) {
                json += "\"" + String(ipsSensorData.pc_values[i]) + "\"";
                if (i < 6) json += ",";
            }
            json += "],\"PM\":[";
            for (int i = 0; i < 7; i++) {
                json += "\"" + String(ipsSensorData.pm_values[i]) + "\"";
                if (i < 6) json += ",";
            }
            json += "]";
        }
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // Combined sensor data endpoint - all sensors in one response
    server.on("/api/data/all", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"t\":" + String(millis() / 1000) + ",";
        json += "\"uptime\":" + String(millis() / 1000) + ",";
        json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
        
        // Solar data
        json += "\"solar\":{";
        json += "\"valid\":" + String(solarSensorStatus && solarData.valid ? "true" : "false");
        if (solarSensorStatus && solarData.valid) {
            json += ",\"V\":\"" + solarData.V + "\",\"I\":\"" + solarData.I + "\",\"VPV\":\"" + solarData.VPV + "\",\"PPV\":\"" + solarData.PPV + "\"";
        }
        json += "},";
        
        // Particle sensors
        json += "\"particles\":{";
        json += "\"opcn3\":{\"valid\":" + String(opcn3SensorStatus && opcn3Data.valid ? "true" : "false");
        if (opcn3SensorStatus && opcn3Data.valid) {
            json += ",\"PM25\":\"" + String(opcn3Data.pm2_5) + "\",\"PM10\":\"" + String(opcn3Data.pm10) + "\"";
        }
        json += "},";
        json += "\"sps30\":{\"valid\":" + String(sps30SensorStatus && sps30Data.valid ? "true" : "false");
        if (sps30SensorStatus && sps30Data.valid) {
            json += ",\"PM25\":\"" + String(sps30Data.pm2_5, 1) + "\",\"PM10\":\"" + String(sps30Data.pm10, 1) + "\"";
        }
        json += "}},";
        
        // Environmental data
        json += "\"environmental\":{";
        json += "\"valid\":" + String(i2cSensorStatus && i2cSensorData.valid ? "true" : "false");
        if (i2cSensorStatus && i2cSensorData.valid) {
            json += ",\"temperature\":\"" + String(i2cSensorData.temperature) + "\",\"humidity\":\"" + String(i2cSensorData.humidity) + "\"";
            if (i2cSensorData.pressure > 0) {
                json += ",\"pressure\":\"" + String(i2cSensorData.pressure) + "\"";
            }
            if (i2cSensorData.co2 > 0) {
                json += ",\"CO2\":\"" + String(i2cSensorData.co2) + "\"";
            }
        }
        json += "}";
        
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // Legacy endpoints for backward compatibility
  
    
    server.on("/api/power", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (ina219SensorStatus && ina219Data.valid) {
            String json = "{";
            json += "\"valid\":true,";
            json += "\"busVoltage\":" + String(ina219Data.busVoltage, 3) + ",";
            json += "\"shuntVoltage\":" + String(ina219Data.shuntVoltage, 2) + ",";
            json += "\"current\":" + String(ina219Data.current, 2) + ",";
            json += "\"power\":" + String(ina219Data.power, 2);
            json += "}";
            request->send(200, "application/json", json);
        } else {
            request->send(200, "application/json", "{\"valid\":false}");
        }
    });
    
    // Configuration endpoints
    server.on("/api/config/solar", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("enable", true)) {
            String enableParam = request->getParam("enable", true)->value();
            config.enableSolarSensor = (enableParam == "true");
            request->send(200, "text/plain", "Solar sensor " + String(config.enableSolarSensor ? "enabled" : "disabled"));
        } else {
            request->send(400, "text/plain", "Missing enable parameter");
        }
    });
    
    server.on("/api/config/opcn3", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("enable", true)) {
            String enableParam = request->getParam("enable", true)->value();
            config.enableOPCN3Sensor = (enableParam == "true");
            request->send(200, "text/plain", "OPCN3 sensor " + String(config.enableOPCN3Sensor ? "enabled" : "disabled"));
        } else {
            request->send(400, "text/plain", "Missing enable parameter");
        }
    });
    
    server.on("/api/config/i2c", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("enable", true)) {
            String enableParam = request->getParam("enable", true)->value();
            config.enableI2CSensors = (enableParam == "true");
            if (config.enableI2CSensors) {
                initializeI2C();
            }
            request->send(200, "text/plain", "I2C sensors " + String(config.enableI2CSensors ? "enabled" : "disabled"));
        } else {
            request->send(400, "text/plain", "Missing enable parameter");
        }
    });
    
    server.on("/api/config/modbus", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("enable", true)) {
            String enableParam = request->getParam("enable", true)->value();
            config.enableModbus = (enableParam == "true");
            if (config.enableModbus) {
                initializeModbus();
            }
            request->send(200, "text/plain", "Modbus " + String(config.enableModbus ? "enabled" : "disabled"));
        } else {
            request->send(400, "text/plain", "Missing enable parameter");
        }
    });
    
    // Legacy endpoints for compatibility
    server.on("/toggleAutoReset", HTTP_POST, [](AsyncWebServerRequest *request) {
        config.autoReset = !config.autoReset;
        request->send(200, "text/plain", config.autoReset ? "true" : "false");
        safePrintln("Auto reset " + String(config.autoReset ? "enabled" : "disabled"));
    });
    
    server.on("/getAutoReset", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", config.autoReset ? "true" : "false");
    });
    
    // OTA Update endpoint
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
    
    // System control endpoints
    server.on("/api/system/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "System restarting...");
        delay(1000);
        ESP.restart();
    });
    
    // Add event source and start server
    server.addHandler(&events);
    server.begin();
    
    safePrintln("Web server started");
}

void WiFiReconnectTask(void *parameter) {
    for (;;) {
        if (WiFi.status() != WL_CONNECTED) {
            int retryCount = 0;
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            safePrintln("Reconnecting to WiFi...");
            
            while (WiFi.status() != WL_CONNECTED && retryCount < 10) {
                retryCount++;
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            
            if (WiFi.status() == WL_CONNECTED) {
                safePrintln("WiFi reconnected");
                safePrintln(WiFi.localIP().toString());
                lastConnectionTime = millis();
            } else if (millis() - lastConnectionTime > CONNECTION_TIMEOUT) {
                safePrintln("WiFi connection lost for more than 20 minutes. Rebooting...");
                delay(100);
                ESP.restart();
            }
        }
        
        // Send periodic events to web clients
        if (WiFi.status() == WL_CONNECTED) {
            String sensorDataJson = "{";
            sensorDataJson += "\"timestamp\":" + String(millis()) + ",";
            sensorDataJson += "\"uptime\":" + String(millis() / 1000) + ",";
            sensorDataJson += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
            sensorDataJson += "\"wifiSignal\":" + String(WiFi.RSSI()) + ",";
            
            // Sensor availability flags
            sensorDataJson += "\"sensorsEnabled\":{";
            sensorDataJson += "\"solar\":" + String(solarSensorStatus ? "true" : "false") + ",";
            sensorDataJson += "\"opcn3\":" + String(opcn3SensorStatus ? "true" : "false") + ",";
            sensorDataJson += "\"i2c\":" + String(i2cSensorStatus ? "true" : "false") + ",";
            sensorDataJson += "\"sps30\":" + String(sps30SensorStatus ? "true" : "false") + ",";
            sensorDataJson += "\"mcp3424\":" + String(mcp3424SensorStatus ? "true" : "false") + ",";
            sensorDataJson += "\"ads1110\":" + String(ads1110SensorStatus ? "true" : "false") + ",";
            sensorDataJson += "\"ina219\":" + String(ina219SensorStatus ? "true" : "false");
            sensorDataJson += "},";
            
            // Solar sensor data
            sensorDataJson += "\"solar\":{";
            if (solarSensorStatus && solarData.valid) {
                sensorDataJson += "\"valid\":true,";
                sensorDataJson += "\"V\":\"" + solarData.V + "\",";
                sensorDataJson += "\"I\":\"" + solarData.I + "\",";
                sensorDataJson += "\"VPV\":\"" + solarData.VPV + "\",";
                sensorDataJson += "\"PPV\":\"" + solarData.PPV + "\"";
            } else {
                sensorDataJson += "\"valid\":false";
            }
            sensorDataJson += "},";
            
            // OPCN3 sensor data
            sensorDataJson += "\"opcn3\":{";
            if (opcn3SensorStatus && opcn3Data.valid) {
                sensorDataJson += "\"valid\":true,";
                sensorDataJson += "\"temperature\":" + String(opcn3Data.getTempC()) + ",";
                sensorDataJson += "\"humidity\":" + String(opcn3Data.getHumidity()) + ",";
                sensorDataJson += "\"pm1\":" + String(opcn3Data.pm1) + ",";
                sensorDataJson += "\"pm2_5\":" + String(opcn3Data.pm2_5) + ",";
                sensorDataJson += "\"pm10\":" + String(opcn3Data.pm10);
            } else {
                sensorDataJson += "\"valid\":false";
            }
            sensorDataJson += "},";
            
            // I2C environmental sensor data
            sensorDataJson += "\"i2c\":{";
            if (i2cSensorStatus && i2cSensorData.valid) {
                sensorDataJson += "\"valid\":true,";
                sensorDataJson += "\"temperature\":" + String(i2cSensorData.temperature) + ",";
                sensorDataJson += "\"humidity\":" + String(i2cSensorData.humidity) + ",";
                sensorDataJson += "\"pressure\":" + String(i2cSensorData.pressure) + ",";
                sensorDataJson += "\"co2\":" + String(i2cSensorData.co2);
            } else {
                sensorDataJson += "\"valid\":false";
            }
            sensorDataJson += "},";
            
            // SPS30 particle sensor data
            sensorDataJson += "\"sps30\":{";
            if (sps30SensorStatus && sps30Data.valid) {
                sensorDataJson += "\"valid\":true,";
                sensorDataJson += "\"pm1_0\":" + String(sps30Data.pm1_0, 1) + ",";
                sensorDataJson += "\"pm2_5\":" + String(sps30Data.pm2_5, 1) + ",";
                sensorDataJson += "\"pm4_0\":" + String(sps30Data.pm4_0, 1) + ",";
                sensorDataJson += "\"pm10\":" + String(sps30Data.pm10, 1) + ",";
                sensorDataJson += "\"nc0_5\":" + String(sps30Data.nc0_5, 1) + ",";
                sensorDataJson += "\"nc1_0\":" + String(sps30Data.nc1_0, 1) + ",";
                sensorDataJson += "\"nc2_5\":" + String(sps30Data.nc2_5, 1) + ",";
                sensorDataJson += "\"nc4_0\":" + String(sps30Data.nc4_0, 1) + ",";
                sensorDataJson += "\"nc10\":" + String(sps30Data.nc10, 1) + ",";
                sensorDataJson += "\"typical_particle_size\":" + String(sps30Data.typical_particle_size, 1);
            } else {
                sensorDataJson += "\"valid\":false";
            }
            sensorDataJson += "},";
            
            // ADC sensor data with detailed device info
            sensorDataJson += "\"adc\":{";
            
            // MCP3424 with full device details
            sensorDataJson += "\"mcp3424\":{";
            sensorDataJson += "\"deviceCount\":" + String(mcp3424Data.deviceCount) + ",";
            sensorDataJson += "\"enabled\":" + String(mcp3424SensorStatus ? "true" : "false") + ",";
            sensorDataJson += "\"devices\":[";
            if (mcp3424SensorStatus) {
                for (uint8_t device = 0; device < mcp3424Data.deviceCount; device++) {
                    sensorDataJson += "{";
                    sensorDataJson += "\"address\":\"0x" + String(mcp3424Data.addresses[device], HEX) + "\",";
                    sensorDataJson += "\"valid\":" + String(mcp3424Data.valid[device] ? "true" : "false") + ",";
                    sensorDataJson += "\"channels\":{";
                    sensorDataJson += "\"ch1\":" + String(mcp3424Data.channels[device][0], 6) + ",";
                    sensorDataJson += "\"ch2\":" + String(mcp3424Data.channels[device][1], 6) + ",";
                    sensorDataJson += "\"ch3\":" + String(mcp3424Data.channels[device][2], 6) + ",";
                    sensorDataJson += "\"ch4\":" + String(mcp3424Data.channels[device][3], 6);
                    sensorDataJson += "}";
                    sensorDataJson += "}";
                    if (device < mcp3424Data.deviceCount - 1) sensorDataJson += ",";
                }
            }
            sensorDataJson += "]},";
            
            // ADS1110 data
            sensorDataJson += "\"ads1110\":{";
            sensorDataJson += "\"enabled\":" + String(ads1110SensorStatus ? "true" : "false") + ",";
            sensorDataJson += "\"valid\":" + String(ads1110SensorStatus && ads1110Data.valid ? "true" : "false") + ",";
            sensorDataJson += "\"voltage\":" + String(ads1110Data.voltage, 6) + ",";
            sensorDataJson += "\"dataRate\":" + String(ads1110Data.dataRate) + ",";
            sensorDataJson += "\"gain\":" + String(ads1110Data.gain);
            sensorDataJson += "}";
            sensorDataJson += "},";
            
            // Power monitor data
            sensorDataJson += "\"power\":{";
            if (ina219SensorStatus && ina219Data.valid) {
                sensorDataJson += "\"valid\":true,";
                sensorDataJson += "\"busVoltage\":" + String(ina219Data.busVoltage, 3) + ",";
                sensorDataJson += "\"shuntVoltage\":" + String(ina219Data.shuntVoltage, 2) + ",";
                sensorDataJson += "\"current\":" + String(ina219Data.current, 2) + ",";
                sensorDataJson += "\"power\":" + String(ina219Data.power, 2);
            } else {
                sensorDataJson += "\"valid\":false";
            }
            sensorDataJson += "}";
            
            sensorDataJson += "}";
            events.send(sensorDataJson.c_str(), "sensorData", millis());
        }
        
        vTaskDelay(10000 / portTICK_PERIOD_MS); // Check every 10 seconds
    }
}

void recvMsg(uint8_t *data, size_t len) {
    WebSerial.println("Received WebSerial Data...");
    safePrintln("Received WebSerial Data...");
    String message = "";
    for (int i = 0; i < len; i++) {
        message += char(data[i]);
    }
    
    // Process WebSerial commands
    message.trim();
    if (message.equals("STATUS")) {
        WebSerial.println("=== System Status ===");
        WebSerial.println("Solar Sensor: " + String(solarSensorStatus ? "OK" : "ERROR"));
        WebSerial.println("OPCN3 Sensor: " + String(opcn3SensorStatus ? "OK" : "ERROR"));
        WebSerial.println("I2C Overall: " + String(i2cSensorStatus ? "OK" : "ERROR"));
        WebSerial.println("  - SHT30: " + String(sht30SensorStatus ? "OK" : (config.enableSHT30 ? "ERROR" : "DISABLED")));
        WebSerial.println("  - BME280: " + String(bme280SensorStatus ? "OK" : (config.enableBME280 ? "ERROR" : "DISABLED")));
        WebSerial.println("  - SCD41: " + String(scd41SensorStatus ? "OK" : (config.enableSCD41 ? "ERROR" : "DISABLED")));
        WebSerial.println("  - SHT40: " + String(sht40SensorStatus ? "OK" : (config.enableSHT40 ? "ERROR" : "DISABLED")));
        WebSerial.println("  - MCP3424: " + String(mcp3424SensorStatus ? "OK" : (config.enableMCP3424 ? "ERROR" : "DISABLED")));
        WebSerial.println("  - ADS1110: " + String(ads1110SensorStatus ? "OK" : (config.enableADS1110 ? "ERROR" : "DISABLED")));
        WebSerial.println("  - INA219: " + String(ina219SensorStatus ? "OK" : (config.enableINA219 ? "ERROR" : "DISABLED")));
        WebSerial.println("IPS Sensor: " + String(ipsSensorStatus ? "OK" : (config.enableIPS ? "ERROR" : "DISABLED")));
        WebSerial.println("SPS30 Sensor: " + String(sps30SensorStatus ? "OK" : (config.enableSPS30 ? "ERROR" : "DISABLED")));
        WebSerial.println("WiFi: " + String(WiFi.status() == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED"));
        WebSerial.println("IP Address: " + String(WiFi.localIP().toString()));
        WebSerial.println("Auto Reset: " + String(config.autoReset ? "ENABLED" : "DISABLED"));
        if (config.enableModbus) {
            WebSerial.print("Modbus Activity: ");
            if (hasHadModbusActivity) {
                WebSerial.println("Last " + String((millis() - lastModbusActivity) / 1000) + " seconds ago");
            } else {
                WebSerial.println("Never detected");
            }
        }
        WebSerial.println("Free Heap: " + String(ESP.getFreeHeap()));
        WebSerial.println("Uptime: " + String(millis() / 1000) + " seconds");
    } else if (message.equals("SEND")) {
        WebSerial.println("=== Sensor Data ===");
        
        if (config.enableSolarSensor && solarSensorStatus) {
            WebSerial.print("Solar Controller: ");
            WebSerial.print("V="); WebSerial.print(solarData.V);
            WebSerial.print("V, I="); WebSerial.print(solarData.I);
            WebSerial.print("A, VPV="); WebSerial.print(solarData.VPV);
            WebSerial.print("V, PPV="); WebSerial.print(solarData.PPV);
            WebSerial.print("W, CS="); WebSerial.print(solarData.CS);
            WebSerial.print(", LOAD="); WebSerial.println(solarData.LOAD);
        }
        
        if (config.enableOPCN3Sensor && opcn3SensorStatus) {
            WebSerial.print("OPCN3: ");
            WebSerial.print("Temp="); WebSerial.print(String(opcn3Data.getTempC()));
            WebSerial.print("°C, Hum="); WebSerial.print(String(opcn3Data.getHumidity()));
            WebSerial.print(", PM1="); WebSerial.print(String(opcn3Data.pm1));
            WebSerial.print(", PM2.5="); WebSerial.print(String(opcn3Data.pm2_5));
            WebSerial.print(", PM10="); WebSerial.println(String(opcn3Data.pm10));
        }
        
        if (config.enableI2CSensors && i2cSensorStatus) {
            WebSerial.print("I2C Sensor: ");
            WebSerial.print("Temp="); WebSerial.print(String(i2cSensorData.temperature));
            WebSerial.print("°C, Hum="); WebSerial.print(String(i2cSensorData.humidity));
            WebSerial.println("%");
        }
        
        if (config.enableSHT40 && sht40SensorStatus) {
            WebSerial.print("SHT40: ");
            WebSerial.print("Temp="); WebSerial.print(String(sht40Data.temperature, 2));
            WebSerial.print("°C, Hum="); WebSerial.print(String(sht40Data.humidity, 2));
            WebSerial.print("%, Press="); WebSerial.print(String(sht40Data.pressure, 2));
            WebSerial.println(" hPa");
        }
        
        if (config.enableSPS30 && sps30SensorStatus) {
            WebSerial.print("SPS30: PM1.0="); WebSerial.print(String(sps30Data.pm1_0, 1));
            WebSerial.print(" PM2.5="); WebSerial.print(String(sps30Data.pm2_5, 1));
            WebSerial.print(" PM4.0="); WebSerial.print(String(sps30Data.pm4_0, 1));
            WebSerial.print(" PM10="); WebSerial.print(String(sps30Data.pm10, 1));
            WebSerial.print(" µg/m³, TPS="); WebSerial.print(String(sps30Data.typical_particle_size, 1));
            WebSerial.println("µm");
        }
        
        if (config.enableMCP3424 && mcp3424SensorStatus) {
            WebSerial.print("MCP3424 Devices: ");
            WebSerial.println(String(mcp3424Data.deviceCount));
            
            for (uint8_t device = 0; device < mcp3424Data.deviceCount; device++) {
                WebSerial.print("  Device ");
                WebSerial.print(String(device));
                WebSerial.print(" (0x");
                WebSerial.print(String(mcp3424Data.addresses[device], HEX));
                WebSerial.print("): ");
                
                if (mcp3424Data.valid[device]) {
                    for (int ch = 0; ch < 4; ch++) {
                        WebSerial.print("Ch"); WebSerial.print(String(ch+1)); WebSerial.print("=");
                        WebSerial.print(String(mcp3424Data.channels[device][ch], 4)); WebSerial.print("V");
                        if (ch < 3) WebSerial.print(", ");
                    }
                    WebSerial.println("");
                } else {
                    WebSerial.println("Invalid");
                }
            }
        }
        
        if (config.enableADS1110 && ads1110SensorStatus) {
            WebSerial.print("ADS1110: Voltage=");
            WebSerial.print(String(ads1110Data.voltage, 6));
            WebSerial.println("V");
        }
        
        if (config.enableINA219 && ina219SensorStatus) {
            WebSerial.print("INA219: Bus=");
            WebSerial.print(String(ina219Data.busVoltage, 3));
            WebSerial.print("V, Current=");
            WebSerial.print(String(ina219Data.current, 2));
            WebSerial.print("mA, Power=");
            WebSerial.print(String(ina219Data.power, 2));
            WebSerial.println("mW");
        }
        
        if (config.enableIPS && ipsSensorStatus) {
            WebSerial.print("IPS Sensor PC: ");
            for (int i = 0; i < 7; i++) {
                WebSerial.print(String(ipsSensorData.pc_values[i]));
                if (i < 6) WebSerial.print(",");
            }
            WebSerial.print(" PM: ");
            for (int i = 0; i < 7; i++) {
                WebSerial.print(String(ipsSensorData.pm_values[i]));
                if (i < 6) WebSerial.print(",");
            }
            WebSerial.println("");
        }
    } else if (message.equals("RESTART")) {
        WebSerial.println("Restarting system...");
        delay(1000);
        ESP.restart();
    } else if (message.equals("MCP3424_DEBUG_ON")) {
        setMCP3424Debug(true);
        WebSerial.println("MCP3424 debug enabled");
    } else if (message.equals("MCP3424_DEBUG_OFF")) {
        setMCP3424Debug(false);
        WebSerial.println("MCP3424 debug disabled");
    } else if (message.equals("ADC_STATUS")) {
        WebSerial.println("=== ADC Sensors Status ===");
        WebSerial.println("MCP3424: " + String(mcp3424SensorStatus ? "OK" : "ERROR"));
        WebSerial.println("MCP3424 Devices: " + String(mcp3424Data.deviceCount));
        WebSerial.println("ADS1110: " + String(ads1110SensorStatus ? "OK" : "ERROR"));
        WebSerial.println("INA219: " + String(ina219SensorStatus ? "OK" : "ERROR"));
        if (mcp3424SensorStatus) {
            for (uint8_t device = 0; device < mcp3424Data.deviceCount; device++) {
                WebSerial.println("MCP3424 Device " + String(device) + " (0x" + String(mcp3424Data.addresses[device], HEX) + "):");
                for (int ch = 0; ch < 4; ch++) {
                    WebSerial.println("  Ch" + String(ch+1) + ": " + String(mcp3424Data.channels[device][ch], 6) + "V");
                }
            }
        }
        if (ads1110SensorStatus) {
            WebSerial.println("ADS1110: " + String(ads1110Data.voltage, 6) + "V");
        }
        if (ina219SensorStatus) {
            WebSerial.println("INA219: " + String(ina219Data.current, 2) + "mA");
        }
    } else if (message.equals("MODBUS_STATUS")) {
        WebSerial.println("=== Modbus Status ===");
        WebSerial.println("Enabled: " + String(config.enableModbus ? "YES" : "NO"));
        WebSerial.println("Had Activity: " + String(hasHadModbusActivity ? "YES" : "NO"));
        WebSerial.println("Last Activity: " + String((millis() - lastModbusActivity) / 1000) + " seconds ago");
        WebSerial.println("Timeout: " + String(MODBUS_TIMEOUT / 1000) + " seconds");
        WebSerial.println("System Uptime: " + String(millis() / 1000) + " seconds");
    } else if (message.startsWith("CONFIG_")) {
        // Configuration commands
        if (message.equals("CONFIG_SOLAR_ON")) {
            config.enableSolarSensor = true;
            WebSerial.println("Solar sensor enabled");
        }
        else if (message.equals("CONFIG_SOLAR_OFF")) {
            config.enableSolarSensor = false;
            WebSerial.println("Solar sensor disabled");
        }
        else if (message.equals("CONFIG_OPCN3_ON")) {
            config.enableOPCN3Sensor = true;
            WebSerial.println("OPCN3 sensor enabled");
        }
        else if (message.equals("CONFIG_OPCN3_OFF")) {
            config.enableOPCN3Sensor = false;
            WebSerial.println("OPCN3 sensor disabled");
        }
        else if (message.equals("CONFIG_I2C_ON")) {
            config.enableI2CSensors = true;
            initializeI2C();
            WebSerial.println("I2C sensors enabled");
        }
        else if (message.equals("CONFIG_I2C_OFF")) {
            config.enableI2CSensors = false;
            WebSerial.println("I2C sensors disabled");
        }
        else if (message.equals("CONFIG_SHT30_ON")) {
            enableI2CSensor(SENSOR_SHT30);
            WebSerial.println("SHT30 sensor enabled");
        }
        else if (message.equals("CONFIG_SHT30_OFF")) {
            disableI2CSensor(SENSOR_SHT30);
            WebSerial.println("SHT30 sensor disabled");
        }
        else if (message.equals("CONFIG_BME280_ON")) {
            enableI2CSensor(SENSOR_BME280);
            WebSerial.println("BME280 sensor enabled");
        }
        else if (message.equals("CONFIG_BME280_OFF")) {
            disableI2CSensor(SENSOR_BME280);
            WebSerial.println("BME280 sensor disabled");
        }
        else if (message.equals("CONFIG_SCD41_ON")) {
            enableI2CSensor(SENSOR_SCD41);
            WebSerial.println("SCD41 sensor enabled");
        }
        else if (message.equals("CONFIG_SCD41_OFF")) {
            disableI2CSensor(SENSOR_SCD41);
            WebSerial.println("SCD41 sensor disabled");
        }
        else if (message.equals("RESET_SCD41")) {
            resetSCD41();
            WebSerial.println("SCD41 sensor reset");
        }
        else if (message.equals("CONFIG_SHT40_ON")) {
            enableI2CSensor(SENSOR_SHT40);
            WebSerial.println("SHT40 sensor enabled");
        }
        else if (message.equals("CONFIG_SHT40_OFF")) {
            disableI2CSensor(SENSOR_SHT40);
            WebSerial.println("SHT40 sensor disabled");
        }
        else if (message.equals("CONFIG_SPS30_ON")) {
            enableI2CSensor(SENSOR_SPS30);
            WebSerial.println("SPS30 sensor enabled");
        }
        else if (message.equals("CONFIG_SPS30_OFF")) {
            disableI2CSensor(SENSOR_SPS30);
            WebSerial.println("SPS30 sensor disabled");
        }
        else if (message.equals("CONFIG_MCP3424_ON")) {
            enableI2CSensor(SENSOR_MCP3424);
            WebSerial.println("MCP3424 sensor enabled");
        }
        else if (message.equals("CONFIG_MCP3424_OFF")) {
            disableI2CSensor(SENSOR_MCP3424);
            WebSerial.println("MCP3424 sensor disabled");
        }
        else if (message.equals("CONFIG_ADS1110_ON")) {
            enableI2CSensor(SENSOR_ADS1110);
            WebSerial.println("ADS1110 sensor enabled");
        }
        else if (message.equals("CONFIG_ADS1110_OFF")) {
            disableI2CSensor(SENSOR_ADS1110);
            WebSerial.println("ADS1110 sensor disabled");
        }
        else if (message.equals("CONFIG_INA219_ON")) {
            enableI2CSensor(SENSOR_INA219);
            WebSerial.println("INA219 sensor enabled");
        }
        else if (message.equals("CONFIG_INA219_OFF")) {
            disableI2CSensor(SENSOR_INA219);
            WebSerial.println("INA219 sensor disabled");
        }
        else if (message.startsWith("CONFIG_ADS1110_")) {
            // Parse ADS1110 configuration: CONFIG_ADS1110_RATE_GAIN (e.g., CONFIG_ADS1110_60_4)
            String params = message.substring(15); // Remove "CONFIG_ADS1110_"
            int separatorIndex = params.indexOf('_');
            if (separatorIndex > 0) {
                uint8_t dataRate = params.substring(0, separatorIndex).toInt();
                uint8_t gain = params.substring(separatorIndex + 1).toInt();
                configureADS1110(dataRate, gain);
                WebSerial.println("ADS1110 configured - Rate: " + String(dataRate) + " SPS, Gain: " + String(gain) + "x");
            } else {
                WebSerial.println("Invalid ADS1110 config format. Use: CONFIG_ADS1110_RATE_GAIN (e.g., CONFIG_ADS1110_60_4)");
            }
        }
        else if (message.equals("CONFIG_IPS_ON")) {
            config.enableIPS = true;
            WebSerial.println("IPS sensor enabled");
        }
        else if (message.equals("CONFIG_IPS_OFF")) {
            config.enableIPS = false;
            WebSerial.println("IPS sensor disabled");
        }
        else if (message.equals("CONFIG_MODBUS_ON")) {
            config.enableModbus = true;
            initializeModbus();
            WebSerial.println("Modbus enabled");
        }
        else if (message.equals("CONFIG_MODBUS_OFF")) {
            config.enableModbus = false;
            WebSerial.println("Modbus disabled");
        }
        else if (message.equals("CONFIG_AUTO_RESET_ON")) {
            config.autoReset = true;
            WebSerial.println("Auto reset enabled");
        }
        else if (message.equals("CONFIG_AUTO_RESET_OFF")) {
            config.autoReset = false;
            WebSerial.println("Auto reset disabled");
        }
        else {
            WebSerial.println("Unknown CONFIG command: " + message);
        }
    } 
    else if (message.equals("MP3424_Status")) {
        WebSerial.println("=== MCP3424 Status ===");
        WebSerial.println("MCP3424: " + String(mcp3424SensorStatus ? "OK" : "ERROR"));
        WebSerial.println("MCP3424 Devices: " + String(mcp3424Data.deviceCount));
      //  WebSerial.println("MCP3424 Debug: " + String(mcp3424Data.debug ? "ON" : "OFF"));
        //write adresses of devices
        for (int i = 0; i < mcp3424Data.deviceCount; i++) {
            WebSerial.println("MCP3424 Device " + String(i) + " Address: " + String(mcp3424Data.addresses[i], HEX));
        }
    }
    else if (message.equals("HELP") || message.equals("help")) {
        WebSerial.println("=== Available Commands ===");
        WebSerial.println("STATUS - system status");
        WebSerial.println("SEND - send all sensor data");  
        WebSerial.println("HELP - show this help");
        WebSerial.println("RESTART - restart system");
        WebSerial.println("MODBUS_STATUS - modbus status");
        WebSerial.println("ADC_STATUS - ADC sensors status");
        WebSerial.println("Use CONFIG_[SENSOR]_ON/OFF to enable/disable sensors");
    }
    else {
        WebSerial.println("Unknown command: " + message);
        WebSerial.println("Type 'HELP' for available commands");
    }
} 