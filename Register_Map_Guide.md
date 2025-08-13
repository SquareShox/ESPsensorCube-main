# ESP32 Sensor Cube - Mapa Rejestrów Modbus

## Nowy Format Rejestrów

Każdy blok czujnika ma teraz standardowy format:
- **Rejestr 0**: Status czujnika (1 = OK, 0 = ERROR)
- **Rejestr 1**: Typ danych (0 = aktualne, 1 = średnia 10s, 2 = średnia 5min)
- **Rejestr 2**: Timestamp aktualizacji danych (dolne 16 bitów)
- **Rejestr 3**: Timestamp aktualizacji danych (górne 16 bitów)
- **Rejestr 4+**: Dane czujnika

## Mapa Rejestrów

### Solar Sensor (0-49)
```
Rejestr 0:  Status czujnika
Rejestr 1:  Typ danych
Rejestr 2:  Timestamp (lower 16 bits)
Rejestr 3:  Timestamp (upper 16 bits)
Rejestr 4:  PID (hex)
Rejestr 5:  Firmware version
Rejestr 6:  Serial number
Rejestr 7:  Battery voltage (mV)
Rejestr 8:  Battery current (mA)
Rejestr 9:  Panel voltage (mV)
Rejestr 10: Panel power (mW)
Rejestr 11: Charge state
Rejestr 12: MPPT state
Rejestr 13: OR register
Rejestr 14: Error flags
Rejestr 15: Load state (1=ON, 0=OFF)
Rejestr 16: Load current (mA)
Rejestr 17: H19 register
Rejestr 18: H20 register
Rejestr 19: H21 register
Rejestr 20: H22 register
Rejestr 21: H23 register
Rejestr 22: HSDS register
Rejestry 23-49: Rezerwowe
```

### OPCN3 Sensor (50-99)
```
Rejestr 0:  Status czujnika
Rejestr 1:  Typ danych
Rejestr 2:  Timestamp (lower 16 bits)
Rejestr 3:  Timestamp (upper 16 bits)
Rejestr 4:  Temperatura (°C * 100)
Rejestr 5:  Wilgotność (% * 100)
Rejestry 6-29: Bin counts (24 bins)
Rejestr 30: Bin1 time to cross
Rejestr 31: Bin3 time to cross
Rejestr 32: Bin5 time to cross
Rejestr 33: Bin7 time to cross
Rejestr 34: Status potwierdzenie
Rejestr 35: Sample flow rate (* 100)
Rejestr 36: Sampling period
Rejestr 37: PM1.0 (* 100)
Rejestr 38: PM2.5 (* 100)
Rejestr 39: PM10 (* 100)
Rejestry 40-99: Rezerwowe
```

### I2C Sensors (100-149)
```
Rejestr 0:  Status czujnika
Rejestr 1:  Typ danych
Rejestr 2:  Timestamp (lower 16 bits)
Rejestr 3:  Timestamp (upper 16 bits)
Rejestr 4:  Temperatura (°C * 100)
Rejestr 5:  Wilgotność (% * 100)
Rejestr 6:  Ciśnienie (hPa * 10)
Rejestr 7:  CO2 (ppm)
Rejestr 8:  Typ czujnika
Rejestry 9-149: Rezerwowe
```

### IPS Sensor (150-199)
```
Rejestr 0:  Status czujnika
Rejestr 1:  Typ danych
Rejestr 2:  Timestamp (lower 16 bits)
Rejestr 3:  Timestamp (upper 16 bits)
Rejestr 4:  Debug mode (1=ON, 0=OFF)
Rejestr 5:  Won period (1=200ms, 2=500ms, 3=1000ms)
Rejestry 6-19: PC values (7 wartości, 32-bit każda)
Rejestry 20-26: PM values (7 wartości * 1000)
Rejestry 27-40: NP values (7 wartości, 32-bit każda, tylko debug mode)
Rejestry 41-47: PW values (7 wartości, tylko debug mode)
Rejestry 48-199: Rezerwowe
```

### MCP3424 ADC (200-349)
```
Rejestr 0:  Status czujnika
Rejestr 1:  Typ danych
Rejestr 2:  Timestamp (lower 16 bits)
Rejestr 3:  Timestamp (upper 16 bits)
Rejestr 4:  Liczba wykrytych urządzeń
Rejestry 5-21: Urządzenie 0 (16 rejestrów)
Rejestry 22-37: Urządzenie 1 (16 rejestrów)
...
Rejestry 325-341: Urządzenie 20 (16 rejestrów)
Rejestry 342-349: Rezerwowe

Format danych urządzenia:
Rejestr 0: Status urządzenia
Rejestr 1: Adres I2C
Rejestr 2: Rozdzielczość
Rejestr 3: Wzmocnienie
Rejestry 4-11: Kanały 0-3 (32-bit każdy, µV)
Rejestr 12: Wiek danych (sekundy)
Rejestry 13-15: Rezerwowe
```

