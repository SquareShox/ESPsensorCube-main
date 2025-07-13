#include "calib.h"
#include "calib_constants.h"
#include <Arduino.h>
#include <math.h>
#include "config.h"
#include "i2c_sensors.h"

// Global kalibrowane dane czujnikow
CalibratedSensorData calibratedData;

// Global konfiguracja kalibracji
CalibrationConfig calibConfig;

// External global MCP3424 data
extern MCP3424Data mcp3424Data;

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
    float ratio = B4_RS / (B4_K * RAW_V / RAW_T - 1);
    return (1.0f / (1.0f / (B4_TO + B4_COK) + (1.0f / B4_B) * logf(ratio / B4_RO))) - B4_COK;
}

// Obliczanie napiecia dla czujnikow B4
float B4_mV(float V) {
    return B4_K * B4_LSB * V / 1000.0f;
}

// Obliczanie temperatury dla czujnikow TGS
float TGS_T(float C1, float C2) {
    return (1.0f / (1.0f / (TGS_TO + TGS_COK) + (1.0f / TGS_B) * logf(C1 / C2))) - TGS_COK;
}

// Obliczanie napiecia dla TGS v4
float TGSv4_mV(float C4) {
    return TGS_K * TGS_LSB * C4 / 1000.0f;
}

// Obliczanie rezystancji TGS v4
float TGSv4_ohm(float C3, float C4, float RL) {
    return RL * (C4 - C3) / (C4 + C3);
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
float getMCP3424Value(uint8_t deviceIndex, uint8_t channel) {
    if (deviceIndex >= mcp3424Data.deviceCount || channel >= 4) {
        return 0.0f;
    }
    if (!mcp3424Data.valid[deviceIndex]) {
        return 0.0f;
    }
    return mcp3424Data.channels[deviceIndex][channel];
}

// Kalibracja temperatury dla czujników K1-K5 (B4)
void calibrateB4Temperature() {
    // K1 temperatura (device 0, channels 2,3)
    float K1_3 = getMCP3424Value(0, 2); // RAW_T
    float K1_4 = getMCP3424Value(0, 3); // RAW_V
    calibratedData.K1_temp = B4_T(K1_3, K1_4);
    
    // K2 temperatura (device 1, channels 2,3)
    float K2_3 = getMCP3424Value(1, 2);
    float K2_4 = getMCP3424Value(1, 3);
    calibratedData.K2_temp = B4_T(K2_3, K2_4);
    
    // K3 temperatura (device 2, channels 2,3)
    float K3_3 = getMCP3424Value(2, 2);
    float K3_4 = getMCP3424Value(2, 3);
    calibratedData.K3_temp = B4_T(K3_3, K3_4);
    
    // K4 temperatura (device 3, channels 2,3)
    float K4_3 = getMCP3424Value(3, 2);
    float K4_4 = getMCP3424Value(3, 3);
    calibratedData.K4_temp = B4_T(K4_3, K4_4);
    
    // K5 temperatura (device 4, channels 2,3)
    float K5_3 = getMCP3424Value(4, 2);
    float K5_4 = getMCP3424Value(4, 3);
    calibratedData.K5_temp = B4_T(K5_3, K5_4);
}

// Kalibracja napiec dla czujników B4
void calibrateB4Voltage() {
    calibratedData.K1_voltage = B4_mV(getMCP3424Value(0, 3));
    calibratedData.K2_voltage = B4_mV(getMCP3424Value(1, 3));
    calibratedData.K3_voltage = B4_mV(getMCP3424Value(2, 3));
    calibratedData.K4_voltage = B4_mV(getMCP3424Value(3, 3));
    calibratedData.K5_voltage = B4_mV(getMCP3424Value(4, 3));
}

// Kalibracja temperatur czujnikow TGS (K6-K12)
void calibrateTGSTemperature() {
    // K6 temperatura (device 5, channels 0,1)
    float K6_1 = getMCP3424Value(0, 0); // C1
    float K6_2 = getMCP3424Value(0, 1); // C2
    calibratedData.K6_temp = TGS_T(K6_1, K6_2);
    
    // K7 temperatura (device 6, channels 0,1) 
    float K7_1 = getMCP3424Value(6, 0);
    float K7_2 = getMCP3424Value(6, 1);
    calibratedData.K7_temp = TGS_T(K7_1, K7_2);
    
    // K8 temperatura (device 7, channels 0,1)
    float K8_1 = getMCP3424Value(7, 0);
    float K8_2 = getMCP3424Value(7, 1);
    calibratedData.K8_temp = TGS_T(K8_1, K8_2);
    
    // Dla K9 i K12 uzywamy tych samych wzorów
    calibratedData.K9_temp = TGS_T(getMCP3424Value(0, 0), getMCP3424Value(0, 1));
    calibratedData.K12_temp = TGS_T(getMCP3424Value(1, 0), getMCP3424Value(1, 1));
}

// Kalibracja napiec czujnikow TGS
void calibrateTGSVoltage() {
    calibratedData.K6_voltage = TGSv4_mV(getMCP3424Value(0, 3));
    calibratedData.K7_voltage = TGSv4_mV(getMCP3424Value(6, 3));
    calibratedData.K8_voltage = TGSv4_mV(getMCP3424Value(7, 3));
    calibratedData.K9_voltage = TGSv4_mV(getMCP3424Value(3, 3));
    calibratedData.K12_voltage = TGSv4_mV(getMCP3424Value(1, 3));
}

// Kalibracja czujnikow TGS
void calibrateTGSSensors() {
    // TGS03 (K6) - zgodnie z calib.tcl
    float K6_3 = getMCP3424Value(0, 2);
    float K6_4 = getMCP3424Value(0, 3);
    calibratedData.TGS03 = TGSv4_ppm(K6_3, K6_4, RL_TGS03, RL_TGS03, 0, 0, TGS03_B1, TGS03_A);
    calibratedData.TGS03_ohm = TGSv4_ohm(K6_3, K6_4, RL_TGS03);
    
    // TGS02 (K7) - FIX version zgodnie z calib.tcl
    float K7_3 = getMCP3424Value(6, 2);
    float K7_4 = getMCP3424Value(6, 3);
    // TGSv4_ppm_FIX implementation
    float R_02 = RL_TGS02 * (2.0f * K7_3 + K7_4) / K7_4;
    float Sr_02 = RL_TGS02 / (R_02 + 0) + 0;
    calibratedData.TGS02 = TGS02_B1 * Sr_02 + TGS02_A;
    calibratedData.TGS02_ohm = R_02;
    
    // TGS12 - przykładowa implementacja
    float Kx_3 = getMCP3424Value(1, 2);
    float Kx_4 = getMCP3424Value(1, 3);
    calibratedData.TGS12 = TGSv4_ppm(Kx_3, Kx_4, RL_TGS12, RL_TGS12, 0, 0, TGS12_B1, TGS12_A);
    calibratedData.TGS12_ohm = TGSv4_ohm(Kx_3, Kx_4, RL_TGS12);
}

// Kalibracja gazow elektrochemicznych w ug/m3
void calibrateGases() {
    // CO (K4) - zgodnie z calib.tcl
    float K4_1 = getMCP3424Value(3, 0); // WRK
    float K4_2 = getMCP3424Value(3, 1); // AUX  
    float K4_3 = getMCP3424Value(3, 2); // TRM
    float K4_4 = getMCP3424Value(3, 3); // VCC
    float T_CO = B4_T(K4_3, K4_4);
    calibratedData.CO = CO_B0 + CO_B1 * K4_1 + CO_B2 * K4_2 + CO_B3 * T_CO;
    calibratedData.CO = fmax(GAS_MIN, fmin(calibratedData.CO, GAS_MAX));
    
    // NO (K1)
    float K1_1 = getMCP3424Value(0, 0); // WRK
    float K1_2 = getMCP3424Value(0, 1); // AUX
    float T_NO = calibratedData.K1_temp;
    calibratedData.NO = NO_B0 + NO_B1 * K1_1 + NO_B2 * K1_2 + NO_B3 * T_NO;
    calibratedData.NO = fmax(GAS_MIN, fmin(calibratedData.NO, GAS_MAX));
    
    // NO2 (K3)
    float K3_1 = getMCP3424Value(2, 0); // WRK
    float K3_2 = getMCP3424Value(2, 1); // AUX
    float T_NO2 = calibratedData.K3_temp;
    calibratedData.NO2 = NO2_B0 + NO2_B1 * K3_1 + NO2_B2 * K3_2 + NO2_B3 * T_NO2;
    calibratedData.NO2 = fmax(GAS_MIN, fmin(calibratedData.NO2, GAS_MAX));
    
    // O3 (K2) z kompensacja NO2
    float K2_1 = getMCP3424Value(1, 0); // WRK
    float K2_2 = getMCP3424Value(1, 1); // AUX
    float T_O3 = calibratedData.K2_temp;
    calibratedData.O3 = O3_B0 + O3_B1 * K2_1 + O3_B2 * K2_2 + O3_B3 * T_O3 + O3_D * calibratedData.NO2;
    calibratedData.O3 = fmax(GAS_MIN, fmin(calibratedData.O3, GAS_MAX));
    
    // SO2 - przykładowa implementacja z Kx (device do określenia)
    float Kx_1 = getMCP3424Value(4, 0); // WRK
    float Kx_2 = getMCP3424Value(4, 1); // AUX
    float Kx_3 = getMCP3424Value(4, 2); // TRM  
    float Kx_4 = getMCP3424Value(4, 3); // VCC
    float T_SO2 = B4_T(Kx_3, Kx_4);
    calibratedData.SO2 = SO2_B0 + SO2_B1 * Kx_1 + SO2_B2 * Kx_2 + SO2_B3 * T_SO2;
    calibratedData.SO2 = fmax(GAS_MIN, fmin(calibratedData.SO2, GAS_MAX));
    
    // NH3 - przykładowa implementacja
    calibratedData.NH3 = NH3_B0 + NH3_B1 * Kx_1 + NH3_B3 * T_SO2;
    calibratedData.NH3 = fmax(GAS_MIN, fmin(calibratedData.NH3, GAS_MAX));
    
    // H2S - przykładowa implementacja
    calibratedData.H2S = H2S_B0 + H2S_B1 * Kx_1 + H2S_B2 * Kx_2 + H2S_B3 * T_SO2;
    calibratedData.H2S = fmax(GAS_MIN, fmin(calibratedData.H2S, GAS_MAX));
}

// Konwersja gazow z ug/m3 na ppb
void calibrateGasesPPB() {
    calibratedData.CO_ppb = calibratedData.CO / CO_PPB_DIV;
    calibratedData.NO_ppb = calibratedData.NO / NO_PPB_DIV;
    calibratedData.NO2_ppb = calibratedData.NO2 / NO2_PPB_DIV;
    calibratedData.O3_ppb = calibratedData.O3 / O3_PPB_DIV;
    calibratedData.SO2_ppb = calibratedData.SO2 / SO2_PPB_DIV;
    calibratedData.H2S_ppb = calibratedData.H2S / H2S_PPB_DIV;
    calibratedData.NH3_ppb = calibratedData.NH3 / NH3_PPB_DIV;
}

// Kalibracja HCHO i PID
void calibrateSpecialSensors() {
    // HCHO - potrzebne dane z PMS5003ST (UART sensor)
    // TODO: Implementacja gdy dostepne beda dane z UART
    // calibratedData.HCHO = RAW * HCHO_B1 * HCHO_PPB_CF; // PPB_CF conversion
    
    // PID-AH v8 - wykorzystujemy Kx kanaly 
    float WRK = getMCP3424Value(5, 2); // Kx_3 - przykładowe przypisanie
    float OFS = getMCP3424Value(5, 3); // Kx_4 - przykładowe przypisanie
    float mv = (WRK + OFS) / 256.0f;
    
    calibratedData.PID = ((mv - PID_OFFSET) * PID_A + PID_B) / PID_CF;
    calibratedData.PID = fmax(PID_MIN, fmin(calibratedData.PID, PID_MAX));
    calibratedData.PID_mV = mv;
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

// Funkcja clamp do ograniczania wartosci
float clamp(float value, float min_val, float max_val) {
    return fmax(min_val, fmin(value, max_val));
}

// Glowna funkcja kalibracji
void performCalibration() {
    // Sprawdz czy kalibracja jest wlaczona
    if (!calibConfig.enableCalibration) {
        calibratedData.valid = false;
        return;
    }
    
    if (!mcp3424Data.deviceCount || (millis() - mcp3424Data.lastUpdate) > 5000) {
        // Brak danych lub dane nieaktualne
        calibratedData.valid = false;
        return;
    }
    
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
    
    // Kalibracja HCHO i PID (kontrolowane przez config)
    if (calibConfig.enableSpecialSensors) {
        calibrateSpecialSensors();
    }
    
    calibratedData.valid = true;
    calibratedData.lastUpdate = millis();
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
