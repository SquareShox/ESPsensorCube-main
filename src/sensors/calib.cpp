#include "calib.h"
#include "calib_constants.h"
#include <Arduino.h>
#include <math.h>
#include "config.h"
#include "i2c_sensors.h"
#include "network_config.h"

// Global kalibrowane dane czujnikow
CalibratedSensorData calibratedData;

// Global konfiguracja kalibracji
CalibrationConfig calibConfig;
#define DEBUG

// External global MCP3424 data
extern MCP3424Data mcp3424Data;
// External global calibration constants
extern CalibrationConstants calibConstants;
//#define DEBUG
const char* gasTypes[] = {"NO", "O3", "NO2", "CO", "SO2", "TGS1", "TGS2", "TGS3", "NH3", "H2S"};
const char* gasNames[] = {"K1", "K2", "K3", "K4", "K5", "K6", "K7", "K8"};
// Implementacja wzorów z calib_functions.tcl

// Procedury kompensacji temperaturowej dla czujnikow elektrochemicznych
float A4_O3(float T) {
    int TS = ((int)(T/10)) * 10;
    int TE = TS + 10;
    float TCS = (TS <= 40) ? 0.18f : 2.87f;
    float TCE = (TE <= 40) ? 0.18f : 2.87f;
    return TCS + ((TCE - TCS) * ((T - TS) / 10.0f));
}

float B4_O3(float T) {
    int TS = ((int)(T/10)) * 10;
    int TE = TS + 10;
    float TCS, TCE;
    
    if (TS <= 10) TCS = -0.90f;
    else if (TS <= 20) TCS = -0.35f;
    else if (TS <= 30) TCS = 0.50f;
    else if (TS <= 40) TCS = 1.15f;
    else TCS = 1.80f;
    
    if (TE <= 10) TCE = -0.90f;
    else if (TE <= 20) TCE = -0.35f;
    else if (TE <= 30) TCE = 0.50f;
    else if (TE <= 40) TCE = 1.15f;
    else TCE = 1.80f;
    
    return TCS + ((TCE - TCS) * ((T - TS) / 10.0f));
}

// Obliczanie temperatury dla czujnikow B4
float B4_T(float RAW_T, float RAW_V) {
    float ratio = calibConstants.B4_RS / (calibConstants.B4_K * RAW_V / RAW_T - 1);
    return (1.0f / (1.0f / (calibConstants.B4_TO + calibConstants.B4_COK) + (1.0f / calibConstants.B4_B) * logf(ratio / calibConstants.B4_RO))) - calibConstants.B4_COK;
}

// Obliczanie napiecia dla czujnikow B4
float B4_mV(float V) {
    return calibConstants.B4_K * calibConstants.B4_LSB * V / 1000.0f;
}

// Obliczanie temperatury dla czujnikow TGS
float TGS_T(float C1, float C2) {
    return (1.0f / (1.0f / (calibConstants.TGS_TO + calibConstants.TGS_COK) + (1.0f / calibConstants.TGS_B) * logf(C1 / C2))) - calibConstants.TGS_COK;
}

// Obliczanie napiecia dla TGS v4 (k=5.0 zgodnie z calib_functions.tcl)
float TGSv4_mV(float C4) {
    const float TGS_K = 5.0f;
    return TGS_K * calibConstants.TGS_LSB * C4 / 1000.0f;
}

// Obliczanie napiecia dla TGS v4 (FIX) – zgodnie z calib_functions.tcl TGSv4_mV_FIX (k=5.0)
float TGSv4_mV_FIX(float C3, float C4) {
    const float TGS_K = 5.0f;
    return TGS_K * calibConstants.TGS_LSB * (C3 + C4) / 1000.0f;
}

// Obliczanie rezystancji TGS v4
float TGSv4_ohm(float C3, float C4, float RL) {
    return RL * ((C4 - C3) / (C4 + C3));
}

// Obliczanie ppm dla TGS v4
float TGSv4_ppm(float C3, float C4, float RL, float A, float B, float C, float b1, float a) {
    float R = TGSv4_ohm(C3, C4, RL);
    float Sr = A / (R + C) + B;
    return b1 * Sr + a;
}

// Korekcja liniowa podstawowa
float Linear_basic(float mul, float RAW, float add) {
    return mul * RAW + add;
}

// Korekcja liniowa zaawansowana
float Linear_advanced(float mul, float RAW, float add) {
    float a2_abs = 2.0f * fabs(add);
    float fx_nom = Linear_basic(mul, RAW, add);
    float fx_alt = (add == 0) ? (mul * RAW) : (mul * RAW) / (1.0f - (add / a2_abs));
    return (fx_nom >= a2_abs) ? fx_nom : fx_alt;
}

// Funkcja do odczytu wartosci z konkretnego kanalu MCP3424
// Teraz uzywa globalnej funkcji z network_config.h

// Dynamiczne przypisanie wartosci do pol Kx_* na podstawie indeksu urzadzenia MCP3424
static inline void assignKTemperatureByDeviceIndex(uint8_t deviceIndex, float temperatureC) {
    uint8_t kNumber = deviceIndex + 1; // K numer = index + 1 (Device 0 -> K1)
    switch (kNumber) {
        case 1: calibratedData.K1_temp = temperatureC; break;
        case 2: calibratedData.K2_temp = temperatureC; break;
        case 3: calibratedData.K3_temp = temperatureC; break;
        case 4: calibratedData.K4_temp = temperatureC; break;
        case 5: calibratedData.K5_temp = temperatureC; break;
        case 6: calibratedData.K6_temp = temperatureC; break;
        case 7: calibratedData.K7_temp = temperatureC; break;
        case 8: calibratedData.K8_temp = temperatureC; break;
        case 9: calibratedData.K9_temp = temperatureC; break;
        case 12: calibratedData.K12_temp = temperatureC; break;
        default: break;
    }
}

