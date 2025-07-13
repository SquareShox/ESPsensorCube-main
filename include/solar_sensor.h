#ifndef SOLAR_SENSOR_H
#define SOLAR_SENSOR_H

#include <Arduino.h>
#include "config.h"

// Solar sensor functions
void initializeSolarSensor();
void readSolarSensor();
bool parseSolarData(const char* data);
void printSolarData();
void resetSolarData();

// Global status
extern bool solarSensorStatus;

#endif 