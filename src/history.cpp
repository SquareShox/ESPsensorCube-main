#include <history.h>
#include <sensors.h>
#include <mean.h>
#include <calib.h>
#include <fan.h>
#include <ArduinoJson.h>
#include <time.h>
#include <cstring>
#include <new> // For std::nothrow
#include <esp_heap_caps.h> // For PSRAM allocation

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

// PSRAM allocation helper functions
template<typename T>
T* allocatePSRAM(size_t count = 1) {
    if (ESP.getPsramSize() == 0) {
        safePrintln("PSRAM not available, falling back to heap");
        return static_cast<T*>(malloc(sizeof(T) * count));
    }
    
    void* ptr = heap_caps_malloc(sizeof(T) * count, MALLOC_CAP_SPIRAM);
    if (ptr) {
        safePrintln("✓ Allocated " + String(sizeof(T) * count) + " bytes in PSRAM");
        return static_cast<T*>(ptr);
    } else {
        safePrintln("PSRAM allocation failed, falling back to heap");
        return static_cast<T*>(malloc(sizeof(T) * count));
    }
}

template<typename T>
void freePSRAM(T* ptr) {
    if (ptr) {
        heap_caps_free(ptr);
    }
}

// Globalny manager historii
HistoryManager historyManager;

// External sensor data
extern SolarData solarData;
extern I2CSensorData i2cSensorData;
extern SPS30Data sps30Data;
extern IPSSensorData ipsSensorData;
extern MCP3424Data mcp3424Data;
extern ADS1110Data ads1110Data;
extern INA219Data ina219Data;
extern SHT40Data sht40Data;
extern CalibratedSensorData calibratedData;
extern HCHOData hchoData;

// External configuration
extern FeatureConfig config;
extern CalibrationConfig calibConfig;

// Helper function to get formatted date and time
void getFormattedDateTime(char* buffer, size_t bufferSize) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        strncpy(buffer, "1970-01-01 00:00:00", bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
        return;
    }
    
    strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", &timeinfo);
}

// External averaging functions
extern SolarData getSolarFastAverage();
extern SolarData getSolarSlowAverage();
extern I2CSensorData getI2CFastAverage();
extern I2CSensorData getI2CSlowAverage();
extern SPS30Data getSPS30FastAverage();
extern SPS30Data getSPS30SlowAverage();
extern IPSSensorData getIPSFastAverage();
extern IPSSensorData getIPSSlowAverage();
extern MCP3424Data getMCP3424FastAverage();
extern MCP3424Data getMCP3424SlowAverage();
extern ADS1110Data getADS1110FastAverage();
extern ADS1110Data getADS1110SlowAverage();
extern INA219Data getINA219FastAverage();
extern INA219Data getINA219SlowAverage();
extern SHT40Data getSHT40FastAverage();
extern SHT40Data getSHT40SlowAverage();
extern CalibratedSensorData getCalibratedFastAverage();
extern CalibratedSensorData getCalibratedSlowAverage();
extern HCHOData getHCHOFastAverage();
extern HCHOData getHCHOSlowAverage();

// HistoryManager implementation
HistoryManager::HistoryManager() {
    // Initialize all pointers to nullptr
    solarHistory = nullptr;
    i2cHistory = nullptr;
    sps30History = nullptr;
    ipsHistory = nullptr;
    mcp3424History = nullptr;
    ads1110History = nullptr;
    ina219History = nullptr;
    sht40History = nullptr;
    calibHistory = nullptr;
    hchoHistory = nullptr;
    fanHistory = nullptr;
    batteryHistory = nullptr;
}

HistoryManager::~HistoryManager() {
    // Cleanup all allocated histories
    delete solarHistory;
    delete i2cHistory;
    delete sps30History;
    delete ipsHistory;
    delete mcp3424History;
    delete ads1110History;
    delete ina219History;
    delete sht40History;
    delete calibHistory;
    delete hchoHistory;
    delete fanHistory;
}