static inline void assignKVoltageByDeviceIndex(uint8_t deviceIndex, float millivolts) {
    uint8_t kNumber = deviceIndex + 1; // K numer = index + 1
    switch (kNumber) {
        case 1: calibratedData.K1_voltage = millivolts; break;
        case 2: calibratedData.K2_voltage = millivolts; break;
        case 3: calibratedData.K3_voltage = millivolts; break;
        case 4: calibratedData.K4_voltage = millivolts; break;
        case 5: calibratedData.K5_voltage = millivolts; break;
        case 6: calibratedData.K6_voltage = millivolts; break;
        case 7: calibratedData.K7_voltage = millivolts; break;
        case 8: calibratedData.K8_voltage = millivolts; break;
        case 9: calibratedData.K9_voltage = millivolts; break;
        case 12: calibratedData.K12_voltage = millivolts; break;
        default: break;
    }
}

// Kalibracja temperatury dla czujnikow B4 (dynamiczne przypisanie do Kx wg deviceIndex)
void calibrateB4Temperature() {
    extern MCP3424Config mcp3424Config;
    for (uint8_t i = 0; i < 8; i++) {
        if (!mcp3424Config.devices[i].enabled || !mcp3424Config.devices[i].autoDetected) continue;
        const char* gt = mcp3424Config.devices[i].gasType;
        if (strcmp(gt, "NO") == 0 || strcmp(gt, "O3") == 0 || strcmp(gt, "NO2") == 0 ||
            strcmp(gt, "CO") == 0 || strcmp(gt, "SO2") == 0 || strcmp(gt, "NH3") == 0 || strcmp(gt, "H2S") == 0) {
            float rawT = getMCP3424Value(i, 2);
            float rawV = getMCP3424Value(i, 3);
            float tempC = B4_T(rawT, rawV);
            assignKTemperatureByDeviceIndex(i, tempC);
        }
    }
}

// Kalibracja napiec dla czujnikow B4 (dynamiczne przypisanie do Kx wg deviceIndex)
void calibrateB4Voltage() {
    extern MCP3424Config mcp3424Config;
    for (uint8_t i = 0; i < 8; i++) {
        if (!mcp3424Config.devices[i].enabled || !mcp3424Config.devices[i].autoDetected) continue;
        const char* gt = mcp3424Config.devices[i].gasType;
        if (strcmp(gt, "NO") == 0 || strcmp(gt, "O3") == 0 || strcmp(gt, "NO2") == 0 ||
            strcmp(gt, "CO") == 0 || strcmp(gt, "SO2") == 0 || strcmp(gt, "NH3") == 0 || strcmp(gt, "H2S") == 0) {
            float mv = B4_mV(getMCP3424Value(i, 3));
            assignKVoltageByDeviceIndex(i, mv);
        }
    }
}

// Kalibracja temperatur czujnikow TGS (dynamiczne przypisanie do Kx wg deviceIndex)
void calibrateTGSTemperature() {
    extern MCP3424Config mcp3424Config;
    for (uint8_t i = 0; i < 8; i++) {
        if (!mcp3424Config.devices[i].enabled || !mcp3424Config.devices[i].autoDetected) continue;
        const char* gt = mcp3424Config.devices[i].gasType;
        if (strcmp(gt, "TGS1") == 0 || strcmp(gt, "TGS2") == 0 || strcmp(gt, "TGS3") == 0) {
            float c1 = getMCP3424Value(i, 0);
            float c2 = getMCP3424Value(i, 1);
            float t = TGS_T(c1, c2);
            assignKTemperatureByDeviceIndex(i, t);
        }
    }
}

// Kalibracja napiec czujnikow TGS (dynamiczne przypisanie do Kx wg deviceIndex)
void calibrateTGSVoltage() {
    extern MCP3424Config mcp3424Config;
    for (uint8_t i = 0; i < 8; i++) {
        if (!mcp3424Config.devices[i].enabled || !mcp3424Config.devices[i].autoDetected) continue;
        const char* gt = mcp3424Config.devices[i].gasType;
        if (strcmp(gt, "TGS1") == 0) {
            float c4 = getMCP3424Value(i, 3);
            assignKVoltageByDeviceIndex(i, TGSv4_mV(c4));
        } else if (strcmp(gt, "TGS2") == 0) {
            float c4 = getMCP3424Value(i, 3);
            assignKVoltageByDeviceIndex(i, TGSv4_mV(c4));
        } else if (strcmp(gt, "TGS3") == 0) {
            float c4 = getMCP3424Value(i, 3);
            assignKVoltageByDeviceIndex(i, TGSv4_mV(c4));
        }
    }
}

