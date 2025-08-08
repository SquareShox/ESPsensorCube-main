#ifndef HISTORY_H
#define HISTORY_H

#include <Arduino.h>
#include <sensors.h>
#include <calib.h>
#include <config.h>
#include <time.h>
#include <cstring>
#include <esp_heap_caps.h> // For PSRAM allocation

// Forward declaration
void getFormattedDateTime(char* buffer, size_t bufferSize);

// Obliczenia pamięci dla historii uśrednień
// Założenia:
// - 1MB (1,048,576 bytes) dostępnej pamięci PSRAM
// - Struktura HistoryEntry: timestamp (4B) + data (rozmiar zależny od typu czujnika)
// - Fast samples (10s): przechowywane przez 1 godzinę = 360 próbek
// - Slow samples (5min): przechowywane przez 24 godziny = 288 próbek

// Rozmiary struktur danych (w bajtach, przybliżone)
#define SOLAR_DATA_SIZE 80        // String fields + validity
#define I2C_SENSOR_SIZE 20        // 4 floats + type + validity
#define SPS30_DATA_SIZE 44        // 10 floats + validity
#define IPS_SENSOR_SIZE 120       // arrays + flags
#define MCP3424_DATA_SIZE 200     // multi-device arrays
#define ADS1110_DATA_SIZE 16      // voltage + settings
#define INA219_DATA_SIZE 20       // 4 floats + validity
#define SHT40_DATA_SIZE 16        // 3 floats + validity
#define CALIB_DATA_SIZE 300       // wiele floatów kalibracji
#define HCHO_DATA_SIZE 32         // 5 floats + flags
#define FAN_DATA_SIZE 16          // dutyCycle, rpm, flags
#define BATTERY_DATA_SIZE 32      // voltage, current, power, chargePercent, flags
#define BATTERY_ENTRY_SIZE 36     // BATTERY_DATA_SIZE + timestamp (4B)

// Rozmiary historii dla baterii
#define BATTERY_FAST_HISTORY 360  // 1 godzina (co 10s)
#define BATTERY_SLOW_HISTORY 288  // 24 godziny (co 5min)

// Struktura wpisu historii
template<typename T>
struct HistoryEntry {
    unsigned long timestamp;
    char dateTime[20];  // Format: "YYYY-MM-DD HH:MM:SS" - fixed size array
    T data;
    
    // Konstruktor domyślny
    HistoryEntry() {
        timestamp = 0;
        memset(dateTime, 0, sizeof(dateTime));
        // Inicjalizuj data do bezpiecznych wartości
        memset(&data, 0, sizeof(T));
    }
    
    // Konstruktor kopiujący
    HistoryEntry(const HistoryEntry& other) {
        timestamp = other.timestamp;
        strncpy(dateTime, other.dateTime, sizeof(dateTime) - 1);
        dateTime[sizeof(dateTime) - 1] = '\0';
        data = other.data;
    }
    
    // Operator przypisania
    HistoryEntry& operator=(const HistoryEntry& other) {
        if (this != &other) {
            timestamp = other.timestamp;
            strncpy(dateTime, other.dateTime, sizeof(dateTime) - 1);
            dateTime[sizeof(dateTime) - 1] = '\0';
            data = other.data;
        }
        return *this;
    }
    
    // Konstruktor przenoszący
    HistoryEntry(HistoryEntry&& other) noexcept {
        timestamp = other.timestamp;
        strncpy(dateTime, other.dateTime, sizeof(dateTime) - 1);
        dateTime[sizeof(dateTime) - 1] = '\0';
        data = other.data;  // Usunięto std::move dla kompatybilności
        other.timestamp = 0;
        memset(other.dateTime, 0, sizeof(other.dateTime));
    }
    
    // Operator przenoszący
    HistoryEntry& operator=(HistoryEntry&& other) noexcept {
        if (this != &other) {
            timestamp = other.timestamp;
            strncpy(dateTime, other.dateTime, sizeof(dateTime) - 1);
            dateTime[sizeof(dateTime) - 1] = '\0';
            data = other.data;  // Usunięto std::move dla kompatybilności
            other.timestamp = 0;
            memset(other.dateTime, 0, sizeof(other.dateTime));
        }
        return *this;
    }
};

