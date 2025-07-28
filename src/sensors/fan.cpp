#include <fan.h>
#include <config.h>
#include <Arduino.h>

// Fan control variables
static bool fanEnabled = false;
static uint8_t fanDutyCycle = 0;  // 0-255
static uint16_t fanRPM = 0;
static bool glineEnabled = false;
static unsigned long lastTachoPulse = 0;
static unsigned long tachoPulseCount = 0;
static unsigned long lastRPMCalculation = 0;

// Sleep mode variables
static bool sleepMode = false;
static unsigned long sleepStartTime = 0;
static unsigned long sleepDuration = 0;
static unsigned long sleepEndTime = 0;
static bool glineStateBeforeSleep = false;

// PWM configuration
static const int PWM_FREQ = 25000;  // 25kHz for quiet operation
static const int PWM_RESOLUTION = 8; // 8-bit resolution (0-255)
static const int PWM_CHANNEL = 0;    // PWM channel 0

// Tacho configuration
static const unsigned long TACHO_TIMEOUT = 1000; // 1 second timeout for RPM calculation
static const int PULSES_PER_REVOLUTION = 2;      // Standard for 3-wire fans

void safePrint(const String& message);
void safePrintln(const String& message);

// Fan control functions
void initializeFan() {
    // Configure PWM for fan control
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, 128); // Start with fan off
    
    // Configure tacho pin as input with pullup
    pinMode(TACHO_PIN, INPUT_PULLUP);
    
    // Configure GLine pin as output
    pinMode(GLine_PIN, OUTPUT);
    digitalWrite(GLine_PIN, HIGH); // Start with router ON
    
    safePrintln("Fan system initialized");
    safePrintln("PWM Pin: " + String(PWM_PIN) + " (Channel " + String(PWM_CHANNEL) + ")");
    safePrintln("Tacho Pin: " + String(TACHO_PIN));
    safePrintln("GLine Pin: " + String(GLine_PIN));
}

void setFanSpeed(uint8_t dutyCycle) {
    if (dutyCycle > 100) dutyCycle = 100; // Clamp to 0-100%
    
    fanDutyCycle = dutyCycle;
    uint8_t pwmValue = map(dutyCycle, 0, 100, 0, 255);
    
    if (dutyCycle == 0) {
        fanEnabled = false;
        ledcWrite(PWM_CHANNEL, 0);
        safePrintln("Fan turned OFF");
    } else {
        fanEnabled = true;
        ledcWrite(PWM_CHANNEL, pwmValue);
        safePrintln("Fan speed set to " + String(dutyCycle) + "% (PWM: " + String(pwmValue) + ")");
    }
}

void setGLine(bool enabled) {
    glineEnabled = enabled;
    digitalWrite(GLine_PIN, enabled ? HIGH : LOW);
    safePrintln("GLine router " + String(enabled ? "ENABLED" : "DISABLED"));
}

// Sleep mode functions
void startSleepMode(unsigned long delaySeconds, unsigned long durationSeconds) {
    if (sleepMode) {
        safePrintln("Sleep mode already active");
        return;
    }
    
    sleepMode = true;
    sleepStartTime = millis() + (delaySeconds * 1000); // Start after delay
    sleepDuration = durationSeconds * 1000; // Convert to milliseconds
    sleepEndTime = sleepStartTime + sleepDuration;
    
    // Store current GLine state
    glineStateBeforeSleep = glineEnabled;
    
    safePrintln("Sleep mode scheduled:");
    safePrintln("- Start: " + String(delaySeconds) + " seconds from now");
    safePrintln("- Duration: " + String(durationSeconds) + " seconds");
    safePrintln("- End time: " + String(sleepEndTime));
}

void stopSleepMode() {
    if (!sleepMode) {
        safePrintln("Sleep mode not active");
        return;
    }
    
    sleepMode = false;
    
    // Restore GLine to previous state
    setGLine(glineStateBeforeSleep);
    
    safePrintln("Sleep mode stopped - GLine restored to previous state");
}

void updateSleepMode() {
    if (!sleepMode) return;
    
    unsigned long currentTime = millis();
    
    // Check if it's time to start sleep
    if (currentTime >= sleepStartTime && glineEnabled) {
        setGLine(false);
        safePrintln("Sleep mode started - GLine DISABLED");
    }
    
    // Check if sleep should end
    if (currentTime >= sleepEndTime) {
        stopSleepMode();
    }
}

bool isSleepModeActive() {
    return sleepMode;
}

