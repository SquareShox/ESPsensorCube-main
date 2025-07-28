# WebSocket System - ESP Sensor Cube

## Przeglad

System WebSocket dla ESP Sensor Cube umozliwia:
- 📊 **Czytanie danych sensorów** w czasie rzeczywistym
- 📈 **Pobieranie historii danych** z PSRAM (fast/slow samples)
- ⚙️ **Sterowanie konfiguracją** systemu
- 🔧 **Komendy systemowe** (restart, status, pamięć)
- 🔬 **Kontrola kalibracji** sensorów gazowych
- 📡 **Broadcast danych** do wszystkich klientów
- 📊 **Monitorowanie historii** (zużycie pamięci, liczba próbek)
- 🌀 **Sterowanie wentylatorem** (prędkość, włącz/wyłącz)
- ⚡ **Sterowanie GLine** (router power control)
- 😴 **Tryb sleep** (czasowe wyłączenie GLine z opóźnieniem)

## Struktura plików

```
src/
├── web_socket.cpp          # Implementacja WebSocket
└── web_server.cpp          # Integracja z serwerem WWW

include/
└── web_socket.h            # Deklaracje funkcji WebSocket

test_websocket.py           # Test WebSocket w Pythonie
websocket_test.html         # Interfejs testowy w przegladarce
```

## Instalacja i uruchomienie

### 1. Kompilacja ESP32

```bash
# W PlatformIO
pio run --target upload

# Lub w Arduino IDE
# Wgraj kod na ESP32
```

### 2. Konfiguracja WiFi

Upewnij sie, ze w `include/config.h` sa poprawne dane WiFi:

```cpp
#define WIFI_SSID "Twoja_Siec_WiFi"
#define WIFI_PASSWORD "Twoje_Haslo"
```

### 3. Konfiguracja serwera

**Aktualne ustawienia:**
- **Port HTTP**: 81 (zmieniony z domyślnego 80)
- **Endpoint WebSocket**: `/ws`
- **Protokół**: HTTP/WebSocket

**Modyfikacja portu:**
```cpp
// W src/web_server.cpp, linia 20
AsyncWebServer server(81);  // Aktualnie ustawiony na port 81
```

### 3. Sprawdzenie IP i portu

Po uruchomieniu ESP32 sprawdz w Serial Monitor:

```
Connected to WiFi
IP Address: 192.168.1.100
Web server started
WebSocket initialized and ready
```

