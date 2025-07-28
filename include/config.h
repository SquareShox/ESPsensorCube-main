#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// WiFi Configuration
#define WIFI_SSID "DAC_WIFI"
#define WIFI_PASSWORD "$GrUnWaLdZkA$"
#define WIFI_TIMEOUT 20000
#define CONNECTION_TIMEOUT (20 * 60 * 1000) // 20 minutes

#define FIRMWARE_VERSION "1.0.0"
#define DEVICE_ID "DAC-001"

// Pin Definitions
#define WS2812_PIN 21
// #define SOLAR_RX_PIN 7
// #define SOLAR_TX_PIN 8
#define SOLAR_RX_PIN 4
#define SOLAR_TX_PIN 2
#define MODBUS_RX_PIN 6
#define MODBUS_TX_PIN 5
#define I2C_SDA_PIN 7
#define I2C_SCL_PIN 8
#define OPCN3_SS_PIN 11
#define IPS_POWER_PIN 1
#define IPS_RX_PIN 2         // Serial1 RX for IPS
#define IPS_TX_PIN 4         // Serial1 TX for IPS
#define HCHO_RX_PIN 9       // HCHO sensor UART RX pin
#define HCHO_TX_PIN 10        // HCHO sensor UART TX pin
#define TACHO_PIN 11
#define PWM_PIN 13 
#define GLine_PIN 12
#define OFF_PIN 1

// Serial Configuration
#define SERIAL_BAUD 115200
#define SOLAR_SERIAL_BAUD 19200
#define MODBUS_BAUD 38400
#define MODBUS_SLAVE_ID 30
#define HCHO_SERIAL_BAUD 9600

// Buffer Sizes
#define BUFFER_SIZE 300
#define MAX_ENTRIES 20
#define REG_COUNT_SOLAR 50
#define REG_COUNT_OPCN3 50
#define REG_COUNT_I2C 50    // Dodano dedykowane rejestry I2C
#define REG_COUNT_IPS 50
#define REG_COUNT_SPS30 50
#define REG_COUNT_MCP3424 150  // Zwiększono do 150 dla wielu urządzeń
#define REG_COUNT_ADS1110 50
#define REG_COUNT_INA219 50
#define REG_COUNT_SHT40 50
#define REG_COUNT_HCHO 50      // Rejestry dla czujnika HCHO
#define REG_COUNT_CALIBRATION 100

// Timeouts
#define SENSOR_TIMEOUT (2 * 60 * 1000) // 2 minutes
#define MODBUS_TIMEOUT (5 * 60 * 1000) // 5 minutes
#define SOLAR_TIMEOUT (60 * 1000) // 1 minute
#define OPCN3_READ_INTERVAL 10000 // 10 seconds
#define OPCN3_SEND_INTERVAL 10000 // 10 seconds
#define I2C_TIMEOUT_MS 100        // I2C timeout 100ms
#define HCHO_TIMEOUT_MS 5000      // HCHO sensor timeout 2 seconds
#define HCHO_READ_INTERVAL 5000   // HCHO read interval 5 seconds

// Feature configuration structure for easy enable/disable of components
struct FeatureConfig {
    bool enableWiFi = true;
    bool enableWebServer = true;
    bool enableSolarSensor = false;   // Wyłączony bo IPS używa tych samych pinów
    bool enableOPCN3Sensor = false;
    bool enableI2CSensors = true;
    bool enableSHT30 = false;      // Osobna kontrola SHT30
    bool enableBME280 = false;     // Osobna kontrola BME280
    bool enableSCD41 = true;      // Osobna kontrola SCD41
    bool enableSHT40 = true;       // Osobna kontrola SHT40
    bool enableSPS30 = true;       // Sensirion SPS30 particle sensor
    bool enableMCP3424 = true;     // 18-bit ADC converter
    bool enableADS1110 = true;     // 16-bit ADC converter
    bool enableINA219 = true;      // Current/voltage sensor
    bool enableIPS = true;         // Kontrola czujnika IPS (UART)
    bool enableIPSDebug = true;    // Kontrola IPS debug mode
    bool enableHCHO = true;        // CB-HCHO-V4 formaldehyde sensor
    bool enableFan = true;         // Fan control system (PWM, Tacho, GLine)
    bool enableHistory = true;    // PSRAM-based sensor history system (temporarily disabled due to memory issues)
    bool enableModbus = true;
    bool autoReset = false;
    bool useAveragedData = true;  // Use fast averages instead of live data in dashboard
    bool lowPowerMode = false;    // Low power mode - disable LED and other power-consuming features
    bool enablePushbullet = true; // Enable Pushbullet notifications
    char pushbulletToken[64] = "o.vLzQdigI51uIXuohHUyxBfocSk5fYCPP"; // Pushbullet access token
};

