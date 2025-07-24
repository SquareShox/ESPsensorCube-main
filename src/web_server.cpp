#include <web_server.h>
#include <update_html.h>
#include <dashboard_html.h>
#include <network_config_html.h>
#include <mcp3424_config_html.h>
#include <charts_html.h>
#include <common_js.h>
#include <sensors.h>
#include <calib.h>
#include <network_config.h>
#include <config.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <time.h>
#include <history.h>
#include <web_socket.h>
#include <ArduinoJson.h>
#include <fan.h>
#include <mean.h>

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
extern SHT40Data sht40Data;
extern HCHOData hchoData;
extern IPSSensorData ipsSensorData;

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

// WebSocket event handler zostanie zastąpiony przez initializeWebSocket

String getAllSensorJson() {
    // Check memory before building JSON
    if (ESP.getFreeHeap() < 15000) {
        DynamicJsonDocument doc(512);
        doc["error"] = "Low memory";
        doc["freeHeap"] = ESP.getFreeHeap();
        String json;
        serializeJson(doc, json);
        return json;
    }
    
    // Create JSON document with appropriate size for all sensor data
    DynamicJsonDocument doc(8192); // 8KB - zmniejszone dla stabilności
    
    doc["t"] = millis();
    doc["uptime"] = millis() / 1000;
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["wifiSignal"] = WiFi.RSSI();
    doc["ntpTime"] = getFormattedTime();
    doc["ntpEpoch"] = getEpochTime();
    doc["ntpDate"] = getFormattedDate();
    doc["ntpValid"] = isTimeSet();
    
    // History status
    extern HistoryManager historyManager;
    JsonObject history = doc.createNestedObject("history");
    history["configEnabled"] = config.enableHistory;
    history["enabled"] = config.enableHistory && historyManager.isInitialized();
    history["memoryUsed"] = config.enableHistory ? historyManager.getTotalMemoryUsed() : 0;
    history["memoryBudget"] = TARGET_MEMORY_BYTES;
    
    doc["psramSize"] = ESP.getPsramSize();
    doc["freePsram"] = ESP.getFreePsram();
    doc["lowPowerMode"] = config.lowPowerMode;
    doc["enablePushbullet"] = config.enablePushbullet;
    
    JsonObject sensorsEnabled = doc.createNestedObject("sensorsEnabled");
    sensorsEnabled["solar"] = solarSensorStatus;
    sensorsEnabled["opcn3"] = opcn3SensorStatus;
    sensorsEnabled["sht40"] = sht40SensorStatus;
    sensorsEnabled["scd41"] = scd41SensorStatus;
    sensorsEnabled["sps30"] = sps30SensorStatus;
    sensorsEnabled["mcp3424"] = mcp3424SensorStatus;
    sensorsEnabled["ads1110"] = ads1110SensorStatus;
    sensorsEnabled["ina219"] = ina219SensorStatus;
    sensorsEnabled["hcho"] = hchoSensorStatus;
    sensorsEnabled["ips"] = ipsSensorStatus;
    sensorsEnabled["fan"] = config.enableFan; // Fan only if enabled
    
    // Solar
    JsonObject solar = doc.createNestedObject("solar");
    if (config.useAveragedData) {
        SolarData avgData = getSolarFastAverage();
        solar["valid"] = solarSensorStatus && avgData.valid;
        if (solarSensorStatus && avgData.valid) {
            solar["V"] = avgData.V;
            solar["I"] = avgData.I;
            solar["VPV"] = avgData.VPV;
            solar["PPV"] = avgData.PPV;
        }
    } else {
        solar["valid"] = solarSensorStatus && solarData.valid;
        if (solarSensorStatus && solarData.valid) {
            solar["V"] = solarData.V;
            solar["I"] = solarData.I;
            solar["VPV"] = solarData.VPV;
            solar["PPV"] = solarData.PPV;
        }
    }
    
    // OPCN3
    JsonObject opcn3 = doc.createNestedObject("opcn3");
    opcn3["valid"] = opcn3SensorStatus && opcn3Data.valid;
    if (opcn3SensorStatus && opcn3Data.valid) {
        opcn3["PM1"] = opcn3Data.pm1;
        opcn3["PM25"] = opcn3Data.pm2_5;
        opcn3["PM10"] = opcn3Data.pm10;
        opcn3["temperature"] = opcn3Data.getTempC();
        opcn3["humidity"] = opcn3Data.getHumidity();
    }
    
    // SPS30
    JsonObject sps30 = doc.createNestedObject("sps30");
    if (config.useAveragedData) {
        SPS30Data avgData = getSPS30FastAverage();
        sps30["valid"] = sps30SensorStatus && avgData.valid;
        if (sps30SensorStatus && avgData.valid) {
            sps30["PM1"] = round(avgData.pm1_0 * 10) / 10.0;
            sps30["PM25"] = round(avgData.pm2_5 * 10) / 10.0;
            sps30["PM4"] = round(avgData.pm4_0 * 10) / 10.0;
            sps30["PM10"] = round(avgData.pm10 * 10) / 10.0;
            sps30["NC05"] = round(avgData.nc0_5 * 10) / 10.0;
            sps30["NC1"] = round(avgData.nc1_0 * 10) / 10.0;
            sps30["NC25"] = round(avgData.nc2_5 * 10) / 10.0;
            sps30["NC4"] = round(avgData.nc4_0 * 10) / 10.0;
            sps30["NC10"] = round(avgData.nc10 * 10) / 10.0;
            sps30["TPS"] = round(avgData.typical_particle_size * 10) / 10.0;
        }
    } else {
        sps30["valid"] = sps30SensorStatus && sps30Data.valid;
        if (sps30SensorStatus && sps30Data.valid) {
            sps30["PM1"] = round(sps30Data.pm1_0 * 10) / 10.0;
            sps30["PM25"] = round(sps30Data.pm2_5 * 10) / 10.0;
            sps30["PM4"] = round(sps30Data.pm4_0 * 10) / 10.0;
            sps30["PM10"] = round(sps30Data.pm10 * 10) / 10.0;
            sps30["NC05"] = round(sps30Data.nc0_5 * 10) / 10.0;
            sps30["NC1"] = round(sps30Data.nc1_0 * 10) / 10.0;
            sps30["NC25"] = round(sps30Data.nc2_5 * 10) / 10.0;
            sps30["NC4"] = round(sps30Data.nc4_0 * 10) / 10.0;
            sps30["NC10"] = round(sps30Data.nc10 * 10) / 10.0;
            sps30["TPS"] = round(sps30Data.typical_particle_size * 10) / 10.0;
        }
    }
    
    // SHT40 (temperature, humidity, pressure)
    JsonObject sht40 = doc.createNestedObject("sht40");
    if (config.useAveragedData) {
        SHT40Data avgData = getSHT40FastAverage();
        sht40["valid"] = sht40SensorStatus && avgData.valid;
        if (sht40SensorStatus && avgData.valid) {
            sht40["temperature"] = round(avgData.temperature * 100) / 100.0;
            sht40["humidity"] = round(avgData.humidity * 100) / 100.0;
            sht40["pressure"] = round(avgData.pressure * 100) / 100.0;
        }
    } else {
        sht40["valid"] = sht40SensorStatus && sht40Data.valid;
        if (sht40SensorStatus && sht40Data.valid) {
            sht40["temperature"] = round(sht40Data.temperature * 100) / 100.0;
            sht40["humidity"] = round(sht40Data.humidity * 100) / 100.0;
            sht40["pressure"] = round(sht40Data.pressure * 100) / 100.0;
        }
    }
    
    // CO2 (SCD41)
    JsonObject scd41 = doc.createNestedObject("scd41");
    scd41["valid"] = scd41SensorStatus;
    if (scd41SensorStatus) {
        scd41["co2"] = i2cSensorData.co2;
    }
    
    // MCP3424 (all devices)
    JsonObject mcp3424 = doc.createNestedObject("mcp3424");
    mcp3424["enabled"] = mcp3424SensorStatus;
    mcp3424["deviceCount"] = mcp3424Data.deviceCount;
    JsonArray devices = mcp3424.createNestedArray("devices");
    for (uint8_t device = 0; device < mcp3424Data.deviceCount; device++) {
        JsonObject deviceObj = devices.createNestedObject();
        deviceObj["address"] = "0x" + String(mcp3424Data.addresses[device], HEX);
        deviceObj["valid"] = mcp3424Data.valid[device];
        deviceObj["resolution"] = mcp3424Data.resolution;
        deviceObj["gain"] = mcp3424Data.gain;
        JsonObject channels = deviceObj.createNestedObject("channels");
        channels["ch1"] = mcp3424Data.channels[device][0];
        channels["ch2"] = mcp3424Data.channels[device][1];
        channels["ch3"] = mcp3424Data.channels[device][2];
        channels["ch4"] = mcp3424Data.channels[device][3];
    }
    
    // ADS1110
    JsonObject ads1110 = doc.createNestedObject("ads1110");
    ads1110["enabled"] = ads1110SensorStatus;
    ads1110["valid"] = ads1110SensorStatus && ads1110Data.valid;
    if (ads1110SensorStatus && ads1110Data.valid) {
        ads1110["voltage"] = round(ads1110Data.voltage * 1000000) / 1000000.0;
        ads1110["dataRate"] = ads1110Data.dataRate;
        ads1110["gain"] = ads1110Data.gain;
    }
    
    // Power
    JsonObject power = doc.createNestedObject("power");
    if (config.useAveragedData) {
        INA219Data avgData = getINA219FastAverage();
        power["valid"] = ina219SensorStatus && avgData.valid;
        if (ina219SensorStatus && avgData.valid) {
            power["busVoltage"] = round(avgData.busVoltage * 1000) / 1000.0;
            power["shuntVoltage"] = round(avgData.shuntVoltage * 100) / 100.0;
            power["current"] = round(avgData.current * 100) / 100.0;
            power["power"] = round(avgData.power * 100) / 100.0;
        }
    } else {
        power["valid"] = ina219SensorStatus && ina219Data.valid;
        if (ina219SensorStatus && ina219Data.valid) {
            power["busVoltage"] = round(ina219Data.busVoltage * 1000) / 1000.0;
            power["shuntVoltage"] = round(ina219Data.shuntVoltage * 100) / 100.0;
            power["current"] = round(ina219Data.current * 100) / 100.0;
            power["power"] = round(ina219Data.power * 100) / 100.0;
        }
    }
    
    // HCHO (formaldehyde sensor)
    JsonObject hcho = doc.createNestedObject("hcho");
    hcho["enabled"] = hchoSensorStatus;
    if (config.useAveragedData) {
        HCHOData avgData = getHCHOFastAverage();
        hcho["valid"] = hchoSensorStatus && avgData.valid;
        if (hchoSensorStatus && avgData.valid) {
            hcho["hcho_mg"] = avgData.hcho;
            hcho["hcho_ppb"] = avgData.hcho_ppb;
            hcho["age"] = (millis() - avgData.lastUpdate) / 1000;
        }
    } else {
        hcho["valid"] = hchoSensorStatus && hchoData.valid;
        if (hchoSensorStatus && hchoData.valid) {
            hcho["hcho_mg"] = hchoData.hcho;
            hcho["hcho_ppb"] = hchoData.hcho_ppb;
            hcho["age"] = (millis() - hchoData.lastUpdate) / 1000;
        }
    }
    
    // IPS (all particle data)
    JsonObject ips = doc.createNestedObject("ips");
    ips["enabled"] = ipsSensorStatus;
    ips["valid"] = ipsSensorStatus && ipsSensorData.valid;
    if (ipsSensorStatus && ipsSensorData.valid) {
        JsonArray pc = ips.createNestedArray("PC");
        JsonArray pm = ips.createNestedArray("PM");
        JsonArray np = ips.createNestedArray("NP");
        JsonArray pw = ips.createNestedArray("PW");
        for (int i = 0; i < 7; i++) {
            pc.add(ipsSensorData.pc_values[i]);
            pm.add(round(ipsSensorData.pm_values[i] * 100) / 100.0);
            np.add(ipsSensorData.np_values[i]);
            pw.add(ipsSensorData.pw_values[i]);
        }
        ips["debugMode"] = ipsSensorData.debugMode;
        ips["won"] = ipsSensorData.won;
    }
    
    // Fan control system
    JsonObject fan = doc.createNestedObject("fan");
    fan["enabled"] = config.enableFan && isFanEnabled();
    fan["dutyCycle"] = config.enableFan ? getFanDutyCycle() : 0;
    fan["rpm"] = config.enableFan ? getFanRPM() : 0;
    fan["glineEnabled"] = config.enableFan && isGLineEnabled();
    fan["pwmValue"] = config.enableFan ? map(getFanDutyCycle(), 0, 100, 0, 255) : 0;
    fan["pwmFreq"] = 25000; // 25kHz
    fan["valid"] = config.enableFan;
    
    // Battery monitoring
    extern BatteryData batteryData;
    JsonObject battery = doc.createNestedObject("battery");
    battery["valid"] = batteryData.valid;
    if (batteryData.valid) {
        battery["voltage"] = round(batteryData.voltage * 1000) / 1000.0;
        battery["current"] = round(batteryData.current * 100) / 100.0;
        battery["power"] = round(batteryData.power * 100) / 100.0;
        battery["chargePercent"] = batteryData.chargePercent;
        battery["isBatteryPowered"] = batteryData.isBatteryPowered;
        battery["lowBattery"] = batteryData.lowBattery;
        battery["criticalBattery"] = batteryData.criticalBattery;
        battery["offPinState"] = digitalRead(OFF_PIN);
        battery["age"] = (millis() - batteryData.lastUpdate) / 1000;
    }
    
    // Calibration data (all sensors if enabled)
    JsonObject calibration = doc.createNestedObject("calibration");
    calibration["enabled"] = calibConfig.enableCalibration;
    if (config.useAveragedData) {
        CalibratedSensorData avgData = getCalibratedFastAverage();
        calibration["valid"] = calibratedData.valid && avgData.valid;
    } else {
        calibration["valid"] = calibratedData.valid;
    }
    if (calibConfig.enableCalibration && calibratedData.valid) {
        // Configuration
        JsonObject configObj = calibration.createNestedObject("config");
        configObj["tgsSensors"] = calibConfig.enableTGSSensors;
        configObj["gasSensors"] = calibConfig.enableGasSensors;
        configObj["ppbConversion"] = calibConfig.enablePPBConversion;
        configObj["specialSensors"] = calibConfig.enableSpecialSensors;
        configObj["movingAverages"] = calibConfig.enableMovingAverages;
        
        // Temperatures (all K sensors)
        JsonObject temperatures = calibration.createNestedObject("temperatures");
        if (!isnan(calibratedData.K1_temp)) temperatures["K1"] = round(calibratedData.K1_temp * 10) / 10.0;
        if (!isnan(calibratedData.K2_temp)) temperatures["K2"] = round(calibratedData.K2_temp * 10) / 10.0;
        if (!isnan(calibratedData.K3_temp)) temperatures["K3"] = round(calibratedData.K3_temp * 10) / 10.0;
        if (!isnan(calibratedData.K4_temp)) temperatures["K4"] = round(calibratedData.K4_temp * 10) / 10.0;
        if (!isnan(calibratedData.K5_temp)) temperatures["K5"] = round(calibratedData.K5_temp * 10) / 10.0;
        if (!isnan(calibratedData.K6_temp)) temperatures["K6"] = round(calibratedData.K6_temp * 10) / 10.0;
        if (!isnan(calibratedData.K7_temp)) temperatures["K7"] = round(calibratedData.K7_temp * 10) / 10.0;
        if (!isnan(calibratedData.K8_temp)) temperatures["K8"] = round(calibratedData.K8_temp * 10) / 10.0;
        if (!isnan(calibratedData.K9_temp)) temperatures["K9"] = round(calibratedData.K9_temp * 10) / 10.0;
        if (!isnan(calibratedData.K12_temp)) temperatures["K12"] = round(calibratedData.K12_temp * 10) / 10.0;
        
        // Voltages (all K sensors)
        JsonObject voltages = calibration.createNestedObject("voltages");
        if (!isnan(calibratedData.K1_voltage)) voltages["K1"] = round(calibratedData.K1_voltage * 100) / 100.0;
        if (!isnan(calibratedData.K2_voltage)) voltages["K2"] = round(calibratedData.K2_voltage * 100) / 100.0;
        if (!isnan(calibratedData.K3_voltage)) voltages["K3"] = round(calibratedData.K3_voltage * 100) / 100.0;
        if (!isnan(calibratedData.K4_voltage)) voltages["K4"] = round(calibratedData.K4_voltage * 100) / 100.0;
        if (!isnan(calibratedData.K5_voltage)) voltages["K5"] = round(calibratedData.K5_voltage * 100) / 100.0;
        if (!isnan(calibratedData.K6_voltage)) voltages["K6"] = round(calibratedData.K6_voltage * 100) / 100.0;
        if (!isnan(calibratedData.K7_voltage)) voltages["K7"] = round(calibratedData.K7_voltage * 100) / 100.0;
        if (!isnan(calibratedData.K8_voltage)) voltages["K8"] = round(calibratedData.K8_voltage * 100) / 100.0;
        if (!isnan(calibratedData.K9_voltage)) voltages["K9"] = round(calibratedData.K9_voltage * 100) / 100.0;
        if (!isnan(calibratedData.K12_voltage)) voltages["K12"] = round(calibratedData.K12_voltage * 100) / 100.0;
        
        // Gases (ug/m3) - if gas sensors enabled
        if (calibConfig.enableGasSensors) {
            JsonObject gases_ugm3 = calibration.createNestedObject("gases_ugm3");
            if (!isnan(calibratedData.CO)) gases_ugm3["CO"] = round(calibratedData.CO * 10) / 10.0;
            if (!isnan(calibratedData.NO)) gases_ugm3["NO"] = round(calibratedData.NO * 10) / 10.0;
            if (!isnan(calibratedData.NO2)) gases_ugm3["NO2"] = round(calibratedData.NO2 * 10) / 10.0;
            if (!isnan(calibratedData.O3)) gases_ugm3["O3"] = round(calibratedData.O3 * 10) / 10.0;
            if (!isnan(calibratedData.SO2)) gases_ugm3["SO2"] = round(calibratedData.SO2 * 10) / 10.0;
            if (!isnan(calibratedData.H2S)) gases_ugm3["H2S"] = round(calibratedData.H2S * 10) / 10.0;
            if (!isnan(calibratedData.NH3)) gases_ugm3["NH3"] = round(calibratedData.NH3 * 10) / 10.0;
            if (!isnan(calibratedData.VOC)) gases_ugm3["VOC"] = round(calibratedData.VOC * 10) / 10.0;
            
            // Gases (ppb) - if PPB conversion enabled
            if (calibConfig.enablePPBConversion) {
                JsonObject gases_ppb = calibration.createNestedObject("gases_ppb");
                if (!isnan(calibratedData.CO_ppb)) gases_ppb["CO"] = round(calibratedData.CO_ppb * 10) / 10.0;
                if (!isnan(calibratedData.NO_ppb)) gases_ppb["NO"] = round(calibratedData.NO_ppb * 10) / 10.0;
                if (!isnan(calibratedData.NO2_ppb)) gases_ppb["NO2"] = round(calibratedData.NO2_ppb * 10) / 10.0;
                if (!isnan(calibratedData.O3_ppb)) gases_ppb["O3"] = round(calibratedData.O3_ppb * 10) / 10.0;
                if (!isnan(calibratedData.SO2_ppb)) gases_ppb["SO2"] = round(calibratedData.SO2_ppb * 10) / 10.0;
                if (!isnan(calibratedData.H2S_ppb)) gases_ppb["H2S"] = round(calibratedData.H2S_ppb * 10) / 10.0;
                if (!isnan(calibratedData.NH3_ppb)) gases_ppb["NH3"] = round(calibratedData.NH3_ppb * 10) / 10.0;
                if (!isnan(calibratedData.VOC_ppb)) gases_ppb["VOC"] = round(calibratedData.VOC_ppb * 10) / 10.0;
            }
        }
        
        // TGS sensors - if TGS sensors enabled
        if (calibConfig.enableTGSSensors) {
            JsonObject tgs = calibration.createNestedObject("tgs");
            if (!isnan(calibratedData.TGS02)) tgs["TGS02"] = round(calibratedData.TGS02 * 1000) / 1000.0;
            if (!isnan(calibratedData.TGS03)) tgs["TGS03"] = round(calibratedData.TGS03 * 1000) / 1000.0;
            if (!isnan(calibratedData.TGS12)) tgs["TGS12"] = round(calibratedData.TGS12 * 1000) / 1000.0;
            if (!isnan(calibratedData.TGS02_ohm)) tgs["TGS02_ohm"] = calibratedData.TGS02_ohm;
            if (!isnan(calibratedData.TGS03_ohm)) tgs["TGS03_ohm"] = calibratedData.TGS03_ohm;
            if (!isnan(calibratedData.TGS12_ohm)) tgs["TGS12_ohm"] = calibratedData.TGS12_ohm;
        }
        
        // Special sensors - if special sensors enabled
        if (calibConfig.enableSpecialSensors) {
            JsonObject special = calibration.createNestedObject("special");
            if (!isnan(calibratedData.HCHO)) special["HCHO_ppb"] = calibratedData.HCHO;
            if (!isnan(calibratedData.PID)) special["PID"] = round(calibratedData.PID * 1000) / 1000.0;
            if (!isnan(calibratedData.PID_mV)) special["PID_mV"] = round(calibratedData.PID_mV * 100) / 100.0;
        }
    }
    
    doc["success"] = true;
    
    String json;
    serializeJson(doc, json);
    return json;
}

