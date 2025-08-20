#include <Adafruit_NeoPixel.h>
#include <config.h>

// Second NeoPixel strip on EXT_LED_PIN
Adafruit_NeoPixel extPixels(EXT_LED_COUNT, EXT_LED_PIN, NEO_GRB + NEO_KHZ800);
static uint8_t baseR = 0, baseG = 0, baseB = 0;
static uint8_t breathPhase = 0; // 0..255 triangle
static unsigned long lastBreathUpdate = 0;
static uint16_t breathPeriodMs = 5000; // full cycle ~2s
static bool debugWhite = false;
static uint8_t baseBrightness = 100; // baseline brightness for breathing
static bool lowPowerModeEnabled = false; // Low power mode flag

void initExtLeds() {
    extPixels.begin();
    extPixels.show(); // Clear
    extPixels.setBrightness(100);
    baseBrightness = 100;
}

void extLedSetAll(uint8_t r, uint8_t g, uint8_t b) {
    if (lowPowerModeEnabled) return; // Skip if low power mode is enabled
    
    for (uint16_t i = 0; i < extPixels.numPixels(); i++) {
        extPixels.setPixelColor(i, extPixels.Color(r, g, b));
    }
    extPixels.show();
}

void extLedSetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b) {
    if (lowPowerModeEnabled) return; // Skip if low power mode is enabled
    if (idx >= extPixels.numPixels()) return;
    extPixels.setPixelColor(idx, extPixels.Color(r, g, b));
    extPixels.show();
}

Adafruit_NeoPixel& getExtPixels() { return extPixels; }

// Map PM2.5 (ug/m3) to color per index table (Bardzo dobry..Bardzo zly)
// Thresholds: 0-13 green, 13.1-35 light green, 35.1-55 yellow, 55.1-75 orange, 75.1-110 red, >110 dark red
static void setBaseColorByPM25(float pm25) {
    if (pm25 <= 13.0f) {            // Bardzo dobry
        baseR = 0;   baseG = 255; baseB = 0;
    } else if (pm25 <= 35.0f) {     // Dobry
        baseR = 102; baseG = 255; baseB = 0;
    } else if (pm25 <= 55.0f) {     // Umiarkowany
        baseR = 255; baseG = 255; baseB = 0;
    } else if (pm25 <= 75.0f) {     // Dostateczny
        baseR = 255; baseG = 128; baseB = 0;
    } else if (pm25 <= 110.0f) {    // Zly
        baseR = 255; baseG = 0;   baseB = 0;
    } else {                        // Bardzo zly
        baseR = 139; baseG = 0;   baseB = 0;
    }
}

void ledSetAirQualityColorFromPM25(float pm25) {
    if (lowPowerModeEnabled) return; // Skip if low power mode is enabled
    
    if (debugWhite) {
        extLedSetAll(255, 255, 255);
        return;
    }
    setBaseColorByPM25(pm25);
    // Immediately apply current base color (breathing will modulate later)
    extLedSetAll(baseR, baseG, baseB);
}

void updateExtBreathing() {
    if (lowPowerModeEnabled) return; // Skip if low power mode is enabled
    
    if (debugWhite) {
        // Keep solid white in debug mode
        extLedSetAll(255, 255, 255);
        return;
    }
    unsigned long now = millis();
    if (now - lastBreathUpdate < 20) return; // ~50 FPS max
    lastBreathUpdate = now;

    // Compute modulation 0.4..1.0 using triangle wave from breathPhase
    // breathPhase advances based on period
    static unsigned long lastPhaseTick = 0;
    unsigned long elapsed = now - lastPhaseTick;
    if (elapsed >= 10) { // update phase every 10ms
        lastPhaseTick = now;
        uint16_t step = (uint16_t)((255UL * elapsed) / (breathPeriodMs / 2));
        breathPhase = (uint8_t)(breathPhase + step);
    }

    // Triangle wave: 0..255..0
    uint8_t tri = (breathPhase < 128) ? (breathPhase * 2) : ((255 - breathPhase) * 2);
    // Brightness scale 40%..100% of baseline
    float scale = 0.2f + (0.6f * (float)tri / 255.0f);
    uint8_t br = (uint8_t)fminf(255.0f, baseBrightness * scale);
    extPixels.setBrightness(br);
    // Keep base color unchanged; just update brightness
    extPixels.show();
}

void ledSetDebugWhite(bool enabled) {
    if (lowPowerModeEnabled) return; // Skip if low power mode is enabled
    
    debugWhite = enabled;
    if (enabled) {
        extPixels.setBrightness(255);
        extLedSetAll(255, 255, 255);
    } else {
        // Re-apply current base color
        extPixels.setBrightness(baseBrightness);
        extLedSetAll(baseR, baseG, baseB);
    }
}

void ledSetLowPowerMode(bool enabled) {
    lowPowerModeEnabled = enabled;
    if (enabled) {
        // Turn off all LEDs and set brightness to 0
        extPixels.setBrightness(0);
        extPixels.show();
    } else {
        // Restore previous brightness and re-apply base color
        extPixels.setBrightness(baseBrightness);
        if (!debugWhite) {
            extLedSetAll(baseR, baseG, baseB);
        }
    }
}
