#include <modbus_handler.h>
#include <sensors.h>
#include <ips_sensor.h>
#include <mean.h>
#include <calib.h>
#include <time.h>
#include <network_config.h>
// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);


// Global Modbus objects and data
ModbusSerial mb(Serial2, MODBUS_SLAVE_ID);
unsigned int modbusRegisters[REG_COUNT_SOLAR];
unsigned int modbusRegistersOPCN3[REG_COUNT_OPCN3];
unsigned int modbusRegistersIPS[REG_COUNT_IPS];

// Modbus activity tracking
unsigned long lastModbusActivity = 0;
bool hasHadModbusActivity = false;
bool offoveride = true;

// Network flag for display
extern bool turnOnNetwork;

// Moving average control (enum defined in header)
DataType currentDataType = DATA_FAST_AVG;

// External sensor data and status
extern HCHOData hchoData;
extern bool hchoSensorStatus;
extern BatteryData batteryData;

// External time functions
extern bool isTimeSet();
extern time_t getEpochTime();

// Function to safely switch data type
bool setCurrentDataType(DataType newType) {
    if (newType < DATA_CURRENT || newType > DATA_SLOW_AVG) {
        safePrintln("Invalid data type: " + String((int)newType));
        return false;
    }
    
    DataType oldType = currentDataType;
    currentDataType = newType;
    
    // Update Modbus register
   // mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, (uint16_t)currentDataType);
    
    String typeNames[] = {"Current", "Fast Average (10s)", "Slow Average (5min)"};
    safePrint("Data type changed from ");
    safePrint(typeNames[oldType]);
    safePrint(" to ");
    safePrintln(typeNames[newType]);
    
    return true;
}

// Get current data type name
String getCurrentDataTypeName() {
    switch (currentDataType) {
        case DATA_CURRENT: return "Current";
        case DATA_FAST_AVG: return "Fast Average (10s)";
        case DATA_SLOW_AVG: return "Slow Average (5min)";
        default: return "Unknown";
    }
}

// Cycle through data types
void cycleDataType() {
    DataType nextType = (DataType)((currentDataType + 1) % 3);
    setCurrentDataType(nextType);
}

// Get current data type
DataType getCurrentDataType() {
    return currentDataType;
}

// External configuration
extern FeatureConfig config;

void initializeModbus() {
    if (!config.enableModbus) return;
    
    // Initialize Serial2 for Modbus communication
    Serial2.begin(MODBUS_BAUD, SERIAL_8N1, MODBUS_RX_PIN, MODBUS_TX_PIN);
    Serial2.setTimeout(1000);
    
    // Initialize Modbus
    mb.config(MODBUS_BAUD);
    mb.setSlaveId(MODBUS_SLAVE_ID);
    
    // Calculate total registers needed
    int totalRegisters = REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219 + REG_COUNT_SPS30 + REG_COUNT_SHT40 + REG_COUNT_HCHO + REG_COUNT_CALIBRATION;
    safePrint("Initializing ");
    safePrint(String(totalRegisters));
    safePrintln(" Modbus registers");
    
    // Initialize all registers to 0
    for (int i = 0; i < totalRegisters; i++) {
        mb.addHreg(i, 0);
    }
    
    // Print register layout
    // safePrintln("Register Layout:");
    // safePrint("Solar:    0-"); safePrintln(String(REG_COUNT_SOLAR-1));
    // safePrint("OPCN3:    "); safePrint(String(REG_COUNT_SOLAR)); safePrint("-"); safePrintln(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3-1));
    // safePrint("I2C:      "); safePrint(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3)); safePrint("-"); safePrintln(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C-1));
    // safePrint("IPS:      "); safePrint(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C)); safePrint("-"); safePrintln(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS-1));
    // safePrint("MCP3424:  "); safePrint(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS)); safePrint("-"); safePrintln(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424-1));
    // safePrint("ADS1110:  "); safePrint(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424)); safePrint("-"); safePrintln(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110-1));
    // safePrint("INA219:   "); safePrint(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110)); safePrint("-"); safePrintln(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219-1));
    // safePrint("SPS30:    "); safePrint(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219)); safePrint("-"); safePrintln(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219 + REG_COUNT_SPS30-1));
    // safePrint("SHT40:    "); safePrint(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219 + REG_COUNT_SPS30)); safePrint("-"); safePrintln(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219 + REG_COUNT_SPS30 + REG_COUNT_SHT40-1));
    // safePrint("HCHO:     "); safePrint(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219 + REG_COUNT_SPS30 + REG_COUNT_SHT40)); safePrint("-"); safePrintln(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219 + REG_COUNT_SPS30 + REG_COUNT_SHT40 + REG_COUNT_HCHO-1));
    // safePrint("Calibration: "); safePrint(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219 + REG_COUNT_SPS30 + REG_COUNT_SHT40 + REG_COUNT_HCHO)); safePrint("-"); safePrintln(String(REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219 + REG_COUNT_SPS30 + REG_COUNT_SHT40 + REG_COUNT_HCHO + REG_COUNT_CALIBRATION-1));

    
    // Initialize activity tracking - give 15 minutes grace period at startup
    lastModbusActivity = millis();
    hasHadModbusActivity = false;
    
    // Initialize moving averages system
    //initializeMovingAverages();
    
    // Set control registers
    mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, (uint16_t)currentDataType);  // Data type selector
    mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+3, 1);  // Moving averages status (1 = active)
    
    safePrintln("Modbus initialized with IPS, HCHO support and moving averages");
}

void updateModbusSolarRegisters() {
    if (!config.enableModbus || !config.enableSolarSensor) return;
    
    // Get appropriate data based on current selection
    SolarData dataToUse;
    switch (currentDataType) {
        case DATA_CURRENT:
            dataToUse = solarData;
            break;
        case DATA_FAST_AVG:
            dataToUse = getSolarFastAverage();
            break;
        case DATA_SLOW_AVG:
            dataToUse = getSolarSlowAverage();
            break;
    }
    
    // Header registers - nowy format
    modbusRegisters[0] = solarSensorStatus ? 1 : 0; // Status flag
    modbusRegisters[1] = (uint16_t)currentDataType; // Typ danych (0=current, 1=fast avg, 2=slow avg)
    
    // Timestamp aktualizacji danych (32-bit)
    unsigned long updateTime = millis();
    modbusRegisters[2] = updateTime & 0xFFFF; // Lower 16 bits
    modbusRegisters[3] = (updateTime >> 16) & 0xFFFF; // Upper 16 bits
    
    // Solar data registers - zaczynamy od rejestru 4
    modbusRegisters[4] = hexToUint16(dataToUse.PID);
    modbusRegisters[5] = (uint16_t)dataToUse.FW.toInt();
    modbusRegisters[6] = (uint16_t)dataToUse.SER.toInt();
    modbusRegisters[7] = (uint16_t)dataToUse.V.toInt();
    modbusRegisters[8] = (int16_t)dataToUse.I.toInt();
    modbusRegisters[9] = (uint16_t)dataToUse.VPV.toInt();
    modbusRegisters[10] = (uint16_t)dataToUse.PPV.toInt();
    modbusRegisters[11] = (uint16_t)dataToUse.CS.toInt();
    modbusRegisters[12] = (uint16_t)dataToUse.MPPT.toInt();
    modbusRegisters[13] = hexToUint16(dataToUse.OR);
    modbusRegisters[14] = (uint16_t)dataToUse.ERR.toInt();
    modbusRegisters[15] = dataToUse.LOAD == "ON" ? 1 : 0;
    modbusRegisters[16] = (uint16_t)dataToUse.IL.toInt();
    modbusRegisters[17] = (uint16_t)dataToUse.H19.toInt();
    modbusRegisters[18] = (uint16_t)dataToUse.H20.toInt();
    modbusRegisters[19] = (uint16_t)dataToUse.H21.toInt();
    modbusRegisters[20] = (uint16_t)dataToUse.H22.toInt();
    modbusRegisters[21] = (uint16_t)dataToUse.H23.toInt();
    modbusRegisters[22] = (uint16_t)dataToUse.HSDS.toInt();
    
    // Update Modbus holding registers
    for (int i = 0; i < REG_COUNT_SOLAR; i++) {
        mb.setHreg(i, modbusRegisters[i]);
    }
}

