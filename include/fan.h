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
    bool sleepMode;     // Czy tryb sleep jest aktywny
    unsigned long sleepStartTime;  // Czas rozpoczęcia sleep
    unsigned long sleepDuration;   // Czas trwania sleep w ms
    unsigned long sleepEndTime;    // Czas zakończenia sleep
};

// Function declarations
void initializeFan();
void setFanSpeed(uint8_t dutyCycle);  // 0-100%
void setGLine(bool enabled);
void updateFanRPM();

// Sleep functionality
void startSleepMode(unsigned long delaySeconds, unsigned long durationSeconds);
void stopSleepMode();
void updateSleepMode();
bool isSleepModeActive();

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