# System Średnich Kroczących dla ESP32 Sensor Cube

## Przegląd

Zaimplementowano zaawansowany system średnich kroczących dla wszystkich czujników w projekcie ESP32 Sensor Cube. System wykorzystuje ważone średnie czasowe z okresami 10 sekund (szybkie) i 5 minut (wolne).

## Funkcje Systemu

### 1. Generyczny Circular Buffer
- Template-based implementation dla różnych typów danych czujników
- Automatyczne zarządzanie pamięcią (statyczne bufory)
- Wsparcie dla timestamp-based weighting
- Exponential decay weights (nowsze próbki mają większą wagę)

### 2. Obsługiwane Czujniki
- **Solar Sensor** (SolarData)
- **I2C Sensors** (I2CSensorData) - temperatura, wilgotność, ciśnienie, CO2
- **SPS30** (SPS30Data) - czujnik cząstek stałych
- **IPS Sensor** (IPSSensorData) - zaawansowany czujnik cząstek
- **MCP3424** (MCP3424Data) - 18-bit ADC converter
- **ADS1110** (ADS1110Data) - 16-bit ADC converter
- **INA219** (INA219Data) - czujnik prądu/napięcia

### 3. Dwa Okresy Średnich
- **Fast Average**: 10 sekund (aktualizacja co 5s)
- **Slow Average**: 5 minut (aktualizacja co 30s)
- Automatyczne starzenie się danych
- Inteligentne ważenie czasowe

## Integracja z Modbus

### Rejestry Kontrolne
- **Rejestr 90**: Typ danych (0=aktualne, 1=średnie 10s, 2=średnie 5min)
- **Rejestr 91**: Komendy systemowe
- **Rejestr 92**: Auto reset flag
- **Rejestr 93**: Status średnich kroczących

### Nowe Komendy Modbus
- **Komenda 7**: Przełącz na dane aktualne
- **Komenda 8**: Przełącz na średnie szybkie (10s)
- **Komenda 9**: Przełącz na średnie wolne (5min)
- **Komenda 10**: Wydrukuj status średnich kroczących

### Automatyczne Przełączanie
System automatycznie używa odpowiedniego typu danych (aktualne/średnie) dla wszystkich czujników w zależności od ustawienia rejestru 90.

## Pliki Kodu

### Nowe Pliki
- `src/mean.cpp` - główna implementacja systemu średnich kroczących
- `include/mean.h` - nagłówek z deklaracjami funkcji
- `test_moving_averages.py` - skrypt testowy Python z obsługą Modbus

### Zmodyfikowane Pliki
- `src/modbus_handler.cpp` - integracja z systemem średnich
- `src/main.cpp` - inicjalizacja i wywołania systemu średnich

## Użycie

### Inicjalizacja
```cpp
// W setup()
initializeMovingAverages();
```

### Aktualizacja Danych
```cpp
// W loop() - wywoływane automatycznie przez processModbusTask()
updateMovingAverages();
```

### Pobieranie Średnich
```cpp
// Szybkie średnie (10s)
SolarData solarFast = getSolarFastAverage();
I2CSensorData i2cFast = getI2CFastAverage();

// Wolne średnie (5min)
SolarData solarSlow = getSolarSlowAverage();
I2CSensorData i2cSlow = getI2CSlowAverage();
```

### Kontrola przez Modbus
```python
# Przełącz na średnie szybkie
client.write_register(90, 1)

# Przełącz na średnie wolne
client.write_register(90, 2)

# Wróć do danych aktualnych
client.write_register(90, 0)
```

## Komendy Serial

Dodano nowe komendy serial:
- `STATUS` - wyświetla status systemu z informacjami o średnich
- `AVGSTATUS` - wyświetla szczegółowy status buforów średnich

## Testing

### Użycie Skryptu Testowego
```bash
# Podstawowy test
python test_moving_averages.py --port COM3

# Test systemu średnich kroczących
python test_moving_averages.py --test-averages

# Monitoring ciągły
python test_moving_averages.py --monitor --interval 10

# Ustawienie typu danych
python test_moving_averages.py --datatype 1  # średnie szybkie
python test_moving_averages.py --datatype 2  # średnie wolne

# Wysłanie komendy
python test_moving_averages.py --command print_avg_status
```

## Konfiguracja Pamięci

### Rozmiary Buforów
- `FAST_BUFFER_SIZE`: 30 próbek (~10s przy próbkowaniu co 300ms)
- `SLOW_BUFFER_SIZE`: 100 próbek (~5min przy próbkowaniu co 3s)

### Optymalizacja
- Statyczne alokacje pamięci (brak malloc/free)
- Template specialization dla efektywności
- Circular buffer prevents memory fragmentation
- Automatic timestamp-based data expiration

## Architektura

### MovingAverageManager Class
- Centralny manager dla wszystkich buforów
- Automatic update scheduling
- Thread-safe operations (używane z poziomu głównej pętli)

### Template Specializations
Każdy typ czujnika ma dedykowane specjalizacje dla:
- `addWeighted()` - dodawanie z wagą
- `addSimple()` - proste dodawanie
- `divideByWeight()` - normalizacja przez wagę
- `divideByCount()` - normalizacja przez liczbę próbek

## Wydajność

### Zużycie Pamięci
- ~15KB RAM dla wszystkich buforów
- Brak dynamicznych alokacji
- Compact data structures

### CPU Usage
- Minimalne obciążenie CPU
- Batch processing co 5s/30s
- Efficient circular buffer operations
- Exponential weighting calculation optimized

## Korzyści

1. **Redukcja Szumu**: Średnie kroczące eliminują krótkoterminowe fluktuacje
2. **Stabilność Danych**: Szczególnie ważne dla komunikacji Modbus
3. **Flexibilność**: Klient może wybierać typ danych w runtime
4. **Skalowalność**: Łatwe dodawanie nowych typów czujników
5. **Efektywność**: Optimized dla embedded systems
6. **Niezawodność**: Automatic error handling i data validation

## Przyszłe Rozszerzenia

Możliwe ulepszenia:
- Konfigurowalny rozmiar buforów przez Modbus
- Dodatkowe okresy średnich (1min, 15min)
- Persistent storage średnich po restarcie
- Adaptive weighting based na jakości danych
- Statistical measures (std dev, min/max)

## Uwagi Implementacyjne

- System działa tylko gdy `config.enableModbus = true`
- Bufory są inicjalizowane przy starcie systemu
- Dane są automatycznie ekspirowane po przekroczeniu wieku
- Template specializations zapewniają type safety
- Wszystkie operacje są non-blocking 