void updateModbusOPCN3Registers() {
    if (!config.enableModbus || !config.enableOPCN3Sensor) return;
    
    // Header registers - nowy format
    modbusRegistersOPCN3[0] = opcn3SensorStatus ? 1 : 0; // Status flag
    modbusRegistersOPCN3[1] = (uint16_t)currentDataType; // Typ danych (0=current, 1=fast avg, 2=slow avg)
    
    // Timestamp aktualizacji danych (32-bit)
    unsigned long updateTime = millis();
    modbusRegistersOPCN3[2] = updateTime & 0xFFFF; // Lower 16 bits
    modbusRegistersOPCN3[3] = (updateTime >> 16) & 0xFFFF; // Upper 16 bits
    
    if (opcn3Data.valid) {
        modbusRegistersOPCN3[4] = (uint16_t)(opcn3Data.getTempC() * 100); // Temperature * 100
        modbusRegistersOPCN3[5] = (uint16_t)(opcn3Data.getHumidity() * 100); // Humidity * 100
        
        // Bin counts (24 bins) - MOST IMPORTANT DATA!
        // Each bin represents particle count in specific size range
        // Registers 6-29 (4+2+i where i=0-23)
        for (int i = 0; i < 24; i++) {
            modbusRegistersOPCN3[6 + i] = opcn3Data.binCounts[i];
        }
        
        // Debug: Print bin counts every 10 seconds
        // static unsigned long lastBinPrint = 0;
        // if (millis() - lastBinPrint > 10000) {
        //     lastBinPrint = millis();
        //     safePrint("OPCN3 Bin Counts: ");
        //     for (int i = 0; i < 24; i++) {
        //         safePrint(String(opcn3Data.binCounts[i]));
        //         if (i < 23) safePrint(",");
        //     }
        //     safePrintln("");
        // }
        
        // Time to cross values
        modbusRegistersOPCN3[30] = opcn3Data.bin1TimeToCross;
        modbusRegistersOPCN3[31] = opcn3Data.bin3TimeToCross;
        modbusRegistersOPCN3[32] = opcn3Data.bin5TimeToCross;
        modbusRegistersOPCN3[33] = opcn3Data.bin7TimeToCross;
        
        // Additional data
        modbusRegistersOPCN3[34] = opcn3SensorStatus ? 1 : 0;
        modbusRegistersOPCN3[35] = (uint16_t)(opcn3Data.sampleFlowRate * 100);
        modbusRegistersOPCN3[36] = opcn3Data.samplingPeriod;
        modbusRegistersOPCN3[37] = (uint16_t)(opcn3Data.pm1 * 100);
        modbusRegistersOPCN3[38] = (uint16_t)(opcn3Data.pm2_5 * 100);
        modbusRegistersOPCN3[39] = (uint16_t)(opcn3Data.pm10 * 100);
    }
    
    // Update Modbus holding registers
    for (int i = 0; i < REG_COUNT_OPCN3; i++) {
        mb.setHreg(REG_COUNT_SOLAR + i, modbusRegistersOPCN3[i]);
    }
}

