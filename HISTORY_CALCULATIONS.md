# Historia Czujników - Obliczenia Pamięci (1MB PSRAM)

## Założenia

- **Cel**: 1MB (1,048,576 bytes) pamięci PSRAM do przechowywania historii
- **Fast samples**: co 10 sekund, przechowywane przez 1 godzinę (360 próbek max)
- **Slow samples**: co 5 minut, przechowywane przez 24 godziny (288 próbek max)

## Rozmiary Struktur Danych

| Typ Czujnika | Rozmiar Danych | Wpis (+ timestamp) | Fast Samples | Slow Samples | Razem |
|--------------|----------------|-------------------|--------------|--------------|-------|
| Solar        | 80B            | 84B               | 300 × 84B    | 240 × 84B    | 45.4KB |
| I2C          | 20B            | 24B               | 360 × 24B    | 288 × 24B    | 15.6KB |
| SPS30        | 44B            | 48B               | 300 × 48B    | 240 × 48B    | 25.9KB |
| IPS          | 120B           | 124B              | 200 × 124B   | 150 × 124B   | 43.4KB |
| MCP3424      | 200B           | 204B              | 150 × 204B   | 120 × 204B   | 55.1KB |
| ADS1110      | 16B            | 20B               | 360 × 20B    | 288 × 20B    | 13.0KB |
| INA219       | 20B            | 24B               | 360 × 24B    | 288 × 24B    | 15.6KB |
| SHT40        | 16B            | 20B               | 360 × 20B    | 288 × 20B    | 13.0KB |
| Calibration  | 300B           | 304B              | 100 × 304B   | 80 × 304B    | 54.7KB |
| HCHO         | 32B            | 36B               | 360 × 36B    | 288 × 36B    | 23.3KB |

## Podsumowanie

- **Całkowite zużycie**: ~305KB (30% z 1MB budżetu)
- **Rezerwa**: ~743KB na przyszły rozwój
- **Czas przechowywania**:
  - Fast (10s): 1 godzina
  - Slow (5min): 24 godziny

## Przykłady Zapytań

### Via Serial Commands
```
HISTORY          # Status historii
STATUS           # Status systemu + historia
```

### Via HTTP API
```
GET /api/history?sensor=solar&timeRange=1h
GET /api/history?sensor=sps30&timeRange=6h  
GET /api/history?sensor=all&timeRange=24h
```

### Via WebSocket
Historia jest automatycznie uwzględniona w JSON:
```json
{
  "history": {
    "enabled": true,
    "memoryUsed": 312345,
    "memoryBudget": 1048576
  }
}
```

## Optymalizacje

1. **PSRAM First**: Próba alokacji w PSRAM, fallback do heap
2. **Kondycjonalne bufory**: Tylko aktywne czujniki alokują pamięć
3. **Circular buffers**: Automatyczne nadpisywanie starych danych
4. **Template-based**: Efektywne zarządzanie różnymi typami czujników

## Przyszłe Rozszerzenia

Z 743KB rezerwy można dodać:
- Kompresję danych historycznych
- Eksport do plików SD
- Zaawansowane zapytania z filtrami
- Agregacje (min/max/avg) w oknie czasowym
- Alerty oparte na trendach historycznych 