**WebSocket działa na:**
- **Port HTTP**: 81 (zmieniony z 80)
- **Endpoint WebSocket**: `/ws`
- **Pełny adres**: `ws://192.168.1.100:81/ws`
- **Protokół**: WebSocket (ws:// lub wss://)

**Dostępne endpointy HTTP:**
- `http://192.168.1.100:81/` - Strona główna (firmware update)
- `http://192.168.1.100:81/dashboard` - Dashboard z danymi sensorów
- `http://192.168.1.100:81/charts` - Wykresy historyczne
- `http://192.168.1.100:81/test` - Test WebSocket (liczba klientów)
- `http://192.168.1.100:81/api/history` - API historii danych

## Testowanie

### Opcja 1: Test Python (zalecane)

```bash
# Instalacja zaleznosci
pip install websockets

# Uruchomienie testu
python test_websocket.py 192.168.1.100:81
```

**Wynik testu:**
```
🚀 Rozpoczęcie testów WebSocket ESP Sensor Cube
📍 Adres: ws://192.168.1.100/ws
✅ Połączono z ws://192.168.1.100/ws

🔍 Test: Status systemu
✅ Uptime: 3600s
✅ Free Heap: 150000 bytes
✅ WiFi RSSI: -45 dBm
✅ WiFi Connected: True
📊 Status sensorów:
   ✅ sht40: True
   ✅ sps30: True
   ✅ co2: True
   ❌ solar: False
```

### Opcja 2: Test w przegladarce

1. Otworz plik `websocket_test.html` w przegladarce
2. Wpisz adres IP ESP32: `ws://192.168.1.100/ws`
3. Kliknij "Połącz"
4. Uzyj przyciskow do testowania komend

### Opcja 3: Test JavaScript

```javascript
const ws = new WebSocket('ws://192.168.1.100:81/ws');

ws.onopen = function() {
    console.log('Połączono');
    
    // Pobierz status
    ws.send(JSON.stringify({cmd: 'status'}));
    
    // Pobierz dane SHT40
    ws.send(JSON.stringify({
        cmd: 'getSensorData',
        sensor: 'sht40'
    }));
};

ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    console.log('Otrzymano:', data);
};
```

## Komendy WebSocket

### Podstawowe komendy

| Komenda | Opis | Przykład |
|---------|------|----------|
| `status` | Status systemu | `{"cmd": "status"}` |
| `getSensorData` | Dane sensorów | `{"cmd": "getSensorData", "sensor": "sht40"}` |
| `getHistory` | Historia danych | `{"cmd": "getHistory", "sensor": "sht40", "timeRange": "1h", "sampleType": "fast"}` |
| `getHistoryInfo` | Informacje o historii | `{"cmd": "getHistoryInfo"}` |
| `getAverages` | Średnie uśrednione | `{"cmd": "getAverages", "sensor": "sht40", "type": "fast"}` |
| `getSensorKeys` | Struktura JSON z kluczami | `{"cmd": "getSensorKeys"}` |

### Komendy systemowe

| Komenda | Opis | Przykład |
|---------|------|----------|
| `getConfig` | Pobierz konfigurację | `{"cmd": "getConfig"}` |
| `setConfig` | Ustaw konfigurację | `{"cmd": "setConfig", "enableSPS30": true}` |
| `system` | Komendy systemowe | `{"cmd": "system", "command": "memory"}` |
| `calibration` | Kontrola kalibracji | `{"cmd": "calibration", "command": "status"}` |

**Komendy wentylatora i GLine:**
| Komenda | Opis | Przykład |
|---------|------|----------|
| `fan_on` | Włącz wentylator | `{"cmd": "system", "command": "fan_on"}` |
| `fan_off` | Wyłącz wentylator | `{"cmd": "system", "command": "fan_off"}` |
| `fan_speed` | Ustaw prędkość | `{"cmd": "system", "command": "fan_speed", "value": 75}` |
| `gline_on` | Włącz GLine | `{"cmd": "system", "command": "gline_on"}` |
| `gline_off` | Wyłącz GLine | `{"cmd": "system", "command": "gline_off"}` |
| `sleep` | Tryb sleep | `{"cmd": "system", "command": "sleep", "delay": 60, "duration": 300}` |
| `sleep_stop` | Zatrzymaj sleep | `{"cmd": "system", "command": "sleep_stop"}` |
| `fan_status` | Status wentylatora | `{"cmd": "system", "command": "fan_status"}` |

## Szczegółowy przewodnik komend

### 1. Status systemu
```json
{
    "cmd": "status"
}
```

**Odpowiedź:**
```json
{
    "cmd": "status",
    "uptime": 3600,
    "freeHeap": 150000,
    "freePsram": 500000,
    "wifiRSSI": -45,
    "wifiConnected": true,
    "sensors": {
        "solar": false,
        "opcn3": false,
        "i2c": true,
        "sps30": true,
        "sht40": true,
        "co2": true,
        "mcp3424": true,
        "ads1110": true,
        "ina219": true,
        "hcho": true,
        "ips": false
    },
    "config": {
        "enableWiFi": true,
        "enableWebServer": true,
        "enableHistory": true,
        "enableModbus": true
    }
}
```

### 2. Pobieranie danych sensorów

#### Wszystkie sensory:
```json
{
    "cmd": "getSensorData",
    "sensor": "all"
}
```

#### Pojedynczy sensor:
```json
{
    "cmd": "getSensorData",
    "sensor": "sht40"
}
```

**Dostępne sensory:**
- `solar` - dane solarne
- `sht40` - temperatura, wilgotność, ciśnienie
- `sps30` - cząsteczki PM1.0, PM2.5, PM4.0, PM10
- `co2` - dwutlenek węgla (CO2)
- `power` - napięcie, prąd, moc (INA219)
- `hcho` - formaldehyd (mg/m³, ppb)
- `ips` - cząsteczki IPS (PC, PM, NP, PW arrays)
- `mcp3424` - napięcia MCP3424 (K1_1_mV, K1_1_V, etc.)
- `fan` - sterowanie wentylatorem (PWM, RPM, GLine)
- `calibration` - dane skalibrowane (temperatury, napięcia, gazy, TGS, specjalne)
- `all` - wszystkie dostępne sensory

### 3. Historia danych

```json
{
    "cmd": "getHistory",
    "sensor": "sht40",
    "timeRange": "1h",
    "sampleType": "fast"
}
```

**Opcje timeRange:**
- `1h` - ostatnia godzina
- `6h` - ostatnie 6 godzin
- `24h` - ostatnie 24 godziny

**Opcje sampleType:**
- `fast` - szybkie próbki (10 sekund) - domyślne
- `slow` - wolne próbki (5 minut)

**Lub z określonymi timestampami (epoch milliseconds):**
```json
{
    "cmd": "getHistory",
    "sensor": "sht40",
    "fromTime": 1700000000000,
    "toTime": 1700003600000,
    "sampleType": "slow"
}
```

**Konwersja czasu do epoch milliseconds:**
```javascript
// Przykład: Pobierz dane z ostatnich 2 godzin
const now = Date.now(); // Aktualny czas w ms
const twoHoursAgo = now - (2 * 60 * 60 * 1000); // 2 godziny wstecz

// Przykład: Pobierz dane z konkretnego dnia
const specificDate = new Date('2024-01-15T10:00:00Z').getTime(); // 15 stycznia 2024, 10:00 UTC
const endDate = specificDate + (24 * 60 * 60 * 1000); // +24 godziny

// Przykład: Pobierz dane z ostatnich 30 minut
const thirtyMinutesAgo = now - (30 * 60 * 1000); // 30 minut wstecz
```

**Przykłady timestampów:**
- `1700000000000` = 13 listopada 2023, 12:00:00 UTC
- `1704067200000` = 1 stycznia 2024, 00:00:00 UTC  
- `1704067200000` = 1 stycznia 2024, 12:00:00 UTC
- `1704153600000` = 2 stycznia 2024, 00:00:00 UTC
- `Date.now()` = aktualny czas w ms

**Przykłady użycia:**
```json
// Szybkie próbki z ostatniej godziny
{
    "cmd": "getHistory",
    "sensor": "sps30",
    "timeRange": "1h",
    "sampleType": "fast"
}

// Wolne próbki z ostatnich 24 godzin
{
    "cmd": "getHistory",
    "sensor": "sht40",
    "timeRange": "24h",
    "sampleType": "slow"
}

// Historia danych kalibracji
{
    "cmd": "getHistory",
    "sensor": "calibration",
    "timeRange": "6h",
    "sampleType": "fast"
}

// Domyślnie fast (można pominąć sampleType)
{
    "cmd": "getHistory",
    "sensor": "sht40",
    "timeRange": "6h"
}

// Z konkretnymi timestampami - ostatnie 2 godziny
{
    "cmd": "getHistory",
    "sensor": "sht40",
    "fromTime": 1704067200000,
    "toTime": 1704074400000,
    "sampleType": "fast"
}

// Z konkretnymi timestampami - konkretny dzień
{
    "cmd": "getHistory",
    "sensor": "sps30",
    "fromTime": 1704067200000,
    "toTime": 1704153600000,
    "sampleType": "slow"
}
```

### 4. Informacje o historii

```json
{
    "cmd": "getHistoryInfo"
}
```

**Odpowiedź:**
```json
{
    "cmd": "historyInfo",
    "enabled": true,
    "initialized": true,
    "totalMemoryUsed": 524288,
    "memoryBudget": 1048576,
    "memoryPercent": 50,
    "sensors": {
        "sht40": {
            "fastSamples": 360,
            "slowSamples": 288
        },
        "sps30": {
            "fastSamples": 300,
            "slowSamples": 240
        },
        "power": {
            "fastSamples": 360,
            "slowSamples": 288
        }
    }
}
```

### 5. Średnie uśrednione

```json
{
    "cmd": "getAverages",
    "sensor": "sht40",
    "type": "fast"
}
```

**Opcje:**
- `sensor`: `solar`, `i2c`, `sps30`, `power`, `hcho`, `ips`, `mcp3424`, `all`
- `type`: `fast` (10s) lub `slow` (5min)

### 6. Struktura JSON z kluczami

#### Pobieranie struktury kluczy:
```json
{
    "cmd": "getSensorKeys"
}
```

**Odpowiedź:**
```json
{
    "cmd": "sensorKeys",
    "success": true,
    "t": 1700000000000,
    "time": "14:30:25",
    "date": "15/12/2024",
    "data": {
        "sht40": {
            "valid": "SHT40_VALID",
            "temperature": "SHT40_TEMPERATURE",
            "humidity": "SHT40_HUMIDITY",
            "pressure": "SHT40_PRESSURE"
        },
        "sps30": {
            "valid": "SPS30_VALID",
            "PM1": "SPS30_PM1",
            "PM25": "SPS30_PM25",
            "PM4": "SPS30_PM4",
            "PM10": "SPS30_PM10",
            "NC05": "SPS30_NC05",
            "NC1": "SPS30_NC1",
            "NC25": "SPS30_NC25",
            "NC4": "SPS30_NC4",
            "NC10": "SPS30_NC10",
            "TPS": "SPS30_TPS"
        },
        "co2": {
            "valid": "CO2_VALID",
            "co2": "CO2_VALUE"
        },
        "power": {
            "valid": "POWER_VALID",
            "busVoltage": "POWER_BUS_VOLTAGE",
            "shuntVoltage": "POWER_SHUNT_VOLTAGE",
            "current": "POWER_CURRENT",
            "power": "POWER_POWER"
        },
        "hcho": {
            "valid": "HCHO_VALID",
            "hcho_mg": "HCHO_MG",
            "hcho_ppb": "HCHO_PPB"
        },
        "ips": {
            "valid": "IPS_VALID",
            "debugMode": "IPS_DEBUG_MODE",
            "won": "IPS_WON",
            "pc": ["IPS_PC_1", "IPS_PC_2", "IPS_PC_3", "IPS_PC_4", "IPS_PC_5", "IPS_PC_6", "IPS_PC_7"],
            "pm": ["IPS_PM_1", "IPS_PM_2", "IPS_PM_3", "IPS_PM_4", "IPS_PM_5", "IPS_PM_6", "IPS_PM_7"],
            "np": ["IPS_NP_1", "IPS_NP_2", "IPS_NP_3", "IPS_NP_4", "IPS_NP_5", "IPS_NP_6", "IPS_NP_7"],
            "pw": ["IPS_PW_1", "IPS_PW_2", "IPS_PW_3", "IPS_PW_4", "IPS_PW_5", "IPS_PW_6", "IPS_PW_7"]
        },
        "mcp3424": {
            "enabled": "MCP3424_ENABLED",
            "deviceCount": "MCP3424_DEVICE_COUNT",
            "valid": "MCP3424_VALID",
            "devices": [
                {
                    "address": "MCP3424_DEVICE_1_ADDRESS",
                    "valid": "MCP3424_DEVICE_1_VALID",
                    "resolution": "MCP3424_DEVICE_1_RESOLUTION",
                    "gain": "MCP3424_DEVICE_1_GAIN",
                    "channels": {
                        "K1_1_mV": "K1_1_mV",
                        "K1_1_V": "K1_1_V",
                        "K1_2_mV": "K1_2_mV",
                        "K1_2_V": "K1_2_V",
                        "K1_3_mV": "K1_3_mV",
                        "K1_3_V": "K1_3_V",
                        "K1_4_mV": "K1_4_mV",
                        "K1_4_V": "K1_4_V"
                    }
                },
                {
                    "address": "MCP3424_DEVICE_2_ADDRESS",
                    "valid": "MCP3424_DEVICE_2_VALID",
                    "resolution": "MCP3424_DEVICE_2_RESOLUTION",
                    "gain": "MCP3424_DEVICE_2_GAIN",
                    "channels": {
                        "K2_1_mV": "K2_1_mV",
                        "K2_1_V": "K2_1_V",
                        "K2_2_mV": "K2_2_mV",
                        "K2_2_V": "K2_2_V",
                        "K2_3_mV": "K2_3_mV",
                        "K2_3_V": "K2_3_V",
                        "K2_4_mV": "K2_4_mV",
                        "K2_4_V": "K2_4_V"
                    }
                }
            ]
        },
        "fan": {
            "enabled": "FAN_ENABLED",
            "dutyCycle": "FAN_DUTY_CYCLE",
            "rpm": "FAN_RPM",
            "glineEnabled": "FAN_GLINE_ENABLED",
            "pwmValue": "FAN_PWM_VALUE",
            "valid": "FAN_VALID"
        }
    }
}
```

**Zastosowanie:**
- **Mapowanie kluczy** - identyfikacja dostępnych pól w JSON
- **Generowanie szablonów** - tworzenie struktur danych
- **Walidacja** - sprawdzanie poprawności kluczy
- **Dokumentacja** - opis struktury danych sensorów

**Klucze MCP3424:**
- **Format**: `K{device}_{channel}_{unit}`
- **Device**: 1-8 (numer urządzenia)
- **Channel**: 1-4 (numer kanału)
- **Unit**: `mV` (miliwolty) lub `V` (wolty)

**Przykłady kluczy MCP3424:**
- `K1_1_mV` - Device 1, Channel 1, miliwolty
- `K1_1_V` - Device 1, Channel 1, wolty
- `K2_3_mV` - Device 2, Channel 3, miliwolty
- `K8_4_V` - Device 8, Channel 4, wolty

### 7. Konfiguracja systemu

#### Pobieranie konfiguracji:
```json
{
    "cmd": "getConfig"
}
```

#### Ustawianie konfiguracji:
```json
{
    "cmd": "setConfig",
    "enableWiFi": true,
    "enableHistory": true,
    "enableSPS30": true,
    "enableSHT40": true,
    "enableHCHO": true,
    "enableINA219": true
}
```

### 7. Komendy systemowe

#### Restart systemu:
```json
{
    "cmd": "system",
    "command": "restart"
}
```

#### Informacje o pamięci:
```json
{
    "cmd": "system",
    "command": "memory"
}
```

#### Status WiFi:
```json
{
    "cmd": "system",
    "command": "wifi"
}
```

#### Sterowanie wentylatorem:
```json
{
    "cmd": "system",
    "command": "fan_on"
}
```

```json
{
    "cmd": "system",
    "command": "fan_off"
}
```

```json
{
    "cmd": "system",
    "command": "fan_speed",
    "value": 75
}
```

#### Sterowanie GLine:
```json
{
    "cmd": "system",
    "command": "gline_on"
}
```

```json
{
    "cmd": "system",
    "command": "gline_off"
}
```

#### Tryb sleep:
```json
{
    "cmd": "system",
    "command": "sleep",
    "delay": 60,
    "duration": 300
}
```

```json
{
    "cmd": "system",
    "command": "sleep_stop"
}
```

#### Status wentylatora:
```json
{
    "cmd": "system",
    "command": "fan_status"
}
```

#### Informacje o historii:
```json
{
    "cmd": "getHistoryInfo"
}
```

**Odpowiedź:**
```json
{
    "cmd": "historyInfo",
    "enabled": true,
    "initialized": true,
    "totalMemoryUsed": 524288,
    "memoryBudget": 1048576,
    "memoryPercent": 50,
    "sensors": {
        "sht40": {
            "fastSamples": 15,
            "slowSamples": 3
        },
        "sps30": {
            "fastSamples": 12,
            "slowSamples": 2
        },
        "i2c": {
            "fastSamples": 18,
            "slowSamples": 4
        }
    }
}
```

### 8. Kalibracja

#### Status kalibracji:
```json
{
    "cmd": "calibration",
    "command": "status"
}
```

#### Start kalibracji:
```json
{
    "cmd": "calibration",
    "command": "start"
}
```

#### Stop kalibracji:
```json
{
    "cmd": "calibration",
    "command": "stop"
}
```

#### Reset kalibracji:
```json
{
    "cmd": "calibration",
    "command": "reset"
}
```

### 9. Komendy debug

#### Debug historii (dodatkowe informacje w odpowiedzi):
```json
{
    "cmd": "getHistory",
    "sensor": "i2c",
    "timeRange": "1h",
    "sampleType": "fast"
}
```

**Odpowiedź z debug info:**
```json
{
    "cmd": "history",
    "sensor": "i2c",
    "timeRange": "1h",
    "sampleType": "fast",
    "fromTime": 1700000000000,
    "toTime": 1700003600000,
    "samples": 15,
    "debug": {
        "historyEnabled": true,
        "historyInitialized": true,
        "freeHeap": 45678,
        "timeSet": true,
        "currentEpoch": 1700003600,
        "currentMillis": 123456
    },
    "data": [...]
}
```

#### Debug informacje w Serial Monitor:
- `History: I2C history pointer: valid/null`
- `History: I2C history initialized`
- `History: I2C samples found: X`
- `History: I2C total samples processed: X`
- `History: Added I2C fast sample`
- `History: I2C data not valid`

### 10. Sterowanie wentylatorem i GLine

#### Włączenie/wyłączenie wentylatora:
```json
{
    "cmd": "system",
    "command": "fan_on"
}
```

```json
{
    "cmd": "system",
    "command": "fan_off"
}
```

#### Ustawienie prędkości wentylatora (duty cycle 0-100%):
```json
{
    "cmd": "system",
    "command": "fan_speed",
    "value": 75
}
```

#### Włączenie/wyłączenie GLine (router):
```json
{
    "cmd": "system",
    "command": "gline_on"
}
```

```json
{
    "cmd": "system",
    "command": "gline_off"
}
```

#### Tryb sleep - czasowe wyłączenie GLine:
```json
{
    "cmd": "system",
    "command": "sleep",
    "delay": 60,
    "duration": 300
}
```
**Parametry:**
- `delay` - opóźnienie w sekundach przed rozpoczęciem sleep (domyślnie 0)
- `duration` - czas trwania sleep w sekundach

#### Przykłady trybu sleep:
```json
// Wyłącz GLine za 60 sekund na 5 minut
{
    "cmd": "system",
    "command": "sleep",
    "delay": 60,
    "duration": 300
}

// Wyłącz GLine natychmiast na 2 godziny
{
    "cmd": "system",
    "command": "sleep",
    "delay": 0,
    "duration": 7200
}

// Wyłącz GLine za 30 minut na 1 godzinę
{
    "cmd": "system",
    "command": "sleep",
    "delay": 1800,
    "duration": 3600
}
```

#### Zatrzymanie trybu sleep:
```json
{
    "cmd": "system",
    "command": "sleep_stop"
}
```

#### Status wentylatora i GLine:
```json
{
    "cmd": "system",
    "command": "fan_status"
}
```

**Odpowiedź:**
```json
{
    "cmd": "fan_status",
    "success": true,
    "data": {
        "enabled": true,
        "dutyCycle": 75,
        "rpm": 1200,
        "glineEnabled": false,
        "sleepMode": true,
        "sleepStartTime": 1700000000000,
        "sleepDuration": 300000,
        "sleepEndTime": 1700000300000,
        "valid": true
    }
}
```

#### Komendy Serial (dodatkowo):
```
FAN_ON              - Włącz wentylator (50% prędkości)
FAN_OFF             - Wyłącz wentylator
FAN_SPEED_75        - Ustaw prędkość na 75%
GLINE_ON            - Włącz GLine
GLINE_OFF           - Wyłącz GLine
SLEEP_60_300        - Sleep: opóźnienie 60s, trwanie 300s
SLEEP_STOP          - Zatrzymaj tryb sleep
FAN_STATUS          - Pokaż status wentylatora
```

### 11. Przełączanie między danymi aktualnymi a uśrednionymi

#### Włączenie uśrednionych danych (fast averages):
```json
{
    "cmd": "setConfig",
    "useAveragedData": true
}
```

#### Wyłączenie uśrednionych danych (live data):
```json
{
    "cmd": "setConfig",
    "useAveragedData": false
}
```

**Efekt:**
- `useAveragedData: true` - dashboard pokazuje uśrednione dane (10-sekundowe średnie)
- `useAveragedData: false` - dashboard pokazuje aktualne dane z sensorów

**Dotyczy sensorów:**
- Solar (V, I, VPV, PPV)
- SHT40 (temperatura, wilgotność, ciśnienie)
- SPS30 (PM1.0, PM2.5, PM4.0, PM10, NC0.5-NC10, TPS)
- HCHO (formaldehyd)
- Power (napięcie, prąd, moc)
- Calibration (dane skalibrowane)

### 12. Konfiguracja Sieci (LittleFS)

#### Pobranie aktualnej konfiguracji sieci:
```json
{
    "cmd": "system",
    "command": "getNetworkConfig"
}
```

#### Zapisanie konfiguracji WiFi:
```json
{
    "cmd": "setConfig",
    "setWiFiConfig": true,
    "ssid": "MojaSiećWiFi",
    "password": "mojehasło123"
}
```

#### Zapisanie konfiguracji sieci:
```json
{
    "cmd": "setConfig",
    "setNetworkConfig": true,
    "useDHCP": false,
    "staticIP": "192.168.1.100",
    "gateway": "192.168.1.1",
    "subnet": "255.255.255.0",
    "dns1": "8.8.8.8",
    "dns2": "8.8.4.4"
}
```

#### Testowanie połączenia WiFi:
```json
{
    "cmd": "system",
    "command": "testWiFi"
}
```

#### Zastosowanie konfiguracji sieci:
```json
{
    "cmd": "system",
    "command": "applyNetworkConfig"
}
```

#### Resetowanie konfiguracji sieci:
```json
{
    "cmd": "system",
    "command": "resetNetworkConfig"
}
```

**Odpowiedź dla getNetworkConfig:**
```json
{
    "cmd": "networkConfig",
    "success": true,
    "useDHCP": true,
    "staticIP": "192.168.1.100",
    "gateway": "192.168.1.1",
    "subnet": "255.255.255.0",
    "dns1": "8.8.8.8",
    "dns2": "8.8.4.4",
    "configValid": true,
    "currentIP": "192.168.1.50",
    "currentSSID": "MojaSiećWiFi",
    "wifiConnected": true,
    "wifiSignal": -45,
    "wifiSSID": "MojaSiećWiFi",
    "wifiPassword": "mojehasło123"
}
```

**Funkcje:**
- **DHCP/Statyczny IP** - automatyczne lub ręczne ustawienie adresu IP
- **Zapisywanie w LittleFS** - konfiguracja zachowana po restarcie
- **Testowanie połączenia** - sprawdzenie statusu WiFi
- **Reset konfiguracji** - usunięcie wszystkich ustawień
- **Interfejs webowy** - dostęp przez `/network`

### 13. Konfiguracja MCP3424 (Mapowanie Urządzeń)

#### Pobranie aktualnej konfiguracji MCP3424:
```json
{
    "cmd": "system",
    "command": "getMCP3424Config"
}
```

#### Zapisanie konfiguracji MCP3424:
```json
{
    "cmd": "setConfig",
    "setMCP3424Config": true,
    "devices": [
        {
            "deviceIndex": 0,
            "gasType": "NO",
            "description": "NO Sensor (K1)",
            "enabled": true
        },
        {
            "deviceIndex": 1,
            "gasType": "O3",
            "description": "O3 Sensor (K2)",
            "enabled": true
        },
        {
            "deviceIndex": 2,
            "gasType": "NO2",
            "description": "NO2 Sensor (K3)",
            "enabled": true
        }
    ]
}
```

#### Resetowanie konfiguracji MCP3424:
```json
{
    "cmd": "system",
    "command": "resetMCP3424Config"
}
```

**Odpowiedź dla getMCP3424Config:**
```json
{
    "cmd": "mcp3424Config",
    "success": true,
    "config": {
        "deviceCount": 8,
        "configValid": true,
        "devices": [
            {
                "deviceIndex": 0,
                "gasType": "NO",
                "description": "NO Sensor (K1)",
                "enabled": true
            },
            {
                "deviceIndex": 1,
                "gasType": "O3",
                "description": "O3 Sensor (K2)",
                "enabled": true
            }
        ]
    }
}
```

**Domyślne przypisanie urządzeń:**
- **Device 0**: NO (K1) - channel 0,1,2,3
- **Device 1**: O3 (K2) - channel 0,1,2,3
- **Device 2**: NO2 (K3) - channel 0,1,2,3
- **Device 3**: CO (K4) - channel 0,1,2,3
- **Device 4**: SO2 (K5) - channel 0,1,2,3
- **Device 5**: TGS1 - channel 0,1,2,3
- **Device 6**: TGS2 - channel 0,1,2,3
- **Device 7**: TGS3 - channel 0,1,2,3

**Kanały dla każdego urządzenia:**
- **Channel 0**: Working Electrode (WRK)
- **Channel 1**: Auxiliary Electrode (AUX)
- **Channel 2**: Temperature (TEMP) / TGS C3
- **Channel 3**: Supply Voltage (VCC) / TGS C4

**Funkcje:**
- **Przypisanie urządzeń** - prosty system przypisywania urządzeń do gazów
- **Konfiguracja przez WebSocket** - dynamiczna zmiana przypisania
- **Zapisywanie w LittleFS** - konfiguracja zachowana po restarcie
- **Walidacja** - sprawdzanie poprawności przypisania
- **Reset do domyślnych** - przywracanie standardowej konfiguracji

## Dostepne sensory

### SHT40 (Temperatura/Wilgotność/Ciśnienie)
```json
{
    "cmd": "getSensorData",
    "sensor": "sht40"
}
```

**Odpowiedź:**
```json
{
    "cmd": "sensorData",
    "sensor": "sht40",
    "data": {
        "temperature": 23.5,
        "humidity": 45.2,
        "pressure": 1013.25,
        "valid": true
    }
}
```

### SPS30 (Cząsteczki)
```json
{
    "cmd": "getSensorData",
    "sensor": "sps30"
}
```

### CO2 (Dwutlenek węgla)
```json
{
    "cmd": "getSensorData",
    "sensor": "scd41"
}
```

**Odpowiedź:**
```json
{
    "cmd": "sensorData",
    "sensor": "co2",
    "data": {
        "co2": 450,
        "temperature": 23.5,
        "humidity": 45.2,
        "valid": true
    }
}
```

### Fan (Sterowanie wentylatorem)
```json
{
    "cmd": "getSensorData",
    "sensor": "fan"
}
```

**Odpowiedź:**
```json
{
    "cmd": "sensorData",
    "sensor": "fan",
    "data": {
        "dutyCycle": 75,
        "rpm": 1200,
        "enabled": true,
        "glineEnabled": false,
        "pwmValue": 191,
        "pwmFreq": 25000,
        "valid": true
    }
}
```

**Odpowiedź:**
```json
{
    "cmd": "sensorData",
    "sensor": "sps30",
    "data": {
        "pm1_0": 5.2,
        "pm2_5": 12.8,
        "pm4_0": 18.5,
        "pm10": 25.3,
        "nc0_5": 1250,
        "nc1_0": 850,
        "nc2_5": 320,
        "nc4_0": 180,
        "nc10": 95,
        "typical_particle_size": 0.35,
        "valid": true
    }
}
```

### CO2 (SCD41)
```json
{
    "cmd": "getSensorData",
    "sensor": "co2"
}
```

### Power (INA219)
```json
{
    "cmd": "getSensorData",
    "sensor": "power"
}
```

### HCHO (Formaldehyd)
```json
{
    "cmd": "getSensorData",
    "sensor": "hcho"
}
```

### IPS (Cząsteczki)
```json
{
    "cmd": "getSensorData",
    "sensor": "ips"
}
```

**Odpowiedź:**
```json
{
    "cmd": "sensorData",
    "sensor": "ips",
    "data": {
        "pc": [123, 456, 789, 234, 567, 890, 345],
        "pm": [1.23, 4.56, 7.89, 2.34, 5.67, 8.90, 3.45],
        "np": [100, 200, 300, 400, 500, 600, 700],
        "pw": [10, 20, 30, 40, 50, 60, 70],
        "debugMode": false,
        "won": true,
        "valid": true
    }
}
```

### MCP3424 (Napięcia ADC)
```json
{
    "cmd": "getSensorData",
    "sensor": "mcp3424"
}
```

**Odpowiedź:**
```json
{
    "cmd": "sensorData",
    "sensor": "mcp3424",
    "data": {
        "enabled": true,
        "deviceCount": 8,
        "devices": [
            {
                "address": "0x68",
                "valid": true,
                "resolution": 18,
                "gain": 1,
                "channels": {
                    "K1_1_mV": 1234.567,
                    "K1_1_V": 1.234567,
                    "K1_2_mV": 2345.678,
                    "K1_2_V": 2.345678,
                    "K1_3_mV": 3456.789,
                    "K1_3_V": 3.456789,
                    "K1_4_mV": 4567.890,
                    "K1_4_V": 4.567890
                }
            }
        ],
        "valid": true
    }
}
```

**Klucze MCP3424:**
- **Format**: `K{device}_{channel}_{unit}`
- **Device**: 1-8 (numer urządzenia MCP3424)
- **Channel**: 1-4 (numer kanału ADC)
- **Unit**: `mV` (miliwolty) lub `V` (wolty)

**Przykłady:**
- `K1_1_mV` - Device 1, Channel 1, miliwolty
- `K1_1_V` - Device 1, Channel 1, wolty
- `K2_3_mV` - Device 2, Channel 3, miliwolty
- `K8_4_V` - Device 8, Channel 4, wolty

### Kalibracja (Dane skalibrowane)
```json
{
    "cmd": "getSensorData",
    "sensor": "calibration"
}
```

**Odpowiedź z danymi kalibracji:**
```json
{
    "cmd": "sensorData",
    "sensor": "calibration",
    "data": {
        "enabled": true,
        "valid": true,
        "config": {
            "tgsSensors": true,
            "gasSensors": true,
            "ppbConversion": true,
            "specialSensors": true,
            "movingAverages": true
        },
        "temperatures": {
            "K1": 23.5,
            "K2": 24.1,
            "K3": 22.8,
            "K4": 23.9,
            "K5": 24.3,
            "K6": 23.7,
            "K7": 24.0,
            "K8": 23.4,
            "K9": 24.2,
            "K12": 23.6
        },
        "voltages": {
            "K1": 2.45,
            "K2": 2.52,
            "K3": 2.38,
            "K4": 2.49,
            "K5": 2.55,
            "K6": 2.42,
            "K7": 2.48,
            "K8": 2.41,
            "K9": 2.53,
            "K12": 2.44
        },
        "gases_ugm3": {
            "CO": 125.5,
            "NO": 45.2,
            "NO2": 32.8,
            "O3": 78.3,
            "SO2": 12.4,
            "H2S": 8.9,
            "NH3": 15.7,
            "VOC": 156.2
        },
        "gases_ppb": {
            "CO": 125500,
            "NO": 45200,
            "NO2": 32800,
            "O3": 78300,
            "SO2": 12400,
            "H2S": 8900,
            "NH3": 15700,
            "VOC": 156200
        },
        "tgs": {
            "TGS02": 0.045,
            "TGS03": 0.032,
            "TGS12": 0.028,
            "TGS02_ohm": 12500,
            "TGS03_ohm": 8900,
            "TGS12_ohm": 7200
        },
        "special": {
            "HCHO_ppb": 12.5,
            "PID": 0.156,
            "PID_mV": 156.8
        }
    }
}
```

**Dostępne dane kalibracji:**

**Temperatury (K1-K12):**
- `K1` - temperatura czujnika K1 (°C)
- `K2` - temperatura czujnika K2 (°C)
- `K3` - temperatura czujnika K3 (°C)
- `K4` - temperatura czujnika K4 (°C)
- `K5` - temperatura czujnika K5 (°C)
- `K6` - temperatura czujnika K6 (°C)
- `K7` - temperatura czujnika K7 (°C)
- `K8` - temperatura czujnika K8 (°C)
- `K9` - temperatura czujnika K9 (°C)
- `K12` - temperatura czujnika K12 (°C)

**Napięcia (K1-K12):**
- `K1` - napięcie czujnika K1 (V)
- `K2` - napięcie czujnika K2 (V)
- `K3` - napięcie czujnika K3 (V)
- `K4` - napięcie czujnika K4 (V)
- `K5` - napięcie czujnika K5 (V)
- `K6` - napięcie czujnika K6 (V)
- `K7` - napięcie czujnika K7 (V)
- `K8` - napięcie czujnika K8 (V)
- `K9` - napięcie czujnika K9 (V)
- `K12` - napięcie czujnika K12 (V)

**Gazy (µg/m³):**
- `CO` - tlenek węgla (µg/m³)
- `NO` - tlenek azotu (µg/m³)
- `NO2` - dwutlenek azotu (µg/m³)
- `O3` - ozon (µg/m³)
- `SO2` - dwutlenek siarki (µg/m³)
- `H2S` - siarkowodór (µg/m³)
- `NH3` - amoniak (µg/m³)
- `VOC` - lotne związki organiczne (µg/m³)

**Gazy (ppb):**
- `CO` - tlenek węgla (ppb)
- `NO` - tlenek azotu (ppb)
- `NO2` - dwutlenek azotu (ppb)
- `O3` - ozon (ppb)
- `SO2` - dwutlenek siarki (ppb)
- `H2S` - siarkowodór (ppb)
- `NH3` - amoniak (ppb)
- `VOC` - lotne związki organiczne (ppb)

**Czujniki TGS:**
- `TGS02` - wartość czujnika TGS02 (V)
- `TGS03` - wartość czujnika TGS03 (V)
- `TGS12` - wartość czujnika TGS12 (V)
- `TGS02_ohm` - rezystancja TGS02 (Ω)
- `TGS03_ohm` - rezystancja TGS03 (Ω)
- `TGS12_ohm` - rezystancja TGS12 (Ω)

**Czujniki specjalne:**
- `HCHO_ppb` - formaldehyd (ppb)
- `PID` - wartość PID (V)
- `PID_mV` - wartość PID (mV)

## Przykłady użycia w JavaScript

### Pobieranie struktury kluczy:
```javascript
function getSensorKeys() {
    ws.send(JSON.stringify({
        cmd: 'getSensorKeys'
    }));
}

// Użycie:
getSensorKeys();

// Przykład obsługi odpowiedzi:
ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    
    if (data.cmd === 'sensorKeys') {
        console.log('Dostępne klucze SHT40:', Object.keys(data.data.sht40));
        console.log('Dostępne klucze SPS30:', Object.keys(data.data.sps30));
        console.log('Klucze MCP3424:', Object.keys(data.data.mcp3424.devices[0].channels));
        
        // Przykład mapowania kluczy
        const mcp3424Keys = data.data.mcp3424.devices[0].channels;
        console.log('MCP3424 Device 1 klucze:', mcp3424Keys);
        // Wynik: {K1_1_mV: "K1_1_mV", K1_1_V: "K1_1_V", ...}
    }
};
```

### Monitorowanie danych w czasie rzeczywistym:
```javascript
const ws = new WebSocket('ws://192.168.1.100/ws');

ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    
    if (data.cmd === 'update') {
        // Aktualizacja danych sensorów
        if (data.sensors.sht40 && data.sensors.sht40.valid) {
            document.getElementById('temperature').textContent = 
                data.sensors.sht40.temperature.toFixed(1) + '°C';
            document.getElementById('humidity').textContent = 
                data.sensors.sht40.humidity.toFixed(1) + '%';
        }
        
        if (data.sensors.sps30 && data.sensors.sps30.valid) {
            document.getElementById('pm25').textContent = 
                data.sensors.sps30.pm2_5.toFixed(1) + ' µg/m³';
        }
    }
};
```

### Pobieranie historii:
```javascript
function getHistory(sensor, timeRange, sampleType = 'fast') {
    ws.send(JSON.stringify({
        cmd: 'getHistory',
        sensor: sensor,
        timeRange: timeRange,
        sampleType: sampleType
    }));
}

// Użycie:
getHistory('sht40', '1h', 'fast');  // Szybkie próbki
getHistory('sps30', '24h', 'slow');  // Wolne próbki
getHistory('sht40', '6h');           // Domyślnie fast
getHistory('calibration', '6h', 'fast');  // Historia kalibracji

// Z timestampami:
getHistoryWithTimestamps('sht40', 1704067200000, 1704074400000, 'fast');  // Ostatnie 2h
getHistoryWithTimestamps('sps30', 1704067200000, 1704153600000, 'slow');  // Konkretny dzień
```

**Funkcja z timestampami:**
```javascript
function getHistoryWithTimestamps(sensor, fromTime, toTime, sampleType = 'fast') {
    ws.send(JSON.stringify({
        cmd: 'getHistory',
        sensor: sensor,
        fromTime: fromTime,
        toTime: toTime,
        sampleType: sampleType
    }));
}

// Przykłady użycia:
// Ostatnie 30 minut
const now = Date.now();
const thirtyMinutesAgo = now - (30 * 60 * 1000);
getHistoryWithTimestamps('sht40', thirtyMinutesAgo, now, 'fast');

// Konkretny dzień (15 stycznia 2024)
const specificDate = new Date('2024-01-15T00:00:00Z').getTime();
const nextDay = specificDate + (24 * 60 * 60 * 1000);
getHistoryWithTimestamps('sps30', specificDate, nextDay, 'slow');

// Ostatnie 6 godzin
const sixHoursAgo = now - (6 * 60 * 60 * 1000);
getHistoryWithTimestamps('calibration', sixHoursAgo, now, 'fast');
```
```

### Sterowanie systemem:
```javascript
function restartSystem() {
    ws.send(JSON.stringify({
        cmd: 'system',
        command: 'restart'
    }));
}

function toggleSensor(sensor, enabled) {
    const config = {};
    config[sensor] = enabled;
    
    ws.send(JSON.stringify({
        cmd: 'setConfig',
        ...config
    }));
}

// Przykłady:
toggleSensor('enableSPS30', true);
toggleSensor('enableSHT40', false);
```

### Sterowanie wentylatorem i GLine:
```javascript
// Sterowanie wentylatorem
function setFanSpeed(speed) {
    ws.send(JSON.stringify({
        cmd: 'system',
        command: 'fan_speed',
        value: speed
    }));
}

function turnFanOn() {
    ws.send(JSON.stringify({
        cmd: 'system',
        command: 'fan_on'
    }));
}

function turnFanOff() {
    ws.send(JSON.stringify({
        cmd: 'system',
        command: 'fan_off'
    }));
}

// Sterowanie GLine
function setGLine(enabled) {
    ws.send(JSON.stringify({
        cmd: 'system',
        command: enabled ? 'gline_on' : 'gline_off'
    }));
}

// Tryb sleep
function startSleepMode(delaySeconds, durationSeconds) {
    ws.send(JSON.stringify({
        cmd: 'system',
        command: 'sleep',
        delay: delaySeconds,
        duration: durationSeconds
    }));
}

function stopSleepMode() {
    ws.send(JSON.stringify({
        cmd: 'system',
        command: 'sleep_stop'
    }));
}

// Status wentylatora
function getFanStatus() {
    ws.send(JSON.stringify({
        cmd: 'system',
        command: 'fan_status'
    }));
}

// Przykłady użycia:
setFanSpeed(75);           // Ustaw prędkość na 75%
turnFanOn();               // Włącz wentylator
setGLine(true);            // Włącz GLine
startSleepMode(60, 300);   // Sleep: opóźnienie 60s, trwanie 5min
stopSleepMode();           // Zatrzymaj sleep
getFanStatus();            // Pobierz status

// Przykład obsługi odpowiedzi:
ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    
    if (data.cmd === 'fan_status' && data.success) {
        const fan = data.data;
        console.log('Wentylator:', fan.enabled ? 'Włączony' : 'Wyłączony');
        console.log('Prędkość:', fan.dutyCycle + '%');
        console.log('RPM:', fan.rpm);
        console.log('GLine:', fan.glineEnabled ? 'Włączony' : 'Wyłączony');
        
        if (fan.sleepMode) {
            const endTime = new Date(fan.sleepEndTime);
            console.log('Tryb sleep aktywny, kończy się:', endTime.toLocaleString());
        }
    }
};
```

### Monitorowanie historii:
```javascript
function getHistoryInfo() {
    ws.send(JSON.stringify({
        cmd: 'getHistoryInfo'
    }));
}

