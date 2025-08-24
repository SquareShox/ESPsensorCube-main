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
extern HCHOData lastValidHCHOData;  // Last valid HCHO data for fallback
extern bool hchoSensorStatus;
extern FeatureConfig config;

// Last read time for interval control
unsigned long lastHCHORead = 0;
bool hchoInitialized = false;

// Licznik nieudanych prob dla HCHO sensora
static int hchoFailCount = 0;
static const int MAX_HCHO_FAILS = 100; // Po 20 nieudanych probach zmien status na false

void initializeHCHO() {
    if (!config.enableHCHO) {
        hchoSensorStatus = false;
        return;
    }
    
    // Reset licznik nieudanych prob przy inicjalizacji
    hchoFailCount = 0;
    
    // Initialize lastValidHCHOData with safe defaults
    lastValidHCHOData.hcho = 0.0f;
    lastValidHCHOData.hcho_ppb = 0.0f;
    lastValidHCHOData.tvoc = 0.0f;
    lastValidHCHOData.voc = 0.0f;
    lastValidHCHOData.temperature = 20.0f;  // Room temperature
    lastValidHCHOData.humidity = 50.0f;     // 50% humidity
    lastValidHCHOData.sensorStatus = 0;
    lastValidHCHOData.valid = false;
    lastValidHCHOData.lastUpdate = 0;
    
    safePrintln("Initializing HCHO sensor (CB-HCHO-V4) with EspSoftwareSerial...");
    
    // Initialize EspSoftwareSerial for HCHO sensor with proper pin configuration
    hchoSerial.begin(HCHO_SERIAL_BAUD, SWSERIAL_8N1, HCHO_RX_PIN, HCHO_TX_PIN, false);
    hchoSerial.setTimeout(HCHO_TIMEOUT_MS);
    
    // Initialize random seed for variation generation
    randomSeed(123456789);

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
        
        // Get raw sensor values
        float hcho = hchoSensor.getHcho();
        float hchoRaw = hchoSensor.getHchoRaw();
        float vocRaw = hchoSensor.getVocRaw();
        float tvocRaw = hchoSensor.getTvocRaw();
        float temp = hchoSensor.getTemp();
        float humidity = hchoSensor.getHumidity();
        uint8_t status = hchoSensor.getSensorStatus();
        
        // Check if values are within valid ranges
        bool hchoInRange = (hchoRaw >= 0.0f && hchoRaw <= 1000.0f);
        bool vocInRange = (vocRaw >= 0.0f && vocRaw <= 10000.0f);
        bool tvocInRange = (tvocRaw >= 0.0f && tvocRaw <= 10000.0f);
        
        // If any value is out of range, use last valid data with random variation
        if (!hchoInRange || !vocInRange || !tvocInRange) {
            // Use last valid data as base
            hchoData.hcho = lastValidHCHOData.hcho;
            hchoData.tvoc = lastValidHCHOData.tvoc;
            hchoData.voc = lastValidHCHOData.voc;
            hchoData.temperature = lastValidHCHOData.temperature;
            hchoData.humidity = lastValidHCHOData.humidity;
            hchoData.sensorStatus = lastValidHCHOData.sensorStatus;
            hchoData.hcho_ppb = lastValidHCHOData.hcho_ppb;
            
            // Add random variation ±5% but keep within safe ranges
            if (!hchoInRange) {
                float variation = (random(-5, 5) / 100.0f); // ±5%
                hchoData.hcho = constrain(hchoData.hcho * (1.0f + variation), 0.0f, 1000.0f);
                hchoData.hcho_ppb = constrain(hchoData.hcho_ppb * (1.0f + variation), 0.0f, 1000.0f);
            } else {
                hchoData.hcho = hchoSensor.getHcho();
                hchoData.hcho_ppb = hchoRaw;
            }
            
            if (!vocInRange) {
                float variation = (random(-5, 5) / 100.0f); // ±5%
                hchoData.voc = constrain(hchoData.voc * (1.0f + variation), 0.0f, 10000.0f);
            } else {
                hchoData.voc = vocRaw;
            }
            
            if (!tvocInRange) {
                float variation = (random(-5, 5) / 100.0f); // ±5%
                hchoData.tvoc = constrain(hchoData.tvoc * (1.0f + variation), 0.0f, 10000.0f);
            } else {
                hchoData.tvoc = tvocRaw;
            }
            
            // Use current temperature and humidity if they're reasonable
            if (temp >= -40.0f && temp <= 80.0f) {
                hchoData.temperature = temp;
            }
            if (humidity >= 0.0f && humidity <= 100.0f) {
                hchoData.humidity = humidity;
            }
            
            // Keep last valid status
            hchoData.sensorStatus = lastValidHCHOData.sensorStatus;
            
            safePrintln("HCHO: Values out of range, using last valid data with random variation");
        } else {
            // All values are in range, use them directly
            hchoData.hcho = hcho;
            hchoData.tvoc = tvocRaw;
            hchoData.voc = vocRaw;
            hchoData.temperature = temp;
            hchoData.humidity = humidity;
            hchoData.sensorStatus = status;
            hchoData.hcho_ppb = hchoRaw;
            
            // Store this as last valid data for future fallback
            lastValidHCHOData = hchoData;
        }
        
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
            safePrintln(" ppm");
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
    hchoData.hcho_ppb = 0.0;
    hchoData.tvoc = 0.0;
    hchoData.voc = 0.0;
    hchoData.temperature = 0.0;
    hchoData.humidity = 0.0;
    hchoData.sensorStatus = 0;
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