bool HistoryManager::initialize() {
    safePrintln("=== Initializing Sensor History (1MB PSRAM) ===");
    
    // Check available memory before initialization
    size_t psramSize = ESP.getPsramSize();
    size_t freePsram = ESP.getFreePsram();
    size_t freeHeap = ESP.getFreeHeap();
    if (psramSize == 0) {
        safePrintln("ERROR: PSRAM not detected, history initialization aborted");
        return false;
    }
    
    safePrint("PSRAM size: ");
    safePrint(String(psramSize));
    safePrint(", free: ");
    safePrintln(String(freePsram));
    safePrint("Heap free: ");
    safePrintln(String(freeHeap));
    
    if (freeHeap < 50000) {
        safePrintln("ERROR: Insufficient heap memory for history initialization");
        return false;
    }
    
    // Wyświetl obliczenia pamięci
    safePrint("Estimated total memory usage: ");
    safePrint(String(TOTAL_MEMORY_ESTIMATE));
    safePrint(" bytes (");
    safePrint(String(TOTAL_MEMORY_ESTIMATE / 1024));
    safePrintln(" KB)");
    
    if (TOTAL_MEMORY_ESTIMATE > TARGET_MEMORY_BYTES) {
        safePrintln("WARNING: Estimated memory usage exceeds 1MB target!");
    }
    
    totalMemoryUsed = 0;
    bool allSuccess = true;
    
    // Initialize history buffers based on enabled sensors
    if (config.enableSolarSensor) {
        solarHistory = new(std::nothrow) SensorHistory<SolarData, SOLAR_FAST_HISTORY, SOLAR_SLOW_HISTORY>();
        if (solarHistory && solarHistory->initialize()) {
            safePrint("Solar history initialized: ");
            safePrint(String(SOLAR_FAST_HISTORY));
            safePrint(" fast + ");
            safePrint(String(SOLAR_SLOW_HISTORY));
            safePrint(" slow samples (");
            safePrint(String((SOLAR_FAST_HISTORY + SOLAR_SLOW_HISTORY) * SOLAR_ENTRY_SIZE));
            safePrintln(" bytes)");
            totalMemoryUsed += (SOLAR_FAST_HISTORY + SOLAR_SLOW_HISTORY) * SOLAR_ENTRY_SIZE;
        } else {
            safePrintln("Failed to initialize Solar history - memory allocation failed");
            if (solarHistory) delete solarHistory;
            solarHistory = nullptr;
            allSuccess = false;
        }
    }
    
    if (config.enableI2CSensors) {
        i2cHistory = new(std::nothrow) SensorHistory<I2CSensorData, I2C_FAST_HISTORY, I2C_SLOW_HISTORY>();
        if (i2cHistory && i2cHistory->initialize()) {
            safePrint("I2C history initialized: ");
            safePrint(String(I2C_FAST_HISTORY));
            safePrint(" fast + ");
            safePrint(String(I2C_SLOW_HISTORY));
            safePrint(" slow samples (");
            safePrint(String((I2C_FAST_HISTORY + I2C_SLOW_HISTORY) * I2C_ENTRY_SIZE));
            safePrintln(" bytes)");
            totalMemoryUsed += (I2C_FAST_HISTORY + I2C_SLOW_HISTORY) * I2C_ENTRY_SIZE;
        } else {
            safePrintln("Failed to initialize I2C history - memory allocation failed");
            if (i2cHistory) delete i2cHistory;
            i2cHistory = nullptr;
            allSuccess = false;
        }
    }
    
    if (config.enableSPS30) {
        sps30History = new(std::nothrow) SensorHistory<SPS30Data, SPS30_FAST_HISTORY, SPS30_SLOW_HISTORY>();
        if (sps30History && sps30History->initialize()) {
            safePrint("SPS30 history initialized: ");
            safePrint(String(SPS30_FAST_HISTORY));
            safePrint(" fast + ");
            safePrint(String(SPS30_SLOW_HISTORY));
            safePrint(" slow samples (");
            safePrint(String((SPS30_FAST_HISTORY + SPS30_SLOW_HISTORY) * SPS30_ENTRY_SIZE));
            safePrintln(" bytes)");
            totalMemoryUsed += (SPS30_FAST_HISTORY + SPS30_SLOW_HISTORY) * SPS30_ENTRY_SIZE;
        } else {
            safePrintln("Failed to initialize SPS30 history - memory allocation failed");
            if (sps30History) delete sps30History;
            sps30History = nullptr;
            allSuccess = false;
        }
    }
    
    if (config.enableIPS) {
        ipsHistory = new(std::nothrow) SensorHistory<IPSSensorData, IPS_FAST_HISTORY, IPS_SLOW_HISTORY>();
        if (ipsHistory && ipsHistory->initialize()) {
            safePrint("IPS history initialized: ");
            safePrint(String(IPS_FAST_HISTORY));
            safePrint(" fast + ");
            safePrint(String(IPS_SLOW_HISTORY));
            safePrint(" slow samples (");
            safePrint(String((IPS_FAST_HISTORY + IPS_SLOW_HISTORY) * IPS_ENTRY_SIZE));
            safePrintln(" bytes)");
            totalMemoryUsed += (IPS_FAST_HISTORY + IPS_SLOW_HISTORY) * IPS_ENTRY_SIZE;
        } else {
            safePrintln("Failed to initialize IPS history - memory allocation failed");
            if (ipsHistory) delete ipsHistory;
            ipsHistory = nullptr;
            allSuccess = false;
        }
    }
    
    if (config.enableMCP3424) {
        mcp3424History = new(std::nothrow) SensorHistory<MCP3424Data, MCP3424_FAST_HISTORY, MCP3424_SLOW_HISTORY>();
        if (mcp3424History && mcp3424History->initialize()) {
            safePrint("MCP3424 history initialized: ");
            safePrint(String(MCP3424_FAST_HISTORY));
            safePrint(" fast + ");
            safePrint(String(MCP3424_SLOW_HISTORY));
            safePrint(" slow samples (");
            safePrint(String((MCP3424_FAST_HISTORY + MCP3424_SLOW_HISTORY) * MCP3424_ENTRY_SIZE));
            safePrintln(" bytes)");
            totalMemoryUsed += (MCP3424_FAST_HISTORY + MCP3424_SLOW_HISTORY) * MCP3424_ENTRY_SIZE;
        } else {
            safePrintln("Failed to initialize MCP3424 history - memory allocation failed");
            if (mcp3424History) delete mcp3424History;
            mcp3424History = nullptr;
            allSuccess = false;
        }
    }
    
    if (config.enableADS1110) {
        ads1110History = new(std::nothrow) SensorHistory<ADS1110Data, ADS1110_FAST_HISTORY, ADS1110_SLOW_HISTORY>();
        if (ads1110History && ads1110History->initialize()) {
            safePrint("ADS1110 history initialized: ");
            safePrint(String(ADS1110_FAST_HISTORY));
            safePrint(" fast + ");
            safePrint(String(ADS1110_SLOW_HISTORY));
            safePrint(" slow samples (");
            safePrint(String((ADS1110_FAST_HISTORY + ADS1110_SLOW_HISTORY) * ADS1110_ENTRY_SIZE));
            safePrintln(" bytes)");
            totalMemoryUsed += (ADS1110_FAST_HISTORY + ADS1110_SLOW_HISTORY) * ADS1110_ENTRY_SIZE;
        } else {
            safePrintln("Failed to initialize ADS1110 history - memory allocation failed");
            if (ads1110History) delete ads1110History;
            ads1110History = nullptr;
            allSuccess = false;
        }
    }
    
    if (config.enableINA219) {
        ina219History = new(std::nothrow) SensorHistory<INA219Data, INA219_FAST_HISTORY, INA219_SLOW_HISTORY>();
        if (ina219History && ina219History->initialize()) {
            safePrint("INA219 history initialized: ");
            safePrint(String(INA219_FAST_HISTORY));
            safePrint(" fast + ");
            safePrint(String(INA219_SLOW_HISTORY));
            safePrint(" slow samples (");
            safePrint(String((INA219_FAST_HISTORY + INA219_SLOW_HISTORY) * INA219_ENTRY_SIZE));
            safePrintln(" bytes)");
            totalMemoryUsed += (INA219_FAST_HISTORY + INA219_SLOW_HISTORY) * INA219_ENTRY_SIZE;
        } else {
            safePrintln("Failed to initialize INA219 history - memory allocation failed");
            if (ina219History) delete ina219History;
            ina219History = nullptr;
            allSuccess = false;
        }
    }
    
    if (config.enableSHT40) {
        sht40History = new(std::nothrow) SensorHistory<SHT40Data, SHT40_FAST_HISTORY, SHT40_SLOW_HISTORY>();
        if (sht40History && sht40History->initialize()) {
            safePrint("SHT40 history initialized: ");
            safePrint(String(SHT40_FAST_HISTORY));
            safePrint(" fast + ");
            safePrint(String(SHT40_SLOW_HISTORY));
            safePrint(" slow samples (");
            safePrint(String((SHT40_FAST_HISTORY + SHT40_SLOW_HISTORY) * SHT40_ENTRY_SIZE));
            safePrintln(" bytes)");
            totalMemoryUsed += (SHT40_FAST_HISTORY + SHT40_SLOW_HISTORY) * SHT40_ENTRY_SIZE;
        } else {
            safePrintln("Failed to initialize SHT40 history - memory allocation failed");
            if (sht40History) delete sht40History;
            sht40History = nullptr;
            allSuccess = false;
        }
    }
    
    if (calibConfig.enableCalibration) {
        calibHistory = new(std::nothrow) SensorHistory<CalibratedSensorData, CALIB_FAST_HISTORY, CALIB_SLOW_HISTORY>();
        if (calibHistory && calibHistory->initialize()) {
            safePrint("Calibration history initialized: ");
            safePrint(String(CALIB_FAST_HISTORY));
            safePrint(" fast + ");
            safePrint(String(CALIB_SLOW_HISTORY));
            safePrint(" slow samples (");
            safePrint(String((CALIB_FAST_HISTORY + CALIB_SLOW_HISTORY) * CALIB_ENTRY_SIZE));
            safePrintln(" bytes)");
            totalMemoryUsed += (CALIB_FAST_HISTORY + CALIB_SLOW_HISTORY) * CALIB_ENTRY_SIZE;
        } else {
            safePrintln("Failed to initialize Calibration history - memory allocation failed");
            if (calibHistory) delete calibHistory;
            calibHistory = nullptr;
            allSuccess = false;
        }
    }
    
    if (config.enableHCHO) {
        hchoHistory = new(std::nothrow) SensorHistory<HCHOData, HCHO_FAST_HISTORY, HCHO_SLOW_HISTORY>();
        if (hchoHistory && hchoHistory->initialize()) {
            safePrint("HCHO history initialized: ");
            safePrint(String(HCHO_FAST_HISTORY));
            safePrint(" fast + ");
            safePrint(String(HCHO_SLOW_HISTORY));
            safePrint(" slow samples (");
            safePrint(String((HCHO_FAST_HISTORY + HCHO_SLOW_HISTORY) * HCHO_ENTRY_SIZE));
            safePrintln(" bytes)");
            totalMemoryUsed += (HCHO_FAST_HISTORY + HCHO_SLOW_HISTORY) * HCHO_ENTRY_SIZE;
        } else {
            safePrintln("Failed to initialize HCHO history - memory allocation failed");
            if (hchoHistory) delete hchoHistory;
            hchoHistory = nullptr;
            allSuccess = false;
        }
    }
    
    // Battery history - always enabled if INA219 is enabled
    if (config.enableINA219) {
        batteryHistory = new(std::nothrow) SensorHistory<BatteryData, BATTERY_FAST_HISTORY, BATTERY_SLOW_HISTORY>();
        if (batteryHistory && batteryHistory->initialize()) {
            safePrint("Battery history initialized: ");
            safePrint(String(BATTERY_FAST_HISTORY));
            safePrint(" fast + ");
            safePrint(String(BATTERY_SLOW_HISTORY));
            safePrint(" slow samples (");
            safePrint(String((BATTERY_FAST_HISTORY + BATTERY_SLOW_HISTORY) * BATTERY_ENTRY_SIZE));
            safePrintln(" bytes)");
            totalMemoryUsed += (BATTERY_FAST_HISTORY + BATTERY_SLOW_HISTORY) * BATTERY_ENTRY_SIZE;
        } else {
            safePrintln("Failed to initialize Battery history - memory allocation failed");
            if (batteryHistory) delete batteryHistory;
            batteryHistory = nullptr;
            allSuccess = false;
        }
    }
    
    // Fan history - only if enabled
    if (config.enableFan) {
        fanHistory = new(std::nothrow) SensorHistory<FanData, FAN_FAST_HISTORY, FAN_SLOW_HISTORY>();
        if (fanHistory && fanHistory->initialize()) {
            safePrint("Fan history initialized: ");
            safePrint(String(FAN_FAST_HISTORY));
            safePrint(" fast + ");
            safePrint(String(FAN_SLOW_HISTORY));
            safePrint(" slow samples (");
            safePrint(String((FAN_FAST_HISTORY + FAN_SLOW_HISTORY) * FAN_ENTRY_SIZE));
            safePrintln(" bytes)");
            totalMemoryUsed += (FAN_FAST_HISTORY + FAN_SLOW_HISTORY) * FAN_ENTRY_SIZE;
        } else {
            safePrintln("Failed to initialize Fan history - memory allocation failed");
            if (fanHistory) delete fanHistory;
            fanHistory = nullptr;
            allSuccess = false;
        }
    }
    
    initialized = allSuccess;
    
    safePrint("Total actual memory used: ");
    safePrint(String(totalMemoryUsed));
    safePrint(" bytes (");
    safePrint(String(totalMemoryUsed / 1024));
    safePrint(" KB, ");
    safePrint(String((totalMemoryUsed * 100) / TARGET_MEMORY_BYTES));
    safePrintln("% of 1MB target)");
    
    if (initialized) {
        safePrintln("History initialization completed successfully");
    } else {
        safePrintln("History initialization failed for some sensors");
    }
    
    return initialized;
}

