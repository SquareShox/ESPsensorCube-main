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

// External global MCP3424 data
extern MCP3424Data mcp3424Data;
//#define DEBUG
const char* gasTypes[] = {"NO", "O3", "NO2", "CO", "SO2", "TGS1", "TGS2", "TGS3"};
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
// Teraz uzywa globalnej funkcji z network_config.h

// Kalibracja temperatury dla czujników K1-K5 (B4)
void calibrateB4Temperature() {
    // K1 temperatura (NO) - channel 2,3
    int8_t deviceNO = getMCP3424DeviceByGasType(gasTypes[0]);
    if (deviceNO >= 0) {
        float K1_3 = getMCP3424Value(deviceNO, 2); // RAW_T
        float K1_4 = getMCP3424Value(deviceNO, 3); // RAW_V
        calibratedData.K1_temp = B4_T(K1_3, K1_4);
        
        #ifdef DEBUG
        Serial.print("[DEBUG] K1 (NO) - device=");
        Serial.print(deviceNO);
        Serial.print(", K1_3=");
        Serial.print(K1_3, 6);
        Serial.print(", K1_4=");
        Serial.print(K1_4, 6);
        Serial.print(", temp=");
        Serial.println(calibratedData.K1_temp, 6);
        #endif
    } else {
        #ifdef DEBUG
        Serial.println("[DEBUG] K1 (NO) - device not found");
        #endif
    }
    
    // K2 temperatura (O3) - channel 2,3
    int8_t deviceO3 = getMCP3424DeviceByGasType(gasTypes[1]);
    if (deviceO3 >= 0) {
        float K2_3 = getMCP3424Value(deviceO3, 2);
        float K2_4 = getMCP3424Value(deviceO3, 3);
        calibratedData.K2_temp = B4_T(K2_3, K2_4);
        
        #ifdef DEBUG
        Serial.print("[DEBUG] K2 (O3) - device=");
        Serial.print(deviceO3);
        Serial.print(", K2_3=");
        Serial.print(K2_3, 6);
        Serial.print(", K2_4=");
        Serial.print(K2_4, 6);
        Serial.print(", temp=");
        Serial.println(calibratedData.K2_temp, 6);
        #endif
    } else {
        #ifdef DEBUG
        Serial.println("[DEBUG] K2 (O3) - device not found");
        #endif
    }
    
        // K3 temperatura (NO2) - channel 2,3
    int8_t deviceNO2 = getMCP3424DeviceByGasType(gasTypes[2]);
    if (deviceNO2 >= 0) {
        float K3_3 = getMCP3424Value(deviceNO2, 2);
        float K3_4 = getMCP3424Value(deviceNO2, 3);
        calibratedData.K3_temp = B4_T(K3_3, K3_4);
        
        #ifdef DEBUG
        Serial.print("[DEBUG] K3 (NO2) - device=");
        Serial.print(deviceNO2);
        Serial.print(", K3_3=");
        Serial.print(K3_3, 6);
        Serial.print(", K3_4=");
        Serial.print(K3_4, 6);
        Serial.print(", temp=");
        Serial.println(calibratedData.K3_temp, 6);
        #endif
    } else {
        #ifdef DEBUG
        Serial.println("[DEBUG] K3 (NO2) - device not found");
        #endif
    }
    
    // K4 temperatura (CO) - channel 2,3
    int8_t deviceCO = getMCP3424DeviceByGasType(gasTypes[3]);
    if (deviceCO >= 0) {
        float K4_3 = getMCP3424Value(deviceCO, 2);
        float K4_4 = getMCP3424Value(deviceCO, 3);
        calibratedData.K4_temp = B4_T(K4_3, K4_4);
        
        #ifdef DEBUG
        Serial.print("[DEBUG] K4 (CO) - device=");
        Serial.print(deviceCO);
        Serial.print(", K4_3=");
        Serial.print(K4_3, 6);
        Serial.print(", K4_4=");
        Serial.print(K4_4, 6);
        Serial.print(", temp=");
        Serial.println(calibratedData.K4_temp, 6);
        #endif
    } else {
        #ifdef DEBUG
        Serial.println("[DEBUG] K4 (CO) - device not found");
        #endif
    }
    
    // K5 temperatura (SO2) - channel 2,3
    int8_t deviceSO2 = getMCP3424DeviceByGasType(gasTypes[4]);
    if (deviceSO2 >= 0) {
        float K5_3 = getMCP3424Value(deviceSO2, 2);
        float K5_4 = getMCP3424Value(deviceSO2, 3);
        calibratedData.K5_temp = B4_T(K5_3, K5_4);
        
        #ifdef DEBUG
        Serial.print("[DEBUG] K5 (SO2) - device=");
        Serial.print(deviceSO2);
        Serial.print(", K5_3=");
        Serial.print(K5_3, 6);
        Serial.print(", K5_4=");
        Serial.print(K5_4, 6);
        Serial.print(", temp=");
        Serial.println(calibratedData.K5_temp, 6);
        #endif
    } else {
        #ifdef DEBUG
        Serial.println("[DEBUG] K5 (SO2) - device not found");
        #endif
    }
}

