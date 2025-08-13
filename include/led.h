#ifndef LED_H
#define LED_H

#include <Adafruit_NeoPixel.h>

// Initialize external NeoPixel strip (EXT_LED_PIN, EXT_LED_COUNT)
void initExtLeds();

// Set all external LEDs to color
void extLedSetAll(uint8_t r, uint8_t g, uint8_t b);

// Set single external LED pixel color
void extLedSetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b);

// Accessor to the external strip object
Adafruit_NeoPixel& getExtPixels();

// Set base color according to PM2.5 index (calibrated value in ug/m3)
void ledSetAirQualityColorFromPM25(float pm25);

// Non-blocking breathing effect update (call each loop)
void updateExtBreathing();

// Enable/disable debug override: force solid white and disable breathing/color mapping
void ledSetDebugWhite(bool enabled);

#endif // LED_H

