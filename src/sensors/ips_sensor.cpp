#include "ips_sensor.h"
#include <sensors.h>

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

// Global sensor data
extern IPSSensorData ipsSensorData;
extern FeatureConfig config;

// Global status flags - extern declaration (defined in main sensors.cpp)
extern bool ipsSensorStatus;

void initializeIPS() {
    safePrintln("Initializing IPS sensor via Serial1 UART...");
    
    // Initialize Serial1 for IPS sensor (hardware serial)
    Serial1.end();  // End previous serial if running
    delay(100);
    
    Serial1.begin(115200, SERIAL_8N1, IPS_RX_PIN, IPS_TX_PIN);
    delay(200);
    
    safePrint("Serial1 initialized: ");
    safePrint("RX pin: "); safePrint(String(IPS_RX_PIN));
    safePrint(", TX pin: "); safePrint(String(IPS_TX_PIN));
    safePrint(", Baud: 115200");
    safePrintln("");
    
    // Wait for sensor boot
    safePrintln("Waiting 5 seconds for IPS sensor boot...");
    delay(5000);
    
    // Send configuration commands  
    safePrintln("Requesting IPS status...");
    Serial1.write("$Wfactory=\r\n");
    delay(100);

    // Disable Azure integration
    Serial1.write("$Wazure=0\r\n");
    delay(100);
    
    // Enable debug mode if configured
    if (config.enableIPSDebug) {
        safePrintln("Enabling IPS debug mode...");
        Serial1.write("$Wdebug=1\r\n");
        Serial1.write("$Won=3\r\n");
        ipsSensorData.won=3;
        delay(100);
        ipsSensorData.debugMode = true;
    } else {
        safePrintln("Using IPS normal mode...");
        Serial1.write("$Wdebug=0\r\n");
        delay(100);
        ipsSensorData.debugMode = false;
    }
    
    // Clear any startup data
    while (Serial1.available()) {
        String c = Serial1.readStringUntil('\n');
        safePrint(c);
    }
    safePrintln("");
    
    ipsSensorStatus = true;
    safePrintln("IPS sensor Serial1 initialization complete");
}

void readIPSSensor() {
    if (!config.enableIPS || !ipsSensorStatus) return;
    
    static unsigned long lastReadTime = 0;
    static int consecutiveErrors = 0;
    unsigned long currentTime = millis();
    
    // Check for data more frequently
    if (Serial1.available()) {
        String serial2Out = Serial1.readStringUntil('\n');
        String fullOutput = serial2Out + "\r\n";
        
        if (serial2Out.length() > 0) {
            // Try to parse the data - sprawdź czy to debug mode czy normalny
            bool parseSuccess = false;
            if (ipsSensorData.debugMode) {
                parseSuccess = parseIPSDebugData(serial2Out);
            } else {
                parseSuccess = parseIPSUARTData(serial2Out);
            }
            
            if (parseSuccess) {
                ipsSensorData.lastUpdate = currentTime;
                ipsSensorStatus = true;
                consecutiveErrors = 0;
            } else {
                consecutiveErrors++;
                if (consecutiveErrors <= 3) {
                    safePrintln("Failed to parse IPS data (attempt " + String(consecutiveErrors) + ")");
                    ipsSensorStatus = false;
                }
            }
        }
    }
    
    // Handle no data timeout
    if (currentTime - lastReadTime >= 15000) { // Check every 15 seconds
        lastReadTime = currentTime;
        
        if (consecutiveErrors > 5) {
            static unsigned long lastErrorReport = 0;
            if (currentTime - lastErrorReport > 30000) {
                safePrintln("IPS sensor disabled due to consecutive errors");
               
                lastErrorReport = currentTime;
            }
            ipsSensorStatus = false;
        } else {
            static unsigned long lastNoDataReport = 0;
            if (currentTime - lastNoDataReport > 15000) {
                safePrintln("No IPS Serial1 data received in last 15 seconds");
               
                lastNoDataReport = currentTime;
            }
        }
    }
}