void updateModbusI2CRegisters() {
    if (!config.enableModbus || !config.enableI2CSensors) return;
    
    // Get appropriate data based on current selection
    I2CSensorData dataToUse;
    switch (currentDataType) {
        case DATA_CURRENT:
            dataToUse = i2cSensorData;
            break;
        case DATA_FAST_AVG:
            dataToUse = getI2CFastAverage();
            break;
        case DATA_SLOW_AVG:
            dataToUse = getI2CSlowAverage();
            break;
    }
    
    // Use dedicated I2C register block
    int baseReg = REG_COUNT_SOLAR + REG_COUNT_OPCN3 + 3;
    
    // Header registers - nowy format
    mb.setHreg(baseReg, i2cSensorStatus ? 1 : 0); // Status
    mb.setHreg(baseReg + 1, (uint16_t)currentDataType); // Typ danych
    
    // Timestamp aktualizacji danych (32-bit)
    unsigned long updateTime = millis();
    mb.setHreg(baseReg + 2, updateTime & 0xFFFF); // Lower 16 bits
    mb.setHreg(baseReg + 3, (updateTime >> 16) & 0xFFFF); // Upper 16 bits

    if (dataToUse.valid) {
        mb.setHreg(baseReg + 4, (int16_t)(dataToUse.temperature * 100)); // Temperature * 100
        mb.setHreg(baseReg + 5, (uint16_t)(dataToUse.humidity * 100)); // Humidity * 100
        mb.setHreg(baseReg + 6, (uint16_t)(dataToUse.pressure * 10)); // Pressure * 10
        mb.setHreg(baseReg + 7, (uint16_t)(dataToUse.co2)); // CO2
        // safePrintln("CO2: " + String(dataToUse.co2));
        mb.setHreg(baseReg + 8, (uint16_t)dataToUse.type); // Sensor type
        
        // Add current time and date from ESP32 built-in time functions
        if (isTimeSet()) {
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                int hours = timeinfo.tm_hour;
                int minutes = timeinfo.tm_min;
                int day = timeinfo.tm_mday;
                int month = timeinfo.tm_mon + 1;
                int year = timeinfo.tm_year + 1900;
                
                mb.setHreg(baseReg + 9, (uint16_t)(hours * 100 + minutes)); // Time as HHMM
                mb.setHreg(baseReg + 10, (uint16_t)(day * 100 + month)); // Date as DDMM
                mb.setHreg(baseReg + 11, (uint16_t)year); // Year
                mb.setHreg(baseReg + 12, (uint16_t)getEpochTime()); // Epoch time (lower 16 bits)
                //print epoch time
             //   safePrint("Epoch time: ");
             //   safePrintln(String(getEpochTime()));
                //network on flag 
                mb.setHreg(baseReg + 13, turnOnNetwork ? 1 : 0);
            } else {
                // Fallback if getLocalTime fails
                mb.setHreg(baseReg + 9, 1200); // 12:00
                mb.setHreg(baseReg + 10, 1801);  // 18/01 (dzisiejszy dzien)
                mb.setHreg(baseReg + 11, 2024); // 2024
                mb.setHreg(baseReg + 12, 0); // No epoch time
                mb.setHreg(baseReg + 13, turnOnNetwork ? 1 : 0); // No network on flag
                //write netowrk data to modbus like netw
            }
        } else {
            // Default time and date if time not synchronized
            mb.setHreg(baseReg + 9, 1200); // 12:00
            mb.setHreg(baseReg + 10, 1801);  // 18/01 (dzisiejszy dzien)
            mb.setHreg(baseReg + 11, 2024); // 2024
            mb.setHreg(baseReg + 12, 0); // No epoch time
            mb.setHreg(baseReg + 13, turnOnNetwork ? 1 : 0); // No network on flag
        }
    } 
    else if (isTimeSet()) {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            int hours = timeinfo.tm_hour;
            int minutes = timeinfo.tm_min;
            int day = timeinfo.tm_mday;
            int month = timeinfo.tm_mon + 1;
            int year = timeinfo.tm_year + 1900;
            
            mb.setHreg(baseReg + 9, (uint16_t)(hours * 100 + minutes)); // Time as HHMM
            mb.setHreg(baseReg + 10, (uint16_t)(day * 100 + month)); // Date as DDMM
            mb.setHreg(baseReg + 11, (uint16_t)year); // Year
            mb.setHreg(baseReg + 12, (uint16_t)getEpochTime()); // Epoch time (lower 16 bits)
            //print epoch time
           // safePrint("Epoch time: ");
           // safePrintln(String(getEpochTime()));
            //network on flag 
            mb.setHreg(baseReg + 13, turnOnNetwork ? 1 : 0);
        } else {
            // Fallback if getLocalTime fails
            mb.setHreg(baseReg + 9, 1200); // 12:00
            mb.setHreg(baseReg + 10, 1801);  // 18/01 (dzisiejszy dzien)
            mb.setHreg(baseReg + 11, 2024); // 2024
            mb.setHreg(baseReg + 12, 0); // No epoch time
            mb.setHreg(baseReg + 13, turnOnNetwork ? 1 : 0); // No network on flag
            //write netowrk data to modbus like netw
        }
    } 
    else {
        // Clear data registers if invalid
        for (int i = 4; i < 14; i++) {
            mb.setHreg(baseReg + i, 0);
        }
        // Default time and date if time not synchronized
        mb.setHreg(baseReg + 9, 1200); // 12:00
        mb.setHreg(baseReg + 10, 1801);  // 18/01 (dzisiejszy dzien)
        mb.setHreg(baseReg + 11, 2024); // 2024
        mb.setHreg(baseReg + 12, 0); // No epoch time
        mb.setHreg(baseReg + 13, turnOnNetwork ? 1 : 0); // No network on flag
    }

    // --- WiFi credentials over Modbus (placed next to turnOnNetwork flag) ---
    // Layout within I2C block (REG_COUNT_I2C = 50):
    // baseReg+14: creds flags (bit0: present, bit1: passwordTruncated)
    // baseReg+15: ssidLen (0..32)
    // baseReg+16..31: SSID data (UTF-8 bytes, 2 chars per register => 32 chars max)
    // baseReg+32: passLen (0..63)
    // baseReg+33..49: PASSWORD data (2 chars per register => 34 chars max, may be truncated)
    {
        extern volatile bool wifiConfigChanged;
        static bool sentOnceAfterBoot = false;
        bool shouldSend = false;

        // Send once after boot, and then only when config changes
        if (!sentOnceAfterBoot) {
            shouldSend = true;
            sentOnceAfterBoot = true;
        } else if (wifiConfigChanged) {
            shouldSend = true;
        }

        if (!shouldSend) {
            // Keep flags/region zeroed if not sending this cycle
            // for (int r = 14; r < 50; r++) {
            //     mb.setHreg(baseReg + r, 0);
            // }
            return;
        }

        char ssid[32] = {0};
        char pwd[64] = {0};
        bool credsLoaded = loadWiFiConfig(ssid, pwd, sizeof(ssid), sizeof(pwd));

        uint16_t flags = 0;
        if (credsLoaded) {
            flags |= 0x0001; // present
        }

        // Clear region first
        for (int r = 14; r < 50; r++) {
            mb.setHreg(baseReg + r, 0);
        }

        if (credsLoaded) {
            // SSID
            size_t ssidLen = strnlen(ssid, sizeof(ssid));
            if (ssidLen > 32) ssidLen = 32;
            mb.setHreg(baseReg + 15, (uint16_t)ssidLen);
            for (size_t i = 0; i < ssidLen; i += 2) {
                uint16_t word = (uint8_t)ssid[i];
                if (i + 1 < ssidLen) word |= ((uint16_t)(uint8_t)ssid[i + 1]) << 8;
                mb.setHreg(baseReg + 16 + (i / 2), word);
            }

            // PASSWORD
            size_t passLen = strnlen(pwd, sizeof(pwd));
            const size_t passMaxChars = 34 * 2; // 34 regs -> 68 bytes
            size_t passToSend = passLen > passMaxChars ? passMaxChars : passLen;
            if (passLen > passMaxChars) {
                flags |= 0x0002; // passwordTruncated
            }
            mb.setHreg(baseReg + 32, (uint16_t)passLen); // store original length
            for (size_t i = 0; i < passToSend; i += 2) {
                uint16_t word = (uint8_t)pwd[i];
                if (i + 1 < passToSend) word |= ((uint16_t)(uint8_t)pwd[i + 1]) << 8;
                mb.setHreg(baseReg + 33 + (i / 2), word);
            }
        }

        mb.setHreg(baseReg + 14, flags);

        // Reset change flag after successful publish
        wifiConfigChanged = false;
    }
}