// Kalibracja napiec dla czujników B4
void calibrateB4Voltage() {
    int8_t deviceNO = getMCP3424DeviceByGasType(gasTypes[0]);
    if (deviceNO >= 0) calibratedData.K1_voltage = B4_mV(getMCP3424Value(deviceNO, 3));
    
    int8_t deviceO3 = getMCP3424DeviceByGasType(gasTypes[1]);
    if (deviceO3 >= 0) calibratedData.K2_voltage = B4_mV(getMCP3424Value(deviceO3, 3));
    
    int8_t deviceNO2 = getMCP3424DeviceByGasType(gasTypes[2]);
    if (deviceNO2 >= 0) calibratedData.K3_voltage = B4_mV(getMCP3424Value(deviceNO2, 3));
    
    int8_t deviceCO = getMCP3424DeviceByGasType(gasTypes[3]);
    if (deviceCO >= 0) calibratedData.K4_voltage = B4_mV(getMCP3424Value(deviceCO, 3));
    
    int8_t deviceSO2 = getMCP3424DeviceByGasType(gasTypes[4]);
    if (deviceSO2 >= 0) calibratedData.K5_voltage = B4_mV(getMCP3424Value(deviceSO2, 3));
}

// Kalibracja temperatur czujnikow TGS (K6-K12)
void calibrateTGSTemperature() {
    // K6 temperatura - TGS1 (channel 0,1)
    int8_t deviceTGS1 = getMCP3424DeviceByGasType(gasTypes[5]);
    if (deviceTGS1 >= 0) {
        float K6_1 = getMCP3424Value(deviceTGS1, 0); // C1
        float K6_2 = getMCP3424Value(deviceTGS1, 1); // C2
        calibratedData.K6_temp = TGS_T(K6_1, K6_2);
    }
    
    // K7 temperatura - TGS2 (channel 0,1)
    int8_t deviceTGS2 = getMCP3424DeviceByGasType(gasTypes[6]);
    if (deviceTGS2 >= 0) {
        float K7_1 = getMCP3424Value(deviceTGS2, 0);
        float K7_2 = getMCP3424Value(deviceTGS2, 1);
        calibratedData.K7_temp = TGS_T(K7_1, K7_2);
    }
    
    // K8 temperatura - TGS3 (channel 0,1)
    int8_t deviceTGS3 = getMCP3424DeviceByGasType(gasTypes[7]);
    if (deviceTGS3 >= 0) {
        float K8_1 = getMCP3424Value(deviceTGS3, 0);
        float K8_2 = getMCP3424Value(deviceTGS3, 1);
        calibratedData.K8_temp = TGS_T(K8_1, K8_2);
    }
    
    // K9 i K12 - używamy TGS1 i TGS2
    if (deviceTGS1 >= 0) {
        calibratedData.K9_temp = TGS_T(getMCP3424Value(deviceTGS1, 0), getMCP3424Value(deviceTGS1, 1));
    }
    if (deviceTGS2 >= 0) {
        calibratedData.K12_temp = TGS_T(getMCP3424Value(deviceTGS2, 0), getMCP3424Value(deviceTGS2, 1));
    }
}

// Kalibracja napiec czujnikow TGS
void calibrateTGSVoltage() {
    int8_t deviceTGS1 = getMCP3424DeviceByGasType(gasTypes[5]);
    if (deviceTGS1 >= 0) calibratedData.K6_voltage = TGSv4_mV(getMCP3424Value(deviceTGS1, 3));
    
    int8_t deviceTGS2 = getMCP3424DeviceByGasType(gasTypes[6]);
    if (deviceTGS2 >= 0) calibratedData.K7_voltage = TGSv4_mV(getMCP3424Value(deviceTGS2, 3));
    
    int8_t deviceTGS3 = getMCP3424DeviceByGasType(gasTypes[7]);
    if (deviceTGS3 >= 0) calibratedData.K8_voltage = TGSv4_mV(getMCP3424Value(deviceTGS3, 3));
    
    // K9 i K12 - używamy CO i O3
    int8_t deviceCO = getMCP3424DeviceByGasType(gasTypes[3]);
    if (deviceCO >= 0) calibratedData.K9_voltage = TGSv4_mV(getMCP3424Value(deviceCO, 3));
    
    int8_t deviceO3 = getMCP3424DeviceByGasType(gasTypes[1]);
    if (deviceO3 >= 0) calibratedData.K12_voltage = TGSv4_mV(getMCP3424Value(deviceO3, 3));
}

