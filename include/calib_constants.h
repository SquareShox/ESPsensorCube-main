#ifndef CALIB_CONSTANTS_H
#define CALIB_CONSTANTS_H

// =============================================================================
// STALE KALIBRACYJNE - LATWE DO EDYCJI
// =============================================================================

// Rezystory obciazajace dla czujnikow TGS (ohm)
#define RL_TGS03     10000.0f    // TGS03 Load Resistance
#define RL_TGS02     3300.0f     // TGS02 Load Resistance  
#define RL_TGS12     10000.0f    // TGS12 Load Resistance

// Parametry kalibracji czujnikow TGS (ppm)
#define TGS03_B1     30.0f       // TGS03 multiplier
#define TGS03_A      -1.0f       // TGS03 offset
#define TGS02_B1     50.0f       // TGS02 multiplier (FIX version)
#define TGS02_A      4.0f        // TGS02 offset (FIX version)
#define TGS12_B1     1000.0f     // TGS12 multiplier
#define TGS12_A      0.0f        // TGS12 offset

// Parametry kalibracji gazow elektrochemicznych (ug/m3)
// CO (K4)
#define CO_B0        70.0f       // CO baseline
#define CO_B1        0.4f        // CO WRK coefficient
#define CO_B2        -0.4f       // CO AUX coefficient
#define CO_B3        0.0f        // CO temperature coefficient

// NO (K1)
#define NO_B0        0.2f        // NO baseline
#define NO_B1        0.03f       // NO WRK coefficient
#define NO_B2        -0.03f      // NO AUX coefficient
#define NO_B3        0.0f        // NO temperature coefficient

// NO2 (K3)
#define NO2_B0       20.0f       // NO2 baseline
#define NO2_B1       -0.09f      // NO2 WRK coefficient
#define NO2_B2       -0.063f     // NO2 AUX coefficient
#define NO2_B3       0.0f        // NO2 temperature coefficient

// O3 (K2) z kompensacja NO2
#define O3_B0        -20.0f      // O3 baseline
#define O3_B1        -0.09f      // O3 WRK coefficient
#define O3_B2        -0.054f     // O3 AUX coefficient
#define O3_B3        0.0f        // O3 temperature coefficient
#define O3_D         -1.35f      // O3 NO2 compensation coefficient

// SO2
#define SO2_B0       4.0f        // SO2 baseline
#define SO2_B1       0.005f      // SO2 WRK coefficient
#define SO2_B2       -0.008f     // SO2 AUX coefficient
#define SO2_B3       0.0f        // SO2 temperature coefficient

// NH3
#define NH3_B0       0.2f        // NH3 baseline
#define NH3_B1       0.00014f    // NH3 WRK coefficient
#define NH3_B3       0.0f        // NH3 temperature coefficient

// H2S
#define H2S_B0       0.2f        // H2S baseline
#define H2S_B1       0.00005f    // H2S WRK coefficient
#define H2S_B2       -0.00009f   // H2S AUX coefficient
#define H2S_B3       0.0f        // H2S temperature coefficient

// Wspolczynniki konwersji ug/m3 na ppb
#define CO_PPB_DIV   1.16f       // CO conversion factor
#define NO_PPB_DIV   1.247f      // NO conversion factor
#define NO2_PPB_DIV  1.913f      // NO2 conversion factor
#define O3_PPB_DIV   2.0f        // O3 conversion factor
#define SO2_PPB_DIV  2.66f       // SO2 conversion factor
#define H2S_PPB_DIV  1.4166f     // H2S conversion factor
#define NH3_PPB_DIV  0.7079f     // NH3 conversion factor

// Parametry kalibracji PID-AH v8
#define PID_OFFSET   140.0f      // PID baseline offset
#define PID_B        0.0f        // PID linear coefficient
#define PID_A        3.0f        // PID multiplier
#define PID_CF       1.0f        // PID correction factor

// Parametry kalibracji HCHO (PMS5003ST)
#define HCHO_B1      1.0f        // HCHO linear multiplier
#define HCHO_A       0.0f        // HCHO offset
#define HCHO_PPB_CF  0.814f      // HCHO PPB conversion factor

// Ograniczenia wartosci (clamp limits)
#define TEMP_MIN     -30.0f      // Minimum temperature [°C]
#define TEMP_MAX     1000.0f     // Maximum temperature [°C]
#define VOLTAGE_MIN  0.0f        // Minimum voltage [mV]
#define VOLTAGE_MAX  7000.0f     // Maximum voltage [mV]
#define GAS_MIN      0.0f        // Minimum gas concentration [ug/m3]
#define GAS_MAX      50000.0f    // Maximum gas concentration [ug/m3]
#define PPB_MIN      0.0f        // Minimum gas concentration [ppb]
#define PPB_MAX      50000.0f    // Maximum gas concentration [ppb]
#define TGS_MIN      0.0f        // Minimum TGS value [ppm]
#define TGS_MAX      1000000.0f  // Maximum TGS value [ppm]
#define HCHO_MIN     0.0f        // Minimum HCHO [ppb]
#define HCHO_MAX     40000.0f    // Maximum HCHO [ppb]
#define PID_MIN      0.0f        // Minimum PID [ppm]
#define PID_MAX      40000.0f    // Maximum PID [ppm]

// Parametry fizyczne dla obliczen temperatury
#define B4_TO        25.0f       // Reference temperature [°C]
#define B4_B         3380.0f     // Beta coefficient
#define B4_RO        10.0f       // Reference resistance [kΩ]
#define B4_RS        33.0f       // Series resistance [kΩ]
#define B4_K         3.2f        // Voltage divider ratio
#define B4_COK       273.15f     // Celsius to Kelvin offset

#define TGS_TO       25.0f       // TGS reference temperature [°C]
#define TGS_B        3380.0f     // TGS Beta coefficient
#define TGS_RO       10.0f       // TGS reference resistance [kΩ]
#define TGS_COK      273.15f     // Celsius to Kelvin offset

// Parametry ADC
#define B4_LSB       3.90625f    // ADC LSB value for B4 sensors
#define TGS_LSB      3.90625f    // ADC LSB value for TGS sensors
#define TGS_K        5.0f        // TGS voltage divider ratio

#endif // CALIB_CONSTANTS_H 