void updateModbusMCP3424Registers() {
    if (!config.enableModbus || !config.enableMCP3424) {
        // safePrint("MCP3424 Modbus update skipped - Modbus: ");
        // safePrint(config.enableModbus ? "ON" : "OFF");
        // safePrint(", MCP3424: ");
        // safePrintln(config.enableMCP3424 ? "ON" : "OFF");
        return;
    }
    if(offoveride) return;
    
    // Get appropriate data based on current selection
    MCP3424Data dataToUse;
    switch (currentDataType) {
        case DATA_CURRENT:
            dataToUse = mcp3424Data;
            break;
        case DATA_FAST_AVG:
            dataToUse = getMCP3424FastAverage();
            break;
        case DATA_SLOW_AVG:
            dataToUse = getMCP3424SlowAverage();
            break;
    }
    
    // Use registers after IPS data for MCP3424 ADC data
    int baseReg = REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS +3;
    
    // Header registers - nowy format
    mb.setHreg(baseReg, mcp3424SensorStatus ? 1 : 0); // Status
    mb.setHreg(baseReg + 1, (uint16_t)currentDataType); // Typ danych
    
    // Timestamp aktualizacji danych (32-bit)
    unsigned long updateTime = millis();
    mb.setHreg(baseReg + 2, updateTime & 0xFFFF); // Lower 16 bits
    mb.setHreg(baseReg + 3, (updateTime >> 16) & 0xFFFF); // Upper 16 bits
    
    // Total device count
    mb.setHreg(baseReg + 4, dataToUse.deviceCount);
    
    // Process each detected device - zaczynamy od rejestru 5
    for (uint8_t device = 0; device < dataToUse.deviceCount && device < MAX_MCP3424_DEVICES; device++) {
        int deviceBaseReg = baseReg + 5 + device * 16;  // Start from baseReg+5, each device gets 16 registers
        
        mb.setHreg(deviceBaseReg, dataToUse.valid[device] ? 1 : 0); // Status
        mb.setHreg(deviceBaseReg + 1, (uint16_t)dataToUse.addresses[device]); // I2C Address
        mb.setHreg(deviceBaseReg + 2, (uint16_t)dataToUse.resolution); // Resolution
        mb.setHreg(deviceBaseReg + 3, (uint16_t)dataToUse.gain); // Gain
        
        if (dataToUse.valid[device]) {
            // Store channels as signed 32-bit values (voltage * 1000000 for µV precision)
            for (int ch = 0; ch < 4; ch++) {
                int32_t voltage_uv = (int32_t)(dataToUse.channels[device][ch] * 1000000);
                mb.setHreg(deviceBaseReg + 4 + ch*2, voltage_uv & 0xFFFF);         // Lower 16 bits
                mb.setHreg(deviceBaseReg + 5 + ch*2, (voltage_uv >> 16) & 0xFFFF); // Upper 16 bits
            }
            mb.setHreg(deviceBaseReg + 12, (uint16_t)((millis() - dataToUse.lastUpdate) / 1000)); // Age in seconds
        } else {
            // Clear data registers if invalid
            for (int i = 4; i < 16; i++) {
                mb.setHreg(deviceBaseReg + i, 0);
            }
        }
    }
    
    // Clear unused device registers
    for (uint8_t device = dataToUse.deviceCount; device < MAX_MCP3424_DEVICES; device++) {
        int deviceBaseReg = baseReg + 5 + device * 16;
        for (int i = 0; i < 16; i++) {
            mb.setHreg(deviceBaseReg + i, 0);
        }
    }
}

void updateModbusADS1110Registers() {
    if (!config.enableModbus || !config.enableADS1110) return;
    
    // Get appropriate data based on current selection
    ADS1110Data dataToUse;
    switch (currentDataType) {
        case DATA_CURRENT:
            dataToUse = ads1110Data;
            break;
        case DATA_FAST_AVG:
            dataToUse = getADS1110FastAverage();
            break;
        case DATA_SLOW_AVG:
            dataToUse = getADS1110SlowAverage();
            break;
    }
    
    // Use registers after MCP3424 data for ADS1110 data
    int baseReg = REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 +3;
    
    // Header registers - nowy format
    mb.setHreg(baseReg, ads1110SensorStatus ? 1 : 0); // Status
    mb.setHreg(baseReg + 1, (uint16_t)currentDataType); // Typ danych
    
    // Timestamp aktualizacji danych (32-bit)
    unsigned long updateTime = millis();
    mb.setHreg(baseReg + 2, updateTime & 0xFFFF); // Lower 16 bits
    mb.setHreg(baseReg + 3, (updateTime >> 16) & 0xFFFF); // Upper 16 bits
    
    mb.setHreg(baseReg + 4, (uint16_t)dataToUse.dataRate); // Data rate
    mb.setHreg(baseReg + 5, (uint16_t)dataToUse.gain); // Gain
    
    if (dataToUse.valid) {
        // Store voltage as signed 32-bit value (voltage * 1000000 for uV precision)
        int32_t voltage_uv = (int32_t)(dataToUse.voltage * 1000000);
        mb.setHreg(baseReg + 6, voltage_uv & 0xFFFF);         // Lower 16 bits
        mb.setHreg(baseReg + 7, (voltage_uv >> 16) & 0xFFFF); // Upper 16 bits
        mb.setHreg(baseReg + 8, (uint16_t)((millis() - dataToUse.lastUpdate) / 1000)); // Age in seconds
    } else {
        // Clear data registers if invalid
        for (int i = 6; i < 9; i++) {
            mb.setHreg(baseReg + i, 0);
        }
    }
}

void updateModbusINA219Registers() {
    if (!config.enableModbus || !config.enableINA219) return;
    
    // Get appropriate data based on current selection
    INA219Data dataToUse;
    switch (currentDataType) {
        case DATA_CURRENT:
            dataToUse = ina219Data;
            break;
        case DATA_FAST_AVG:
            dataToUse = getINA219FastAverage();
            break;
        case DATA_SLOW_AVG:
            dataToUse = getINA219SlowAverage();
            break;
    }
   
    // Use registers after ADS1110 data for INA219 data
    int baseReg = REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 ;
    
    // Header registers - nowy format
    mb.setHreg(baseReg, ina219SensorStatus ? 1 : 0); // Status
    mb.setHreg(baseReg + 1, (uint16_t)currentDataType); // Typ danych
    
    // Timestamp aktualizacji danych (32-bit)
    unsigned long updateTime = millis();
    mb.setHreg(baseReg + 2, updateTime & 0xFFFF); // Lower 16 bits
    mb.setHreg(baseReg + 3, (updateTime >> 16) & 0xFFFF); // Upper 16 bits
    
    if (dataToUse.valid) {
        // Bus voltage (mV precision)
        mb.setHreg(baseReg + 4, (uint16_t)(dataToUse.busVoltage * 1000));
        // Shunt voltage (already in mV)
        mb.setHreg(baseReg + 5, (uint16_t)(dataToUse.shuntVoltage * 10)); // 0.1mV precision
        // Current (mA precision)
        mb.setHreg(baseReg + 6, (uint16_t)(dataToUse.current));
        // Power (mW precision)
        mb.setHreg(baseReg + 7, (uint16_t)(dataToUse.power));
        mb.setHreg(baseReg + 8, (uint16_t)((millis() - dataToUse.lastUpdate) / 1000)); // Age in seconds
        //use battery data
     
        mb.setHreg(baseReg + 9, batteryData.isBatteryPowered ? 1 : 0);
        mb.setHreg(baseReg + 10, batteryData.lowBattery ? 1 : 0);
        mb.setHreg(baseReg + 11, batteryData.criticalBattery ? 1 : 0);
    
   
    } else {
        // Clear data registers if invalid
        for (int i = 4; i < 12; i++) {
            mb.setHreg(baseReg + i, 0);
        }
    }
}