// Kalibracja czujnikow TGS
void calibrateTGSSensors() {
    // TGS03 (K6) - TGS1 (channel 2,3)
    int8_t deviceTGS1 = getMCP3424DeviceByGasType(gasTypes[5]);
    if (deviceTGS1 >= 0) {
        float K6_3 = getMCP3424Value(deviceTGS1, 2);
        float K6_4 = getMCP3424Value(deviceTGS1, 3);
        calibratedData.TGS03 = TGSv4_ppm(K6_3, K6_4, calibConstants.RL_TGS03, calibConstants.RL_TGS03, 0, 0, calibConstants.TGS03_B1, calibConstants.TGS03_A);
        calibratedData.TGS03_ohm = TGSv4_ohm(K6_3, K6_4, calibConstants.RL_TGS03);
    }
    
    // TGS02 (K7) - TGS2 (channel 2,3)
    int8_t deviceTGS2 = getMCP3424DeviceByGasType(gasTypes[6]);
    if (deviceTGS2 >= 0) {
        float K7_3 = getMCP3424Value(deviceTGS2, 2);
        float K7_4 = getMCP3424Value(deviceTGS2, 3);
        // TGSv4_ppm_FIX implementation zgodnie z calib.tcl linie 184-193
        float R_02 = TGSv4_ohm(K7_3, K7_4, calibConstants.RL_TGS02);
        // float Sr_02 = calibConstants.RL_TGS02 / (R_02 + 0) + 0; // A=RL_TGS02, B=0, C=0
        // calibratedData.TGS02 = calibConstants.TGS02_B1 * Sr_02 + calibConstants.TGS02_A;
        calibratedData.TGS02 = TGSv4_ppm(K7_3, K7_4, calibConstants.RL_TGS02, calibConstants.RL_TGS02, 0, 0, calibConstants.TGS02_B1, calibConstants.TGS02_A);
        calibratedData.TGS02_ohm = R_02;
    }
    
    // TGS12 - TGS3 (channel 2,3)
    int8_t deviceTGS3 = getMCP3424DeviceByGasType(gasTypes[7]);
    if (deviceTGS3 >= 0) {
        float Kx_3 = getMCP3424Value(deviceTGS3, 2);
        float Kx_4 = getMCP3424Value(deviceTGS3, 3);
        calibratedData.TGS12 = TGSv4_ppm(Kx_3, Kx_4, calibConstants.RL_TGS12, calibConstants.RL_TGS12, 0, 0, calibConstants.TGS12_B1, calibConstants.TGS12_A);
        calibratedData.TGS12_ohm = TGSv4_ohm(Kx_3, Kx_4, calibConstants.RL_TGS12);
    }
}

