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
#include <Wire.h>
#include <LittleFS.h>
#include <mean.h>

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

// WebSocket Task Management
static TaskHandle_t webSocketTaskHandle = NULL;
static QueueHandle_t webSocketQueue = NULL;
static SemaphoreHandle_t webSocketSemaphore = NULL;

// Monitoring i resetowanie WebSocket
static unsigned long lastWebSocketActivity = 0;
static unsigned long lastHeapCheck = 0;
static bool webSocketResetPending = false;
static uint32_t webSocketResetCount = 0;

// Struktura dla komunikatów WebSocket
struct WebSocketMessage {
    AsyncWebSocketClient* client;
    String message;
    uint32_t timestamp;
};

#define WEBSOCKET_QUEUE_SIZE 10
#define WEBSOCKET_STACK_SIZE 40960  // 40KB stack
#define WEBSOCKET_PRIORITY 2
#define WEBSOCKET_CORE 1  // Używaj core 1

// Timeouty i limity
#define WEBSOCKET_INACTIVITY_TIMEOUT (5 * 60 * 1000)  // 5 minut
#define HEAP_CHECK_INTERVAL (30 * 1000)               // 30 sekund
#define MIN_HEAP_SIZE 20480                           // 20KB minimum heap
#define CRITICAL_HEAP_SIZE 10240                      // 10KB critical heap

// Struktura do sledzenia klientow WebSocket (uproszczona)
struct WebSocketClientInfo {
    AsyncWebSocketClient* client;
};

#define MAX_WS_CLIENTS 5
WebSocketClientInfo wsClients[MAX_WS_CLIENTS];
int wsClientCount = 0;

// Zmienne globalne dla zarzadzania pamiecia WebSocket


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

// External network flag
extern bool turnOnNetwork;

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
extern SCD41Data getSCD41FastAverage();
extern SCD41Data getSCD41SlowAverage();
extern FanData getFANFastAverage();
extern FanData getFANSlowAverage();