void updateModbusSPS30Registers() {
    if (!config.enableModbus || !config.enableSPS30) {
        // safePrint("SPS30 Modbus update skipped - Modbus: ");
        // safePrint(config.enableModbus ? "ON" : "OFF");
        // safePrint(", SPS30: ");
        // safePrintln(config.enableSPS30 ? "ON" : "OFF");
        return;
    }
    if(offoveride) return;
    // Get appropriate data based on current selection
    SPS30Data dataToUse;
    switch (currentDataType) {
        case DATA_CURRENT:
            dataToUse = sps30Data;
            break;
        case DATA_FAST_AVG:
            dataToUse = getSPS30FastAverage();
            break;
        case DATA_SLOW_AVG:
            dataToUse = getSPS30SlowAverage();
            break;
    }
    
    // Debug output every 30 seconds
    // static unsigned long lastSPS30ModbusDebug = 0;
    // if (millis() - lastSPS30ModbusDebug > 30000) {
    //     lastSPS30ModbusDebug = millis();
    //     safePrint("Updating SPS30 Modbus registers - Status: ");
    //     safePrint(sps30SensorStatus ? "OK" : "ERROR");
    //     safePrint(", Valid: ");
    //     safePrintln(dataToUse.valid ? "true" : "false");
    // }
    
    // Use registers after INA219 data for SPS30 data
    int baseReg = REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219 +3;
    
    // Header registers - nowy format
    mb.setHreg(baseReg, sps30SensorStatus ? 1 : 0); // Status
    mb.setHreg(baseReg + 1, (uint16_t)currentDataType); // Typ danych
    
    // Timestamp aktualizacji danych (32-bit)
    unsigned long updateTime = millis();
    mb.setHreg(baseReg + 2, updateTime & 0xFFFF); // Lower 16 bits
    mb.setHreg(baseReg + 3, (updateTime >> 16) & 0xFFFF); // Upper 16 bits
    
    if (dataToUse.valid) {
        // PM values scaled x10 with 16-bit saturation
        uint32_t pm1_scaled = (uint32_t)(dataToUse.pm1_0 * 10);
        if (pm1_scaled > 65535) pm1_scaled = 65535;
        uint32_t pm25_scaled = (uint32_t)(dataToUse.pm2_5 * 10);
        if (pm25_scaled > 65535) pm25_scaled = 65535;
        uint32_t pm4_scaled = (uint32_t)(dataToUse.pm4_0 * 10);
        if (pm4_scaled > 65535) pm4_scaled = 65535;
        uint32_t pm10_scaled = (uint32_t)(dataToUse.pm10 * 10);
        if (pm10_scaled > 65535) pm10_scaled = 65535;

        mb.setHreg(baseReg + 4, (uint16_t)pm1_scaled);  // PM1.0 * 10
        mb.setHreg(baseReg + 5, (uint16_t)pm25_scaled); // PM2.5 * 10
        mb.setHreg(baseReg + 6, (uint16_t)pm4_scaled);  // PM4.0 * 10
        mb.setHreg(baseReg + 7, (uint16_t)pm10_scaled); // PM10 * 10
        mb.setHreg(baseReg + 8, (uint16_t)(dataToUse.nc0_5 * 1000)); // NC0.5 * 1000
        mb.setHreg(baseReg + 9, (uint16_t)(dataToUse.nc1_0 * 1000)); // NC1.0 * 1000
        mb.setHreg(baseReg + 10, (uint16_t)(dataToUse.nc2_5 * 1000)); // NC2.5 * 1000
        mb.setHreg(baseReg + 11, (uint16_t)(dataToUse.nc4_0 * 1000)); // NC4.0 * 1000
        mb.setHreg(baseReg + 12, (uint16_t)(dataToUse.nc10 * 1000)); // NC10 * 1000
        mb.setHreg(baseReg + 13, (uint16_t)(dataToUse.typical_particle_size * 1000)); // Typical particle size * 1000
        mb.setHreg(baseReg + 14, (uint16_t)((millis() - dataToUse.lastUpdate) / 1000)); // Age in seconds
    } else {
        // Clear data registers if invalid
        for (int i = 4; i < 15; i++) {
            mb.setHreg(baseReg + i, 0);
        }
    }
}

void updateModbusIPSRegisters() {
    if (!config.enableModbus || !config.enableIPS) return;
    
    // Get appropriate data based on current selection
    IPSSensorData dataToUse;
    switch (currentDataType) {
        case DATA_CURRENT:
            dataToUse = ipsSensorData;
            break;
        case DATA_FAST_AVG:
            dataToUse = getIPSFastAverage();
            break;
        case DATA_SLOW_AVG:
            dataToUse = getIPSSlowAverage();
            break;
    }
    if(offoveride) return;
    // Use registers after I2C data for IPS sensor data
    int baseReg = REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C +3;
    
    // Header registers - nowy format
    modbusRegistersIPS[0] = ipsSensorStatus ? 1 : 0; // Status flag
    modbusRegistersIPS[1] = (uint16_t)currentDataType; // Typ danych
    
    // Timestamp aktualizacji danych (32-bit)
    unsigned long updateTime = millis();
    modbusRegistersIPS[2] = updateTime & 0xFFFF; // Lower 16 bits
    modbusRegistersIPS[3] = (updateTime >> 16) & 0xFFFF; // Upper 16 bits
    
    modbusRegistersIPS[4] = dataToUse.debugMode ? 1 : 0; // Debug mode flag
    modbusRegistersIPS[5] = dataToUse.won; // Won period (1=200ms, 2=500ms, 3=1000ms)
    
    if (dataToUse.valid && ipsSensorStatus) {
        // PC Values (7 values) - registers 6-19
        for (int i = 0; i < 7; i++) {
            uint32_t pcValue = dataToUse.pc_values[i];
            // Store as two 16-bit registers for large values
            modbusRegistersIPS[6 + i*2] = pcValue & 0xFFFF;         // Lower 16 bits
            modbusRegistersIPS[6 + i*2 + 1] = (pcValue >> 16) & 0xFFFF; // Upper 16 bits
        }
        
        // PM Values (7 values) - registers 20-26 (scaled by 1000 for precision)
        for (int i = 0; i < 7; i++) {
            modbusRegistersIPS[20 + i] = (uint16_t)(ipsSensorData.pm_values[i] * 10);
        }
        
        // Debug mode data (only if debug mode enabled)
        if (dataToUse.debugMode) {
            // NP Values (7 values) - registers 27-40
            for (int i = 0; i < 7; i++) {
                uint32_t npValue = dataToUse.np_values[i];
                // Store as two 16-bit registers for large values
                modbusRegistersIPS[27 + i*2] = npValue & 0xFFFF;         // Lower 16 bits
                modbusRegistersIPS[27 + i*2 + 1] = (npValue >> 16) & 0xFFFF; // Upper 16 bits
            }
            
            // PW Values (7 values) - registers 41-47
            for (int i = 0; i < 7; i++) {
                modbusRegistersIPS[41 + i] = (uint16_t)dataToUse.pw_values[i];
            }
        } else {
            // Clear debug registers when not in debug mode
            for (int i = 27; i < 48; i++) {
                modbusRegistersIPS[i] = 0;
            }
        }
        
        // Debug: Print IPS data every 30 seconds
        static unsigned long lastIPSPrint = 0;
        if (millis() - lastIPSPrint > 30000) {
            lastIPSPrint = millis();
            // safePrint("IPS Modbus Update - Status: ");
            // safePrint(ipsSensorStatus ? "OK" : "ERROR");
            // safePrint(", Valid: ");
            // safePrint(ipsSensorData.valid ? "YES" : "NO");
            // safePrint(", Debug: ");
            // safePrint(ipsSensorData.debugMode ? "ON" : "OFF");
            // safePrint(", Won: ");
            // safePrint(String(ipsSensorData.won));
            // safePrint(", PC0: ");
            // safePrint(String(ipsSensorData.pc_values[0]));
            // safePrint(", PM0: ");
            // safePrint(String(ipsSensorData.pm_values[0], 3));
            // if (ipsSensorData.debugMode) {
            //     safePrint(", NP0: ");
            //     safePrint(String(ipsSensorData.np_values[0]));
            //     safePrint(", PW0: ");
            //     safePrint(String(ipsSensorData.pw_values[0]));
            // }
            // safePrintln("");
        }
    } else {
        // Clear data registers if invalid
        for (int i = 6; i < REG_COUNT_IPS; i++) {
            modbusRegistersIPS[i] = 0;
        }
    }
    
    // Update Modbus holding registers - ensure all registers are updated
    for (int i = 0; i < REG_COUNT_IPS; i++) {
        mb.setHreg(baseReg + i, modbusRegistersIPS[i]);
    }
}

