#include "opcn3_sensor.h"
#include <sensors.h>
#include <Adafruit_NeoPixel.h>

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

// Global sensor data
extern HistogramData opcn3Data;
extern OPCN3 myOPCN3;
extern FeatureConfig config;
extern Adafruit_NeoPixel pixels;

// Global status flags - extern declaration (defined in main sensors.cpp)
extern bool opcn3SensorStatus;

void initializeOPCN3() {
    myOPCN3.initialize();
    delay(1000);
    
    // Test OPCN3 communication
    DACandPowerStatus status = myOPCN3.readDACandPowerStatus();
    if (status.valid) {
        opcn3SensorStatus = true;
        safePrintln("OPCN3 sensor initialized successfully");
    } else {
        opcn3SensorStatus = false;
        safePrintln("OPCN3 sensor initialization failed");
    }
}

void readOPCN3Sensor() {
    if (!config.enableOPCN3Sensor) return;
    
    static unsigned long lastReadTime = 0;
    static unsigned long lastSendTime = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastReadTime >= OPCN3_READ_INTERVAL) {
        lastReadTime = currentTime;
        opcn3Data = myOPCN3.readHistogramData();
        
        if (opcn3Data.valid) {
            opcn3SensorStatus = true;
            
            if (currentTime - lastSendTime >= OPCN3_SEND_INTERVAL) {
                lastSendTime = currentTime;
                
                safePrint("OPCN3 - Temp: ");
                safePrint(String(opcn3Data.getTempC()));
                safePrint("°C, Humidity: ");
                safePrint(String(opcn3Data.getHumidity()));
                safePrint(", PM1: ");
                safePrint(String(opcn3Data.pm1));
                safePrint(", PM2.5: ");
                safePrint(String(opcn3Data.pm2_5));
                safePrint(", PM10: ");
                safePrintln(String(opcn3Data.pm10));
                
                // Set LED to green when reading successfully
                pixels.setPixelColor(0, pixels.Color(0, 255, 0));
                pixels.show();
            }
        } else {
            opcn3SensorStatus = false;
            safePrintln("Error reading OPCN3");
            
            // Try to reinitialize
            DACandPowerStatus status = myOPCN3.readDACandPowerStatus();
            if (status.valid) {
                opcn3SensorStatus = true;
            }
            
            // Set LED to red on error
            pixels.setPixelColor(0, pixels.Color(255, 0, 0));
            pixels.show();
        }
    }
}

bool isOPCN3Valid() {
    return opcn3SensorStatus && opcn3Data.valid;
} 