### ADS1110 ADC (350-399)
```
Rejestr 0:  Status czujnika
Rejestr 1:  Typ danych
Rejestr 2:  Timestamp (lower 16 bits)
Rejestr 3:  Timestamp (upper 16 bits)
Rejestr 4:  Data rate
Rejestr 5:  Gain
Rejestry 6-7: Napięcie (32-bit, µV)
Rejestr 8:  Wiek danych (sekundy)
Rejestry 9-399: Rezerwowe
```

### INA219 Power Monitor (400-449)
```
Rejestr 0:  Status czujnika
Rejestr 1:  Typ danych
Rejestr 2:  Timestamp (lower 16 bits)
Rejestr 3:  Timestamp (upper 16 bits)
Rejestr 4:  Bus voltage (mV)
Rejestr 5:  Shunt voltage (0.1mV)
Rejestr 6:  Current (mA)
Rejestr 7:  Power (mW)
Rejestr 8:  Wiek danych (sekundy)
Rejestry 9-449: Rezerwowe
```

### SPS30 Particle Sensor (450-499)
```
Rejestr 0:  Status czujnika
Rejestr 1:  Typ danych
Rejestr 2:  Timestamp (lower 16 bits)
Rejestr 3:  Timestamp (upper 16 bits)
Rejestr 4:  PM1.0 (µg/m³ * 10)
Rejestr 5:  PM2.5 (µg/m³ * 10)
Rejestr 6:  PM4.0 (µg/m³ * 10)
Rejestr 7:  PM10 (µg/m³ * 10)
Rejestr 8:  NC0.5 (#/cm³ * 1000)
Rejestr 9:  NC1.0 (#/cm³ * 1000)
Rejestr 10: NC2.5 (#/cm³ * 1000)
Rejestr 11: NC4.0 (#/cm³ * 1000)
Rejestr 12: NC10 (#/cm³ * 1000)
Rejestr 13: Typical particle size (µm * 1000)
Rejestr 14: Wiek danych (sekundy)
Rejestry 15-499: Rezerwowe
```

## Kontrola Systemowa

### Rejestry Kontrolne
```
Rejestr 90:  Typ danych (0=aktualne, 1=średnia 10s, 2=średnia 5min)
Rejestr 91:  Komendy systemowe (pisz wartość, automatycznie zeruje)
Rejestr 92:  Stan auto-reset
Rejestr 93:  Status systemu średnich kroczących
```

### Komendy Systemowe (Rejestr 91)
```
1:  Odczyt OPCN3
2:  Reset systemu
3:  Przełącz auto-reset
4:  Odczyt IPS
5:  Włącz IPS debug mode
6:  Wyłącz IPS debug mode
7:  Przełącz na dane aktualne
8:  Przełącz na średnie szybkie (10s)
9:  Przełącz na średnie wolne (5min)
10: Wydrukuj status średnich
11: Cykliczne przełączanie typów danych
```

## Odczyt Timestamp

Timestamp jest zapisany jako 32-bit liczba (millis() z ESP32):
```python
timestamp = (register_upper << 16) | register_lower
```

## Przykłady Użycia

### Python - Odczyt temperatury I2C
```python
import modbus_tk
from modbus_tk import modbus_rtu

# Odczyt typu danych
data_type = mb.read_holding_registers(101, 1)[0]  # I2C rejestr 1
timestamp_lower = mb.read_holding_registers(102, 1)[0]
timestamp_upper = mb.read_holding_registers(103, 1)[0]
temperature = mb.read_holding_registers(104, 1)[0] / 100.0  # Temperatura

timestamp = (timestamp_upper << 16) | timestamp_lower
print(f"Temperatura: {temperature}°C, Typ: {data_type}, Timestamp: {timestamp}")
```

### Python - Przełączanie na średnie 10s
```python
# Przełącz na średnie 10-sekundowe
mb.write_single_register(90, 1)  # Typ danych = 1 (fast average)
```

### Python - Przełączanie przez komendy
```python
# Przełącz na średnie 10-sekundowe przez komendę
mb.write_single_register(91, 8)  # Komenda 8 = fast average
```

## Uwagi

1. **Timestamp**: Automatycznie aktualizowany przy każdej zmianie danych
2. **Typ danych**: Pokazuje czy dane są aktualne czy średnie
3. **Standardowy format**: Każdy czujnik ma identyczne pierwsze 4 rejestry
4. **Rozszerzone bloki**: Każdy czujnik ma 50 rejestrów (MCP3424 ma 150)
5. **Kontrola w czasie rzeczywistym**: Możliwość przełączania między typami danych 