// Kalibracja czujnikow TGS
void calibrateTGSSensors() {
    // TGS03 (K6) - TGS1 (channel 2,3)
    int8_t deviceTGS1 = getMCP3424DeviceByGasType(gasTypes[5]);
    if (deviceTGS1 >= 0) {
        float K6_3 = getMCP3424Value(deviceTGS1, 2);
        float K6_4 = getMCP3424Value(deviceTGS1, 3);
        calibratedData.TGS03 = TGSv4_ppm(K6_3, K6_4, RL_TGS03, RL_TGS03, 0, 0, TGS03_B1, TGS03_A);
        calibratedData.TGS03_ohm = TGSv4_ohm(K6_3, K6_4, RL_TGS03);
    }
    
    // TGS02 (K7) - TGS2 (channel 2,3)
    int8_t deviceTGS2 = getMCP3424DeviceByGasType(gasTypes[6]);
    if (deviceTGS2 >= 0) {
        float K7_3 = getMCP3424Value(deviceTGS2, 2);
        float K7_4 = getMCP3424Value(deviceTGS2, 3);
        // TGSv4_ppm_FIX implementation
        float R_02 = RL_TGS02 * (2.0f * K7_3 + K7_4) / K7_4;
        float Sr_02 = RL_TGS02 / (R_02 + 0) + 0;
        calibratedData.TGS02 = TGS02_B1 * Sr_02 + TGS02_A;
        calibratedData.TGS02_ohm = R_02;
    }
    
    // TGS12 - TGS3 (channel 2,3)
    int8_t deviceTGS3 = getMCP3424DeviceByGasType(gasTypes[7]);
    if (deviceTGS3 >= 0) {
        float Kx_3 = getMCP3424Value(deviceTGS3, 2);
        float Kx_4 = getMCP3424Value(deviceTGS3, 3);
        calibratedData.TGS12 = TGSv4_ppm(Kx_3, Kx_4, RL_TGS12, RL_TGS12, 0, 0, TGS12_B1, TGS12_A);
        calibratedData.TGS12_ohm = TGSv4_ohm(Kx_3, Kx_4, RL_TGS12);
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
        calibratedData.CO = CO_B0 + CO_B1 * K4_1 + CO_B2 * K4_2 + CO_B3 * T_CO;
        calibratedData.CO = fmax(GAS_MIN, fmin(calibratedData.CO, GAS_MAX));
        
        #ifdef DEBUG
        Serial.print("[DEBUG] CO calculation - device=");
        Serial.print(deviceCO);
        Serial.print(", K4_1=");
        Serial.print(K4_1, 6);
        Serial.print(", K4_2=");
        Serial.print(K4_2, 6);
        Serial.print(", K4_3=");
        Serial.print(K4_3, 6);
        Serial.print(", K4_4=");
        Serial.print(K4_4, 6);
        Serial.print(", T_CO=");
        Serial.print(T_CO, 6);
        Serial.print(", CO=");
        Serial.println(calibratedData.CO, 6);
        #endif
    } else {
        #ifdef DEBUG
        Serial.println("[DEBUG] CO - device not found");
        #endif
    }
    
    // NO (K1) - channel 0,1
    int8_t deviceNO = getMCP3424DeviceByGasType(gasTypes[0]);
    if (deviceNO >= 0) {
        float K1_1 = getMCP3424Value(deviceNO, 0); // WRK
        float K1_2 = getMCP3424Value(deviceNO, 1); // AUX
        float T_NO = calibratedData.K1_temp;
        calibratedData.NO = NO_B0 + NO_B1 * K1_1 + NO_B2 * K1_2 + NO_B3 * T_NO;
        calibratedData.NO = fmax(GAS_MIN, fmin(calibratedData.NO, GAS_MAX));
    }
    
    // NO2 (K3) - channel 0,1
    int8_t deviceNO2 = getMCP3424DeviceByGasType(gasTypes[2]);
    if (deviceNO2 >= 0) {
        float K3_1 = getMCP3424Value(deviceNO2, 0); // WRK
        float K3_2 = getMCP3424Value(deviceNO2, 1); // AUX
        float T_NO2 = calibratedData.K3_temp;
        calibratedData.NO2 = NO2_B0 + NO2_B1 * K3_1 + NO2_B2 * K3_2 + NO2_B3 * T_NO2;
        calibratedData.NO2 = fmax(GAS_MIN, fmin(calibratedData.NO2, GAS_MAX));
    }
    
    // O3 (K2) z kompensacja NO2 - channel 0,1
    int8_t deviceO3 = getMCP3424DeviceByGasType(gasTypes[1]);
    if (deviceO3 >= 0) {
        float K2_1 = getMCP3424Value(deviceO3, 0); // WRK
        float K2_2 = getMCP3424Value(deviceO3, 1); // AUX
        float T_O3 = calibratedData.K2_temp;
        calibratedData.O3 = O3_B0 + O3_B1 * K2_1 + O3_B2 * K2_2 + O3_B3 * T_O3 + O3_D * calibratedData.NO2;
        calibratedData.O3 = fmax(GAS_MIN, fmin(calibratedData.O3, GAS_MAX));
    }
    
    // SO2 (K5) - channel 0,1,2,3
    int8_t deviceSO2 = getMCP3424DeviceByGasType(gasTypes[4]);
    if (deviceSO2 >= 0) {
        float K5_1 = getMCP3424Value(deviceSO2, 0); // WRK
        float K5_2 = getMCP3424Value(deviceSO2, 1); // AUX
        float K5_3 = getMCP3424Value(deviceSO2, 2); // TRM  
        float K5_4 = getMCP3424Value(deviceSO2, 3); // VCC
        float T_SO2 = B4_T(K5_3, K5_4);
        calibratedData.SO2 = SO2_B0 + SO2_B1 * K5_1 + SO2_B2 * K5_2 + SO2_B3 * T_SO2;
        calibratedData.SO2 = fmax(GAS_MIN, fmin(calibratedData.SO2, GAS_MAX));
        
        // NH3 i H2S - używamy danych z SO2
        calibratedData.NH3 = NH3_B0 + NH3_B1 * K5_1 + NH3_B3 * T_SO2;
        calibratedData.NH3 = fmax(GAS_MIN, fmin(calibratedData.NH3, GAS_MAX));
        
        calibratedData.H2S = H2S_B0 + H2S_B1 * K5_1 + H2S_B2 * K5_2 + H2S_B3 * T_SO2;
        calibratedData.H2S = fmax(GAS_MIN, fmin(calibratedData.H2S, GAS_MAX));
    }
    
    // VOC (Volatile Organic Compounds) - suma NH3 + H2S + SO2
    calibratedData.VOC = calibratedData.NH3 + calibratedData.H2S + calibratedData.SO2;
    calibratedData.VOC = fmax(GAS_MIN, fmin(calibratedData.VOC, GAS_MAX));
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
    
    // VOC w ppb - suma NH3 + H2S + SO2 w ppb
    calibratedData.VOC_ppb = calibratedData.NH3_ppb + calibratedData.H2S_ppb + calibratedData.SO2_ppb;
}

