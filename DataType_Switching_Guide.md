# Przewodnik Przełączania Typów Danych - ESP32 Sensor Cube

## Przegląd
System ESP32 Sensor Cube obsługuje trzy typy danych:
- **Current (0)**: Aktualne odczyty z czujników
- **Fast Average (1)**: Średnie kroczące 10-sekundowe z ważeniem eksponencjalnym
- **Slow Average (2)**: Średnie kroczące 5-minutowe z ważeniem eksponencjalnym

## Metody Przełączania

### 1. Komendy Serial
Połącz się przez port szeregowy (115200 baud) i wyślij:

```
DATATYPE_CURRENT     # Przełącz na dane aktualne
DATATYPE_FAST        # Przełącz na średnie 10s
DATATYPE_SLOW        # Przełącz na średnie 5min
DATATYPE_CYCLE       # Przełącz na następny typ (cyklicznie)
DATATYPE_STATUS      # Pokaż aktualny typ danych
```

### 2. Modbus Registers
#### Rejestr 90 - Data Type Selector
```
0 = Current readings
1 = Fast averages (10s)
2 = Slow averages (5min)
```

Przykład Python Modbus:
```python
# Przełącz na średnie szybkie
client.write_register(90, 1)

# Przełącz na średnie wolne  
client.write_register(90, 2)

# Wróć do danych aktualnych
client.write_register(90, 0)
```

#### Rejestr 91 - System Commands
```
7  = Switch to current data
8  = Switch to fast average (10s)
9  = Switch to slow average (5min)
11 = Cycle through data types
```

### 3. Python Test Script
```bash
# Ustaw typ danych
python test_moving_averages.py --datatype 1    # Fast averages
python test_moving_averages.py --datatype 2    # Slow averages
python test_moving_averages.py --datatype 0    # Current data

# Wyślij komendę przełączania
python test_moving_averages.py --command switch_to_fast_avg
python test_moving_averages.py --command cycle_data_type

# Demo przełączania
python test_moving_averages.py --demo-switching

# Test kompletnego systemu
python test_moving_averages.py --test-averages
```

## API Functions (C++)

### Include Header
```cpp
#include <modbus_handler.h>
```

### Funkcje Dostępne
```cpp
// Ustaw typ danych (zwraca true jeśli sukces)
bool setCurrentDataType(DataType newType);

// Pobierz aktualny typ danych
DataType getCurrentDataType();

// Pobierz nazwę aktualnego typu danych
String getCurrentDataTypeName();

// Przełącz na następny typ (cyklicznie)
void cycleDataType();
```

### Przykład Użycia
```cpp
// Przełącz na średnie szybkie
if (setCurrentDataType(DATA_FAST_AVG)) {
    Serial.println("Switched to fast averages");
}

// Sprawdź aktualny typ
DataType current = getCurrentDataType();
Serial.println("Current type: " + getCurrentDataTypeName());

// Przełączanie cykliczne
cycleDataType();  // Current -> Fast -> Slow -> Current -> ...
```

## Automatyczne Przełączanie

Gdy ustawisz typ danych, **wszystkie** czujniki automatycznie zaczynają używać wybranego typu:

- **Solar Sensor** - napięcie, prąd, moc
- **I2C Sensors** - temperatura, wilgotność, ciśnienie, CO2
- **SPS30** - wszystkie pomiary PM i NC
- **IPS Sensor** - wszystkie kanały PC i PM
- **MCP3424** - wszystkie kanały ADC 
- **ADS1110** - pomiary napięcia
- **INA219** - prąd, napięcie, moc

## Monitoring Statusu

### Serial Commands
```
STATUS       # Pełny status systemu + typ danych
AVGSTATUS    # Status buforów średnich kroczących
DATATYPE_STATUS  # Tylko aktualny typ danych
```

### Modbus Registers
```
Reg 90: Aktualny typ danych (0-2)
Reg 93: Status systemu średnich (1=aktywny)
```

### Python Script
```bash
# Podstawowy status
python test_moving_averages.py

# Monitoring ciągły z typem danych
python test_moving_averages.py --monitor --interval 5
```

## Wskazówki Użytkowania

### Kiedy Używać Poszczególnych Typów:

1. **Current Data (0)**:
   - Szybka diagnostyka
   - Analiza w czasie rzeczywistym
   - Debugging czujników

2. **Fast Average (1) - 10s**:
   - Stabilne odczyty bez krótkoterminowych skoków
   - Komunikacja Modbus z klientami wymagającymi stabilności
   - Real-time monitoring z redukcją szumu

3. **Slow Average (2) - 5min**:
   - Trendy długoterminowe
   - Logging danych
   - Analiza środowiskowa
   - Systemy alarmowe

### Przykładowe Scenariusze:

```bash
# Szybka diagnostyka problemu z czujnikiem
python test_moving_averages.py --datatype 0
python test_moving_averages.py --monitor --interval 2

# Stabilny monitoring dla systemu SCADA
python test_moving_averages.py --datatype 1

# Długoterminowa analiza jakości powietrza  
python test_moving_averages.py --datatype 2
python test_moving_averages.py --monitor --interval 60
```

## Rozwiązywanie Problemów

### Problem: Przełączanie nie działa
```bash
# Sprawdź status Modbus
MODBUS_STATUS

# Sprawdź czy Modbus jest włączony
CONFIG_MODBUS_ON
```

### Problem: Średnie są zerowe
- System potrzebuje czasu na zebranie próbek
- Fast averages: ~10 sekund
- Slow averages: ~5 minut
- Sprawdź czy czujniki są aktywne: `STATUS`

### Problem: Brak różnicy między typami
- Sprawdź czy dane rzeczywiście się zmieniają
- Użyj `--demo-switching` do porównania
- Sprawdź status buforów: `AVGSTATUS`

## Integracja z Innymi Systemami

### SCADA/HMI Systems
```python
# Ustaw stabilne dane dla SCADA
client.write_register(90, 1)  # Fast averages
```

### Data Logging
```python
# Dla długoterminowych trendów
client.write_register(90, 2)  # Slow averages
```

### Real-time Monitoring
```python
# Dla szybkich alertów
client.write_register(90, 0)  # Current data
``` 