void updateModbusSHT40Registers() {
    if (!config.enableModbus || !config.enableSHT40) return;
    if(offoveride) return;
    // Get appropriate data based on current selection
    SHT40Data dataToUse;
    switch (currentDataType) {
        case DATA_CURRENT:
            dataToUse = sht40Data;
            break;
        case DATA_FAST_AVG:
            dataToUse = getSHT40FastAverage();
            break;
        case DATA_SLOW_AVG:
            dataToUse = getSHT40SlowAverage();
            break;
    }
    
    // Use registers after SPS30 data for SHT40 data
    int baseReg = REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219 + REG_COUNT_SPS30;
    
    // Header registers - nowy format
    mb.setHreg(baseReg, sht40SensorStatus ? 1 : 0); // Status
    mb.setHreg(baseReg + 1, (uint16_t)currentDataType); // Typ danych
    
    // Timestamp aktualizacji danych (32-bit)
    unsigned long updateTime = millis();
    mb.setHreg(baseReg + 2, updateTime & 0xFFFF); // Lower 16 bits
    mb.setHreg(baseReg + 3, (updateTime >> 16) & 0xFFFF); // Upper 16 bits
    
    if (dataToUse.valid) {
        // Temperature (x100 for 0.01°C precision)
        mb.setHreg(baseReg + 4, (int16_t)(dataToUse.temperature * 100));
        // Humidity (x100 for 0.01% precision)
        mb.setHreg(baseReg + 5, (uint16_t)(dataToUse.humidity * 100));
        // Pressure (x10 for 0.1 hPa precision)
        mb.setHreg(baseReg + 6, (uint16_t)(dataToUse.pressure * 10));
        // Age in seconds
        mb.setHreg(baseReg + 7, (uint16_t)((millis() - dataToUse.lastUpdate) / 1000));
    } else {
        // Clear data registers if invalid
        for (int i = 4; i < 8; i++) {
            mb.setHreg(baseReg + i, 0);
        }
    }
    
    // Debug output every 30 seconds
    static unsigned long lastSHT40ModbusDebug = 0;
    // if (millis() - lastSHT40ModbusDebug > 30000) {
    //     lastSHT40ModbusDebug = millis();
    //     safePrint("Updating SHT40 Modbus registers - Status: ");
    //     safePrint(sht40SensorStatus ? "OK" : "ERROR");
    //     safePrint(", Valid: ");
    //     safePrint(dataToUse.valid ? "true" : "false");
    //     if (dataToUse.valid) {
    //         safePrint(", Temp: ");
    //         safePrint(String(dataToUse.temperature, 2));
    //         safePrint("°C, Hum: ");
    //         safePrint(String(dataToUse.humidity, 2));
    //         safePrint("%, Press: ");
    //         safePrint(String(dataToUse.pressure, 2));
    //         safePrint(" kPa");
    //     }
    //     safePrintln("");
    // }
}

void updateModbusHCHORegisters() {
    if (!config.enableModbus || !config.enableHCHO) return;
    if(offoveride) return;
    // Get appropriate data based on current selection
    HCHOData dataToUse;
    switch (currentDataType) {
        case DATA_CURRENT:
            dataToUse = hchoData;
            break;
        case DATA_FAST_AVG:
            dataToUse = getHCHOFastAverage();
            break;
        case DATA_SLOW_AVG:
            dataToUse = getHCHOSlowAverage();
            break;
    }
    
    // Use registers after SHT40 data for HCHO data
    int baseReg = REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219 + REG_COUNT_SPS30 + REG_COUNT_SHT40;
    
    // Header registers - nowy format
    mb.setHreg(baseReg, hchoSensorStatus ? 1 : 0); // Status
    mb.setHreg(baseReg + 1, (uint16_t)currentDataType); // Typ danych
    
    // Timestamp aktualizacji danych (32-bit)
    unsigned long updateTime = millis();
    mb.setHreg(baseReg + 2, updateTime & 0xFFFF); // Lower 16 bits
    mb.setHreg(baseReg + 3, (updateTime >> 16) & 0xFFFF); // Upper 16 bits
    
    if (dataToUse.valid) {
        // HCHO concentration (x1000 for 0.001 mg/m³ precision) - primary measurement
        mb.setHreg(baseReg + 4, (uint16_t)(dataToUse.hcho * 1000));
        // Age in seconds
        mb.setHreg(baseReg + 5, (uint16_t)((millis() - dataToUse.lastUpdate) / 1000));
    } else {
        // Clear data registers if invalid
        for (int i = 4; i < 6; i++) {
            mb.setHreg(baseReg + i, 0);
        }
    }
    
    // Debug output every 30 seconds
    // static unsigned long lastHCHOModbusDebug = 0;
    // if (millis() - lastHCHOModbusDebug > 30000) {
    //     lastHCHOModbusDebug = millis();
    //     safePrint("Updating HCHO Modbus registers - Status: ");
    //     safePrint(hchoSensorStatus ? "OK" : "ERROR");
    //     safePrint(", Valid: ");
    //     safePrint(dataToUse.valid ? "true" : "false");
    //     if (dataToUse.valid) {
    //         safePrint(", HCHO: ");
    //         safePrint(String(dataToUse.hcho, 3));
    //         safePrint(" mg/m³");
    //     }
    //     safePrintln("");
    // }
}

