#include <LittleFS.h>
#include <config.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <network_config.h>
#include <Wire.h>

// Global network configuration
NetworkConfig networkConfig;

// Global MCP3424 configuration
MCP3424Config mcp3424Config;

// File paths
const char* NETWORK_CONFIG_FILE = "/network.json";
const char* WIFI_CONFIG_FILE = "/wifi.json";
const char* MCP3424_CONFIG_FILE = "/mcp3424.json";
const char* SYSTEM_CONFIG_FILE = "/system.json";

// Forward declarations
void safePrint(const String& message);
void safePrintln(const String& message);

// Check and repair LittleFS if needed
bool checkAndRepairLittleFS() {
    safePrintln("Checking LittleFS status...");
    
    // Try to mount
    if (!LittleFS.begin()) {
        safePrintln("LittleFS not mounted, attempting to mount...");
        if (!LittleFS.begin(true)) {
            safePrintln("Failed to mount LittleFS - filesystem may be corrupted");
            return false;
        }
    }
    
    // Check space
    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    size_t freeBytes = totalBytes - usedBytes;
    
    safePrintln("LittleFS status:");
    safePrintln("- Total: " + String(totalBytes) + " bytes");
    safePrintln("- Used: " + String(usedBytes) + " bytes");
    safePrintln("- Free: " + String(freeBytes) + " bytes");
    
    if (freeBytes < 1024) {
        safePrintln("WARNING: Less than 1KB free space!");
        return false;
    }
    
    // Test write
    File testFile = LittleFS.open("/test_repair.txt", "w");
    if (!testFile) {
        safePrintln("ERROR: Cannot write to LittleFS");
        return false;
    }
    testFile.println("Test");
    testFile.close();
    
    // Test read
    testFile = LittleFS.open("/test_repair.txt", "r");
    if (!testFile) {
        safePrintln("ERROR: Cannot read from LittleFS");
        LittleFS.remove("/test_repair.txt");
        return false;
    }
    testFile.close();
    LittleFS.remove("/test_repair.txt");
    
    safePrintln("LittleFS check: PASSED");
    return true;
}

// Clean and repair LittleFS
bool cleanLittleFS() {
    safePrintln("Starting LittleFS cleanup...");
    
    // List all files before cleanup
    safePrintln("Files before cleanup:");
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    size_t totalFiles = 0;
    size_t totalSize = 0;
    while (file) {
        safePrintln("  " + String(file.name()) + " (" + String(file.size()) + " bytes)");
        totalSize += file.size();
        totalFiles++;
        file = root.openNextFile();
    }
    safePrintln("Total files: " + String(totalFiles) + ", Total size: " + String(totalSize) + " bytes");
    
    // Check for corrupted files and remove them
    bool cleanupNeeded = false;
    root = LittleFS.open("/");
    file = root.openNextFile();
    while (file) {
        String fileName = file.name();
        size_t fileSize = file.size();
        file.close();
        
        // Try to read the file to check if it's corrupted
        File testRead = LittleFS.open(fileName, "r");
        if (!testRead) {
            safePrintln("Corrupted file detected: " + fileName);
            if (LittleFS.remove(fileName)) {
                safePrintln("Removed corrupted file: " + fileName);
                cleanupNeeded = true;
            } else {
                safePrintln("Failed to remove corrupted file: " + fileName);
            }
        } else {
            testRead.close();
        }
        
        file = root.openNextFile();
    }
    
    if (cleanupNeeded) {
        safePrintln("LittleFS cleanup completed");
    } else {
        safePrintln("No cleanup needed");
    }
    
    return true;
}

