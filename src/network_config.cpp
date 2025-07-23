#include <LittleFS.h>
#include <config.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <network_config.h>

// Global network configuration
NetworkConfig networkConfig;

// Global MCP3424 configuration
MCP3424Config mcp3424Config;

// File paths
const char* NETWORK_CONFIG_FILE = "/network.json";
const char* WIFI_CONFIG_FILE = "/wifi.json";
const char* MCP3424_CONFIG_FILE = "/mcp3424.json";

// Forward declarations
void safePrint(const String& message);
void safePrintln(const String& message);

// Initialize LittleFS
bool initLittleFS() {
    if (!LittleFS.begin(true)) {
        safePrintln("LittleFS mount failed");
        return false;
    }
    safePrintln("LittleFS mounted successfully");
    return true;
}

// Save network configuration to LittleFS
bool saveNetworkConfig(const NetworkConfig& config) {
    File file = LittleFS.open(NETWORK_CONFIG_FILE, "w");
    if (!file) {
        safePrintln("Failed to open network config file for writing");
        return false;
    }
    
    DynamicJsonDocument doc(1024);
    doc["useDHCP"] = config.useDHCP;
    doc["staticIP"] = config.staticIP;
    doc["gateway"] = config.gateway;
    doc["subnet"] = config.subnet;
    doc["dns1"] = config.dns1;
    doc["dns2"] = config.dns2;
    doc["configValid"] = true;
    
    size_t bytesWritten = serializeJson(doc, file);
    file.close();
    
    if (bytesWritten == 0) {
        safePrintln("Failed to write network config");
        return false;
    }
    
    safePrintln("Network config saved successfully");
    return true;
}

// Load network configuration from LittleFS
bool loadNetworkConfig(NetworkConfig& config) {
    if (!LittleFS.exists(NETWORK_CONFIG_FILE)) {
        safePrintln("Network config file not found, using defaults");
        return false;
    }
    
    File file = LittleFS.open(NETWORK_CONFIG_FILE, "r");
    if (!file) {
        safePrintln("Failed to open network config file for reading");
        return false;
    }
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        safePrintln("Failed to parse network config JSON");
        return false;
    }
    
    config.useDHCP = doc["useDHCP"] | true;
    strlcpy(config.staticIP, doc["staticIP"] | "192.168.1.100", sizeof(config.staticIP));
    strlcpy(config.gateway, doc["gateway"] | "192.168.1.1", sizeof(config.gateway));
    strlcpy(config.subnet, doc["subnet"] | "255.255.255.0", sizeof(config.subnet));
    strlcpy(config.dns1, doc["dns1"] | "8.8.8.8", sizeof(config.dns1));
    strlcpy(config.dns2, doc["dns2"] | "8.8.4.4", sizeof(config.dns2));
    config.configValid = doc["configValid"] | false;
    
    safePrintln("Network config loaded successfully");
    return config.configValid;
}

// Save WiFi configuration to LittleFS
bool saveWiFiConfig(const char* ssid, const char* password) {
    File file = LittleFS.open(WIFI_CONFIG_FILE, "w");
    if (!file) {
        safePrintln("Failed to open WiFi config file for writing");
        return false;
    }
    
    DynamicJsonDocument doc(512);
    doc["ssid"] = ssid;
    doc["password"] = password;
    
    size_t bytesWritten = serializeJson(doc, file);
    file.close();
    
    if (bytesWritten == 0) {
        safePrintln("Failed to write WiFi config");
        return false;
    }
    
    safePrintln("WiFi config saved successfully");
    return true;
}

// Load WiFi configuration from LittleFS
bool loadWiFiConfig(char* ssid, char* password, size_t ssidSize, size_t passwordSize) {
    if (!LittleFS.exists(WIFI_CONFIG_FILE)) {
        safePrintln("WiFi config file not found");
        return false;
    }
    
    File file = LittleFS.open(WIFI_CONFIG_FILE, "r");
    if (!file) {
        safePrintln("Failed to open WiFi config file for reading");
        return false;
    }
    
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        safePrintln("Failed to parse WiFi config JSON");
        return false;
    }
    
    strlcpy(ssid, doc["ssid"] | "", ssidSize);
    strlcpy(password, doc["password"] | "", passwordSize);
    
    safePrintln("WiFi config loaded successfully");
    return true;
}

