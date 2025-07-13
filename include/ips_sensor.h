#ifndef IPS_SENSOR_H
#define IPS_SENSOR_H

#include <Arduino.h>
#include "config.h"

// IPS sensor functions
void initializeIPS();
void readIPSSensor();
bool readIPS(IPSSensorData& data);
bool parseIPSData(String data);
bool parseIPSUARTData(String data);
bool parseIPSDebugData(String data);
void enableIPSDebugMode();
void disableIPSDebugMode();
bool testIPSCommunication();
void resetIPSData();

// Global status
extern bool ipsSensorStatus;

#endif 