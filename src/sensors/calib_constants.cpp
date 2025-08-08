#include "calib_constants.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

// =============================================================================
// STALE KALIBRACYJNE - DOMYSLNE WARTOSCI (JAKO FALLBACK)
// =============================================================================

// Global instance
CalibrationConstants calibConstants;

void setDefaultCalibrationConstants(CalibrationConstants& constants) {
    // B4 sensor constants
    constants.B4_RS = 33.0f;
    constants.B4_K = 3.2f;
    constants.B4_TO = 25.0f;
    constants.B4_COK = 273.15f;
    constants.B4_B = 3380.0f;
    constants.B4_RO = 10.0f;
    constants.B4_LSB = 3.90625f;
    
    // TGS sensor constants
    constants.TGS_TO = 25.0f;
    constants.TGS_COK = 273.15f;
    constants.TGS_B = 3380.0f;
    constants.TGS_LSB = 3.90625f;
    
    // TGS03 constants
    constants.RL_TGS03 = 10000.0f;
    constants.TGS03_B1 = 30.0f;
    constants.TGS03_A = -1.0f;
    
    // TGS02 constants
    constants.RL_TGS02 = 3300.0f;
    constants.TGS02_B1 = 50.0f;
    constants.TGS02_A = 4.0f;
    
    // TGS12 constants
    constants.RL_TGS12 = 10000.0f;
    constants.TGS12_B1 = 1000.0f;
    constants.TGS12_A = 0.0f;
    
    // Gas sensor constants
    constants.CO_B0 = 70.0f;
    constants.CO_B1 = 0.4f;
    constants.CO_B2 = -0.4f;
    constants.CO_B3 = 0.0f;
    
    constants.NO_B0 = 0.2f;
    constants.NO_B1 = 0.03f;
    constants.NO_B2 = -0.03f;
    constants.NO_B3 = 0.0f;
    
    constants.NO2_B0 = 20.0f;
    constants.NO2_B1 = -0.09f;
    constants.NO2_B2 = -0.063f;
    constants.NO2_B3 = 0.0f;
    
    constants.O3_B0 = -20.0f;
    constants.O3_B1 = -0.09f;
    constants.O3_B2 = -0.054f;
    constants.O3_B3 = 0.0f;
    constants.O3_D = -1.35f;
    
    constants.SO2_B0 = 4.0f;
    constants.SO2_B1 = 0.005f;
    constants.SO2_B2 = -0.008f;
    constants.SO2_B3 = 0.0f;
    
    constants.H2S_B0 = 0.2f;
    constants.H2S_B1 = 0.00005f;
    constants.H2S_B2 = -0.00009f;
    constants.H2S_B3 = 0.0f;
    
    constants.NH3_B0 = 0.2f;
    constants.NH3_B1 = 0.00014f;
    constants.NH3_B2 = 0.0f; // Not used in original
    constants.NH3_B3 = 0.0f;
    
    // Temperature compensation constants
    constants.K1_temp = 0.0f;
    constants.K2_temp = 0.0f;
    constants.K3_temp = 0.0f;
    constants.K4_temp = 0.0f;
    
    // Gas limits
    constants.GAS_MIN = 0.0f;
    constants.GAS_MAX = 50000.0f;
    
    // HCHO sensor constants
    constants.HCHO_PPB_CF = 0.814f;
    constants.HCHO_B1 = 1.0f;
    constants.HCHO_A = 0.0f;
    constants.HCHO_MIN = 0.0f;
    constants.HCHO_MAX = 40000.0f;
    
    // PID sensor constants
    constants.PID_OFFSET = 140.0f;
    constants.PID_A = 3.0f;
    constants.PID_B = 0.0f;
    constants.PID_CF = 1.0f;
    constants.PID_MIN = 0.0f;
    constants.PID_MAX = 40000.0f;
    
    // ODO sensor constants
    constants.ODO_A0 = -0.9641f;
    constants.ODO_A1 = 2.1529f;
    constants.ODO_A2 = 1.6481f;
    constants.ODO_A3 = 0.0164f;
    constants.ODO_A4 = 0.2739f;
    constants.ODO_A5 = -1.0731f;
    constants.ODO_MIN = 0.0f;
    constants.ODO_MAX = 1000000.0f;
    
    // PM sensor constants - new naming convention
    constants.PM1_A = 1.0f;      // PM1 multiplication
    constants.PM1_B = 0.0f;      // PM1 additive
    constants.PM25_A = 1.0f;     // PM2.5 multiplication
    constants.PM25_B = 0.0f;     // PM2.5 additive
    constants.PM10_A = 1.0f;     // PM10 multiplication
    constants.PM10_B = 0.0f;     // PM10 additive
    constants.PM_MIN = 0.0f;
    constants.PM_MAX = 5000.0f;
    
    // Environmental sensor constants - new naming convention
    // Ambient sensors
    constants.AMBIENT_TEMP_A = 1.0f;
    constants.AMBIENT_TEMP_B = 0.0f;
    constants.AMBIENT_HUMID_A = 1.0f;
    constants.AMBIENT_HUMID_B = 0.0f;
    constants.AMBIENT_PRESS_A = 1.0f;
    constants.AMBIENT_PRESS_B = 0.0f;
    
    // Dust sensors
    constants.DUST_TEMP_A = 1.0f;
    constants.DUST_TEMP_B = 0.0f;
    constants.DUST_HUMID_A = 1.0f;
    constants.DUST_HUMID_B = 0.0f;
    constants.DUST_PRESS_A = 1.0f;
    constants.DUST_PRESS_B = 0.0f;
    
    // Gas sensors
    constants.GAS_TEMP_A = 1.0f;
    constants.GAS_TEMP_B = 0.0f;
    constants.GAS_HUMID_A = 1.0f;
    constants.GAS_HUMID_B = 0.0f;
    constants.GAS_PRESS_A = 1.0f;
    constants.GAS_PRESS_B = 0.0f;
    
    // Environmental limits
    constants.ENV_MIN = -100.0f;
    constants.ENV_MAX = 2000.0f;
    
    // CO2 sensor constants - new naming convention
    constants.SCD_CO2_A = 1.0f;      // CO2 multiplication
    constants.SCD_CO2_B = 0.0f;      // CO2 additive
    constants.SCD_T_A = 1.0f;        // Temperature multiplication
    constants.SCD_T_B = 0.0f;        // Temperature additive
    constants.SCD_RH_A = 1.0f;       // Humidity multiplication
    constants.SCD_RH_B = 0.0f;       // Humidity additive
    constants.SCD_CO2_MIN = 0.0f;
    constants.SCD_CO2_MAX = 60000.0f;
    constants.SCD_T_MIN = -100.0f;
    constants.SCD_T_MAX = 100.0f;
    constants.SCD_RH_MIN = 0.0f;
    constants.SCD_RH_MAX = 100.0f;
}