void updateFanRPM() {
    static unsigned long lastPulseTime = 0;
    unsigned long currentTime = millis();
    
    // Check for tacho pulse (falling edge detection)
    static bool lastTachoState = HIGH;
    bool currentTachoState = digitalRead(TACHO_PIN);
    
    if (lastTachoState == HIGH && currentTachoState == LOW) {
        // Falling edge detected - pulse received
        tachoPulseCount++;
        lastPulseTime = currentTime;
    }
    
    lastTachoState = currentTachoState;
    
    // Calculate RPM every second
    if (currentTime - lastRPMCalculation >= 1000) {
        if (tachoPulseCount > 0) {
            // Calculate RPM: (pulses * 60 seconds) / (pulses per revolution * time in seconds)
            unsigned long timeDiff = currentTime - lastRPMCalculation;
            fanRPM = (tachoPulseCount * 60000) / (PULSES_PER_REVOLUTION * timeDiff);
            tachoPulseCount = 0;
        } else {
            // No pulses detected - fan might be stopped or disconnected
            if (currentTime - lastPulseTime > TACHO_TIMEOUT) {
                fanRPM = 0;
            }
        }
        
        lastRPMCalculation = currentTime;
    }
    
    // Update sleep mode
    updateSleepMode();
}

// Getter functions
bool isFanEnabled() {
    return fanEnabled;
}

uint8_t getFanDutyCycle() {
    return fanDutyCycle;
}

uint16_t getFanRPM() {
    return fanRPM;
}

bool isGLineEnabled() {
    return glineEnabled;
}

// Fan status structure
FanStatus getFanStatus() {
    FanStatus status;
    status.enabled = fanEnabled;
    status.dutyCycle = fanDutyCycle;
    status.rpm = fanRPM;
    status.glineEnabled = glineEnabled;
    status.sleepMode = sleepMode;
    status.sleepStartTime = sleepStartTime;
    status.sleepDuration = sleepDuration;
    status.sleepEndTime = sleepEndTime;
    status.valid = true;
    return status;
}

// JSON output for WebSocket
String getFanJson() {
    String json = "{";
    json += "\"enabled\":" + String(fanEnabled ? "true" : "false") + ",";
    json += "\"dutyCycle\":" + String(fanDutyCycle) + ",";
    json += "\"rpm\":" + String(fanRPM) + ",";
    json += "\"glineEnabled\":" + String(glineEnabled ? "true" : "false") + ",";
    json += "\"sleepMode\":" + String(sleepMode ? "true" : "false") + ",";
    json += "\"sleepStartTime\":" + String(sleepStartTime) + ",";
    json += "\"sleepDuration\":" + String(sleepDuration) + ",";
    json += "\"sleepEndTime\":" + String(sleepEndTime) + ",";
    json += "\"valid\":true";
    json += "}";
    return json;
}

// Command processing for Serial and WebSocket
bool processFanCommand(const String& command, const String& value) {
    if (command == "fan_speed" || command == "speed") {
        int speed = value.toInt();
        if (speed >= 0 && speed <= 100) {
            setFanSpeed(speed);
            return true;
        } else {
            safePrintln("Invalid fan speed: " + value + " (0-100)");
            return false;
        }
    } else if (command == "fan_on" || command == "on") {
        setFanSpeed(50); // Default 50% speed
        return true;
    } else if (command == "fan_off" || command == "off") {
        setFanSpeed(0);
        return true;
    } else if (command == "gline_on") {
        setGLine(true);
        return true;
    } else if (command == "gline_off") {
        setGLine(false);
        return true;
    } else if (command == "sleep") {
        // Format: sleep [delay] [duration]
        int spaceIndex = value.indexOf(' ');
        if (spaceIndex > 0) {
            unsigned long delaySec = value.substring(0, spaceIndex).toInt();
            unsigned long durationSec = value.substring(spaceIndex + 1).toInt();
            if (delaySec > 0 && durationSec > 0) {
                startSleepMode(delaySec, durationSec);
                return true;
            }
        }
        safePrintln("Invalid sleep format: sleep [delay_seconds] [duration_seconds]");
        return false;
    } else if (command == "sleep_stop" || command == "wake") {
        stopSleepMode();
        return true;
    } else if (command == "status") {
        FanStatus status = getFanStatus();
        safePrintln("Fan Status:");
        safePrintln("- Enabled: " + String(status.enabled ? "YES" : "NO"));
        safePrintln("- Duty Cycle: " + String(status.dutyCycle) + "%");
        safePrintln("- RPM: " + String(status.rpm));
        safePrintln("- GLine: " + String(status.glineEnabled ? "ON" : "OFF"));
        safePrintln("- Sleep Mode: " + String(status.sleepMode ? "ACTIVE" : "INACTIVE"));
        if (status.sleepMode) {
            safePrintln("- Sleep Start: " + String(status.sleepStartTime));
            safePrintln("- Sleep Duration: " + String(status.sleepDuration) + "ms");
            safePrintln("- Sleep End: " + String(status.sleepEndTime));
        }
        return true;
    }
    
    return false;
}