// Apply network configuration to WiFi
bool applyNetworkConfig() {
    if (networkConfig.useDHCP) {
        // Use DHCP
        if (!WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE)) {
            safePrintln("Failed to configure DHCP");
            return false;
        }
        safePrintln("DHCP configuration applied");
    } else {
        // Use static IP
        IPAddress staticIP, gateway, subnet, dns1, dns2;
        
        if (!staticIP.fromString(networkConfig.staticIP) ||
            !gateway.fromString(networkConfig.gateway) ||
            !subnet.fromString(networkConfig.subnet) ||
            !dns1.fromString(networkConfig.dns1) ||
            !dns2.fromString(networkConfig.dns2)) {
            safePrintln("Invalid IP address in config");
            return false;
        }
        
        if (!WiFi.config(staticIP, gateway, subnet, dns1, dns2)) {
            safePrintln("Failed to configure static IP");
            return false;
        }
        safePrintln("Static IP configuration applied");
    }
    
    return true;
}

// Get current network configuration as JSON
String getNetworkConfigJson() {
    DynamicJsonDocument doc(1024);
    
    doc["useDHCP"] = networkConfig.useDHCP;
    doc["staticIP"] = networkConfig.staticIP;
    doc["gateway"] = networkConfig.gateway;
    doc["subnet"] = networkConfig.subnet;
    doc["dns1"] = networkConfig.dns1;
    doc["dns2"] = networkConfig.dns2;
    doc["configValid"] = networkConfig.configValid;
    
    // Current WiFi status
    doc["currentIP"] = WiFi.localIP().toString();
    doc["currentSSID"] = WiFi.SSID();
    doc["wifiConnected"] = WiFi.status() == WL_CONNECTED;
    doc["wifiSignal"] = WiFi.RSSI();
    
    String json;
    serializeJson(doc, json);
    return json;
}

// Delete all configuration files
bool deleteAllConfig() {
    bool success = true;
    
    if (LittleFS.exists(NETWORK_CONFIG_FILE)) {
        success &= LittleFS.remove(NETWORK_CONFIG_FILE);
    }
    
    if (LittleFS.exists(WIFI_CONFIG_FILE)) {
        success &= LittleFS.remove(WIFI_CONFIG_FILE);
    }
    
    if (success) {
        safePrintln("All configuration files deleted");
    } else {
        safePrintln("Failed to delete some configuration files");
    }
    
    return success;
}

// List all files in LittleFS
String listLittleFSFiles() {
    String fileList = "Files in LittleFS:\n";
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    
    while (file) {
        fileList += "  " + String(file.name()) + " (" + String(file.size()) + " bytes)\n";
        file = root.openNextFile();
    }
    
    return fileList;
}

// Save MCP3424 configuration to LittleFS
bool saveMCP3424Config(const MCP3424Config& config) {
    File file = LittleFS.open(MCP3424_CONFIG_FILE, "w");
    if (!file) {
        safePrintln("Failed to open MCP3424 config file for writing");
        return false;
    }
    
    DynamicJsonDocument doc(2048); // 2KB for MCP3424 config
    doc["deviceCount"] = config.deviceCount;
    doc["configValid"] = true;
    
    JsonArray devices = doc.createNestedArray("devices");
    for (uint8_t i = 0; i < config.deviceCount; i++) {
        JsonObject device = devices.createNestedObject();
        device["deviceIndex"] = config.devices[i].deviceIndex;
        device["gasType"] = config.devices[i].gasType;
        device["description"] = config.devices[i].description;
        device["enabled"] = config.devices[i].enabled;
    }
    
    size_t bytesWritten = serializeJson(doc, file);
    file.close();
    
    if (bytesWritten == 0) {
        safePrintln("Failed to write MCP3424 config");
        return false;
    }
    
    safePrintln("MCP3424 config saved successfully");
    return true;
}