// Rozmiary pojedynczego wpisu historii (timestamp + dateTime + data)
// String dateTime zajmuje około 20 bajtów (19 znaków + null terminator)
#define DATETIME_SIZE 20
#define SOLAR_ENTRY_SIZE (sizeof(unsigned long) + DATETIME_SIZE + SOLAR_DATA_SIZE)
#define I2C_ENTRY_SIZE (sizeof(unsigned long) + DATETIME_SIZE + I2C_SENSOR_SIZE)
#define SPS30_ENTRY_SIZE (sizeof(unsigned long) + DATETIME_SIZE + SPS30_DATA_SIZE)
#define IPS_ENTRY_SIZE (sizeof(unsigned long) + DATETIME_SIZE + IPS_SENSOR_SIZE)
#define MCP3424_ENTRY_SIZE (sizeof(unsigned long) + DATETIME_SIZE + MCP3424_DATA_SIZE)
#define ADS1110_ENTRY_SIZE (sizeof(unsigned long) + DATETIME_SIZE + ADS1110_DATA_SIZE)
#define INA219_ENTRY_SIZE (sizeof(unsigned long) + DATETIME_SIZE + INA219_DATA_SIZE)
#define SHT40_ENTRY_SIZE (sizeof(unsigned long) + DATETIME_SIZE + SHT40_DATA_SIZE)
#define CALIB_ENTRY_SIZE (sizeof(unsigned long) + DATETIME_SIZE + CALIB_DATA_SIZE)
#define HCHO_ENTRY_SIZE (sizeof(unsigned long) + DATETIME_SIZE + HCHO_DATA_SIZE)
#define FAN_ENTRY_SIZE (sizeof(unsigned long) + DATETIME_SIZE + FAN_DATA_SIZE)

// Kalkulacja liczby próbek dla 1MB pamięci
// Zakładamy równy podział między Fast (1h) i Slow (24h) dla każdego czujnika
#define TARGET_MEMORY_BYTES (1024 * 1024)  // 1MB

// Liczba próbek dla każdego typu (Fast + Slow łącznie)
#define SOLAR_FAST_HISTORY 300      // 1h przy 10s = 360, zmniejszone dla oszczędności
#define SOLAR_SLOW_HISTORY 240      // 24h przy 5min = 288, zmniejszone
#define I2C_FAST_HISTORY 360        // pełne 1h
#define I2C_SLOW_HISTORY 288        // pełne 24h
#define SPS30_FAST_HISTORY 300
#define SPS30_SLOW_HISTORY 240
#define IPS_FAST_HISTORY 200        // zmniejszone ze względu na duży rozmiar
#define IPS_SLOW_HISTORY 150
#define MCP3424_FAST_HISTORY 150    // duże struktury, zmniejszone
#define MCP3424_SLOW_HISTORY 120
#define ADS1110_FAST_HISTORY 360
#define ADS1110_SLOW_HISTORY 288
#define INA219_FAST_HISTORY 360
#define INA219_SLOW_HISTORY 288
#define SHT40_FAST_HISTORY 360
#define SHT40_SLOW_HISTORY 288
#define CALIB_FAST_HISTORY 100      // bardzo duże struktury
#define CALIB_SLOW_HISTORY 80
#define HCHO_FAST_HISTORY 360
#define HCHO_SLOW_HISTORY 288
#define FAN_FAST_HISTORY 360
#define FAN_SLOW_HISTORY 288

// Obliczenie całkowitego zużycia pamięci (dla weryfikacji)
#define TOTAL_MEMORY_ESTIMATE ( \
    (SOLAR_FAST_HISTORY + SOLAR_SLOW_HISTORY) * SOLAR_ENTRY_SIZE + \
    (I2C_FAST_HISTORY + I2C_SLOW_HISTORY) * I2C_ENTRY_SIZE + \
    (SPS30_FAST_HISTORY + SPS30_SLOW_HISTORY) * SPS30_ENTRY_SIZE + \
    (IPS_FAST_HISTORY + IPS_SLOW_HISTORY) * IPS_ENTRY_SIZE + \
    (MCP3424_FAST_HISTORY + MCP3424_SLOW_HISTORY) * MCP3424_ENTRY_SIZE + \
    (ADS1110_FAST_HISTORY + ADS1110_SLOW_HISTORY) * ADS1110_ENTRY_SIZE + \
    (INA219_FAST_HISTORY + INA219_SLOW_HISTORY) * INA219_ENTRY_SIZE + \
    (SHT40_FAST_HISTORY + SHT40_SLOW_HISTORY) * SHT40_ENTRY_SIZE + \
    (CALIB_FAST_HISTORY + CALIB_SLOW_HISTORY) * CALIB_ENTRY_SIZE + \
    (HCHO_FAST_HISTORY + HCHO_SLOW_HISTORY) * HCHO_ENTRY_SIZE + \
    (FAN_FAST_HISTORY + FAN_SLOW_HISTORY) * FAN_ENTRY_SIZE \
)

// Klasa zarządzająca historią dla jednego typu czujnika
template<typename T, size_t FAST_SIZE, size_t SLOW_SIZE>
class SensorHistory {
private:
    HistoryEntry<T>* fastHistory;
    HistoryEntry<T>* slowHistory;
    size_t fastHead = 0;
    size_t slowHead = 0;
    size_t fastCount = 0;
    size_t slowCount = 0;
    bool initialized = false;

public:
    SensorHistory() : fastHistory(nullptr), slowHistory(nullptr) {}
    
