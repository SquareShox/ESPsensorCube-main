#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <OPCN3.h>
#include "config.h"
#include "sps30_sensor.h"

// Data structures for sensor readings
struct SolarData {
    String PID = "";
    String FW = "";
    String SER = "";
    String V = "";
    String I = "";
    String VPV = "";
    String PPV = "";
    String CS = "";
    String MPPT = "";
    String OR = "";
    String ERR = "";
    String LOAD = "";
    String IL = "";
    String H19 = "";
    String H20 = "";
    String H21 = "";
    String H22 = "";
    String H23 = "";
    String HSDS = "";
    String Checksum = "";
    bool valid = false;
    unsigned long lastUpdate = 0;
};

// I2CSensorData structure moved to config.h to avoid circular dependency

struct SerialSensorData {
    String rawData = "";
    float values[10]; // Generic array for parsed values
    int valueCount = 0;
    bool valid = false;
    unsigned long lastUpdate = 0;
};

// IPSSensorData structure moved to config.h to avoid circular dependency

// Function declarations
void initializeSensors();
void initializeI2C();
bool initializeI2CSensor(I2CSensorType sensorType);
void enableI2CSensor(I2CSensorType sensorType);
void disableI2CSensor(I2CSensorType sensorType);

// Solar sensor functions
void initializeSolarSensor();
void readSolarSensor();
bool parseSolarData(const char* data);
void printSolarData();
void resetSolarData();

// OPCN3 sensor functions  
void initializeOPCN3();
void readOPCN3Sensor();
bool isOPCN3Valid();

// SHT40 sensor functions
bool initializeSHT40();
bool readSHT40();
bool isSHT40Valid();
void resetSHT40();
void printSHT40Data();

// I2C sensor functions moved to i2c_sensors.h to avoid circular dependency

// IPS sensor functions moved to ips_sensor.h to avoid circular dependency

// HCHO sensor functions
void initializeHCHO();
bool readHCHO();
bool isHCHODataValid();
void resetHCHOData();

// Serial sensors functions
void initializeSerialSensors();
void readSerialSensors();
void configureSerialSensor(int index, int rxPin, int txPin, long baud, const char* name, const char* protocol);

// Status functions
bool isSensorDataValid(unsigned long lastUpdate, unsigned long timeout);
void resetSensorData();

// Global sensor data
extern SolarData solarData;
extern I2CSensorData i2cSensorData;
extern SerialSensorData serialSensorData[MAX_SERIAL_SENSORS];
extern HistogramData opcn3Data;
extern OPCN3 myOPCN3;
extern IPSSensorData ipsSensorData;

// Global sensor status flags
extern bool solarSensorStatus;
extern bool opcn3SensorStatus;
extern bool i2cSensorStatus;
extern bool sht30SensorStatus;      // Osobny status SHT30
extern bool bme280SensorStatus;     // Osobny status BME280  
extern bool scd41SensorStatus;      // Osobny status SCD41
extern bool sht40SensorStatus;      // Osobny status SHT40
extern bool sps30SensorStatus;      // Status SPS30
extern bool mcp3424SensorStatus;    // Status MCP3424
extern bool ads1110SensorStatus;    // Status ADS1110
extern bool ina219SensorStatus;     // Status INA219
extern bool ipsSensorStatus;        // Status czujnika IPS
extern bool hchoSensorStatus;       // Status czujnika HCHO

// Global ADC sensor data
extern MCP3424Data mcp3424Data;
extern ADS1110Data ads1110Data;
extern INA219Data ina219Data;
extern SPS30Data sps30Data;
extern SHT40Data sht40Data;
extern HCHOData hchoData;           // HCHO sensor data

#endif 