#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <config.h>

// Function declarations
bool cleanLittleFS();
bool checkAndRepairLittleFS();
bool initLittleFS();
bool saveNetworkConfig(const NetworkConfig& config);
bool loadNetworkConfig(NetworkConfig& config);
bool saveWiFiConfig(const char* ssid, const char* password);
bool loadWiFiConfig(char* ssid, char* password, size_t ssidSize, size_t passwordSize);
bool applyNetworkConfig();
String getNetworkConfigJson();
bool deleteAllConfig();
String listLittleFSFiles();

// Main system configuration functions
bool saveSystemConfig(const FeatureConfig& config);
bool loadSystemConfig(FeatureConfig& config);

// MCP3424 configuration functions
bool saveMCP3424Config(const MCP3424Config& config);
bool loadMCP3424Config(MCP3424Config& config);
String getMCP3424ConfigJson();
void initializeDefaultMCP3424Mapping();
bool addMCP3424Device(uint8_t deviceIndex, const char* gasType, const char* description, bool enabled);
bool addMCP3424DeviceWithAddress(uint8_t deviceIndex, uint8_t i2cAddress, const char* gasType, const char* description, bool enabled);
int8_t getMCP3424DeviceByGasType(const char* gasType);
int8_t getMCP3424DeviceByI2CAddress(uint8_t i2cAddress);
uint8_t getMCP3424I2CAddressByDeviceIndex(uint8_t deviceIndex);
String getMCP3424GasTypeByI2CAddress(uint8_t i2cAddress);
bool updateMCP3424DeviceDetection(uint8_t i2cAddress, bool detected);
void scanAndMapMCP3424Devices();
void displayMCP3424MappingInfo();
bool isMCP3424DeviceValid(uint8_t deviceIndex);
float getMCP3424Value(uint8_t deviceIndex, uint8_t channel);

// External configurations
extern NetworkConfig networkConfig;
extern MCP3424Config mcp3424Config;

// Flag: set to true when WiFi config has changed (for conditional Modbus export)
extern volatile bool wifiConfigChanged;

#endif // NETWORK_CONFIG_H 