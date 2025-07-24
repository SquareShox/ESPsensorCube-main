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
    json += "\"pwmValue\":" + String(map(fanDutyCycle, 0, 100, 0, 255)) + ",";
    json += "\"pwmFreq\":" + String(PWM_FREQ);
    json += "}";
    return json;
}

// Command processing for WebSocket
bool processFanCommand(const String& command, const String& value) {
    if (command == "fan_speed") {
        int speed = value.toInt();
        if (speed >= 0 && speed <= 100) {
            setFanSpeed(speed);
            return true;
        }
    } else if (command == "fan_on") {
        setFanSpeed(50); // Default 50% speed
        return true;
    } else if (command == "fan_off") {
        setFanSpeed(0);
        return true;
    } else if (command == "gline_on") {
        setGLine(true);
        return true;
    } else if (command == "gline_off") {
        setGLine(false);
        return true;
    }
    
    return false; // Unknown command
}