void HistoryManager::updateHistory() {
    if (!initialized) return;
    
    // Użyj epoch time w sekundach dla spójności z WebSocket
    unsigned long currentTime = 0;
    if (time(nullptr) > 8 * 3600 * 2) { // Jeśli czas jest zsynchronizowany
        currentTime = time(nullptr); // Używaj sekund (epoch timestamp)
    } else {
        currentTime = millis() / 1000; // Fallback do sekund od uruchomienia
    }
    
    static unsigned long lastDebugTime = 0;
    if (currentTime - lastDebugTime >= 60) { // Debug co minutę
        lastDebugTime = currentTime;
        safePrintln("History timestamp debug: currentTime=" + String(currentTime) + " epoch=" + String(time(nullptr)));
    }
    
    static unsigned long lastFastUpdate = 0;
    static unsigned long lastSlowUpdate = 0;
    
    // Update fast averages every 10 seconds
    if (currentTime - lastFastUpdate >= 10) {
        lastFastUpdate = currentTime;
        
        if (solarHistory && config.enableSolarSensor) {
            SolarData fastAvg = getSolarFastAverage();
            if (fastAvg.valid) {
                solarHistory->addFastSample(fastAvg, currentTime);
            }
        }
        
        if (i2cHistory && config.enableI2CSensors) {
            I2CSensorData fastAvg = getI2CFastAverage();
            if (fastAvg.valid) {
                i2cHistory->addFastSample(fastAvg, currentTime);
               // safePrintln("History: Added I2C fast sample");
            } else {
               // safePrintln("History: I2C data not valid");
            }
        }
        
        if (config.enableSPS30) {
            if (sps30History) {
            SPS30Data fastAvg = getSPS30FastAverage();
                // safePrintln("History: SPS30 fast check - valid=" + String(fastAvg.valid) + " pm25=" + String(fastAvg.pm2_5));
            if (fastAvg.valid) {
                sps30History->addFastSample(fastAvg, currentTime);
                    // safePrintln("History: Added SPS30 fast sample");
                } else {
                    // safePrintln("History: SPS30 data not valid for fast sample");
            }
            } else {
                safePrintln("History: SPS30 history not initialized");
            }
        } else {
            safePrintln("History: SPS30 disabled in config");
        }
        
        if (ipsHistory && config.enableIPS) {
            IPSSensorData fastAvg = getIPSFastAverage();
            if (fastAvg.valid) {
                ipsHistory->addFastSample(fastAvg, currentTime);
            }
        }
        
        if (mcp3424History && config.enableMCP3424) {
            MCP3424Data fastAvg = getMCP3424FastAverage();
            if (fastAvg.deviceCount > 0) {
                mcp3424History->addFastSample(fastAvg, currentTime);
            }
        }
        
        if (ads1110History && config.enableADS1110) {
            ADS1110Data fastAvg = getADS1110FastAverage();
            if (fastAvg.valid) {
                ads1110History->addFastSample(fastAvg, currentTime);
            }
        }
        
        if (ina219History && config.enableINA219) {
            INA219Data fastAvg = getINA219FastAverage();
            if (fastAvg.valid) {
                ina219History->addFastSample(fastAvg, currentTime);
            }
        }
        
        if (sht40History && config.enableSHT40) {
            SHT40Data fastAvg = getSHT40FastAverage();
            if (fastAvg.valid) {
                sht40History->addFastSample(fastAvg, currentTime);
            }
        }
        
        if (calibHistory && calibConfig.enableCalibration) {
            CalibratedSensorData fastAvg = getCalibratedFastAverage();
            if (fastAvg.valid) {
                calibHistory->addFastSample(fastAvg, currentTime);
            }
        }
        
        if (hchoHistory && config.enableHCHO) {
            HCHOData fastAvg = getHCHOFastAverage();
            if (fastAvg.valid) {
                hchoHistory->addFastSample(fastAvg, currentTime);
            }
        }
        
        // Battery data - always if INA219 enabled
        if (batteryHistory && config.enableINA219) {
            extern BatteryData batteryData;
            if (batteryData.valid) {
                batteryHistory->addFastSample(batteryData, currentTime);
            }
        }
        
        // Fan data - only if enabled
        if (fanHistory && config.enableFan) {
            FanData fanData;
            fanData.dutyCycle = getFanDutyCycle();
            fanData.rpm = getFanRPM();
            fanData.enabled = isFanEnabled();
            fanData.glineEnabled = isGLineEnabled();
            fanData.valid = true;
            fanData.lastUpdate = currentTime;
            fanHistory->addFastSample(fanData, currentTime);
        }
    }
    
    // Update slow averages every 5 minutes (300 seconds)
    if (currentTime - lastSlowUpdate >= 300) {
        lastSlowUpdate = currentTime;
        
        if (solarHistory && config.enableSolarSensor) {
            SolarData slowAvg = getSolarSlowAverage();
            if (slowAvg.valid) {
                solarHistory->addSlowSample(slowAvg, currentTime);
            }
        }
        
        if (i2cHistory && config.enableI2CSensors) {
            I2CSensorData slowAvg = getI2CSlowAverage();
            if (slowAvg.valid) {
                i2cHistory->addSlowSample(slowAvg, currentTime);
            }
        }
        
        if (sps30History && config.enableSPS30) {
            SPS30Data slowAvg = getSPS30SlowAverage();
            if (slowAvg.valid) {
                sps30History->addSlowSample(slowAvg, currentTime);
                // safePrintln("History: Added SPS30 slow sample");
            } else {
                // safePrintln("History: SPS30 data not valid for slow sample");
            }
        }
        
        if (ipsHistory && config.enableIPS) {
            IPSSensorData slowAvg = getIPSSlowAverage();
            if (slowAvg.valid) {
                ipsHistory->addSlowSample(slowAvg, currentTime);
            }
        }
        
        if (mcp3424History && config.enableMCP3424) {
            MCP3424Data slowAvg = getMCP3424SlowAverage();
            if (slowAvg.deviceCount > 0) {
                mcp3424History->addSlowSample(slowAvg, currentTime);
            }
        }
        
        if (ads1110History && config.enableADS1110) {
            ADS1110Data slowAvg = getADS1110SlowAverage();
            if (slowAvg.valid) {
                ads1110History->addSlowSample(slowAvg, currentTime);
            }
        }
        
        if (ina219History && config.enableINA219) {
            INA219Data slowAvg = getINA219SlowAverage();
            if (slowAvg.valid) {
                ina219History->addSlowSample(slowAvg, currentTime);
            }
        }
        
        if (sht40History && config.enableSHT40) {
            SHT40Data slowAvg = getSHT40SlowAverage();
            if (slowAvg.valid) {
                sht40History->addSlowSample(slowAvg, currentTime);
            }
        }
        
        if (calibHistory && calibConfig.enableCalibration) {
            CalibratedSensorData slowAvg = getCalibratedSlowAverage();
            if (slowAvg.valid) {
                calibHistory->addSlowSample(slowAvg, currentTime);
            }
        }
        
        if (hchoHistory && config.enableHCHO) {
            HCHOData slowAvg = getHCHOSlowAverage();
            if (slowAvg.valid) {
                hchoHistory->addSlowSample(slowAvg, currentTime);
            }
        }
        
        // Battery slow samples - always if INA219 enabled
        if (batteryHistory && config.enableINA219) {
            extern BatteryData batteryData;
            if (batteryData.valid) {
                batteryHistory->addSlowSample(batteryData, currentTime);
            }
        }
        
        // Fan slow samples - only if enabled
        if (fanHistory && config.enableFan) {
            FanData fanData;
            fanData.dutyCycle = getFanDutyCycle();
            fanData.rpm = getFanRPM();
            fanData.enabled = isFanEnabled();
            fanData.glineEnabled = isGLineEnabled();
            fanData.valid = true;
            fanData.lastUpdate = currentTime;
            fanHistory->addSlowSample(fanData, currentTime);
        }
    }
}

