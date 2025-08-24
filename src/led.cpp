#include <Adafruit_NeoPixel.h>
#include <config.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// Forward declarations for safe printing functions
void safePrintln(const String& message);

// OTA update control functions
void setOtaUpdateStatus(bool inProgress);
bool isOtaUpdateInProgress();

// Second NeoPixel strip on EXT_LED_PIN
Adafruit_NeoPixel extPixels(EXT_LED_COUNT, EXT_LED_PIN, NEO_GRB + NEO_KHZ800);
static uint8_t baseR = 0, baseG = 0, baseB = 0;
static uint8_t breathPhase = 0; // 0..255 triangle
static unsigned long lastBreathUpdate = 0;
static uint16_t breathPeriodMs = 5000; // full cycle ~2s
static bool debugWhite = false;
static uint8_t baseBrightness = 100; // baseline brightness for breathing
static bool lowPowerModeEnabled = false; // Low power mode flag

// FreeRTOS task variables
static TaskHandle_t ledTaskHandle = NULL;
static SemaphoreHandle_t ledMutex = NULL;
static bool ledTaskRunning = false;
static bool otaUpdateInProgress = false; // OTA update detection
static const uint32_t LED_TASK_STACK_SIZE = 8192;
static const uint32_t LED_TASK_PRIORITY = 2;
static const uint32_t LED_UPDATE_INTERVAL_MS = 33; // ~30 FPS (1000ms / 30 = 33.33ms)

void initExtLeds() {
    // Add delay to avoid RMT conflict with internal pixels
    delay(100);
    
    extPixels.begin();
    extPixels.show(); // Clear
    extPixels.setBrightness(100);
    baseBrightness = 100;
    
    // Initialize default colors (green for good air quality)
    baseR = 0;
    baseG = 255;
    baseB = 0;
    
    // Create mutex for thread-safe LED operations
    ledMutex = xSemaphoreCreateMutex();
    if (ledMutex == NULL) {
        safePrintln("ERROR: Failed to create LED mutex in initExtLeds");
        return;
    }
    
    // Set initial LED color for ALL LEDs
    extPixels.setBrightness(baseBrightness);
    for (uint16_t i = 0; i < extPixels.numPixels(); i++) {
        extPixels.setPixelColor(i, extPixels.Color(baseR, baseG, baseB));
    }
    extPixels.show();
    
    safePrintln("External LEDs initialized: " + String(extPixels.numPixels()) + " LEDs");
}

// Forward declarations
static void updateExtBreathingInternal();
void startLedTask();
void stopLedTask();

// LED task function - runs at 30 FPS
void ledTask(void *parameter) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (ledTaskRunning) {
        // Check if OTA update is in progress - if so, pause LED updates
        if (otaUpdateInProgress) {
            vTaskDelay(pdMS_TO_TICKS(100)); // Sleep longer during OTA
            continue;
        }
        
        // Wait for next 30 FPS tick
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(LED_UPDATE_INTERVAL_MS));
        
        if (xSemaphoreTake(ledMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            // Update breathing effect
            updateExtBreathingInternal();
            xSemaphoreGive(ledMutex);
        }
    }
    // Task cleanup
    vTaskDelete(NULL);
}