// Sprawdź status historii
getHistoryInfo();
```

### Pobieranie danych kalibracji:
```javascript
function getCalibrationData() {
    ws.send(JSON.stringify({
        cmd: 'getSensorData',
        sensor: 'calibration'
    }));
}

### Konwersja czasu dla timestampów:
```javascript
// Pomocnicze funkcje do konwersji czasu
function getTimestampFromDate(dateString) {
    return new Date(dateString).getTime();
}

function getTimestampFromNow(minutesAgo) {
    return Date.now() - (minutesAgo * 60 * 1000);
}

function getTimestampFromHours(hoursAgo) {
    return Date.now() - (hoursAgo * 60 * 60 * 1000);
}

// Przykłady:
const now = Date.now();
const oneHourAgo = getTimestampFromHours(1);
const thirtyMinutesAgo = getTimestampFromNow(30);
const specificDate = getTimestampFromDate('2024-01-15T10:00:00Z');

console.log('Aktualny czas:', new Date(now).toISOString());
console.log('Godzinę temu:', new Date(oneHourAgo).toISOString());
console.log('30 minut temu:', new Date(thirtyMinutesAgo).toISOString());
console.log('Konkretna data:', new Date(specificDate).toISOString());
```

// Przykład użycia w onmessage:
ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    
    if (data.cmd === 'sensorData') {
        // IPS Sensor
        if (data.sensor === 'ips' && data.data.valid) {
            console.log('IPS PC values:', data.data.pc);
            console.log('IPS PM values:', data.data.pm);
            console.log('IPS NP values:', data.data.np);
            console.log('IPS PW values:', data.data.pw);
            console.log('IPS Debug Mode:', data.data.debugMode);
            console.log('IPS WON:', data.data.won);
        }
        
        // MCP3424 Sensor
        if (data.sensor === 'mcp3424' && data.data.valid) {
            console.log('MCP3424 Device Count:', data.data.deviceCount);
            data.data.devices.forEach((device, index) => {
                console.log(`Device ${index + 1} (${device.address}):`);
                console.log('  Valid:', device.valid);
                console.log('  Resolution:', device.resolution);
                console.log('  Gain:', device.gain);
                console.log('  Channels:', device.channels);
                
                // Przykład dostępu do konkretnych kanałów
                const deviceNum = index + 1;
                console.log(`  K${deviceNum}_1_mV:`, device.channels[`K${deviceNum}_1_mV`]);
                console.log(`  K${deviceNum}_1_V:`, device.channels[`K${deviceNum}_1_V`]);
            });
        }
        
        // Calibration Data
        if (data.sensor === 'calibration' && data.data.valid) {
            // Wyświetl temperatury
            console.log('Temperatura K1:', data.data.temperatures.K1 + '°C');
            console.log('Temperatura K2:', data.data.temperatures.K2 + '°C');
            
            // Wyświetl gazy (µg/m³)
            console.log('CO:', data.data.gases_ugm3.CO + ' µg/m³');
            console.log('NO2:', data.data.gases_ugm3.NO2 + ' µg/m³');
            console.log('O3:', data.data.gases_ugm3.O3 + ' µg/m³');
            
            // Wyświetl gazy (ppb)
            console.log('CO (ppb):', data.data.gases_ppb.CO + ' ppb');
            console.log('NO2 (ppb):', data.data.gases_ppb.NO2 + ' ppb');
            
            // Wyświetl czujniki TGS
            console.log('TGS02:', data.data.tgs.TGS02 + ' V');
            console.log('TGS02_ohm:', data.data.tgs.TGS02_ohm + ' Ω');
            
            // Wyświetl czujniki specjalne
            console.log('HCHO:', data.data.special.HCHO_ppb + ' ppb');
            console.log('PID:', data.data.special.PID + ' V');
        }
    }
};
```

## Broadcast danych

System automatycznie wysyla dane do wszystkich klientów co 5 sekund:

```json
{
    "cmd": "update",
    "timestamp": 1700000000000,
    "uptime": 3600,
    "freeHeap": 150000,
    "sensors": {
        "sht40": {
            "temperature": 23.5,
            "humidity": 45.2,
            "pressure": 1013.25,
            "valid": true
        },
        "sps30": {
            "pm1_0": 5.2,
            "pm2_5": 12.8,
            "pm10": 25.3,
            "valid": true
        }
    }
}
```

## Struktura odpowiedzi

Wszystkie odpowiedzi zawierają:
- `cmd` - nazwa komendy
- `success` - true/false (jeśli dotyczy)
- `error` - opis błędu (jeśli wystąpił)

### Przykład błędu:
```json
{
    "cmd": "getSensorData",
    "error": "Sensor not available or invalid: unknown_sensor"
}
```

### Przykład sukcesu:
```json
{
    "cmd": "setConfig",
    "success": true,
    "message": "Configuration updated",
    "enableSPS30": true
}
```

## Obsługa błędów

### Przykład błędu
```json
{
    "cmd": "getSensorData",
    "error": "Sensor not available or invalid: unknown_sensor"
}
```

### Przykład sukcesu
```json
{
    "cmd": "setConfig",
    "success": true,
    "message": "Configuration updated",
    "enableSPS30": true
}
```

## Limity i bezpieczeństwo

### Limity pamięci
- **Minimum free heap**: 10KB przed operacjami WebSocket
- **JSON response limit**: 3KB
- **History samples limit**: 30 próbek na żądanie
- **Broadcast memory check**: 15KB przed broadcastem

### Timeouty
- **WebSocket timeout**: 2000ms
- **JSON parsing timeout**: 100ms
- **Memory check**: przed każdą operacją

### Bezpieczeństwo
- Wszystkie błędy są logowane
- Sprawdzanie dostępnej pamięci
- Graceful degradation przy niskiej pamięci
- Timeouty dla wszystkich operacji

## Rozwiązywanie problemów

### Problem: Brak połączenia WebSocket
```
❌ Błąd połączenia: [Errno 111] Connection refused
```

**Rozwiązanie:**
1. Sprawdź czy ESP32 jest podłączone do WiFi
2. Sprawdź adres IP w Serial Monitor
3. Sprawdź czy WebSocket jest włączony w konfiguracji

### Problem: Niskie zużycie pamięci
```
❌ Błąd: Low memory
```

**Rozwiązanie:**
1. Sprawdź dostępną pamięć: `{"cmd": "system", "command": "memory"}`
2. Wyłącz nieużywane sensory w konfiguracji
3. Zmniejsz rozmiar historii danych

### Problem: Brak danych sensorów
```
❌ sht40: brak danych lub błąd
```

**Rozwiązanie:**
1. Sprawdź status sensorów: `{"cmd": "status"}`
2. Sprawdź konfigurację: `{"cmd": "getConfig"}`
3. Włącz sensor: `{"cmd": "setConfig", "enableSHT40": true}`

## Timeouty i bezpieczeństwo

- **Timeout WebSocket**: 2000ms
- **Timeout JSON parsing**: 100ms
- **Memory check**: przed każdą operacją sprawdzana jest dostępna pamięć
- **Error handling**: wszystkie błędy są logowane i zwracane do klienta

## Limity pamięci

- **Minimum free heap**: 10KB przed operacjami WebSocket
- **JSON response limit**: 3KB
- **History samples limit**: 30 próbek na żądanie
- **Broadcast memory check**: 15KB przed broadcastem

## Bezpieczeństwo i firewall

### Porty do otwarcia:
- **Port 81** (HTTP) - dla interfejsu web i WebSocket
- **Port 443** (HTTPS) - jeśli używany (obecnie nie)

### Firewall:
```bash
# Linux/Ubuntu
sudo ufw allow 81/tcp

