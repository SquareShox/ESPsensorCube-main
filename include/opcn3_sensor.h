#ifndef OPCN3_SENSOR_H
#define OPCN3_SENSOR_H

#include <Arduino.h>
#include <OPCN3.h>
#include "config.h"

// OPCN3 sensor functions  
void initializeOPCN3();
void readOPCN3Sensor();
bool isOPCN3Valid();

// Global status
extern bool opcn3SensorStatus;

#endif 