// WebSocket command handlers
void handleGetStatus(AsyncWebSocketClient* client, DynamicJsonDocument& doc) {
    DynamicJsonDocument response(2048);
    response["cmd"] = "status";
    response["timestamp"] = time(nullptr); // Epoch timestamp
    response["uptime"] = millis() / 1000;
    response["freeHeap"] = ESP.getFreeHeap();
    response["freePsram"] = ESP.getFreePsram();
    response["wifiRSSI"] = WiFi.RSSI();
    response["wifiConnected"] = WiFi.status() == WL_CONNECTED;
    
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
    response["timestamp"] = time(nullptr); // Epoch timestamp
    
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
        
        // SCD41 (CO2 sensor) - dedicated bucket, always show when valid
        if (scd41SensorStatus) {
            extern SCD41Data scd41Data;
            if (scd41Data.valid) {
            JsonObject scd41 = data.createNestedObject("scd41");
            scd41["temperature"] = scd41Data.temperature;
            scd41["humidity"] = scd41Data.humidity;
            scd41["co2"] = scd41Data.co2;
            scd41["valid"] = true;
            }
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
            hcho["tvoc_mg"] = hchoData.tvoc;
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
            
            // ODO
            calib["ODO"] = calibratedData.ODO;
            
            // PM sensors (SPS30)
            calib["PM1"] = calibratedData.PM1;
            calib["PM25"] = calibratedData.PM25;
            calib["PM10"] = calibratedData.PM10;
            
            // Environmental sensors
       
            
            calib["DUST_TEMP"] = calibratedData.DUST_TEMP;
            calib["DUST_HUMID"] = calibratedData.DUST_HUMID;
            calib["DUST_PRESS"] = calibratedData.DUST_PRESS;
            
         
            
            // CO2 sensor
            calib["SCD_CO2"] = calibratedData.SCD_CO2;
            calib["SCD_T"] = calibratedData.SCD_T;
            calib["SCD_RH"] = calibratedData.SCD_RH;
            
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
        } else if (sensorType == "scd41" && scd41SensorStatus && i2cSensorData.valid && i2cSensorData.type == SENSOR_SCD41) {
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

// Struktura do przechowywania informacji o automatycznym wysyłaniu pakietów
struct AutoPacketSender {
    AsyncWebSocketClient* client;
    String sensorType;
    String timeRange;
    String sampleType;
    unsigned long fromTime;
    unsigned long toTime;
    int packetSize;
    int currentPacket;
    int totalPackets;
    unsigned long lastSendTime;
};

// Globalny storage dla automatycznych wysyłek (max 5 jednocześnie)
#define MAX_AUTO_SENDERS 5
AutoPacketSender autoSenders[MAX_AUTO_SENDERS];
int autoSenderCount = 0;

// Funkcja do automatycznego wysyłania wszystkich pakietów
void sendHistoryPacketsAutomatically(AsyncWebSocketClient* client, const String& sensorType, 
                                   const String& timeRange, const String& sampleType,
                                   unsigned long fromTime, unsigned long toTime, int packetSize) {
    safePrintln("Starting automatic packet sending for " + sensorType);
    safePrintln("[DEBUG] Auto params: timeRange=" + timeRange + " from=" + String(fromTime) + " to=" + String(toTime) + " packetSize=" + String(packetSize));
    
    // Najpierw pobierz pierwszy pakiet aby sprawdzić łączną liczbę pakietów
    String jsonResponse;
    size_t samples = getHistoricalData(sensorType, timeRange, jsonResponse, fromTime, toTime, sampleType, 0, packetSize);
    
    safePrintln("[DEBUG] First packet response: " + String(samples) + " samples, " + String(jsonResponse.length()) + " bytes");
    
    // Parse response to get total packets info
    DynamicJsonDocument responseDoc(12288); // Zwiększ bufor dla większych odpowiedzi
    DeserializationError parseError = deserializeJson(responseDoc, jsonResponse);
    
    if (parseError) {
        safePrintln("Auto packet sender: JSON parse error for first packet: " + String(parseError.c_str()));
        safePrintln("JSON size: " + String(jsonResponse.length()) + " bytes");
        safePrintln("JSON preview: " + jsonResponse.substring(0, 200));
        return;
    }
    
    // Debug: sprawdź zawartość JSON przed kopiowaniem metadanych
    safePrintln("[DEBUG] JSON keys: hasData=" + String(responseDoc.containsKey("data")) + 
               " hasTotalSamples=" + String(responseDoc.containsKey("totalSamples")) +
               " hasTotalPackets=" + String(responseDoc.containsKey("totalPackets")));
    
    if (responseDoc.containsKey("totalSamples")) {
        safePrintln("[DEBUG] Raw totalSamples type: " + String(responseDoc["totalSamples"].is<int>() ? "int" : "other"));
        safePrintln("[DEBUG] Raw totalSamples value: " + String(responseDoc["totalSamples"].as<int>()));
    }
    
    int totalPackets = responseDoc["totalPackets"] | 1;
    
    // Wyślij pierwszy pakiet
    DynamicJsonDocument response(12288); // Zwiększ rozmiar bufora
    response["cmd"] = "history";
    response["sensor"] = sensorType;
    response["timeRange"] = timeRange;
    response["sampleType"] = sampleType;
    response["fromTime"] = fromTime;
    response["toTime"] = toTime;
    response["samples"] = samples;
    response["timestamp"] = time(nullptr);
    response["autoMode"] = true; // Oznacz że to automatyczny tryb
    
    // Copy data from parsed response
    if (responseDoc.containsKey("data")) {
        response["data"] = responseDoc["data"];
        response["totalSamples"] = responseDoc["totalSamples"];
        response["totalAvailableSamples"] = responseDoc["totalAvailableSamples"];
        response["packetIndex"] = responseDoc["packetIndex"];
        response["packetSize"] = responseDoc["packetSize"];
        response["totalPackets"] = responseDoc["totalPackets"];
        response["hasMorePackets"] = responseDoc["hasMorePackets"];
        response["success"] = responseDoc["success"];
        if (responseDoc.containsKey("error")) {
            response["error"] = responseDoc["error"];
        }
        
        // Debug: sprawdź czy metadane są kopiowane
        safePrintln("[DEBUG] History: totalSamples=" + String(responseDoc["totalSamples"] | 0) + 
                   " totalPackets=" + String(responseDoc["totalPackets"] | 0));
    } else {
        safePrintln("[ERROR] Auto packet: No data field in parsed response");
    }
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
    
    safePrintln("Sent packet 0/" + String(totalPackets) + " for " + sensorType);
    safePrintln("[DEBUG] Auto mode: totalPackets=" + String(totalPackets) + " autoSenderCount=" + String(autoSenderCount));
    
    // Jeśli są więcej pakietów, dodaj do kolejki automatycznego wysyłania
    if (totalPackets > 1 && autoSenderCount < MAX_AUTO_SENDERS) {
        AutoPacketSender& sender = autoSenders[autoSenderCount];
        sender.client = client;
        sender.sensorType = sensorType;
        sender.timeRange = timeRange;
        sender.sampleType = sampleType;
        sender.fromTime = fromTime;
        sender.toTime = toTime;
        sender.packetSize = packetSize;
        sender.currentPacket = 1; // Następny pakiet do wysłania
        sender.totalPackets = totalPackets;
        sender.lastSendTime = millis();
        
        autoSenderCount++;
        safePrintln("Added to auto sender queue. Will send " + String(totalPackets - 1) + " more packets");
    }
}

// Funkcja do przetwarzania kolejki automatycznych wysyłek (wywoływana periodycznie)
void processAutoPacketSenders() {
    unsigned long currentTime = millis();
    
    for (int i = 0; i < autoSenderCount; i++) {
        AutoPacketSender& sender = autoSenders[i];
        
        // Sprawdź czy klient nadal jest połączony
        if (!sender.client || sender.client->status() != WS_CONNECTED) {
            // Usuń z kolejki
            for (int j = i; j < autoSenderCount - 1; j++) {
                autoSenders[j] = autoSenders[j + 1];
            }
            autoSenderCount--;
            i--; // Sprawdź ten sam index ponownie
            continue;
        }
        
        // Sprawdź czy czas na wysłanie kolejnego pakietu (co 100ms)
        if (currentTime - sender.lastSendTime >= 100) {
            if (sender.currentPacket < sender.totalPackets) {
                // Wyślij kolejny pakiet
                String jsonResponse;
                size_t samples = getHistoricalData(sender.sensorType, sender.timeRange, jsonResponse, 
                                                 sender.fromTime, sender.toTime, sender.sampleType, 
                                                 sender.currentPacket, sender.packetSize);
                
                // Parse and send packet
                DynamicJsonDocument responseDoc(12288);
                DeserializationError parseError = deserializeJson(responseDoc, jsonResponse);
                
                if (!parseError && responseDoc.containsKey("data")) {
                    DynamicJsonDocument response(12288); // Zwiększ rozmiar bufora
                    response["cmd"] = "history";
                    response["sensor"] = sender.sensorType;
                    response["timeRange"] = sender.timeRange;
                    response["sampleType"] = sender.sampleType;
                    response["fromTime"] = sender.fromTime;
                    response["toTime"] = sender.toTime;
                    response["samples"] = samples;
                    response["timestamp"] = time(nullptr);
                    response["autoMode"] = true;
                    
                    // Copy data
                    response["data"] = responseDoc["data"];
                    response["totalSamples"] = responseDoc["totalSamples"];
                    response["totalAvailableSamples"] = responseDoc["totalAvailableSamples"];
                    response["packetIndex"] = responseDoc["packetIndex"];
                    response["packetSize"] = responseDoc["packetSize"];
                    response["totalPackets"] = responseDoc["totalPackets"];
                    response["hasMorePackets"] = responseDoc["hasMorePackets"];
                    response["success"] = responseDoc["success"];
                    
                    String responseStr;
                    serializeJson(response, responseStr);
                    sender.client->text(responseStr);
                    
                    safePrintln("Auto sent packet " + String(sender.currentPacket) + "/" + String(sender.totalPackets) + " for " + sender.sensorType + " (" + String(responseStr.length()) + " bytes)");
                } else {
                    safePrintln("[ERROR] Auto packet parse failed or no data for packet " + String(sender.currentPacket) + " sensor " + sender.sensorType);
                }
                
                sender.currentPacket++;
                sender.lastSendTime = currentTime;
            } else {
                // Wszystkie pakiety wysłane, usuń z kolejki
                safePrintln("Auto packet sending completed for " + sender.sensorType);
                for (int j = i; j < autoSenderCount - 1; j++) {
                    autoSenders[j] = autoSenders[j + 1];
                }
                autoSenderCount--;
                i--; // Sprawdź ten sam index ponownie
            }
        }
    }
}

void handleGetHistory(AsyncWebSocketClient* client, JsonDocument& doc) {
    String sensorType = doc["sensor"] | "";
    String timeRange = doc["timeRange"] | "1h";
    String sampleType = doc["sampleType"] | "fast";
    unsigned long fromTime = doc["fromTime"] | 0;
    unsigned long toTime = doc["toTime"] | 0;
    int packetIndex = doc["packetIndex"] | -1; // -1 = automatyczne dzielenie na pakiety
    int packetSize = doc["packetSize"] | 20;   // Rozmiar pakietu
    bool autoSendAllPackets = (packetIndex == -1); // Automatyczne wysyłanie wszystkich pakietów
    
    // Konwersja timeRange na timestampy (epoch seconds) - TYLKO gdy brak fromTime/toTime
    if ((fromTime == 0 && toTime == 0) && timeRange != "custom") {
        unsigned long now = 0;
        if (time(nullptr) > 8 * 3600 * 2) { // Jeśli czas jest zsynchronizowany
            now = time(nullptr); // Używaj sekund, nie milisekund!
        } else {
            now = millis() / 1000; // Fallback do sekund
        }
        
        if (timeRange == "1h") {
            fromTime = now - (60 * 60); // 1 godzina w sekundach
        } else if (timeRange == "6h") {
            fromTime = now - (6 * 60 * 60); // 6 godzin w sekundach
        } else if (timeRange == "24h") {
            fromTime = now - (24 * 60 * 60); // 24 godziny w sekundach
        } else if (timeRange == "7d") {
            fromTime = now - (7 * 24 * 60 * 60); // 7 dni w sekundach
        } else {
            fromTime = now - (60 * 60); // domyślnie 1h w sekundach
        }
        toTime = now;
        
        safePrintln("[DEBUG] Auto timeRange " + timeRange + ": from=" + String(fromTime) + " to=" + String(toTime));
    } else if (timeRange == "custom" && (fromTime != 0 || toTime != 0)) {
        safePrintln("[DEBUG] Custom timeRange: from=" + String(fromTime) + " to=" + String(toTime));
    } else {
        safePrintln("[WARNING] Invalid time parameters or mixed timeRange+fromTime: timeRange=" + timeRange + " fromTime=" + String(fromTime) + " toTime=" + String(toTime));
        // Jeśli użytkownik podał fromTime/toTime, wymuś timeRange="custom"
        if (fromTime != 0 || toTime != 0) {
            timeRange = "custom";
            safePrintln("[FIX] Forced timeRange to 'custom' due to fromTime/toTime");
        }
    }
    
    // Jeśli automatyczne dzielenie, najpierw pobierz informacje o łącznej liczbie próbek
    if (autoSendAllPackets) {
        packetIndex = 0; // Zacznij od pierwszego pakietu
        sendHistoryPacketsAutomatically(client, sensorType, timeRange, sampleType, fromTime, toTime, packetSize);
        return;
    }
    
    String jsonResponse;
    size_t samples = getHistoricalData(sensorType, timeRange, jsonResponse, fromTime, toTime, sampleType, packetIndex, packetSize);
    
    safePrintln("History request: " + sensorType + " " + timeRange + " " + sampleType + " packet: " + String(packetIndex) + "/" + String(packetSize) + " samples: " + String(samples));
    safePrintln("JSON response length: " + String(jsonResponse.length()));
    safePrintln("History enabled: " + String(config.enableHistory));
    safePrintln("History initialized: " + String(historyManager.isInitialized()));
    safePrintln("Free heap: " + String(ESP.getFreeHeap()));
    
    DynamicJsonDocument response(4096);
    response["cmd"] = "history";
    response["sensor"] = sensorType;
    response["timeRange"] = timeRange;
    response["sampleType"] = sampleType;
    response["fromTime"] = fromTime;
    response["toTime"] = toTime;
    response["samples"] = samples;
    response["timestamp"] = time(nullptr); // Epoch timestamp
    

    
    // Parse existing JSON response to check if data exists
    DynamicJsonDocument dataDoc(8192);
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
    response["timestamp"] = time(nullptr); // Epoch timestamp
    
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
                JsonObject scd41 = sensors.createNestedObject("scd41");
                scd41["fastSamples"] = historyManager.getI2CHistory()->getFastCount();
                scd41["slowSamples"] = historyManager.getI2CHistory()->getSlowCount();
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
    response["timestamp"] = time(nullptr); // Epoch timestamp
    
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

    // SCD41 (dedykowany kubelek averages)
    if (sensorType == "scd41" || sensorType == "all") {
        if (avgType == "fast") {
            SCD41Data avg = getSCD41FastAverage();
            if (avg.valid) {
                JsonObject scd41 = data.createNestedObject("scd41");
                scd41["co2"] = avg.co2;
                scd41["temperature"] = avg.temperature;
                scd41["humidity"] = avg.humidity;
                scd41["valid"] = true;
            }
        } else {
            SCD41Data avg = getSCD41SlowAverage();
            if (avg.valid) {
                JsonObject scd41 = data.createNestedObject("scd41");
                scd41["co2"] = avg.co2;
                scd41["temperature"] = avg.temperature;
                scd41["humidity"] = avg.humidity;
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
                hcho["tvoc_mg"] = avg.tvoc;
                hcho["valid"] = true;
            }
        } else {
            HCHOData avg = getHCHOSlowAverage();
            if (avg.valid) {
                JsonObject hcho = data.createNestedObject("hcho");
                hcho["hcho_mg"] = avg.hcho;
                hcho["hcho_ppb"] = avg.hcho_ppb;
                hcho["tvoc_mg"] = avg.tvoc;
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
    
    // K_channels - wszystkie kanały K jako osobne klucze
    // if (sensorType == "mcp3424" || sensorType == "all") {
    //     if (avgType == "fast") {
    //         MCP3424Data avg = getMCP3424FastAverage();
            
    //         if (avg.deviceCount > 0) {
    //             JsonObject k_channels = data.createNestedObject("K_channels");
                
    //             // Użyj systemu mapowania adresów I2C do typów gazów
    //             const char* gasTypes[] = {"NO", "O3", "NO2", "CO", "SO2", "TGS1", "TGS2", "TGS3"};
    //             const char* gasNames[] = {"K1", "K2", "K3", "K4", "K5", "K6", "K7", "K8"};
                
    //             for (int i = 0; i < 8; i++) {
    //                 int8_t deviceIndex = getMCP3424DeviceByGasType(gasTypes[i]);
                    
    //                 // Fallback: jeśli gasType mapping nie działa, użyj deviceIndex bezpośrednio
    //                 if (deviceIndex == -1 && i < (int)avg.deviceCount && avg.valid[i]) {
    //                     deviceIndex = i;
    //                 }
                    
    //                 if (deviceIndex >= 0 && deviceIndex < (int)avg.deviceCount) {
    //                     if (avg.valid[deviceIndex]) {
    //                         // Find actual device index in config based on I2C address
    //                         uint8_t i2cAddress = avg.addresses[deviceIndex];
    //                         int actualDeviceIndex = -1;
                            
    //                         // Search for this I2C address in MCP3424 config to get device index
    //                         extern MCP3424Config mcp3424Config;
    //                         for (int d = 0; d < 8; d++) {
    //                             if (mcp3424Config.devices[d].i2cAddress == i2cAddress) {
    //                                 actualDeviceIndex = d;
    //                                 break;
    //                             }
    //                         }
                            
    //                         // K number = device index + 1 (Device 0->K1, Device 4->K5, Device 6->K7)
    //                         uint8_t kNumber = (actualDeviceIndex >= 0) ? (actualDeviceIndex + 1) : (deviceIndex + 1);
                            
    //                         for (uint8_t ch = 0; ch < 4; ch++) {
    //                             String key = "K" + String(kNumber) + "_" + String(ch+1);
    //                             float value = avg.channels[deviceIndex][ch];
    //                             k_channels[key] = value;
    //                         }
    //                     }
    //                 }
    //             }
    //             k_channels["valid"] = true;
    //         }
    //     } else {
    //         MCP3424Data avg = getMCP3424SlowAverage();
            
    //         if (avg.deviceCount > 0) {
    //             JsonObject k_channels = data.createNestedObject("K_channels");
                
    //             // Użyj systemu mapowania adresów I2C do typów gazów
    //             const char* gasTypes[] = {"NO", "O3", "NO2", "CO", "SO2", "TGS1", "TGS2", "TGS3"};
    //             const char* gasNames[] = {"K1", "K2", "K3", "K4", "K5", "K6", "K7", "K8"};
                
    //             for (int i = 0; i < 8; i++) {
    //                 int8_t deviceIndex = getMCP3424DeviceByGasType(gasTypes[i]);
                    
    //                 // Fallback: jeśli gasType mapping nie działa, użyj deviceIndex bezpośrednio
    //                 if (deviceIndex == -1 && i < (int)avg.deviceCount && avg.valid[i]) {
    //                     deviceIndex = i;
    //                 }
                    
    //                 if (deviceIndex >= 0 && deviceIndex < (int)avg.deviceCount) {
    //                     if (avg.valid[deviceIndex]) {
    //                         // Find actual device index in config based on I2C address
    //                         uint8_t i2cAddress = avg.addresses[deviceIndex];
    //                         int actualDeviceIndex = -1;
                            
    //                         // Search for this I2C address in MCP3424 config to get device index
    //                         extern MCP3424Config mcp3424Config;
    //                         for (int d = 0; d < 8; d++) {
    //                             if (mcp3424Config.devices[d].i2cAddress == i2cAddress) {
    //                                 actualDeviceIndex = d;
    //                                 break;
    //                             }
    //                         }
                            
    //                         // K number = device index + 1 (Device 0->K1, Device 4->K5, Device 6->K7)
    //                         uint8_t kNumber = (actualDeviceIndex >= 0) ? (actualDeviceIndex + 1) : (deviceIndex + 1);
                            
    //                         for (uint8_t ch = 0; ch < 4; ch++) {
    //                             String key = "K" + String(kNumber) + "_" + String(ch+1);
    //                             float value = avg.channels[deviceIndex][ch];
    //                             k_channels[key] = value;
    //                         }
    //                     }
    //                 }
    //             }
    //             k_channels["valid"] = true;
    //         }
    //     }
    // }

    if (sensorType == "mcp3424" || sensorType == "all") {
        if (avgType == "fast") {
            MCP3424Data avg = getMCP3424FastAverage();

            if (avg.deviceCount > 0) {
                JsonObject k_channels = data.createNestedObject("K_channels");

                for (uint8_t deviceIndex = 0; deviceIndex < MAX_MCP3424_DEVICES; deviceIndex++) {
                    uint8_t i2cAddress = getMCP3424I2CAddressByDeviceIndex(deviceIndex);
                    if (i2cAddress == 0x00) {
                        continue; // Device not configured
                    }

                    int idx = -1;
                    for (uint8_t j = 0; j < avg.deviceCount; j++) {
                        if (avg.addresses[j] == i2cAddress) {
                            idx = j;
                            break;
                        }
                    }

                    if (idx >= 0 && avg.valid[idx]) {
                        uint8_t kNumber = deviceIndex + 1;
                        for (uint8_t ch = 0; ch < 4; ch++) {
                            String key = "K" + String(kNumber) + "_" + String(ch + 1);
                            k_channels[key] = avg.channels[idx][ch];
                        }
                    }
                }
                k_channels["valid"] = true;
            }
        } else {
            MCP3424Data avg = getMCP3424SlowAverage();

            if (avg.deviceCount > 0) {
                JsonObject k_channels = data.createNestedObject("K_channels");

                for (uint8_t deviceIndex = 0; deviceIndex < MAX_MCP3424_DEVICES; deviceIndex++) {
                    uint8_t i2cAddress = getMCP3424I2CAddressByDeviceIndex(deviceIndex);
                    if (i2cAddress == 0x00) {
                        continue; // Device not configured
                    }

                    int idx = -1;
                    for (uint8_t j = 0; j < avg.deviceCount; j++) {
                        if (avg.addresses[j] == i2cAddress) {
                            idx = j;
                            break;
                        }
                    }

                    if (idx >= 0 && avg.valid[idx]) {
                        uint8_t kNumber = deviceIndex + 1;
                        for (uint8_t ch = 0; ch < 4; ch++) {
                            String key = "K" + String(kNumber) + "_" + String(ch + 1);
                            k_channels[key] = avg.channels[idx][ch];
                        }
                    }
                }
                k_channels["valid"] = true;
            }
        }
    }

    //fan 
    if (sensorType == "fan" || sensorType == "all") {
        if (avgType == "fast") {
            FanData avg = getFANFastAverage();
            if (avg.valid) {
                JsonObject fan = data.createNestedObject("fan");
                fan["dutyCycle"] = getFanDutyCycle();
                fan["rpm"] = getFanRPM();
                fan["enabled"] = avg.enabled;
                fan["glineEnabled"] = avg.glineEnabled;
                fan["valid"] = true;
            }
        } else {
            FanData avg = getFANSlowAverage();
            if (avg.valid) {
                JsonObject fan = data.createNestedObject("fan");
                fan["dutyCycle"] = getFanDutyCycle();
                fan["rpm"] = getFanRPM();
                fan["enabled"] = avg.enabled;
                fan["glineEnabled"] = avg.glineEnabled;
                fan["valid"] = true;
            }
        }
    }
    
    // Calibrated sensor data
    if (sensorType == "calibrated" || sensorType == "all") {
        if (avgType == "fast") {
            CalibratedSensorData avg = getCalibratedFastAverage();
            if (avg.valid) {
                JsonObject calibrated = data.createNestedObject("calibrated");
                
                // B4 temperatures (K1-K5)
               
                
                // Gases in ug/m3
                calibrated["CO"] = avg.CO;
                calibrated["NO"] = avg.NO;
                calibrated["NO2"] = avg.NO2;
                calibrated["O3"] = avg.O3;
                calibrated["SO2"] = avg.SO2;
                calibrated["H2S"] = avg.H2S;
                calibrated["NH3"] = avg.NH3;
                
                // Gases in ppb
                calibrated["CO_ppb"] = avg.CO_ppb;
                calibrated["NO_ppb"] = avg.NO_ppb;
                calibrated["NO2_ppb"] = avg.NO2_ppb;
                calibrated["O3_ppb"] = avg.O3_ppb;
                calibrated["SO2_ppb"] = avg.SO2_ppb;
                calibrated["H2S_ppb"] = avg.H2S_ppb;
                calibrated["NH3_ppb"] = avg.NH3_ppb;
                
                // TGS sensors
                calibrated["TGS02"] = avg.TGS02;
                calibrated["TGS03"] = avg.TGS03;
                calibrated["TGS12"] = avg.TGS12;
                calibrated["TGS02_ohm"] = avg.TGS02_ohm;
                calibrated["TGS03_ohm"] = avg.TGS03_ohm;
                calibrated["TGS12_ohm"] = avg.TGS12_ohm;
                
                // HCHO and PID
                calibrated["HCHO"] = avg.HCHO;
                calibrated["PID"] = avg.PID;
                calibrated["PID_mV"] = avg.PID_mV;
                
                // VOC
                calibrated["VOC"] = avg.VOC;
                calibrated["VOC_ppb"] = avg.VOC_ppb;
                
                calibrated["valid"] = true;
            }
        } else {
            CalibratedSensorData avg = getCalibratedSlowAverage();
            if (avg.valid) {
                JsonObject calibrated = data.createNestedObject("calibrated");
                
                // B4 temperatures (K1-K5)
                
                
                // Gases in ug/m3
                calibrated["CO"] = avg.CO;
                calibrated["NO"] = avg.NO;
                calibrated["NO2"] = avg.NO2;
                calibrated["O3"] = avg.O3;
                calibrated["SO2"] = avg.SO2;
                calibrated["H2S"] = avg.H2S;
                calibrated["NH3"] = avg.NH3;
                
                // Gases in ppb
                calibrated["CO_ppb"] = avg.CO_ppb;
                calibrated["NO_ppb"] = avg.NO_ppb;
                calibrated["NO2_ppb"] = avg.NO2_ppb;
                calibrated["O3_ppb"] = avg.O3_ppb;
                calibrated["SO2_ppb"] = avg.SO2_ppb;
                calibrated["H2S_ppb"] = avg.H2S_ppb;
                calibrated["NH3_ppb"] = avg.NH3_ppb;
                
                // TGS sensors
                calibrated["TGS02"] = avg.TGS02;
                calibrated["TGS03"] = avg.TGS03;
                calibrated["TGS12"] = avg.TGS12;
                calibrated["TGS02_ohm"] = avg.TGS02_ohm;
                calibrated["TGS03_ohm"] = avg.TGS03_ohm;
                calibrated["TGS12_ohm"] = avg.TGS12_ohm;
                
                // HCHO and PID
                calibrated["HCHO"] = avg.HCHO;
                calibrated["PID"] = avg.PID;
                calibrated["PID_mV"] = avg.PID_mV;
                
                // VOC
                calibrated["VOC"] = avg.VOC;
                calibrated["VOC_ppb"] = avg.VOC_ppb;
                
                // ODO
                calibrated["ODO"] = avg.ODO;
                
                // PM sensors (SPS30)
                calibrated["PM1"] = avg.PM1;
                calibrated["PM25"] = avg.PM25;
                calibrated["PM10"] = avg.PM10;
                
                // Environmental sensors
             
                
                calibrated["DUST_TEMP"] = avg.DUST_TEMP;
                calibrated["DUST_HUMID"] = avg.DUST_HUMID;
                calibrated["DUST_PRESS"] = avg.DUST_PRESS;
                
             
                
                // CO2 sensor
                calibrated["SCD_CO2"] = avg.SCD_CO2;
                calibrated["SCD_T"] = avg.SCD_T;
                calibrated["SCD_RH"] = avg.SCD_RH;
                
                calibrated["valid"] = true;
            }
        }
    }
    
    // ADS1110 ADC converter
    if (sensorType == "ads1110" || sensorType == "all") {
        if (avgType == "fast") {
            ADS1110Data avg = getADS1110FastAverage();
            if (avg.valid) {
                JsonObject ads1110 = data.createNestedObject("ads1110");
                ads1110["voltage"] = avg.voltage;
                ads1110["dataRate"] = avg.dataRate;
                ads1110["gain"] = avg.gain;
                ads1110["valid"] = true;
            }
        } else {
            ADS1110Data avg = getADS1110SlowAverage();
            if (avg.valid) {
                JsonObject ads1110 = data.createNestedObject("ads1110");
                ads1110["voltage"] = avg.voltage;
                ads1110["dataRate"] = avg.dataRate;
                ads1110["gain"] = avg.gain;
                ads1110["valid"] = true;
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
    
    // System status (always included in "all")
    if (sensorType == "system" || sensorType == "all") {
        JsonObject system = data.createNestedObject("system");
        system["uptime"] = millis()/1000;
        system["freeHeap"] = ESP.getFreeHeap();
        system["wifiSignal"] = WiFi.RSSI();
        system["ntpTime"] = getFormattedTime();
        system["DeviceID"] = config.DeviceID; // Add DeviceID from config
        system["valid"] = true;
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
    
    if (doc.containsKey("DeviceID")) {
        strncpy(config.DeviceID, doc["DeviceID"] | "SCUBE-001", sizeof(config.DeviceID) - 1);
        config.DeviceID[sizeof(config.DeviceID) - 1] = '\0';
        response["DeviceID"] = config.DeviceID;
        
        // Save configuration to file
        if (saveSystemConfig(config)) {
            response["success"] = true;
            response["message"] = "Device ID saved";
        } else {
            response["success"] = false;
            response["error"] = "Failed to save configuration";
        }
    }
    
    // Save system configuration to file if any changes were made
    if (doc.containsKey("enableWiFi") || doc.containsKey("enableHistory") || 
        doc.containsKey("useAveragedData") || doc.containsKey("enableModbus") ||
        doc.containsKey("enableSolarSensor") || doc.containsKey("enableSPS30") ||
        doc.containsKey("enableSHT40") || doc.containsKey("enableHCHO") ||
        doc.containsKey("enableINA219") || doc.containsKey("enableSHT30") ||
        doc.containsKey("enableBME280") || doc.containsKey("enableSCD41") ||
        doc.containsKey("enableI2CSensors") || doc.containsKey("enableMCP3424") ||
        doc.containsKey("enableADS1110") || doc.containsKey("enableOPCN3Sensor") ||
        doc.containsKey("enableIPS") || doc.containsKey("enableIPSDebug") ||
        doc.containsKey("enableWebServer") || doc.containsKey("enableFan") ||
        doc.containsKey("autoReset") || doc.containsKey("lowPowerMode") ||
        doc.containsKey("DeviceID")) {
        
        if (saveSystemConfig(config)) {
            response["success"] = true;
            response["message"] = "Configuration updated and saved";
            response["savedToFile"] = true;
            safePrintln("SUCCESS: System config saved to file");
        } else {
            response["success"] = false;
            response["error"] = "Configuration updated in memory but failed to save to file";
            response["savedToFile"] = false;
            safePrintln("ERROR: Failed to save system config to file");
        }
    } else {
        response["success"] = true;
        response["message"] = "Configuration updated";
    }
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}

void handleGetConfig(AsyncWebSocketClient* client, JsonDocument& doc) {
    DynamicJsonDocument response(2048);
    response["cmd"] = "getConfig";
    response["timestamp"] = time(nullptr); // Epoch timestamp
    
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
    configObj["DeviceID"] = config.DeviceID;
    
    response["success"] = true;
    
    String responseStr;
    serializeJson(response, responseStr);
    client->text(responseStr);
}





// Główna funkcja obsługi WebSocket
void handleWebSocketMessage(AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len) {
    // Aktualizuj czas ostatniej aktywności
    lastWebSocketActivity = millis();
    
    // Sprawdź czy WebSocket task jest inicjalizowany
    if (!webSocketTaskHandle || !webSocketQueue) {
        safePrintln("WebSocket: Task system not initialized, falling back to direct processing");
        
        // Fallback do bezpośredniego przetwarzania w przypadku problemów z task
        String message = String((char*)data, len);
        
        DynamicJsonDocument doc(4096); // Zwiększony buffer dla MCP3424 config
        DeserializationError error = deserializeJson(doc, message);
        
        if (!error) {
            // Wywołaj stary handler bezpośrednio
            handleWebSocketMessageInTask(client, doc);
        } else {
            DynamicJsonDocument errorResponse(256);
            errorResponse["error"] = "Invalid JSON format";
            errorResponse["fallback"] = true;
            
            String errorStr;
            serializeJson(errorResponse, errorStr);
            client->text(errorStr);
        }
        return;
    }
    
    // Sprawdź czy kolejka nie jest pełna
    if (uxQueueSpacesAvailable(webSocketQueue) == 0) {
        safePrintln("WebSocket: Queue full, dropping message");
        
        DynamicJsonDocument errorResponse(256);
        errorResponse["error"] = "Server busy - queue full";
        errorResponse["queueSize"] = WEBSOCKET_QUEUE_SIZE;
        
        String errorStr;
        serializeJson(errorResponse, errorStr);
        client->text(errorStr);
        return;
    }
    
    // Konwertuj dane na string
    String message = String((char*)data, len);
    
    // Wyślij do WebSocket task
    if (!sendToWebSocketTask(client, message)) {
        // Fallback jeśli wysyłanie nie powiodło się
        safePrintln("WebSocket: Failed to send to task, using fallback");
        
        DynamicJsonDocument doc(4096); // Zwiększony buffer dla MCP3424 config
        DeserializationError error = deserializeJson(doc, message);
        
        if (!error) {
            handleWebSocketMessageInTask(client, doc);
        } else {
            safePrintln("WebSocket: JSON parse error: " + String(error.c_str()));
            safePrintln("Problematic message: '" + message + "'");
            safePrintln("Message length: " + String(message.length()));
            
            DynamicJsonDocument errorResponse(256);
            errorResponse["error"] = "Invalid JSON format";
            errorResponse["fallback"] = true;
            errorResponse["details"] = String(error.c_str());
            errorResponse["messageLength"] = message.length();
            
            String errorStr;
            serializeJson(errorResponse, errorStr);
            client->text(errorStr);
        }
    }
}

// Funkcja dodawania klienta do sledzenia
void addWebSocketClient(AsyncWebSocketClient* client) {
    if (wsClientCount < MAX_WS_CLIENTS) {
        wsClients[wsClientCount].client = client;
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

// Funkcja aktualizacji stanu klienta (uproszczona)
void updateClientPongTime(AsyncWebSocketClient* client) {
    // Funkcja zachowana dla kompatybilności z event handlerami
    // Natywny ping/pong jest obsługiwany przez bibliotekę AsyncWebSocket
}

// Funkcja wysylania natywnego ping WebSocket
void sendNativePing(AsyncWebSocketClient* client) {
    if (client && client->status() == 1) { // WS_CONNECTED = 1
        client->ping();
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
            
            // Sprawdź pamięć po rozłączeniu
            if (ESP.getFreeHeap() < MIN_HEAP_SIZE) {
                safePrintln("WebSocket: Low memory after disconnect - scheduling reset");
                webSocketResetPending = true;
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







// Główny task WebSocket do przetwarzania komunikatów
void webSocketTask(void* parameters) {
    safePrintln("WebSocket Task: Started with " + String(WEBSOCKET_STACK_SIZE) + " bytes stack");
    
    WebSocketMessage msg;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 100ms cycle
    
    // Inicjalizuj timery
    lastWebSocketActivity = millis();
    lastHeapCheck = millis();
    
    while (true) {
        // Sprawdź czy jest oczekujący reset
        if (webSocketResetPending) {
            resetWebSocket();
        }
        
        // Sprawdź heap i aktywność co 100ms
        checkHeapAndReset();
        checkWebSocketActivity();
        
        // Monitor stack usage
        UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        if (stackHighWaterMark < 512) { // Mniej niż 512 bytes pozostało
            safePrintln("WebSocket Task: WARNING - Low stack space: " + String(stackHighWaterMark) + " bytes remaining");
        }
        
        // Sprawdź dostępną RAM przed przetwarzaniem
        uint32_t freeHeap = ESP.getFreeHeap();
        if (freeHeap < MIN_HEAP_SIZE) {
            safePrintln("WebSocket Task: Low memory (" + String(freeHeap) + " bytes), skipping processing");
            vTaskDelayUntil(&xLastWakeTime, xFrequency * 5); // Dłuższe opóźnienie przy niskiej pamięci
            continue;
        }
        
        // Odbierz komunikat z kolejki (z timeout)
        if (xQueueReceive(webSocketQueue, &msg, pdMS_TO_TICKS(50)) == pdTRUE) {
            // Sprawdź czy komunikat nie jest za stary
            if ((millis() - msg.timestamp) > 5000) {
                safePrintln("WebSocket Task: Dropping old message (" + 
                           String(millis() - msg.timestamp) + "ms old)");
                continue;
            }
            
            // Sprawdź czy klient nadal jest połączony
            if (!msg.client || msg.client->status() != WS_CONNECTED) {
                safePrintln("WebSocket Task: Client disconnected, dropping message");
                continue;
            }
            
            // Zaktualizuj czas ostatniej aktywności
            lastWebSocketActivity = millis();
            
            // Przetwórz komunikat JSON - zwiększony buffer dla MCP3424 config (8 devices = ~1KB)
            DynamicJsonDocument doc(4096);
            DeserializationError error = deserializeJson(doc, msg.message);
            
            if (error) {
                safePrintln("WebSocket Task: JSON parse error: " + String(error.c_str()));
                safePrintln("Problematic message: '" + msg.message + "'");
                safePrintln("Message length: " + String(msg.message.length()));
                
                DynamicJsonDocument errorResponse(256);
                errorResponse["error"] = "Invalid JSON format";
                errorResponse["code"] = "PARSE_ERROR";
                errorResponse["details"] = String(error.c_str());
                errorResponse["messageLength"] = msg.message.length();
                
                String errorStr;
                serializeJson(errorResponse, errorStr);
                msg.client->text(errorStr);
                continue;
            }
            
            // Obsłuż komunikat w kontekście task
            handleWebSocketMessageInTask(msg.client, doc);
            
        } else {
            // Timeout - sprawdź stan systemu
            if (freeHeap < CRITICAL_HEAP_SIZE) {
                safePrintln("WebSocket Task: Critical heap detected during idle");
                webSocketResetPending = true;
            }
        }
        
        // Przetwórz automatyczne wysyłanie pakietów
        processAutoPacketSenders();
        
        // Regularne opóźnienie
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// Handler komunikatów WebSocket w kontekście task
void handleWebSocketMessageInTask(AsyncWebSocketClient* client, DynamicJsonDocument& doc) {
    safePrintln("WebSocket Task: Processing command in dedicated task context");
    
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
    } else if (cmd == "pingpong") {
        // Ping/pong w task context
        DynamicJsonDocument response(256);
        response["cmd"] = "pingpong";
        response["command"] = "pong";
        response["timestamp"] = time(nullptr); // Epoch timestamp
        response["freeHeap"] = ESP.getFreeHeap();
        response["taskStack"] = uxTaskGetStackHighWaterMark(NULL);
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
    } else if (cmd == "getNetworkConfig") {
        // Inline network config handler
        DynamicJsonDocument response(2048);
        response["success"] = true;
        response["cmd"] = "networkConfig";
        
        response["useDHCP"] = networkConfig.useDHCP;
        response["staticIP"] = networkConfig.staticIP;
        response["gateway"] = networkConfig.gateway;
        response["subnet"] = networkConfig.subnet;
        response["dns1"] = networkConfig.dns1;
        response["dns2"] = networkConfig.dns2;
        response["configValid"] = networkConfig.configValid;
        
        response["currentIP"] = WiFi.localIP().toString();
        response["currentSSID"] = WiFi.SSID();
        response["wifiConnected"] = WiFi.status() == WL_CONNECTED;
        response["wifiSignal"] = WiFi.RSSI();
        
        char ssid[32], password[64];
        if (loadWiFiConfig(ssid, password, sizeof(ssid), sizeof(password))) {
            response["wifiSSID"] = ssid;
            // Nie zwracamy hasła ze względów bezpieczeństwa
            response["wifiPasswordSet"] = (strlen(password) > 0);
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
    } else if (cmd == "setWiFiConfig") {
        // Inline WiFi config handler
        String ssid = doc["ssid"] | "";
        String password = doc["password"] | "";
        
        DynamicJsonDocument response(1024);
        response["cmd"] = "setWiFiConfig";
        
        if (ssid.length() > 0) {
            // Jeśli hasło jest puste, zachowaj istniejące hasło
            if (password.length() == 0) {
                char currentSsid[32], currentPassword[64];
                if (loadWiFiConfig(currentSsid, currentPassword, sizeof(currentSsid), sizeof(currentPassword))) {
                    password = String(currentPassword);
                    safePrintln("WiFi: Keeping existing password for SSID: " + ssid);
                }
            }
            
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
    } else if (cmd == "setNetworkFlag") {
        // Handle network flag setting
        bool enabled = doc["enabled"] | false;
        turnOnNetwork = enabled;
        
        DynamicJsonDocument response(512);
        response["cmd"] = "setNetworkFlag";
        response["success"] = true;
        response["enabled"] = enabled;
        response["message"] = enabled ? "Network flag enabled" : "Network flag disabled";
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
        safePrintln("Network flag set to: " + String(enabled ? "ON" : "OFF"));
        
    } else if (cmd == "setNetworkConfig") {
        // Inline network config handler
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
        response["timestamp"] = time(nullptr); // Epoch timestamp
        //heap
        // response["freeHeap"] = ESP.getFreeHeap();
        // //wifi
        // response["wifiSignal"] = WiFi.RSSI();
      
        
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
        // JsonObject mcp3424 = data.createNestedObject("mcp3424");
        // mcp3424["enabled"] = "MCP3424_ENABLED";
        // mcp3424["deviceCount"] = "MCP3424_DEVICE_COUNT";
        // mcp3424["valid"] = "MCP3424_VALID";
        
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
//sleep time
        // Calibrated sensor data keys
        JsonObject calibrated = data.createNestedObject("calibrated");
        calibrated["valid"] = "CALIBRATED_VALID";
        
        // Gases in ug/m3
        calibrated["CO"] = "CALIBRATED_CO";
        calibrated["NO"] = "CALIBRATED_NO";
        calibrated["NO2"] = "CALIBRATED_NO2";
        calibrated["O3"] = "CALIBRATED_O3";
        calibrated["SO2"] = "CALIBRATED_SO2";
        calibrated["H2S"] = "CALIBRATED_H2S";
        calibrated["NH3"] = "CALIBRATED_NH3";
        
        // Gases in ppb
        calibrated["CO_ppb"] = "CALIBRATED_CO_PPB";
        calibrated["NO_ppb"] = "CALIBRATED_NO_PPB";
        calibrated["NO2_ppb"] = "CALIBRATED_NO2_PPB";
        calibrated["O3_ppb"] = "CALIBRATED_O3_PPB";
        calibrated["SO2_ppb"] = "CALIBRATED_SO2_PPB";
        calibrated["H2S_ppb"] = "CALIBRATED_H2S_PPB";
        calibrated["NH3_ppb"] = "CALIBRATED_NH3_PPB";
        
        // TGS sensors
        calibrated["TGS02"] = "CALIBRATED_TGS02";
        calibrated["TGS03"] = "CALIBRATED_TGS03";
        calibrated["TGS12"] = "CALIBRATED_TGS12";
        calibrated["TGS02_ohm"] = "CALIBRATED_TGS02_OHM";
        calibrated["TGS03_ohm"] = "CALIBRATED_TGS03_OHM";
        calibrated["TGS12_ohm"] = "CALIBRATED_TGS12_OHM";
        
        // HCHO and PID
        calibrated["HCHO"] = "CALIBRATED_HCHO";
        calibrated["PID"] = "CALIBRATED_PID";
        calibrated["PID_mV"] = "CALIBRATED_PID_MV";
        
        // VOC
        calibrated["VOC"] = "CALIBRATED_VOC";
        calibrated["VOC_ppb"] = "CALIBRATED_VOC_PPB";
        
        // ADS1110 ADC converter
        JsonObject ads1110 = data.createNestedObject("ads1110");
        ads1110["valid"] = "ADS1110_VALID";
        ads1110["voltage"] = "ADS1110_VOLTAGE";
        ads1110["dataRate"] = "ADS1110_DATA_RATE";
        ads1110["gain"] = "ADS1110_GAIN";
        
        // Fan control system (updated from existing fan section)
        JsonObject fan = data.createNestedObject("fan");
        fan["valid"] = "FAN_VALID";
        fan["dutyCycle"] = "FAN_DUTY_CYCLE";
        fan["rpm"] = "FAN_RPM";
        fan["enabled"] = "FAN_ENABLED";
        fan["glineEnabled"] = "FAN_GLINE_ENABLED";
        
        // System status
        JsonObject system = data.createNestedObject("system");
        system["valid"] = "SYSTEM_VALID";
        system["uptime"] = "SYSTEM_UPTIME";
        system["freeHeap"] = "SYSTEM_FREE_HEAP";
        system["wifiSignal"] = "SYSTEM_WIFI_SIGNAL";
        system["ntpTime"] = "SYSTEM_NTP_TIME";
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "getMCP3424Config") {
        // Handle MCP3424 config request - reload from LittleFS first
        safePrintln("WebSocket: getMCP3424Config requested - reloading from LittleFS");
        
        // Reload config from file to ensure latest data
        if (!loadMCP3424Config(mcp3424Config)) {
            safePrintln("Failed to load MCP3424 config from LittleFS, using current memory");
        } else {
            safePrintln("MCP3424 config reloaded successfully from LittleFS");
        }
        
        DynamicJsonDocument response(2048);
        response["cmd"] = "mcp3424Config";
        response["success"] = true;
        
        // Manually serialize MCP3424Config to JSON
        JsonObject configObj = response.createNestedObject("config");
        configObj["deviceCount"] = mcp3424Config.deviceCount;
        configObj["configValid"] = mcp3424Config.configValid;
        
        JsonArray devices = configObj.createNestedArray("devices");
        safePrintln("MCP3424 config: deviceCount = " + String(mcp3424Config.deviceCount));
        
        // ALWAYS return all 8 devices for frontend UI (not just enabled ones)
        for (uint8_t i = 0; i < 8; i++) {
            JsonObject device = devices.createNestedObject();
            device["deviceIndex"] = i;
            device["i2cAddress"] = mcp3424Config.devices[i].i2cAddress;
            device["gasType"] = mcp3424Config.devices[i].gasType;
            device["description"] = mcp3424Config.devices[i].description;
            device["enabled"] = mcp3424Config.devices[i].enabled;
            device["autoDetected"] = mcp3424Config.devices[i].autoDetected;
            
            safePrintln("Device " + String(i) + ": '" + String(mcp3424Config.devices[i].gasType) + 
                       "' @ 0x" + String(mcp3424Config.devices[i].i2cAddress, HEX) + 
                       " (enabled: " + String(mcp3424Config.devices[i].enabled ? "true" : "false") + ")");
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        safePrintln("Sending MCP3424 config response: " + responseStr);
        client->text(responseStr);
        
    } else if (cmd == "scanI2CAddresses") {
        // I2C scan command
        DynamicJsonDocument response(1024);
        response["cmd"] = "i2cScanResult";
        response["timestamp"] = time(nullptr);
        
        JsonArray foundAddresses = response.createNestedArray("foundAddresses");
        int foundCount = 0;
        
        // Scan I2C addresses typical for MCP3424 (0x68-0x6F)
        safePrintln("WebSocket: Starting I2C scan for MCP3424 devices...");
        for (uint8_t addr = 0x68; addr <= 0x6F; addr++) {
            if (addr == 0x69) continue; // Skip excluded address
            
            Wire.beginTransmission(addr);
            uint8_t result = Wire.endTransmission();
            
            if (result == 0) {
                // Device found
                foundAddresses.add(addr);
                foundCount++;
                safePrintln("I2C device found at 0x" + String(addr, HEX));
            }
        }
        
        response["success"] = true;
        response["message"] = "I2C scan completed. Found " + String(foundCount) + " devices.";
        response["foundCount"] = foundCount;
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "getWebSocketStatus") {
        // Komenda diagnostyczna
        DynamicJsonDocument response(1024);
        response["cmd"] = "webSocketStatus";
        response["success"] = true;
        response["status"] = getWebSocketStatus();
        response["timestamp"] = time(nullptr); // Epoch timestamp
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
    } else if (cmd == "forceWebSocketReset") {
        // Komenda do ręcznego resetowania
        String reason = doc["reason"] | "Manual reset from client";
        forceWebSocketReset(reason);
        
        DynamicJsonDocument response(256);
        response["cmd"] = "webSocketResetScheduled";
        response["success"] = true;
        response["reason"] = reason;
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
    } else if (cmd == "restart" || cmd == "reset") {
        // System restart/reset commands
        DynamicJsonDocument response(512);
        response["cmd"] = cmd;
        response["success"] = true;
        response["message"] = "System " + cmd + "ing...";
        response["timestamp"] = time(nullptr); // Epoch timestamp
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
        delay(1000);
        ESP.restart();
        
    } else if (cmd == "memory") {
        // Memory status command
        DynamicJsonDocument response(512);
        response["cmd"] = "memory";
        response["success"] = true;
        response["freeHeap"] = ESP.getFreeHeap();
        response["freePsram"] = ESP.getFreePsram();
        response["psramSize"] = ESP.getPsramSize();
        response["uptime"] = millis() / 1000;
        response["timestamp"] = time(nullptr); // Epoch timestamp
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "lowPowerOn" || cmd == "lowPowerOff") {
        // Low power mode commands
        config.lowPowerMode = (cmd == "lowPowerOn");
        
        DynamicJsonDocument response(512);
        response["cmd"] = cmd;
        response["success"] = true;
        response["message"] = String("Low power mode ") + (config.lowPowerMode ? "enabled" : "disabled");
        response["lowPowerMode"] = config.lowPowerMode;
        response["timestamp"] = time(nullptr); // Epoch timestamp
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "pushbulletTest") {
        // Pushbullet test notification
        DynamicJsonDocument response(512);
        response["cmd"] = "pushbulletTest";
        response["timestamp"] = time(nullptr); // Epoch timestamp
        
        if (config.enablePushbullet && strlen(config.pushbulletToken) > 0) {
            sendPushbulletNotification("🧪 Test Notification", "This is a test notification from ESP32 Sensor Cube\nTime: " + getFormattedTime() + "\nUptime: " + getUptimeString());
            response["success"] = true;
            response["message"] = "Test notification sent";
        } else {
            response["success"] = false;
            response["message"] = "Pushbullet not configured";
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "pushbulletBatteryTest") {
        // Pushbullet battery test notification
        DynamicJsonDocument response(512);
        response["cmd"] = "pushbulletBatteryTest";
        response["timestamp"] = time(nullptr); // Epoch timestamp
        
        if (config.enablePushbullet && strlen(config.pushbulletToken) > 0) {
            sendBatteryCriticalNotification();
            digitalWrite(OFF_PIN, HIGH);
            response["success"] = true;
            response["message"] = "Battery critical test notification sent";
        } else {
            response["success"] = false;
            response["message"] = "Pushbullet not configured";
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "wifi") {
        // WiFi status command
        DynamicJsonDocument response(512);
        response["cmd"] = "wifi";
        response["success"] = true;
        response["connected"] = WiFi.status() == WL_CONNECTED;
        response["ssid"] = WiFi.SSID();
        response["rssi"] = WiFi.RSSI();
        response["localIP"] = WiFi.localIP().toString();
        response["timestamp"] = time(nullptr); // Epoch timestamp
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "fan_speed" || cmd == "fan_on" || cmd == "fan_off" || 
               cmd == "gline_on" || cmd == "gline_off" || cmd == "sleep" || 
               cmd == "sleep_stop" || cmd == "wake") {
        // Fan control commands
        DynamicJsonDocument response(1024);
        response["cmd"] = cmd;
        response["timestamp"] = time(nullptr); // Epoch timestamp
        
        safePrintln("=== Fan command received: " + cmd + " ===");
        safePrintln("config.enableFan = " + String(config.enableFan ? "true" : "false"));
        
        if (!config.enableFan) {
            response["success"] = false;
            response["error"] = "Fan control is DISABLED in configuration";
            safePrintln("Fan control DISABLED - returning error");
        } else {
            String value = "";
            if (cmd == "fan_speed") {
                safePrintln("=== Processing fan_speed command ===");
                
                // Debug: print raw JSON
                String rawJson;
                serializeJson(doc, rawJson);
                safePrintln("Raw JSON received: " + rawJson);
                
                // Check if value field exists and what type it is
                if (doc.containsKey("value")) {
                    safePrintln("Value field exists");
                    if (doc["value"].is<int>()) {
                        safePrintln("Value is integer");
                    } else if (doc["value"].is<const char*>()) {
                        safePrintln("Value is string");
                    } else if (doc["value"].is<float>()) {
                        safePrintln("Value is float");
                    } else {
                        safePrintln("Value is unknown type");
                    }
                } else {
                    safePrintln("Value field does NOT exist");
                }
                
                // Try different parsing methods
                int receivedValue1 = doc["value"] | 0;  // Original method
                int receivedValue2 = doc["value"].as<int>();  // Direct cast
                String valueStr = doc["value"].as<String>();  // As string first
                int receivedValue3 = valueStr.toInt();  // String to int
                
                safePrintln("Method 1 (| 0): " + String(receivedValue1));
                safePrintln("Method 2 (.as<int>()): " + String(receivedValue2));
                safePrintln("Method 3 (string->int): " + String(receivedValue3));
                safePrintln("Value as string: '" + valueStr + "'");
                
                value = String(receivedValue1);
                safePrintln("Fan speed command: received value=" + String(receivedValue1) + ", parsed value=" + value);
                safePrintln("Original JSON: value field exists=" + String(doc.containsKey("value") ? "true" : "false"));
            } else if (cmd == "sleep") {
                unsigned long delaySec = doc["delay"] | 0;
                unsigned long durationSec = doc["duration"] | 0;
                value = String(delaySec) + " " + String(durationSec);
            }
            
            safePrintln("Calling processFanCommand with cmd=" + cmd + ", value=" + value);
            if (processFanCommand(cmd, value)) {
                response["success"] = true;
                if (cmd == "fan_speed") {
                    response["speed"] = value.toInt();
                    response["message"] = "Fan speed set to " + value + "%";
                } else if (cmd == "fan_on") {
                    response["message"] = "Fan turned ON at 50% speed";
                    response["speed"] = 50;
                } else if (cmd == "fan_off") {
                    response["message"] = "Fan turned OFF";
                    response["speed"] = 0;
                } else if (cmd == "gline_on") {
                    response["message"] = "GLine router ENABLED";
                } else if (cmd == "gline_off") {
                    response["message"] = "GLine router DISABLED";
                } else if (cmd == "sleep") {
                    unsigned long delaySec = doc["delay"] | 0;
                    unsigned long durationSec = doc["duration"] | 0;
                    response["message"] = "Sleep mode scheduled: " + String(delaySec) + "s delay, " + String(durationSec) + "s duration";
                    response["delay"] = delaySec;
                    response["duration"] = durationSec;
                } else if (cmd == "sleep_stop" || cmd == "wake") {
                    response["message"] = "Sleep mode stopped";
                }
            } else {
                response["success"] = false;
                response["error"] = "Failed to execute fan command: " + cmd;
            }
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "fan_status") {
        // Fan status command
        DynamicJsonDocument response(1024);
        response["cmd"] = "fan_status";
        response["success"] = true;
        response["enabled"] = config.enableFan && isFanEnabled();
        response["dutyCycle"] = config.enableFan ? getFanDutyCycle() : 0;
        response["rpm"] = config.enableFan ? getFanRPM() : 0;
        response["glineEnabled"] = config.enableFan && isGLineEnabled();
        response["pwmValue"] = config.enableFan ? map(getFanDutyCycle(), 0, 100, 0, 255) : 0;
        response["timestamp"] = time(nullptr); // Epoch timestamp
        
        // Add sleep mode information
        if (config.enableFan) {
            FanStatus fanStatus = getFanStatus();
            response["sleepMode"] = fanStatus.sleepMode;
            response["sleepStartTime"] = fanStatus.sleepStartTime;
            response["sleepDuration"] = fanStatus.sleepDuration;
            response["sleepEndTime"] = fanStatus.sleepEndTime;
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "setMCP3424Config") {
        // MCP3424 configuration setting
        DynamicJsonDocument response(1024);
        response["cmd"] = "setMCP3424Config";
        response["timestamp"] = time(nullptr); // Epoch timestamp
        
        if (doc.containsKey("devices")) {
            JsonArray devices = doc["devices"];
            
            // Reset all devices to disabled first
            for (int i = 0; i < 8; i++) {
                mcp3424Config.devices[i].enabled = false;
                mcp3424Config.devices[i].autoDetected = false;
                strcpy(mcp3424Config.devices[i].gasType, "");
                strcpy(mcp3424Config.devices[i].description, "");
            }
            
            mcp3424Config.deviceCount = 0;
            
            // Only process enabled devices from frontend
            for (JsonObject device : devices) {
                if (mcp3424Config.deviceCount < 8) {
                    int deviceIndex = device["deviceIndex"] | 0;
                    if (deviceIndex >= 0 && deviceIndex < 8) {
                        MCP3424DeviceAssignment& newDevice = mcp3424Config.devices[deviceIndex];
                        newDevice.deviceIndex = deviceIndex;
                        newDevice.i2cAddress = device["i2cAddress"] | (0x68 + deviceIndex);
                        strlcpy(newDevice.gasType, device["gasType"] | "", sizeof(newDevice.gasType));
                        strlcpy(newDevice.description, device["description"] | "", sizeof(newDevice.description));
                        newDevice.enabled = device["enabled"] | true;
                        newDevice.autoDetected = device["autoDetected"] | false;
                        mcp3424Config.deviceCount++;
                    }
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
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "resetMCP3424Config") {
        // MCP3424 configuration reset
        DynamicJsonDocument response(512);
        response["cmd"] = "resetMCP3424Config";
        response["timestamp"] = time(nullptr); // Epoch timestamp
        
        initializeDefaultMCP3424Mapping();
        
        // Update deviceCount to reflect enabled devices (should be 0 for new defaults)
        int enabledCount = 0;
        for (int i = 0; i < 8; i++) {
            if (mcp3424Config.devices[i].enabled) {
                enabledCount++;
            }
        }
        mcp3424Config.deviceCount = enabledCount;
        
        if (saveMCP3424Config(mcp3424Config)) {
            response["success"] = true;
            response["message"] = "MCP3424 configuration reset to defaults (all devices disabled)";
            safePrintln("MCP3424 config reset: " + String(enabledCount) + " devices enabled");
        } else {
            response["success"] = false;
            response["error"] = "Failed to save default MCP3424 configuration";
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
    } else if (cmd == "filesystemInfo") {
        // Handle filesystem info request
        DynamicJsonDocument response(1024);
        response["cmd"] = "filesystemInfo";
        response["success"] = true;
        response["timestamp"] = time(nullptr); // Epoch timestamp
        
        // Get filesystem information
        size_t totalBytes = LittleFS.totalBytes();
        size_t usedBytes = LittleFS.usedBytes();
        size_t freeBytes = totalBytes - usedBytes;
        float usedPercent = (totalBytes > 0) ? ((float)usedBytes / totalBytes) * 100.0f : 0.0f;
        
        response["totalBytes"] = totalBytes;
        response["usedBytes"] = usedBytes;
        response["freeBytes"] = freeBytes;
        response["usedPercent"] = round(usedPercent * 10) / 10.0f; // Round to 1 decimal place
        
        // Format filesystem info string
        String filesystemInfo = String(usedBytes) + "/" + String(totalBytes) + " bytes (" + 
                               String(usedPercent, 1) + "%)";
        response["filesystemInfo"] = filesystemInfo;
        
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
        
        safePrintln("Filesystem info requested: " + filesystemInfo);
        
    } else {
        DynamicJsonDocument errorResponse(1024);
        errorResponse["error"] = "Unknown command: " + cmd;
        String errorStr;
        serializeJson(errorResponse, errorStr);
        client->text(errorStr);
    }
    
    // Sprawdź pamięć po przetworzeniu wiadomości
    if (ESP.getFreeHeap() < CRITICAL_HEAP_SIZE) {
        safePrintln("WebSocket: Critical memory after message processing - scheduling reset");
        webSocketResetPending = true;
    }
}

// Funkcja do wysyłania komunikatu do WebSocket task
bool sendToWebSocketTask(AsyncWebSocketClient* client, const String& message) {
    if (!webSocketQueue || !client) {
        safePrintln("WebSocket Task: Queue or client is null");
        return false;
    }
    
    // Sprawdź czy kolejka nie jest pełna
    if (uxQueueSpacesAvailable(webSocketQueue) == 0) {
        safePrintln("WebSocket Task: Queue is full, dropping message");
        return false;
    }
    
    // Utwórz komunikat
    WebSocketMessage msg;
    msg.client = client;
    msg.message = message;
    msg.timestamp = millis();
    
    // Wyślij do kolejki
    if (xQueueSend(webSocketQueue, &msg, pdMS_TO_TICKS(100)) == pdTRUE) {
        return true;
    } else {
        safePrintln("WebSocket Task: Failed to send message to queue");
        return false;
    }
}

// Inicjalizacja WebSocket task system
bool initializeWebSocketTask() {
    // Utwórz kolejkę
    webSocketQueue = xQueueCreate(WEBSOCKET_QUEUE_SIZE, sizeof(WebSocketMessage));
    if (!webSocketQueue) {
        safePrintln("WebSocket Task: Failed to create queue");
        return false;
    }
    
    // Utwórz semaphore
    webSocketSemaphore = xSemaphoreCreateMutex();
    if (!webSocketSemaphore) {
        safePrintln("WebSocket Task: Failed to create semaphore");
        vQueueDelete(webSocketQueue);
        return false;
    }
    
    // Utwórz task
    BaseType_t result = xTaskCreatePinnedToCore(
        webSocketTask,           // Funkcja task
        "WebSocketTask",         // Nazwa task
        WEBSOCKET_STACK_SIZE,    // Rozmiar stack
        NULL,                    // Parametry
        WEBSOCKET_PRIORITY,      // Priorytet
        &webSocketTaskHandle,    // Handle task
        WEBSOCKET_CORE           // Core CPU
    );
    
    if (result != pdPASS) {
        safePrintln("WebSocket Task: Failed to create task");
        vQueueDelete(webSocketQueue);
        vSemaphoreDelete(webSocketSemaphore);
        return false;
    }
    
    safePrintln("WebSocket Task system initialized successfully");
    safePrintln("- Queue size: " + String(WEBSOCKET_QUEUE_SIZE));
    safePrintln("- Stack size: " + String(WEBSOCKET_STACK_SIZE) + " bytes");
    safePrintln("- Priority: " + String(WEBSOCKET_PRIORITY));
    safePrintln("- CPU Core: " + String(WEBSOCKET_CORE));
    
    return true;
}

// Funkcja zatrzymania WebSocket task system
void stopWebSocketTask() {
    if (webSocketTaskHandle) {
        vTaskDelete(webSocketTaskHandle);
        webSocketTaskHandle = NULL;
    }
    
    if (webSocketQueue) {
        vQueueDelete(webSocketQueue);
        webSocketQueue = NULL;
    }
    
    if (webSocketSemaphore) {
        vSemaphoreDelete(webSocketSemaphore);
        webSocketSemaphore = NULL;
    }
    
    safePrintln("WebSocket Task system stopped");
}

// Funkcja sprawdzania heap i resetowania WebSocket
void checkHeapAndReset() {
    uint32_t freeHeap = ESP.getFreeHeap();
    unsigned long currentTime = millis();
    
    // Sprawdź heap tylko co 30 sekund
    if (currentTime - lastHeapCheck < HEAP_CHECK_INTERVAL) {
        return;
    }
    lastHeapCheck = currentTime;
    
    // Krytycznie niski heap - natychmiastowy reset
    if (freeHeap < CRITICAL_HEAP_SIZE) {
        safePrintln("WebSocket: CRITICAL heap (" + String(freeHeap) + 
                   " bytes) - forcing immediate reset");
        webSocketResetPending = true;
        return;
    }
    
    // Niski heap - ostrzeżenie i przygotowanie do reset
    if (freeHeap < MIN_HEAP_SIZE) {
        safePrintln("WebSocket: Low heap (" + String(freeHeap) + 
                   " bytes) - preparing for reset");
        
        // Synchronizuj licznik klientów WebSocket
        if (wsClientCount != ws.count()) {
            wsClientCount = ws.count();
        }
        
        // Jeśli nadal niski heap, zaplanuj reset
        if (ESP.getFreeHeap() < MIN_HEAP_SIZE) {
            webSocketResetPending = true;
        }
    }
}

// Funkcja sprawdzania aktywności WebSocket
void checkWebSocketActivity() {
    unsigned long currentTime = millis();
    
    // Sprawdź czy są aktywni klienci
    if (ws.count() == 0) {
        lastWebSocketActivity = currentTime; // Resetuj timer jeśli brak klientów
        return;
    }
    
    // Sprawdź timeout nieaktywności
    if (lastWebSocketActivity > 0 && 
        (currentTime - lastWebSocketActivity) > WEBSOCKET_INACTIVITY_TIMEOUT) {
        
        safePrintln("WebSocket: No activity for " + 
                   String((currentTime - lastWebSocketActivity) / 1000) + 
                   " seconds - scheduling reset");
        webSocketResetPending = true;
    }
}

// Funkcja resetowania WebSocket
void resetWebSocket() {
    safePrintln("WebSocket: Starting reset process (count: " + String(webSocketResetCount + 1) + ")");
    
    uint32_t heapBefore = ESP.getFreeHeap();
    
    // Zamknij wszystkie połączenia
    ws.cleanupClients();
    
    // Wyczyść dane klientów
    wsClientCount = 0;
    memset(wsClients, 0, sizeof(wsClients));
    
    // Krótkie opóźnienie dla stabilizacji
    delay(100);
    
    // Wyczyść kolejkę WebSocket task
    if (webSocketQueue) {
        WebSocketMessage msg;
        while (xQueueReceive(webSocketQueue, &msg, 0) == pdTRUE) {
            // Usuń wszystkie oczekujące komunikaty
        }
    }
    
    uint32_t heapAfter = ESP.getFreeHeap();
    webSocketResetCount++;
    webSocketResetPending = false;
    lastWebSocketActivity = millis();
    
    safePrintln("WebSocket: Reset completed. Heap before: " + String(heapBefore) + 
               " bytes, after: " + String(heapAfter) + " bytes, recovered: " + 
               String((int32_t)heapAfter - (int32_t)heapBefore) + " bytes");
}

// Funkcja do ręcznego resetowania WebSocket (dla zewnętrznych wywołań)
void forceWebSocketReset(const String& reason) {
    safePrintln("WebSocket: Force reset requested - " + reason);
    webSocketResetPending = true;
}

// Funkcja sprawdzania statusu WebSocket
String getWebSocketStatus() {
    uint32_t freeHeap = ESP.getFreeHeap();
    unsigned long timeSinceActivity = millis() - lastWebSocketActivity;
    
    String status = "WebSocket Status:\n";
    status += "- Active clients: " + String(ws.count()) + "\n";
    status += "- Tracked clients: " + String(wsClientCount) + "\n";
    status += "- Free heap: " + String(freeHeap) + " bytes\n";
    status += "- Time since activity: " + String(timeSinceActivity / 1000) + " seconds\n";
    status += "- Reset count: " + String(webSocketResetCount) + "\n";
    status += "- Reset pending: " + String(webSocketResetPending ? "YES" : "NO") + "\n";
    
    if (webSocketQueue) {
        status += "- Queue messages: " + String(uxQueueMessagesWaiting(webSocketQueue)) + "/" + String(WEBSOCKET_QUEUE_SIZE) + "\n";
    }
    
    if (webSocketTaskHandle) {
        UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(webSocketTaskHandle);
        status += "- Task stack remaining: " + String(stackHighWaterMark) + " bytes\n";
    }
    
    return status;
}