    ~SensorHistory() {
        if (fastHistory) {
            // heap_caps_free działa zarówno dla PSRAM jak i heap
            heap_caps_free(fastHistory);
            fastHistory = nullptr;
        }
        if (slowHistory) {
            heap_caps_free(slowHistory);
            slowHistory = nullptr;
        }
    }
    
    bool initialize() {
        // Alokuj w PSRAM jeśli dostępny, inaczej fallback do heap
        size_t fastSize = FAST_SIZE * sizeof(HistoryEntry<T>);
        size_t slowSize = SLOW_SIZE * sizeof(HistoryEntry<T>);
        
        if (ESP.getPsramSize() > 0) {
            // Próbuj PSRAM
            fastHistory = (HistoryEntry<T>*)heap_caps_malloc(fastSize, MALLOC_CAP_SPIRAM);
            slowHistory = (HistoryEntry<T>*)heap_caps_malloc(slowSize, MALLOC_CAP_SPIRAM);
            
            if (fastHistory && slowHistory) {
                Serial.println("✓ SensorHistory allocated in PSRAM (" + String(fastSize + slowSize) + " bytes)");
            } else {
                // Cleanup partial allocation
                if (fastHistory) heap_caps_free(fastHistory);
                if (slowHistory) heap_caps_free(slowHistory);
                fastHistory = nullptr;
                slowHistory = nullptr;
            }
        }
        
        // Nie alokuj w heap jeśli PSRAM się nie udało
        if (!fastHistory || !slowHistory) {
            Serial.println("✗ SensorHistory allocation failed - PSRAM unavailable or insufficient");
        }
        
        initialized = (fastHistory != nullptr && slowHistory != nullptr);
        return initialized;
    }
    
    void addFastSample(const T& data, unsigned long timestamp) {
        if (!initialized || !fastHistory) return;
        
        fastHistory[fastHead].timestamp = timestamp;
        getFormattedDateTime(fastHistory[fastHead].dateTime, sizeof(fastHistory[fastHead].dateTime));
        fastHistory[fastHead].data = data;
        fastHead = (fastHead + 1) % FAST_SIZE;
        if (fastCount < FAST_SIZE) fastCount++;
    }
    
    void addSlowSample(const T& data, unsigned long timestamp) {
        if (!initialized || !slowHistory) return;
        
        slowHistory[slowHead].timestamp = timestamp;
        getFormattedDateTime(slowHistory[slowHead].dateTime, sizeof(slowHistory[slowHead].dateTime));
        slowHistory[slowHead].data = data;
        slowHead = (slowHead + 1) % SLOW_SIZE;
        if (slowCount < SLOW_SIZE) slowCount++;
    }
    
    // Pobierz próbki z określonego zakresu czasowego
    size_t getFastSamples(HistoryEntry<T>* buffer, size_t bufferSize, 
                         unsigned long fromTime, unsigned long toTime) const {
        if (!initialized || !fastHistory || !buffer) return 0;
        
        size_t found = 0;
        for (size_t i = 0; i < fastCount && found < bufferSize; i++) {
            size_t index = (fastHead + FAST_SIZE - fastCount + i) % FAST_SIZE;
            if (fastHistory[index].timestamp >= fromTime && 
                fastHistory[index].timestamp <= toTime) {
                buffer[found++] = fastHistory[index];
            }
        }
        return found;
    }
    
    size_t getSlowSamples(HistoryEntry<T>* buffer, size_t bufferSize,
                         unsigned long fromTime, unsigned long toTime) const {
        if (!initialized || !slowHistory || !buffer) return 0;
        
        size_t found = 0;
        for (size_t i = 0; i < slowCount && found < bufferSize; i++) {
            size_t index = (slowHead + SLOW_SIZE - slowCount + i) % SLOW_SIZE;
            if (slowHistory[index].timestamp >= fromTime && 
                slowHistory[index].timestamp <= toTime) {
                buffer[found++] = slowHistory[index];
            }
        }
        return found;
    }
    
    size_t getFastCount() const { return fastCount; }
    size_t getSlowCount() const { return slowCount; }
    bool isInitialized() const { return initialized; }
    
    // Pobierz najnowszą próbkę
    bool getLatestFast(HistoryEntry<T>& entry) const {
        if (!initialized || fastCount == 0) return false;
        size_t index = (fastHead + FAST_SIZE - 1) % FAST_SIZE;
        entry = fastHistory[index];
        return true;
    }
    
    bool getLatestSlow(HistoryEntry<T>& entry) const {
        if (!initialized || slowCount == 0) return false;
        size_t index = (slowHead + SLOW_SIZE - 1) % SLOW_SIZE;
        entry = slowHistory[index];
        return true;
    }
};

