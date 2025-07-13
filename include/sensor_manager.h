#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "config.h"

// Main sensor management functions
void initializeSensors();
void initializeSerialSensors();
void readSerialSensors();
void configureSerialSensor(int index, int rxPin, int txPin, long baud, String name, String protocol);
bool isSensorDataValid(unsigned long lastUpdate, unsigned long timeout);
void resetSensorData();

#endif 