# Windows
netsh advfirewall firewall add rule name="ESP32 WebSocket" dir=in action=allow protocol=TCP localport=81

# macOS
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --add /path/to/your/app
```

### Test połączenia:
```bash
# Test HTTP
curl http://192.168.1.100:81/test

# Test WebSocket (wymaga specjalnego klienta)
# Użyj test_websocket.py lub przeglądarki
```

## Wsparcie

W przypadku problemów:
1. Sprawdź logi w Serial Monitor
2. Użyj testu Python: `python test_websocket.py`
3. Sprawdź konfigurację w `include/config.h`
4. Sprawdź czy port 80 nie jest blokowany przez firewall 

# WebSocket API - Komendy Sterowania Wentylatorem i GLine (Sleep Mode)

## Nowe Komendy WebSocket

### 1. Ustawienie predkosci wentylatora
```json
{
  "cmd": "system",
  "command": "fan_speed",
  "value": 60
}
```
- `value`: liczba (0-100), procent PWM
- Odpowiedz:
```json
{
  "cmd": "system",
  "command": "fan_speed",
  "success": true,
  "message": "Fan speed set to 60%"
}
```

### 2. Wlaczenie/Wylaczenie wentylatora
```json
{
  "cmd": "system",
  "command": "fan_on"
}
```
```json
{
  "cmd": "system",
  "command": "fan_off"
}
```
- Odpowiedz:
```json
{
  "cmd": "system",
  "command": "fan_on",
  "success": true,
  "message": "Fan enabled"
}
```

### 3. Wlaczenie/Wylaczenie GLine (Router Power)
```json
{
  "cmd": "system",
  "command": "gline_on"
}
```
```json
{
  "cmd": "system",
  "command": "gline_off"
}
```
- Odpowiedz:
```json
{
  "cmd": "system",
  "command": "gline_on",
  "success": true,
  "message": "GLine enabled"
}
```

### 4. Sleep Mode (czasowe wylaczenie GLine)
```json
{
  "cmd": "system",
  "command": "sleep",
  "delay": 60,      // sekundy do rozpoczecia sleep
  "duration": 300   // sekundy trwania sleep
}
```
- Odpowiedz:
```json
{
  "cmd": "system",
  "command": "sleep",
  "success": true,
  "message": "Sleep mode scheduled: delay 60s, duration 300s"
}
```

#### Przerwanie sleep/wake:
```json
{
  "cmd": "system",
  "command": "sleep_stop"
}
```
LUB
```json
{
  "cmd": "system",
  "command": "wake"
}
```
- Odpowiedz:
```json
{
  "cmd": "system",
  "command": "sleep_stop",
  "success": true,
  "message": "Sleep mode stopped"
}
```

### 5. Status wentylatora i sleep
```json
{
  "cmd": "system",
  "command": "fan_status"
}
```
- Odpowiedz:
```json
{
  "cmd": "system",
  "command": "fan_status",
  "success": true,
  "status": {
    "enabled": true,
    "dutyCycle": 60,
    "rpm": 1200,
    "glineEnabled": false,
    "sleepMode": true,
    "sleepStartTime": 1712345678,
    "sleepDuration": 300000,
    "sleepEndTime": 1712345978,
    "valid": true
  }
}
```
- `sleepMode`: true gdy sleep aktywny
- `sleepStartTime`, `sleepEndTime`: timestamp (sekundy od 1970)
- `sleepDuration`: ms

---

## Komendy Serial (Monitor portu szeregowego)

- `FAN_ON` - wlacz wentylator (50%)
- `FAN_OFF` - wylacz wentylator
- `FAN_SPEED_[0-100]` - ustaw PWM
- `GLINE_ON` - wlacz GLine
- `GLINE_OFF` - wylacz GLine
- `FAN_STATUS` - wypisz status
- `SLEEP_[delay]_[duration]` - sleep mode (np. `SLEEP_60_300`)
- `SLEEP_STOP` lub `SLEEP_WAKE` - przerwij sleep

Przyklad:
```
FAN_SPEED_80
SLEEP_30_120
FAN_STATUS
```

---

## FAQ

**Q: Co sie stanie gdy sleep mode jest aktywny?**
A: GLine zostanie wylaczony na zadany czas, po czym automatycznie powroci do poprzedniego stanu.

**Q: Czy sleep mozna przerwac?**
A: Tak, komenda `sleep_stop` (WebSocket) lub `SLEEP_STOP`/`SLEEP_WAKE` (Serial) natychmiast przywraca GLine.

**Q: Czy status sleep jest widoczny?**
A: Tak, w odpowiedzi na `fan_status` (`sleepMode: true` oraz czasy).

---

## Uwagi
- Wszystkie komendy WebSocket wymagaja pola `cmd: "system"`.
- Odpowiedzi zawsze zawieraja pole `success` oraz `message`.
- Sleep mode nie wplywa na prace wentylatora.
- Komendy dzialaja tylko gdy `config.enableFan == true`.

---

**Ostatnia aktualizacja: [data automatyczna przez AI]** 