// Network Configuration Structure
struct NetworkConfig {
    bool useDHCP = true;           // Use DHCP (true) or static IP (false)
    char staticIP[16] = "192.168.1.100";
    char gateway[16] = "192.168.1.1";
    char subnet[16] = "255.255.255.0";
    char dns1[16] = "8.8.8.8";
    char dns2[16] = "8.8.4.4";
    char wifiSSID[32] = "";
    char wifiPassword[64] = "";
    bool configValid = false;      // Flag to check if config was loaded
};

// MCP3424 Device Assignment Structure
struct MCP3424DeviceAssignment {
    uint8_t deviceIndex;           // Index of MCP3424 device (0-7)
    uint8_t i2cAddress;            // Physical I2C address (0x68-0x6F)
    char gasType[16];              // Gas type (e.g., "NO", "O3", "NO2", "CO", "SO2", "TGS1", "TGS2", "TGS3")
    char description[32];          // Human readable description
    bool enabled;                  // Whether this device is active
    bool autoDetected;             // Whether device was found during I2C scan
};

// MCP3424 Device Configuration
struct MCP3424Config {
    MCP3424DeviceAssignment devices[8];  // Up to 8 devices
    uint8_t deviceCount;           // Number of active devices
    bool configValid;              // Flag to check if config was loaded
};

// I2C Sensor Types
enum I2CSensorType {
    SENSOR_NONE = 0,
    SENSOR_SHT30 = 1,
    SENSOR_BME280 = 2,
    SENSOR_SCD41 = 3,
    SENSOR_SHT40 = 4,
    SENSOR_IPS = 5,
    SENSOR_SPS30 = 6,       // Sensirion SPS30 particle sensor
    SENSOR_MCP3424 = 7,
    SENSOR_ADS1110 = 8,
    SENSOR_INA219 = 9,
    SENSOR_HCHO = 10        // CB-HCHO-V4 formaldehyde sensor
};

// Serial Sensor Configuration
struct SerialSensorConfig {
    bool enabled = false;
    int rxPin = -1;
    int txPin = -1;
    long baudRate = 9600;
    const char* name = "";
    const char* protocol = ""; // "NMEA", "JSON", "CSV", "CUSTOM"
};

#define MAX_SERIAL_SENSORS 4

// ADC Sensor addresses and configuration
#define MCP3424_DEFAULT_ADDR 0x68    // 0x68-0x6F configurable
#define ADS1110_DEFAULT_ADDR 0x48    // 0x48-0x4B configurable  
#define INA219_DEFAULT_ADDR 0x40     // 0x40-0x4F configurable
#define SHT40_DEFAULT_ADDR 0x44      // 0x44 or 0x45 configurable

// ADC Sensor data structures
#define MAX_MCP3424_DEVICES 8        // Max 8 devices (0x68-0x6F)

struct MCP3424Data {
    uint8_t deviceCount;             // Number of active devices
    uint8_t addresses[MAX_MCP3424_DEVICES]; // I2C addresses of devices
    float channels[MAX_MCP3424_DEVICES][4];  // 4 channels per device
    uint8_t resolution;              // 12, 14, 16, 18 bit (same for all)
    uint8_t gain;                   // 1x, 2x, 4x, 8x (same for all)
    bool valid[MAX_MCP3424_DEVICES]; // Valid flag per device
    unsigned long lastUpdate;
};

struct ADS1110Data {
    float voltage;                  // Single channel voltage
    uint8_t dataRate;              // 15, 30, 60, 240 SPS
    uint8_t gain;                  // 1x, 2x, 4x, 8x
    bool valid;
    unsigned long lastUpdate;
};

struct INA219Data {
    float busVoltage;              // V
    float current;                 // mA
    float power;                   // mW
    float shuntVoltage;           // mV
    bool valid;
    unsigned long lastUpdate;
};

// I2C Environmental Sensor Data Structure (moved from sensors.h to avoid circular dependency)
struct I2CSensorData {
    float temperature = 0.0;
    float humidity = 0.0;
    float pressure = 0.0;
    float co2 = 0.0;
    bool valid = false;
    unsigned long lastUpdate = 0;
    I2CSensorType type = SENSOR_NONE;
};

