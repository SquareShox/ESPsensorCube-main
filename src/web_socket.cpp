#include <web_socket.h>
#include <sensors.h>
#include <history.h>
#include <calib.h>
#include <config.h>
#include <fan.h>
#include <network_config.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <network_config.h>
#include <esp_task_wdt.h>

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

// Struktura do sledzenia klientow WebSocket
struct WebSocketClientInfo {
    AsyncWebSocketClient* client;
    unsigned long lastPingTime;
    unsigned long lastPongTime;
    bool pingSent;
    bool isActive;
};

#define MAX_WS_CLIENTS 5
WebSocketClientInfo wsClients[MAX_WS_CLIENTS];
int wsClientCount = 0;

// Zmienne globalne dla zarzadzania pamiecia WebSocket
unsigned long lastPingTime = 0;
unsigned long lastCleanupTime = 0;
unsigned long lastMemoryCheck = 0;
unsigned long lastNativePingTime = 0;

// External WebSocket object
extern AsyncWebSocket ws;

// External sensor data
extern SolarData solarData;
extern I2CSensorData i2cSensorData;
extern SPS30Data sps30Data;
extern IPSSensorData ipsSensorData;
extern MCP3424Data mcp3424Data;
extern ADS1110Data ads1110Data;
extern INA219Data ina219Data;
extern SHT40Data sht40Data;
extern CalibratedSensorData calibratedData;
extern HCHOData hchoData;

// External configuration
extern FeatureConfig config;
extern CalibrationConfig calibConfig;

// External sensor status flags
extern bool solarSensorStatus;
extern bool opcn3SensorStatus;
extern bool i2cSensorStatus;
extern bool sps30SensorStatus;
extern bool sht40SensorStatus;
extern bool scd41SensorStatus;
extern bool mcp3424SensorStatus;
extern bool ads1110SensorStatus;
extern bool ina219SensorStatus;
extern bool hchoSensorStatus;
extern bool ipsSensorStatus;

// External averaging functions
extern SolarData getSolarFastAverage();
extern SolarData getSolarSlowAverage();
extern I2CSensorData getI2CFastAverage();
extern I2CSensorData getI2CSlowAverage();
extern SPS30Data getSPS30FastAverage();
extern SPS30Data getSPS30SlowAverage();
extern IPSSensorData getIPSFastAverage();
extern IPSSensorData getIPSSlowAverage();
extern MCP3424Data getMCP3424FastAverage();
extern MCP3424Data getMCP3424SlowAverage();
extern ADS1110Data getADS1110FastAverage();
extern ADS1110Data getADS1110SlowAverage();
extern INA219Data getINA219FastAverage();
extern INA219Data getINA219SlowAverage();
extern SHT40Data getSHT40FastAverage();
extern SHT40Data getSHT40SlowAverage();
extern CalibratedSensorData getCalibratedFastAverage();
extern CalibratedSensorData getCalibratedSlowAverage();
extern HCHOData getHCHOFastAverage();
extern HCHOData getHCHOSlowAverage();

