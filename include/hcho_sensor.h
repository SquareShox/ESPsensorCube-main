#ifndef HCHO_SENSOR_H
#define HCHO_SENSOR_H

#include <config.h>
#include <CB_HCHO_V4.h>

// Function declarations for HCHO sensor
void initializeHCHO();
bool readHCHO();
bool isHCHODataValid();
void resetHCHOData();

// Configuration functions
bool setHCHOAutoCalibration(uint8_t mode);
float getHCHOConcentration();

// Diagnostic function
void printHCHODiagnostics();

// External data access
extern HCHOData hchoData;
extern bool hchoSensorStatus;

#endif // HCHO_SENSOR_H 