// Manager historii dla wszystkich czujników
class HistoryManager {
private:
    SensorHistory<SolarData, SOLAR_FAST_HISTORY, SOLAR_SLOW_HISTORY>* solarHistory;
    SensorHistory<I2CSensorData, I2C_FAST_HISTORY, I2C_SLOW_HISTORY>* i2cHistory;
    SensorHistory<SPS30Data, SPS30_FAST_HISTORY, SPS30_SLOW_HISTORY>* sps30History;
    SensorHistory<IPSSensorData, IPS_FAST_HISTORY, IPS_SLOW_HISTORY>* ipsHistory;
    SensorHistory<MCP3424Data, MCP3424_FAST_HISTORY, MCP3424_SLOW_HISTORY>* mcp3424History;
    SensorHistory<ADS1110Data, ADS1110_FAST_HISTORY, ADS1110_SLOW_HISTORY>* ads1110History;
    SensorHistory<INA219Data, INA219_FAST_HISTORY, INA219_SLOW_HISTORY>* ina219History;
    SensorHistory<SHT40Data, SHT40_FAST_HISTORY, SHT40_SLOW_HISTORY>* sht40History;
    SensorHistory<CalibratedSensorData, CALIB_FAST_HISTORY, CALIB_SLOW_HISTORY>* calibHistory;
    SensorHistory<HCHOData, HCHO_FAST_HISTORY, HCHO_SLOW_HISTORY>* hchoHistory;
    SensorHistory<FanData, FAN_FAST_HISTORY, FAN_SLOW_HISTORY>* fanHistory;
    SensorHistory<BatteryData, BATTERY_FAST_HISTORY, BATTERY_SLOW_HISTORY>* batteryHistory;
    
    bool initialized = false;
    size_t totalMemoryUsed = 0;

public:
    HistoryManager();
    ~HistoryManager();
    
    bool initialize();
    void updateHistory();
    void printMemoryUsage() const;
    void printHistoryStatus() const;
    
    // Funkcje dostępu do historii poszczególnych czujników
    SensorHistory<SolarData, SOLAR_FAST_HISTORY, SOLAR_SLOW_HISTORY>* getSolarHistory() { return solarHistory; }
    SensorHistory<I2CSensorData, I2C_FAST_HISTORY, I2C_SLOW_HISTORY>* getI2CHistory() { return i2cHistory; }
    SensorHistory<SPS30Data, SPS30_FAST_HISTORY, SPS30_SLOW_HISTORY>* getSPS30History() { return sps30History; }
    SensorHistory<IPSSensorData, IPS_FAST_HISTORY, IPS_SLOW_HISTORY>* getIPSHistory() { return ipsHistory; }
    SensorHistory<MCP3424Data, MCP3424_FAST_HISTORY, MCP3424_SLOW_HISTORY>* getMCP3424History() { return mcp3424History; }
    SensorHistory<ADS1110Data, ADS1110_FAST_HISTORY, ADS1110_SLOW_HISTORY>* getADS1110History() { return ads1110History; }
    SensorHistory<INA219Data, INA219_FAST_HISTORY, INA219_SLOW_HISTORY>* getINA219History() { return ina219History; }
    SensorHistory<SHT40Data, SHT40_FAST_HISTORY, SHT40_SLOW_HISTORY>* getSHT40History() { return sht40History; }
    SensorHistory<CalibratedSensorData, CALIB_FAST_HISTORY, CALIB_SLOW_HISTORY>* getCalibHistory() { return calibHistory; }
    SensorHistory<HCHOData, HCHO_FAST_HISTORY, HCHO_SLOW_HISTORY>* getHCHOHistory() { return hchoHistory; }
    SensorHistory<FanData, FAN_FAST_HISTORY, FAN_SLOW_HISTORY>* getFanHistory() { return fanHistory; }
    SensorHistory<BatteryData, BATTERY_FAST_HISTORY, BATTERY_SLOW_HISTORY>* getBatteryHistory() { return batteryHistory; }
    
    bool isInitialized() const { return initialized; }
    size_t getTotalMemoryUsed() const { return totalMemoryUsed; }
};

// Globalne funkcje interfejsu
extern HistoryManager historyManager;

void initializeHistory();
void updateSensorHistory();
void printHistoryMemoryUsage();
void printHistoryStatus();
void checkHistoryMemoryType();

// Funkcje API do pobierania danych historycznych z pakietowaniem
size_t getHistoricalData(const String& sensor, const String& timeRange, 
                        String& jsonResponse, unsigned long fromTime = 0, unsigned long toTime = 0,
                        const String& sampleType = "fast", int packetIndex = 0, int packetSize = 20);

#endif // HISTORY_H 