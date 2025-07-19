# Dokumentacja Systemu ESP32 Sensor Cube

## Spis treści
1. [Wprowadzenie](#wprowadzenie)
2. [Główne możliwości](#główne-możliwości)
3. [Struktura katalogów](#struktura-katalogów)
4. [Przykładowa konfiguracja w kodzie](#przykładowa-konfiguracja-w-kodzie)
5. [Najważniejsze komendy serial](#najważniejsze-komendy-serial)
6. [Przełączanie typu danych](#przełączanie-typu-danych)
7. [Rejestry Modbus](#rejestry-modbus)
8. [System średnich kroczących](#system-średnich-kroczących)
9. [Kompilacja i użytkowanie](#kompilacja-i-użytkowanie)
10. [Status LED](#status-led)
11. [Dalsze materiały](#dalsze-materiały)

## Wprowadzenie
Niniejszy dokument podsumowuje najważniejsze funkcje oprogramowania w repozytorium.
Projekt realizuje rozbudowany system pomiarowy oparty o mikrokontroler ESP32 z
obsługą wielu czujników i komunikacji Modbus oraz Wi‑Fi.

## Główne możliwości
- obsługa czujników solarnych, OPCN3, IPS oraz wielu czujników I2C (SHT30, BME280, SCD40 i inne)
- komunikacja Modbus RTU na porcie UART2
- wbudowany serwer WWW z REST API i wsparciem OTA
- dynamiczna konfiguracja czujników w czasie działania (komendy przez UART lub API)
- mechanizm średnich kroczących (10 s i 5 min) dla stabilizacji danych
- możliwość przełączania typu danych (aktualne, średnie szybkie, średnie wolne)

## Struktura katalogów
```
src/       – kod źródłowy programu
include/   – pliki nagłówkowe
lib/       – zewnętrzne biblioteki
``` 
Szczegółowa struktura katalogu `src` znajduje się w pliku README.md【F:README.md†L7-L18】.

## Przykładowa konfiguracja w kodzie
```cpp
config.enableSolarSensor = true;
config.enableOPCN3Sensor = true;
config.enableI2CSensors = true;
config.enableIPS = true;
config.enableModbus = true;
config.enableWebServer = true;
config.enableWiFi = true;
config.autoReset = true;
```
Konfigurację można modyfikować bez ponownej kompilacji za pomocą komend opisanych
w README【F:README.md†L39-L57】.

## Najważniejsze komendy serial
- `SEND` – wydruk danych ze wszystkich czujników
- `STATUS` – status systemu i czujników
- `RESTART` – restart urządzenia
- `CONFIG_…` – włączanie/wyłączanie poszczególnych funkcji
Kompletną listę komend zawiera plik README.md w sekcji „Komendy Serial”【F:README.md†L33-L57】.

## Przełączanie typu danych
System pozwala wybierać pomiędzy danymi bieżącymi a średnimi z różnych okresów.
Służą do tego komendy `DATATYPE_CURRENT`, `DATATYPE_FAST` i `DATATYPE_SLOW` lub
odpowiednie rejestry Modbus【F:DataType_Switching_Guide.md†L1-L24】【F:DataType_Switching_Guide.md†L25-L46】.

## Rejestry Modbus
Każdy blok czujników ma jednolitą strukturę rejestrów: status, typ danych,
timestamp i właściwe dane pomiarowe. Pełną mapę rejestrów opisuje dokument
`Register_Map_Guide.md`【F:Register_Map_Guide.md†L1-L21】【F:Register_Map_Guide.md†L22-L72】.

## System średnich kroczących
W projekcie zaimplementowano uniwersalny mechanizm obliczania średnich ważonych
dla wszystkich czujników. Działa on w dwóch oknach czasowych – 10 sekund i 5
minut – a przełączanie między trybami odbywa się przez Modbus lub komendy
serial【F:README_MovingAverages.md†L1-L26】【F:README_MovingAverages.md†L27-L39】.

## Kompilacja i użytkowanie
1. Skonfiguruj parametry sieci Wi‑Fi w pliku `config.h`.
2. Wybierz, które czujniki mają być aktywne.
3. Wgraj oprogramowanie na płytkę ESP32 (projekt wykorzystuje PlatformIO).
4. Monitoruj działanie przez port szeregowy lub stronę WWW.
5. Dane można odczytywać poprzez Modbus bądź REST API.
Wymagane biblioteki i dodatkowe informacje znajdują się w README【F:README.md†L166-L188】.

## Status LED
- biały – faza inicjalizacji
- zielony – wszystkie czujniki działają poprawnie
- żółty – część czujników zwraca błąd
- czerwony – brak odczytów z aktywnych czujników

## Dalsze materiały
Szczegółowe opisy poszczególnych modułów znajdują się w plikach:
- `README_MovingAverages.md` – system średnich kroczących
- `DataType_Switching_Guide.md` – przełączanie typu danych
- `Register_Map_Guide.md` – mapa rejestrów Modbus
- `HISTORY_CALCULATIONS.md` – obliczenia dotyczące buforowania historii danych
