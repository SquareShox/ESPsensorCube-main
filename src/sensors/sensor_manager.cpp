#include "sensor_manager.h"
#include "solar_sensor.h"
#include "opcn3_sensor.h"
#include "i2c_sensors.h"
#include "ips_sensor.h"
#include <sensors.h>

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

// Global configuration
extern FeatureConfig config;

void initializeSensors() {
    safePrintln("Initializing sensors...");
    
    if (config.enableSolarSensor) {
        safePrint("- Solar sensor: ");
        initializeSolarSensor();
        safePrintln("initialized");
    }
    
    if (config.enableOPCN3Sensor) {
        safePrint("- OPCN3 sensor: ");
        initializeOPCN3();
        safePrintln(opcn3SensorStatus ? "OK" : "FAILED");
    }
    
    if (config.enableI2CSensors) {
        safePrint("- I2C sensors: ");
        initializeI2C();
        safePrintln(i2cSensorStatus ? "OK" : "no sensors detected");
    }
    
    if (config.enableIPS) {
        safePrint("- IPS sensor: ");
        initializeIPS();
        safePrintln(ipsSensorStatus ? "OK" : "FAILED");
    }
    
    initializeSerialSensors();
    safePrintln("Sensor initialization complete");
}

void initializeSerialSensors() {
    // Initialize configured serial sensors
    extern SerialSensorConfig serialSensorConfigs[MAX_SERIAL_SENSORS];
    
    for (int i = 0; i < MAX_SERIAL_SENSORS; i++) {
        if (serialSensorConfigs[i].enabled) {
            // Initialize serial port for sensor
            safePrint("Initializing serial sensor ");
            safePrint(String(i));
            safePrint(": ");
            safePrintln(serialSensorConfigs[i].name);
        }
    }
}

void readSerialSensors() {
    // Implementation for additional serial sensors
    extern SerialSensorConfig serialSensorConfigs[MAX_SERIAL_SENSORS];
    
    for (int i = 0; i < MAX_SERIAL_SENSORS; i++) {
        if (serialSensorConfigs[i].enabled) {
            // Read from configured serial sensors
            // Implementation would depend on specific sensor protocols
        }
    }
}

void configureSerialSensor(int index, int rxPin, int txPin, long baud, String name, String protocol) {
    extern SerialSensorConfig serialSensorConfigs[MAX_SERIAL_SENSORS];
    
    if (index >= 0 && index < MAX_SERIAL_SENSORS) {
        serialSensorConfigs[index].enabled = true;
        serialSensorConfigs[index].rxPin = rxPin;
        serialSensorConfigs[index].txPin = txPin;
        serialSensorConfigs[index].baudRate = baud;
        serialSensorConfigs[index].name = name;
        serialSensorConfigs[index].protocol = protocol;
        
        safePrint("Configured serial sensor ");
        safePrint(String(index));
        safePrint(": ");
        safePrintln(name);
    }
}

bool isSensorDataValid(unsigned long lastUpdate, unsigned long timeout) {
    return (millis() - lastUpdate) < timeout;
}

void resetSensorData() {
    resetSolarData();
    resetIPSData();
    
    // Reset I2C sensor data
    extern I2CSensorData i2cSensorData;
    i2cSensorData.valid = false;
    i2cSensorData.temperature = 0.0;
    i2cSensorData.humidity = 0.0;
    i2cSensorData.pressure = 0.0;
    i2cSensorData.co2 = 0.0;
} 