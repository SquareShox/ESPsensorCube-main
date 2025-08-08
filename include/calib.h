#ifndef CALIB_H
#define CALIB_H

#include <Arduino.h>

// Konfiguracja wlaczania kalibracji dla oszczędzania RAM
struct CalibrationConfig {
    bool enableCalibration = true;         // Globalne wlaczenie kalibracji
    bool enableMovingAverages = true;      // Wlaczenie średnich dla kalibrowanych danych
    bool enableTGSSensors = true;          // Wlaczenie czujnikow TGS (TGS02, TGS03, TGS12)
    bool enableGasSensors = true;          // Wlaczenie gazow elektrochemicznych
    bool enablePPBConversion = true;       // Wlaczenie konwersji na ppb
    bool enableSpecialSensors = true;      // Wlaczenie HCHO i PID
};

// Struktura przechowujaca skalibrowane dane czujnikow
struct CalibratedSensorData {
    // Temperatury czujnikow B4 (K1-K5)
    float K1_temp = 0.0f;    // Temperatura K1 [°C]
    float K2_temp = 0.0f;    // Temperatura K2 [°C]  
    float K3_temp = 0.0f;    // Temperatura K3 [°C]
    float K4_temp = 0.0f;    // Temperatura K4 [°C]
    float K5_temp = 0.0f;    // Temperatura K5 [°C]
    
    // Napiecia czujnikow B4 (K1-K5)
    float K1_voltage = 0.0f; // Napiecie K1 [mV]
    float K2_voltage = 0.0f; // Napiecie K2 [mV]
    float K3_voltage = 0.0f; // Napiecie K3 [mV]
    float K4_voltage = 0.0f; // Napiecie K4 [mV]
    float K5_voltage = 0.0f; // Napiecie K5 [mV]
    
    // Temperatury czujnikow TGS (K6-K12)
    float K6_temp = 0.0f;    // Temperatura K6 [°C]
    float K7_temp = 0.0f;    // Temperatura K7 [°C]
    float K8_temp = 0.0f;    // Temperatura K8 [°C]
    float K9_temp = 0.0f;    // Temperatura K9 [°C]
    float K12_temp = 0.0f;   // Temperatura K12 [°C]
    
    // Napiecia czujnikow TGS
    float K6_voltage = 0.0f; // Napiecie K6 [mV]
    float K7_voltage = 0.0f; // Napiecie K7 [mV]
    float K8_voltage = 0.0f; // Napiecie K8 [mV]
    float K9_voltage = 0.0f; // Napiecie K9 [mV]
    float K12_voltage = 0.0f;// Napiecie K12 [mV]
    
    // Gazy w ug/m3
    float CO = 0.0f;         // Tlenek wegla [ug/m3]
    float NO = 0.0f;         // Tlenek azotu [ug/m3]
    float NO2 = 0.0f;        // Dwutlenek azotu [ug/m3]
    float O3 = 0.0f;         // Ozon [ug/m3]
    float SO2 = 0.0f;        // Dwutlenek siarki [ug/m3]
    float H2S = 0.0f;        // Siarkowodor [ug/m3]
    float NH3 = 0.0f;        // Amoniak [ug/m3]
    
    // Gazy w ppb
    float CO_ppb = 0.0f;     // Tlenek wegla [ppb]
    float NO_ppb = 0.0f;     // Tlenek azotu [ppb]
    float NO2_ppb = 0.0f;    // Dwutlenek azotu [ppb]
    float O3_ppb = 0.0f;     // Ozon [ppb]
    float SO2_ppb = 0.0f;    // Dwutlenek siarki [ppb]
    float H2S_ppb = 0.0f;    // Siarkowodor [ppb]
    float NH3_ppb = 0.0f;    // Amoniak [ppb]
    
    // Czujniki TGS
    float TGS02 = 0.0f;      // TGS02 [ppm]
    float TGS03 = 0.0f;      // TGS03 [ppm]
    float TGS12 = 0.0f;      // TGS12 [ppm]
    float TGS02_ohm = 0.0f;  // Rezystancja TGS02 [ohm]
    float TGS03_ohm = 0.0f;  // Rezystancja TGS03 [ohm]
    float TGS12_ohm = 0.0f;  // Rezystancja TGS12 [ohm]
    
