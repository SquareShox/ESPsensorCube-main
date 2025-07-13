#ifndef I2C_SENSORS_H
#define I2C_SENSORS_H

#include <Arduino.h>
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <SensirionI2cScd4x.h>

// I2C sensor functions
void initializeI2C();
bool initializeI2CSensor(I2CSensorType sensorType);
void enableI2CSensor(I2CSensorType sensorType);
void disableI2CSensor(I2CSensorType sensorType);

// I2C sensor reading functions
void readI2CSensors();
void readBatteryVoltage();
bool initializeSCD41();
bool readSHT30(I2CSensorData& data);
bool readBME280(I2CSensorData& data);
bool readSCD41(I2CSensorData& data);
bool readSHT40(SHT40Data& data);

// ADC sensor reading functions
bool readMCP3424(MCP3424Data& data);
bool startMCP3424Conversion(MCP3424Data& data);
bool readMCP3424Results(MCP3424Data& data);
void mcp3424ConversionTask(void* parameters);
bool readADS1110(ADS1110Data& data);
bool readINA219(INA219Data& data);

// ADC sensor configuration functions
void configureADS1110(uint8_t dataRate, uint8_t gain);
void setMCP3424Debug(bool enabled);
void resetSCD41();

// SCD41 task function
void scd41Task(void* parameters);

// Global status flags
extern bool i2cSensorStatus;
extern bool sht30SensorStatus;      // Osobny status SHT30
extern bool bme280SensorStatus;     // Osobny status BME280  
extern bool scd41SensorStatus;      // Osobny status SCD41
extern bool sht40SensorStatus;      // Osobny status SHT40
extern bool mcp3424SensorStatus;    // Status MCP3424
extern bool ads1110SensorStatus;    // Status ADS1110
extern bool ina219SensorStatus;     // Status INA219

// Global ADC data
extern MCP3424Data mcp3424Data;
extern ADS1110Data ads1110Data;
extern INA219Data ina219Data;
extern SHT40Data sht40Data;

// I2C semaphore and task management
extern SemaphoreHandle_t i2c_semaphore;
extern TaskHandle_t mcp3424_task_handle;
extern TaskHandle_t scd41_task_handle;
extern volatile bool mcp3424_conversion_in_progress;
extern volatile unsigned long mcp3424_conversion_start_time;

// Timeout constants
#define I2C_TIMEOUT_MS 100

#endif 