#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#include <CB_HCHO_V4.h>
#include <config.h>

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

// Global HCHO sensor object and data - using EspSoftwareSerial
EspSoftwareSerial::UART hchoSerial; // Use EspSoftwareSerial for HCHO
CB_HCHO_V4 hchoSensor(&hchoSerial);
extern HCHOData hchoData;
extern bool hchoSensorStatus;
extern FeatureConfig config;

// Last read time for interval control
unsigned long lastHCHORead = 0;
bool hchoInitialized = false;

// Licznik nieudanych prob dla HCHO sensora
static int hchoFailCount = 0;
static const int MAX_HCHO_FAILS = 20; // Po 20 nieudanych probach zmien status na false

void initializeHCHO() {
    if (!config.enableHCHO) {
        hchoSensorStatus = false;
        return;
    }
    
    // Reset licznik nieudanych prob przy inicjalizacji
    hchoFailCount = 0;
    
    safePrintln("Initializing HCHO sensor (CB-HCHO-V4) with EspSoftwareSerial...");
    
    // Initialize EspSoftwareSerial for HCHO sensor with proper pin configuration
    hchoSerial.begin(HCHO_SERIAL_BAUD, SWSERIAL_8N1, HCHO_RX_PIN, HCHO_TX_PIN, false);
    hchoSerial.setTimeout(HCHO_TIMEOUT_MS);

    // hchoSerial.print("AT\r\n");
    // delay(1000);
    // hchoSerial.print("AT+VER\r\n");
    // delay(1000);
    // hchoSerial.print("AT+VER\r\n");
    // delay(1000);
    
    // Check if the serial port initialized correctly
    if (!hchoSerial) {
        safePrintln("Failed to initialize HCHO serial port - invalid pin configuration");
        hchoSensorStatus = false;
        hchoInitialized = false;
        return;
    }
    
    // Wait for sensor to stabilize
   // delay(1000);
    
    // Try to read from sensor to verify connection
    bool testRead = hchoSensor.read();
    
    if (testRead) {
        hchoSensorStatus = true;
        hchoInitialized = true;
        lastHCHORead = millis();
        
        safePrint("HCHO sensor initialized successfully - Status: ");
        safePrint(String(hchoSensor.getSensorStatus()));
        safePrint(", Auto Cal: ");
        safePrintln(String(hchoSensor.getAutoCalibrationSwitch()));
        
        // Set auto calibration to manual mode for stable readings
        if (hchoSensor.setAutoCalibration(CB_HCHO_V4_ACS_MANUAL)) {
            safePrintln("HCHO auto calibration set to manual mode");
        } else {
            safePrintln("Warning: Failed to set HCHO auto calibration mode");
        }
        
    } else {
        hchoSensorStatus = false;
        hchoInitialized = false;
        safePrintln("Failed to initialize HCHO sensor - no response");
    }
}

bool readHCHO() {
    if (!config.enableHCHO || !hchoInitialized) {
        return false;
    }
    
    // Check if enough time has passed since last read
    if (millis() - lastHCHORead < HCHO_READ_INTERVAL) {
        return true; // Return true but don't read yet
    }
    
    bool success = hchoSensor.read();
    
    if (success) {
        // Reset licznik nieudanych prob przy udanym odczycie
        hchoFailCount = 0;
        
        // Only collect HCHO data as requested
        hchoData.hcho = hchoSensor.getHcho();
        hchoData.tvoc = hchoSensor.getTvoc();
        //calculate mg/m3 to ppb for HCHO (molar mass: 30.03 g/mol)
        hchoData.hcho_ppb = hchoData.hcho * 814.2; // ppb = mg/m³ × (24.45/30.03) × 1000
        hchoData.valid = true;
        hchoData.lastUpdate = millis();
        
        hchoSensorStatus = true;
        lastHCHORead = millis();
        
        // Debug output every 10 reads
        static int readCount = 0;
        readCount++;
        if (readCount % 10 == 0) {
            safePrint("HCHO reading - HCHO: ");
            safePrint(String(hchoData.hcho, 3));
            safePrintln(" mg/m³");
            //voc
            safePrint("HCHO reading - TVOC: ");
            safePrint(String(hchoData.tvoc, 3));
            safePrintln(" ppb");
        }
        
        return true;
        
    } else {
        // Zwieksz licznik nieudanych prob
        hchoFailCount++;
        
        // Failed to read
        hchoData.valid = false;
        
        // Sprawdz czy przekroczono limit nieudanych prob
        if (hchoFailCount >= MAX_HCHO_FAILS) {
            hchoSensorStatus = false;
            safePrint("HCHO sensor status changed to FALSE after ");
            safePrint(String(hchoFailCount));
            safePrintln(" failed attempts");
        }
        
        static unsigned long lastErrorLog = 0;
        if (millis() - lastErrorLog > 30000) { // Log error every 30 seconds
            lastErrorLog = millis();
            safePrint("Error: Failed to read from HCHO sensor (fail count: ");
            safePrint(String(hchoFailCount));
            safePrintln("/" + String(MAX_HCHO_FAILS) + ")");
        }
        
        return false;
    }
}

bool isHCHODataValid() {
    return hchoData.valid && (millis() - hchoData.lastUpdate) < SENSOR_TIMEOUT;
}

void resetHCHOData() {
    hchoData.hcho = 0.0;
    hchoData.valid = false;
    hchoData.lastUpdate = 0;
}

// Funkcja do resetowania licznika nieudanych prob
void resetHCHOFailCount() {
    hchoFailCount = 0;
    safePrintln("HCHO fail count reset to 0");
}

// Configuration functions
bool setHCHOAutoCalibration(uint8_t mode) {
    if (!config.enableHCHO || !hchoInitialized) {
        return false;
    }
    
    bool result = hchoSensor.setAutoCalibration(mode);
    if (result) {
        safePrint("HCHO auto calibration mode set to: ");
        safePrintln(String(mode));
    } else {
        safePrintln("Failed to set HCHO auto calibration mode");
    }
    
    return result;
}

float getHCHOConcentration() {
    if (!hchoData.valid) {
        return -1.0; // Invalid reading
    }
    return hchoData.hcho;
}

// Diagnostic function
void printHCHODiagnostics() {
    safePrintln("=== HCHO Sensor Diagnostics ===");
    safePrint("Enabled: "); safePrintln(config.enableHCHO ? "YES" : "NO");
    safePrint("Initialized: "); safePrintln(hchoInitialized ? "YES" : "NO");
    safePrint("Status: "); safePrintln(hchoSensorStatus ? "OK" : "ERROR");
    safePrint("Data Valid: "); safePrintln(hchoData.valid ? "YES" : "NO");
    safePrint("Fail Count: "); safePrint(String(hchoFailCount)); safePrintln("/" + String(MAX_HCHO_FAILS));
    
    if (hchoData.valid) {
        safePrint("HCHO: "); safePrint(String(hchoData.hcho, 3)); safePrintln(" mg/m³");
        safePrint("Data Age: "); safePrint(String((millis() - hchoData.lastUpdate) / 1000)); safePrintln(" seconds");
    }
    
    safePrintln("Communication: EspSoftwareSerial");
    safePrint("Serial RX Pin: "); safePrintln(String(HCHO_RX_PIN));
    safePrint("Serial TX Pin: "); safePrintln(String(HCHO_TX_PIN));
    safePrint("Baud Rate: "); safePrintln(String(HCHO_SERIAL_BAUD));
    safePrintln("===============================");
}

