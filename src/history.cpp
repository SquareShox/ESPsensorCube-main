#include <history.h>
#include <sensors.h>
#include <mean.h>
#include <calib.h>

// Forward declarations for safe printing functions
void safePrint(const String& message);
void safePrintln(const String& message);

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
}

bool HistoryManager::initialize() {
    safePrintln("=== Initializing Sensor History (1MB PSRAM) ===");
    
    // Check available memory before initialization
    size_t psramSize = ESP.getPsramSize();
    size_t freePsram = ESP.getFreePsram();
    size_t freeHeap = ESP.getFreeHeap();
    
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
        solarHistory = new SensorHistory<SolarData, SOLAR_FAST_HISTORY, SOLAR_SLOW_HISTORY>();
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
            safePrintln("Failed to initialize Solar history");
            allSuccess = false;
        }
    }
    
    if (config.enableI2CSensors) {
        i2cHistory = new SensorHistory<I2CSensorData, I2C_FAST_HISTORY, I2C_SLOW_HISTORY>();
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
            safePrintln("Failed to initialize I2C history");
            allSuccess = false;
        }
    }
    
    if (config.enableSPS30) {
        sps30History = new SensorHistory<SPS30Data, SPS30_FAST_HISTORY, SPS30_SLOW_HISTORY>();
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
            safePrintln("Failed to initialize SPS30 history");
            allSuccess = false;
        }
    }
    
    if (config.enableIPS) {
        ipsHistory = new SensorHistory<IPSSensorData, IPS_FAST_HISTORY, IPS_SLOW_HISTORY>();
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
            safePrintln("Failed to initialize IPS history");
            allSuccess = false;
        }
    }
    
    if (config.enableMCP3424) {
        mcp3424History = new SensorHistory<MCP3424Data, MCP3424_FAST_HISTORY, MCP3424_SLOW_HISTORY>();
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
            safePrintln("Failed to initialize MCP3424 history");
            allSuccess = false;
        }
    }
    
    if (config.enableADS1110) {
        ads1110History = new SensorHistory<ADS1110Data, ADS1110_FAST_HISTORY, ADS1110_SLOW_HISTORY>();
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
            safePrintln("Failed to initialize ADS1110 history");
            allSuccess = false;
        }
    }
    
    if (config.enableINA219) {
        ina219History = new SensorHistory<INA219Data, INA219_FAST_HISTORY, INA219_SLOW_HISTORY>();
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
            safePrintln("Failed to initialize INA219 history");
            allSuccess = false;
        }
    }
    
    if (config.enableSHT40) {
        sht40History = new SensorHistory<SHT40Data, SHT40_FAST_HISTORY, SHT40_SLOW_HISTORY>();
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
            safePrintln("Failed to initialize SHT40 history");
            allSuccess = false;
        }
    }
    
    if (calibConfig.enableCalibration) {
        calibHistory = new SensorHistory<CalibratedSensorData, CALIB_FAST_HISTORY, CALIB_SLOW_HISTORY>();
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
            safePrintln("Failed to initialize Calibration history");
            allSuccess = false;
        }
    }
    
    if (config.enableHCHO) {
        hchoHistory = new SensorHistory<HCHOData, HCHO_FAST_HISTORY, HCHO_SLOW_HISTORY>();
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
            safePrintln("Failed to initialize HCHO history");
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
    
    unsigned long currentTime = millis();
    static unsigned long lastFastUpdate = 0;
    static unsigned long lastSlowUpdate = 0;
    
    // Update fast averages every 10 seconds
    if (currentTime - lastFastUpdate >= 10000) {
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
            }
        }
        
        if (sps30History && config.enableSPS30) {
            SPS30Data fastAvg = getSPS30FastAverage();
            if (fastAvg.valid) {
                sps30History->addFastSample(fastAvg, currentTime);
            }
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
    }
    
    // Update slow averages every 5 minutes (300 seconds)
    if (currentTime - lastSlowUpdate >= 300000) {
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
    
    // Sprawdź dostępność PSRAM
    if (ESP.getPsramSize() > 0) {
        safePrint("PSRAM total: ");
        safePrint(String(ESP.getPsramSize() / 1024));
        safePrintln(" KB");
        safePrint("PSRAM free: ");
        safePrint(String(ESP.getFreePsram() / 1024));
        safePrintln(" KB");
    } else {
        safePrintln("PSRAM not available - using heap memory");
    }
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
}

