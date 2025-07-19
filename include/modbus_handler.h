#ifndef MODBUS_HANDLER_H
#define MODBUS_HANDLER_H

#include <Arduino.h>
#include <ModbusSerial.h>
#include "config.h"
#include "sensors.h"

// Data type enumeration
enum DataType {
    DATA_CURRENT = 0,   // Current readings
    DATA_FAST_AVG = 1,  // 10-second average
    DATA_SLOW_AVG = 2   // 5-minute average
};

// Function declarations
void initializeModbus();
void updateModbusSolarRegisters();
void updateModbusOPCN3Registers();
void updateModbusI2CRegisters();
void updateModbusMCP3424Registers();
void updateModbusADS1110Registers();
void updateModbusINA219Registers();
void updateModbusSPS30Registers();
void updateModbusSHT40Registers();
void updateModbusIPSRegisters();
void updateModbusHCHORegisters();
void updateModbusCalibrationRegisters();
void processModbusTask();

// Data type control functions
bool setCurrentDataType(DataType newType);
String getCurrentDataTypeName();
void cycleDataType();
DataType getCurrentDataType();

// Utility functions
uint16_t hexToUint16(String hex);
String convertToHex(String value);

// Global Modbus objects and data
extern ModbusSerial mb;
extern unsigned int modbusRegisters[REG_COUNT_SOLAR];
extern unsigned int modbusRegistersOPCN3[REG_COUNT_OPCN3];

// Modbus activity tracking
extern unsigned long lastModbusActivity;
extern bool hasHadModbusActivity;

// Current data type
extern DataType currentDataType;

#endif 