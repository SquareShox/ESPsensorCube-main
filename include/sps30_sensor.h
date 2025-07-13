#ifndef SPS30_SENSOR_H
#define SPS30_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <sps30.h>
#include "config.h"

// SPS30 I2C address (fixed)
#define SPS30_I2C_ADDRESS 0x69

// Function declarations
bool initializeSPS30();
bool readSPS30(SPS30Data& data);
void cleanSPS30();
void resetSPS30();
void startSPS30Measurement();
void stopSPS30Measurement();
bool isSPS30DataReady();
void enableSPS30Sensor();
void disableSPS30Sensor();

// Status variables - extern declarations (defined in sps30_sensor.cpp)
extern bool sps30SensorStatus;
extern SPS30Data sps30Data;

#endif // SPS30_SENSOR_H 