// Load MCP3424 configuration from LittleFS
bool loadMCP3424Config(MCP3424Config& config) {
    if (!LittleFS.exists(MCP3424_CONFIG_FILE)) {
        safePrintln("MCP3424 config file not found, using defaults");
        return false;
    }
    
    File file = LittleFS.open(MCP3424_CONFIG_FILE, "r");
    if (!file) {
        safePrintln("Failed to open MCP3424 config file for reading");
        return false;
    }
    
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        safePrintln("Failed to parse MCP3424 config JSON");
        return false;
    }
    
    config.deviceCount = doc["deviceCount"] | 0;
    config.configValid = doc["configValid"] | false;
    
    if (config.deviceCount > 8) {
        config.deviceCount = 8; // Safety limit
    }
    
    JsonArray devices = doc["devices"];
    for (uint8_t i = 0; i < config.deviceCount && i < devices.size(); i++) {
        JsonObject device = devices[i];
        config.devices[i].deviceIndex = device["deviceIndex"] | 0;
        strlcpy(config.devices[i].gasType, device["gasType"] | "", sizeof(config.devices[i].gasType));
        strlcpy(config.devices[i].description, device["description"] | "", sizeof(config.devices[i].description));
        config.devices[i].enabled = device["enabled"] | true;
    }
    
    safePrintln("MCP3424 config loaded successfully");
    return config.configValid;
}

// Get MCP3424 configuration as JSON
String getMCP3424ConfigJson() {
    DynamicJsonDocument doc(2048);
    
    doc["deviceCount"] = mcp3424Config.deviceCount;
    doc["configValid"] = mcp3424Config.configValid;
    
    JsonArray devices = doc.createNestedArray("devices");
    for (uint8_t i = 0; i < mcp3424Config.deviceCount; i++) {
        JsonObject device = devices.createNestedObject();
        device["deviceIndex"] = mcp3424Config.devices[i].deviceIndex;
        device["gasType"] = mcp3424Config.devices[i].gasType;
        device["description"] = mcp3424Config.devices[i].description;
        device["enabled"] = mcp3424Config.devices[i].enabled;
    }
    
    String json;
    serializeJson(doc, json);
    return json;
}

// Initialize default MCP3424 device assignment
void initializeDefaultMCP3424Mapping() {
    mcp3424Config.deviceCount = 0;
    mcp3424Config.configValid = false;
    
    // Assign devices to gas types
    addMCP3424Device(0, "NO", "NO Sensor (K1)", true);
    addMCP3424Device(1, "O3", "O3 Sensor (K2)", true);
    addMCP3424Device(2, "NO2", "NO2 Sensor (K3)", true);
    addMCP3424Device(3, "CO", "CO Sensor (K4)", true);
    addMCP3424Device(4, "SO2", "SO2 Sensor (K5)", true);
    addMCP3424Device(5, "TGS1", "TGS Sensor 1", true);
    addMCP3424Device(6, "TGS2", "TGS Sensor 2", true);
    addMCP3424Device(7, "TGS3", "TGS Sensor 3", true);
    
    mcp3424Config.configValid = true;
    safePrintln("Default MCP3424 device assignment initialized");
}

// Add a new MCP3424 device assignment
bool addMCP3424Device(uint8_t deviceIndex, const char* gasType, const char* description, bool enabled) {
    if (mcp3424Config.deviceCount >= 8) {
        safePrintln("MCP3424 device limit reached (8)");
        return false;
    }
    
    if (deviceIndex > 7) {
        safePrintln("Invalid device index");
        return false;
    }
    
    MCP3424DeviceAssignment& device = mcp3424Config.devices[mcp3424Config.deviceCount];
    device.deviceIndex = deviceIndex;
    strlcpy(device.gasType, gasType, sizeof(device.gasType));
    strlcpy(device.description, description, sizeof(device.description));
    device.enabled = enabled;
    
    mcp3424Config.deviceCount++;
    return true;
}

// Get MCP3424 device index by gas type
int8_t getMCP3424DeviceByGasType(const char* gasType) {
    for (uint8_t i = 0; i < mcp3424Config.deviceCount; i++) {
        if (strcmp(mcp3424Config.devices[i].gasType, gasType) == 0 && mcp3424Config.devices[i].enabled) {
            return mcp3424Config.devices[i].deviceIndex;
        }
    }
    return -1; // Device not found or disabled
}

// Get MCP3424 value by device and channel (existing function)
float getMCP3424Value(uint8_t deviceIndex, uint8_t channel) {
    extern MCP3424Data mcp3424Data;
    
    if (deviceIndex >= mcp3424Data.deviceCount || channel >= 4) {
        return 0.0f;
    }
    if (!mcp3424Data.valid[deviceIndex]) {
        return 0.0f;
    }
    return mcp3424Data.channels[deviceIndex][channel];
} 