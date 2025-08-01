# Pliki Konfiguracyjne - Folder `data/`

Ten folder zawiera przykładowe pliki konfiguracyjne dla systemu ESP Sensor Cube, które są używane przez LittleFS (system plików flash).

## 📋 **Spis Plików**

### 🌐 **`network.json`** - Konfiguracja Sieci
```json
{
  "useDHCP": true,             // Używaj DHCP (true) lub konfiguracji statycznej (false)
  "staticIP": "192.168.1.100", // IP statyczne (gdy useDHCP = false)
  "gateway": "192.168.1.1",    // Bramka domyślna
  "subnet": "255.255.255.0",   // Maska podsieci
  "dns1": "8.8.8.8",          // Pierwszy serwer DNS
  "dns2": "8.8.4.4"           // Drugi serwer DNS
}
```

### 📶 **`wifi.json`** - Konfiguracja WiFi
```json
{
  "ssid": "ESP_SensorCube_WiFi", // Nazwa sieci WiFi
  "password": "password123"       // Hasło WiFi
}
```

### ⚙️ **`system.json`** - Konfiguracja Systemu
```json
{
  "enableWiFi": true,        // Włącz WiFi
  "enableWebServer": true,   // Włącz serwer WWW
  "enableHistory": true,     // Włącz zapis historii
  "useAveragedData": false,  // Używaj uśrednionych danych
  "enableModbus": false,     // Włącz komunikację Modbus
  
  // Czujniki środowiskowe
  "enableSPS30": false,      // Włącz czujnik SPS30 (pył)
  "enableSHT40": false,      // Włącz czujnik SHT40 (temp/wilgotność)
  "enableSHT30": false,      // Włącz czujnik SHT30 (temp/wilgotność)
  "enableBME280": false,     // Włącz czujnik BME280 (temp/wilgotność/ciśnienie)
  "enableSCD41": false,      // Włącz czujnik SCD41 (CO2/temp/wilgotność)
  "enableHCHO": false,       // Włącz czujnik HCHO (formaldehyd)
  
  // Konwertery ADC
  "enableMCP3424": false,    // Włącz konwerter ADC MCP3424 (18-bit)
  "enableADS1110": false,    // Włącz konwerter ADC ADS1110 (16-bit)
  
  // Monitoring zasilania
  "enableINA219": false,     // Włącz monitoring zasilania INA219
  "enableSolarSensor": false,// Włącz czujnik panelu słonecznego
  
  // Czujniki pyłu
  "enableIPS": false,        // Włącz czujnik pyłu IPS (UART)
  "enableIPSDebug": false,   // Włącz tryb debug IPS
  "enableOPCN3Sensor": false,// Włącz czujnik pyłu OPCN3 (optyczny)
  
  // Systemy kontrolne  
  "enableI2CSensors": false, // Włącz ogólne czujniki I2C
  "enableFan": false,        // Włącz sterowanie wentylatorem
  
  // Tryby pracy
  "lowPowerMode": false,     // Tryb niskiego poboru energii
  "autoReset": false,        // Automatyczny restart systemu
  
  // Powiadomienia
  "enablePushbullet": false, // Włącz powiadomienia Pushbullet
  "pushbulletToken": ""      // Token API Pushbullet
}
```

### 🔌 **`mcp3424.json`** - Konfiguracja MCP3424 (ADC)
```json
{
  "deviceCount": 8,          // Liczba skonfigurowanych urządzeń
  "devices": [               // Lista urządzeń MCP3424
    {
      "deviceIndex": 0,      // Indeks urządzenia (0-7)
      "i2cAddress": 104,     // Adres I2C (0x68 = 104)
      "gasType": "NO",       // Typ gazu/czujnika
      "description": "NO Sensor (K1)", // Opis czujnika
      "enabled": true,       // Włączone (true/false)
      "autoDetected": false  // Auto-wykryte podczas skanowania I2C
    }
    // ... kolejne urządzenia
  ]
}
```

## 🛠️ **Jak Działają Te Pliki**

1. **📁 Folder `data/`** jest kopiowany do flash ESP32 przez PlatformIO podczas upload
2. **🔄 LittleFS** ładuje te pliki przy starcie systemu
3. **⚙️ System** używa tych wartości jako domyślnych jeśli plik nie istnieje w flash
4. **💾 Konfiguracja** może być zmieniana przez interfejs WWW i zapisywana z powrotem do flash

## 📝 **Uwagi**

- **Adresy I2C** w `mcp3424.json` są podane w formacie dziesiętnym:
  - `104` = `0x68` (hex)
  - `106` = `0x6A` (hex)
  - itd.

- **WiFi hasło** w `wifi.json` można zmienić przez interfejs sieciowy na ESP

- **System konfiguracja** może być modyfikowana przez panel sterowania na stronie `/`

- **Pliki są tylko przykładowe** - system może je nadpisać podczas działania

## 🚀 **Przydatne Komendy PlatformIO**

```bash
# Upload plików data do LittleFS
pio run --target uploadfs

# Kompilacja i upload firmware + data
pio run --target upload --target uploadfs
```