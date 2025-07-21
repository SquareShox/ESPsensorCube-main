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

### Komendy systemowe

| Komenda | Opis | Przykład |
|---------|------|----------|
| `getConfig` | Pobierz konfigurację | `{"cmd": "getConfig"}` |
| `setConfig` | Ustaw konfigurację | `{"cmd": "setConfig", "enableSPS30": true}` |
| `system` | Komendy systemowe | `{"cmd": "system", "command": "memory"}` |
| `calibration` | Kontrola kalibracji | `{"cmd": "calibration", "command": "status"}` |

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
        "scd41": true,
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
- `scd41` - dwutlenek węgla (CO2)
- `power` - napięcie, prąd, moc (INA219)
- `hcho` - formaldehyd
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
- `sensor`: `solar`, `i2c`, `sps30`, `power`, `hcho`, `all`
- `type`: `fast` (10s) lub `slow` (5min)

### 6. Konfiguracja systemu

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

### 10. Sterowanie wentylatorem

#### Włączenie wentylatora:
```json
{
    "cmd": "setConfig",
    "enableFan": true
}
```

#### Wyłączenie wentylatora:
```json
{
    "cmd": "setConfig",
    "enableFan": false
}
```

#### Ustawienie prędkości (duty cycle 0-100%):
```json
{
    "cmd": "system",
    "command": "fan",
    "dutyCycle": 75
}
```

#### Włączenie/wyłączenie GLine (router):
```json
{
    "cmd": "system",
    "command": "gline",
    "enabled": true
}
```

#### Status wentylatora:
```json
{
    "cmd": "getSensorData",
    "sensor": "fan"
}
```

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

### SCD41 (Dwutlenek węgla)
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
    "sensor": "scd41",
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
    
    if (data.cmd === 'sensorData' && data.sensor === 'calibration') {
        if (data.data.valid) {
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