void updateModbusCalibrationRegisters() {
    if (!config.enableModbus || !calibConfig.enableCalibration) return;
    
    // Get appropriate data based on current selection
    CalibratedSensorData dataToUse = calibratedData;
    switch (currentDataType) {
        case DATA_CURRENT:
            dataToUse = calibratedData;
            break;
        case DATA_FAST_AVG:
            dataToUse = getCalibratedFastAverage();
            break;
        case DATA_SLOW_AVG:
            dataToUse = getCalibratedSlowAverage();
            break;
    }

    //int baseReg = REG_COUNT_SOLAR + REG_COUNT_OPCN3 + REG_COUNT_I2C + REG_COUNT_IPS + REG_COUNT_MCP3424 + REG_COUNT_ADS1110 + REG_COUNT_INA219 + REG_COUNT_SPS30 + REG_COUNT_SHT40 +3;
    int baseReg = 550;
    mb.setHreg(baseReg, calibConfig.enableCalibration ? 1 : 0); // Status flag
    mb.setHreg(baseReg + 1, (uint16_t)currentDataType); // Typ danych

    // Timestamp aktualizacji danych (32-bit)
    unsigned long updateTime = millis();
    mb.setHreg(baseReg + 2, updateTime & 0xFFFF); // Lower 16 bits
    mb.setHreg(baseReg + 3, (updateTime >> 16) & 0xFFFF); // Upper 16 bits
    
    // Flagi statusowe czujników - rejestry 4-9
    // Sprawdź czy czujniki są włączone i dostępne
    extern bool sps30SensorStatus;
    extern bool scd41SensorStatus;
    extern bool hchoSensorStatus;
    extern bool mcp3424SensorStatus;
    
    // Sprawdź konkretne czujniki gazowe używając getMCP3424DeviceByGasType()
    bool hasNOSensor = false;
    bool hasNO2Sensor = false;
    bool hasCOSensor = false;
    bool hasTVOCSensor = false;
    
    if (mcp3424SensorStatus) {
        // Sprawdź czy konkretne czujniki gazowe są przypisane i dostępne
        int8_t noDevice = getMCP3424DeviceByGasType("NO");
     
        int8_t no2Device = getMCP3424DeviceByGasType("NO2");
        int8_t coDevice = getMCP3424DeviceByGasType("CO");
        int8_t tgs2Device = getMCP3424DeviceByGasType("TGS2");
        
        // Sprawdź czy urządzenia są dostępne i mają ważne dane
        extern MCP3424Data mcp3424Data;
        hasNOSensor = (noDevice >= 0 && isMCP3424DeviceValid(noDevice));
       
        hasNO2Sensor = (no2Device >= 0 && isMCP3424DeviceValid(no2Device));
        hasCOSensor = (coDevice >= 0 && isMCP3424DeviceValid(coDevice));
        hasTVOCSensor = (tgs2Device >= 0 && isMCP3424DeviceValid(tgs2Device));
    }
    
    mb.setHreg(baseReg + 4, sps30SensorStatus ? 1 : 0);        // PM sensors (SPS30)
    mb.setHreg(baseReg + 5, scd41SensorStatus ? 1 : 0);         // CO2 sensor (SCD41)
    mb.setHreg(baseReg + 6, hasNOSensor ? 1 : 0);               // NO sensor (MCP3424)
    mb.setHreg(baseReg + 7, hasNO2Sensor ? 1 : 0);              // NO2 sensor (MCP3424)
    mb.setHreg(baseReg + 8, hasCOSensor ? 1 : 0);               // CO sensor (MCP3424)
    mb.setHreg(baseReg + 9, hasTVOCSensor ? 1 : 0);             // TVOC sensor (TGS2)
    mb.setHreg(baseReg + 10, hchoSensorStatus ? 1 : 0);         // HCHO sensor

    if (dataToUse.valid) {
        mb.setHreg(baseReg + 11, (int16_t)(dataToUse.CO * 100));
        mb.setHreg(baseReg + 12, (int16_t)(dataToUse.NO * 100));
        mb.setHreg(baseReg + 13, (int16_t)(dataToUse.NO2 * 100));
        mb.setHreg(baseReg + 14, (int16_t)(dataToUse.O3 * 100));
        mb.setHreg(baseReg + 15, (int16_t)(dataToUse.SO2 * 100));
        mb.setHreg(baseReg + 16, (int16_t)(dataToUse.H2S * 100));
        mb.setHreg(baseReg + 17, (int16_t)(dataToUse.NH3 * 100));
        mb.setHreg(baseReg + 18, (int16_t)(dataToUse.HCHO * 100));
       // Serial.println(dataToUse.HCHO*100);
        mb.setHreg(baseReg + 19, (int16_t)(dataToUse.PID * 100));
        mb.setHreg(baseReg + 20, (int16_t)(dataToUse.TGS02 * 100));
        mb.setHreg(baseReg + 21, (int16_t)(dataToUse.TGS03 * 100));
        mb.setHreg(baseReg + 22, (int16_t)(dataToUse.TGS12 * 100));
        mb.setHreg(baseReg + 23, (int16_t)(dataToUse.TGS02_ohm * 100));
        mb.setHreg(baseReg + 24, (int16_t)(dataToUse.TGS03_ohm * 100));
        mb.setHreg(baseReg + 25, (int16_t)(dataToUse.TGS12_ohm * 100));
        // mb.setHreg(baseReg + 26, (int16_t)(dataToUse.TGS02_ppm * 100));
        // mb.setHreg(baseReg + 27, (int16_t)(dataToUse.TGS03_ppm * 100));
        // mb.setHreg(baseReg + 28, (int16_t)(dataToUse.TGS12_ppm * 100));
        // mb.setHreg(baseReg + 29, (int16_t)(dataToUse.TGS02_ppb * 100));
        // mb.setHreg(baseReg + 30, (int16_t)(dataToUse.TGS03_ppb * 100));
        // mb.setHreg(baseReg + 31, (int16_t)(dataToUse.TGS12_ppb * 100));
        mb.setHreg(baseReg + 32, (int16_t)(dataToUse.CO_ppb * 100));
        mb.setHreg(baseReg + 33, (int16_t)(dataToUse.NO_ppb * 100));
        mb.setHreg(baseReg + 34, (int16_t)(dataToUse.NO2_ppb * 100));
        mb.setHreg(baseReg + 35, (int16_t)(dataToUse.O3_ppb * 100));
        mb.setHreg(baseReg + 36, (int16_t)(dataToUse.SO2_ppb * 100));
        mb.setHreg(baseReg + 37, (int16_t)(dataToUse.H2S_ppb * 100));
        mb.setHreg(baseReg + 38, (int16_t)(dataToUse.NH3_ppb * 100));
        mb.setHreg(baseReg + 39, (int16_t)(dataToUse.HCHO * 100));
        mb.setHreg(baseReg + 40, (int16_t)(dataToUse.PID * 100));
        mb.setHreg(baseReg + 41, (int16_t)(dataToUse.VOC ));        // VOC ug/m3
        mb.setHreg(baseReg + 42, (int16_t)(dataToUse.VOC_ppb  ));    // TVOC ppb (TGS02)
        mb.setHreg(baseReg + 43, (int16_t)(dataToUse.ODO * 100));        // ODO value
        
        // PM sensors (SPS30) - rejestry 44-46
        mb.setHreg(baseReg + 44, (int16_t)(dataToUse.PM1 * 100));        // PM1.0 [ug/m3]
        mb.setHreg(baseReg + 45, (int16_t)(dataToUse.PM25 * 100));       // PM2.5 [ug/m3]
        mb.setHreg(baseReg + 46, (int16_t)(dataToUse.PM10 * 100));       // PM10 [ug/m3]
        
        // Environmental sensors - rejestry 47-55
        mb.setHreg(baseReg + 47, (int16_t)(dataToUse.AMBIENT_TEMP * 100));  // Temperatura zewnętrzna [°C]
        mb.setHreg(baseReg + 48, (int16_t)(dataToUse.AMBIENT_HUMID * 100)); // Wilgotność zewnętrzna [%]
        mb.setHreg(baseReg + 49, (int16_t)(dataToUse.AMBIENT_PRESS * 100)); // Ciśnienie zewnętrzne [hPa]
        
        mb.setHreg(baseReg + 50, (int16_t)(dataToUse.DUST_TEMP *100));     // Temperatura toru pylowego [°C]
        mb.setHreg(baseReg + 51, (int16_t)(dataToUse.DUST_HUMID *10));    // Wilgotność toru pylowego [%]
        mb.setHreg(baseReg + 52, (int16_t)(dataToUse.DUST_PRESS *10));    // Ciśnienie toru pylowego [hPa]
     
     
        
        mb.setHreg(baseReg + 53, (int16_t)(dataToUse.GAS_TEMP * 100));      // Temperatura toru gazowego [°C]
        mb.setHreg(baseReg + 54, (int16_t)(dataToUse.GAS_HUMID * 100));     // Wilgotność toru gazowego [%]
        mb.setHreg(baseReg + 55, (int16_t)(dataToUse.GAS_PRESS * 100));     // Ciśnienie toru gazowego [hPa]
        
        // CO2 sensor (SCD41) - rejestry 56-58
        mb.setHreg(baseReg + 56, (int16_t)(dataToUse.SCD_CO2 ));       // CO2 [ppm]
        mb.setHreg(baseReg + 57, (int16_t)(dataToUse.SCD_T * 100));         // SCD temperatura [°C]
        mb.setHreg(baseReg + 58, (int16_t)(dataToUse.SCD_RH * 100));        // SCD wilgotność [%]

    } else {
        // Clear data registers if invalid
        for (int i = 4; i < 59; i++) {
            mb.setHreg(baseReg + i, 0);
        }
    }
}