bool parseIPSUARTData(String data) {
    // Parse IPS UART data in format: PC0.1,value,PC0.3,value,PM0.1,value...
    data.trim();
    
    if (data.length() < 10) return false;
    
    // Parse key-value pairs separated by commas
    int startIndex = 0;
    bool parsingKey = true;
    String currentKey = "";
    bool hasValidData = false;  // Sprawdź czy są niezerowe wartości
    
    for (int i = 0; i <= data.length(); i++) {
        if (i == data.length() || data.charAt(i) == ',') {
            String segment = data.substring(startIndex, i);
            segment.trim();
            
            if (parsingKey) {
                currentKey = segment;
                parsingKey = false;
            } else {
                // Parse value for the current key
                float value = segment.toFloat();
                
                // Map keys to array indices - AKTUALIZUJ TYLKO NIEZEROWE WARTOŚCI
                if (currentKey.equals("PC0.1") && value > 0) {
                    ipsSensorData.pc_values[0] = (unsigned long)value;
                    hasValidData = true;
                } else if (currentKey.equals("PC0.3") && value > 0) {
                    ipsSensorData.pc_values[1] = (unsigned long)value;
                    hasValidData = true;
                } else if (currentKey.equals("PC0.5") && value > 0) {
                    ipsSensorData.pc_values[2] = (unsigned long)value;
                    hasValidData = true;
                } else if (currentKey.equals("PC1.0") && value > 0) {
                    ipsSensorData.pc_values[3] = (unsigned long)value;
                    hasValidData = true;
                } else if (currentKey.equals("PC2.5") && value > 0) {
                    ipsSensorData.pc_values[4] = (unsigned long)value;
                    hasValidData = true;
                } else if (currentKey.equals("PC5.0") && value > 0) {
                    ipsSensorData.pc_values[5] = (unsigned long)value;
                    hasValidData = true;
                } else if (currentKey.equals("PC10") && value > 0) {
                    ipsSensorData.pc_values[6] = (unsigned long)value;
                    hasValidData = true;
                } else if (currentKey.equals("PM0.1") && value > 0.0) {
                    ipsSensorData.pm_values[0] = value;
                    hasValidData = true;
                } else if (currentKey.equals("PM0.3") && value > 0.0) {
                    ipsSensorData.pm_values[1] = value;
                    hasValidData = true;
                } else if (currentKey.equals("PM0.5") && value > 0.0) {
                    ipsSensorData.pm_values[2] = value;
                    hasValidData = true;
                } else if (currentKey.equals("PM1.0") && value > 0.0) {
                    ipsSensorData.pm_values[3] = value;
                    hasValidData = true;
                } else if (currentKey.equals("PM2.5") && value > 0.0) {
                    ipsSensorData.pm_values[4] = value;
                    hasValidData = true;
                } else if (currentKey.equals("PM5.0") && value > 0.0) {
                    ipsSensorData.pm_values[5] = value;
                    hasValidData = true;
                } else if (currentKey.equals("PM10") && value > 0.0) {
                    ipsSensorData.pm_values[6] = value;
                    hasValidData = true;
                }
                
                parsingKey = true;
                currentKey = "";
            }
            startIndex = i + 1;
        }
    }
    
    // Sprawdź czy ramka zawierała jakiekolwiek niezerowe dane
    if (!hasValidData) {
        // Sprawdź czy mamy poprzednie poprawne dane
        for (int i = 0; i < 7; i++) {
            if (ipsSensorData.pc_values[i] > 0 || ipsSensorData.pm_values[i] > 0) {
                hasValidData = true;
                break;
            }
        }
    }
    
    ipsSensorData.valid = hasValidData;
    return hasValidData;
}

