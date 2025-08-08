#ifndef CALIB_CONSTANTS_H
#define CALIB_CONSTANTS_H

#include <Arduino.h>

// Structure to hold all calibration constants
struct CalibrationConstants {
    // B4 sensor constants
    float B4_RS;
    float B4_K;
    float B4_TO;
    float B4_COK;
    float B4_B;
    float B4_RO;
    float B4_LSB;
    
    // TGS sensor constants
    float TGS_TO;
    float TGS_COK;
    float TGS_B;
    float TGS_LSB;
    
    // TGS03 constants
    float RL_TGS03;
    float TGS03_B1;
    float TGS03_A;
    
    // TGS02 constants
    float RL_TGS02;
    float TGS02_B1;
    float TGS02_A;
    
    // TGS12 constants
    float RL_TGS12;
    float TGS12_B1;
    float TGS12_A;
    
    // Gas sensor constants
    float CO_B0, CO_B1, CO_B2, CO_B3;
    float NO_B0, NO_B1, NO_B2, NO_B3;
    float NO2_B0, NO2_B1, NO2_B2, NO2_B3;
    float O3_B0, O3_B1, O3_B2, O3_B3, O3_D;
    float SO2_B0, SO2_B1, SO2_B2, SO2_B3;
    float H2S_B0, H2S_B1, H2S_B2, H2S_B3;
    float NH3_B0, NH3_B1, NH3_B2, NH3_B3;
    
    // Temperature compensation constants
    float K1_temp, K2_temp, K3_temp, K4_temp;
    
    // Gas limits
    float GAS_MIN;
    float GAS_MAX;
    
    // HCHO sensor constants
    float HCHO_PPB_CF;
    float HCHO_B1;
    float HCHO_A;
    float HCHO_MIN;
    float HCHO_MAX;
    
    // PID sensor constants
    float PID_OFFSET;
    float PID_A;
    float PID_B;
    float PID_CF;
    float PID_MIN;
    float PID_MAX;
    
    // ODO sensor constants
    float ODO_A0, ODO_A1, ODO_A2, ODO_A3, ODO_A4, ODO_A5;
    float ODO_MIN;
    float ODO_MAX;
    
    // PM sensor constants - new naming convention
    float PM1_A, PM1_B;      // PM1 multiplication and additive
    float PM25_A, PM25_B;    // PM2.5 multiplication and additive
    float PM10_A, PM10_B;    // PM10 multiplication and additive
    float PM_MIN;
    float PM_MAX;
    
    // Environmental sensor constants - new naming convention
    // Ambient sensors
    float AMBIENT_TEMP_A, AMBIENT_TEMP_B;
    float AMBIENT_HUMID_A, AMBIENT_HUMID_B;
    float AMBIENT_PRESS_A, AMBIENT_PRESS_B;
    
    // Dust sensors
    float DUST_TEMP_A, DUST_TEMP_B;
    float DUST_HUMID_A, DUST_HUMID_B;
    float DUST_PRESS_A, DUST_PRESS_B;
    
    // Gas sensors
    float GAS_TEMP_A, GAS_TEMP_B;
    float GAS_HUMID_A, GAS_HUMID_B;
    float GAS_PRESS_A, GAS_PRESS_B;
    
    // Environmental limits
    float ENV_MIN;
    float ENV_MAX;
    
    // CO2 sensor constants - new naming convention
    float SCD_CO2_A, SCD_CO2_B;    // CO2 multiplication and additive
    float SCD_T_A, SCD_T_B;        // Temperature multiplication and additive
    float SCD_RH_A, SCD_RH_B;      // Humidity multiplication and additive
    float SCD_CO2_MIN;
    float SCD_CO2_MAX;
    float SCD_T_MIN;
    float SCD_T_MAX;
    float SCD_RH_MIN;
    float SCD_RH_MAX;
};

// Global instance
extern CalibrationConstants calibConstants;

// Function declarations
bool loadCalibrationConstants(CalibrationConstants& constants);
bool saveCalibrationConstants(const CalibrationConstants& constants);
void setDefaultCalibrationConstants(CalibrationConstants& constants);

#endif // CALIB_CONSTANTS_H 