void HistoryManager::printMemoryUsage() const {
    safePrintln("=== History Memory Usage ===");
    safePrint("Total allocated: ");
    safePrint(String(totalMemoryUsed));
    safePrint(" bytes (");
    safePrint(String(totalMemoryUsed / 1024));
    safePrintln(" KB)");
    
    safePrint("Target budget: ");
    safePrint(String(TARGET_MEMORY_BYTES / 1024));
    safePrintln(" KB (1MB)");
    
    safePrint("Usage: ");
    safePrint(String((totalMemoryUsed * 100) / TARGET_MEMORY_BYTES));
    safePrintln("% of budget");
    
    // Sprawdź dostępność PSRAM i heap
    safePrint("Heap free: ");
    safePrint(String(ESP.getFreeHeap() / 1024));
    safePrintln(" KB");
    
    if (ESP.getPsramSize() > 0) {
        safePrint("PSRAM total: ");
        safePrint(String(ESP.getPsramSize() / 1024));
        safePrintln(" KB");
        safePrint("PSRAM free: ");
        safePrint(String(ESP.getFreePsram() / 1024));
        safePrintln(" KB");
        safePrint("PSRAM used: ");
        safePrint(String((ESP.getPsramSize() - ESP.getFreePsram()) / 1024));
        safePrintln(" KB");
        
        // Sprawdz czy historia rzeczywiście używa PSRAM
        size_t psramUsedByHistory = ESP.getPsramSize() - ESP.getFreePsram();
        if (psramUsedByHistory >= totalMemoryUsed / 2) {
            safePrintln("STATUS: Historia prawdopodobnie używa PSRAM ✓");
        } else {
            safePrintln("WARNING: Historia może używać heap zamiast PSRAM!");
        }
    } else {
        safePrintln("PSRAM not available - using heap memory");
        safePrintln("WARNING: Cala historia w heap - mozliwe problemy z pamiecia!");
    }
    
    // Dodatkowe informacje o alokacji
    safePrint("Heap largest free block: ");
    safePrint(String(ESP.getMaxAllocHeap() / 1024));
    safePrintln(" KB");
}

void HistoryManager::printHistoryStatus() const {
    safePrintln("=== History Status ===");
    
    if (solarHistory && solarHistory->isInitialized()) {
        safePrint("Solar: ");
        safePrint(String(solarHistory->getFastCount()));
        safePrint(" fast, ");
        safePrint(String(solarHistory->getSlowCount()));
        safePrintln(" slow samples");
    }
    
    if (i2cHistory && i2cHistory->isInitialized()) {
        safePrint("I2C: ");
        safePrint(String(i2cHistory->getFastCount()));
        safePrint(" fast, ");
        safePrint(String(i2cHistory->getSlowCount()));
        safePrintln(" slow samples");
    }
    
    if (sps30History && sps30History->isInitialized()) {
        safePrint("SPS30: ");
        safePrint(String(sps30History->getFastCount()));
        safePrint(" fast, ");
        safePrint(String(sps30History->getSlowCount()));
        safePrintln(" slow samples");
    }
    
    if (ipsHistory && ipsHistory->isInitialized()) {
        safePrint("IPS: ");
        safePrint(String(ipsHistory->getFastCount()));
        safePrint(" fast, ");
        safePrint(String(ipsHistory->getSlowCount()));
        safePrintln(" slow samples");
    }
    
    if (mcp3424History && mcp3424History->isInitialized()) {
        safePrint("MCP3424: ");
        safePrint(String(mcp3424History->getFastCount()));
        safePrint(" fast, ");
        safePrint(String(mcp3424History->getSlowCount()));
        safePrintln(" slow samples");
    }
    
    if (ads1110History && ads1110History->isInitialized()) {
        safePrint("ADS1110: ");
        safePrint(String(ads1110History->getFastCount()));
        safePrint(" fast, ");
        safePrint(String(ads1110History->getSlowCount()));
        safePrintln(" slow samples");
    }
    
    if (ina219History && ina219History->isInitialized()) {
        safePrint("INA219: ");
        safePrint(String(ina219History->getFastCount()));
        safePrint(" fast, ");
        safePrint(String(ina219History->getSlowCount()));
        safePrintln(" slow samples");
    }
    
    if (sht40History && sht40History->isInitialized()) {
        safePrint("SHT40: ");
        safePrint(String(sht40History->getFastCount()));
        safePrint(" fast, ");
        safePrint(String(sht40History->getSlowCount()));
        safePrintln(" slow samples");
    }
    
    if (calibHistory && calibHistory->isInitialized()) {
        safePrint("Calibration: ");
        safePrint(String(calibHistory->getFastCount()));
        safePrint(" fast, ");
        safePrint(String(calibHistory->getSlowCount()));
        safePrintln(" slow samples");
    }
    
    if (hchoHistory && hchoHistory->isInitialized()) {
        safePrint("HCHO: ");
        safePrint(String(hchoHistory->getFastCount()));
        safePrint(" fast, ");
        safePrint(String(hchoHistory->getSlowCount()));
        safePrintln(" slow samples");
    }
    
    if (fanHistory && fanHistory->isInitialized()) {
        safePrint("Fan: ");
        safePrint(String(fanHistory->getFastCount()));
        safePrint(" fast, ");
        safePrint(String(fanHistory->getSlowCount()));
        safePrintln(" slow samples");
    }
}

// Function to check memory allocation type
void checkHistoryMemoryType() {
    if (!historyManager.isInitialized()) {
        safePrintln("History not initialized");
        return;
    }
    
    safePrintln("=== Historia Memory Allocation Check ===");
    
    size_t psramBefore = ESP.getFreePsram();
    size_t heapBefore = ESP.getFreeHeap();
    
    // Alokuj mały blok w PSRAM i heap dla porównania
    void* testPsram = heap_caps_malloc(1024, MALLOC_CAP_SPIRAM);
    void* testHeap = malloc(1024);
    
    size_t psramAfter = ESP.getFreePsram();
    size_t heapAfter = ESP.getFreeHeap();
    
    if (testPsram) {
        safePrint("Test PSRAM allocation: ");
        safePrint(String(psramBefore - psramAfter));
        safePrintln(" bytes difference");
        heap_caps_free(testPsram);
    } else {
        safePrintln("PSRAM test allocation failed");
    }
    
    if (testHeap) {
        safePrint("Test Heap allocation: ");
        safePrint(String(heapBefore - heapAfter));
        safePrintln(" bytes difference");
        free(testHeap);
    }
    
    // Pokaż informacje o pamięci historii
    safePrint("Historia total size: ");
    safePrint(String(historyManager.getTotalMemoryUsed() / 1024));
    safePrintln(" KB");
    
    safePrint("PSRAM used total: ");
    safePrint(String((ESP.getPsramSize() - ESP.getFreePsram()) / 1024));
    safePrintln(" KB");
    
    if ((ESP.getPsramSize() - ESP.getFreePsram()) >= historyManager.getTotalMemoryUsed() / 2) {
        safePrintln("✓ Historia prawdopodobnie używa PSRAM");
    } else {
        safePrintln("⚠ Historia może używać heap - sprawdź logi inicjalizacji");
    }
}

// Global interface functions
void initializeHistory() {
    if (!config.enableHistory) {
        safePrintln("History system disabled in configuration");
        return;
    }
    
    safePrintln("=== Initializing History System ===");
    safePrint("PSRAM available: ");
    safePrint(String(ESP.getPsramSize() / 1024));
    safePrint(" KB, free: ");
    safePrint(String(ESP.getFreePsram() / 1024));
    safePrintln(" KB");
    
    bool success = historyManager.initialize();
    
    if (success) {
        safePrintln("Historia initialized successfully");
        // Sprawdź gdzie pamięć została alokowana
        checkHistoryMemoryType();
    } else {
        safePrintln("Historia initialization FAILED - disabling history feature");
        config.enableHistory = false;
        //safePrintln("Historia initialization FAILED");
    }
}

void updateSensorHistory() {
    if (!config.enableHistory) return;
    
    // Użyj rzeczywistego czasu zamiast millis()
    unsigned long currentTime = 0;
    if (time(nullptr) > 8 * 3600 * 2) { // Jeśli czas jest zsynchronizowany
        currentTime = time(nullptr) * 1000; // Konwertuj na milisekundy
    } else {
        currentTime = millis(); // Fallback do millis()
    }
    
    historyManager.updateHistory();
}

void printHistoryMemoryUsage() {
    historyManager.printMemoryUsage();
}

void printHistoryStatus() {
    historyManager.printHistoryStatus();
}