// Initialize LittleFS
bool initLittleFS() {
    if (!LittleFS.begin(true)) {
        safePrintln("LittleFS mount failed");
        return false;
    }
    
    // Print LittleFS information
    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    size_t freeBytes = totalBytes - usedBytes;
    
    safePrintln("LittleFS mounted successfully");
    safePrintln("LittleFS info:");
    safePrintln("- Total space: " + String(totalBytes) + " bytes");
    safePrintln("- Used space: " + String(usedBytes) + " bytes");
    safePrintln("- Free space: " + String(freeBytes) + " bytes");
    
    // Test write capability
    File testFile = LittleFS.open("/test_write.txt", "w");
    if (testFile) {
        testFile.println("Test write successful");
        testFile.close();
        LittleFS.remove("/test_write.txt");
        safePrintln("LittleFS write test: PASSED");
    } else {
        safePrintln("LittleFS write test: FAILED");
        return false;
    }
    
    return true;
}

// Save network configuration to LittleFS
bool saveNetworkConfig(const NetworkConfig& config) {
    // Check if LittleFS is mounted
    if (!LittleFS.begin()) {
        safePrintln("LittleFS not mounted, attempting to mount...");
        if (!LittleFS.begin(true)) {
            safePrintln("Failed to mount LittleFS for network config save");
            return false;
        }
    }
    
    File file = LittleFS.open(NETWORK_CONFIG_FILE, "w");
    if (!file) {
        safePrintln("Failed to open network config file for writing");
        safePrintln("LittleFS status: " + String(LittleFS.usedBytes()) + "/" + String(LittleFS.totalBytes()) + " bytes");
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
    // Check if LittleFS is mounted
    if (!LittleFS.begin()) {
        safePrintln("LittleFS not mounted, attempting to mount...");
        if (!LittleFS.begin(true)) {
            safePrintln("Failed to mount LittleFS for WiFi config save");
            return false;
        }
    }
    
    // Check if we can create/write files
    File testFile = LittleFS.open("/test.txt", "w");
    if (!testFile) {
        safePrintln("Cannot create test file - LittleFS write permission issue");
        safePrintln("Free space: " + String(LittleFS.totalBytes() - LittleFS.usedBytes()) + " bytes");
        
        // Try to format and remount
        safePrintln("Attempting to format LittleFS...");
        LittleFS.end();
        if (LittleFS.begin(true)) {
            safePrintln("LittleFS formatted and remounted successfully");
        } else {
            safePrintln("Failed to format LittleFS");
            return false;
        }
    } else {
        testFile.close();
        LittleFS.remove("/test.txt");
    }
    
    // Debug: Print file path being used
    safePrintln("Attempting to create WiFi config file: " + String(WIFI_CONFIG_FILE));
    safePrintln("LittleFS mounted: " + String(LittleFS.totalBytes() > 0 ? "YES" : "NO"));
    
    // Try to create WiFi config file with retry
    File file = LittleFS.open(WIFI_CONFIG_FILE, "w");
    if (!file) {
        safePrintln("Failed to open WiFi config file for writing: " + String(WIFI_CONFIG_FILE));
        safePrintln("LittleFS status: " + String(LittleFS.usedBytes()) + "/" + String(LittleFS.totalBytes()) + " bytes");
        safePrintln("Free space: " + String(LittleFS.totalBytes() - LittleFS.usedBytes()) + " bytes");
        
        // List files to debug
        safePrintln("LittleFS files:");
        File root = LittleFS.open("/");
        File fileItem = root.openNextFile();
        while (fileItem) {
            safePrintln("  " + String(fileItem.name()) + " (" + String(fileItem.size()) + " bytes)");
            fileItem = root.openNextFile();
        }
        
        // Try alternative path
        safePrintln("Trying alternative file creation...");
        File altFile = LittleFS.open("/wificonfig.txt", "w");
        if (altFile) {
            altFile.println("test");
            altFile.close();
            LittleFS.remove("/wificonfig.txt");
            safePrintln("Alternative file creation successful");
        } else {
            safePrintln("Alternative file creation also failed");
        }
        
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
    
    if (LittleFS.exists(MCP3424_CONFIG_FILE)) {
        success &= LittleFS.remove(MCP3424_CONFIG_FILE);
    }
    
    if (LittleFS.exists(SYSTEM_CONFIG_FILE)) {
        success &= LittleFS.remove(SYSTEM_CONFIG_FILE);
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
    // ALWAYS save all 8 devices (not just enabled ones)
    for (uint8_t i = 0; i < 8; i++) {
        JsonObject device = devices.createNestedObject();
        device["deviceIndex"] = config.devices[i].deviceIndex;
        device["i2cAddress"] = config.devices[i].i2cAddress;
        device["gasType"] = config.devices[i].gasType;
        device["description"] = config.devices[i].description;
        device["enabled"] = config.devices[i].enabled;
        device["autoDetected"] = config.devices[i].autoDetected;
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
    
    // Initialize all devices to defaults first
    const uint8_t defaultAddresses[] = {0x68, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x69};
    for (uint8_t i = 0; i < 8; i++) {
        config.devices[i].deviceIndex = i;
        config.devices[i].i2cAddress = defaultAddresses[i];
        strcpy(config.devices[i].gasType, "");
        strcpy(config.devices[i].description, "");
        config.devices[i].enabled = false;
        config.devices[i].autoDetected = false;
    }
    
    // Load devices from JSON array
    JsonArray devices = doc["devices"];
    for (uint8_t i = 0; i < devices.size() && i < 8; i++) {
        JsonObject device = devices[i];
        uint8_t deviceIndex = device["deviceIndex"] | i;
        
        if (deviceIndex < 8) {
            config.devices[deviceIndex].deviceIndex = deviceIndex;
            config.devices[deviceIndex].i2cAddress = device["i2cAddress"] | defaultAddresses[deviceIndex];
            strlcpy(config.devices[deviceIndex].gasType, device["gasType"] | "", sizeof(config.devices[deviceIndex].gasType));
            strlcpy(config.devices[deviceIndex].description, device["description"] | "", sizeof(config.devices[deviceIndex].description));
            config.devices[deviceIndex].enabled = device["enabled"] | false;
            config.devices[deviceIndex].autoDetected = device["autoDetected"] | false;
        }
    }
    
    safePrintln("MCP3424 config loaded successfully");
    safePrint("  - deviceCount: ");
    safePrintln(String(config.deviceCount));
    safePrint("  - configValid: ");
    safePrintln(config.configValid ? "true" : "false");
    
    // Show loaded devices
    safePrintln("=== Loaded MCP3424 Devices ===");
    for (uint8_t i = 0; i < 8; i++) {
        safePrint("  Device ");
        safePrint(String(i));
        safePrint(": ");
        safePrint(config.devices[i].gasType);
        safePrint(" @ 0x");
        safePrint(String(config.devices[i].i2cAddress, HEX));
        safePrint(" (enabled=");
        safePrint(config.devices[i].enabled ? "true" : "false");
        safePrint(", detected=");
        safePrint(config.devices[i].autoDetected ? "true" : "false");
        safePrintln(")");
    }
    safePrintln("===============================");
    
    return config.configValid;
}

// Get MCP3424 configuration as JSON
String getMCP3424ConfigJson() {
    DynamicJsonDocument doc(2048);
    
    doc["deviceCount"] = mcp3424Config.deviceCount;
    doc["configValid"] = mcp3424Config.configValid;
    
    JsonArray devices = doc.createNestedArray("devices");
    // Return all 8 devices for frontend UI
    for (uint8_t i = 0; i < 8; i++) {
        JsonObject device = devices.createNestedObject();
        device["deviceIndex"] = mcp3424Config.devices[i].deviceIndex;
        device["i2cAddress"] = mcp3424Config.devices[i].i2cAddress;
        device["gasType"] = mcp3424Config.devices[i].gasType;
        device["description"] = mcp3424Config.devices[i].description;
        device["enabled"] = mcp3424Config.devices[i].enabled;
        device["autoDetected"] = mcp3424Config.devices[i].autoDetected;
    }
    
    String json;
    serializeJson(doc, json);
    return json;
}

// Initialize default MCP3424 device assignment
void initializeDefaultMCP3424Mapping() {
    mcp3424Config.deviceCount = 0;
    mcp3424Config.configValid = false;
    
    // Assign devices to gas types with specific I2C addresses
    // Format: deviceIndex, i2cAddress, gasType, description, enabled
    // NOTE: MCP3424 valid addresses are 0x68-0x6F only!
    addMCP3424DeviceWithAddress(0, 0x68, "NO", "NO Sensor (K1)", false);   // Default disabled
    addMCP3424DeviceWithAddress(1, 0x6A, "O3", "O3 Sensor (K2)", false);   // Default disabled
    addMCP3424DeviceWithAddress(2, 0x6B, "NO2", "NO2 Sensor (K3)", false); // Default disabled
    addMCP3424DeviceWithAddress(3, 0x6C, "CO", "CO Sensor (K4)", false);   // Default disabled
    addMCP3424DeviceWithAddress(4, 0x6D, "SO2", "SO2 Sensor (K5)", false); // Default disabled
    addMCP3424DeviceWithAddress(5, 0x6E, "TGS1", "TGS Sensor 1", false);   // Default disabled
    addMCP3424DeviceWithAddress(6, 0x6F, "TGS2", "TGS Sensor 2", false);   // Default disabled
    addMCP3424DeviceWithAddress(7, 0x69, "TGS3", "TGS Sensor 3", false);   // Fixed: 0x69 not 0x70!
    
    mcp3424Config.configValid = true;
    safePrintln("Default MCP3424 device assignment with I2C addresses initialized");
    
    // Debug: show what was initialized
    safePrintln("=== Initialized MCP3424 Devices ===");
    for (uint8_t i = 0; i < 8; i++) {
        safePrint("Device ");
        safePrint(String(i));
        safePrint(": ");
        safePrint(mcp3424Config.devices[i].gasType);
        safePrint(" @ 0x");
        safePrint(String(mcp3424Config.devices[i].i2cAddress, HEX));
        safePrint(" (enabled=");
        safePrint(mcp3424Config.devices[i].enabled ? "true" : "false");
        safePrintln(")");
    }
    safePrintln("==================================");
    
    safePrintln("=== MCP3424 Default Address Mapping (All DISABLED by default) ===");
    safePrintln("0x68 -> Device 0 -> NO (K1)");
    safePrintln("0x6A -> Device 1 -> O3 (K2)");
    safePrintln("0x6B -> Device 2 -> NO2 (K3)");
    safePrintln("0x6C -> Device 3 -> CO (K4)");
    safePrintln("0x6D -> Device 4 -> SO2 (K5)");
    safePrintln("0x6E -> Device 5 -> TGS1");
    safePrintln("0x6F -> Device 6 -> TGS2");
    safePrintln("0x69 -> Device 7 -> TGS3 (FIXED: was 0x70 which is invalid)");
    safePrintln("Use web interface to enable specific devices as needed.");
    safePrintln("=================================================================");
}

// Add a new MCP3424 device assignment (legacy function for compatibility)
bool addMCP3424Device(uint8_t deviceIndex, const char* gasType, const char* description, bool enabled) {
    // Use default I2C address mapping (0x68 + deviceIndex)
    uint8_t defaultAddress = 0x68 + deviceIndex;
    if (deviceIndex >= 8) defaultAddress = 0x68; // Fallback for invalid index
    
    return addMCP3424DeviceWithAddress(deviceIndex, defaultAddress, gasType, description, enabled);
}

// Add a new MCP3424 device assignment with specific I2C address
bool addMCP3424DeviceWithAddress(uint8_t deviceIndex, uint8_t i2cAddress, const char* gasType, const char* description, bool enabled) {
    if (mcp3424Config.deviceCount >= 8) {
        safePrintln("MCP3424 device limit reached (8)");
        return false;
    }
    
    if (deviceIndex > 7) {
        safePrintln("Invalid device index");
        return false;
    }
    
    // Validate I2C address range (0x68-0x6F typical for MCP3424)
    if (i2cAddress < 0x68 || i2cAddress > 0x6F) {
        safePrint("Warning: I2C address 0x");
        safePrint(String(i2cAddress, HEX));
        safePrintln(" is outside typical MCP3424 range (0x68-0x6F)");
    }
    
    MCP3424DeviceAssignment& device = mcp3424Config.devices[mcp3424Config.deviceCount];
    device.deviceIndex = deviceIndex;
    device.i2cAddress = i2cAddress;
    strlcpy(device.gasType, gasType, sizeof(device.gasType));
    strlcpy(device.description, description, sizeof(device.description));
    device.enabled = enabled;
    device.autoDetected = false; // Will be set during I2C scan
    
    mcp3424Config.deviceCount++;
    
    safePrint("Added MCP3424 mapping: Device ");
    safePrint(String(deviceIndex));
    safePrint(" (0x");
    safePrint(String(i2cAddress, HEX));
    safePrint(") -> ");
    safePrint(gasType);
    safePrint(" (");
    safePrint(description);
    safePrintln(")");
    
    return true;
}

// Get MCP3424 device index by gas type
int8_t getMCP3424DeviceByGasType(const char* gasType) {
    // safePrint("getMCP3424DeviceByGasType(\"");
    // safePrint(gasType);
    // safePrint("\") - searching through all 8 devices:");
    
    // Search through ALL 8 devices (not just deviceCount)
    for (uint8_t i = 0; i < 8; i++) {
        // safePrint("  Device ");
        // safePrint(String(i));
        // safePrint(": gasType=\"");
        // safePrint(mcp3424Config.devices[i].gasType);
        // safePrint("\", enabled=");
        // safePrint(mcp3424Config.devices[i].enabled ? "true" : "false");
        // safePrint(", autoDetected=");
        // safePrint(mcp3424Config.devices[i].autoDetected ? "true" : "false");
        // safePrint(", deviceIndex=");
        // safePrint(String(mcp3424Config.devices[i].deviceIndex));
        // safePrint(", I2C=0x");
        // safePrintln(String(mcp3424Config.devices[i].i2cAddress, HEX));
        
        if (strcmp(mcp3424Config.devices[i].gasType, gasType) == 0 && 
            mcp3424Config.devices[i].enabled && 
            mcp3424Config.devices[i].autoDetected) {
            // safePrint("  -> FOUND device ");
            // safePrint(String(mcp3424Config.devices[i].deviceIndex));
            // safePrint(" (I2C 0x");
            // safePrint(String(mcp3424Config.devices[i].i2cAddress, HEX));
            // safePrintln(")");
            return mcp3424Config.devices[i].deviceIndex;
        }
    }
   // safePrintln("  -> NOT FOUND");
    return -1; // Device not found, disabled, or not detected
}

// Get MCP3424 device index by I2C address
int8_t getMCP3424DeviceByI2CAddress(uint8_t i2cAddress) {
    // Search through ALL 8 devices (not just deviceCount)
    for (uint8_t i = 0; i < 8; i++) {
        if (mcp3424Config.devices[i].i2cAddress == i2cAddress && 
            mcp3424Config.devices[i].enabled &&
            mcp3424Config.devices[i].autoDetected) {
            return mcp3424Config.devices[i].deviceIndex;
        }
    }
    return -1; // Device not found, disabled, or not detected
}

// Get I2C address by device index
uint8_t getMCP3424I2CAddressByDeviceIndex(uint8_t deviceIndex) {
    // Search through ALL 8 devices (not just deviceCount)
    for (uint8_t i = 0; i < 8; i++) {
        if (mcp3424Config.devices[i].deviceIndex == deviceIndex && 
            mcp3424Config.devices[i].enabled) {
            return mcp3424Config.devices[i].i2cAddress;
        }
    }
    return 0x00; // Invalid address if not found
}

// Get gas type by I2C address
String getMCP3424GasTypeByI2CAddress(uint8_t i2cAddress) {
    // Search through ALL 8 devices (not just deviceCount)
    for (uint8_t i = 0; i < 8; i++) {
        if (mcp3424Config.devices[i].i2cAddress == i2cAddress && 
            mcp3424Config.devices[i].enabled &&
            mcp3424Config.devices[i].autoDetected) {
            return String(mcp3424Config.devices[i].gasType);
        }
    }
    return "UNKNOWN"; // Not found or not detected
}

// Update device detection status after I2C scan
bool updateMCP3424DeviceDetection(uint8_t i2cAddress, bool detected) {
    // Search through ALL 8 devices (not just deviceCount)
    for (uint8_t i = 0; i < 8; i++) {
        if (mcp3424Config.devices[i].i2cAddress == i2cAddress) {
            mcp3424Config.devices[i].autoDetected = detected;
            
            if (detected) {
                safePrint("MCP3424 detected at 0x");
                safePrint(String(i2cAddress, HEX));
                safePrint(" -> Device ");
                safePrint(String(mcp3424Config.devices[i].deviceIndex));
                safePrint(" (");
                safePrint(mcp3424Config.devices[i].gasType);
                safePrintln(")");
            }
            return true;
        }
    }
    
    // Device found but address not in any of 8 slots - this shouldn't happen if defaults are set correctly
    if (detected) {
        safePrint("WARNING: Unknown MCP3424 device found at 0x");
        safePrint(String(i2cAddress, HEX));
        safePrint(" - not in default mapping! Check default addresses.");
        safePrintln("");
    }
    
    return false;
}

// Scan I2C bus and map detected devices
void scanAndMapMCP3424Devices() {
    safePrintln("=== Scanning and mapping MCP3424 devices ===");
    
    // First, mark ALL devices (0-7) as not detected
    for (uint8_t i = 0; i < 8; i++) {
        mcp3424Config.devices[i].autoDetected = false;
    }
    
    // Scan I2C addresses  
    safePrintln("Scanning I2C addresses 0x68-0x6F...");
    for (uint8_t addr = 0x68; addr <= 0x6F; addr++) {
        // Note: 0x69 is valid MCP3424 address, don't skip it
        
        safePrint("Checking address 0x");
        safePrint(String(addr, HEX));
        safePrint("... ");
        
        Wire.beginTransmission(addr);
        uint8_t result = Wire.endTransmission();
        
        if (result == 0) {
            safePrintln("FOUND");
            // Device found - update detection status
            updateMCP3424DeviceDetection(addr, true);
        } else {
            safePrintln("NOT FOUND");
        }
    }
    
    // Show summary for ALL 8 devices
    safePrintln("=== MCP3424 Detection Summary ===");
    uint8_t detectedCount = 0;
    for (uint8_t i = 0; i < 8; i++) {
        safePrint("Device ");
        safePrint(String(i));
        safePrint(" (0x");
        safePrint(String(mcp3424Config.devices[i].i2cAddress, HEX));
        safePrint(") -> ");
        safePrint(mcp3424Config.devices[i].gasType);
        safePrint(": ");
        if (mcp3424Config.devices[i].autoDetected && mcp3424Config.devices[i].enabled) {
            safePrintln("DETECTED & ENABLED");
            detectedCount++;
        } else if (mcp3424Config.devices[i].autoDetected && !mcp3424Config.devices[i].enabled) {
            safePrintln("DETECTED but DISABLED");
        } else if (!mcp3424Config.devices[i].autoDetected && mcp3424Config.devices[i].enabled) {
            safePrintln("NOT DETECTED (enabled in config)");
        } else {
            safePrintln("NOT DETECTED & DISABLED");
        }
    }
    
    safePrint("Total active devices: ");
    safePrintln(String(detectedCount));
    safePrintln("================================");
}

// Display detailed MCP3424 mapping information
void displayMCP3424MappingInfo() {
    safePrintln("=== MCP3424 Mapping System Info ===");
    safePrint("Configuration valid: ");
    safePrintln(mcp3424Config.configValid ? "YES" : "NO");
    safePrint("Configured devices: ");
    safePrintln(String(mcp3424Config.deviceCount));
    safePrintln("");
    
    if (mcp3424Config.deviceCount == 0) {
        safePrintln("No devices configured!");
        safePrintln("Run initializeDefaultMCP3424Mapping() to setup defaults");
        return;
    }
    
    safePrintln("Device mapping details:");
    safePrintln("Dev | I2C Addr | Gas Type | Description        | Enabled | Detected");
    safePrintln("----|----------|----------|--------------------|---------|---------");
    
    for (uint8_t i = 0; i < mcp3424Config.deviceCount; i++) {
        char line[120];
        snprintf(line, sizeof(line), 
            " %2d | 0x%02X     | %-8s | %-18s | %-7s | %-8s",
            mcp3424Config.devices[i].deviceIndex,
            mcp3424Config.devices[i].i2cAddress,
            mcp3424Config.devices[i].gasType,
            mcp3424Config.devices[i].description,
            mcp3424Config.devices[i].enabled ? "YES" : "NO",
            mcp3424Config.devices[i].autoDetected ? "YES" : "NO"
        );
        safePrintln(line);
    }
    
    safePrintln("");
    safePrintln("Usage examples:");
    safePrintln("- getMCP3424DeviceByGasType(\"SO2\") -> returns device index for SO2 sensor");
    safePrintln("- getMCP3424DeviceByI2CAddress(0x6A) -> returns device index for address 0x6A");
    safePrintln("- getMCP3424I2CAddressByDeviceIndex(4) -> returns I2C address for device 4");
    safePrintln("- getMCP3424GasTypeByI2CAddress(0x6D) -> returns gas type for address 0x6D");
    safePrintln("");
    
    // Show active gas measurements
    safePrintln("Active gas measurements:");
    for (uint8_t i = 0; i < mcp3424Config.deviceCount; i++) {
        if (mcp3424Config.devices[i].enabled && mcp3424Config.devices[i].autoDetected) {
            safePrint("- ");
            safePrint(mcp3424Config.devices[i].gasType);
            safePrint(" at 0x");
            safePrint(String(mcp3424Config.devices[i].i2cAddress, HEX));
            safePrint(" (Device ");
            safePrint(String(mcp3424Config.devices[i].deviceIndex));
            safePrintln(")");
        }
    }
    safePrintln("===================================");
}

// Get MCP3424 value by device and channel (existing function)
float getMCP3424Value(uint8_t deviceIndex, uint8_t channel) {
    extern MCP3424Data mcp3424Data;
    extern MCP3424Config mcp3424Config;
    
    if (channel >= 4) {
        return 0.0f;
    }
    
    // Find the actual data array index for this device index
    for (uint8_t i = 0; i < mcp3424Data.deviceCount; i++) {
        if (mcp3424Data.addresses[i] == mcp3424Config.devices[deviceIndex].i2cAddress) {
            if (!mcp3424Data.valid[i]) {
                return 0.0f;
            }
            return mcp3424Data.channels[i][channel];
        }
    }
    
    // Device not found in data array - debug output
    safePrint("DEBUG: getMCP3424Value failed - device ");
    safePrint(String(deviceIndex));
    safePrint(" (I2C 0x");
    safePrint(String(mcp3424Config.devices[deviceIndex].i2cAddress, HEX));
    safePrint(") not found in data array (");
    safePrint(String(mcp3424Data.deviceCount));
    safePrintln(" devices)");
    return 0.0f;
}

// Save system configuration to LittleFS
bool saveSystemConfig(const FeatureConfig& config) {
    File file = LittleFS.open(SYSTEM_CONFIG_FILE, "w");
    if (!file) {
        safePrintln("Failed to open system config file for writing");
        return false;
    }
    
    DynamicJsonDocument doc(2048);
    doc["enableWiFi"] = config.enableWiFi;
    doc["enableWebServer"] = config.enableWebServer;
    doc["enableHistory"] = config.enableHistory;
    doc["useAveragedData"] = config.useAveragedData;
    doc["enableModbus"] = config.enableModbus;
    doc["enableSolarSensor"] = config.enableSolarSensor;
    doc["enableOPCN3Sensor"] = config.enableOPCN3Sensor;
    doc["enableI2CSensors"] = config.enableI2CSensors;
    doc["enableSHT30"] = config.enableSHT30;
    doc["enableBME280"] = config.enableBME280;
    doc["enableSCD41"] = config.enableSCD41;
    doc["enableSHT40"] = config.enableSHT40;
    doc["enableSPS30"] = config.enableSPS30;
    doc["enableMCP3424"] = config.enableMCP3424;
    doc["enableADS1110"] = config.enableADS1110;
    doc["enableINA219"] = config.enableINA219;
    doc["enableIPS"] = config.enableIPS;
    doc["enableIPSDebug"] = config.enableIPSDebug;
    doc["enableHCHO"] = config.enableHCHO;
    doc["enableFan"] = config.enableFan;
    doc["autoReset"] = config.autoReset;
    doc["lowPowerMode"] = config.lowPowerMode;
    doc["enablePushbullet"] = config.enablePushbullet;
    doc["pushbulletToken"] = config.pushbulletToken;
    doc["DeviceID"] = config.DeviceID;
    
    size_t bytesWritten = serializeJson(doc, file);
    file.close();
    
    if (bytesWritten == 0) {
        safePrintln("Failed to write system config");
        return false;
    }
    
    safePrintln("System config saved successfully");
    return true;
}

// Load system configuration from LittleFS
bool loadSystemConfig(FeatureConfig& config) {
    if (!LittleFS.exists(SYSTEM_CONFIG_FILE)) {
        safePrintln("System config file not found, using defaults");
        return false;
    }
    
    File file = LittleFS.open(SYSTEM_CONFIG_FILE, "r");
    if (!file) {
        safePrintln("Failed to open system config file for reading");
        return false;
    }
    
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        safePrintln("Failed to parse system config JSON");
        return false;
    }
    
    config.enableWiFi = doc["enableWiFi"] | true;
    config.enableWebServer = doc["enableWebServer"] | true;
    config.enableHistory = doc["enableHistory"] | true;
    config.useAveragedData = doc["useAveragedData"] | false;
    config.enableModbus = doc["enableModbus"] | false;
    config.enableSolarSensor = doc["enableSolarSensor"] | false;
    config.enableOPCN3Sensor = doc["enableOPCN3Sensor"] | false;
    config.enableI2CSensors = doc["enableI2CSensors"] | false;
    config.enableSHT30 = doc["enableSHT30"] | false;
    config.enableBME280 = doc["enableBME280"] | false;
    config.enableSCD41 = doc["enableSCD41"] | false;
    config.enableSHT40 = doc["enableSHT40"] | false;
    config.enableSPS30 = doc["enableSPS30"] | false;
    config.enableMCP3424 = doc["enableMCP3424"] | false;
    config.enableADS1110 = doc["enableADS1110"] | false;
    config.enableINA219 = doc["enableINA219"] | false;
    config.enableIPS = doc["enableIPS"] | false;
    config.enableIPSDebug = doc["enableIPSDebug"] | false;
    config.enableHCHO = doc["enableHCHO"] | false;
    config.enableFan = doc["enableFan"] | false;
    config.autoReset = doc["autoReset"] | false;
    config.lowPowerMode = doc["lowPowerMode"] | false;
    config.enablePushbullet = doc["enablePushbullet"] | true;
    strlcpy(config.pushbulletToken, doc["pushbulletToken"] | "", sizeof(config.pushbulletToken));
    strlcpy(config.DeviceID, doc["DeviceID"] | "SCUBE-001", sizeof(config.DeviceID));
    
    safePrintln("System config loaded successfully");
    return true;
} 