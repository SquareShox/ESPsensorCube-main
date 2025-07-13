#include <sensors.h>
#include <OPCN3.h>

// Global sensor data instances
SolarData solarData;
I2CSensorData i2cSensorData;
SerialSensorData serialSensorData[MAX_SERIAL_SENSORS];
HistogramData opcn3Data;
OPCN3 myOPCN3(OPCN3_SS_PIN);
IPSSensorData ipsSensorData;

// Global ADC sensor data
MCP3424Data mcp3424Data;
ADS1110Data ads1110Data;
INA219Data ina219Data;
SPS30Data sps30Data;
SHT40Data sht40Data;

// Global sensor status flags  
bool solarSensorStatus = false;
bool opcn3SensorStatus = false;
bool i2cSensorStatus = false;
bool sht30SensorStatus = false;
bool bme280SensorStatus = false;
bool scd41SensorStatus = false;
bool sht40SensorStatus = false;
bool sps30SensorStatus = false;
bool mcp3424SensorStatus = false;
bool ads1110SensorStatus = false;
bool ina219SensorStatus = false;
bool ipsSensorStatus = false; 