// Kalibracja gazow elektrochemicznych w ug/m3
void calibrateGases() {
    // CO (K4) - channel 0,1,2,3
    int8_t deviceCO = getMCP3424DeviceByGasType(gasTypes[3]);
    if (deviceCO >= 0) {
        float K4_1 = getMCP3424Value(deviceCO, 0); // WRK
        float K4_2 = getMCP3424Value(deviceCO, 1); // AUX  
        float K4_3 = getMCP3424Value(deviceCO, 2); // TRM
        float K4_4 = getMCP3424Value(deviceCO, 3); // VCC
        float T_CO = B4_T(K4_3, K4_4);
        calibratedData.CO = calibConstants.CO_B0 + calibConstants.CO_B1 * K4_1 + calibConstants.CO_B2 * K4_2 + calibConstants.CO_B3 * T_CO;
        calibratedData.CO = fmax(calibConstants.GAS_MIN, fmin(calibratedData.CO, calibConstants.GAS_MAX));
        
        #ifdef DEBUG
  
        #endif
    } else {
        #ifdef DEBUG
       
        #endif
    }
    
    // NO (K1) - channel 0,1
    int8_t deviceNO = getMCP3424DeviceByGasType(gasTypes[0]);
    if (deviceNO >= 0) {
        float K1_1 = getMCP3424Value(deviceNO, 0); // WRK
        float K1_2 = getMCP3424Value(deviceNO, 1); // AUX
        float T_NO = calibratedData.K1_temp;
        calibratedData.NO = calibConstants.NO_B0 + calibConstants.NO_B1 * K1_1 + calibConstants.NO_B2 * K1_2 + calibConstants.NO_B3 * T_NO;
        calibratedData.NO = fmax(calibConstants.GAS_MIN, fmin(calibratedData.NO, calibConstants.GAS_MAX));
    }
    
    // NO2 (K3) - channel 0,1
    int8_t deviceNO2 = getMCP3424DeviceByGasType(gasTypes[2]);
    if (deviceNO2 >= 0) {
        float K3_1 = getMCP3424Value(deviceNO2, 0); // WRK
        float K3_2 = getMCP3424Value(deviceNO2, 1); // AUX
        float T_NO2 = calibratedData.K3_temp;
        calibratedData.NO2 = calibConstants.NO2_B0 + calibConstants.NO2_B1 * K3_1 + calibConstants.NO2_B2 * K3_2 + calibConstants.NO2_B3 * T_NO2;
        // Zastosuj kompensacje temperatury dedykowana dla NO2
       // calibratedData.NO2 = Linear_basic(A4_NO2(T_NO2), calibratedData.NO2, B4_NO2(T_NO2));
        calibratedData.NO2 = fmax(calibConstants.GAS_MIN, fmin(calibratedData.NO2, calibConstants.GAS_MAX));
    }
    
    // O3 (K2) z kompensacja NO2 - channel 0,1
    int8_t deviceO3 = getMCP3424DeviceByGasType(gasTypes[1]);
    if (deviceO3 >= 0) {
        float K2_1 = getMCP3424Value(deviceO3, 0); // WRK
        float K2_2 = getMCP3424Value(deviceO3, 1); // AUX
        float T_O3 = calibratedData.K2_temp;
        calibratedData.O3 = calibConstants.O3_B0 + calibConstants.O3_B1 * K2_1 + calibConstants.O3_B2 * K2_2 + calibConstants.O3_B3 * T_O3 + calibConstants.O3_D * calibratedData.NO2;
        // Zastosuj kompensacje temperatury dedykowana dla O3
      //  calibratedData.O3 = Linear_basic(A4_O3(T_O3), calibratedData.O3, B4_O3(T_O3));
        calibratedData.O3 = fmax(calibConstants.GAS_MIN, fmin(calibratedData.O3, calibConstants.GAS_MAX));
    }
    
    // SO2 (K5) - channel 0,1,2,3
    int8_t deviceSO2 = getMCP3424DeviceByGasType(gasTypes[4]);
    if (deviceSO2 >= 0) {
        float K5_1 = getMCP3424Value(deviceSO2, 0); // WRK
        float K5_2 = getMCP3424Value(deviceSO2, 1); // AUX
        float K5_3 = getMCP3424Value(deviceSO2, 2); // TRM  
        float K5_4 = getMCP3424Value(deviceSO2, 3); // VCC
        float T_SO2 = B4_T(K5_3, K5_4);
        calibratedData.SO2 = calibConstants.SO2_B0 + calibConstants.SO2_B1 * K5_1 + calibConstants.SO2_B2 * K5_2 + calibConstants.SO2_B3 * T_SO2;
        // Zastosuj kompensacje temperatury dedykowana dla SO2
      //  calibratedData.SO2 = Linear_basic(A4_SO2(T_SO2), calibratedData.SO2, B4_SO2(T_SO2));
        calibratedData.SO2 = fmax(calibConstants.GAS_MIN, fmin(calibratedData.SO2, calibConstants.GAS_MAX));
        
    }
    
    // NH3 - używamy dedykowanego wzoru (może być osobny czujnik lub SO2)
    // Próbuj znaleźć dedykowany czujnik NH3, w przeciwnym razie użyj SO2
    int8_t deviceNH3 = getMCP3424DeviceByGasType(gasTypes[8]);
    
    if (deviceNH3 >= 0) {
        float Kx_1 = getMCP3424Value(deviceNH3, 0); // WRK
        float Kx_3 = getMCP3424Value(deviceNH3, 2); // TRM
        float Kx_4 = getMCP3424Value(deviceNH3, 3); // VCC
        float T_NH3 = B4_T(Kx_3, Kx_4);
        calibratedData.NH3 = calibConstants.NH3_B0 + calibConstants.NH3_B1 * Kx_1 + calibConstants.NH3_B3 * T_NH3;
        calibratedData.NH3 = fmax(calibConstants.GAS_MIN, fmin(calibratedData.NH3, calibConstants.GAS_MAX));
    }
    
    // H2S - używamy dedykowanego wzoru (może być osobny czujnik lub SO2)
    // Próbuj znaleźć dedykowany czujnik H2S, w przeciwnym razie użyj SO2
    int8_t deviceH2S = getMCP3424DeviceByGasType(gasTypes[9]);
    
    if (deviceH2S >= 0) {
        float Kx_1 = getMCP3424Value(deviceH2S, 0); // WRK
        float Kx_2 = getMCP3424Value(deviceH2S, 1); // AUX
        float Kx_3 = getMCP3424Value(deviceH2S, 2); // TRM
        float Kx_4 = getMCP3424Value(deviceH2S, 3); // VCC
        float T_H2S = B4_T(Kx_3, Kx_4);
        calibratedData.H2S = calibConstants.H2S_B0 + calibConstants.H2S_B1 * Kx_1 + calibConstants.H2S_B2 * Kx_2 + calibConstants.H2S_B3 * T_H2S;
        // Zastosuj kompensacje temperatury dedykowana dla H2S
      //  calibratedData.H2S = Linear_basic(A4_H2S(T_H2S), calibratedData.H2S, B4_H2S(T_H2S));
        calibratedData.H2S = fmax(calibConstants.GAS_MIN, fmin(calibratedData.H2S, calibConstants.GAS_MAX));
    }
}

// Konwersja gazow z ug/m3 na ppb
void calibrateGasesPPB() {
    // Używamy stałych konwersji z calibConstants
    extern CalibrationConstants calibConstants;
    
    // Konwersja na ppb - używamy stałych z calibConstants
    // Dla uproszczenia używamy standardowych współczynników konwersji
    calibratedData.CO_ppb = calibratedData.CO / 1.16f;  // CO conversion factor
    calibratedData.NO_ppb = calibratedData.NO / 1.247f; // NO conversion factor
    calibratedData.NO2_ppb = calibratedData.NO2 / 1.913f; // NO2 conversion factor
    calibratedData.O3_ppb = calibratedData.O3 / 2.0f;   // O3 conversion factor
    calibratedData.SO2_ppb = calibratedData.SO2 / 2.66f; // SO2 conversion factor
    calibratedData.H2S_ppb = calibratedData.H2S / 1.4166f; // H2S conversion factor
    calibratedData.NH3_ppb = calibratedData.NH3 / 0.7079f; // NH3 conversion factor
    
    // VOC w ug/m3 - suma NH3 + H2S + SO2 (zgodnie z calib.tcl)
  //  calibratedData.VOC = calibratedData.NH3 + calibratedData.H2S + calibratedData.SO2;
  //  calibratedData.VOC = fmax(calibConstants.GAS_MIN, fmin(calibratedData.VOC, calibConstants.GAS_MAX));
    
    // TVOC w ppb - używamy TGS02 (zgodnie z calib.tcl linia 423)
    calibratedData.VOC_ppb = calibratedData.TGS02;
    
    // ODO (Odor Detection Unit) - wzór z calib.tcl linie 656-668
    // ODO = A0 + (A1 * TGS02) + (A2 * TGS03) + (A3 * TGS12) + (A4 * H2S) + (A5 * NH3)
    calibratedData.ODO = calibConstants.ODO_A0 + 
                        (calibConstants.ODO_A1 * calibratedData.TGS02) + 
                        (calibConstants.ODO_A2 * calibratedData.TGS03) + 
                        (calibConstants.ODO_A3 * calibratedData.TGS12) + 
                        (calibConstants.ODO_A4 * calibratedData.H2S) + 
                        (calibConstants.ODO_A5 * calibratedData.NH3);
    
    // Ograniczenie wartości ODO zgodnie z calib_constants.h
    calibratedData.ODO = fmax(calibConstants.ODO_MIN, fmin(calibratedData.ODO, calibConstants.ODO_MAX));
}