// Global interface functions
void initializeHistory() {
    if (!config.enableHistory) {
        safePrintln("History system disabled in configuration");
        return;
    }
    historyManager.initialize();
}

void updateSensorHistory() {
    if (!config.enableHistory) return;
    historyManager.updateHistory();
}

void printHistoryMemoryUsage() {
    historyManager.printMemoryUsage();
}

void printHistoryStatus() {
    historyManager.printHistoryStatus();
}

// API function for getting historical data
size_t getHistoricalData(const String& sensor, const String& timeRange, 
                        String& jsonResponse, unsigned long fromTime, unsigned long toTime) {
    if (!config.enableHistory) {
        jsonResponse = "{\"error\":\"History disabled in configuration\",\"data\":[]}";
        return 0;
    }
    
    if (!historyManager.isInitialized()) {
        jsonResponse = "{\"error\":\"History not initialized\",\"data\":[]}";
        return 0;
    }
    
    // Check available memory before starting
    size_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < 20000) { // Need at least 20KB free
        jsonResponse = "{\"error\":\"Low memory\",\"data\":[],\"freeHeap\":" + String(freeHeap) + "}";
        return 0;
    }
    
    // Reserve memory for JSON response to avoid reallocations
    jsonResponse.reserve(2048); // Reserve 2KB for single sensor
    jsonResponse = "{\"sensor\":\"" + sensor + "\",\"timeRange\":\"" + timeRange + "\",\"data\":[";
    
    // Buffer for samples (limited to avoid memory issues)
    const size_t MAX_SAMPLES = 30; // Further reduced for single sensor requests
    bool hasData = false;
    size_t totalSamples = 0;
    
    if (sensor == "i2c") {
        auto* i2cHist = historyManager.getI2CHistory();
        if (i2cHist && i2cHist->isInitialized()) {
            HistoryEntry<I2CSensorData> buffer[MAX_SAMPLES];
            size_t count = i2cHist->getFastSamples(buffer, MAX_SAMPLES, fromTime, toTime);
            
            for (size_t i = 0; i < count; i++) {
                // Check memory before adding more data
                if (ESP.getFreeHeap() < 10000) {
                    safePrintln("Low memory during JSON build, stopping");
                    break;
                }
                
                if (hasData) jsonResponse += ",";
                jsonResponse += "{\"timestamp\":" + String(buffer[i].timestamp) + 
                               ",\"data\":{\"temperature\":" + String(buffer[i].data.temperature, 1) +
                               ",\"humidity\":" + String(buffer[i].data.humidity, 1) +
                               ",\"pressure\":" + String(buffer[i].data.pressure, 1) +
                               ",\"co2\":" + String(buffer[i].data.co2) + "}}";
                hasData = true;
                totalSamples++;
            }
        }
    } else if (sensor == "solar") {
        auto* solarHist = historyManager.getSolarHistory();
        if (solarHist && solarHist->isInitialized()) {
            HistoryEntry<SolarData> buffer[MAX_SAMPLES];
            size_t count = solarHist->getFastSamples(buffer, MAX_SAMPLES, fromTime, toTime);
            
            for (size_t i = 0; i < count; i++) {
                if (ESP.getFreeHeap() < 10000) break;
                if (hasData) jsonResponse += ",";
                jsonResponse += "{\"timestamp\":" + String(buffer[i].timestamp) + 
                               ",\"data\":{\"V\":\"" + buffer[i].data.V +
                               "\",\"I\":\"" + buffer[i].data.I +
                               "\",\"PPV\":\"" + buffer[i].data.PPV + "\"}}";
                hasData = true;
                totalSamples++;
            }
        }
    } else if (sensor == "sps30") {
        auto* sps30Hist = historyManager.getSPS30History();
        if (sps30Hist && sps30Hist->isInitialized()) {
            HistoryEntry<SPS30Data> buffer[MAX_SAMPLES];
            size_t count = sps30Hist->getFastSamples(buffer, MAX_SAMPLES, fromTime, toTime);
            
            for (size_t i = 0; i < count; i++) {
                if (ESP.getFreeHeap() < 10000) break;
                if (hasData) jsonResponse += ",";
                jsonResponse += "{\"timestamp\":" + String(buffer[i].timestamp) + 
                               ",\"data\":{\"pm1_0\":" + String(buffer[i].data.pm1_0, 1) +
                               ",\"pm2_5\":" + String(buffer[i].data.pm2_5, 1) +
                               ",\"pm10\":" + String(buffer[i].data.pm10, 1) + "}}";
                hasData = true;
                totalSamples++;
            }
        }
    } else if (sensor == "power") {
        auto* powerHist = historyManager.getINA219History();
        if (powerHist && powerHist->isInitialized()) {
            HistoryEntry<INA219Data> buffer[MAX_SAMPLES];
            size_t count = powerHist->getFastSamples(buffer, MAX_SAMPLES, fromTime, toTime);
            
            for (size_t i = 0; i < count; i++) {
                if (ESP.getFreeHeap() < 10000) break;
                if (hasData) jsonResponse += ",";
                jsonResponse += "{\"timestamp\":" + String(buffer[i].timestamp) + 
                               ",\"data\":{\"busVoltage\":" + String(buffer[i].data.busVoltage, 3) +
                               ",\"current\":" + String(buffer[i].data.current, 2) +
                               ",\"power\":" + String(buffer[i].data.power, 2) + "}}";
                hasData = true;
                totalSamples++;
            }
        }
    } else if (sensor == "hcho") {
        auto* hchoHist = historyManager.getHCHOHistory();
        if (hchoHist && hchoHist->isInitialized()) {
            HistoryEntry<HCHOData> buffer[MAX_SAMPLES];
            size_t count = hchoHist->getFastSamples(buffer, MAX_SAMPLES, fromTime, toTime);
            
            for (size_t i = 0; i < count; i++) {
                if (ESP.getFreeHeap() < 10000) break;
                if (hasData) jsonResponse += ",";
                jsonResponse += "{\"timestamp\":" + String(buffer[i].timestamp) + 
                               ",\"data\":{\"hcho\":" + String(buffer[i].data.hcho, 3) +
                               ",\"voc\":" + String(buffer[i].data.voc, 3) +
                               ",\"tvoc\":" + String(buffer[i].data.tvoc, 3) + "}}";
                hasData = true;
                totalSamples++;
            }
        }
    } else {
        // Unknown sensor type
        jsonResponse += "],\"error\":\"Unknown sensor type: " + sensor + "\",\"totalSamples\":0,\"freeHeap\":" + String(ESP.getFreeHeap()) + "}";
        return 0;
    }
    
    jsonResponse += "],\"totalSamples\":" + String(totalSamples) + ",\"freeHeap\":" + String(ESP.getFreeHeap()) + "}";
    
    // Limit response size to prevent memory issues
    if (jsonResponse.length() > 3000) {
        safePrintln("JSON response too large: " + String(jsonResponse.length()) + " bytes, truncating");
        jsonResponse = "{\"error\":\"Response too large\",\"size\":" + String(jsonResponse.length()) + ",\"freeHeap\":" + String(ESP.getFreeHeap()) + "}";
        return 0;
    }
    
    return totalSamples;
} 