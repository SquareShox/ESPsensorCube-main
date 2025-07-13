# ESP32 Multi-Sensor System

Uporządkowany system wieloczujnikowy dla ESP32 z obsługą czujników solarnych, OPCN3, I2C oraz komunikacją Modbus i Wi-Fi.

## Struktura Projektu

```
src/
├── main.cpp              # Główny plik programu
├── config.h              # Konfiguracja systemu i definicje
├── sensors.h             # Deklaracje funkcji obsługi czujników
├── sensors.cpp           # Implementacja obsługi czujników
├── modbus_handler.h      # Deklaracje obsługi Modbus
├── modbus_handler.cpp    # Implementacja obsługi Modbus
├── web_server.h          # Deklaracje serwera web
├── web_server.cpp        # Implementacja serwera web
└── html.h                # Kod HTML dla interfejsu web
```

## Funkcje Systemu

### Obsługa Czujników
- **Czujnik Solar**: Odczyt danych po MySerial (piny 6/5)
- **OPCN3**: Czujnik cząstek PM (pin SS: 10)
- **IPS**: Czujnik cząstek I2C (adres 0x4B, pin power: 1)
- **I2C**: Obsługa czujników SHT30, BME280, SCD40 (piny 4/5 z pull-up)
- **Serial Sensors**: Konfigurowalnie do 4 dodatkowych czujników

### Komunikacja
- **Modbus RTU**: Slave na Serial2 (piny 44/43)
- **Wi-Fi**: Połączenie z możliwością automatycznego przywracania
- **Web Server**: API REST i WebSerial
- **OTA Updates**: Aktualizacje przez web interface

### Konfiguracja Runtime

System pozwala na dynamiczne włączanie/wyłączanie funkcji:

```cpp
// Przykład konfiguracji
config.enableSolarSensor = true;
config.enableOPCN3Sensor = true;
config.enableI2CSensors = true;
config.enableIPS = true;
config.enableModbus = true;
config.enableWebServer = true;
config.enableWiFi = true;
config.autoReset = true;
```

## Komendy Serial

### Podstawowe Komendy
- `SEND` - Wyświetl dane ze wszystkich czujników
- `STATUS` - Status systemu i czujników
- `RESTART` - Restart systemu

### Komendy Konfiguracyjne
- `CONFIG_SOLAR_ON/OFF` - Włącz/wyłącz czujnik solar
- `CONFIG_OPCN3_ON/OFF` - Włącz/wyłącz czujnik OPCN3
- `CONFIG_I2C_ON/OFF` - Włącz/wyłącz wszystkie czujniki I2C
- `CONFIG_SHT30_ON/OFF` - Włącz/wyłącz czujnik SHT30
- `CONFIG_BME280_ON/OFF` - Włącz/wyłącz czujnik BME280
- `CONFIG_SCD40_ON/OFF` - Włącz/wyłącz czujnik SCD40
- `CONFIG_IPS_ON/OFF` - Włącz/wyłącz czujnik IPS
- `CONFIG_MODBUS_ON/OFF` - Włącz/wyłącz komunikację Modbus
- `CONFIG_AUTO_RESET_ON/OFF` - Włącz/wyłącz automatyczny restart

## API Web

### Endpoints Status
- `GET /api/status` - Status systemu
- `GET /api/solar` - Dane czujnika solar
- `GET /api/opcn3` - Dane czujnika OPCN3
- `GET /api/i2c` - Dane czujników I2C

### Endpoints Konfiguracji
- `POST /api/config/solar` - Konfiguracja czujnika solar
- `POST /api/config/opcn3` - Konfiguracja czujnika OPCN3
- `POST /api/config/i2c` - Konfiguracja czujników I2C
- `POST /api/config/modbus` - Konfiguracja Modbus

### System Control
- `POST /api/system/restart` - Restart systemu

## Modbus Registers

### Solar Data (Registers 0-39)
- 0: Status flag (1 = valid data)
- 1: Device ID
- 2: Data length
- 3: Timestamp
- 4-22: Solar panel data (PID, FW, V, I, VPV, PPV, etc.)

### OPCN3 Data (Registers 40-89)
- 40: Status flag
- 41: Device ID (2)
- 42: Data length
- 43: Timestamp
- 44-45: Temperature & Humidity
- 46-69: Bin counts (24 bins)
- 77-79: PM values (PM1, PM2.5, PM10)

### I2C Data (Registers 90-95)
- 90: Status flag
- 91: Temperature
- 92: Humidity
- 93: Pressure
- 94: CO2
- 95: Sensor type

### IPS Data (Registers 96-115)
- 96: Status flag (1=OK, 0=Error)
- 97: Device ID (4)
- 98: Data length
- 99: Timestamp
- 100-113: PC Values (7 wartości, każda jako 2 rejestry 32-bit)
  - 100-101: PC 0.1μm (32-bit)
  - 102-103: PC 0.3μm (32-bit)
  - 104-105: PC 0.5μm (32-bit)
  - 106-107: PC 1.0μm (32-bit)
  - 108-109: PC 2.5μm (32-bit)
  - 110-111: PC 5.0μm (32-bit)
  - 112-113: PC 10μm (32-bit)
- 114-120: PM Values (masa cząstek *1000)
  - 114: PM 0.1μm (*1000)
  - 115: PM 0.3μm (*1000)
  - 116: PM 0.5μm (*1000)
  - 117: PM 1.0μm (*1000)
  - 118: PM 2.5μm (*1000)
  - 119: PM 5.0μm (*1000)
  - 120: PM 10μm (*1000)

## Dodawanie Nowych Czujników