// Kalibracja HCHO i PID
void calibrateSpecialSensors() {
    // HCHO - używamy danych z czujnika CB-HCHO-V4
    extern HCHOData hchoData;
    extern bool hchoSensorStatus;
    
    if (hchoSensorStatus && hchoData.valid) {
        // Przeliczamy HCHO z mg/m3 na ppb (1 mg/m3 = 813 ppb dla HCHO w 25C, 1013hPa)
        calibratedData.HCHO = hchoData.hcho_ppb;

        // Kalibracja z parametrami z calib_constants.h
        calibratedData.HCHO = Linear_basic(calibConstants.HCHO_B1, calibratedData.HCHO, calibConstants.HCHO_A);
        
        // Ograniczenie wartosci zgodnie z calib_constants.h
        calibratedData.HCHO = fmax(calibConstants.HCHO_MIN, fmin(calibratedData.HCHO, calibConstants.HCHO_MAX));

        // TVOC i VOC z HCHO sensora (jesli dostepne):
        // - tvoc (mg/m3) bezposrednio z sensora
        // - VOC (ug/m3) - konwersja mg/m3 -> ug/m3
        // - VOC_ppb (ppb) pozostaje z TGS02 (calib.tcl)
        // Uwaga: jezeli HCHO tvoc nieobecne, pozostaje z poprzednich zrodel
        // Tutaj aktualizujemy tylko gdy wartosc jest > 0
        if (hchoData.tvoc > 0) {
            float tvoc_ugm3 = hchoData.tvoc * 1000.0f;
            calibratedData.VOC = tvoc_ugm3; // nadpisujemy VOC ug/m3 wartoscia z HCHO
            calibratedData.VOC = fmax(calibConstants.GAS_MIN, fmin(calibratedData.VOC, calibConstants.GAS_MAX));
        }
        
    } else {
        // Brak danych z czujnika HCHO
        calibratedData.HCHO = 0.0f;
    }
    
    // PID-AH v8 - wykorzystujemy TGS1
    int8_t deviceTGS1 = getMCP3424DeviceByGasType(gasTypes[5]);
    if (deviceTGS1 >= 0) {
        float WRK = getMCP3424Value(deviceTGS1, 2); // WRK
        float OFS = getMCP3424Value(deviceTGS1, 3); // OFS
        float mv = (WRK + OFS) / 256.0f;
        
        calibratedData.PID = ((mv - calibConstants.PID_OFFSET) * calibConstants.PID_A + calibConstants.PID_B) / calibConstants.PID_CF;
        calibratedData.PID = fmax(calibConstants.PID_MIN, fmin(calibratedData.PID, calibConstants.PID_MAX));
        calibratedData.PID_mV = mv;
    }
}

// Dodatkowe funkcje kompensacji temperaturowej
float A4_SO2(float T) {
    int TS = ((int)(T/10)) * 10;
    int TE = TS + 10;
    float TCS, TCE;
    
    if (TS <= 10) TCS = 0.85f;
    else if (TS <= 20) TCS = 1.15f;
    else if (TS <= 30) TCS = 1.45f;
    else if (TS <= 40) TCS = 1.75f;
    else TCS = 1.95f;
    
    if (TE <= 10) TCE = 0.85f;
    else if (TE <= 20) TCE = 1.15f;
    else if (TE <= 30) TCE = 1.45f;
    else if (TE <= 40) TCE = 1.75f;
    else TCE = 1.95f;
    
    return TCS + ((TCE - TCS) * ((T - TS) / 10.0f));
}

float B4_SO2(float T) {
    return 1.0f; // Stala wartosc dla B4_SO2
}

float A4_NO2(float T) {
    int TS = ((int)(T/10)) * 10;
    int TE = TS + 10;
    float TCS = (TS <= 30) ? 1.18f : (TS <= 40) ? 2.00f : 2.70f;
    float TCE = (TE <= 30) ? 1.18f : (TE <= 40) ? 2.00f : 2.70f;
    return TCS + ((TCE - TCS) * ((T - TS) / 10.0f));
}

float B4_NO2(float T) {
    return 0.70f; // Stala wartosc dla B4_NO2
}

// Dodatkowe funkcje kompensacji dla H2S
float A4_H2S(float T) {
    return 0.65f; // Stala wartosc dla A4_H2S
}

float B4_H2S(float T) {
    int TS = ((int)(T/10)) * 10;
    int TE = TS + 10;
    float TCS = (TS <= 0) ? 1.85f : -1.80f;
    float TCE = (TE <= 0) ? 1.85f : -1.80f;
    return TCS + ((TCE - TCS) * ((T - TS) / 10.0f));
}