bool loadCalibrationConstants(CalibrationConstants& constants) {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS");
        return false;
    }

    if (!LittleFS.exists("/data/calib_nums.json")) {
        Serial.println("Calibration file not found, using defaults");
        setDefaultCalibrationConstants(constants);
        return false;
    }

    File file = LittleFS.open("/data/calib_nums.json", "r");
    if (!file) {
        Serial.println("Failed to open calibration file");
        setDefaultCalibrationConstants(constants);
        return false;
    }

    DynamicJsonDocument doc(16384);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        setDefaultCalibrationConstants(constants);
        return false;
    }

    // Wczytaj wszystkie stałe z JSON z fallback do wartości domyślnych
    // Rezystory obciazajace dla czujnikow TGS (ohm)
    constants.RL_TGS03 = doc["RL_TGS03"] | 10000.0f;
    constants.RL_TGS02 = doc["RL_TGS02"] | 3300.0f;
    constants.RL_TGS12 = doc["RL_TGS12"] | 10000.0f;

    // Parametry kalibracji czujnikow TGS (ppm)
    constants.TGS03_B1 = doc["TGS03_B1"] | 30.0f;
    constants.TGS03_A = doc["TGS03_A"] | -1.0f;
    constants.TGS02_B1 = doc["TGS02_B1"] | 50.0f;
    constants.TGS02_A = doc["TGS02_A"] | 4.0f;
    constants.TGS12_B1 = doc["TGS12_B1"] | 1000.0f;
    constants.TGS12_A = doc["TGS12_A"] | 0.0f;

    // Parametry kalibracji gazow elektrochemicznych (ug/m3)
    // CO (K4)
    constants.CO_B0 = doc["CO_B0"] | 70.0f;
    constants.CO_B1 = doc["CO_B1"] | 0.4f;
    constants.CO_B2 = doc["CO_B2"] | -0.4f;
    constants.CO_B3 = doc["CO_B3"] | 0.0f;

    // NO (K1)
    constants.NO_B0 = doc["NO_B0"] | 0.2f;
    constants.NO_B1 = doc["NO_B1"] | 0.03f;
    constants.NO_B2 = doc["NO_B2"] | -0.03f;
    constants.NO_B3 = doc["NO_B3"] | 0.0f;

    // NO2 (K3)
    constants.NO2_B0 = doc["NO2_B0"] | 20.0f;
    constants.NO2_B1 = doc["NO2_B1"] | -0.09f;
    constants.NO2_B2 = doc["NO2_B2"] | -0.063f;
    constants.NO2_B3 = doc["NO2_B3"] | 0.0f;

    // O3 (K2) z kompensacja NO2
    constants.O3_B0 = doc["O3_B0"] | -20.0f;
    constants.O3_B1 = doc["O3_B1"] | -0.09f;
    constants.O3_B2 = doc["O3_B2"] | -0.054f;
    constants.O3_B3 = doc["O3_B3"] | 0.0f;
    constants.O3_D = doc["O3_D"] | -1.35f;

    // SO2
    constants.SO2_B0 = doc["SO2_B0"] | 4.0f;
    constants.SO2_B1 = doc["SO2_B1"] | 0.005f;
    constants.SO2_B2 = doc["SO2_B2"] | -0.008f;
    constants.SO2_B3 = doc["SO2_B3"] | 0.0f;

    // NH3
    constants.NH3_B0 = doc["NH3_B0"] | 0.2f;
    constants.NH3_B1 = doc["NH3_B1"] | 0.00014f;
    constants.NH3_B2 = doc["NH3_B2"] | 0.0f;
    constants.NH3_B3 = doc["NH3_B3"] | 0.0f;

    // H2S
    constants.H2S_B0 = doc["H2S_B0"] | 0.2f;
    constants.H2S_B1 = doc["H2S_B1"] | 0.00005f;
    constants.H2S_B2 = doc["H2S_B2"] | -0.00009f;
    constants.H2S_B3 = doc["H2S_B3"] | 0.0f;

    // Parametry kalibracji PID-AH v8
    constants.PID_OFFSET = doc["PID_OFFSET"] | 140.0f;
    constants.PID_B = doc["PID_B"] | 0.0f;
    constants.PID_A = doc["PID_A"] | 3.0f;
    constants.PID_CF = doc["PID_CF"] | 1.0f;

    // Parametry kalibracji HCHO (PMS5003ST)
    constants.HCHO_B1 = doc["HCHO_B1"] | 1.0f;
    constants.HCHO_A = doc["HCHO_A"] | 0.0f;
    constants.HCHO_PPB_CF = doc["HCHO_PPB_CF"] | 0.814f;

    // Parametry kalibracji czujnikow PM (SPS30) - nowa konwencja nazewnictwa
    constants.PM1_A = doc["PM1_A"] | 1.0f;
    constants.PM1_B = doc["PM1_B"] | 0.0f;
    constants.PM25_A = doc["PM25_A"] | 1.0f;
    constants.PM25_B = doc["PM25_B"] | 0.0f;
    constants.PM10_A = doc["PM10_A"] | 1.0f;
    constants.PM10_B = doc["PM10_B"] | 0.0f;

    // Parametry kalibracji czujnikow srodowiskowych (SHT40) - nowa konwencja nazewnictwa
    // Ambient sensors
    constants.AMBIENT_TEMP_A = doc["AMBIENT_TEMP_A"] | 1.0f;
    constants.AMBIENT_TEMP_B = doc["AMBIENT_TEMP_B"] | 0.0f;
    constants.AMBIENT_HUMID_A = doc["AMBIENT_HUMID_A"] | 1.0f;
    constants.AMBIENT_HUMID_B = doc["AMBIENT_HUMID_B"] | 0.0f;
    constants.AMBIENT_PRESS_A = doc["AMBIENT_PRESS_A"] | 1.0f;
    constants.AMBIENT_PRESS_B = doc["AMBIENT_PRESS_B"] | 0.0f;
    
    // Dust sensors
    constants.DUST_TEMP_A = doc["DUST_TEMP_A"] | 1.0f;
    constants.DUST_TEMP_B = doc["DUST_TEMP_B"] | 0.0f;
    constants.DUST_HUMID_A = doc["DUST_HUMID_A"] | 1.0f;
    constants.DUST_HUMID_B = doc["DUST_HUMID_B"] | 0.0f;
    constants.DUST_PRESS_A = doc["DUST_PRESS_A"] | 1.0f;
    constants.DUST_PRESS_B = doc["DUST_PRESS_B"] | 0.0f;
    
    // Gas sensors
    constants.GAS_TEMP_A = doc["GAS_TEMP_A"] | 1.0f;
    constants.GAS_TEMP_B = doc["GAS_TEMP_B"] | 0.0f;
    constants.GAS_HUMID_A = doc["GAS_HUMID_A"] | 1.0f;
    constants.GAS_HUMID_B = doc["GAS_HUMID_B"] | 0.0f;
    constants.GAS_PRESS_A = doc["GAS_PRESS_A"] | 1.0f;
    constants.GAS_PRESS_B = doc["GAS_PRESS_B"] | 0.0f;

    // Parametry kalibracji czujnika CO2 (SCD41) - nowa konwencja nazewnictwa
    constants.SCD_CO2_A = doc["SCD_CO2_A"] | 1.0f;
    constants.SCD_CO2_B = doc["SCD_CO2_B"] | 0.0f;
    constants.SCD_T_A = doc["SCD_T_A"] | 1.0f;
    constants.SCD_T_B = doc["SCD_T_B"] | 0.0f;
    constants.SCD_RH_A = doc["SCD_RH_A"] | 1.0f;
    constants.SCD_RH_B = doc["SCD_RH_B"] | 0.0f;

    // Parametry obliczenia ODO (z calib.tcl linie 656-668)
    constants.ODO_A0 = doc["ODO_A0"] | -0.9641f;
    constants.ODO_A1 = doc["ODO_A1"] | 2.1529f;
    constants.ODO_A2 = doc["ODO_A2"] | 1.6481f;
    constants.ODO_A3 = doc["ODO_A3"] | 0.0164f;
    constants.ODO_A4 = doc["ODO_A4"] | 0.2739f;
    constants.ODO_A5 = doc["ODO_A5"] | -1.0731f;

    // Parametry fizyczne dla obliczen temperatury
    constants.B4_TO = doc["B4_TO"] | 25.0f;
    constants.B4_B = doc["B4_B"] | 3380.0f;
    constants.B4_RO = doc["B4_RO"] | 10.0f;
    constants.B4_RS = doc["B4_RS"] | 33.0f;
    constants.B4_K = doc["B4_K"] | 3.2f;
    constants.B4_COK = doc["B4_COK"] | 273.15f;

    constants.TGS_TO = doc["TGS_TO"] | 25.0f;
    constants.TGS_B = doc["TGS_B"] | 3380.0f;
    constants.TGS_COK = doc["TGS_COK"] | 273.15f;

    // Parametry ADC
    constants.B4_LSB = doc["B4_LSB"] | 3.90625f;
    constants.TGS_LSB = doc["TGS_LSB"] | 3.90625f;

    // Ograniczenia wartosci (clamp limits)
    constants.GAS_MIN = doc["GAS_MIN"] | 0.0f;
    constants.GAS_MAX = doc["GAS_MAX"] | 50000.0f;
    constants.HCHO_MIN = doc["HCHO_MIN"] | 0.0f;
    constants.HCHO_MAX = doc["HCHO_MAX"] | 40000.0f;
    constants.PID_MIN = doc["PID_MIN"] | 0.0f;
    constants.PID_MAX = doc["PID_MAX"] | 40000.0f;

    // Ograniczenia dla nowych czujnikow
    constants.PM_MIN = doc["PM_MIN"] | 0.0f;
    constants.PM_MAX = doc["PM_MAX"] | 5000.0f;
    constants.ENV_MIN = doc["ENV_MIN"] | -100.0f;
    constants.ENV_MAX = doc["ENV_MAX"] | 100.0f;
    constants.SCD_CO2_MIN = doc["SCD_CO2_MIN"] | 0.0f;
    constants.SCD_CO2_MAX = doc["SCD_CO2_MAX"] | 60000.0f;
    constants.SCD_T_MIN = doc["SCD_T_MIN"] | -100.0f;
    constants.SCD_T_MAX = doc["SCD_T_MAX"] | 100.0f;
    constants.SCD_RH_MIN = doc["SCD_RH_MIN"] | 0.0f;
    constants.SCD_RH_MAX = doc["SCD_RH_MAX"] | 100.0f;
    constants.ODO_MIN = doc["ODO_MIN"] | 0.0f;
    constants.ODO_MAX = doc["ODO_MAX"] | 1000000.0f;

    Serial.println("Calibration constants loaded from file successfully");
    return true;
}