void extLedSetAll(uint8_t r, uint8_t g, uint8_t b) {
    if (lowPowerModeEnabled || otaUpdateInProgress) return; // Skip if low power mode or OTA update
    if (ledMutex == NULL) return; // Mutex not initialized
    
    if (xSemaphoreTake(ledMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        for (uint16_t i = 0; i < extPixels.numPixels(); i++) {
            extPixels.setPixelColor(i, extPixels.Color(r, g, b));
        }
        extPixels.show();
        xSemaphoreGive(ledMutex);
    }
}

void extLedSetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b) {
    if (lowPowerModeEnabled || otaUpdateInProgress) return; // Skip if low power mode or OTA update
    if (idx >= extPixels.numPixels()) return;
    if (ledMutex == NULL) return; // Mutex not initialized
    
    if (xSemaphoreTake(ledMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        extPixels.setPixelColor(idx, extPixels.Color(r, g, b));
        extPixels.show();
        xSemaphoreGive(ledMutex);
    }
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
    if (lowPowerModeEnabled || otaUpdateInProgress) return; // Skip if low power mode or OTA update
    if (ledMutex == NULL) return; // Mutex not initialized
    
    if (xSemaphoreTake(ledMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        if (debugWhite) {
            // Set white directly without calling extLedSetAll (which would deadlock)
            for (uint16_t i = 0; i < extPixels.numPixels(); i++) {
                extPixels.setPixelColor(i, extPixels.Color(255, 255, 255));
            }
            extPixels.show();
        } else {
            setBaseColorByPM25(pm25);
            // Immediately apply current base color (breathing will modulate later)
            for (uint16_t i = 0; i < extPixels.numPixels(); i++) {
                extPixels.setPixelColor(i, extPixels.Color(baseR, baseG, baseB));
            }
            extPixels.show();
        }
        xSemaphoreGive(ledMutex);
    }
}

// Internal breathing update function (called from task)
static void updateExtBreathingInternal() {
    if (lowPowerModeEnabled || otaUpdateInProgress) return; // Skip if low power mode or OTA update
    
    // Add small delay to reduce RMT conflicts
    delayMicroseconds(100);
    
    if (debugWhite) {
        // Keep solid white in debug mode - direct call to avoid nested mutex
        for (uint16_t i = 0; i < extPixels.numPixels(); i++) {
            extPixels.setPixelColor(i, extPixels.Color(255, 255, 255));
        }
        extPixels.show();
        return;
    }
    
    // Check if we have valid colors
    if (baseR == 0 && baseG == 0 && baseB == 0) {
        // Set default color if none set
        baseR = 0;
        baseG = 255;
        baseB = 0;
    }
    
    // Compute modulation 0.4..1.0 using triangle wave from breathPhase
    // breathPhase advances based on period
    static unsigned long lastPhaseTick = 0;
    unsigned long now = millis();
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
    
    // Apply color and brightness to ALL LEDs
    extPixels.setBrightness(br);
    for (uint16_t i = 0; i < extPixels.numPixels(); i++) {
        extPixels.setPixelColor(i, extPixels.Color(baseR, baseG, baseB));
    }
    extPixels.show();
}

// Public breathing update function (thread-safe wrapper)
void updateExtBreathing() {
    if (ledMutex == NULL || otaUpdateInProgress) return; // Mutex not initialized or OTA update
    
    if (xSemaphoreTake(ledMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        updateExtBreathingInternal();
        xSemaphoreGive(ledMutex);
    }
}

void ledSetDebugWhite(bool enabled) {
    if (lowPowerModeEnabled || otaUpdateInProgress) return; // Skip if low power mode or OTA update
    if (ledMutex == NULL) return; // Mutex not initialized
    
    if (xSemaphoreTake(ledMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        debugWhite = enabled;
        if (enabled) {
            extPixels.setBrightness(255);
            // Set white directly without calling extLedSetAll (which would deadlock)
            for (uint16_t i = 0; i < extPixels.numPixels(); i++) {
                extPixels.setPixelColor(i, extPixels.Color(255, 255, 255));
            }
            extPixels.show();
        } else {
            // Re-apply current base color
            extPixels.setBrightness(baseBrightness);
            // Set color directly without calling extLedSetAll (which would deadlock)
            for (uint16_t i = 0; i < extPixels.numPixels(); i++) {
                extPixels.setPixelColor(i, extPixels.Color(baseR, baseG, baseB));
            }
            extPixels.show();
        }
        xSemaphoreGive(ledMutex);
    }
}

// Task management functions
void startLedTask() {
    if (ledTaskRunning) return;
    
    // Don't start LED task during OTA update
    if (otaUpdateInProgress) {
        safePrintln("LED: Skipping LED task start - OTA update in progress");
        return;
    }
    
    // Ensure mutex exists
    if (ledMutex == NULL) {
        ledMutex = xSemaphoreCreateMutex();
        if (ledMutex == NULL) {
            // Failed to create mutex
            safePrintln("ERROR: Failed to create LED mutex");
            return;
        }
    }
    
    ledTaskRunning = true;
    
    // Create LED task with 8192 bytes stack and priority 2
    BaseType_t result = xTaskCreate(
        ledTask,                    // Task function
        "LED_Task",                 // Task name
        LED_TASK_STACK_SIZE,        // Stack size (8192 bytes)
        NULL,                       // Task parameters
        LED_TASK_PRIORITY,          // Priority
        &ledTaskHandle              // Task handle
    );
    
    if (result != pdPASS) {
        ledTaskRunning = false;
        safePrintln("ERROR: Failed to create LED task");
    } else {
        safePrintln("LED task started successfully");
    }
}

void stopLedTask() {
    if (!ledTaskRunning) return;
    
    ledTaskRunning = false;
    
    // Wait for task to finish
    if (ledTaskHandle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(100)); // Give task time to finish
        ledTaskHandle = NULL;
    }
    
    // If stopped due to OTA update, log it
    if (otaUpdateInProgress) {
        safePrintln("LED: Task stopped for OTA update");
    }
}

bool isLedTaskRunning() {
    bool status = ledTaskRunning && (ledTaskHandle != NULL);
    if (ledTaskRunning != status) {
        // Task state mismatch - fix it
        ledTaskRunning = status;
    }
    return status;
}

// OTA update control functions
void setOtaUpdateStatus(bool inProgress) {
    otaUpdateInProgress = inProgress;
    
    if (inProgress) {
        // OTA update started - stop LED task to free up resources
        if (ledTaskRunning && !lowPowerModeEnabled) {
            safePrintln("LED: OTA update detected - pausing LED task");
            stopLedTask();
        }
    } else {
        // OTA update finished - restart LED task if needed
        if (!ledTaskRunning && !lowPowerModeEnabled) {
            safePrintln("LED: OTA update finished - restarting LED task");
            startLedTask();
        }
    }
}

bool isOtaUpdateInProgress() {
    return otaUpdateInProgress;
}

void ledSetLowPowerMode(bool enabled) {
    if (ledMutex == NULL) return; // Mutex not initialized
    
    if (xSemaphoreTake(ledMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        lowPowerModeEnabled = enabled;
        if (enabled) {
            // Turn off all LEDs and set brightness to 0
            extPixels.setBrightness(0);
            extPixels.show(); // Show the change
            // Stop LED task for smooth animations
            stopLedTask();
        } else {
            // Restore previous brightness and re-apply base color
            extPixels.setBrightness(baseBrightness);
            if (!debugWhite) {
                extLedSetAll(baseR, baseG, baseB);
            }
            // Start LED task for smooth animations (only if not in OTA update)
            if (!otaUpdateInProgress) {
                startLedTask();
                
                // Verify task started successfully
                if (!isLedTaskRunning()) {
                    safePrintln("WARNING: LED task failed to start after disabling low power mode");
                }
            } else {
                safePrintln("LED: Skipping LED task start - OTA update in progress");
            }
        }
        xSemaphoreGive(ledMutex);
    }
}