// Funkcja clamp do ograniczania wartosci
float clamp(float value, float min_val, float max_val) {
    return fmax(min_val, fmin(value, max_val));
}

// Glowna funkcja kalibracji
void performCalibration() {
    // DEBUG: start kalibracji
    //#define DEBUG

 

    // Sprawdz czy kalibracja jest wlaczona
    if (!calibConfig.enableCalibration) {
      
        calibratedData.valid = false;
        return;
    }
    
// Kalibracja HCHO i PID (kontrolowane przez config)
    if (calibConfig.enableSpecialSensors) {

        calibrateSpecialSensors();
    }
    
    // Kalibracja czujnikow PM (kontrolowane przez config)
    if (calibConfig.enableSpecialSensors) {
        calibratePMSensors();
    }
    
    // Kalibracja czujnikow środowiskowych (kontrolowane przez config)  
    if (calibConfig.enableSpecialSensors) {
        calibrateEnvironmentalSensors();
    }
    
    // Kalibracja CO2 (kontrolowane przez config)
    if (calibConfig.enableSpecialSensors) {
        calibrateCO2Sensor();
    }
        // Kalibracja HCHO i PID (kontrolowane przez config)
    if (calibConfig.enableSpecialSensors) {

        calibrateSpecialSensors();
    }
    
    // Kalibracja czujnikow PM (kontrolowane przez config)
    if (calibConfig.enableSpecialSensors) {
        calibratePMSensors();
    }
    
    // Kalibracja czujnikow środowiskowych (kontrolowane przez config)  
    if (calibConfig.enableSpecialSensors) {
        calibrateEnvironmentalSensors();
    }
    
    // Kalibracja CO2 (kontrolowane przez config)
    if (calibConfig.enableSpecialSensors) {
        calibrateCO2Sensor();
    }
    

    if (mcp3424Data.deviceCount > 0 && (millis() - mcp3424Data.lastUpdate) < 5000) {
        // Brak danych lub dane nieaktualne

         // Kalibracja podstawowych pomiarow (zawsze wlaczone gdy kalibracja aktywna)

    calibrateB4Temperature();
    calibrateB4Voltage();
    calibrateTGSTemperature();
    calibrateTGSVoltage();
    
    // Kalibracja czujnikow TGS (kontrolowane przez config)
        if (calibConfig.enableTGSSensors) {
   
            calibrateTGSSensors();
        }
        
        // Kalibracja gazow elektrochemicznych (kontrolowane przez config)
        if (calibConfig.enableGasSensors) {
         
            calibrateGases();
            
            // Konwersja na ppb (tylko jesli gazy wlaczone)
            if (calibConfig.enablePPBConversion) {
             
                calibrateGasesPPB();
            }
        }
        //calibratedData.valid = false;
       
    }
    
   
    
    
    
    calibratedData.valid = true;
    calibratedData.lastUpdate = millis();

    // Periodic debug logging (every 10s) for calibration outputs
    // Focus on ohms and ppb/ppm; skip PM and SCD as requested
    #ifdef DEBUG
    {
        static unsigned long lastCalibDebug = 0;
        unsigned long now = millis();
        if (now - lastCalibDebug >= 10000) {
            lastCalibDebug = now;
            if (calibratedData.valid) {
                Serial.println("[CALIB] --- Periodic (10s) calibration snapshot ---");
                // Ohms from TGS sensors (if computed)
                Serial.print("[CALIB] Ohms: ");
                Serial.print("TGS02_ohm="); Serial.print(calibratedData.TGS02_ohm, 2); Serial.print(", ");
                Serial.print("TGS03_ohm="); Serial.print(calibratedData.TGS03_ohm, 2); Serial.print(", ");
                Serial.print("TGS12_ohm="); Serial.println(calibratedData.TGS12_ohm, 2);

                // Gases (ug/m3) - main calibrated values
                Serial.print("[CALIB] ug/m3: ");
                Serial.print("CO="); Serial.print(calibratedData.CO, 2); Serial.print(", ");
                Serial.print("NO="); Serial.print(calibratedData.NO, 2); Serial.print(", ");
                Serial.print("NO2="); Serial.print(calibratedData.NO2, 2); Serial.print(", ");
                Serial.print("O3="); Serial.print(calibratedData.O3, 2); Serial.print(", ");
                Serial.print("SO2="); Serial.print(calibratedData.SO2, 2); Serial.print(", ");
                Serial.print("H2S="); Serial.print(calibratedData.H2S, 2); Serial.print(", ");
                Serial.print("NH3="); Serial.print(calibratedData.NH3, 2); Serial.print(", ");
                Serial.print("VOC(ug/m3)="); Serial.println(calibratedData.VOC, 2);

                // Gases (ppb)
                Serial.print("[CALIB] ppb: ");
                Serial.print("CO_ppb="); Serial.print(calibratedData.CO_ppb, 1); Serial.print(", ");
                Serial.print("NO_ppb="); Serial.print(calibratedData.NO_ppb, 1); Serial.print(", ");
                Serial.print("NO2_ppb="); Serial.print(calibratedData.NO2_ppb, 1); Serial.print(", ");
                Serial.print("O3_ppb="); Serial.print(calibratedData.O3_ppb, 1); Serial.print(", ");
                Serial.print("SO2_ppb="); Serial.print(calibratedData.SO2_ppb, 1); Serial.print(", ");
                Serial.print("H2S_ppb="); Serial.print(calibratedData.H2S_ppb, 1); Serial.print(", ");
                Serial.print("NH3_ppb="); Serial.print(calibratedData.NH3_ppb, 1); Serial.print(", ");
                Serial.print("VOC_ppb="); Serial.println(calibratedData.VOC_ppb, 1);

                // Specials
                Serial.print("[CALIB] HCHO_ppb="); Serial.print(calibratedData.HCHO, 1);
                Serial.print(", ODO="); Serial.println(calibratedData.ODO, 1);
            } else {
                Serial.println("[CALIB] Data invalid - skipping snapshot");
            }
        }
    }
    #endif
}