bool saveCalibrationConstants(const CalibrationConstants& constants) {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS");
        return false;
    }

    // Utworz katalog data jesli nie istnieje
    if (!LittleFS.exists("/data")) {
        LittleFS.mkdir("/data");
    }

    File file = LittleFS.open("/data/calib_nums.json", "w");
    if (!file) {
        Serial.println("Failed to create calibration file");
        return false;
    }

    DynamicJsonDocument doc(16384);
    
    // Zapisz wszystkie stałe do JSON
    // Rezystory obciazajace dla czujnikow TGS (ohm)
    doc["RL_TGS03"] = constants.RL_TGS03;
    doc["RL_TGS02"] = constants.RL_TGS02;
    doc["RL_TGS12"] = constants.RL_TGS12;

    // Parametry kalibracji czujnikow TGS (ppm)
    doc["TGS03_B1"] = constants.TGS03_B1;
    doc["TGS03_A"] = constants.TGS03_A;
    doc["TGS02_B1"] = constants.TGS02_B1;
    doc["TGS02_A"] = constants.TGS02_A;
    doc["TGS12_B1"] = constants.TGS12_B1;
    doc["TGS12_A"] = constants.TGS12_A;

    // Parametry kalibracji gazow elektrochemicznych (ug/m3)
    // CO (K4)
    doc["CO_B0"] = constants.CO_B0;
    doc["CO_B1"] = constants.CO_B1;
    doc["CO_B2"] = constants.CO_B2;
    doc["CO_B3"] = constants.CO_B3;

    // NO (K1)
    doc["NO_B0"] = constants.NO_B0;
    doc["NO_B1"] = constants.NO_B1;
    doc["NO_B2"] = constants.NO_B2;
    doc["NO_B3"] = constants.NO_B3;

    // NO2 (K3)
    doc["NO2_B0"] = constants.NO2_B0;
    doc["NO2_B1"] = constants.NO2_B1;
    doc["NO2_B2"] = constants.NO2_B2;
    doc["NO2_B3"] = constants.NO2_B3;

    // O3 (K2) z kompensacja NO2
    doc["O3_B0"] = constants.O3_B0;
    doc["O3_B1"] = constants.O3_B1;
    doc["O3_B2"] = constants.O3_B2;
    doc["O3_B3"] = constants.O3_B3;
    doc["O3_D"] = constants.O3_D;

    // SO2
    doc["SO2_B0"] = constants.SO2_B0;
    doc["SO2_B1"] = constants.SO2_B1;
    doc["SO2_B2"] = constants.SO2_B2;
    doc["SO2_B3"] = constants.SO2_B3;

    // NH3
    doc["NH3_B0"] = constants.NH3_B0;
    doc["NH3_B1"] = constants.NH3_B1;
    doc["NH3_B2"] = constants.NH3_B2;
    doc["NH3_B3"] = constants.NH3_B3;

    // H2S
    doc["H2S_B0"] = constants.H2S_B0;
    doc["H2S_B1"] = constants.H2S_B1;
    doc["H2S_B2"] = constants.H2S_B2;
    doc["H2S_B3"] = constants.H2S_B3;

    // Parametry kalibracji PID-AH v8
    doc["PID_OFFSET"] = constants.PID_OFFSET;
    doc["PID_B"] = constants.PID_B;
    doc["PID_A"] = constants.PID_A;
    doc["PID_CF"] = constants.PID_CF;

    // Parametry kalibracji HCHO (PMS5003ST)
    doc["HCHO_B1"] = constants.HCHO_B1;
    doc["HCHO_A"] = constants.HCHO_A;
    doc["HCHO_PPB_CF"] = constants.HCHO_PPB_CF;

    // Parametry kalibracji czujnikow PM (SPS30) - nowa konwencja nazewnictwa
    doc["PM1_A"] = constants.PM1_A;
    doc["PM1_B"] = constants.PM1_B;
    doc["PM25_A"] = constants.PM25_A;
    doc["PM25_B"] = constants.PM25_B;
    doc["PM10_A"] = constants.PM10_A;
    doc["PM10_B"] = constants.PM10_B;

    // Parametry kalibracji czujnikow srodowiskowych (SHT40) - nowa konwencja nazewnictwa
    // Ambient sensors
    doc["AMBIENT_TEMP_A"] = constants.AMBIENT_TEMP_A;
    doc["AMBIENT_TEMP_B"] = constants.AMBIENT_TEMP_B;
    doc["AMBIENT_HUMID_A"] = constants.AMBIENT_HUMID_A;
    doc["AMBIENT_HUMID_B"] = constants.AMBIENT_HUMID_B;
    doc["AMBIENT_PRESS_A"] = constants.AMBIENT_PRESS_A;
    doc["AMBIENT_PRESS_B"] = constants.AMBIENT_PRESS_B;
    
    // Dust sensors
    doc["DUST_TEMP_A"] = constants.DUST_TEMP_A;
    doc["DUST_TEMP_B"] = constants.DUST_TEMP_B;
    doc["DUST_HUMID_A"] = constants.DUST_HUMID_A;
    doc["DUST_HUMID_B"] = constants.DUST_HUMID_B;
    doc["DUST_PRESS_A"] = constants.DUST_PRESS_A;
    doc["DUST_PRESS_B"] = constants.DUST_PRESS_B;
    
    // Gas sensors
    doc["GAS_TEMP_A"] = constants.GAS_TEMP_A;
    doc["GAS_TEMP_B"] = constants.GAS_TEMP_B;
    doc["GAS_HUMID_A"] = constants.GAS_HUMID_A;
    doc["GAS_HUMID_B"] = constants.GAS_HUMID_B;
    doc["GAS_PRESS_A"] = constants.GAS_PRESS_A;
    doc["GAS_PRESS_B"] = constants.GAS_PRESS_B;

    // Parametry kalibracji czujnika CO2 (SCD41) - nowa konwencja nazewnictwa
    doc["SCD_CO2_A"] = constants.SCD_CO2_A;
    doc["SCD_CO2_B"] = constants.SCD_CO2_B;
    doc["SCD_T_A"] = constants.SCD_T_A;
    doc["SCD_T_B"] = constants.SCD_T_B;
    doc["SCD_RH_A"] = constants.SCD_RH_A;
    doc["SCD_RH_B"] = constants.SCD_RH_B;

    // Parametry obliczenia ODO (z calib.tcl linie 656-668)
    doc["ODO_A0"] = constants.ODO_A0;
    doc["ODO_A1"] = constants.ODO_A1;
    doc["ODO_A2"] = constants.ODO_A2;
    doc["ODO_A3"] = constants.ODO_A3;
    doc["ODO_A4"] = constants.ODO_A4;
    doc["ODO_A5"] = constants.ODO_A5;

    // Parametry fizyczne dla obliczen temperatury
    doc["B4_TO"] = constants.B4_TO;
    doc["B4_B"] = constants.B4_B;
    doc["B4_RO"] = constants.B4_RO;
    doc["B4_RS"] = constants.B4_RS;
    doc["B4_K"] = constants.B4_K;
    doc["B4_COK"] = constants.B4_COK;

    doc["TGS_TO"] = constants.TGS_TO;
    doc["TGS_B"] = constants.TGS_B;
    doc["TGS_COK"] = constants.TGS_COK;

    // Parametry ADC
    doc["B4_LSB"] = constants.B4_LSB;
    doc["TGS_LSB"] = constants.TGS_LSB;

    // Ograniczenia wartosci (clamp limits)
    doc["GAS_MIN"] = constants.GAS_MIN;
    doc["GAS_MAX"] = constants.GAS_MAX;
    doc["HCHO_MIN"] = constants.HCHO_MIN;
    doc["HCHO_MAX"] = constants.HCHO_MAX;
    doc["PID_MIN"] = constants.PID_MIN;
    doc["PID_MAX"] = constants.PID_MAX;

    // Ograniczenia dla nowych czujnikow
    doc["PM_MIN"] = constants.PM_MIN;
    doc["PM_MAX"] = constants.PM_MAX;
    doc["ENV_MIN"] = constants.ENV_MIN;
    doc["ENV_MAX"] = constants.ENV_MAX;
    doc["SCD_CO2_MIN"] = constants.SCD_CO2_MIN;
    doc["SCD_CO2_MAX"] = constants.SCD_CO2_MAX;
    doc["SCD_T_MIN"] = constants.SCD_T_MIN;
    doc["SCD_T_MAX"] = constants.SCD_T_MAX;
    doc["SCD_RH_MIN"] = constants.SCD_RH_MIN;
    doc["SCD_RH_MAX"] = constants.SCD_RH_MAX;
    doc["ODO_MIN"] = constants.ODO_MIN;
    doc["ODO_MAX"] = constants.ODO_MAX;

    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write calibration file");
        file.close();
        return false;
    }

    file.close();
    Serial.println("Calibration constants saved to file successfully");
    return true;
}