// WebSocket command handlers
void handleGetStatus(AsyncWebSocketClient* client, JsonDocument& doc) {
    DynamicJsonDocument response(2048);
    response["cmd"] = "status";
    response["t"] = millis();
    response["uptime"] = millis() / 1000;
    response["freeHeap"] = ESP.getFreeHeap();
    response["freePsram"] = ESP.getFreePsram();
    response["wifiRSSI"] = WiFi.RSSI();
    response["wifiConnected"] = WiFi.status() == WL_CONNECTED;
    
    // Add date and time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
        response["time"] = String(timeStr);
        
        char dateStr[20];
        strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
        response["date"] = String(dateStr);
    } else {
        response["time"] = "00:00:00";
        response["date"] = "01/01/2024";
    }
    
    // System data
    JsonObject data = response.createNestedObject("data");
    
    // Sensor status
    JsonObject sensors = data.createNestedObject("sensors");
    sensors["solar"] = solarSensorStatus;
    sensors["opcn3"] = opcn3SensorStatus;
    sensors["i2c"] = i2cSensorStatus;
    sensors["sps30"] = sps30SensorStatus;
    sensors["sht40"] = sht40SensorStatus;
    sensors["scd41"] = scd41SensorStatus;
    sensors["mcp3424"] = mcp3424SensorStatus;
    sensors["ads1110"] = ads1110SensorStatus;
    sensors["ina219"] = ina219SensorStatus;
    sensors["hcho"] = hchoSensorStatus;
    sensors["ips"] = ipsSensorStatus;
    
    // Configuration
    JsonObject configObj = data.createNestedObject("config");
    configObj["enableWiFi"] = config.enableWiFi;
    configObj["enableWebServer"] = config.enableWebServer;
    configObj["enableHistory"] = config.enableHistory;
    configObj["enableModbus"] = config.enableModbus;
    configObj["lowPowerMode"] = config.lowPowerMode;
    
    response["success"] = true;
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleGetSensorData(AsyncWebSocketClient* client, JsonDocument& doc) {
    String sensorType = doc["sensor"] | "";
    DynamicJsonDocument response(4096);
    response["cmd"] = "sensorData";
    response["sensor"] = sensorType;
    response["t"] = millis();
    
    // Add date and time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
        response["time"] = String(timeStr);
        
        char dateStr[20];
        strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
        response["date"] = String(dateStr);
    } else {
        response["time"] = "00:00:00";
        response["date"] = "01/01/2024";
    }
    
    if (sensorType == "all") {
        // Wszystkie dane sensorów
        JsonObject data = response.createNestedObject("data");
        
        // Solar
        // if (solarSensorStatus && solarData.valid) {
        //     JsonObject solar = data.createNestedObject("solar");
        //     solar["V"] = solarData.V;
        //     solar["I"] = solarData.I;
        //     solar["VPV"] = solarData.VPV;
        //     solar["PPV"] = solarData.PPV;
        //     solar["valid"] = true;
        // }
        
        // SHT40
        if (sht40SensorStatus && sht40Data.valid) {
            JsonObject sht40 = data.createNestedObject("sht40");
            sht40["temperature"] = sht40Data.temperature;
            sht40["humidity"] = sht40Data.humidity;
            sht40["pressure"] = sht40Data.pressure;
            sht40["valid"] = true;
        }
        
        // SPS30
        if (sps30SensorStatus && sps30Data.valid) {
            JsonObject sps30 = data.createNestedObject("sps30");
            sps30["PM1"] = sps30Data.pm1_0;
            sps30["PM25"] = sps30Data.pm2_5;
            sps30["PM4"] = sps30Data.pm4_0;
            sps30["PM10"] = sps30Data.pm10;
            sps30["NC05"] = sps30Data.nc0_5;
            sps30["NC1"] = sps30Data.nc1_0;
            sps30["NC25"] = sps30Data.nc2_5;
            sps30["NC4"] = sps30Data.nc4_0;
            sps30["NC10"] = sps30Data.nc10;
            sps30["TPS"] = sps30Data.typical_particle_size;
            sps30["valid"] = true;
        }
        
        // I2C (SCD41 CO2)
        if (scd41SensorStatus && i2cSensorData.valid && i2cSensorData.type == SENSOR_SCD41) {
            JsonObject i2c = data.createNestedObject("i2c");
            i2c["temperature"] = i2cSensorData.temperature;
            i2c["humidity"] = i2cSensorData.humidity;
            i2c["pressure"] = i2cSensorData.pressure;
            i2c["co2"] = i2cSensorData.co2;
            i2c["valid"] = true;
        }
        
        // MCP3424
        if (mcp3424SensorStatus && mcp3424Data.deviceCount > 0) {
            JsonObject mcp3424 = data.createNestedObject("mcp3424");
            mcp3424["deviceCount"] = mcp3424Data.deviceCount;
            JsonArray devices = mcp3424.createNestedArray("devices");
            for (uint8_t i = 0; i < mcp3424Data.deviceCount; i++) {
                JsonObject device = devices.createNestedObject();
                device["address"] = "0x" + String(mcp3424Data.addresses[i], HEX);
                device["valid"] = mcp3424Data.valid[i];
                device["resolution"] = mcp3424Data.resolution;
                device["gain"] = mcp3424Data.gain;
                JsonArray channels = device.createNestedArray("channels");
                for (int ch = 0; ch < 4; ch++) {
                    channels.add(mcp3424Data.channels[i][ch]);
                }
            }
        }
        
        // ADS1110
        if (ads1110SensorStatus && ads1110Data.valid) {
            JsonObject ads1110 = data.createNestedObject("ads1110");
            ads1110["voltage"] = ads1110Data.voltage;
            ads1110["dataRate"] = ads1110Data.dataRate;
            ads1110["gain"] = ads1110Data.gain;
            ads1110["valid"] = true;
        }
        
        // INA219 (Power)
        if (ina219SensorStatus && ina219Data.valid) {
            JsonObject power = data.createNestedObject("power");
            power["busVoltage"] = ina219Data.busVoltage;
            power["shuntVoltage"] = ina219Data.shuntVoltage;
            power["current"] = ina219Data.current;
            power["power"] = ina219Data.power;
            power["valid"] = true;
        }
        
        // HCHO
        if (hchoSensorStatus && hchoData.valid) {
            JsonObject hcho = data.createNestedObject("hcho");
            hcho["hcho_mg"] = hchoData.hcho;
            hcho["hcho_ppb"] = hchoData.hcho_ppb;
            hcho["valid"] = true;
        }
        
        // IPS
        if (ipsSensorStatus && ipsSensorData.valid) {
            JsonObject ips = data.createNestedObject("ips");
            JsonArray pc = ips.createNestedArray("pc");
            JsonArray pm = ips.createNestedArray("pm");
            for (int i = 0; i < 7; i++) {
                pc.add(ipsSensorData.pc_values[i]);
                pm.add(ipsSensorData.pm_values[i]);
            }
            ips["valid"] = true;
        }
        
        // Calibration data
        if (calibConfig.enableCalibration && calibratedData.valid) {
            JsonObject calib = data.createNestedObject("calibration");
            calib["CO"] = calibratedData.CO;
            calib["NO"] = calibratedData.NO;
            calib["NO2"] = calibratedData.NO2;
            calib["O3"] = calibratedData.O3;
            calib["SO2"] = calibratedData.SO2;
            calib["H2S"] = calibratedData.H2S;
            calib["NH3"] = calibratedData.NH3;
            calib["VOC"] = calibratedData.VOC;
            calib["VOC_ppb"] = calibratedData.VOC_ppb;
            calib["HCHO"] = calibratedData.HCHO;
            calib["PID"] = calibratedData.PID;
            calib["valid"] = true;
        }
        
    } else {
        // Pojedynczy sensor
        JsonObject data = response.createNestedObject("data");
        
        if (sensorType == "solar" && solarSensorStatus && solarData.valid) {
            data["V"] = solarData.V;
            data["I"] = solarData.I;
            data["VPV"] = solarData.VPV;
            data["PPV"] = solarData.PPV;
            data["valid"] = true;
        } else if (sensorType == "sht40" && sht40SensorStatus && sht40Data.valid) {
            data["temperature"] = sht40Data.temperature;
            data["humidity"] = sht40Data.humidity;
            data["pressure"] = sht40Data.pressure;
            data["valid"] = true;
        } else if (sensorType == "sps30" && sps30SensorStatus && sps30Data.valid) {
            data["PM1"] = sps30Data.pm1_0;
            data["PM25"] = sps30Data.pm2_5;
            data["PM4"] = sps30Data.pm4_0;
            data["PM10"] = sps30Data.pm10;
            data["NC05"] = sps30Data.nc0_5;
            data["NC1"] = sps30Data.nc1_0;
            data["NC25"] = sps30Data.nc2_5;
            data["NC4"] = sps30Data.nc4_0;
            data["NC10"] = sps30Data.nc10;
            data["TPS"] = sps30Data.typical_particle_size;
            data["valid"] = true;
        } else if (sensorType == "co2" && scd41SensorStatus && i2cSensorData.valid && i2cSensorData.type == SENSOR_SCD41) {
            data["co2"] = i2cSensorData.co2;
            data["temperature"] = i2cSensorData.temperature;
            data["humidity"] = i2cSensorData.humidity;
            data["valid"] = true;
        } else if (sensorType == "power" && ina219SensorStatus && ina219Data.valid) {
            data["busVoltage"] = ina219Data.busVoltage;
            data["shuntVoltage"] = ina219Data.shuntVoltage;
            data["current"] = ina219Data.current;
            data["power"] = ina219Data.power;
            data["valid"] = true;
        } else if (sensorType == "hcho" && hchoSensorStatus && hchoData.valid) {
            data["hcho_mg"] = hchoData.hcho;
            data["hcho_ppb"] = hchoData.hcho_ppb;
            data["valid"] = true;
        } else {
            response["error"] = "Sensor not available or invalid: " + sensorType;
            response["success"] = false;
        }
    }
    
    if (!response.containsKey("success")) {
        response["success"] = true;
    }
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleGetHistory(AsyncWebSocketClient* client, JsonDocument& doc) {
    String sensorType = doc["sensor"] | "";
    String timeRange = doc["timeRange"] | "1h";
    String sampleType = doc["sampleType"] | "fast"; // Nowy parametr
    unsigned long fromTime = doc["fromTime"] | 0;
    unsigned long toTime = doc["toTime"] | 0;
    
    // Konwersja timeRange na timestampy (epoch milliseconds)
    if (fromTime == 0 && toTime == 0) {
        unsigned long now = 0;
        if (time(nullptr) > 8 * 3600 * 2) { // Jeśli czas jest zsynchronizowany
            now = time(nullptr) * 1000; // Konwertuj na milisekundy
        } else {
            now = millis(); // Fallback do millis()
        }
        
        if (timeRange == "1h") {
            fromTime = now - (60 * 60 * 1000); // 1 godzina
        } else if (timeRange == "6h") {
            fromTime = now - (6 * 60 * 60 * 1000); // 6 godzin
        } else if (timeRange == "24h") {
            fromTime = now - (24 * 60 * 60 * 1000); // 24 godziny
        } else {
            fromTime = now - (60 * 60 * 1000); // domyślnie 1h
        }
        toTime = now;
    }
    
    String jsonResponse;
    size_t samples = getHistoricalData(sensorType, timeRange, jsonResponse, fromTime, toTime, sampleType);
    
    safePrintln("History request: " + sensorType + " " + timeRange + " " + sampleType + " samples: " + String(samples));
    safePrintln("JSON response length: " + String(jsonResponse.length()));
    safePrintln("History enabled: " + String(config.enableHistory));
    safePrintln("History initialized: " + String(historyManager.isInitialized()));
    safePrintln("Free heap: " + String(ESP.getFreeHeap()));
    
    DynamicJsonDocument response(2048);
    response["cmd"] = "history";
    response["sensor"] = sensorType;
    response["timeRange"] = timeRange;
    response["sampleType"] = sampleType;
    response["fromTime"] = fromTime;
    response["toTime"] = toTime;
    response["samples"] = samples;
    response["t"] = millis();
    
    // Add date and time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
        response["time"] = String(timeStr);
        
        char dateStr[20];
        strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
        response["date"] = String(dateStr);
    } else {
        response["time"] = "00:00:00";
        response["date"] = "01/01/2024";
    }
    

    
    // Parse existing JSON response to check if data exists
    DynamicJsonDocument dataDoc(4096);
    DeserializationError parseError = deserializeJson(dataDoc, jsonResponse);
    
    safePrintln("JSON parse error: " + String(parseError.c_str()));
    safePrintln("JSON response preview: " + jsonResponse.substring(0, 200) + "...");
    
    if (!parseError) {
        if (dataDoc.containsKey("error")) {
            // getHistoricalData returned an error
            response["data"] = JsonArray();
            response["success"] = false;
            response["error"] = dataDoc["error"].as<String>();
            safePrintln("History error from getHistoricalData: " + dataDoc["error"].as<String>());
        } else if (dataDoc.containsKey("data") && dataDoc["data"].is<JsonArray>()) {
            JsonArray dataArray = dataDoc["data"];
            response["data"] = dataArray;
            if (dataArray.size() > 0) {
                response["success"] = true;
                safePrintln("History success: " + String(dataArray.size()) + " samples from JSON");
            } else {
                response["success"] = false;
                response["error"] = "No data in response array";
                safePrintln("History error: Empty data array");
            }
        } else {
            response["data"] = JsonArray();
            response["success"] = false;
            response["error"] = "No data field in JSON response";
            safePrintln("History error: No data field in response");
        }
    } else {
        response["data"] = JsonArray();
        response["success"] = false;
        response["error"] = "JSON parse error: " + String(parseError.c_str());
        safePrintln("History error: JSON parse failed - " + String(parseError.c_str()));
    }
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleGetHistoryInfo(AsyncWebSocketClient* client, JsonDocument& doc) {
    DynamicJsonDocument response(2048);
    response["cmd"] = "historyInfo";
    response["t"] = millis();
    
    // Add date and time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
        response["time"] = String(timeStr);
        
        char dateStr[20];
        strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
        response["date"] = String(dateStr);
    } else {
        response["time"] = "00:00:00";
        response["date"] = "01/01/2024";
    }
    
    // Sprawdź czy historia jest włączona
    if (!config.enableHistory) {
        response["enabled"] = false;
        response["error"] = "History system is disabled";
        response["success"] = false;
    } else {
        response["enabled"] = true;
        response["initialized"] = historyManager.isInitialized();
        
        if (historyManager.isInitialized()) {
            response["totalMemoryUsed"] = historyManager.getTotalMemoryUsed();
            response["memoryBudget"] = TARGET_MEMORY_BYTES;
            response["memoryPercent"] = (historyManager.getTotalMemoryUsed() * 100) / TARGET_MEMORY_BYTES;
            
            // Status poszczególnych sensorów
            JsonObject sensors = response.createNestedObject("sensors");
            
            if (config.enableSolarSensor && historyManager.getSolarHistory()) {
                JsonObject solar = sensors.createNestedObject("solar");
                solar["fastSamples"] = historyManager.getSolarHistory()->getFastCount();
                solar["slowSamples"] = historyManager.getSolarHistory()->getSlowCount();
            }
            
            if (config.enableI2CSensors && historyManager.getI2CHistory()) {
                JsonObject i2c = sensors.createNestedObject("i2c");
                i2c["fastSamples"] = historyManager.getI2CHistory()->getFastCount();
                i2c["slowSamples"] = historyManager.getI2CHistory()->getSlowCount();
            }
            
            if (config.enableSPS30 && historyManager.getSPS30History()) {
                JsonObject sps30 = sensors.createNestedObject("sps30");
                sps30["fastSamples"] = historyManager.getSPS30History()->getFastCount();
                sps30["slowSamples"] = historyManager.getSPS30History()->getSlowCount();
            }
            
            if (config.enableSHT40 && historyManager.getSHT40History()) {
                JsonObject sht40 = sensors.createNestedObject("sht40");
                sht40["fastSamples"] = historyManager.getSHT40History()->getFastCount();
                sht40["slowSamples"] = historyManager.getSHT40History()->getSlowCount();
            }
            
            if (config.enableHCHO && historyManager.getHCHOHistory()) {
                JsonObject hcho = sensors.createNestedObject("hcho");
                hcho["fastSamples"] = historyManager.getHCHOHistory()->getFastCount();
                hcho["slowSamples"] = historyManager.getHCHOHistory()->getSlowCount();
            }
            
            if (config.enableINA219 && historyManager.getINA219History()) {
                JsonObject power = sensors.createNestedObject("power");
                power["fastSamples"] = historyManager.getINA219History()->getFastCount();
                power["slowSamples"] = historyManager.getINA219History()->getSlowCount();
            }
            
            if (config.enableINA219 && historyManager.getBatteryHistory()) {
                JsonObject battery = sensors.createNestedObject("battery");
                battery["fastSamples"] = historyManager.getBatteryHistory()->getFastCount();
                battery["slowSamples"] = historyManager.getBatteryHistory()->getSlowCount();
            }
            
            if (config.enableFan && historyManager.getFanHistory()) {
                JsonObject fan = sensors.createNestedObject("fan");
                fan["fastSamples"] = historyManager.getFanHistory()->getFastCount();
                fan["slowSamples"] = historyManager.getFanHistory()->getSlowCount();
            }
        } else {
            response["error"] = "History system not initialized";
            response["success"] = false;
        }
    }
    
    if (!response.containsKey("success")) {
        response["success"] = true;
    }
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleGetAverages(AsyncWebSocketClient* client, JsonDocument& doc) {
    String sensorType = doc["sensor"] | "";
    String avgType = doc["type"] | "fast"; // fast lub slow
    
    DynamicJsonDocument response(4096); // Zwiększamy rozmiar dla więcej danych
    response["cmd"] = "averages";
    response["sensor"] = sensorType;
    response["type"] = avgType;
    response["t"] = millis(); // Timestamp w ms
    
    // Dodaj date i time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
        response["time"] = String(timeStr);
        
        char dateStr[20];
        strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
        response["date"] = String(dateStr);
    } else {
        response["time"] = "00:00:00";
        response["date"] = "01/01/2024";
    }
    
    JsonObject data = response.createNestedObject("data");
    
    if (sensorType == "solar" || sensorType == "all") {
        if (avgType == "fast") {
            SolarData avg = getSolarFastAverage();
            if (avg.valid) {
                JsonObject solar = data.createNestedObject("solar");
                solar["V"] = avg.V;
                solar["I"] = avg.I;
                solar["VPV"] = avg.VPV;
                solar["PPV"] = avg.PPV;
                solar["valid"] = true;
            }
        } else {
            SolarData avg = getSolarSlowAverage();
            if (avg.valid) {
                JsonObject solar = data.createNestedObject("solar");
                solar["V"] = avg.V;
                solar["I"] = avg.I;
                solar["VPV"] = avg.VPV;
                solar["PPV"] = avg.PPV;
                solar["valid"] = true;
            }
        }
    }
    
  

    // SHT40 (temperatura, wilgotność, ciśnienie)
    if (sensorType == "sht40" || sensorType == "all") {
        if (avgType == "fast") {
            SHT40Data avg = getSHT40FastAverage();
            if (avg.valid) {
                JsonObject sht40 = data.createNestedObject("sht40");
                sht40["temperature"] = avg.temperature;
                sht40["humidity"] = avg.humidity;
                sht40["pressure"] = avg.pressure;
                sht40["valid"] = true;
            }
        } else {
            SHT40Data avg = getSHT40SlowAverage();
            if (avg.valid) {
                JsonObject sht40 = data.createNestedObject("sht40");
                sht40["temperature"] = avg.temperature;
                sht40["humidity"] = avg.humidity;
                sht40["pressure"] = avg.pressure;
                sht40["valid"] = true;
            }
        }
    }

    // CO2 (osobna sekcja)
    if (sensorType == "scd41" || sensorType == "all") {
        if (avgType == "fast") {
            I2CSensorData avg = getI2CFastAverage();
            if (avg.valid) {
                JsonObject scd41 = data.createNestedObject("scd41");
                scd41["co2"] = avg.co2;
                scd41["valid"] = true;
            }
        } else {
            I2CSensorData avg = getI2CSlowAverage();
            if (avg.valid) {
                JsonObject scd41 = data.createNestedObject("scd41");
                scd41["co2"] = avg.co2;
                scd41["valid"] = true;
            }
        }
    }

    
    if (sensorType == "sps30" || sensorType == "all") {
        if (avgType == "fast") {
            SPS30Data avg = getSPS30FastAverage();
            if (avg.valid) {
                JsonObject sps30 = data.createNestedObject("sps30");
                sps30["PM1"] = avg.pm1_0;
                sps30["PM25"] = avg.pm2_5;
                sps30["PM4"] = avg.pm4_0;
                sps30["PM10"] = avg.pm10;
                sps30["NC05"] = avg.nc0_5;
                sps30["NC1"] = avg.nc1_0;
                sps30["NC25"] = avg.nc2_5;
                sps30["NC4"] = avg.nc4_0;
                sps30["NC10"] = avg.nc10;
                sps30["TPS"] = avg.typical_particle_size;
                sps30["valid"] = true;
            }
        } else {
            SPS30Data avg = getSPS30SlowAverage();
            if (avg.valid) {
                JsonObject sps30 = data.createNestedObject("sps30");
                sps30["PM1"] = avg.pm1_0;
                sps30["PM25"] = avg.pm2_5;
                sps30["PM4"] = avg.pm4_0;
                sps30["PM10"] = avg.pm10;
                sps30["NC05"] = avg.nc0_5;
                sps30["NC1"] = avg.nc1_0;
                sps30["NC25"] = avg.nc2_5;
                sps30["NC4"] = avg.nc4_0;
                sps30["NC10"] = avg.nc10;
                sps30["TPS"] = avg.typical_particle_size;
                sps30["valid"] = true;
            }
        }
    }
    
    if (sensorType == "power" || sensorType == "all") {
        if (avgType == "fast") {
            INA219Data avg = getINA219FastAverage();
            if (avg.valid) {
                JsonObject power = data.createNestedObject("power");
                power["busVoltage"] = avg.busVoltage;
                power["shuntVoltage"] = avg.shuntVoltage;
                power["current"] = avg.current;
                power["power"] = avg.power;
                power["valid"] = true;
            }
        } else {
            INA219Data avg = getINA219SlowAverage();
            if (avg.valid) {
                JsonObject power = data.createNestedObject("power");
                power["busVoltage"] = avg.busVoltage;
                power["shuntVoltage"] = avg.shuntVoltage;
                power["current"] = avg.current;
                power["power"] = avg.power;
                power["valid"] = true;
            }
        }
    }
    
    if (sensorType == "hcho" || sensorType == "all") {
        if (avgType == "fast") {
            HCHOData avg = getHCHOFastAverage();
            if (avg.valid) {
                JsonObject hcho = data.createNestedObject("hcho");
                hcho["hcho_mg"] = avg.hcho;
                hcho["hcho_ppb"] = avg.hcho_ppb;
                hcho["valid"] = true;
            }
        } else {
            HCHOData avg = getHCHOSlowAverage();
            if (avg.valid) {
                JsonObject hcho = data.createNestedObject("hcho");
                hcho["hcho_mg"] = avg.hcho;
                hcho["hcho_ppb"] = avg.hcho_ppb;
                hcho["valid"] = true;
            }
        }
    }
    
    if (sensorType == "ips" || sensorType == "all") {
        if (avgType == "fast") {
            IPSSensorData avg = getIPSFastAverage();
            if (avg.valid) {
                JsonObject ips = data.createNestedObject("ips");
                for (int i = 0; i < 7; i++) {
                    ips["pc_" + String(i+1)] = avg.pc_values[i];
                    ips["pm_" + String(i+1)] = avg.pm_values[i];
                    ips["np_" + String(i+1)] = avg.np_values[i];
                    ips["pw_" + String(i+1)] = avg.pw_values[i];
                }
                ips["debugMode"] = avg.debugMode;
                ips["won"] = avg.won;
                ips["valid"] = true;
            }
        } else {
            IPSSensorData avg = getIPSSlowAverage();
            if (avg.valid) {
                JsonObject ips = data.createNestedObject("ips");
                for (int i = 0; i < 7; i++) {
                    ips["pc_" + String(i+1)] = avg.pc_values[i];
                    ips["pm_" + String(i+1)] = avg.pm_values[i];
                    ips["np_" + String(i+1)] = avg.np_values[i];
                    ips["pw_" + String(i+1)] = avg.pw_values[i];
                }
                ips["debugMode"] = avg.debugMode;
                ips["won"] = avg.won;
                ips["valid"] = true;
            }
        }
    }
    
    // if (sensorType == "mcp3424" || sensorType == "all") {
    //     if (avgType == "fast") {
    //         MCP3424Data avg = getMCP3424FastAverage();
    //         if (avg.valid) {
    //             JsonObject mcp3424 = data.createNestedObject("mcp3424");
    //             mcp3424["enabled"] = true;
    //             mcp3424["deviceCount"] = avg.deviceCount;
                
    //             JsonArray devices = mcp3424.createNestedArray("devices");
    //             for (uint8_t device = 0; device < avg.deviceCount; device++) {
    //                 JsonObject deviceObj = devices.createNestedObject();
    //                 deviceObj["address"] = "0x" + String(avg.addresses[device], HEX);
    //                 deviceObj["valid"] = avg.valid[device];
    //                 deviceObj["resolution"] = avg.resolution;
    //                 deviceObj["gain"] = avg.gain;
                    
    //                 JsonObject channels = deviceObj.createNestedObject("channels");
    //                 // Klucze w formacie K%d_%d (numer urządzenia, numer kanału)
    //                 for (uint8_t ch = 0; ch < 4; ch++) {
    //                     String key_mV = "K" + String(device+1) + "_" + String(ch+1) ;
    //                   //  String key_V = "K" + String(device+1) + "_" + String(ch+1) + "_V";
    //                     channels[key_mV] = round(avg.channels[device][ch] * 1000 * 1000) / 1000.0;
    //                   //  channels[key_V] = round(avg.channels[device][ch] * 1000000) / 1000000.0;
    //                 }
    //             }
    //             mcp3424["valid"] = true;
    //         }
    //     } else {
    //         MCP3424Data avg = getMCP3424SlowAverage();
    //         if (avg.valid) {
    //             JsonObject mcp3424 = data.createNestedObject("mcp3424");
    //             mcp3424["enabled"] = true;
    //             mcp3424["deviceCount"] = avg.deviceCount;
                
    //             JsonArray devices = mcp3424.createNestedArray("devices");
    //             for (uint8_t device = 0; device < avg.deviceCount; device++) {
    //                 JsonObject deviceObj = devices.createNestedObject();
    //                 deviceObj["address"] = "0x" + String(avg.addresses[device], HEX);
    //                 deviceObj["valid"] = avg.valid[device];
    //                 deviceObj["resolution"] = avg.resolution;
    //                 deviceObj["gain"] = avg.gain;
                    
    //                 JsonObject channels = deviceObj.createNestedObject("channels");
    //                 // Klucze w formacie K%d_%d (numer urzadzenia, numer kanalu)
    //                 for (uint8_t ch = 0; ch < 4; ch++) {
    //                     String key_mV = "K" + String(device+1) + "_" + String(ch+1);
    //                   //  String key_V = "K" + String(device+1) + "_" + String(ch+1) ;
    //                     channels[key_mV] = round(avg.channels[device][ch] * 1000 * 1000) / 1000.0;
    //                   //  channels[key_V] = round(avg.channels[device][ch] * 1000000) / 1000000.0;
    //                 }
    //             }
    //             mcp3424["valid"] = true;
    //         }
    //     }
    // }
    
    // K_channels - wszystkie kanały K jako osobne klucze
    if (sensorType == "mcp3424" || sensorType == "all") {
        if (avgType == "fast") {
            MCP3424Data avg = getMCP3424FastAverage();
            if (avg.valid) {
                JsonObject k_channels = data.createNestedObject("K_channels");
                for (uint8_t device = 0; device < avg.deviceCount; device++) {
                    if (avg.valid[device]) {
                        for (uint8_t ch = 0; ch < 4; ch++) {
                            String key = "K" + String(device+1) + "_" + String(ch+1);
                            k_channels[key] = round(avg.channels[device][ch] * 1000 * 1000) / 1000.0;
                        }
                    }
                }
                k_channels["valid"] = true;
            }
        } else {
            MCP3424Data avg = getMCP3424SlowAverage();
            if (avg.valid) {
                JsonObject k_channels = data.createNestedObject("K_channels");
                for (uint8_t device = 0; device < avg.deviceCount; device++) {
                    if (avg.valid[device]) {
                        for (uint8_t ch = 0; ch < 4; ch++) {
                            String key = "K" + String(device+1) + "_" + String(ch+1);
                            k_channels[key] = round(avg.channels[device][ch] * 1000 * 1000) / 1000.0;
                        }
                    }
                }
                k_channels["valid"] = true;
            }
        }
    }
    
    // Battery monitoring
    if (sensorType == "battery" || sensorType == "all") {
        extern BatteryData batteryData;
        if (batteryData.valid) {
            JsonObject battery = data.createNestedObject("battery");
            battery["voltage"] = round(batteryData.voltage * 1000) / 1000.0;
            battery["current"] = round(batteryData.current * 100) / 100.0;
            battery["power"] = round(batteryData.power * 100) / 100.0;
            battery["chargePercent"] = batteryData.chargePercent;
            battery["isBatteryPowered"] = batteryData.isBatteryPowered;
            battery["lowBattery"] = batteryData.lowBattery;
            battery["criticalBattery"] = batteryData.criticalBattery;
            battery["offPinState"] = digitalRead(OFF_PIN);
            battery["valid"] = true;
        }
    }
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleSetConfig(AsyncWebSocketClient* client, JsonDocument& doc) {
    DynamicJsonDocument response(1024);
    response["cmd"] = "setConfig";
    response["success"] = false;
    
    // Sprawdź czy są dane konfiguracyjne
    if (doc.containsKey("enableWiFi")) {
        config.enableWiFi = doc["enableWiFi"];
        response["enableWiFi"] = config.enableWiFi;
    }
    
    if (doc.containsKey("enableHistory")) {
        config.enableHistory = doc["enableHistory"];
        response["enableHistory"] = config.enableHistory;
    }
    if (doc.containsKey("useAveragedData")) {
        config.useAveragedData = doc["useAveragedData"];
        response["useAveragedData"] = config.useAveragedData;
    }
    
    // Network configuration commands
    if (doc.containsKey("setWiFiConfig")) {
        String ssid = doc["ssid"] | "";
        String password = doc["password"] | "";
        
        if (ssid.length() > 0) {
            if (saveWiFiConfig(ssid.c_str(), password.c_str())) {
                response["success"] = true;
                response["message"] = "WiFi configuration saved";
                response["cmd"] = "setWiFiConfig";
            } else {
                response["success"] = false;
                response["error"] = "Failed to save WiFi config";
                response["cmd"] = "setWiFiConfig";
            }
        } else {
            response["success"] = false;
            response["error"] = "SSID cannot be empty";
            response["cmd"] = "setWiFiConfig";
        }
    }
    
    if (doc.containsKey("setNetworkConfig")) {
        networkConfig.useDHCP = doc["useDHCP"] | true;
        strlcpy(networkConfig.staticIP, doc["staticIP"] | "192.168.1.100", sizeof(networkConfig.staticIP));
        strlcpy(networkConfig.gateway, doc["gateway"] | "192.168.1.1", sizeof(networkConfig.gateway));
        strlcpy(networkConfig.subnet, doc["subnet"] | "255.255.255.0", sizeof(networkConfig.subnet));
        strlcpy(networkConfig.dns1, doc["dns1"] | "8.8.8.8", sizeof(networkConfig.dns1));
        strlcpy(networkConfig.dns2, doc["dns2"] | "8.8.4.4", sizeof(networkConfig.dns2));
        
        if (saveNetworkConfig(networkConfig)) {
            response["success"] = true;
            response["message"] = "Network configuration saved";
            response["cmd"] = "setNetworkConfig";
        } else {
            response["success"] = false;
            response["error"] = "Failed to save network config";
            response["cmd"] = "setNetworkConfig";
        }
    }
    
    if (doc.containsKey("enableModbus")) {
        config.enableModbus = doc["enableModbus"];
        response["enableModbus"] = config.enableModbus;
    }
    
    if (doc.containsKey("enableSolarSensor")) {
        config.enableSolarSensor = doc["enableSolarSensor"];
        response["enableSolarSensor"] = config.enableSolarSensor;
    }
    
    if (doc.containsKey("enableSPS30")) {
        config.enableSPS30 = doc["enableSPS30"];
        response["enableSPS30"] = config.enableSPS30;
    }
    
    if (doc.containsKey("enableSHT40")) {
        config.enableSHT40 = doc["enableSHT40"];
        response["enableSHT40"] = config.enableSHT40;
    }
    
    if (doc.containsKey("enableHCHO")) {
        config.enableHCHO = doc["enableHCHO"];
        response["enableHCHO"] = config.enableHCHO;
    }
    
    if (doc.containsKey("enableINA219")) {
        config.enableINA219 = doc["enableINA219"];
        response["enableINA219"] = config.enableINA219;
    }
    
    if (doc.containsKey("enableSHT30")) {
        config.enableSHT30 = doc["enableSHT30"];
        response["enableSHT30"] = config.enableSHT30;
    }
    
    if (doc.containsKey("enableBME280")) {
        config.enableBME280 = doc["enableBME280"];
        response["enableBME280"] = config.enableBME280;
    }
    
    if (doc.containsKey("enableSCD41")) {
        config.enableSCD41 = doc["enableSCD41"];
        response["enableSCD41"] = config.enableSCD41;
    }
    
    if (doc.containsKey("enableI2CSensors")) {
        config.enableI2CSensors = doc["enableI2CSensors"];
        response["enableI2CSensors"] = config.enableI2CSensors;
    }
    
    if (doc.containsKey("enableMCP3424")) {
        config.enableMCP3424 = doc["enableMCP3424"];
        response["enableMCP3424"] = config.enableMCP3424;
    }
    
    if (doc.containsKey("enableADS1110")) {
        config.enableADS1110 = doc["enableADS1110"];
        response["enableADS1110"] = config.enableADS1110;
    }
    
    if (doc.containsKey("enableOPCN3Sensor")) {
        config.enableOPCN3Sensor = doc["enableOPCN3Sensor"];
        response["enableOPCN3Sensor"] = config.enableOPCN3Sensor;
    }
    
    if (doc.containsKey("enableIPS")) {
        config.enableIPS = doc["enableIPS"];
        response["enableIPS"] = config.enableIPS;
    }
    
    if (doc.containsKey("enableIPSDebug")) {
        config.enableIPSDebug = doc["enableIPSDebug"];
        response["enableIPSDebug"] = config.enableIPSDebug;
    }
    
    if (doc.containsKey("enableWebServer")) {
        config.enableWebServer = doc["enableWebServer"];
        response["enableWebServer"] = config.enableWebServer;
    }
    
    if (doc.containsKey("enableFan")) {
        config.enableFan = doc["enableFan"];
        response["enableFan"] = config.enableFan;
    }
    
    if (doc.containsKey("autoReset")) {
        config.autoReset = doc["autoReset"];
        response["autoReset"] = config.autoReset;
    }
    
    if (doc.containsKey("lowPowerMode")) {
        config.lowPowerMode = doc["lowPowerMode"];
        response["lowPowerMode"] = config.lowPowerMode;
    }
    
    if (doc.containsKey("enablePushbullet")) {
        config.enablePushbullet = doc["enablePushbullet"];
        response["enablePushbullet"] = config.enablePushbullet;
        
        // Save configuration to file
        if (saveSystemConfig(config)) {
            response["success"] = true;
            response["message"] = "Pushbullet setting saved";
        } else {
            response["success"] = false;
            response["error"] = "Failed to save configuration";
        }
    }
    
    if (doc.containsKey("pushbulletToken")) {
        strncpy(config.pushbulletToken, doc["pushbulletToken"] | "", sizeof(config.pushbulletToken) - 1);
        config.pushbulletToken[sizeof(config.pushbulletToken) - 1] = '\0';
        response["pushbulletToken"] = config.pushbulletToken;
        
        // Save configuration to file
        if (saveSystemConfig(config)) {
            response["success"] = true;
            response["message"] = "Pushbullet token saved";
        } else {
            response["success"] = false;
            response["error"] = "Failed to save configuration";
        }
    }
    
    response["success"] = true;
    response["message"] = "Configuration updated";
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleGetConfig(AsyncWebSocketClient* client, JsonDocument& doc) {
    DynamicJsonDocument response(2048);
    response["cmd"] = "getConfig";
    response["t"] = millis();
    
    // Add date and time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
        response["time"] = String(timeStr);
        
        char dateStr[20];
        strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
        response["date"] = String(dateStr);
    } else {
        response["time"] = "00:00:00";
        response["date"] = "01/01/2024";
    }
    
    JsonObject configObj = response.createNestedObject("config");
    configObj["enableWiFi"] = config.enableWiFi;
    configObj["enableWebServer"] = config.enableWebServer;
    configObj["enableHistory"] = config.enableHistory;
    configObj["useAveragedData"] = config.useAveragedData;
    configObj["enableModbus"] = config.enableModbus;
    configObj["enableSolarSensor"] = config.enableSolarSensor;
    configObj["enableOPCN3Sensor"] = config.enableOPCN3Sensor;
    configObj["enableI2CSensors"] = config.enableI2CSensors;
    configObj["enableSHT30"] = config.enableSHT30;
    configObj["enableBME280"] = config.enableBME280;
    configObj["enableSCD41"] = config.enableSCD41;
    configObj["enableSHT40"] = config.enableSHT40;
    configObj["enableSPS30"] = config.enableSPS30;
    configObj["enableMCP3424"] = config.enableMCP3424;
    configObj["enableADS1110"] = config.enableADS1110;
    configObj["enableINA219"] = config.enableINA219;
    configObj["enableIPS"] = config.enableIPS;
    configObj["enableIPSDebug"] = config.enableIPSDebug;
    configObj["enableHCHO"] = config.enableHCHO;
    configObj["enableFan"] = config.enableFan;
    configObj["autoReset"] = config.autoReset;
    configObj["lowPowerMode"] = config.lowPowerMode;
    configObj["enablePushbullet"] = config.enablePushbullet;
    configObj["pushbulletToken"] = config.pushbulletToken;
    
    response["success"] = true;
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleSystemCommand(AsyncWebSocketClient* client, JsonDocument& doc) {
    String command = doc["command"] | "";
    
    DynamicJsonDocument response(1024);
    response["cmd"] = "system";
    response["command"] = command;
    response["t"] = millis();
    
    // Add date and time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
        response["time"] = String(timeStr);
        
        char dateStr[20];
        strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
        response["date"] = String(dateStr);
    } else {
        response["time"] = "00:00:00";
        response["date"] = "01/01/2024";
    }
    
    if (command == "restart") {
        response["success"] = true;
        response["message"] = "System restarting...";
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
        // Restart po krótkim opóźnieniu
        delay(1000);
        ESP.restart();
        
    } else if (command == "reset") {
        response["success"] = true;
        response["message"] = "System resetting...";
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
        // Reset po krótkim opóźnieniu
        delay(1000);
        ESP.restart();
        
    } else if (command == "memory") {
        response["success"] = true;
        response["freeHeap"] = ESP.getFreeHeap();
        response["freePsram"] = ESP.getFreePsram();
        response["psramSize"] = ESP.getPsramSize();
        response["uptime"] = millis() / 1000;
        
    } else if (command == "lowPowerOn") {
        config.lowPowerMode = true;
        response["success"] = true;
        response["message"] = "Low power mode enabled";
        response["lowPowerMode"] = true;
        
    } else if (command == "lowPowerOff") {
        config.lowPowerMode = false;
        response["success"] = true;
        response["message"] = "Low power mode disabled";
        response["lowPowerMode"] = false;
        
    } else if (command == "pushbulletTest") {
        if (config.enablePushbullet && strlen(config.pushbulletToken) > 0) {
            // Send test notification
            sendPushbulletNotification("🧪 Test Notification", "This is a test notification from ESP32 Sensor Cube\nTime: " + getFormattedTime() + "\nUptime: " + getUptimeString());
            
            response["success"] = true;
            response["message"] = "Test notification sent";
        } else {
            response["success"] = false;
            response["message"] = "Pushbullet not configured";
        }
        
    } else if (command == "pushbulletBatteryTest") {
        if (config.enablePushbullet && strlen(config.pushbulletToken) > 0) {
            // Send battery critical test notification
            sendBatteryCriticalNotification();
            
            response["success"] = true;
            response["message"] = "Battery critical test notification sent";
        } else {
            response["success"] = false;
            response["message"] = "Pushbullet not configured";
        }
        
    } else if (command == "getNetworkConfig") {
        response["success"] = true;
        response["cmd"] = "networkConfig";
        
        // Add network configuration data
        response["useDHCP"] = networkConfig.useDHCP;
        response["staticIP"] = networkConfig.staticIP;
        response["gateway"] = networkConfig.gateway;
        response["subnet"] = networkConfig.subnet;
        response["dns1"] = networkConfig.dns1;
        response["dns2"] = networkConfig.dns2;
        response["configValid"] = networkConfig.configValid;
        
        // Add current WiFi status
        response["currentIP"] = WiFi.localIP().toString();
        response["currentSSID"] = WiFi.SSID();
        response["wifiConnected"] = WiFi.status() == WL_CONNECTED;
        response["wifiSignal"] = WiFi.RSSI();
        
        // Add WiFi credentials (if available)
        char ssid[32], password[64];
        if (loadWiFiConfig(ssid, password, sizeof(ssid), sizeof(password))) {
            response["wifiSSID"] = ssid;
            response["wifiPassword"] = password;
        }
        
    } else if (command == "testWiFi") {
        response["success"] = true;
        response["cmd"] = "testWiFi";
        response["wifiConnected"] = WiFi.status() == WL_CONNECTED;
        response["ssid"] = WiFi.SSID();
        response["rssi"] = WiFi.RSSI();
        response["localIP"] = WiFi.localIP().toString();
    } else if (command == "getMCP3424Config") {
        // Get current MCP3424 configuration
        response["success"] = true;
        response["cmd"] = "mcp3424Config";
        
        // Manually serialize MCP3424Config to JSON
        JsonObject configObj = response.createNestedObject("config");
        configObj["deviceCount"] = mcp3424Config.deviceCount;
        configObj["configValid"] = mcp3424Config.configValid;
        
        JsonArray devices = configObj.createNestedArray("devices");
        for (uint8_t i = 0; i < mcp3424Config.deviceCount; i++) {
            JsonObject device = devices.createNestedObject();
            device["deviceIndex"] = mcp3424Config.devices[i].deviceIndex;
            device["gasType"] = mcp3424Config.devices[i].gasType;
            device["description"] = mcp3424Config.devices[i].description;
            device["enabled"] = mcp3424Config.devices[i].enabled;
        }
        
    } else if (command == "applyNetworkConfig") {
        if (applyNetworkConfig()) {
            response["success"] = true;
            response["message"] = "Network configuration applied successfully";
            response["cmd"] = "applyNetworkConfig";
        } else {
            response["success"] = false;
            response["error"] = "Failed to apply network configuration";
            response["cmd"] = "applyNetworkConfig";
        }
        
    } else if (command == "resetNetworkConfig") {
        if (deleteAllConfig()) {
            response["success"] = true;
            response["message"] = "All network configuration reset";
            response["cmd"] = "resetNetworkConfig";
        } else {
            response["success"] = false;
            response["error"] = "Failed to reset network configuration";
            response["cmd"] = "resetNetworkConfig";
        }
        
    } else if (command == "getMCP3424Config") {
        response["success"] = true;
        response["cmd"] = "mcp3424Config";
        response["config"] = getMCP3424ConfigJson();
        
    } else if (command == "setMCP3424Config") {
        // Parse MCP3424 configuration from JSON
        if (doc.containsKey("devices")) {
            JsonArray devices = doc["devices"];
            mcp3424Config.deviceCount = 0;
            
            for (JsonObject device : devices) {
                if (mcp3424Config.deviceCount < 8) {
                    MCP3424DeviceAssignment& newDevice = mcp3424Config.devices[mcp3424Config.deviceCount];
                    newDevice.deviceIndex = device["deviceIndex"] | 0;
                    strlcpy(newDevice.gasType, device["gasType"] | "", sizeof(newDevice.gasType));
                    strlcpy(newDevice.description, device["description"] | "", sizeof(newDevice.description));
                    newDevice.enabled = device["enabled"] | true;
                    mcp3424Config.deviceCount++;
                }
            }
            
            if (saveMCP3424Config(mcp3424Config)) {
                response["success"] = true;
                response["message"] = "MCP3424 configuration saved";
            } else {
                response["success"] = false;
                response["error"] = "Failed to save MCP3424 configuration";
            }
        } else {
            response["success"] = false;
            response["error"] = "No devices provided";
        }
        
    } else if (command == "resetMCP3424Config") {
        initializeDefaultMCP3424Mapping();
        if (saveMCP3424Config(mcp3424Config)) {
            response["success"] = true;
            response["message"] = "MCP3424 configuration reset to defaults";
        } else {
            response["success"] = false;
            response["error"] = "Failed to save default MCP3424 configuration";
        }
    } else if (command == "getMCP3424Config") {
        // Get current MCP3424 configuration
        response["cmd"] = "mcp3424Config";
        response["success"] = true;
        
        // Manually serialize MCP3424Config to JSON
        JsonObject configObj = response.createNestedObject("config");
        configObj["deviceCount"] = mcp3424Config.deviceCount;
        configObj["configValid"] = mcp3424Config.configValid;
        
        JsonArray devices = configObj.createNestedArray("devices");
        for (uint8_t i = 0; i < mcp3424Config.deviceCount; i++) {
            JsonObject device = devices.createNestedObject();
            device["deviceIndex"] = mcp3424Config.devices[i].deviceIndex;
            device["gasType"] = mcp3424Config.devices[i].gasType;
            device["description"] = mcp3424Config.devices[i].description;
            device["enabled"] = mcp3424Config.devices[i].enabled;
        }
        
    } else if (command == "wifi") {
        response["success"] = true;
        response["connected"] = WiFi.status() == WL_CONNECTED;
        response["ssid"] = WiFi.SSID();
        response["rssi"] = WiFi.RSSI();
        response["localIP"] = WiFi.localIP().toString();
        
    } else if (command == "fan_speed") {
        if (!config.enableFan) {
            response["success"] = false;
            response["error"] = "Fan control is DISABLED in configuration";
        } else {
            int speed = doc["value"] | 0;
            if (speed >= 0 && speed <= 100) {
                setFanSpeed(speed);
                response["success"] = true;
                response["message"] = "Fan speed set to " + String(speed) + "%";
                response["speed"] = speed;
            } else {
                response["success"] = false;
                response["error"] = "Invalid speed value (0-100): " + String(speed);
            }
        }
        
    } else if (command == "fan_on") {
        if (!config.enableFan) {
            response["success"] = false;
            response["error"] = "Fan control is DISABLED in configuration";
        } else {
            setFanSpeed(50); // Default 50% speed
            response["success"] = true;
            response["message"] = "Fan turned ON at 50% speed";
            response["speed"] = 50;
        }
        
    } else if (command == "fan_off") {
        if (!config.enableFan) {
            response["success"] = false;
            response["error"] = "Fan control is DISABLED in configuration";
        } else {
            setFanSpeed(0);
            response["success"] = true;
            response["message"] = "Fan turned OFF";
            response["speed"] = 0;
        }
        
    } else if (command == "gline_on") {
        if (!config.enableFan) {
            response["success"] = false;
            response["error"] = "Fan control is DISABLED in configuration";
        } else {
            setGLine(true);
            response["success"] = true;
            response["message"] = "GLine router ENABLED";
        }
        
    } else if (command == "gline_off") {
        if (!config.enableFan) {
            response["success"] = false;
            response["error"] = "Fan control is DISABLED in configuration";
        } else {
            setGLine(false);
            response["success"] = true;
            response["message"] = "GLine router DISABLED";
        }
        
    } else if (command == "fan_status") {
        response["success"] = true;
        response["enabled"] = config.enableFan && isFanEnabled();
        response["dutyCycle"] = config.enableFan ? getFanDutyCycle() : 0;
        response["rpm"] = config.enableFan ? getFanRPM() : 0;
        response["glineEnabled"] = config.enableFan && isGLineEnabled();
        response["pwmValue"] = config.enableFan ? map(getFanDutyCycle(), 0, 100, 0, 255) : 0;
        
    } else {
        response["success"] = false;
        response["error"] = "Unknown command: " + command;
    }
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}



