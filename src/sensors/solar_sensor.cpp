#include "solar_sensor.h"
#include <sensors.h>

// Forward declarations for safe printing functions  
void safePrint(const String& message);
void safePrintln(const String& message);

// Global sensor data
extern SolarData solarData;
extern HardwareSerial MySerial;
extern FeatureConfig config;

// Global status flags - extern declaration (defined in main sensors.cpp)
extern bool solarSensorStatus;

// Buffer for solar sensor data parsing
char solarBuffer[BUFFER_SIZE];
int solarBufferIndex = 0;

// Key-Value pairs for solar data mapping
struct KeyValuePair {
    String key;
    String value;
};

KeyValuePair dataMap[MAX_ENTRIES] = {
    {"PID", ""}, {"FW", ""}, {"SER#", ""}, {"V", ""}, {"I", ""},
    {"VPV", ""}, {"PPV", ""}, {"CS", ""}, {"MPPT", ""}, {"OR", ""},
    {"ERR", ""}, {"LOAD", ""}, {"IL", ""}, {"H19", ""}, {"H20", ""},
    {"H21", ""}, {"H22", ""}, {"H23", ""}, {"HSDS", ""}, {"Checksum", ""}
};

void initializeSolarSensor() {
    MySerial.begin(SOLAR_SERIAL_BAUD, SERIAL_8N1, SOLAR_RX_PIN, SOLAR_TX_PIN);
    while (!MySerial) {
        delay(100);
    }
    safePrintln("Solar sensor serial initialized");
}

void readSolarSensor() {
    if (!config.enableSolarSensor) return;
    
    while (MySerial.available() > 0) {
        char incomingByte = MySerial.read();
        
        if (solarBufferIndex < BUFFER_SIZE - 1) {
            solarBuffer[solarBufferIndex++] = incomingByte;
            solarBuffer[solarBufferIndex] = '\0';
        } else {
            // Shift buffer left when full
            for (int i = 1; i < BUFFER_SIZE; i++) {
                solarBuffer[i - 1] = solarBuffer[i];
            }
            solarBuffer[BUFFER_SIZE - 2] = incomingByte;
            solarBuffer[BUFFER_SIZE - 1] = '\0';
        }
        
        if (incomingByte == '\n') {
            if (parseSolarData(solarBuffer)) {
                solarData.valid = true;
                solarData.lastUpdate = millis();
                solarSensorStatus = true;
            }
            
            // Reset buffer
            solarBufferIndex = 0;
            solarBuffer[0] = '\0';
        }
    }
    
    // Check if data is still valid
    if (!isSensorDataValid(solarData.lastUpdate, SOLAR_TIMEOUT)) {
        solarData.valid = false;
        solarSensorStatus = false;
        resetSolarData();
    }
}

bool parseSolarData(const char* data) {
    const char* separatorPos = data;
    while (*separatorPos && !isspace(*separatorPos)) {
        separatorPos++;
    }
    
    if (*separatorPos == '\0') {
        return false;
    }
    
    String key = String(data).substring(0, separatorPos - data);
    String value = String(separatorPos + 1);
    value.trim();
    
    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (dataMap[i].key == key) {
            if (key == "Checksum") {
                // Convert checksum to hex format
                String hexValue = "";
                for (int j = 0; j < value.length(); j++) {
                    char hexBuffer[3];
                    sprintf(hexBuffer, "%02X", (unsigned char)value[j]);
                    hexValue += hexBuffer;
                }
                value = "0x" + hexValue;
            }
            dataMap[i].value = value;
            
            // Update structured data
            if (key == "PID") solarData.PID = value;
            else if (key == "FW") solarData.FW = value;
            else if (key == "SER#") solarData.SER = value;
            else if (key == "V") solarData.V = value;
            else if (key == "I") solarData.I = value;
            else if (key == "VPV") solarData.VPV = value;
            else if (key == "PPV") solarData.PPV = value;
            else if (key == "CS") solarData.CS = value;
            else if (key == "MPPT") solarData.MPPT = value;
            else if (key == "OR") solarData.OR = value;
            else if (key == "ERR") solarData.ERR = value;
            else if (key == "LOAD") solarData.LOAD = value;
            else if (key == "IL") solarData.IL = value;
            else if (key == "H19") solarData.H19 = value;
            else if (key == "H20") solarData.H20 = value;
            else if (key == "H21") solarData.H21 = value;
            else if (key == "H22") solarData.H22 = value;
            else if (key == "H23") solarData.H23 = value;
            else if (key == "HSDS") solarData.HSDS = value;
            else if (key == "Checksum") solarData.Checksum = value;
            
            return true;
        }
    }
    
    return false;
}

void resetSolarData() {
    // Reset data map
    for (int i = 0; i < MAX_ENTRIES; i++) {
        dataMap[i].value = "";
    }
    
    // Reset structured data
    solarData.PID = "";
    solarData.FW = "";
    solarData.SER = "";
    solarData.V = "";
    solarData.I = "";
    solarData.VPV = "";
    solarData.PPV = "";
    solarData.CS = "";
    solarData.MPPT = "";
    solarData.OR = "";
    solarData.ERR = "";
    solarData.LOAD = "";
    solarData.IL = "";
    solarData.H19 = "";
    solarData.H20 = "";
    solarData.H21 = "";
    solarData.H22 = "";
    solarData.H23 = "";
    solarData.HSDS = "";
    solarData.Checksum = "";
    solarData.valid = false;
}

void printSolarData() {
    for (int i = 0; i < MAX_ENTRIES; i++) {
        safePrint("SOLAR_" + dataMap[i].key + " ");
        safePrintln(dataMap[i].value);
    }
}

// Function isSensorDataValid moved to sensor_manager.cpp to avoid duplication