// Kalibracja HCHO i PID
void calibrateSpecialSensors() {
    // HCHO - używamy danych z czujnika CB-HCHO-V4
    extern HCHOData hchoData;
    extern bool hchoSensorStatus;
    
    if (hchoSensorStatus && hchoData.valid) {
        // Przeliczamy HCHO z mg/m3 na ppb (1 mg/m3 = 813 ppb dla HCHO w 25C, 1013hPa)
        calibratedData.HCHO = hchoData.hcho * 813.0f;

        // Opcjonalna kalibracja/korekcja
        // calibratedData.HCHO = Linear_basic(HCHO_CALIB_MULT, calibratedData.HCHO, HCHO_CALIB_OFFSET);
        
        // Ograniczenie wartosci do rozsadnych granic (0-40000 ppb)
        calibratedData.HCHO = fmax(0.0f, fmin(calibratedData.HCHO, 40000.0f));
        
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
        
        calibratedData.PID = ((mv - PID_OFFSET) * PID_A + PID_B) / PID_CF;
        calibratedData.PID = fmax(PID_MIN, fmin(calibratedData.PID, PID_MAX));
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

// Funkcja clamp do ograniczania wartosci
float clamp(float value, float min_val, float max_val) {
    return fmax(min_val, fmin(value, max_val));
}

// Glowna funkcja kalibracji
void performCalibration() {
    // DEBUG: start kalibracji
    #define DEBUG

 

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
