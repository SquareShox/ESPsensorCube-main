#include <web_socket.h>
#include <sensors.h>
#include <history.h>
#include <calib.h>
#include <config.h>
#include <fan.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

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
    response["uptime"] = millis() / 1000;
    response["freeHeap"] = ESP.getFreeHeap();
    response["freePsram"] = ESP.getFreePsram();
    response["wifiRSSI"] = WiFi.RSSI();
    response["wifiConnected"] = WiFi.status() == WL_CONNECTED;
    
    // Sensor status
    JsonObject sensors = response.createNestedObject("sensors");
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
    JsonObject configObj = response.createNestedObject("config");
    configObj["enableWiFi"] = config.enableWiFi;
    configObj["enableWebServer"] = config.enableWebServer;
    configObj["enableHistory"] = config.enableHistory;
    configObj["enableModbus"] = config.enableModbus;
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleGetSensorData(AsyncWebSocketClient* client, JsonDocument& doc) {
    String sensorType = doc["sensor"] | "";
    DynamicJsonDocument response(4096);
    response["cmd"] = "sensorData";
    response["sensor"] = sensorType;
    
    if (sensorType == "all") {
        // Wszystkie dane sensorów
        JsonObject data = response.createNestedObject("data");
        
        // Solar
        if (solarSensorStatus && solarData.valid) {
            JsonObject solar = data.createNestedObject("solar");
            solar["V"] = solarData.V;
            solar["I"] = solarData.I;
            solar["VPV"] = solarData.VPV;
            solar["PPV"] = solarData.PPV;
            solar["valid"] = true;
        }
        
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
            sps30["pm1_0"] = sps30Data.pm1_0;
            sps30["pm2_5"] = sps30Data.pm2_5;
            sps30["pm4_0"] = sps30Data.pm4_0;
            sps30["pm10"] = sps30Data.pm10;
            sps30["nc0_5"] = sps30Data.nc0_5;
            sps30["nc1_0"] = sps30Data.nc1_0;
            sps30["nc2_5"] = sps30Data.nc2_5;
            sps30["nc4_0"] = sps30Data.nc4_0;
            sps30["nc10"] = sps30Data.nc10;
            sps30["typical_particle_size"] = sps30Data.typical_particle_size;
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
            hcho["hcho"] = hchoData.hcho;
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
            data["pm1_0"] = sps30Data.pm1_0;
            data["pm2_5"] = sps30Data.pm2_5;
            data["pm4_0"] = sps30Data.pm4_0;
            data["pm10"] = sps30Data.pm10;
            data["nc0_5"] = sps30Data.nc0_5;
            data["nc1_0"] = sps30Data.nc1_0;
            data["nc2_5"] = sps30Data.nc2_5;
            data["nc4_0"] = sps30Data.nc4_0;
            data["nc10"] = sps30Data.nc10;
            data["typical_particle_size"] = sps30Data.typical_particle_size;
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
            data["hcho"] = hchoData.hcho;
            data["valid"] = true;
        } else {
            response["error"] = "Sensor not available or invalid: " + sensorType;
        }
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
    
    DynamicJsonDocument response(2048);
    response["cmd"] = "history";
    response["sensor"] = sensorType;
    response["timeRange"] = timeRange;
    response["sampleType"] = sampleType; // Dodaj informację o typie próbek
    response["fromTime"] = fromTime;
    response["toTime"] = toTime;
    response["samples"] = samples;
    

    
    if (samples > 0) {
        // Parse existing JSON response
        DynamicJsonDocument dataDoc(2048);
        deserializeJson(dataDoc, jsonResponse);
        response["data"] = dataDoc["data"];
    } else {
        response["error"] = "No data available";
    }
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleGetHistoryInfo(AsyncWebSocketClient* client, JsonDocument& doc) {
    DynamicJsonDocument response(2048);
    response["cmd"] = "historyInfo";
    
    // Sprawdź czy historia jest włączona
    if (!config.enableHistory) {
        response["enabled"] = false;
        response["error"] = "History system is disabled";
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
            
            if (config.enableFan && historyManager.getFanHistory()) {
                JsonObject fan = sensors.createNestedObject("fan");
                fan["fastSamples"] = historyManager.getFanHistory()->getFastCount();
                fan["slowSamples"] = historyManager.getFanHistory()->getSlowCount();
            }
        } else {
            response["error"] = "History system not initialized";
        }
    }
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleGetAverages(AsyncWebSocketClient* client, JsonDocument& doc) {
    String sensorType = doc["sensor"] | "";
    String avgType = doc["type"] | "fast"; // fast lub slow
    
    DynamicJsonDocument response(2048);
    response["cmd"] = "averages";
    response["sensor"] = sensorType;
    response["type"] = avgType;
    
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
    
    if (sensorType == "i2c" || sensorType == "all") {
        if (avgType == "fast") {
            I2CSensorData avg = getI2CFastAverage();
            if (avg.valid) {
                JsonObject i2c = data.createNestedObject("i2c");
                i2c["temperature"] = avg.temperature;
                i2c["humidity"] = avg.humidity;
                i2c["pressure"] = avg.pressure;
                i2c["co2"] = avg.co2;
                i2c["valid"] = true;
            }
        } else {
            I2CSensorData avg = getI2CSlowAverage();
            if (avg.valid) {
                JsonObject i2c = data.createNestedObject("i2c");
                i2c["temperature"] = avg.temperature;
                i2c["humidity"] = avg.humidity;
                i2c["pressure"] = avg.pressure;
                i2c["co2"] = avg.co2;
                i2c["valid"] = true;
            }
        }
    }
    
    if (sensorType == "sps30" || sensorType == "all") {
        if (avgType == "fast") {
            SPS30Data avg = getSPS30FastAverage();
            if (avg.valid) {
                JsonObject sps30 = data.createNestedObject("sps30");
                sps30["pm1_0"] = avg.pm1_0;
                sps30["pm2_5"] = avg.pm2_5;
                sps30["pm4_0"] = avg.pm4_0;
                sps30["pm10"] = avg.pm10;
                sps30["nc0_5"] = avg.nc0_5;
                sps30["nc1_0"] = avg.nc1_0;
                sps30["nc2_5"] = avg.nc2_5;
                sps30["nc4_0"] = avg.nc4_0;
                sps30["nc10"] = avg.nc10;
                sps30["typical_particle_size"] = avg.typical_particle_size;
                sps30["valid"] = true;
            }
        } else {
            SPS30Data avg = getSPS30SlowAverage();
            if (avg.valid) {
                JsonObject sps30 = data.createNestedObject("sps30");
                sps30["pm1_0"] = avg.pm1_0;
                sps30["pm2_5"] = avg.pm2_5;
                sps30["pm4_0"] = avg.pm4_0;
                sps30["pm10"] = avg.pm10;
                sps30["nc0_5"] = avg.nc0_5;
                sps30["nc1_0"] = avg.nc1_0;
                sps30["nc2_5"] = avg.nc2_5;
                sps30["nc4_0"] = avg.nc4_0;
                sps30["nc10"] = avg.nc10;
                sps30["typical_particle_size"] = avg.typical_particle_size;
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
                hcho["hcho"] = avg.hcho;
                hcho["valid"] = true;
            }
        } else {
            HCHOData avg = getHCHOSlowAverage();
            if (avg.valid) {
                JsonObject hcho = data.createNestedObject("hcho");
                hcho["hcho"] = avg.hcho;
                hcho["valid"] = true;
            }
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
    
    response["success"] = true;
    response["message"] = "Configuration updated";
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleGetConfig(AsyncWebSocketClient* client, JsonDocument& doc) {
    DynamicJsonDocument response(2048);
    response["cmd"] = "getConfig";
    
    JsonObject configObj = response.createNestedObject("config");
    configObj["enableWiFi"] = config.enableWiFi;
    configObj["enableWebServer"] = config.enableWebServer;
    configObj["enableHistory"] = config.enableHistory;
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
    configObj["autoReset"] = config.autoReset;
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleSystemCommand(AsyncWebSocketClient* client, JsonDocument& doc) {
    String command = doc["command"] | "";
    
    DynamicJsonDocument response(1024);
    response["cmd"] = "system";
    response["command"] = command;
    
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

void handleCalibrationCommand(AsyncWebSocketClient* client, JsonDocument& doc) {
    String command = doc["command"] | "";
    
    DynamicJsonDocument response(2048);
    response["cmd"] = "calibration";
    response["command"] = command;
    
    if (command == "start") {
        // Start kalibracji
        response["success"] = true;
        response["message"] = "Calibration started";
        
    } else if (command == "stop") {
        // Stop kalibracji
        response["success"] = true;
        response["message"] = "Calibration stopped";
        
    } else if (command == "reset") {
        // Reset kalibracji
        response["success"] = true;
        response["message"] = "Calibration reset";
        
    } else if (command == "status") {
        // Status kalibracji
        response["success"] = true;
        response["enabled"] = calibConfig.enableCalibration;
        response["valid"] = calibratedData.valid;
        if (calibratedData.valid) {
            JsonObject data = response.createNestedObject("data");
            data["CO"] = calibratedData.CO;
            data["NO"] = calibratedData.NO;
            data["NO2"] = calibratedData.NO2;
            data["O3"] = calibratedData.O3;
            data["SO2"] = calibratedData.SO2;
            data["H2S"] = calibratedData.H2S;
            data["NH3"] = calibratedData.NH3;
            data["VOC"] = calibratedData.VOC;
            data["VOC_ppb"] = calibratedData.VOC_ppb;
            data["HCHO"] = calibratedData.HCHO;
            data["PID"] = calibratedData.PID;
        }
        
    } else {
        response["success"] = false;
        response["error"] = "Unknown calibration command: " + command;
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
        DynamicJsonDocument errorResponse(1024);
        errorResponse["error"] = "Low memory";
        errorResponse["freeHeap"] = ESP.getFreeHeap();
        String errorStr;
        serializeJson(errorResponse, errorStr);
        client->text(errorStr);
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
    } else if (cmd == "calibration") {
        handleCalibrationCommand(client, doc);
    } else {
        DynamicJsonDocument errorResponse(1024);
        errorResponse["error"] = "Unknown command: " + cmd;
        String errorStr;
        serializeJson(errorResponse, errorStr);
        client->text(errorStr);
    }
}

// Funkcja inicjalizacji WebSocket
void initializeWebSocket(AsyncWebSocket& ws) {
    ws.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            safePrintln("WebSocket client connected");
            
            // Wyślij powitalną wiadomość
            DynamicJsonDocument welcome(512);   
            welcome["cmd"] = "welcome";
            welcome["message"] = "WebSocket connected";
            welcome["uptime"] = millis() / 1000;
            welcome["version"] = "1.0";
            
            String welcomeStr;
            serializeJson(welcome, welcomeStr);
            client->text(welcomeStr);
            
        } else if (type == WS_EVT_DISCONNECT) {
            safePrintln("WebSocket client disconnected");
        } else if (type == WS_EVT_DATA) {
            handleWebSocketMessage(client, arg, data, len);
        }
    });
    
    safePrintln("WebSocket initialized");
}

// Funkcja do wysyłania danych do wszystkich klientów WebSocket
void broadcastSensorData(AsyncWebSocket& ws) {
    if (ws.count() == 0) return; // Brak klientów
    
    // Sprawdź pamięć
    if (ESP.getFreeHeap() < 15000) return;
    
    DynamicJsonDocument doc(2048);  
    doc["cmd"] = "update";
    doc["timestamp"] = millis();
    doc["uptime"] = millis() / 1000;
    doc["freeHeap"] = ESP.getFreeHeap();
    
    // Dodaj dane sensorów
    JsonObject sensors = doc.createNestedObject("sensors");
    
    // SHT40
    if (sht40SensorStatus && sht40Data.valid) {
        JsonObject sht40 = sensors.createNestedObject("sht40");
        sht40["temperature"] = sht40Data.temperature;
        sht40["humidity"] = sht40Data.humidity;
        sht40["pressure"] = sht40Data.pressure;
        sht40["valid"] = true;
    }
    
    // SPS30
    if (sps30SensorStatus && sps30Data.valid) {
        JsonObject sps30 = sensors.createNestedObject("sps30");
        sps30["pm1_0"] = sps30Data.pm1_0;
        sps30["pm2_5"] = sps30Data.pm2_5;
        sps30["pm10"] = sps30Data.pm10;
        sps30["valid"] = true;
    }
    
    // CO2
    if (scd41SensorStatus && i2cSensorData.valid && i2cSensorData.type == SENSOR_SCD41) {
        JsonObject co2 = sensors.createNestedObject("co2");
        co2["co2"] = i2cSensorData.co2;
        co2["valid"] = true;
    }
    
    // Power
    if (ina219SensorStatus && ina219Data.valid) {
        JsonObject power = sensors.createNestedObject("power");
        power["busVoltage"] = ina219Data.busVoltage;
        power["current"] = ina219Data.current;
        power["power"] = ina219Data.power;
        power["valid"] = true;
    }
    
    // HCHO
    if (hchoSensorStatus && hchoData.valid) {
        JsonObject hcho = sensors.createNestedObject("hcho");
        hcho["hcho"] = hchoData.hcho;
        hcho["valid"] = true;
    }
    
    // Fan control system
    JsonObject fan = sensors.createNestedObject("fan");
    fan["enabled"] = config.enableFan && isFanEnabled();
    fan["dutyCycle"] = config.enableFan ? getFanDutyCycle() : 0;
    fan["rpm"] = config.enableFan ? getFanRPM() : 0;
    fan["glineEnabled"] = config.enableFan && isGLineEnabled();
    fan["pwmValue"] = config.enableFan ? map(getFanDutyCycle(), 0, 100, 0, 255) : 0;
    fan["valid"] = config.enableFan;
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    
    // Wyślij do wszystkich klientów
    ws.textAll(jsonStr);
}