void processModbusTask() {
    if (!config.enableModbus) return;
    
    // Update moving averages with fresh sensor data
  
    
    // Check for Modbus activity BEFORE mb.task() processes and clears the buffer
    if (Serial2.available() > 0) {
        lastModbusActivity = millis();
        hasHadModbusActivity = true;
        
        // Debug: log first Modbus activity and periodic updates
        static bool firstActivity = true;
        static unsigned long lastActivityLog = 0;
        
        if (firstActivity) {
            firstActivity = false;
          //  safePrintln("First Modbus activity detected!");
        }
        
        // Log activity every 30 seconds
        if (millis() - lastActivityLog > 30000) {
            lastActivityLog = millis();
          //  safePrintln("Modbus activity detected - updating timestamp");
           // Serial2.println("Modbus activity detected - updating timestamp");
        }
    }
    
    mb.task();
    
    // Also update activity if any commands were processed
    static uint16_t lastChecksum = 0;
    uint16_t currentChecksum = 0;
    // Simple checksum of first few registers to detect any changes
    for (int i = 0; i < 5; i++) {
        currentChecksum += mb.hreg(i);
    }
    
    if (currentChecksum != lastChecksum) {
        lastModbusActivity = millis();
        hasHadModbusActivity = true;
        lastChecksum = currentChecksum;
    }
    
    // Check for data type selection changes (register 90)
    static uint16_t lastDataType = 0;
    uint16_t newDataType = mb.hreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+2);
    
    if (newDataType != lastDataType && newDataType <= 2) {
        lastDataType = newDataType;
        setCurrentDataType((DataType)newDataType);
    }
    
    // Check for special commands from Modbus master
    static uint16_t lastCommand = 0;
    uint16_t currentCommand = mb.hreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1); // Use register 100 for commands
    
    if (currentCommand != lastCommand && currentCommand != 0) {
        lastCommand = currentCommand;
        
        switch (currentCommand) {
            case 1: // Request fresh OPCN3 reading
                if (config.enableOPCN3Sensor) {
                    opcn3Data = myOPCN3.readHistogramData();
                    updateModbusOPCN3Registers();
                }
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, 0); // Clear command register
                break;
                
            case 2: // Reset system
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, 0); // Clear command register
                delay(100);
                ESP.restart();
                break;
                
            case 3: // Toggle auto reset
                config.autoReset = !config.autoReset;
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+2, config.autoReset ? 1 : 0); // Store state in register 92
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, 0); // Clear command register
                break;
                
            case 4: // Request fresh IPS reading
                if (config.enableIPS) {
                    readIPS(ipsSensorData);
                    updateModbusIPSRegisters();
                }
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, 0); // Clear command register
                break;
                
            case 5: // Enable IPS debug mode
                if (config.enableIPS) {
                    enableIPSDebugMode();
                    updateModbusIPSRegisters();
                }
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, 0); // Clear command register
                break;
                
            case 6: // Disable IPS debug mode
                if (config.enableIPS) {
                    disableIPSDebugMode();
                    updateModbusIPSRegisters();
                }
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, 0); // Clear command register
                break;
                
            case 7: // Switch to current data
                setCurrentDataType(DATA_CURRENT);
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, 0); // Clear command register
                break;
                
            case 8: // Switch to fast average (10s)
                setCurrentDataType(DATA_FAST_AVG);
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, 0); // Clear command register
                break;
                
            case 9: // Switch to slow average (5min)
                setCurrentDataType(DATA_SLOW_AVG);
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, 0); // Clear command register
                break;
                
            case 11: // Cycle through data types
                cycleDataType();
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, 0); // Clear command register
                break;
                
            case 10: // Print moving average status
                printMovingAverageStatus();
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, 0); // Clear command register
                break;
                
            default:
                mb.setHreg(REG_COUNT_SOLAR + REG_COUNT_OPCN3+1, 0); // Clear unknown command
                break;
        }
    }
}

uint16_t hexToUint16(String hex) {
    if (hex.length() == 0) return 0;
    return (uint16_t)strtol(hex.c_str(), NULL, 16);
}

String convertToHex(String value) {
    String hexString = "";
    for (int i = 0; i < value.length(); i++) {
        char hexBuffer[3];
        sprintf(hexBuffer, "%02X", (unsigned char)value[i]);
        hexString += hexBuffer;
    }
    return hexString;
} 