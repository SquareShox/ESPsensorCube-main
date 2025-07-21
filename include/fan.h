#ifndef FAN_H
#define FAN_H

#include <Arduino.h>

// Fan status structure
struct FanStatus {
    bool enabled;
    uint8_t dutyCycle;  // 0-100%
    uint16_t rpm;
    bool glineEnabled;
    bool valid;
};

// Function declarations
void initializeFan();
void setFanSpeed(uint8_t dutyCycle);  // 0-100%
void setGLine(bool enabled);
void updateFanRPM();

// Getter functions
bool isFanEnabled();
uint8_t getFanDutyCycle();
uint16_t getFanRPM();
bool isGLineEnabled();
FanStatus getFanStatus();

// JSON and command processing
String getFanJson();
bool processFanCommand(const String& command, const String& value);

// External variables for sensor data
extern FanStatus fanStatus;

#endif // FAN_H 