void wsBroadcastTask(void *parameter) {
    for (;;) {
        // Skip broadcast if memory is low
        if (ESP.getFreeHeap() > 20000) {
            // Użyj getAllSensorJson zamiast broadcastSensorData
            String jsonData = getAllSensorJson();
            if (jsonData.length() > 0) {
                ws.textAll(jsonData);
                safePrintln("WebSocket broadcast: " + String(jsonData.length()) + " bytes");
            } else {
                safePrintln("WebSocket broadcast: empty JSON");
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
                if (config.enablePushbullet && strlen(config.pushbulletToken) > 0) {
                    // Wait a bit for WiFi to stabilize
                    //delay(5000);
                    
                }
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
    
    // Initialize LittleFS
    if (!initLittleFS()) {
        safePrintln("Failed to initialize LittleFS, using default WiFi config");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    } else {
        // Load network configuration
        if (loadNetworkConfig(networkConfig)) {
            safePrintln("Network config loaded from LittleFS");
            
            // Apply network configuration
            if (!applyNetworkConfig()) {
                safePrintln("Failed to apply network config, using defaults");
            }
        } else {
            safePrintln("No network config found, using defaults");
        }
        
        // Load WiFi credentials
        char ssid[32], password[64];
        if (loadWiFiConfig(ssid, password, sizeof(ssid), sizeof(password))) {
            safePrintln("WiFi config loaded from LittleFS");
            WiFi.begin(ssid, password);
        } else {
            safePrintln("No WiFi config found, using defaults");
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
    }
    
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
    server.on("/network", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", network_config_html);
    });
    server.on("/mcp3424", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", mcp3424_config_html);
    });
    server.on("/common.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/javascript", common_js);
    });
    server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "WebSocket test: " + String(ws.count()) + " clients connected");
    });
    server.on("/api/history", HTTP_GET, [](AsyncWebServerRequest *request) {
        String sensor = "i2c";  // Default to i2c instead of "all"
        String timeRange = "1h";
        String sampleType = "fast";  // Default to fast samples
        
        if (request->hasParam("sensor")) {
            sensor = request->getParam("sensor")->value();
        }
        if (request->hasParam("timeRange")) {
            timeRange = request->getParam("timeRange")->value();
        }
        if (request->hasParam("sampleType")) {
            sampleType = request->getParam("sampleType")->value();
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
        //debug print fromTime and currentTime and info about request
        safePrintln("History request:");
        safePrint("sensor: ");
        safePrintln(sensor);
        safePrint("fromTime: ");
        safePrintln(String(fromTime));
        safePrint("currentTime: ");
        safePrintln(String(currentTime));
        safePrint("timeRange: ");
        safePrintln(timeRange);
        String jsonResponse;
        size_t dataCount = getHistoricalData(sensor, timeRange, jsonResponse, fromTime, currentTime, sampleType);
        
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
    // Inicjalizacja WebSocket z nowym systemem komend
    initializeWebSocket(ws);
    server.addHandler(&ws);
    server.begin();
    safePrintln("Web server started");
    safePrintln("WebSocket initialized and ready");
    xTaskCreatePinnedToCore(wsBroadcastTask, "wsBroadcastTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(WiFiReconnectTask, "wifiReconnectTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(timeCheckTask, "timeCheckTask", 2048, NULL, 1, NULL, 0);
}

 