#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "DAC_WIFI"
#define WIFI_PASSWORD "$GrUnWaLdZkA$"
#define WIFI_TIMEOUT 20000
#define CONNECTION_TIMEOUT (20 * 60 * 1000) // 20 minutes

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
#define OPCN3_SS_PIN 10
#define IPS_POWER_PIN 1
#define IPS_RX_PIN 7         // Serial1 RX for IPS
#define IPS_TX_PIN 8         // Serial1 TX for IPS

// Serial Configuration
#define SERIAL_BAUD 115200
#define SOLAR_SERIAL_BAUD 19200
#define MODBUS_BAUD 38400
#define MODBUS_SLAVE_ID 30

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
#define REG_COUNT_CALIBRATION 100

// Timeouts
#define SENSOR_TIMEOUT (2 * 60 * 1000) // 2 minutes
#define MODBUS_TIMEOUT (5 * 60 * 1000) // 5 minutes
#define SOLAR_TIMEOUT (60 * 1000) // 1 minute
#define OPCN3_READ_INTERVAL 10000 // 10 seconds
#define OPCN3_SEND_INTERVAL 10000 // 10 seconds
#define I2C_TIMEOUT_MS 100        // I2C timeout 100ms

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
    bool enableIPS = false;         // Kontrola czujnika IPS (UART)
    bool enableIPSDebug = false;    // Kontrola IPS debug mode
    bool enableModbus = true;
    bool autoReset = false;
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
    SENSOR_INA219 = 9
};

// Serial Sensor Configuration
struct SerialSensorConfig {
    bool enabled = false;
    int rxPin = -1;
    int txPin = -1;
    long baudRate = 9600;
    String name = "";
    String protocol = ""; // "NMEA", "JSON", "CSV", "CUSTOM"
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
    float pm1_0;                // PM1.0 mass concentration [µg/m³]
    float pm2_5;                // PM2.5 mass concentration [µg/m³]
    float pm4_0;                // PM4.0 mass concentration [µg/m³]
    float pm10;                 // PM10 mass concentration [µg/m³]
    float nc0_5;                // Number concentration 0.5µm [#/cm³]
    float nc1_0;                // Number concentration 1.0µm [#/cm³]
    float nc2_5;                // Number concentration 2.5µm [#/cm³]
    float nc4_0;                // Number concentration 4.0µm [#/cm³]
    float nc10;                 // Number concentration 10µm [#/cm³]
    float typical_particle_size; // Typical particle size [µm]
    bool valid = false;
    unsigned long lastUpdate = 0;
};

#endif 