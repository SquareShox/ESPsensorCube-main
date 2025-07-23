#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <config.h>

// Function declarations
bool initLittleFS();
bool saveNetworkConfig(const NetworkConfig& config);
bool loadNetworkConfig(NetworkConfig& config);
bool saveWiFiConfig(const char* ssid, const char* password);
bool loadWiFiConfig(char* ssid, char* password, size_t ssidSize, size_t passwordSize);
bool applyNetworkConfig();
String getNetworkConfigJson();
bool deleteAllConfig();
String listLittleFSFiles();

// MCP3424 configuration functions
bool saveMCP3424Config(const MCP3424Config& config);
bool loadMCP3424Config(MCP3424Config& config);
String getMCP3424ConfigJson();
void initializeDefaultMCP3424Mapping();
bool addMCP3424Device(uint8_t deviceIndex, const char* gasType, const char* description, bool enabled);
int8_t getMCP3424DeviceByGasType(const char* gasType);
float getMCP3424Value(uint8_t deviceIndex, uint8_t channel);

// External configurations
extern NetworkConfig networkConfig;
extern MCP3424Config mcp3424Config;

#endif // NETWORK_CONFIG_H 