// API function for getting historical data with pagination
size_t getHistoricalData(const String& sensor, const String& timeRange, 
                        String& jsonResponse, unsigned long fromTime, unsigned long toTime,
                        const String& sampleType, int packetIndex, int packetSize) {
    if (!config.enableHistory) {
        safePrintln("[ERROR] getHistoricalData: History system is disabled in configuration.");
        DynamicJsonDocument doc(512);
        doc["error"] = "History disabled in configuration";
        doc["data"] = JsonArray();
        serializeJson(doc, jsonResponse);
        return 0;
    }
    
    if (!historyManager.isInitialized()) {
        safePrintln("[ERROR] getHistoricalData: History manager is not initialized.");
        DynamicJsonDocument doc(512);
        doc["error"] = "History not initialized";
        doc["data"] = JsonArray();
        serializeJson(doc, jsonResponse);
        return 0;
    }
    
    // Check available memory before starting
    size_t freeHeap = ESP.getFreeHeap();
    size_t freePsram = ESP.getFreePsram();
    size_t maxAllocHeap = ESP.getMaxAllocHeap();
    
    safePrint("[DEBUG] getHistoricalData: Free heap: ");
    safePrint(String(freeHeap / 1024));
    safePrint(" KB, Free PSRAM: ");
    safePrint(String(freePsram / 1024));
    safePrint(" KB, Max alloc: ");
    safePrint(String(maxAllocHeap / 1024));
    safePrintln(" KB");
    
    if (freeHeap < 30000) { // Need at least 30KB free
        safePrintln("[ERROR] getHistoricalData: Low heap memory. Free heap: " + String(freeHeap));
        DynamicJsonDocument doc(512);
        doc["error"] = "Low heap memory";
        doc["data"] = JsonArray();
        doc["freeHeap"] = freeHeap;
        doc["freePsram"] = freePsram;
        serializeJson(doc, jsonResponse);
        return 0;
    }
    
    // Create JSON document with appropriate size based on sensor type
    size_t docSize = 4096; // Increased base size for pagination metadata
    if (sensor == "calibration" || sensor == "all") {
        docSize = 6144; // Larger for calibration data
    } else if (sensor == "mcp3424") {
        docSize = 4096; // Medium for MCP3424 with multiple devices
    } else if (sensor == "ips") {
        docSize = 5120; // Large for IPS with many data points (pc, pm, np, pw arrays)
    } else if (sensor == "sps30") {
        docSize = 5120; // Larger for SPS30 with many data points
    } else if (sensor == "solar") {
        docSize = 4096; // Solar data can be extensive
    }
    
    // Check memory again before creating JSON document
    if (maxAllocHeap < docSize + 5000) {
        safePrintln("[ERROR] getHistoricalData: Insufficient contiguous memory for JSON. Required: " + String(docSize) + ", Available: " + String(maxAllocHeap));
        DynamicJsonDocument doc(512);
        doc["error"] = "Insufficient contiguous memory for JSON document";
        doc["requiredSize"] = docSize;
        doc["maxAllocHeap"] = maxAllocHeap;
        doc["freeHeap"] = freeHeap;
        serializeJson(doc, jsonResponse);
        return 0;
    }
    
    DynamicJsonDocument doc(docSize);
    doc["sensor"] = sensor;
    doc["timeRange"] = timeRange;
    JsonArray dataArray = doc.createNestedArray("data");
    
    // Buffer for samples with pagination (zmniejszony dla oszczędności stosu)
    const size_t MAX_TOTAL_SAMPLES = 150; // Zmniejszony buffer żeby oszczędzić stos w WebSocket task
    const size_t DEFAULT_PACKET_SIZE = 10; // Zmniejszony rozmiar pakietu dla lepszej paginacji
    
    // Użyj przekazanego packet size lub domyślnego
    size_t effectivePacketSize = (packetSize > 0 && packetSize <= 50) ? packetSize : DEFAULT_PACKET_SIZE;
    int currentPacketIndex = (packetIndex >= 0) ? packetIndex : 0;
    
    size_t totalSamples = 0;
    size_t totalAvailableSamples = 0;
    // Uzyj sampleType przekazanego jako argument funkcji, nie deklaruj lokalnie
    if (sensor == "solar") {
        auto* solarHist = historyManager.getSolarHistory();
        if (solarHist && solarHist->isInitialized()) {
            // Użyj PSRAM dla dużego bufora tymczasowego
            HistoryEntry<SolarData>* buffer = allocatePSRAM<HistoryEntry<SolarData>>(MAX_TOTAL_SAMPLES);
            if (!buffer) {
                safePrintln("[ERROR] getHistoricalData: Failed to allocate buffer for solar data");
                doc["error"] = "Memory allocation failed for solar buffer";
                serializeJson(doc, jsonResponse);
                return 0;
            }
            size_t count;
            if (sampleType == "slow") {
                count = solarHist->getSlowSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            } else {
                count = solarHist->getFastSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            }
            
            totalAvailableSamples = count;
            size_t totalPackets = (count + effectivePacketSize - 1) / effectivePacketSize;
            
            // Sprawdź czy żądany pakiet istnieje
            if (currentPacketIndex >= totalPackets) {
                currentPacketIndex = totalPackets - 1; // Użyj ostatniego pakietu
            }
            
            // Oblicz zakres dla tego pakietu (od najnowszych)
            size_t startIdx = currentPacketIndex * effectivePacketSize;
            size_t endIdx = min(startIdx + effectivePacketSize, count);
            
            // Dodaj próbki z pakietu (w odwrotnej kolejności)
            for (size_t i = startIdx; i < endIdx; i++) {
                if (ESP.getFreeHeap() < 10000) {
                    safePrintln("[ERROR] getHistoricalData: Low memory during JSON build (solar), stopping at sample " + String(i));
                    break;
                }
                size_t idx = count - 1 - i; // Odwróć kolejność (najnowsze pierwsze)
                JsonObject sample = dataArray.createNestedObject();
                sample["timestamp"] = buffer[idx].timestamp;
                sample["dateTime"] = buffer[idx].dateTime;
                JsonObject data = sample.createNestedObject("data");
                data["V"] = buffer[idx].data.V;
                data["I"] = buffer[idx].data.I;
                data["PPV"] = buffer[idx].data.PPV;
                totalSamples++;
            }
            
            // Zwolnij pamięć bufora
            freePSRAM(buffer);
        }
    } else if (sensor == "sps30") {
        auto* sps30Hist = historyManager.getSPS30History();
        if (sps30Hist && sps30Hist->isInitialized()) {
            // Użyj heap zamiast stosu dla dużego bufora
            HistoryEntry<SPS30Data>* buffer = allocatePSRAM<HistoryEntry<SPS30Data>>(MAX_TOTAL_SAMPLES);
            if (!buffer) {
                safePrintln("[ERROR] getHistoricalData: Failed to allocate buffer for sps30 data");
                doc["error"] = "Memory allocation failed for sps30 buffer";
                serializeJson(doc, jsonResponse);
                return 0;
            }
            size_t count;
            if (sampleType == "slow") {
                count = sps30Hist->getSlowSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            } else {
                count = sps30Hist->getFastSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            }
            
            totalAvailableSamples = count;
            size_t totalPackets = (count + effectivePacketSize - 1) / effectivePacketSize;
            
            if (currentPacketIndex >= totalPackets) {
                currentPacketIndex = totalPackets - 1;
            }
            
            size_t startIdx = currentPacketIndex * effectivePacketSize;
            size_t endIdx = min(startIdx + effectivePacketSize, count);
            
            for (size_t i = startIdx; i < endIdx; i++) {
                if (ESP.getFreeHeap() < 10000) {
                    safePrintln("[ERROR] getHistoricalData: Low memory during JSON build (sps30), stopping at sample " + String(i));
                    break;
                }
                size_t idx = count - 1 - i;
                JsonObject sample = dataArray.createNestedObject();
                sample["timestamp"] = buffer[idx].timestamp;
                sample["dateTime"] = buffer[idx].dateTime;
                JsonObject data = sample.createNestedObject("data");
                data["PM1"] = round(buffer[idx].data.pm1_0 * 10) / 10.0;
                data["PM25"] = round(buffer[idx].data.pm2_5 * 10) / 10.0;
                data["PM4"] = round(buffer[idx].data.pm4_0 * 10) / 10.0;
                data["PM10"] = round(buffer[idx].data.pm10 * 10) / 10.0;
                data["NC05"] = round(buffer[idx].data.nc0_5 * 10) / 10.0;
                data["NC1"] = round(buffer[idx].data.nc1_0 * 10) / 10.0;
                data["NC25"] = round(buffer[idx].data.nc2_5 * 10) / 10.0;
                data["NC4"] = round(buffer[idx].data.nc4_0 * 10) / 10.0;
                data["NC10"] = round(buffer[idx].data.nc10 * 10) / 10.0;
                data["TPS"] = round(buffer[idx].data.typical_particle_size * 10) / 10.0;
                totalSamples++;
            }
            
            // Zwolnij pamięć bufora
            freePSRAM(buffer);
        }
    } else if (sensor == "power") {
        auto* powerHist = historyManager.getINA219History();
        if (powerHist && powerHist->isInitialized()) {
            // Użyj heap zamiast stosu dla dużego bufora
            HistoryEntry<INA219Data>* buffer = allocatePSRAM<HistoryEntry<INA219Data>>(MAX_TOTAL_SAMPLES);
            if (!buffer) {
                safePrintln("[ERROR] getHistoricalData: Failed to allocate buffer for power data");
                doc["error"] = "Memory allocation failed for power buffer";
                serializeJson(doc, jsonResponse);
                return 0;
            }
            size_t count;
            if (sampleType == "slow") {
                count = powerHist->getSlowSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            } else {
                count = powerHist->getFastSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            }
            
            totalAvailableSamples = count;
            size_t totalPackets = (count + effectivePacketSize - 1) / effectivePacketSize;
            if (currentPacketIndex >= totalPackets) currentPacketIndex = totalPackets - 1;
            
            size_t startIdx = currentPacketIndex * effectivePacketSize;
            size_t endIdx = min(startIdx + effectivePacketSize, count);
            
            for (size_t i = startIdx; i < endIdx; i++) {
                if (ESP.getFreeHeap() < 10000) break;
                size_t idx = count - 1 - i;
                JsonObject sample = dataArray.createNestedObject();
                sample["timestamp"] = buffer[idx].timestamp;
                sample["dateTime"] = buffer[idx].dateTime;
                JsonObject data = sample.createNestedObject("data");
                data["busVoltage"] = round(buffer[idx].data.busVoltage * 1000) / 1000.0;
                data["current"] = round(buffer[idx].data.current * 100) / 100.0;
                data["power"] = round(buffer[idx].data.power * 100) / 100.0;
                totalSamples++;
            }
            
            // Zwolnij pamięć bufora
            freePSRAM(buffer);
        }
    } else if (sensor == "battery") {
        auto* batteryHist = historyManager.getBatteryHistory();
        if (batteryHist && batteryHist->isInitialized()) {
            // Użyj heap zamiast stosu dla dużego bufora
            HistoryEntry<BatteryData>* buffer = allocatePSRAM<HistoryEntry<BatteryData>>(MAX_TOTAL_SAMPLES);
            if (!buffer) {
                safePrintln("[ERROR] getHistoricalData: Failed to allocate buffer for battery data");
                doc["error"] = "Memory allocation failed for battery buffer";
                serializeJson(doc, jsonResponse);
                return 0;
            }
            size_t count;
            if (sampleType == "slow") {
                count = batteryHist->getSlowSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            } else {
                count = batteryHist->getFastSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            }
            
            totalAvailableSamples = count;
            size_t totalPackets = (count + effectivePacketSize - 1) / effectivePacketSize;
            if (currentPacketIndex >= totalPackets) currentPacketIndex = totalPackets - 1;
            
            size_t startIdx = currentPacketIndex * effectivePacketSize;
            size_t endIdx = min(startIdx + effectivePacketSize, count);
            
            for (size_t i = startIdx; i < endIdx; i++) {
                if (ESP.getFreeHeap() < 10000) break;
                size_t idx = count - 1 - i;
                JsonObject sample = dataArray.createNestedObject();
                sample["timestamp"] = buffer[idx].timestamp;
                sample["dateTime"] = buffer[idx].dateTime;
                JsonObject data = sample.createNestedObject("data");
                data["voltage"] = round(buffer[idx].data.voltage * 1000) / 1000.0;
                data["current"] = round(buffer[idx].data.current * 100) / 100.0;
                data["power"] = round(buffer[idx].data.power * 100) / 100.0;
                data["chargePercent"] = buffer[idx].data.chargePercent;
                data["isBatteryPowered"] = buffer[idx].data.isBatteryPowered;
                data["lowBattery"] = buffer[idx].data.lowBattery;
                data["criticalBattery"] = buffer[idx].data.criticalBattery;
                totalSamples++;
            }
            
            // Zwolnij pamięć bufora
            freePSRAM(buffer);
        }
    } else if (sensor == "sht40") {
        auto* sht40Hist = historyManager.getSHT40History();
        if (sht40Hist && sht40Hist->isInitialized()) {
            // Użyj heap zamiast stosu dla dużego bufora
            HistoryEntry<SHT40Data>* buffer = allocatePSRAM<HistoryEntry<SHT40Data>>(MAX_TOTAL_SAMPLES);
            if (!buffer) {
                safePrintln("[ERROR] getHistoricalData: Failed to allocate buffer for sht40 data");
                doc["error"] = "Memory allocation failed for sht40 buffer";
                serializeJson(doc, jsonResponse);
                return 0;
            }
            size_t count;
            if (sampleType == "slow") {
                count = sht40Hist->getSlowSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            } else {
                count = sht40Hist->getFastSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            }
            
            totalAvailableSamples = count;
            size_t totalPackets = (count + effectivePacketSize - 1) / effectivePacketSize;
            if (currentPacketIndex >= totalPackets) currentPacketIndex = totalPackets - 1;
            
            size_t startIdx = currentPacketIndex * effectivePacketSize;
            size_t endIdx = min(startIdx + effectivePacketSize, count);
            
            for (size_t i = startIdx; i < endIdx; i++) {
                if (ESP.getFreeHeap() < 10000) break;
                size_t idx = count - 1 - i;
                JsonObject sample = dataArray.createNestedObject();
                sample["timestamp"] = buffer[idx].timestamp;
                sample["dateTime"] = buffer[idx].dateTime;
                JsonObject data = sample.createNestedObject("data");
                data["temperature"] = round(buffer[idx].data.temperature * 10) / 10.0;
                data["humidity"] = round(buffer[idx].data.humidity * 10) / 10.0;
                data["pressure"] = round(buffer[idx].data.pressure * 10) / 10.0;
                totalSamples++;
            }
            
            // Zwolnij pamięć bufora
            freePSRAM(buffer);
        }
    } else if (sensor == "scd41") {
        auto* i2cHist = historyManager.getI2CHistory();
        if (i2cHist && i2cHist->isInitialized()) {
            // Użyj heap zamiast stosu dla dużego bufora
            HistoryEntry<I2CSensorData>* buffer = allocatePSRAM<HistoryEntry<I2CSensorData>>(MAX_TOTAL_SAMPLES);
            if (!buffer) {
                safePrintln("[ERROR] getHistoricalData: Failed to allocate buffer for i2c data");
                doc["error"] = "Memory allocation failed for i2c buffer";
                serializeJson(doc, jsonResponse);
                return 0;
            }
            size_t count;
            if (sampleType == "slow") {
                count = i2cHist->getSlowSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            } else {
                count = i2cHist->getFastSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            }
            
            // Policz SCD41 samples
            size_t scd41Count = 0;
            for (size_t j = 0; j < count; j++) {
                if (buffer[j].data.type == SENSOR_SCD41) scd41Count++;
            }
            
            totalAvailableSamples = scd41Count;
            size_t totalPackets = (scd41Count + effectivePacketSize - 1) / effectivePacketSize;
            if (currentPacketIndex >= totalPackets) currentPacketIndex = totalPackets - 1;
            
            size_t scd41Processed = 0;
            size_t startIdx = currentPacketIndex * effectivePacketSize;
            size_t endIdx = min(startIdx + effectivePacketSize, scd41Count);
            
            for (int i = count - 1; i >= 0 && scd41Processed < endIdx; i--) {
                if (ESP.getFreeHeap() < 10000) break;
                if (buffer[i].data.type == SENSOR_SCD41) {
                    if (scd41Processed >= startIdx) {
                    JsonObject sample = dataArray.createNestedObject();
                    sample["timestamp"] = buffer[i].timestamp;
                    sample["dateTime"] = buffer[i].dateTime;
                    JsonObject data = sample.createNestedObject("data");
                    data["co2"] = buffer[i].data.co2;
                    data["temperature"] = round(buffer[i].data.temperature * 10) / 10.0;
                    data["humidity"] = round(buffer[i].data.humidity * 10) / 10.0;
                    totalSamples++;
                }
                    scd41Processed++;
            }
            }
            
            // Zwolnij pamięć bufora
            freePSRAM(buffer);
        }
    } else if (sensor == "hcho") {
        auto* hchoHist = historyManager.getHCHOHistory();
        if (hchoHist && hchoHist->isInitialized()) {
            // Użyj heap zamiast stosu dla dużego bufora
            HistoryEntry<HCHOData>* buffer = allocatePSRAM<HistoryEntry<HCHOData>>(MAX_TOTAL_SAMPLES);
            if (!buffer) {
                safePrintln("[ERROR] getHistoricalData: Failed to allocate buffer for hcho data");
                doc["error"] = "Memory allocation failed for hcho buffer";
                serializeJson(doc, jsonResponse);
                return 0;
            }
            size_t count;
            if (sampleType == "slow") {
                count = hchoHist->getSlowSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            } else {
                count = hchoHist->getFastSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            }
            
            totalAvailableSamples = count;
            size_t totalPackets = (count + effectivePacketSize - 1) / effectivePacketSize;
            if (currentPacketIndex >= totalPackets) currentPacketIndex = totalPackets - 1;
            size_t startIdx = currentPacketIndex * effectivePacketSize;
            size_t endIdx = min(startIdx + effectivePacketSize, count);
            
            for (size_t i = startIdx; i < endIdx; i++) {
                if (ESP.getFreeHeap() < 10000) break;
                size_t idx = count - 1 - i;
                JsonObject sample = dataArray.createNestedObject();
                sample["timestamp"] = buffer[idx].timestamp;
                sample["dateTime"] = buffer[idx].dateTime;
                JsonObject data = sample.createNestedObject("data");
                data["hcho_mg"] = buffer[idx].data.hcho;
                data["hcho_ppb"] = buffer[idx].data.hcho_ppb;
                data["tvoc_mg"] = buffer[idx].data.tvoc;
                totalSamples++;
            }
            
            // Zwolnij pamięć bufora
            freePSRAM(buffer);
        }
    } else if (sensor == "mcp3424") {
        auto* mcp3424Hist = historyManager.getMCP3424History();
        if (mcp3424Hist && mcp3424Hist->isInitialized()) {
            // Użyj PSRAM dla dużego bufora tymczasowego
            HistoryEntry<MCP3424Data>* buffer = allocatePSRAM<HistoryEntry<MCP3424Data>>(MAX_TOTAL_SAMPLES);
            if (!buffer) {
                safePrintln("[ERROR] getHistoricalData: Failed to allocate buffer for mcp3424 data");
                doc["error"] = "Memory allocation failed for mcp3424 buffer";
                serializeJson(doc, jsonResponse);
                return 0;
            }
            size_t count;
            if (sampleType == "slow") {
                count = mcp3424Hist->getSlowSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            } else {
                count = mcp3424Hist->getFastSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            }
            
            totalAvailableSamples = count;
            size_t totalPackets = (count + effectivePacketSize - 1) / effectivePacketSize;
            if (currentPacketIndex >= totalPackets) currentPacketIndex = totalPackets - 1;
            
            size_t startIdx = currentPacketIndex * effectivePacketSize;
            size_t endIdx = min(startIdx + effectivePacketSize, count);
            
            for (size_t i = startIdx; i < endIdx; i++) {
                if (ESP.getFreeHeap() < 10000) break;
                size_t idx = count - 1 - i;
                JsonObject sample = dataArray.createNestedObject();
                sample["timestamp"] = buffer[idx].timestamp;
                sample["dateTime"] = buffer[idx].dateTime;
                JsonObject data = sample.createNestedObject("data");
                
                // Format jak w getAverages - device-dependent naming
                for (uint8_t dev = 0; dev < buffer[idx].data.deviceCount && dev < MAX_MCP3424_DEVICES; dev++) {
                    if (buffer[idx].data.valid[dev]) {
                        // Find actual device index in config based on I2C address
                        uint8_t i2cAddress = buffer[idx].data.addresses[dev];
                        int actualDeviceIndex = -1;
                        
                        // Search for this I2C address in MCP3424 config to get device index
                        extern MCP3424Config mcp3424Config;
                        for (int d = 0; d < 8; d++) {
                            if (mcp3424Config.devices[d].i2cAddress == i2cAddress) {
                                actualDeviceIndex = d;
                    break;
                }
                        }
                        
                        // K number = device index + 1 (Device 0->K1, Device 4->K5, Device 6->K7)
                        uint8_t kNumber = (actualDeviceIndex >= 0) ? (actualDeviceIndex + 1) : (dev + 1);
                        
                        for (uint8_t ch = 0; ch < 4; ch++) {
                            String key = "K" + String(kNumber) + "_" + String(ch+1);
                            data[key] = round(buffer[idx].data.channels[dev][ch] * 1000) / 1000.0;
                        }
                    }
                }
                data["deviceCount"] = buffer[idx].data.deviceCount;
                totalSamples++;
            }
            
            // Zwolnij pamięć bufora
            freePSRAM(buffer);
        }
    } else if (sensor == "ips") {
        auto* ipsHist = historyManager.getIPSHistory();
        if (ipsHist && ipsHist->isInitialized()) {
            // Użyj PSRAM dla dużego bufora tymczasowego
            HistoryEntry<IPSSensorData>* buffer = allocatePSRAM<HistoryEntry<IPSSensorData>>(MAX_TOTAL_SAMPLES);
            if (!buffer) {
                safePrintln("[ERROR] getHistoricalData: Failed to allocate buffer for ips data");
                doc["error"] = "Memory allocation failed for ips buffer";
                serializeJson(doc, jsonResponse);
                return 0;
            }
            size_t count;
            if (sampleType == "slow") {
                count = ipsHist->getSlowSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            } else {
                count = ipsHist->getFastSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            }
            
            totalAvailableSamples = count;
            size_t totalPackets = (count + effectivePacketSize - 1) / effectivePacketSize;
            if (currentPacketIndex >= totalPackets) currentPacketIndex = totalPackets - 1;
            
            size_t startIdx = currentPacketIndex * effectivePacketSize;
            size_t endIdx = min(startIdx + effectivePacketSize, count);
            
            for (size_t i = startIdx; i < endIdx; i++) {
                if (ESP.getFreeHeap() < 10000) break;
                size_t idx = count - 1 - i;
                JsonObject sample = dataArray.createNestedObject();
                sample["timestamp"] = buffer[idx].timestamp;
                sample["dateTime"] = buffer[idx].dateTime;
                JsonObject data = sample.createNestedObject("data");
                
                // Format jak w getAverages - pc, pm, np, pw arrays
                for (int j = 0; j < 7; j++) {
                    data["pc_" + String(j+1)] = buffer[idx].data.pc_values[j];
                    data["pm_" + String(j+1)] = round(buffer[idx].data.pm_values[j] * 100) / 100.0;
                    data["np_" + String(j+1)] = buffer[idx].data.np_values[j];
                    data["pw_" + String(j+1)] = buffer[idx].data.pw_values[j];
                }
                data["debugMode"] = buffer[idx].data.debugMode;
                data["won"] = buffer[idx].data.won;
                totalSamples++;
            }
            
            // Zwolnij pamięć bufora
            freePSRAM(buffer);
        }
    } else if (sensor == "fan") {
        auto* fanHist = historyManager.getFanHistory();
        if (fanHist && fanHist->isInitialized()) {
            // Użyj heap zamiast stosu dla dużego bufora
            HistoryEntry<FanData>* buffer = allocatePSRAM<HistoryEntry<FanData>>(MAX_TOTAL_SAMPLES);
            if (!buffer) {
                safePrintln("[ERROR] getHistoricalData: Failed to allocate buffer for fan data");
                doc["error"] = "Memory allocation failed for fan buffer";
                serializeJson(doc, jsonResponse);
                return 0;
            }
            size_t count;
            if (sampleType == "slow") {
                count = fanHist->getSlowSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            } else {
                count = fanHist->getFastSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            }
            
            // Dodaj próbki w odwrotnej kolejności (od najnowszych do najstarszych)
            for (int i = count - 1; i >= 0; i--) {
                if (ESP.getFreeHeap() < 10000) {
                    safePrintln("[ERROR] getHistoricalData: Low memory during JSON build (fan), stopping at sample " + String(count - 1 - i));
                    break;
                }
                JsonObject sample = dataArray.createNestedObject();
                sample["timestamp"] = buffer[i].timestamp;
                sample["dateTime"] = buffer[i].dateTime;
                JsonObject data = sample.createNestedObject("data");
                data["dutyCycle"] = buffer[i].data.dutyCycle;
                data["rpm"] = buffer[i].data.rpm;
                data["enabled"] = buffer[i].data.enabled;
                data["glineEnabled"] = buffer[i].data.glineEnabled;
                totalSamples++;
            }
            
            // Zwolnij pamięć bufora
            freePSRAM(buffer);
        }
    } else if (sensor == "calibration" || sensor == "voc" || sensor == "co" || sensor == "no" || 
               sensor == "no2" || sensor == "o3" || sensor == "so2" || sensor == "h2s" || sensor == "nh3" ||
               sensor == "pm1" || sensor == "pm25" || sensor == "pm10") {
        auto* calibHist = historyManager.getCalibHistory();
        if (calibHist && calibHist->isInitialized()) {
            // Użyj heap zamiast stosu dla dużego bufora
            HistoryEntry<CalibratedSensorData>* buffer = allocatePSRAM<HistoryEntry<CalibratedSensorData>>(MAX_TOTAL_SAMPLES);
            if (!buffer) {
                safePrintln("[ERROR] getHistoricalData: Failed to allocate buffer for calibration data");
                doc["error"] = "Memory allocation failed for calibration buffer";
                serializeJson(doc, jsonResponse);
                return 0;
            }
            size_t count;
            if (sampleType == "slow") {
                count = calibHist->getSlowSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            } else {
                count = calibHist->getFastSamples(buffer, MAX_TOTAL_SAMPLES, fromTime, toTime);
            }
            
            // Dodaj próbki w odwrotnej kolejności (od najnowszych do najstarszych)
            for (int i = count - 1; i >= 0; i--) {
                if (ESP.getFreeHeap() < 10000) {
                    safePrintln("[ERROR] getHistoricalData: Low memory during JSON build (calibration/" + sensor + "), stopping at sample " + String(count - 1 - i));
                    break;
                }
                
                JsonObject sample = dataArray.createNestedObject();
                sample["timestamp"] = buffer[i].timestamp;
                sample["dateTime"] = buffer[i].dateTime;
                JsonObject data = sample.createNestedObject("data");
                
                if (sensor == "voc") {
                    // Tylko VOC
                    data["voc_ugm3"] = round(buffer[i].data.VOC * 10) / 10.0;
                    data["voc_ppb"] = round(buffer[i].data.VOC_ppb * 10) / 10.0;
                } else if (sensor == "co") {
                    // Tylko CO
                    data["co_ugm3"] = round(buffer[i].data.CO * 10) / 10.0;
                    data["co_ppb"] = round(buffer[i].data.CO_ppb * 10) / 10.0;
                } else if (sensor == "no") {
                    // Tylko NO
                    data["no_ugm3"] = round(buffer[i].data.NO * 10) / 10.0;
                    data["no_ppb"] = round(buffer[i].data.NO_ppb * 10) / 10.0;
                } else if (sensor == "no2") {
                    // Tylko NO2
                    data["no2_ugm3"] = round(buffer[i].data.NO2 * 10) / 10.0;
                    data["no2_ppb"] = round(buffer[i].data.NO2_ppb * 10) / 10.0;
                } else if (sensor == "o3") {
                    // Tylko O3
                    data["o3_ugm3"] = round(buffer[i].data.O3 * 10) / 10.0;
                    data["o3_ppb"] = round(buffer[i].data.O3_ppb * 10) / 10.0;
                } else if (sensor == "so2") {
                    // Tylko SO2
                    data["so2_ugm3"] = round(buffer[i].data.SO2 * 10) / 10.0;
                    data["so2_ppb"] = round(buffer[i].data.SO2_ppb * 10) / 10.0;
                } else if (sensor == "h2s") {
                    // Tylko H2S
                    data["h2s_ugm3"] = round(buffer[i].data.H2S * 10) / 10.0;
                    data["h2s_ppb"] = round(buffer[i].data.H2S_ppb * 10) / 10.0;
                } else if (sensor == "nh3") {
                    // Tylko NH3
                    data["nh3_ugm3"] = round(buffer[i].data.NH3 * 10) / 10.0;
                    data["nh3_ppb"] = round(buffer[i].data.NH3_ppb * 10) / 10.0;
                } else if (sensor == "pm1") {
                    // Tylko PM1
                    data["pm1_ugm3"] = round(buffer[i].data.PM1 * 10) / 10.0;
                } else if (sensor == "pm25") {
                    // Tylko PM2.5
                    data["pm25_ugm3"] = round(buffer[i].data.PM25 * 10) / 10.0;
                } else if (sensor == "pm10") {
                    // Tylko PM10
                    data["pm10_ugm3"] = round(buffer[i].data.PM10 * 10) / 10.0;
                } else {
                    // Wszystkie gazy (calibration)
                    data["CO"] = round(buffer[i].data.CO * 10) / 10.0;
                    data["NO"] = round(buffer[i].data.NO * 10) / 10.0;
                    data["NO2"] = round(buffer[i].data.NO2 * 10) / 10.0;
                    data["O3"] = round(buffer[i].data.O3 * 10) / 10.0;
                    data["SO2"] = round(buffer[i].data.SO2 * 10) / 10.0;
                    data["H2S"] = round(buffer[i].data.H2S * 10) / 10.0;
                    data["NH3"] = round(buffer[i].data.NH3 * 10) / 10.0;
                    data["VOC"] = round(buffer[i].data.VOC * 10) / 10.0;
                    data["VOC_ppb"] = round(buffer[i].data.VOC_ppb * 10) / 10.0;
                    data["HCHO"] = round(buffer[i].data.HCHO * 10) / 10.0;
                    data["PID"] = round(buffer[i].data.PID * 1000) / 1000.0;
                    
                    // ODO
                    data["ODO"] = round(buffer[i].data.ODO * 10) / 10.0;
                    
                    // PM sensors (SPS30)
                    data["PM1"] = round(buffer[i].data.PM1 * 10) / 10.0;
                    data["PM25"] = round(buffer[i].data.PM25 * 10) / 10.0;
                    data["PM10"] = round(buffer[i].data.PM10 * 10) / 10.0;
                    
                    // Environmental sensors
                    
                    
                    data["DUST_TEMP"] = round(buffer[i].data.DUST_TEMP * 10) / 10.0;
                    data["DUST_HUMID"] = round(buffer[i].data.DUST_HUMID * 10) / 10.0;
                    data["DUST_PRESS"] = round(buffer[i].data.DUST_PRESS * 10) / 10.0;
                    
                    
                    
                    // CO2 sensor
                    data["SCD_CO2"] = round(buffer[i].data.SCD_CO2 * 10) / 10.0;
                    data["SCD_T"] = round(buffer[i].data.SCD_T * 10) / 10.0;
                    data["SCD_RH"] = round(buffer[i].data.SCD_RH * 10) / 10.0;
                }
                totalSamples++;
            }
            
            // Zwolnij pamięć bufora
            freePSRAM(buffer);
        }
    } else {
        // Unknown sensor type
        safePrintln("[ERROR] getHistoricalData: Unknown sensor type: " + sensor);
        doc["error"] = "Unknown sensor type: " + sensor;
        doc["totalSamples"] = 0;
        doc["freeHeap"] = ESP.getFreeHeap();
        serializeJson(doc, jsonResponse);
        return 0;
    }
    
    // Check if JSON document has enough space for metadata
    size_t currentMemoryUsage = doc.memoryUsage();
    size_t docCapacity = doc.capacity();
    safePrintln("[DEBUG] JSON memory: used=" + String(currentMemoryUsage) + 
               " capacity=" + String(docCapacity) + " free=" + String(docCapacity - currentMemoryUsage));
    
    // Add pagination metadata
    size_t totalPackets = (totalAvailableSamples + effectivePacketSize - 1) / effectivePacketSize;
    if (totalPackets == 0) totalPackets = 1; // Min 1 pakiet nawet gdy brak danych
    
    doc["totalSamples"] = totalSamples;
    doc["totalAvailableSamples"] = totalAvailableSamples;
    doc["packetIndex"] = currentPacketIndex;
    doc["packetSize"] = effectivePacketSize;
    doc["totalPackets"] = totalPackets;
    doc["hasMorePackets"] = (currentPacketIndex < totalPackets - 1);
    doc["freeHeap"] = ESP.getFreeHeap();
    
    // Debug: sprawdź pamięć i metadane po dodaniu
    size_t finalMemoryUsage = doc.memoryUsage();
    safePrintln("[DEBUG] JSON memory after metadata: used=" + String(finalMemoryUsage) + 
               " capacity=" + String(doc.capacity()) + " overflow=" + String(doc.overflowed()));
    
    // Debug: sprawdź metadane przed serializacją
    safePrintln("[DEBUG] History: totalSamples=" + String(totalSamples) + 
               " totalAvailableSamples=" + String(totalAvailableSamples) +
               " totalPackets=" + String(totalPackets) +
               " packetIndex=" + String(currentPacketIndex));
    
    // Serialize to string
    serializeJson(doc, jsonResponse);
    //print size of response
    safePrintln("JSON response size: " + String(jsonResponse.length()) + " bytes");
    
    // Limit response size to prevent memory issues
    if (jsonResponse.length() > 8000) { // Increased limit for DynamicJsonDocument
        safePrintln("JSON response too large: " + String(jsonResponse.length()) + " bytes, truncating");
        DynamicJsonDocument errorDoc(512);
        errorDoc["error"] = "Response too large";
        errorDoc["size"] = jsonResponse.length();
        errorDoc["freeHeap"] = ESP.getFreeHeap();
        serializeJson(errorDoc, jsonResponse);
        return 0;
    }
    
    return totalSamples;
}