bool parseIPSDebugData(String data) {
    // Parse IPS debug data format zgodnie z dokumentacją
    data.trim();
    
    safePrint("Parsing IPS DEBUG data: ");
    safePrintln(data);
    
    if (data.length() < 10) return false;
    
    // Sprawdź czy zaczyna się od "$Won="
    if (data.startsWith("$Won=")) {
        int wonValue = data.substring(5, 6).toInt();
        ipsSensorData.won = wonValue;
        return true;
    }
    
    // Sprawdź czy to linia z danymi (zaczyna się od "Init" lub zawiera "N0[x]")
    if (data.startsWith("Init") || data.indexOf("N0[x]") >= 0) {
        return true;
    }
    
    // Parse data lines
    // Clear TYLKO debug arrays (NP i PW) - zachowaj PC i PM
    for (int i = 0; i < 7; i++) {
        ipsSensorData.np_values[i] = 0;     // Zawsze zeruj NP (debug data)
        ipsSensorData.pw_values[i] = 0;     // Zawsze zeruj PW (debug data)
    }
    
    // Split by commas and parse
    int startIndex = 0;
    String token = "";
    String currentSection = "";
    int arrayIndex = 0;
    bool hasValidPCPM = false;
    bool hasDebugData = false;
    
    for (int i = 0; i <= data.length(); i++) {
        if (i == data.length() || data.charAt(i) == ',') {
            token = data.substring(startIndex, i);
            token.trim();
            
            // Detect section headers
            if (token.startsWith("N0[x]")) {
                currentSection = "N";
                arrayIndex = 0;
            } else if (token.startsWith("P[x]")) {
                currentSection = "P";  
                arrayIndex = 0;
            } else if (token.startsWith("PC0.1")) {
                currentSection = "PC";
                arrayIndex = 0;
            } else if (token.startsWith("PC0.3")) {
                arrayIndex = 1;
            } else if (token.startsWith("PC0.5")) {
                arrayIndex = 2;
            } else if (token.startsWith("PC1.0")) {
                arrayIndex = 3;
            } else if (token.startsWith("PC2.5")) {
                arrayIndex = 4;
            } else if (token.startsWith("PC5.0")) {
                arrayIndex = 5;
            } else if (token.startsWith("PC10")) {
                arrayIndex = 6;
            } else if (token.startsWith("PM0.1")) {
                currentSection = "PM";
                arrayIndex = 0;
            } else if (token.startsWith("PM0.3")) {
                arrayIndex = 1;
            } else if (token.startsWith("PM0.5")) {
                arrayIndex = 2;
            } else if (token.startsWith("PM1.0")) {
                arrayIndex = 3;
            } else if (token.startsWith("PM2.5")) {
                arrayIndex = 4;
            } else if (token.startsWith("PM5.0")) {
                arrayIndex = 5;
            } else if (token.startsWith("PM10")) {
                arrayIndex = 6;
            } else {
                // Try to parse as numeric value
                float value = token.toFloat();
                unsigned long uvalue = (unsigned long)value;
                
                if (currentSection == "N" && arrayIndex < 7) {
                    ipsSensorData.np_values[arrayIndex] = uvalue;
                    hasDebugData = true;
                    arrayIndex++;
                } else if (currentSection == "P" && arrayIndex < 7) {
                    ipsSensorData.pw_values[arrayIndex] = uvalue;
                    hasDebugData = true;
                    arrayIndex++;
                } else if (currentSection == "PC" && arrayIndex < 7) {
                    ipsSensorData.pc_values[arrayIndex] = uvalue;
                    hasValidPCPM = true;
                } else if (currentSection == "PM" && arrayIndex < 7) {
                    if (value > 0.0) {
                        ipsSensorData.pm_values[arrayIndex] = value;
                        hasValidPCPM = true;
                    }
                }
            }
            
            startIndex = i + 1;
        }
    }
    
    // Sprawdź czy mamy jakiekolwiek znaczące dane
    bool hasData = hasDebugData || hasValidPCPM;
    
    // Jeśli nie ma nowych niezerowych PC/PM, sprawdź poprzednie
    if (!hasValidPCPM) {
        for (int i = 0; i < 7; i++) {
            if (ipsSensorData.pc_values[i] > 0 || ipsSensorData.pm_values[i] > 0.0) {
                hasData = true;
                break;
            }
        }
    }
    
    ipsSensorData.valid = hasData;
    return hasData;
}

bool readIPS(IPSSensorData& data) {
    // For UART implementation, this function checks if fresh data is available
    if (!ipsSensorData.valid) {
        data.valid = false;
        return false;
    }
    
    // Copy current data
    for (int i = 0; i < 7; i++) {
        data.pc_values[i] = ipsSensorData.pc_values[i];
        data.pm_values[i] = ipsSensorData.pm_values[i];
        data.np_values[i] = ipsSensorData.np_values[i];  // Debug data
        data.pw_values[i] = ipsSensorData.pw_values[i];  // Debug data
    }
    
    // Copy debug mode info
    data.debugMode = ipsSensorData.debugMode;
    data.won = ipsSensorData.won;
    
    data.valid = true;
    data.lastUpdate = ipsSensorData.lastUpdate;
    return true;
}

bool testIPSCommunication() {
    // For UART, test if serial port is available and working
    if (!Serial1) return false;
    
    // Send a simple command and check response (optional)
    return true;
}

void enableIPSDebugMode() {
    safePrintln("Enabling IPS debug mode...");
    Serial1.write("$Wdebug=1\r\n");
    delay(100);
    ipsSensorData.debugMode = true;
    config.enableIPSDebug = true;
}

void disableIPSDebugMode() {
    safePrintln("Disabling IPS debug mode...");
    Serial1.write("$Wdebug=0\r\n");
    delay(100);
    ipsSensorData.debugMode = false;
    config.enableIPSDebug = false;
}

void resetIPSData() {
    // Reset IPS data 
    for (int i = 0; i < 7; i++) {
        ipsSensorData.pc_values[i] = 0;
        ipsSensorData.pm_values[i] = 0.0;
        ipsSensorData.np_values[i] = 0;
        ipsSensorData.pw_values[i] = 0;
    }
    ipsSensorData.debugMode = false;
    ipsSensorData.won = 0;
    ipsSensorData.valid = false;
} 