// Główna funkcja obsługi WebSocket
void handleWebSocketMessage(AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len) {
    String message = String((char*)data, len);
    
    // Sprawdź dostępną pamięć
    if (ESP.getFreeHeap() < 10000) {
        safePrintln("WebSocket: Low memory in message handler: " + String(ESP.getFreeHeap()));
        DynamicJsonDocument errorResponse(1024);
        errorResponse["error"] = "Low memory";
        errorResponse["freeHeap"] = ESP.getFreeHeap();
        String errorStr;
        serializeJson(errorResponse, errorStr);
        client->text(errorStr);
        
        // Wymus czyszczenie pamieci
        cleanupWebSocketMemory();
        return;
    }
    
    // Parse JSON
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        DynamicJsonDocument errorResponse(1024);
        errorResponse["error"] = "Invalid JSON: " + String(error.c_str());
        String errorStr;
        serializeJson(errorResponse, errorStr);
        client->text(errorStr);
        return;
    }
    
    // Sprawdź komendę
    String cmd = doc["cmd"] | "";
    
    if (cmd == "status") {
        handleGetStatus(client, doc);
    } else if (cmd == "getSensorData") {
        handleGetSensorData(client, doc);
    } else if (cmd == "getHistory") {
        handleGetHistory(client, doc);
    } else if (cmd == "getHistoryInfo") {
        handleGetHistoryInfo(client, doc);
    } else if (cmd == "getAverages") {
        handleGetAverages(client, doc);
    } else if (cmd == "setConfig") {
        handleSetConfig(client, doc);
    } else if (cmd == "getConfig") {
        handleGetConfig(client, doc);
    } else if (cmd == "system") {
        handleSystemCommand(client, doc);
    } else if (cmd == "pingpong") {
        // Natywne ping/pong jest obsługiwane przez WebSocket framework
        // Ta komenda jest zachowana dla kompatybilności z frontendem
        DynamicJsonDocument response(256);
        response["cmd"] = "pingpong";
        response["command"] = "pong";
        response["timestamp"] = millis();
        response["freeHeap"] = ESP.getFreeHeap();
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
    } else if (cmd == "getNetworkConfig") {
        // Handle network config request
        DynamicJsonDocument response(2048);
        response["success"] = true;
        response["cmd"] = "networkConfig";
        
        // Add network configuration data
        response["useDHCP"] = networkConfig.useDHCP;
        response["staticIP"] = networkConfig.staticIP;
        response["gateway"] = networkConfig.gateway;
        response["subnet"] = networkConfig.subnet;
        response["dns1"] = networkConfig.dns1;
        response["dns2"] = networkConfig.dns2;
        response["configValid"] = networkConfig.configValid;
        
        // Add current WiFi status
        response["currentIP"] = WiFi.localIP().toString();
        response["currentSSID"] = WiFi.SSID();
        response["wifiConnected"] = WiFi.status() == WL_CONNECTED;
        response["wifiSignal"] = WiFi.RSSI();
        
        // Add WiFi credentials (if available)
        char ssid[32], password[64];
        if (loadWiFiConfig(ssid, password, sizeof(ssid), sizeof(password))) {
            response["wifiSSID"] = ssid;
            response["wifiPassword"] = password;
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "setWiFiConfig") {
        // Handle WiFi config save
        String ssid = doc["ssid"] | "";
        String password = doc["password"] | "";
        
        DynamicJsonDocument response(1024);
        response["cmd"] = "setWiFiConfig";
        
        if (ssid.length() > 0) {
            if (saveWiFiConfig(ssid.c_str(), password.c_str())) {
                response["success"] = true;
                response["message"] = "WiFi configuration saved";
            } else {
                response["success"] = false;
                response["error"] = "Failed to save WiFi config";
            }
        } else {
            response["success"] = false;
            response["error"] = "SSID cannot be empty";
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "setNetworkConfig") {
        // Handle network config save
        DynamicJsonDocument response(1024);
        response["cmd"] = "setNetworkConfig";
        
        networkConfig.useDHCP = doc["useDHCP"] | true;
        strlcpy(networkConfig.staticIP, doc["staticIP"] | "192.168.1.100", sizeof(networkConfig.staticIP));
        strlcpy(networkConfig.gateway, doc["gateway"] | "192.168.1.1", sizeof(networkConfig.gateway));
        strlcpy(networkConfig.subnet, doc["subnet"] | "255.255.255.0", sizeof(networkConfig.subnet));
        strlcpy(networkConfig.dns1, doc["dns1"] | "8.8.8.8", sizeof(networkConfig.dns1));
        strlcpy(networkConfig.dns2, doc["dns2"] | "8.8.4.4", sizeof(networkConfig.dns2));
        
        if (saveNetworkConfig(networkConfig)) {
            response["success"] = true;
            response["message"] = "Network configuration saved";
        } else {
            response["success"] = false;
            response["error"] = "Failed to save network config";
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "testWiFi") {
        // Handle WiFi test
        DynamicJsonDocument response(1024);
        response["success"] = true;
        response["cmd"] = "testWiFi";
        response["wifiConnected"] = WiFi.status() == WL_CONNECTED;
        response["ssid"] = WiFi.SSID();
        response["rssi"] = WiFi.RSSI();
        response["localIP"] = WiFi.localIP().toString();
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "applyNetworkConfig") {
        // Handle network config apply
        DynamicJsonDocument response(1024);
        response["cmd"] = "applyNetworkConfig";
        
        if (applyNetworkConfig()) {
            response["success"] = true;
            response["message"] = "Network configuration applied successfully";
        } else {
            response["success"] = false;
            response["error"] = "Failed to apply network configuration";
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "resetNetworkConfig") {
        // Handle network config reset
        DynamicJsonDocument response(1024);
        response["cmd"] = "resetNetworkConfig";
        
        if (deleteAllConfig()) {
            response["success"] = true;
            response["message"] = "All network configuration reset";
        } else {
            response["success"] = false;
            response["error"] = "Failed to reset network configuration";
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "getSensorKeys") {
        // Handle sensor keys request - returns JSON structure with keys instead of values
        safePrintln("WebSocket: getSensorKeys requested");
        DynamicJsonDocument response(4096);
        response["cmd"] = "sensorKeys";
        response["success"] = true;
        response["t"] = millis(); // Timestamp w ms
        
        // Dodaj date i time
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            char timeStr[20];
            strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
            response["time"] = String(timeStr);
            
            char dateStr[20];
            strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
            response["date"] = String(dateStr);
        } else {
            response["time"] = "00:00:00";
            response["date"] = "01/01/2024";
        }
        
        JsonObject data = response.createNestedObject("data");
        
        // SHT40
        JsonObject sht40 = data.createNestedObject("sht40");
        sht40["valid"] = "SHT40_VALID";
        sht40["temperature"] = "I2C11_TEMP";
        sht40["humidity"] = "I2C11_HUMID";
        sht40["pressure"] = "I2C11_PRESS";
        
        // SPS30
        JsonObject sps30 = data.createNestedObject("sps30");
        sps30["valid"] = "SPS30_VALID";
        sps30["PM1"] = "US3_SPS30_PM1";
        sps30["PM25"] = "US3_SPS30_PM25";
        sps30["PM4"] = "US3_SPS30_PM4";
        sps30["PM10"] = "US3_SPS30_PM10";
        sps30["NC05"] = "US3_SPS30_NC05";
        sps30["NC1"] = "US3_SPS30_NC1";
        sps30["NC25"] = "US3_SPS30_NC25";
        sps30["NC4"] = "US3_SPS30_NC4";
        sps30["NC10"] = "US3_SPS30_NC10";
        sps30["TPS"] = "US3_SPS30_TPS";
        sps30["valid"] = "SPS30_VALID";
        
        // CO2
        JsonObject scd41 = data.createNestedObject("scd41");
        scd41["valid"] = "CO2_VALID";
        scd41["co2"] = "I2C5_PRESS";
        
        // Power
        JsonObject power = data.createNestedObject("power");
        power["valid"] = "POWER_VALID";
        power["busVoltage"] = "POWER_BUS_VOLTAGE";
        power["shuntVoltage"] = "POWER_SHUNT_VOLTAGE";
        power["current"] = "POWER_CURRENT";
        power["power"] = "POWER_POWER";
        
        // HCHO
        JsonObject hcho = data.createNestedObject("hcho");
        hcho["valid"] = "HCHO_VALID";
        hcho["hcho"] = "US0_PMS5003ST_HCHO_UG";
        hcho["hcho_ppb"] = "HCHO_PPB";
        
        // IPS Sensor
        JsonObject ips = data.createNestedObject("ips");
        ips["valid"] = "IPS_VALID";
        ips["debugMode"] = "IPS_DEBUG_MODE";
        ips["won"] = "IPS_WON";
        for (int i = 0; i < 7; i++) {
            ips["pc_" + String(i+1)] = "IPS_PC_" + String(i+1);
            ips["pm_" + String(i+1)] = "IPS_PM_" + String(i+1);
            ips["np_" + String(i+1)] = "IPS_NP_" + String(i+1);
            ips["pw_" + String(i+1)] = "IPS_PW_" + String(i+1);
        }
        
        // MCP3424 - wszystkie urządzenia z kluczami K1_1, K1_2, etc.
        JsonObject mcp3424 = data.createNestedObject("mcp3424");
        mcp3424["enabled"] = "MCP3424_ENABLED";
        mcp3424["deviceCount"] = "MCP3424_DEVICE_COUNT";
        mcp3424["valid"] = "MCP3424_VALID";
        
        // JsonArray devices = mcp3424.createNestedArray("devices");
        // for (uint8_t device = 0; device < 8; device++) { // Wszystkie możliwe urządzenia
        //     JsonObject deviceObj = devices.createNestedObject();
        //     deviceObj["address"] = "MCP3424_DEVICE_" + String(device+1) + "_ADDRESS";
        //     deviceObj["valid"] = "MCP3424_DEVICE_" + String(device+1) + "_VALID";
        //     deviceObj["resolution"] = "MCP3424_DEVICE_" + String(device+1) + "_RESOLUTION";
        //     deviceObj["gain"] = "MCP3424_DEVICE_" + String(device+1) + "_GAIN";
            
        //     JsonObject channels = deviceObj.createNestedObject("channels");
        //     // Klucze w formacie K%d_%d (numer urządzenia, numer kanału)
        //     for (uint8_t ch = 0; ch < 4; ch++) {
        //         String key_mV = "K" + String(device+1) + "_" + String(ch+1) ;
        //         String key_V = "K" + String(device+1) + "_" + String(ch+1) ;
        //         channels[key_mV] = key_mV;
        //         channels[key_V] = key_V;
        //     }
        // }
        
        // K_channels - wszystkie kanały K jako osobne klucze
        JsonObject k_channels = data.createNestedObject("K_channels");
        k_channels["valid"] = "K_CHANNELS_VALID";
        for (uint8_t device = 0; device < 8; device++) {
            for (uint8_t ch = 0; ch < 4; ch++) {
                String key = "K" + String(device+1) + "_" + String(ch+1);
                k_channels[key] = key;
            }
        }
        
        // Fan control system
        JsonObject fan = data.createNestedObject("fan");
        fan["enabled"] = "FAN_ENABLED";
        fan["dutyCycle"] = "FAN_DUTY_CYCLE";
        fan["rpm"] = "FAN_RPM";
        fan["glineEnabled"] = "FAN_GLINE_ENABLED";
        fan["pwmValue"] = "FAN_PWM_VALUE";
        fan["valid"] = "FAN_VALID";
        
        // Battery monitoring
        JsonObject battery = data.createNestedObject("battery");
        battery["valid"] = "BATTERY_VALID";
        battery["voltage"] = "BATTERY_VOLTAGE";
        battery["current"] = "BATTERY_CURRENT";
        battery["power"] = "BATTERY_POWER";
        battery["chargePercent"] = "BATTERY_CHARGE_PERCENT";
        battery["isBatteryPowered"] = "BATTERY_IS_BATTERY_POWERED";
        battery["lowBattery"] = "BATTERY_LOW_BATTERY";
        battery["criticalBattery"] = "BATTERY_CRITICAL_BATTERY";
        battery["offPinState"] = "BATTERY_OFF_PIN_STATE";
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "getMCP3424Config") {
        // Handle MCP3424 config request directly
        safePrintln("WebSocket: getMCP3424Config requested");
        DynamicJsonDocument response(2048);
        response["cmd"] = "mcp3424Config";
        response["success"] = true;
        
        // Manually serialize MCP3424Config to JSON
        JsonObject configObj = response.createNestedObject("config");
        configObj["deviceCount"] = mcp3424Config.deviceCount;
        configObj["configValid"] = mcp3424Config.configValid;
        
        JsonArray devices = configObj.createNestedArray("devices");
        safePrintln("MCP3424 config: deviceCount = " + String(mcp3424Config.deviceCount));
        for (uint8_t i = 0; i < mcp3424Config.deviceCount; i++) {
            JsonObject device = devices.createNestedObject();
            device["deviceIndex"] = mcp3424Config.devices[i].deviceIndex;
            device["gasType"] = mcp3424Config.devices[i].gasType;
            device["description"] = mcp3424Config.devices[i].description;
            device["enabled"] = mcp3424Config.devices[i].enabled;
            safePrintln("Device " + String(i) + ": " + mcp3424Config.devices[i].gasType);
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        safePrintln("Sending MCP3424 config response: " + responseStr);
        client->text(responseStr);
    } else {
        DynamicJsonDocument errorResponse(1024);
        errorResponse["error"] = "Unknown command: " + cmd;
        String errorStr;
        serializeJson(errorResponse, errorStr);
        client->text(errorStr);
    }
    
    // Sprawdz pamiec po przetworzeniu wiadomosci
    if (ESP.getFreeHeap() < 15000) {
       // safePrintln("WebSocket: Low memory after message processing: " + String(ESP.getFreeHeap()));
        cleanupWebSocketMemory();
    }
}

// Funkcja dodawania klienta do sledzenia
void addWebSocketClient(AsyncWebSocketClient* client) {
    if (wsClientCount < MAX_WS_CLIENTS) {
        wsClients[wsClientCount].client = client;
        wsClients[wsClientCount].lastPingTime = 0;
        wsClients[wsClientCount].lastPongTime = millis();
        wsClients[wsClientCount].pingSent = false;
        wsClients[wsClientCount].isActive = true;
        wsClientCount++;
        safePrintln("WebSocket: Client added to tracking, total: " + String(wsClientCount));
    }
}

// Funkcja usuwania klienta ze sledzenia
void removeWebSocketClient(AsyncWebSocketClient* client) {
    for (int i = 0; i < wsClientCount; i++) {
        if (wsClients[i].client == client) {
            // Przesun pozostale klienty
            for (int j = i; j < wsClientCount - 1; j++) {
                wsClients[j] = wsClients[j + 1];
            }
            wsClientCount--;
            safePrintln("WebSocket: Client removed from tracking, remaining: " + String(wsClientCount));
            break;
        }
    }
}

// Funkcja aktualizacji czasu pong dla klienta
void updateClientPongTime(AsyncWebSocketClient* client) {
    for (int i = 0; i < wsClientCount; i++) {
        if (wsClients[i].client == client) {
            wsClients[i].lastPongTime = millis();
            wsClients[i].pingSent = false;
            wsClients[i].isActive = true;
            break;
        }
    }
}

// Funkcja wysylania natywnego ping WebSocket
void sendNativePing(AsyncWebSocketClient* client) {
    if (client && client->status() == 1) { // WS_CONNECTED = 1
        client->ping();
      //  safePrintln("WebSocket: Native ping sent to client");
    }
}

// Funkcja sprawdzania i czyszczenia nieaktywnych klientow
void cleanupInactiveClients() {
    unsigned long currentTime = millis();
    const unsigned long PING_TIMEOUT = 30000; // 30 sekund
    const unsigned long PONG_TIMEOUT = 10000; // 10 sekund
    
    for (int i = wsClientCount - 1; i >= 0; i--) {
        if (!wsClients[i].isActive) continue;
        
        // Sprawdz czy klient odpowiada na ping
        if (wsClients[i].pingSent && (currentTime - wsClients[i].lastPingTime > PONG_TIMEOUT)) {
            safePrintln("WebSocket: Client timeout - no pong response");
            wsClients[i].client->close();
            wsClients[i].isActive = false;
            removeWebSocketClient(wsClients[i].client);
            continue;
        }
        
        // Sprawdz czy klient jest aktywny
        if (currentTime - wsClients[i].lastPongTime > PING_TIMEOUT) {
            if (!wsClients[i].pingSent) {
                // Wyslij ping
                sendNativePing(wsClients[i].client);
                wsClients[i].lastPingTime = currentTime;
                wsClients[i].pingSent = true;
              //  safePrintln("WebSocket: Sending ping to inactive client");
            }
        }
    }
}

// Funkcja wysylania natywnego ping WebSocket (zachowana dla kompatybilnosci)
void sendPingToClient(AsyncWebSocketClient* client) {
    sendNativePing(client);
}

// Funkcja sprawdzania polaczen WebSocket
void checkWebSocketConnections() {
    unsigned long currentTime = millis();
    
    // Sprawdz pamiec co 30 sekund
    if (currentTime - lastMemoryCheck > 30000) {
        lastMemoryCheck = currentTime;
        
        uint32_t freeHeap = ESP.getFreeHeap();
      //  safePrintln("WebSocket memory check - Free heap: " + String(freeHeap) + " bytes");
        
        // Jezeli pamiec jest niska, wymus czyszczenie
        if (freeHeap < 30000) {
            safePrintln("WebSocket: Low memory detected, forcing cleanup");
            cleanupWebSocketMemory();
        }
    }
    
    // Sprawdz nieaktywne klienty co 15 sekund
    if (currentTime - lastNativePingTime > 15000) {
        lastNativePingTime = currentTime;
        cleanupInactiveClients();
    }
    
    // Czyszczenie pamieci co 5 minut
    if (currentTime - lastCleanupTime > 300000) {
        lastCleanupTime = currentTime;
        safePrintln("WebSocket: Periodic memory cleanup");
        cleanupWebSocketMemory();
    }
}

// Funkcja czyszczenia pamieci WebSocket
void cleanupWebSocketMemory() {
    safePrintln("WebSocket: Starting memory cleanup");
    
    uint32_t freeHeapBefore = ESP.getFreeHeap();
    
    // Sprawdz nieaktywne klienty i wymus czyszczenie
    cleanupInactiveClients();
    
    // Sprawdz czy klienci odpowiadaja
    safePrintln("WebSocket: Tracked clients: " + String(wsClientCount) + ", Total clients: " + String(ws.count()));
    
    // Użyj inteligentnego czyszczenia pamięci
    intelligentMemoryCleanup();
    
    uint32_t freeHeapAfter = ESP.getFreeHeap();
    int32_t recovered = (int32_t)freeHeapAfter - (int32_t)freeHeapBefore;
    
    safePrintln("WebSocket: Memory cleanup completed. Before: " + String(freeHeapBefore) + 
                " bytes, After: " + String(freeHeapAfter) + " bytes, Recovered: " + String(recovered) + " bytes");
    
    // Jezeli pamiec nadal jest niska, zaloguj ostrzezenie
    if (freeHeapAfter < 20000) {
        safePrintln("WebSocket: WARNING - Memory still low after cleanup!");
        safePrintln("WebSocket: Consider running MEMORY_EMERGENCY command for aggressive cleanup");
    }
}

// Funkcja inicjalizacji WebSocket
void initializeWebSocket(AsyncWebSocket& ws) {
    ws.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            safePrintln("WebSocket client connected");
            
            // Dodaj klienta do sledzenia
            addWebSocketClient(client);
            
            // Sprawdz pamiec przed wyslaniem powitalnej wiadomosci
            if (ESP.getFreeHeap() > 15000) {
                // Wyślij powitalną wiadomość
                DynamicJsonDocument welcome(512);   
                welcome["cmd"] = "welcome";
                welcome["message"] = "WebSocket connected";
                welcome["uptime"] = millis() / 1000;
                welcome["version"] = "1.0";
                welcome["freeHeap"] = ESP.getFreeHeap();
                
                String welcomeStr;
                serializeJson(welcome, welcomeStr);
                client->text(welcomeStr);
            } else {
                safePrintln("WebSocket: Low memory, skipping welcome message");
            }
            
        } else if (type == WS_EVT_DISCONNECT) {
            safePrintln("WebSocket client disconnected");
            
            // Usun klienta ze sledzenia
            removeWebSocketClient(client);
            
            // Wymus czyszczenie pamieci po rozlaczeniu
            if (ESP.getFreeHeap() < 40000) {
                safePrintln("WebSocket: Forcing memory cleanup after disconnect");
                cleanupWebSocketMemory();
            }
            
            // Zaloguj liczbe pozostalych klientow
            safePrintln("WebSocket: Remaining clients: " + String(server->count()));
            
        } else if (type == WS_EVT_PONG) {
            // Klient odpowiedzial na ping
            safePrintln("WebSocket: Pong received from client");
            updateClientPongTime(client);
            
        } else if (type == WS_EVT_ERROR) {
            safePrintln("WebSocket: Error event");
            removeWebSocketClient(client);
            
        } else if (type == WS_EVT_DATA) {
            // Aktualizuj czas aktywnosci klienta
            updateClientPongTime(client);
            
            // Sprawdz pamiec przed przetwarzaniem wiadomosci
            if (ESP.getFreeHeap() > 10000) {
                handleWebSocketMessage(client, arg, data, len);
            } else {
                safePrintln("WebSocket: Low memory, skipping message processing");
                
                // Wyslij blad pamieci do klienta
                DynamicJsonDocument errorResponse(256);
                errorResponse["error"] = "Low memory";
                errorResponse["freeHeap"] = ESP.getFreeHeap();
                String errorStr;
                serializeJson(errorResponse, errorStr);
                client->text(errorStr);
            }
        }
    });
    
    safePrintln("WebSocket initialized with native ping/pong management");
}

// Funkcja do wysyłania danych do wszystkich klientów WebSocket

// ==============================================
// ZAAWANSOWANE CZYSZCZENIE PAMIECI
// ==============================================

void forceGarbageCollection() {
    safePrintln("WebSocket: Starting forced garbage collection");
    
    uint32_t heap_before = ESP.getFreeHeap();
    uint32_t internal_before = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    uint32_t spiram_before = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    
    // 1. Sprawdz integralnosc heap
    bool integrity_ok = heap_caps_check_integrity_all(false);
    if (!integrity_ok) {
        safePrintln("ALERT: Heap corruption detected during cleanup!");
    }
    
    // 2. Wymus defragmentacje przez wielokrotne alokacje/dealokacje
    const int NUM_ATTEMPTS = 5;
    const size_t BLOCK_SIZES[] = {64, 128, 256, 512, 1024};
    
    for (int attempt = 0; attempt < NUM_ATTEMPTS; attempt++) {
        void* ptrs[20];
        int allocated = 0;
        
        // Alokuj bloki różnej wielkości
        for (int i = 0; i < 20; i++) {
            size_t size = BLOCK_SIZES[i % 5];
            ptrs[i] = heap_caps_malloc(size, MALLOC_CAP_INTERNAL);
            if (ptrs[i]) {
                allocated++;
                // Wypelnij losowymi danymi aby wymusic fizyczne uzycie
                memset(ptrs[i], (uint8_t)(i + attempt), size);
            }
        }
        
        // Zwolnij w odwrotnej kolejności
        for (int i = allocated - 1; i >= 0; i--) {
            if (ptrs[i]) {
                heap_caps_free(ptrs[i]);
                ptrs[i] = nullptr;
            }
        }
        
        // Mikro-delay między próbami
        delayMicroseconds(100);
    }
    
    // 3. Arduino/FreeRTOS cleanup
    yield();
    vTaskDelay(pdMS_TO_TICKS(1)); // FreeRTOS task switch
    
    // 4. WebSocket specific cleanup
    cleanupInactiveClients();
    
    // 5. Wymus czyszczenie TCP/IP bufferów
    WiFi.printDiag(Serial); // To może wymusić czyszczenie WiFi bufferów
    
    uint32_t heap_after = ESP.getFreeHeap();
    uint32_t internal_after = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    uint32_t spiram_after = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    
    int32_t heap_recovered = (int32_t)heap_after - (int32_t)heap_before;
    int32_t internal_recovered = (int32_t)internal_after - (int32_t)internal_before;
    int32_t spiram_recovered = (int32_t)spiram_after - (int32_t)spiram_before;
    
    safePrintln("Forced GC Results:");
    safePrintln("- Total heap recovered: " + String(heap_recovered) + " bytes");
    safePrintln("- Internal recovered: " + String(internal_recovered) + " bytes");
    safePrintln("- SPIRAM recovered: " + String(spiram_recovered) + " bytes");
    safePrintln("- Final free heap: " + String(heap_after) + " bytes");
    
    if (heap_recovered > 1000) {
        safePrintln("SUCCESS: Significant memory recovered!");
    } else if (heap_recovered > 0) {
        safePrintln("INFO: Some memory recovered");
    } else {
        safePrintln("WARNING: No memory recovered - possible leak");
    }
}

void intelligentMemoryCleanup() {
    uint32_t free_heap = ESP.getFreeHeap();
    uint32_t min_free = ESP.getMinFreeHeap();
    
    safePrintln("Starting intelligent memory cleanup");
    safePrintln("Current free: " + String(free_heap) + ", Min ever: " + String(min_free));
    
    // Determine cleanup strategy based on memory state
    if (free_heap < 20000) {
        safePrintln("CRITICAL: Very low memory - aggressive cleanup");
        forceGarbageCollection();
        cleanupInactiveClients();
        
        // Emergency: close all WebSocket connections if still critical
        if (ESP.getFreeHeap() < 15000) {
            safePrintln("EMERGENCY: Closing all WebSocket connections");
            ws.closeAll();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
    } else if (free_heap < 40000) {
        safePrintln("LOW: Moderate cleanup needed");
        cleanupInactiveClients();
        
        // Light defragmentation
        void* temp = heap_caps_malloc(1024, MALLOC_CAP_INTERNAL);
        if (temp) {
            heap_caps_free(temp);
        }
        yield();
        
    } else {
        safePrintln("NORMAL: Light maintenance cleanup");
        cleanupInactiveClients();
        yield();
    }
    
    uint32_t final_free = ESP.getFreeHeap();
    safePrintln("Cleanup completed - Final free: " + String(final_free) + " bytes");
}

// Funkcja do wywołania z main.cpp jako komenda Serial
void performEmergencyCleanup() {
    safePrintln("=== EMERGENCY MEMORY CLEANUP ===");
    forceGarbageCollection();
    intelligentMemoryCleanup();
    
    // Final report
    safePrintln("=== CLEANUP SUMMARY ===");
    safePrintln("Free heap: " + String(ESP.getFreeHeap()) + " bytes");
    safePrintln("Free internal: " + String(heap_caps_get_free_size(MALLOC_CAP_INTERNAL)) + " bytes");
    safePrintln("Free SPIRAM: " + String(heap_caps_get_free_size(MALLOC_CAP_SPIRAM)) + " bytes");
    safePrintln("Largest block: " + String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)) + " bytes");
}