// SHT40 Sensor Data Structure
struct SHT40Data {
    float temperature = 0.0;        // Temperature in Celsius
    float humidity = 0.0;           // Relative humidity in %
    float pressure = 0.0;           // Pressure in kPa (from PADS sensor)
    bool valid = false;
    unsigned long lastUpdate = 0;
};

// IPS Sensor Data Structure (moved from sensors.h to avoid circular dependency)
struct IPSSensorData {
    unsigned long pc_values[7];  // Particle count values  
    float pm_values[7];          // Particle mass values
    
    // Debug mode data
    bool debugMode = false;      // Czy włączony debug mode
    int won = 0;                 // Okres uśredniania (1=200ms, 2=500ms, 3=1000ms)
    
    // Number of Pulses (Np) data - 7 size bins
    unsigned long np_values[7];  // Np0.1, Np0.3, Np0.5, Np1.0, Np2.5, Np5.0, Np10
    
    // Pulse Width (Pw) data - 7 size bins  
    unsigned long pw_values[7];  // Pw0.1, Pw0.3, Pw0.5, Pw1.0, Pw2.5, Pw5.0, Pw10
    
    bool valid = false;
    unsigned long lastUpdate = 0;
};

// SPS30 Sensor Data Structure - Sensirion particle sensor
struct SPS30Data {
    float pm1_0 = 0.0;                // PM1.0 mass concentration [µg/m³]
    float pm2_5 = 0.0;                // PM2.5 mass concentration [µg/m³]
    float pm4_0 = 0.0;                // PM4.0 mass concentration [µg/m³]
    float pm10 = 0.0;                 // PM10 mass concentration [µg/m³]
    float nc0_5 = 0.0;                // Number concentration 0.5µm [#/cm³]
    float nc1_0 = 0.0;                // Number concentration 1.0µm [#/cm³]
    float nc2_5 = 0.0;                // Number concentration 2.5µm [#/cm³]
    float nc4_0 = 0.0;                // Number concentration 4.0µm [#/cm³]
    float nc10 = 0.0;                 // Number concentration 10µm [#/cm³]
    float typical_particle_size = 0.0; // Typical particle size [µm]
    bool valid = false;
    unsigned long lastUpdate = 0;
};

// HCHO Sensor Data Structure - CB-HCHO-V4 formaldehyde sensor
struct HCHOData {
    float hcho = 0.0;              // Formaldehyde concentration [mg/m³]
    float hcho_ppb = 0.0;          // Formaldehyde concentration [ppb]
    bool valid = false;
    unsigned long lastUpdate = 0;
};

// Fan Control Data Structure
struct FanData {
    uint8_t dutyCycle = 0;         // PWM duty cycle (0-100%)
    uint16_t rpm = 0;              // Fan RPM from tacho
    bool enabled = false;           // Fan enabled/disabled
    bool glineEnabled = false;      // GLine router enabled/disabled
    bool valid = false;
    unsigned long lastUpdate = 0;
};

// Battery monitoring structure
struct BatteryData {
    float voltage = 0.0;           // Battery voltage (V)
    float current = 0.0;           // Current (mA) - negative = charging, positive = discharging
    float power = 0.0;             // Power (mW)
    uint8_t chargePercent = 0;     // Battery charge percentage (0-100%)
    bool isBatteryPowered = false; // true = battery, false = external power
    bool lowBattery = false;       // true when battery < 7.2V
    bool criticalBattery = false;  // true when battery < 7.0V
    bool valid = false;
    unsigned long lastUpdate = 0;
    
    // Moving average for voltage (to avoid fluctuations)
    static const int VOLTAGE_AVERAGE_SIZE = 10;
    float voltageHistory[VOLTAGE_AVERAGE_SIZE];
    int voltageIndex = 0;
    bool voltageHistoryFull = false;
};

// Battery monitoring functions
void initializeBatteryMonitoring();
void updateBatteryStatus();
float calculateBatteryPercentage(float voltage);
bool isBatteryPowered(float current);
void checkBatteryShutdown();
void setOffPin(bool state);

// Pushbullet notification function declarations
void sendPushbulletNotification(const String& title, const String& message);
void sendBatteryCriticalNotification();
void sendSystemStartupNotification();
String getUptimeString();

// Time helper function declarations
String getFormattedTime();
String getFormattedDate();
time_t getEpochTime();
bool isTimeSet();

#endif 