### I2C Sensors
1. Dodaj nowy typ w `enum I2CSensorType` w `config.h`
2. Zaimplementuj funkcję odczytu w `sensors.cpp`
3. Dodaj wykrywanie w `initializeI2CSensor()`

### Serial Sensors
1. Użyj `configureSerialSensor()` do konfiguracji portu
2. Zaimplementuj parsing w `readSerialSensors()`
3. Dodaj obsługę protokołu (NMEA, JSON, CSV, CUSTOM)

## Konfiguracja Pinów

```cpp
// Główne piny
#define WS2812_PIN 21          // LED status
#define SOLAR_RX_PIN 6         // Solar sensor RX
#define SOLAR_TX_PIN 5         // Solar sensor TX
#define MODBUS_RX_PIN 44       // Modbus RX
#define MODBUS_TX_PIN 43       // Modbus TX
#define I2C_SDA_PIN 2          // I2C SDA (z pull-up)
#define I2C_SCL_PIN 4          // I2C SCL (z pull-up)
#define OPCN3_SS_PIN 10        // OPCN3 Slave Select
#define IPS_POWER_PIN 1        // IPS Power Control (output to GND)
#define IPS_RX_PIN 9           // IPS Serial1 RX
#define IPS_TX_PIN 10          // IPS Serial1 TX
```

## Status LED

- **Biały**: Inicjalizacja
- **Zielony**: Wszystkie czujniki OK
- **Żółty**: Częściowy sukces czujników
- **Czerwony**: Błąd czujników

## Kompilacja

Wymagane biblioteki:
- ESP32 Core
- WiFi
- ESPAsyncWebServer
- ModbusSerial
- OPCN3
- Adafruit_NeoPixel
- WebSerial

## Użycie

1. Skonfiguruj parametry Wi-Fi w `config.h`
2. Wybierz które czujniki mają być aktywne
3. Uploaduj kod na ESP32
4. Monitoruj przez Serial Monitor lub Web Interface
5. Czytaj dane przez Modbus lub API

## Uwagi

- System automatycznie wykrywa czujniki I2C
- Pull-up resistory są włączane programowo dla I2C
- Watchdog chroni przed zawieszeniem systemu
- Wszystkie funkcje można włączać/wyłączać bez rekompilacji

## Modbus RTU

### Rejestr Map
**Solar Data (0-39)**
- 0: Status (1=OK, 0=Error)
- 1: Device ID (1)
- 2: Data length in bits
- 3: Timestamp (16-bit)
- 4-22: Solar parameters (PID, FW, SER#, V, I, VPV, PPV, CS, MPPT, OR, ERR, LOAD, IL, H19-H23, HSDS)

**OPCN3 Data (40-89)**
- 40: Status (1=OK, 0=Error) 
- 41: Device ID (2)
- 42: Data length in bits
- 43: Timestamp (16-bit)
- 44: Temperature (*100)
- 45: Humidity (*100)
- **46-69: Bin Counts (24 rejestry)** - **NAJWAŻNIEJSZE DANE!**
  - Każdy rejestr to liczba cząstek w określonym zakresie rozmiarów
  - Bin 0 (reg 46): 0.38-0.54 μm
  - Bin 1 (reg 47): 0.54-0.78 μm
  - Bin 2 (reg 48): 0.78-1.05 μm
  - ...kolejne zakresy aż do Bin 23 (reg 69)
- 70-73: Time to cross values
- 74: Sensor status
- 75: Sample flow rate (*100)
- 76: Sampling period
- 77: PM1 (*100)
- 78: PM2.5 (*100)
- 79: PM10 (*100)

**I2C Sensors (90-95)**
- 90: Status (1=OK, 0=Error)
- 91: Temperature (*100)
- 92: Humidity (*100)
- 93: Pressure (*10)
- 94: CO2 level
- 95: Sensor type (1=SHT30, 2=BME280, 3=SCD40)

### Modbus Commands
- Register 91: Command register
  - 1: Request fresh OPCN3 reading
  - 2: System restart
  - 3: Toggle auto reset
  - 4: Request fresh IPS reading
- Register 92: Status responses 

## Przykłady Użycia

### Sprawdzanie Statusu Systemu
```
STATUS
```
Wynik:
```
=== System Status ===
Solar Sensor: OK
OPCN3 Sensor: OK  
I2C Overall: OK
  - SHT30: OK
  - BME280: DISABLED
  - SCD40: ERROR
WiFi: CONNECTED
Free Heap: 234567
Uptime: 1234 seconds
```

### Sterowanie Czujnikami I2C
```
CONFIG_SHT30_OFF        # Wyłącz tylko SHT30
CONFIG_BME280_ON        # Włącz tylko BME280
CONFIG_I2C_OFF          # Wyłącz wszystkie czujniki I2C
CONFIG_IPS_ON           # Włącz czujnik IPS
CONFIG_IPS_OFF          # Wyłącz czujnik IPS
```

### Monitorowanie Bin Counts OPCN3
Najważniejsze dane OPCN3 (bin counts) są dostępne w rejestrach Modbus 46-69.
Każdy bin reprezentuje liczbę cząstek w określonym zakresie mikrometrów:
- Bin 0: 0.38-0.54 μm  
- Bin 1: 0.54-0.78 μm
- Bin 23: największe cząstki

### Odczyt Danych przez Serial
```
SEND
```
Wynik:
```
=== Sensor Data ===
SOLAR_V 12.34
SOLAR_I 1.23
OPCN3: Temp=23.5°C, Hum=45.2%, PM1=5, PM2.5=12, PM10=18
I2C Sensor: Temp=24.1°C, Hum=48.3%
IPS Sensor PC: 1234,567,89,45,23,12,5 PM: 1.234,0.567,0.089,0.045,0.023,0.012,0.005
``` 