// Funkcje dostepowe
CalibratedSensorData getCalibratedData() {
    return calibratedData;
}

bool isCalibratedDataValid() {
    return calibratedData.valid && (millis() - calibratedData.lastUpdate) < 10000;
}

// Funkcje konfiguracji kalibracji
void setCalibrationConfig(const CalibrationConfig& config) {
    calibConfig = config;
    Serial.println("Calibration config updated:");
    Serial.printf("  - Calibration enabled: %s\n", calibConfig.enableCalibration ? "true" : "false");
    Serial.printf("  - Moving averages: %s\n", calibConfig.enableMovingAverages ? "true" : "false");
    Serial.printf("  - TGS sensors: %s\n", calibConfig.enableTGSSensors ? "true" : "false");
    Serial.printf("  - Gas sensors: %s\n", calibConfig.enableGasSensors ? "true" : "false");
    Serial.printf("  - PPB conversion: %s\n", calibConfig.enablePPBConversion ? "true" : "false");
    Serial.printf("  - Special sensors: %s\n", calibConfig.enableSpecialSensors ? "true" : "false");
}

CalibrationConfig getCalibrationConfig() {
    return calibConfig;
}

// Kalibracja czujnikow PM z SPS30 - zgodnie z calib.tcl linie 1017-1048
void calibratePMSensors() {
    extern SPS30Data sps30Data;
    extern bool sps30SensorStatus;
    
    if (sps30SensorStatus && sps30Data.valid) {
        // Kalibracja PM1 ug/m3 - calib.tcl linie 1039-1048
        calibratedData.PM1 = Linear_advanced(calibConstants.PM1_A, sps30Data.pm1_0, calibConstants.PM1_B);
        calibratedData.PM1 = fmax(calibConstants.PM_MIN, fmin(calibratedData.PM1, calibConstants.PM_MAX));
        
        // Kalibracja PM25 ug/m3 - calib.tcl linie 1028-1037
        calibratedData.PM25 = Linear_advanced(calibConstants.PM25_A, sps30Data.pm2_5, calibConstants.PM25_B);
        calibratedData.PM25 = fmax(calibConstants.PM_MIN, fmin(calibratedData.PM25, calibConstants.PM_MAX));
        
        // Kalibracja PM10 ug/m3 - calib.tcl linie 1017-1026
        calibratedData.PM10 = Linear_advanced(calibConstants.PM10_A, sps30Data.pm10, calibConstants.PM10_B);
        calibratedData.PM10 = fmax(calibConstants.PM_MIN, fmin(calibratedData.PM10, calibConstants.PM_MAX));
        
        #ifdef DEBUG
        // Serial.printf("PM Calibration - PM1=%.2f, PM25=%.2f, PM10=%.2f\n", 
        //              calibratedData.PM1, calibratedData.PM25, calibratedData.PM10);
        #endif
    } else {
        // Brak danych z SPS30
        calibratedData.PM1 = 0.0f;
        calibratedData.PM25 = 0.0f;
        calibratedData.PM10 = 0.0f;
    }
}

