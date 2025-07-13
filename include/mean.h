#ifndef MEAN_H
#define MEAN_H

#include <Arduino.h>
#include <config.h>
#include <sensors.h>
#include <calib.h>

// Initialize the moving average system
void initializeMovingAverages();

// Update moving averages with new sensor data
void updateMovingAverages();

// Print status of moving average buffers
void printMovingAverageStatus();

// Getter functions for fast averages (10 second period)
SolarData getSolarFastAverage();
I2CSensorData getI2CFastAverage();
SPS30Data getSPS30FastAverage();
IPSSensorData getIPSFastAverage();
MCP3424Data getMCP3424FastAverage();
ADS1110Data getADS1110FastAverage();
INA219Data getINA219FastAverage();
SHT40Data getSHT40FastAverage();

// Getter functions for slow averages (5 minute period)
SolarData getSolarSlowAverage();
I2CSensorData getI2CSlowAverage();
SPS30Data getSPS30SlowAverage();
IPSSensorData getIPSSlowAverage();
MCP3424Data getMCP3424SlowAverage();
ADS1110Data getADS1110SlowAverage();
INA219Data getINA219SlowAverage();
SHT40Data getSHT40SlowAverage();

// Getter functions for calibrated sensor data averages
CalibratedSensorData getCalibratedFastAverage();
CalibratedSensorData getCalibratedSlowAverage();

#endif // MEAN_H 