    // HCHO i PID
    float HCHO = 0.0f;       // Formaldehyd [ppb]
    float PID = 0.0f;        // PID sensor [ppm]
    float PID_mV = 0.0f;     // PID napięcie [mV]
    
    // VOC (Volatile Organic Compounds) - suma NH3, H2S, SO2
    float VOC = 0.0f;        // VOC [ug/m3] - suma NH3 + H2S + SO2
    float VOC_ppb = 0.0f;    // TVOC [ppb] - TGS02 value
    
    // ODO (Odor Detection Unit) - według wzoru z calib.tcl
    float ODO = 0.0f;        // ODO value based on TGS sensors
    
    // PM sensors (SPS30) - zgodnie z calib.tcl linie 1017-1048
    float PM1 = 0.0f;        // PM1.0 skalibrowane [ug/m3]
    float PM25 = 0.0f;       // PM2.5 skalibrowane [ug/m3]
    float PM10 = 0.0f;       // PM10 skalibrowane [ug/m3]
    
    // Environmental sensors (I2C sensors) - zgodnie z calib.tcl
    float AMBIENT_TEMP = 0.0f;     // Temperatura zewnętrzna [°C]
    float AMBIENT_HUMID = 0.0f;    // Wilgotność zewnętrzna [%]
    float AMBIENT_PRESS = 0.0f;    // Ciśnienie zewnętrzne [hPa]
    
    float DUST_TEMP = 0.0f;        // Temperatura toru pylowego [°C]
    float DUST_HUMID = 0.0f;       // Wilgotność toru pylowego [%]
    float DUST_PRESS = 0.0f;       // Ciśnienie toru pylowego [hPa]
    
    float GAS_TEMP = 0.0f;         // Temperatura toru gazowego [°C]
    float GAS_HUMID = 0.0f;        // Wilgotność toru gazowego [%]
    float GAS_PRESS = 0.0f;        // Ciśnienie toru gazowego [hPa]
    
    // CO2 sensor
    float SCD_CO2 = 0.0f;          // CO2 [ppm]
    float SCD_T = 0.0f;            // SCD temperatura [°C]
    float SCD_RH = 0.0f;           // SCD wilgotność [%]
    
    // Metadata
    bool valid = false;
    unsigned long lastUpdate = 0;
};

// Deklaracje funkcji kalibracyjnych z calib_functions.tcl

// Funkcje kompensacji temperaturowej
float A4_O3(float T);
float B4_O3(float T);
float A4_SO2(float T);
float B4_SO2(float T);
float A4_NO2(float T);
float B4_NO2(float T);
float A4_H2S(float T);
float B4_H2S(float T);

// Funkcje obliczania temperatury i napiecia
float B4_T(float T, float V);
float B4_mV(float V);
float TGS_T(float C1, float C2);
float TGSv4_mV(float C4);
float TGSv4_ohm(float C3, float C4, float RL);
float TGSv4_ppm(float C3, float C4, float RL, float A, float B, float C, float b1, float a);

// Funkcje korekcji liniowej
float Linear_basic(float mul, float RAW, float add);
float Linear_advanced(float mul, float RAW, float add);

// Funkcje glowne
void performCalibration();
CalibratedSensorData getCalibratedData();
bool isCalibratedDataValid();

// Funkcje pomocnicze
float getMCP3424Value(uint8_t deviceIndex, uint8_t channel);

// Funkcje kalibracji poszczegolnych grup czujnikow
void calibrateB4Temperature();
void calibrateB4Voltage();
void calibrateTGSTemperature();
void calibrateTGSVoltage();
void calibrateTGSSensors();
void calibrateGases();
void calibrateGasesPPB();
void calibrateSpecialSensors();
void calibratePMSensors();
void calibrateEnvironmentalSensors();
void calibrateCO2Sensor();

// Global data
extern CalibratedSensorData calibratedData;

// Funkcje konfiguracji i kontroli
extern CalibrationConfig calibConfig;
void setCalibrationConfig(const CalibrationConfig& config);
CalibrationConfig getCalibrationConfig();

#endif // CALIB_H 