// Kalibracja czujnikow środowiskowych - zgodnie z calib.tcl linie 406-472
void calibrateEnvironmentalSensors() {
    extern I2CSensorData i2cSensorData;
    extern SHT40Data sht40Data;
    extern bool sht40SensorStatus;
    
    // Główne dane środowiskowe - I2C10 (SHT40 lub podobny)
    if (sht40SensorStatus && sht40Data.valid) {
        // Temperatura zewnętrzna - calib.tcl linie 406-415
        calibratedData.AMBIENT_TEMP = Linear_basic(calibConstants.AMBIENT_TEMP_A, sht40Data.temperature, calibConstants.AMBIENT_TEMP_B);
        calibratedData.AMBIENT_TEMP = fmax(calibConstants.ENV_MIN, fmin(calibratedData.AMBIENT_TEMP, calibConstants.ENV_MAX));
        
        // Wilgotność zewnętrzna - calib.tcl linie 417-426
        calibratedData.AMBIENT_HUMID = Linear_advanced(calibConstants.AMBIENT_HUMID_A, sht40Data.humidity, calibConstants.AMBIENT_HUMID_B);
        calibratedData.AMBIENT_HUMID = fmax(calibConstants.ENV_MIN, fmin(calibratedData.AMBIENT_HUMID, calibConstants.ENV_MAX));
        
        // Ciśnienie zewnętrzne - calib.tcl linie 450-459
        calibratedData.AMBIENT_PRESS = Linear_basic(calibConstants.AMBIENT_PRESS_A, sht40Data.pressure, calibConstants.AMBIENT_PRESS_B);
        calibratedData.AMBIENT_PRESS = fmax(calibConstants.ENV_MIN, fmin(calibratedData.AMBIENT_PRESS, calibConstants.ENV_MAX));
        
        #ifdef DEBUG
        // Serial.printf("Ambient: T=%.2f°C, RH=%.2f%%, P=%.2f hPa\n", 
        //              calibratedData.AMBIENT_TEMP, calibratedData.AMBIENT_HUMID, calibratedData.AMBIENT_PRESS);
        #endif
    } else if (i2cSensorData.valid) {
        // Fallback do głównych danych I2C
        calibratedData.AMBIENT_TEMP = Linear_basic(calibConstants.AMBIENT_TEMP_A, i2cSensorData.temperature, calibConstants.AMBIENT_TEMP_B);
        calibratedData.AMBIENT_HUMID = Linear_advanced(calibConstants.AMBIENT_HUMID_A, i2cSensorData.humidity, calibConstants.AMBIENT_HUMID_B);
        calibratedData.AMBIENT_PRESS = Linear_basic(calibConstants.AMBIENT_PRESS_A, i2cSensorData.pressure, calibConstants.AMBIENT_PRESS_B);
    } else {
        // Brak danych środowiskowych
        calibratedData.AMBIENT_TEMP = 0.0f;
        calibratedData.AMBIENT_HUMID = 0.0f;
        calibratedData.AMBIENT_PRESS = 0.0f;
    }
    
    // TODO: Dodać kalibracje DUST_ i GAS_ gdy będą dostępne czujniki I2C11 i I2C12
    // Na razie kopiujemy dane z głównego czujnika z odpowiednimi parametrami kalibracyjnymi
    calibratedData.DUST_TEMP = Linear_basic(calibConstants.DUST_TEMP_A, sht40Data.temperature, calibConstants.DUST_TEMP_B);
    calibratedData.DUST_TEMP = fmax(calibConstants.ENV_MIN, fmin(calibratedData.DUST_TEMP, calibConstants.ENV_MAX));
    
    calibratedData.DUST_HUMID = Linear_advanced(calibConstants.DUST_HUMID_A, sht40Data.humidity, calibConstants.DUST_HUMID_B);
    calibratedData.DUST_HUMID = fmax(calibConstants.ENV_MIN, fmin(calibratedData.DUST_HUMID, calibConstants.ENV_MAX));
    
    calibratedData.DUST_PRESS = Linear_basic(calibConstants.DUST_PRESS_A, sht40Data.pressure, calibConstants.DUST_PRESS_B);
    calibratedData.DUST_PRESS = fmax(calibConstants.ENV_MIN, fmin(calibratedData.DUST_PRESS, calibConstants.ENV_MAX));
    
    calibratedData.GAS_TEMP = Linear_basic(calibConstants.GAS_TEMP_A, calibratedData.AMBIENT_TEMP, calibConstants.GAS_TEMP_B);
    calibratedData.GAS_TEMP = fmax(calibConstants.ENV_MIN, fmin(calibratedData.GAS_TEMP, calibConstants.ENV_MAX));
    
    calibratedData.GAS_HUMID = Linear_advanced(calibConstants.GAS_HUMID_A, calibratedData.AMBIENT_HUMID, calibConstants.GAS_HUMID_B);
    calibratedData.GAS_HUMID = fmax(calibConstants.ENV_MIN, fmin(calibratedData.GAS_HUMID, calibConstants.ENV_MAX));
    
    calibratedData.GAS_PRESS = Linear_basic(calibConstants.GAS_PRESS_A, calibratedData.AMBIENT_PRESS, calibConstants.GAS_PRESS_B);
    calibratedData.GAS_PRESS = fmax(calibConstants.ENV_MIN, fmin(calibratedData.GAS_PRESS, calibConstants.ENV_MAX));
}

// Kalibracja CO2 z SCD41 - zgodnie z calib.tcl linie 523-542 (dedykowany kubelek SCD41)
void calibrateCO2Sensor() {
    extern SCD41Data scd41Data;
    extern bool scd41SensorStatus;
    
    if (scd41SensorStatus && scd41Data.valid) {
        // SCD CO2 - calib.tcl linie 537-542
        calibratedData.SCD_CO2 = Linear_basic(calibConstants.SCD_CO2_A, scd41Data.co2, calibConstants.SCD_CO2_B);
        calibratedData.SCD_CO2 = fmax(calibConstants.SCD_CO2_MIN, fmin(calibratedData.SCD_CO2, calibConstants.SCD_CO2_MAX));
        
        // SCD T - calib.tcl linie 523-528 (natywna temperatura z SCD41)
        calibratedData.SCD_T = Linear_basic(calibConstants.SCD_T_A, scd41Data.temperature, calibConstants.SCD_T_B);
        calibratedData.SCD_T = fmax(calibConstants.SCD_T_MIN, fmin(calibratedData.SCD_T, calibConstants.SCD_T_MAX));
        
        // SCD RH - calib.tcl linie 530-535 (natywna wilgotnosc z SCD41)
        calibratedData.SCD_RH = Linear_advanced(calibConstants.SCD_RH_A, scd41Data.humidity, calibConstants.SCD_RH_B);
        calibratedData.SCD_RH = fmax(calibConstants.SCD_RH_MIN, fmin(calibratedData.SCD_RH, calibConstants.SCD_RH_MAX));
        
        #ifdef DEBUG
        // Serial.printf("SCD41: CO2=%.0f ppm, T=%.2f°C, RH=%.2f%%\n", 
        //              calibratedData.SCD_CO2, calibratedData.SCD_T, calibratedData.SCD_RH);
        #endif
    } else {
        // Brak danych z SCD41
        calibratedData.SCD_CO2 = 0.0f;
        calibratedData.SCD_T = 0.0f;
        calibratedData.SCD_RH = 0.0f;
    }
}
