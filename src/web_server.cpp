#include <web_server.h>
#include <web_socket.h>
#include <update_html.h>
#include <dashboard_html.h>
#include <network_config_html.h>
#include <mcp3424_config_html.h>
#include <charts_html.h>
#include <common_js.h>
#include <sensors.h>
#include <calib.h>
#include <calib_constants.h>
#include <network_config.h>
#include <config.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <time.h>
#include <history.h>
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
    doc["DeviceID"] = config.DeviceID;
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
        scd41["temperature"] = scd41Data.temperature;
        scd41["humidity"] = scd41Data.humidity;
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
            hcho["tvoc_mg"] = avgData.tvoc;
            hcho["age"] = (millis() - avgData.lastUpdate) / 1000;
        }
    } else {
        hcho["valid"] = hchoSensorStatus && hchoData.valid;
        if (hchoSensorStatus && hchoData.valid) {
            hcho["hcho_mg"] = hchoData.hcho;
            hcho["hcho_ppb"] = hchoData.hcho_ppb;
            hcho["tvoc_mg"] = hchoData.tvoc;
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
            
            // ODO
            if (!isnan(calibratedData.ODO)) special["ODO"] = round(calibratedData.ODO * 10) / 10.0;
            
            // PM sensors (SPS30)
            if (!isnan(calibratedData.PM1)) special["PM1"] = round(calibratedData.PM1 * 10) / 10.0;
            if (!isnan(calibratedData.PM25)) special["PM25"] = round(calibratedData.PM25 * 10) / 10.0;
            if (!isnan(calibratedData.PM10)) special["PM10"] = round(calibratedData.PM10 * 10) / 10.0;
            
            // Environmental sensors
         
            
            if (!isnan(calibratedData.DUST_TEMP)) special["DUST_TEMP"] = round(calibratedData.DUST_TEMP * 10) / 10.0;
            if (!isnan(calibratedData.DUST_HUMID)) special["DUST_HUMID"] = round(calibratedData.DUST_HUMID * 10) / 10.0;
            if (!isnan(calibratedData.DUST_PRESS)) special["DUST_PRESS"] = round(calibratedData.DUST_PRESS * 10) / 10.0;
         
            
            // CO2 sensor
            if (!isnan(calibratedData.SCD_CO2)) special["SCD_CO2"] = round(calibratedData.SCD_CO2 * 10) / 10.0;
            if (!isnan(calibratedData.SCD_T)) special["SCD_T"] = round(calibratedData.SCD_T * 10) / 10.0;
            if (!isnan(calibratedData.SCD_RH)) special["SCD_RH"] = round(calibratedData.SCD_RH * 10) / 10.0;
        }
    }
    
    doc["success"] = true;
    
    String json;
    serializeJson(doc, json);
    return json;
}

void wsBroadcastTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10000); // 10 sekund
    
    for (;;) {
        // Sprawdź czy są klienci WebSocket
        if (ws.count() == 0) {
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
            continue;
        }
        
        // Sprawdź pamięć - użyj tych samych progów co WebSocket task
        uint32_t freeHeap = ESP.getFreeHeap();
        if (freeHeap < 20480) { // 20KB minimum heap
            // Jeśli pamięć krytycznie niska, zasugeruj reset WebSocket
            if (freeHeap < 10240) { // 10KB critical heap
                forceWebSocketReset("Critical memory during broadcast");
            }
            vTaskDelayUntil(&xLastWakeTime, xFrequency * 2); // Dłuższe opóźnienie
            continue;
        }
        
        // Sprawdź czy WebSocket nie ma problemów (alternatywnie sprawdź status)
        if (freeHeap < 30720) { // 30KB - dodatkowy buffer dla broadcast task
            vTaskDelayUntil(&xLastWakeTime, xFrequency * 2);
            continue;
        }
        
        // Wyślij dane do wszystkich klientów
        String jsonData = getAllSensorJson();
        if (jsonData.length() > 0 && jsonData.length() < 8192) { // Limit rozmiaru
            ws.textAll(jsonData);
        }
        
        // Regularne opóźnienie
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void timeCheckTask(void *parameter) {
    for (;;) {
        if (WiFi.status() == WL_CONNECTED && !timeInitialized) {
            // Sprawdz czy czas zostal zsynchronizowany
            time_t now = time(nullptr);
            if (now > 8 * 3600 * 2) { // > year 1970
                timeInitialized = true;
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
            
            // Ustaw hostname na Device ID + MAC adres przed ponownym połączeniem
            String hostname = String(config.DeviceID) + "-" + WiFi.macAddress().substring(12); // Ostatnie 6 znaków MAC
            WiFi.setHostname(hostname.c_str());
            safePrintln("Hostname reset to: " + hostname);
            
            // Próbuj połączyć się z konfiguracją z LittleFS, potem z domyślną
            bool connected = false;
            
            // Sprawdź czy LittleFS jest dostępny
            if (initLittleFS()) {
                // Load WiFi credentials z konfiguracji
                char ssid[32], password[64];
                if (loadWiFiConfig(ssid, password, sizeof(ssid), sizeof(password))) {
                    safePrintln("WiFi reconnect: using config from LittleFS");
                    WiFi.begin(ssid, password);
                    
                    int retryCount = 0;
                    while (WiFi.status() != WL_CONNECTED && retryCount < 5) {
                        retryCount++;
                        delay(1000);
                        safePrint("Reconnecting to WiFi (config)... attempt ");
                        safePrintln(String(retryCount));
                    }
                    
                    if (WiFi.status() == WL_CONNECTED) {
                        connected = true;
                    }
                }
            }
            
            // Jeśli nie udało się połączyć z konfiguracją, spróbuj z domyślną
            if (!connected) {
                safePrintln("WiFi reconnect: trying default credentials");
                WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
                
                int retryCount = 0;
                while (WiFi.status() != WL_CONNECTED && retryCount < 5) {
                    retryCount++;
                    delay(1000);
                    safePrint("Reconnecting to WiFi (default)... attempt ");
                    safePrintln(String(retryCount));
                }
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
        vTaskDelay(20000 / portTICK_PERIOD_MS); // Check every 20 seconds
    }
}

void initializeWiFi() {
    if (!config.enableWiFi) return;
    
    // Ustaw hostname na Device ID + MAC adres
    String hostname = String(config.DeviceID) + "-" + WiFi.macAddress().substring(15); // Ostatnie 6 znaków MAC
    WiFi.setHostname(hostname.c_str());
    safePrintln("Hostname set to: " + hostname);
    
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
    
    
    // Skanuj i wyświetl dostępne sieci WiFi
    safePrintln("=== Skanowanie sieci WiFi ===");
    int n = WiFi.scanNetworks();
    if (n == 0) {
        safePrintln("Nie znaleziono sieci WiFi");
    } else {
        safePrintln("Znaleziono " + String(n) + " sieci WiFi:");
        // Wyświetl pierwsze 5 sieci
        int maxNetworks = (n > 5) ? 5 : n;
        for (int i = 0; i < maxNetworks; ++i) {
            safePrint(String(i + 1) + ". " + WiFi.SSID(i));
            safePrint(" (");
            safePrint(String(WiFi.RSSI(i)));
            safePrint(" dBm) ");
            safePrintln((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "OPEN" : "SECURED");
        }
        if (n > 5) {
            safePrintln("... i " + String(n - 5) + " więcej sieci");
        }
    }
    safePrintln("=== Koniec skanowania ===");
    
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
        request->send(200, "text/html", String(update_html) + common_js);
    });
    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", String(dashboard_html) + common_js);
    });
    server.on("/charts", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", String(charts_html) + common_js);
    });
    server.on("/network", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", String(network_config_html) + common_js);
    });
    server.on("/mcp3424", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", String(mcp3424_config_html) + common_js);
    });
    server.on("/common.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/javascript", common_js);
    });
    server.on("/common.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/javascript", common_js);
    });
    server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "WebSocket test: " + String(ws.count()) + " clients connected");
    });
    
    // Calibration constants endpoints
    server.on("/api/calib-constants", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(16384);
        
        // Add all calibration constants to JSON
        doc["RL_TGS03"] = calibConstants.RL_TGS03;
        doc["RL_TGS02"] = calibConstants.RL_TGS02;
        doc["RL_TGS12"] = calibConstants.RL_TGS12;
        doc["TGS03_B1"] = calibConstants.TGS03_B1;
        doc["TGS03_A"] = calibConstants.TGS03_A;
        doc["TGS02_B1"] = calibConstants.TGS02_B1;
        doc["TGS02_A"] = calibConstants.TGS02_A;
        doc["TGS12_B1"] = calibConstants.TGS12_B1;
        doc["TGS12_A"] = calibConstants.TGS12_A;
        doc["CO_B0"] = calibConstants.CO_B0;
        doc["CO_B1"] = calibConstants.CO_B1;
        doc["CO_B2"] = calibConstants.CO_B2;
        doc["CO_B3"] = calibConstants.CO_B3;
        doc["NO_B0"] = calibConstants.NO_B0;
        doc["NO_B1"] = calibConstants.NO_B1;
        doc["NO_B2"] = calibConstants.NO_B2;
        doc["NO_B3"] = calibConstants.NO_B3;
        doc["NO2_B0"] = calibConstants.NO2_B0;
        doc["NO2_B1"] = calibConstants.NO2_B1;
        doc["NO2_B2"] = calibConstants.NO2_B2;
        doc["NO2_B3"] = calibConstants.NO2_B3;
        doc["O3_B0"] = calibConstants.O3_B0;
        doc["O3_B1"] = calibConstants.O3_B1;
        doc["O3_B2"] = calibConstants.O3_B2;
        doc["O3_B3"] = calibConstants.O3_B3;
        doc["O3_D"] = calibConstants.O3_D;
        doc["SO2_B0"] = calibConstants.SO2_B0;
        doc["SO2_B1"] = calibConstants.SO2_B1;
        doc["SO2_B2"] = calibConstants.SO2_B2;
        doc["SO2_B3"] = calibConstants.SO2_B3;
        doc["NH3_B0"] = calibConstants.NH3_B0;
        doc["NH3_B1"] = calibConstants.NH3_B1;
        doc["NH3_B2"] = calibConstants.NH3_B2;
        doc["NH3_B3"] = calibConstants.NH3_B3;
        doc["H2S_B0"] = calibConstants.H2S_B0;
        doc["H2S_B1"] = calibConstants.H2S_B1;
        doc["H2S_B2"] = calibConstants.H2S_B2;
        doc["H2S_B3"] = calibConstants.H2S_B3;
        doc["PID_OFFSET"] = calibConstants.PID_OFFSET;
        doc["PID_B"] = calibConstants.PID_B;
        doc["PID_A"] = calibConstants.PID_A;
        doc["PID_CF"] = calibConstants.PID_CF;
        doc["HCHO_B1"] = calibConstants.HCHO_B1;
        doc["HCHO_A"] = calibConstants.HCHO_A;
        doc["HCHO_PPB_CF"] = calibConstants.HCHO_PPB_CF;
        // PM sensor constants - new naming convention
        doc["PM1_A"] = calibConstants.PM1_A;
        doc["PM1_B"] = calibConstants.PM1_B;
        doc["PM25_A"] = calibConstants.PM25_A;
        doc["PM25_B"] = calibConstants.PM25_B;
        doc["PM10_A"] = calibConstants.PM10_A;
        doc["PM10_B"] = calibConstants.PM10_B;
        
        // Environmental sensor constants - new naming convention
        // Ambient sensors
        doc["AMBIENT_TEMP_A"] = calibConstants.AMBIENT_TEMP_A;
        doc["AMBIENT_TEMP_B"] = calibConstants.AMBIENT_TEMP_B;
        doc["AMBIENT_HUMID_A"] = calibConstants.AMBIENT_HUMID_A;
        doc["AMBIENT_HUMID_B"] = calibConstants.AMBIENT_HUMID_B;
        doc["AMBIENT_PRESS_A"] = calibConstants.AMBIENT_PRESS_A;
        doc["AMBIENT_PRESS_B"] = calibConstants.AMBIENT_PRESS_B;
        
        // Dust sensors
        doc["DUST_TEMP_A"] = calibConstants.DUST_TEMP_A;
        doc["DUST_TEMP_B"] = calibConstants.DUST_TEMP_B;
        doc["DUST_HUMID_A"] = calibConstants.DUST_HUMID_A;
        doc["DUST_HUMID_B"] = calibConstants.DUST_HUMID_B;
        doc["DUST_PRESS_A"] = calibConstants.DUST_PRESS_A;
        doc["DUST_PRESS_B"] = calibConstants.DUST_PRESS_B;
        
        // Gas sensors
        doc["GAS_TEMP_A"] = calibConstants.GAS_TEMP_A;
        doc["GAS_TEMP_B"] = calibConstants.GAS_TEMP_B;
        doc["GAS_HUMID_A"] = calibConstants.GAS_HUMID_A;
        doc["GAS_HUMID_B"] = calibConstants.GAS_HUMID_B;
        doc["GAS_PRESS_A"] = calibConstants.GAS_PRESS_A;
        doc["GAS_PRESS_B"] = calibConstants.GAS_PRESS_B;
        
        // CO2 sensor constants - new naming convention
        doc["SCD_CO2_A"] = calibConstants.SCD_CO2_A;
        doc["SCD_CO2_B"] = calibConstants.SCD_CO2_B;
        doc["SCD_T_A"] = calibConstants.SCD_T_A;
        doc["SCD_T_B"] = calibConstants.SCD_T_B;
        doc["SCD_RH_A"] = calibConstants.SCD_RH_A;
        doc["SCD_RH_B"] = calibConstants.SCD_RH_B;
        doc["ODO_A0"] = calibConstants.ODO_A0;
        doc["ODO_A1"] = calibConstants.ODO_A1;
        doc["ODO_A2"] = calibConstants.ODO_A2;
        doc["ODO_A3"] = calibConstants.ODO_A3;
        doc["ODO_A4"] = calibConstants.ODO_A4;
        doc["ODO_A5"] = calibConstants.ODO_A5;
        doc["B4_TO"] = calibConstants.B4_TO;
        doc["B4_B"] = calibConstants.B4_B;
        doc["B4_RO"] = calibConstants.B4_RO;
        doc["B4_RS"] = calibConstants.B4_RS;
        doc["B4_K"] = calibConstants.B4_K;
        doc["B4_COK"] = calibConstants.B4_COK;
        doc["TGS_TO"] = calibConstants.TGS_TO;
        doc["TGS_B"] = calibConstants.TGS_B;
        doc["TGS_COK"] = calibConstants.TGS_COK;
        doc["B4_LSB"] = calibConstants.B4_LSB;
        doc["TGS_LSB"] = calibConstants.TGS_LSB;
        doc["GAS_MIN"] = calibConstants.GAS_MIN;
        doc["GAS_MAX"] = calibConstants.GAS_MAX;
        doc["HCHO_MIN"] = calibConstants.HCHO_MIN;
        doc["HCHO_MAX"] = calibConstants.HCHO_MAX;
        doc["PID_MIN"] = calibConstants.PID_MIN;
        doc["PID_MAX"] = calibConstants.PID_MAX;
        doc["PM_MIN"] = calibConstants.PM_MIN;
        doc["PM_MAX"] = calibConstants.PM_MAX;
        doc["ENV_MIN"] = calibConstants.ENV_MIN;
        doc["ENV_MAX"] = calibConstants.ENV_MAX;
        doc["SCD_CO2_MIN"] = calibConstants.SCD_CO2_MIN;
        doc["SCD_CO2_MAX"] = calibConstants.SCD_CO2_MAX;
        doc["SCD_T_MIN"] = calibConstants.SCD_T_MIN;
        doc["SCD_T_MAX"] = calibConstants.SCD_T_MAX;
        doc["SCD_RH_MIN"] = calibConstants.SCD_RH_MIN;
        doc["SCD_RH_MAX"] = calibConstants.SCD_RH_MAX;
        doc["ODO_MIN"] = calibConstants.ODO_MIN;
        doc["ODO_MAX"] = calibConstants.ODO_MAX;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    server.on("/api/calib-constants", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("data", true)) {
            String jsonData = request->getParam("data", true)->value();
            DynamicJsonDocument doc(16384);
            DeserializationError error = deserializeJson(doc, jsonData);
            
            if (error) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }
            
            // Update calibration constants from JSON
            calibConstants.RL_TGS03 = doc["RL_TGS03"] | calibConstants.RL_TGS03;
            calibConstants.RL_TGS02 = doc["RL_TGS02"] | calibConstants.RL_TGS02;
            calibConstants.RL_TGS12 = doc["RL_TGS12"] | calibConstants.RL_TGS12;
            calibConstants.TGS03_B1 = doc["TGS03_B1"] | calibConstants.TGS03_B1;
            calibConstants.TGS03_A = doc["TGS03_A"] | calibConstants.TGS03_A;
            calibConstants.TGS02_B1 = doc["TGS02_B1"] | calibConstants.TGS02_B1;
            calibConstants.TGS02_A = doc["TGS02_A"] | calibConstants.TGS02_A;
            calibConstants.TGS12_B1 = doc["TGS12_B1"] | calibConstants.TGS12_B1;
            calibConstants.TGS12_A = doc["TGS12_A"] | calibConstants.TGS12_A;
            calibConstants.CO_B0 = doc["CO_B0"] | calibConstants.CO_B0;
            calibConstants.CO_B1 = doc["CO_B1"] | calibConstants.CO_B1;
            calibConstants.CO_B2 = doc["CO_B2"] | calibConstants.CO_B2;
            calibConstants.CO_B3 = doc["CO_B3"] | calibConstants.CO_B3;
            calibConstants.NO_B0 = doc["NO_B0"] | calibConstants.NO_B0;
            calibConstants.NO_B1 = doc["NO_B1"] | calibConstants.NO_B1;
            calibConstants.NO_B2 = doc["NO_B2"] | calibConstants.NO_B2;
            calibConstants.NO_B3 = doc["NO_B3"] | calibConstants.NO_B3;
            calibConstants.NO2_B0 = doc["NO2_B0"] | calibConstants.NO2_B0;
            calibConstants.NO2_B1 = doc["NO2_B1"] | calibConstants.NO2_B1;
            calibConstants.NO2_B2 = doc["NO2_B2"] | calibConstants.NO2_B2;
            calibConstants.NO2_B3 = doc["NO2_B3"] | calibConstants.NO2_B3;
            calibConstants.O3_B0 = doc["O3_B0"] | calibConstants.O3_B0;
            calibConstants.O3_B1 = doc["O3_B1"] | calibConstants.O3_B1;
            calibConstants.O3_B2 = doc["O3_B2"] | calibConstants.O3_B2;
            calibConstants.O3_B3 = doc["O3_B3"] | calibConstants.O3_B3;
            calibConstants.O3_D = doc["O3_D"] | calibConstants.O3_D;
            calibConstants.SO2_B0 = doc["SO2_B0"] | calibConstants.SO2_B0;
            calibConstants.SO2_B1 = doc["SO2_B1"] | calibConstants.SO2_B1;
            calibConstants.SO2_B2 = doc["SO2_B2"] | calibConstants.SO2_B2;
            calibConstants.SO2_B3 = doc["SO2_B3"] | calibConstants.SO2_B3;
            calibConstants.NH3_B0 = doc["NH3_B0"] | calibConstants.NH3_B0;
            calibConstants.NH3_B1 = doc["NH3_B1"] | calibConstants.NH3_B1;
            calibConstants.NH3_B2 = doc["NH3_B2"] | calibConstants.NH3_B2;
            calibConstants.NH3_B3 = doc["NH3_B3"] | calibConstants.NH3_B3;
            calibConstants.H2S_B0 = doc["H2S_B0"] | calibConstants.H2S_B0;
            calibConstants.H2S_B1 = doc["H2S_B1"] | calibConstants.H2S_B1;
            calibConstants.H2S_B2 = doc["H2S_B2"] | calibConstants.H2S_B2;
            calibConstants.H2S_B3 = doc["H2S_B3"] | calibConstants.H2S_B3;
            calibConstants.PID_OFFSET = doc["PID_OFFSET"] | calibConstants.PID_OFFSET;
            calibConstants.PID_B = doc["PID_B"] | calibConstants.PID_B;
            calibConstants.PID_A = doc["PID_A"] | calibConstants.PID_A;
            calibConstants.PID_CF = doc["PID_CF"] | calibConstants.PID_CF;
            calibConstants.HCHO_B1 = doc["HCHO_B1"] | calibConstants.HCHO_B1;
            calibConstants.HCHO_A = doc["HCHO_A"] | calibConstants.HCHO_A;
            calibConstants.HCHO_PPB_CF = doc["HCHO_PPB_CF"] | calibConstants.HCHO_PPB_CF;
            // PM sensor constants - new naming convention
            calibConstants.PM1_A = doc["PM1_A"] | calibConstants.PM1_A;
            calibConstants.PM1_B = doc["PM1_B"] | calibConstants.PM1_B;
            calibConstants.PM25_A = doc["PM25_A"] | calibConstants.PM25_A;
            calibConstants.PM25_B = doc["PM25_B"] | calibConstants.PM25_B;
            calibConstants.PM10_A = doc["PM10_A"] | calibConstants.PM10_A;
            calibConstants.PM10_B = doc["PM10_B"] | calibConstants.PM10_B;
            
            // Environmental sensor constants - new naming convention
            // Ambient sensors
            calibConstants.AMBIENT_TEMP_A = doc["AMBIENT_TEMP_A"] | calibConstants.AMBIENT_TEMP_A;
            calibConstants.AMBIENT_TEMP_B = doc["AMBIENT_TEMP_B"] | calibConstants.AMBIENT_TEMP_B;
            calibConstants.AMBIENT_HUMID_A = doc["AMBIENT_HUMID_A"] | calibConstants.AMBIENT_HUMID_A;
            calibConstants.AMBIENT_HUMID_B = doc["AMBIENT_HUMID_B"] | calibConstants.AMBIENT_HUMID_B;
            calibConstants.AMBIENT_PRESS_A = doc["AMBIENT_PRESS_A"] | calibConstants.AMBIENT_PRESS_A;
            calibConstants.AMBIENT_PRESS_B = doc["AMBIENT_PRESS_B"] | calibConstants.AMBIENT_PRESS_B;
            
            // Dust sensors
            calibConstants.DUST_TEMP_A = doc["DUST_TEMP_A"] | calibConstants.DUST_TEMP_A;
            calibConstants.DUST_TEMP_B = doc["DUST_TEMP_B"] | calibConstants.DUST_TEMP_B;
            calibConstants.DUST_HUMID_A = doc["DUST_HUMID_A"] | calibConstants.DUST_HUMID_A;
            calibConstants.DUST_HUMID_B = doc["DUST_HUMID_B"] | calibConstants.DUST_HUMID_B;
            calibConstants.DUST_PRESS_A = doc["DUST_PRESS_A"] | calibConstants.DUST_PRESS_A;
            calibConstants.DUST_PRESS_B = doc["DUST_PRESS_B"] | calibConstants.DUST_PRESS_B;
            
            // Gas sensors
            calibConstants.GAS_TEMP_A = doc["GAS_TEMP_A"] | calibConstants.GAS_TEMP_A;
            calibConstants.GAS_TEMP_B = doc["GAS_TEMP_B"] | calibConstants.GAS_TEMP_B;
            calibConstants.GAS_HUMID_A = doc["GAS_HUMID_A"] | calibConstants.GAS_HUMID_A;
            calibConstants.GAS_HUMID_B = doc["GAS_HUMID_B"] | calibConstants.GAS_HUMID_B;
            calibConstants.GAS_PRESS_A = doc["GAS_PRESS_A"] | calibConstants.GAS_PRESS_A;
            calibConstants.GAS_PRESS_B = doc["GAS_PRESS_B"] | calibConstants.GAS_PRESS_B;
            
            // CO2 sensor constants - new naming convention
            calibConstants.SCD_CO2_A = doc["SCD_CO2_A"] | calibConstants.SCD_CO2_A;
            calibConstants.SCD_CO2_B = doc["SCD_CO2_B"] | calibConstants.SCD_CO2_B;
            calibConstants.SCD_T_A = doc["SCD_T_A"] | calibConstants.SCD_T_A;
            calibConstants.SCD_T_B = doc["SCD_T_B"] | calibConstants.SCD_T_B;
            calibConstants.SCD_RH_A = doc["SCD_RH_A"] | calibConstants.SCD_RH_A;
            calibConstants.SCD_RH_B = doc["SCD_RH_B"] | calibConstants.SCD_RH_B;
            calibConstants.ODO_A0 = doc["ODO_A0"] | calibConstants.ODO_A0;
            calibConstants.ODO_A1 = doc["ODO_A1"] | calibConstants.ODO_A1;
            calibConstants.ODO_A2 = doc["ODO_A2"] | calibConstants.ODO_A2;
            calibConstants.ODO_A3 = doc["ODO_A3"] | calibConstants.ODO_A3;
            calibConstants.ODO_A4 = doc["ODO_A4"] | calibConstants.ODO_A4;
            calibConstants.ODO_A5 = doc["ODO_A5"] | calibConstants.ODO_A5;
            calibConstants.B4_TO = doc["B4_TO"] | calibConstants.B4_TO;
            calibConstants.B4_B = doc["B4_B"] | calibConstants.B4_B;
            calibConstants.B4_RO = doc["B4_RO"] | calibConstants.B4_RO;
            calibConstants.B4_RS = doc["B4_RS"] | calibConstants.B4_RS;
            calibConstants.B4_K = doc["B4_K"] | calibConstants.B4_K;
            calibConstants.B4_COK = doc["B4_COK"] | calibConstants.B4_COK;
            calibConstants.TGS_TO = doc["TGS_TO"] | calibConstants.TGS_TO;
            calibConstants.TGS_B = doc["TGS_B"] | calibConstants.TGS_B;
            calibConstants.TGS_COK = doc["TGS_COK"] | calibConstants.TGS_COK;
            calibConstants.B4_LSB = doc["B4_LSB"] | calibConstants.B4_LSB;
            calibConstants.TGS_LSB = doc["TGS_LSB"] | calibConstants.TGS_LSB;
            calibConstants.GAS_MIN = doc["GAS_MIN"] | calibConstants.GAS_MIN;
            calibConstants.GAS_MAX = doc["GAS_MAX"] | calibConstants.GAS_MAX;
            calibConstants.HCHO_MIN = doc["HCHO_MIN"] | calibConstants.HCHO_MIN;
            calibConstants.HCHO_MAX = doc["HCHO_MAX"] | calibConstants.HCHO_MAX;
            calibConstants.PID_MIN = doc["PID_MIN"] | calibConstants.PID_MIN;
            calibConstants.PID_MAX = doc["PID_MAX"] | calibConstants.PID_MAX;
            calibConstants.PM_MIN = doc["PM_MIN"] | calibConstants.PM_MIN;
            calibConstants.PM_MAX = doc["PM_MAX"] | calibConstants.PM_MAX;
            calibConstants.ENV_MIN = doc["ENV_MIN"] | calibConstants.ENV_MIN;
            calibConstants.ENV_MAX = doc["ENV_MAX"] | calibConstants.ENV_MAX;
            calibConstants.SCD_CO2_MIN = doc["SCD_CO2_MIN"] | calibConstants.SCD_CO2_MIN;
            calibConstants.SCD_CO2_MAX = doc["SCD_CO2_MAX"] | calibConstants.SCD_CO2_MAX;
            calibConstants.SCD_T_MIN = doc["SCD_T_MIN"] | calibConstants.SCD_T_MIN;
            calibConstants.SCD_T_MAX = doc["SCD_T_MAX"] | calibConstants.SCD_T_MAX;
            calibConstants.SCD_RH_MIN = doc["SCD_RH_MIN"] | calibConstants.SCD_RH_MIN;
            calibConstants.SCD_RH_MAX = doc["SCD_RH_MAX"] | calibConstants.SCD_RH_MAX;
            calibConstants.ODO_MIN = doc["ODO_MIN"] | calibConstants.ODO_MIN;
            calibConstants.ODO_MAX = doc["ODO_MAX"] | calibConstants.ODO_MAX;
            
            // Save to file
            if (saveCalibrationConstants(calibConstants)) {
                request->send(200, "application/json", "{\"success\":true,\"message\":\"Calibration constants saved\"}");
            } else {
                request->send(500, "application/json", "{\"error\":\"Failed to save calibration constants\"}");
            }
        } else {
            request->send(400, "application/json", "{\"error\":\"No data provided\"}");
        }
    });
    server.on("/api/history", HTTP_GET, [](AsyncWebServerRequest *request) {
        String sensor = "scd41";  // Default to scd41 (CO2 sensor)
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
            safePrint("Firmware Update Start: " + filename);
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
                safePrint("Firmware Update Success: " + String(index + len) + "B");
                safePrintln("Firmware update complete! Rebooting...");
                request->send(200, "text/plain", "Firmware update complete! Rebooting...\n");
                delay(100);
                ESP.restart();
            } else {
                Update.printError(Serial);
            }
        }
    });
    
    // Filesystem update endpoint
    server.on("/updatefs", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            safePrint("Filesystem Update Start: " + filename);
            // Begin filesystem update - use U_FS for filesystem updates
                            if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS)) {
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
                safePrint("Filesystem Update Success: " + String(index + len) + "B");
                safePrintln("Filesystem update complete!");
                request->send(200, "text/plain", "Filesystem update complete!\n");
                // Filesystem update doesn't require restart
            } else {
                Update.printError(Serial);
            }
        }
    });
    // Inicjalizacja WebSocket z nowym systemem komend
    initializeWebSocket(ws);
    
    // Inicjalizacja WebSocket Task System
    if (initializeWebSocketTask()) {
        safePrintln("WebSocket Task system initialized successfully");
    } else {
        safePrintln("WARNING: WebSocket Task system failed to initialize, using fallback mode");
    }
    
    server.addHandler(&ws);
    server.begin();
    xTaskCreatePinnedToCore(wsBroadcastTask, "wsBroadcastTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(WiFiReconnectTask, "wifiReconnectTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(timeCheckTask, "timeCheckTask", 2048, NULL, 1, NULL, 0);
}

 