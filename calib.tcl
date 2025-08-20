# -----------------------------------------------------------------------------
# Slownik kalibracji
# SCube v1.0 
# nowa wersja elchem!
# nowe funkcje elchem

set calibration_dict {

  "obliczenie temperatury K1 C" {
    slot_name {K1_T}
    values {RAW_T K1_3 RAW_V K1_4}
    params {
      T {[B4_T $RAW_T $RAW_V]}
    }
    equation {$T}
    clamp {min -30 max 1000}}

  "obliczenie temperatury K2 C" {
    slot_name {K2_T}
    values {RAW_T K2_3 RAW_V K2_4}
    params {
      T {[B4_T $RAW_T $RAW_V]}
    }
    equation {$T}
    clamp {min -30 max 1000}}

  "obliczenie temperatury K3 C" {
    slot_name {K3_T}
    values {RAW_T K3_3 RAW_V K3_4}
    params {
      T {[B4_T $RAW_T $RAW_V]}
    }
    equation {$T}
    clamp {min -30 max 1000}}
		
  "obliczenie temperatury K4 C" {
    slot_name {K4_T}
    values {RAW_T K4_3 RAW_V K4_4}
    params {
      T {[B4_T $RAW_T $RAW_V]}
    }
    equation {$T}
    clamp {min -30 max 1000}}
	
 "obliczenie temperatury K5 C" {
    slot_name {K5_3}
    values {RAW_T K5_3 RAW_V K5_4}
    params {
      T {[B4_T $RAW_T $RAW_V]}
    }
    equation {$T}
    clamp {min -30 max 250}}

 "obliczenie temperatury K6 C" {
   slot_name {K6_1}
    values {C1 K6_1 C2 K6_2}
    params {
      T {[TGS_T $C1 $C2]}
    }
    equation {$T}
    clamp {min -30 max 250}}

  "obliczenie temperatury K7 C" {
   slot_name {K7_T}
    values {C1 K7_1 C2 K7_2}
    params {
      T {[TGS_T $C1 $C2]}
    }
    equation {$T}
    clamp {min -30 max 250}}
 
  "obliczenie temperatury K8 C" {
    slot_name {K8_1}
    values {C1 K8_1 C2 K8_2}
    params {
      T {[TGS_T $C1 $C2]}
    }
    equation {$T}
    clamp {min -30 max 250}}
	
  "obliczenie temperatury K9 C" {
    slot_name {K9_1}
    values {C1 K9_1 C2 K9_2}
    params {
      T {[TGS_T $C1 $C2]}
    }
    equation {$T}
    clamp {min -30 max 250}}
	
  "obliczenie temperatury K12 C" {
    slot_name {K12_1}
    values {C1 K12_1 C2 K12_2}
    params {
      T {[TGS_T $C1 $C2]}
    }
    equation {$T}
    clamp {min -30 max 250}}	

  "obliczenie napiecia K1 mV" {
    slot_name {K1_V}
    values {RAW K1_4}
    params {
      V {[B4_mV $RAW]}
    }
    equation {$V}
    clamp {min 0 max 6000}}

  "obliczenie napiecia K2 mV" {
    slot_name {K2_V}
    values {RAW K2_4}
    params {
      V {[B4_mV $RAW]}
    }
    equation {$V}
    clamp {min 0 max 6000}}

 "obliczenie napiecia K3 mV" {
    slot_name {K3_V}
    values {RAW K3_4}
    params {
      V {[B4_mV $RAW]}
    }
    equation {$V}
    clamp {min 0 max 6000}}
	
  "obliczenie napiecia K4 mV" {
    slot_name {K4_V}
    values {RAW K4_4}
    params {
      V {[B4_mV $RAW]}
    }
    equation {$V}
    clamp {min 0 max 6000}}

 "obliczenie napiecia K5 mV" {
    slot_name {K5_V}
    values {RAW K5_4}
    params {
      V {[B4_mV $RAW]}
    }
    equation {$V}
    clamp {min 0 max 6000}}

  "obliczenie napiecia K6 mV" {
   slot_name {K6_2}
    values {C4 K6_4}
    params {
      V {[TGSv4_mV $C4]}
    }
    equation {$V}
    clamp {min 0 max 7000}}
	
  "obliczenie napiecia K7 mV FIX" {
    slot_name {K7_V}
    values {C3 K7_3 C4 K7_4}
    params {
      V {[TGSv4_mV_FIX $C3 $C4]}
    }
    equation {$V}
    clamp {min 0 max 7000}}	

   "obliczenie napiecia K8 mV" {
   slot_name {K8_2}
    values {C4 K8_4}
    params {
      V {[TGSv4_mV $C4]}
    }
    equation {$V}
    clamp {min 0 max 7000}}
	
  "obliczenie napiecia K9 mV" {
    slot_name {K9_2}
    values {C4 K9_4}
    params {
      V {[TGSv4_mV $C4]}
    }
    equation {$V}
    clamp {min 0 max 7000}}
	
  "obliczenie napiecia K12 mV" {
    slot_name {K12_2}
    values {C4 K12_4}
    params {
      V {[TGSv4_mV $C4]}
    }
    equation {$V}
    clamp {min 0 max 7000}}	

   "obliczenie TGS12 ppm v4" {
    slot_name {TGS12}
    values {C3 Kx_3 C4 Kx_4}
    params {
 	  RL 10000.0
      b1 1000.0
      a  0.0
      TGS_value {[TGSv4_ppm $C3 $C4 $RL $RL 0 0 $b1 $a]}
    }
    equation {$TGS_value}
    clamp {min 0 max 1000000}}

   "obliczenie TGS12 OHM v4" {
    slot_name {TGS12_OHM}
    values {C3 Kx_3 C4 Kx_4}
    params {
      RL 10000.0
      TGS_value {[TGSv4_ohm $C3 $C4 $RL]}
    }
    equation {$TGS_value}
    clamp {min 0 max 10000000}}

  "obliczenie TGS02 ppm v4 FIX" {
    slot_name {TGS02}
    values {C3 K7_3 C4 K7_4}
    params {
	  RL 3300.0
      b1 50.0
      a  4.0
      TGS_value {[TGSv4_ppm_FIX $C3 $C4 $RL $RL 0 0 $b1 $a]}
    }
    equation {$TGS_value}
    clamp {min 0 max 1000000}}

  "obliczenie TGS02 OHM v4 FIX" {
    slot_name {TGS02_OHM}
    values {C3 K7_3 C4 K7_4}
    params {
      RL 3300.0
      TGS_value {[TGSv4_ohm_FIX $C3 $C4 $RL]}
    }
    equation {$TGS_value}
    clamp {min 0 max 10000000}}

  "obliczenie TGS03 ppm v4" {
    slot_name {TGS03}
    values {C3 K6_3 C4 K6_4}
    params {
      RL 10000.0
      b1 30.0
      a  -1.0
      TGS_value {[TGSv4_ppm $C3 $C4 $RL $RL 0 0 $b1 $a]}
    }
    equation {$TGS_value}
    clamp {min 0 max 1000000}}
	
  "obliczenie TGS03 OHM v4" {
    slot_name {TGS03_OHM}
    values {C3 K6_3 C4 K6_4}
    params {
      RL 10000.0
      TGS_value {[TGSv4_ohm $C3 $C4 $RL]}
    }
    equation {$TGS_value}
    clamp {min 0 max 10000000}}

  "obliczenie TGS00 ppm v4" {
    slot_name {TGS00}
    values {C3 Kx_3 C4 Kx_4}
    params {
      RL 10000.0
      b1 20.0
      a  -6.0
      TGS_value {[TGSv4_ppm $C3 $C4 $RL $RL 0 0 $b1 $a]}
    }
    equation {$TGS_value}
    clamp {min 0 max 1000000}}

  "obliczenie TGS00 OHM v4" {
    slot_name {TGS00_OHM}
    values {C3 Kx_3 C4 Kx_4}
    params {
      RL 10000.0
      TGS_value {[TGSv4_ohm $C3 $C4 $RL]}
    }
    equation {$TGS_value}
    clamp {min 0 max 10000000}}

  "kalibracja HCHO ppb PMS5003ST" {
    slot_name {HCHO}
    values {RAW US0_PMS5003ST_HCHO_UG}
    params {
      b1 1.0
      a  0.0
	  PPB_CF 0.814
      fx_lin {[Linear_basic $b1 $RAW $a]}
    }
    equation {$fx_lin * $PPB_CF}
    clamp {min 0 max 40000}}

  "temperatura HCHO PMS5003ST" {
    slot_name {HCHO.TA}
    values {V US0_PMS5003ST_TEMP}
    params {}
    equation {$V}
    clamp {min -100 max 100}}

  "wilgotnosc HCHO PMS5003ST" {
    slot_name {HCHO.RH}
    values {V US0_PMS5003ST_HUMID}
    params {}
    equation {$V}
    clamp {min 0 max 100}}
  
  "kalibracja PID-AH v8" {
    slot_name {PID}
    values {WRK Kx_3 OFS Kx_4}
    params {
      mv {($WRK+$OFS) / 256.0}
      o 140.0
      b 0.0
      a 3.0
      cf 1.0
    }
    equation {((($mv - $o) * $a + $b) / $cf)}
    clamp {min 0 max 40000}}

  "kalibracja PID-AH v8 mV" {
    slot_name {PID_mV}
    values {WRK Kx_3 OFS Kx_4}
    params {
      mv {($WRK+$OFS) / 256.0}
    }
    equation {$mv}
    clamp {min 0 max 5000}}

  "kalibracja LUX" {
    slot_name {LUX}
    values {L I2C9_LUX}
    params {
      b1 1.0
      a  0.0
      fx_lin {[Linear_basic $b1 $L $a]}
    }
    equation {$L}
    clamp {min 0 max 1000000}}

  "temperatura toru pylowego" {
    slot_name {DUST_TEMP}
    values {V I2C11_TEMP}
    params {}
    equation {$V}
    clamp {min -100 max 100}}

  "wilgotnosc toru pylowego" {
    slot_name {DUST_HUMID}
    values {V I2C11_HUMID}
    params {}
    equation {$V}
    clamp {min 0 max 100}}

  "temperatura aux toru pylowego" {
    slot_name {DUST_TEMP_AUX}
    values {V I2C11_TEMP_AUX}
    params {}
    equation {$V}
    clamp {min -100 max 100}}

  "wilgotnosc aux toru pylowego" {
    slot_name {DUST_HUMID_AUX}
    values {V I2C11_HUMID_AUX}
    params {}
    equation {$V}
    clamp {min 0 max 100}}

  "cisnienie toru pylowego" {
    slot_name {DUST_PRESS}
    values {V I2C11_PRESS}
    params {}
    equation {$V}
    clamp {min 0 max 2000}}

  "temperatura toru gazowego" {
    slot_name {GAS_TEMP}
    values {V I2C12_TEMP}
    params {}
    equation {$V}
    clamp {min -100 max 100}}

  "wilgotnosc toru gazowego" {
    slot_name {GAS_HUMID}
    values {V I2C12_HUMID}
    params {}
    equation {$V}
    clamp {min 0 max 100}}

  "temperatura aux toru gazowego" {
    slot_name {GAS_TEMP_AUX}
    values {V I2C12_TEMP_AUX}
    params {}
    equation {$V}
    clamp {min -100 max 100}}

  "wilgotnosc aux toru gazowego" {
    slot_name {GAS_HUMID_AUX}
    values {V I2C12_HUMID_AUX}
    params {}
    equation {$V}
    clamp {min 0 max 100}}

  "cisnienie toru gazowego" {
    slot_name {GAS_PRESS}
    values {V I2C12_PRESS}
    params {}
    equation {$V}
    clamp {min 0 max 2000}}

  "temperatura zewnetrzna" {
    slot_name {AMBIENT_TEMP}
    values {T I2C10_TEMP}
    params {
      b1 1.0
      a  0.0
      fx_lin {[Linear_basic $b1 $T $a]}
    }
    equation {$fx_lin}
    clamp {min -100 max 100}}

  "wilgotnosc zewnetrzna" {
    slot_name {AMBIENT_HUMID}
    values {H I2C10_HUMID}
    params {
      b1 1.0
      a  0.0
      fx_lin {[Linear_advanced $b1 $H $a]}
    }
    equation {$fx_lin}
    clamp {min 0 max 100}}

  "temperatura aux zewnetrzna" {
    slot_name {AMBIENT_TEMP_AUX}
    values {T I2C10_TEMP_AUX}
    params {
      b1 1.0
      a  0.0
      fx_lin {[Linear_basic $b1 $T $a]}
    }
    equation {$fx_lin}
    clamp {min -100 max 100}}

  "wilgotnosc aux zewnetrzna" {
    slot_name {AMBIENT_HUMID_AUX}
    values {H I2C10_HUMID_AUX}
    params {
      b1 1.0
      a  0.0
      fx_lin {[Linear_advanced $b1 $H $a]}
    }
    equation {$fx_lin}
    clamp {min 0 max 100}}

  "cisnienie zewnetrzne" {
    slot_name {AMBIENT_PRESS}
    values {P I2C10_PRESS}
    params {
      b1 1.0
      a  0.0
      fx_lin {[Linear_basic $b1 $P $a]}
    }
    equation {$fx_lin}
    clamp {min 0 max 2000}}

  "wilgotnosc symulowana zewnetrzna" {
    slot_name {AMBIENT_HUMID_SIM}
    values {RH_tor DUST_HUMID T_tor DUST_TEMP T_zew AMBIENT_TEMP}
    params {
      Ps_tor {611.21 * exp((18.678 - $T_tor / 234.5) * ($T_tor / (257.14 + $T_tor)))}
      Ps_zew {611.21 * exp((18.678 - $T_zew / 234.5) * ($T_zew / (257.14 + $T_zew)))}
      Rw 461.5
      AH_tor {($RH_tor * $Ps_tor) / ($Rw * ($T_tor + 273.15) * 100.0)}
      RH_zew {($AH_tor * ($Rw * ($T_zew + 273.15) * 100.0)) / $Ps_zew}
    }
    equation {$RH_zew}
    clamp {min 0 max 100}}

  "temperatura grzalki pylowej NTC1B" {
    slot_name {DUST_HEATER_NTC}
    values {V ADC5_NTC}
    params {}
    equation {$V}
    clamp {min -100 max 10000}}

  "ntc toru pylowego NTC1A" {
    slot_name {DUST_AIR_NTC}
    values {V ADC4_NTC}
    params {}
    equation {$V}
    clamp {min -100 max 10000}}

  "temperatura grzalki gazowej NTC2B" {
    slot_name {GAS_HEATER_NTC}
    values {V ADC2_NTC}
    params {}
    equation {$V}
    clamp {min -100 max 10000}}

  "ntc toru gazowego NTC2A" {
    slot_name {GAS_AIR_NTC}
    values {V ADC1_NTC}
    params {}
    equation {$V}
    clamp {min -100 max 10000}}

  "ntc wnetrza" {
    slot_name {INSIDE_NTC}
    values {V ADC3_NTC}
    params {}
    equation {$V}
    clamp {min -100 max 10000}}

  "rpm wentylatora toru gazowego" {
    slot_name {DUST_RPM}
    values {V FAN1_TACHO}
    params {}
    equation {$V}
    clamp {min 0 max 10000}}

  "rpm wentylatora toru gazowego" {
    slot_name {GAS_RPM}
    values {V FAN2_TACHO}
    params {}
    equation {$V}
    clamp {min 0 max 10000}}

  "SCD T" {
    slot_name {SCD_T}
    values {T I2C5_TEMP}
    params {}
    equation {$T}
    clamp {min -30 max 250}}

  "SCD RH" {
    slot_name {SCD_RH}
    values {RH I2C5_HUMID}
    params {}
    equation {$RH}
    clamp {min 0 max 100}}

  "SCD CO2" {
    slot_name {SCD_CO2}
    values {CO2 I2C5_PRESS}
    params {}
    equation {$CO2}
    clamp {min 0 max 60000}}

  "kalibracja CO ug/m3" {
    slot_name {CO}
    values {WRK K3_1 AUX K3_2 TRM K3_T VCC K3_V}
    params {
    b0  70.0
    b1  0.4
	  b2  -0.4
	  b3  0
    }
    equation { $b0 + $b1 * $WRK + $b2 * $AUX + $b3 * $TRM }
    clamp {min 0 max 50000}}

  "kalibracja NO ug/m3" {
    slot_name {NO}
    values {WRK K4_1 AUX K4_2 TRM K4_T VCC K4_V}
    params {
    b0  0.2
    b1  0.03
	  b2  -0.03
	  b3  0
    }
    equation { $b0 + $b1 * $WRK + $b2 * $AUX + $b3 * $TRM }
    clamp {min 0 max 50000}}

  "kalibracja SO2 ug/m3" {
    slot_name {SO2}
    values {WRK Kx_1 AUX Kx_2 TRM Kx_3 VCC Kx_4}
    params {
	b0  4
    b1  0.005
	b2  -0.008
	b3  0.0
	T {[B4_T $TRM $VCC]}
    }
    equation { $b0 + $b1 * $WRK + $b2 * $AUX + $b3 * $T }
    clamp {min 0 max 50000}}

  "kalibracja NO2 ug/m3" {
    slot_name {NO2}
    values {WRK K2_1 AUX K2_2 TRM K2_T VCC K2_V}
    params {
    b0  20.0
    b1  -0.09
	  b2  -0.063
	  b3  0
    }
    equation { $b0 + $b1 * $WRK + $b2 * $AUX + $b3 * $TRM }
    clamp {min 0 max 50000}}

  "kalibracja O3 ug/m3" {
    slot_name {O3}
    values {WRK K5_1 AUX K5_2 TRM K5_T VCC K5_V NO2_COMP NO2}
    params {
    b0  -20.0
    b1  -0.09
	b2  -0.054
	b3  0
	d   -1.35
	T {[B4_T $TRM $VCC]}
    }
    equation { $b0 + $b1 * $WRK + $b2 * $AUX + $b3 * $T + $d * $NO2_COMP }
    clamp {min 0 max 50000}}
  
  "kalibracja HCL ug/m3" {
    slot_name {HCL}
    values {WRK Kx_1}
    params {
      b1 0.0006
      b0  3.0
      fx_lin {[Linear_basic $b1 $WRK $b0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 40000}}

  "kalibracja HCN ug/m3" {
    slot_name {HCN}
    values {WRK Kx_1}
    params {
      b1 0.006
      b0  3.0
      fx_lin {[Linear_basic $b1 $WRK $b0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 40000}}

  "kalibracja NH3 ug/m3" {
    slot_name {NH3}
    values {WRK Kx_1 TRM Kx_T VCC Kx_V}
    params {
      b0 0.2
      b1 0.00014
      b3 0
    }
    equation { $b0 + $b1 * $WRK + $b3 * $TRM }
    clamp {min 0 max 50000}}

  "kalibracja H2S ug/m3" {
    slot_name {H2S}
    values {WRK K1_1 AUX K1_2 TRM K1_T VCC K1_V}
    params {
      b0 0.2
	  b1 0.00005
      b2 -0.00009
	  b3 0.0 
    }
    equation { $b0 + $b1 * $WRK + $b2 * $AUX + $b3 * $TRM }
    clamp {min 0 max 50000}}

 "wyliczenie ODO w oparciu o ug/m3" {
    slot_name {ODO}
    values {T02 TGS02 T03 TGS03 T12 TGS12 H H2S N NH3}
    params {
      A0 -0.9641
      A1 2.1529
      A2 1.6481
      A3 0.0164
      A4 0.2739
      A5 -1.0731
    }
    equation {$A0 + ($A1 * $T02) + ($A2 * $T03) + ($A3 * $T12) + ($A4 * $H) + ($A5 * $N)}
    clamp {min 0 max 1000000}}

  "kalibracja CO ppb" {
    slot_name {CO}
    values {ug CO}
    params {
      ppb_div 1.16
    }
    equation {$ug / $ppb_div}
    clamp {min 0 max 50000}}

  "kalibracja NO2 ppb" {
    slot_name {NO2}
    values {ug NO2}
    params {
      ppb_div 1.913
    }
    equation {$ug / $ppb_div}
    clamp {min 0 max 50000}}

  "kalibracja O3 ppb" {
    slot_name {O3}
    values {ug O3}
    params {
      ppb_div 2.0
    }
    equation {$ug / $ppb_div}
    clamp {min 0 max 50000}}

  "kalibracja SO2 ppb" {
    slot_name {SO2}
    values {ug SO2}
    params {
      ppb_div 2.66
    }
    equation {$ug / $ppb_div}
    clamp {min 0 max 50000}}

  "kalibracja NO ppb" {
    slot_name {NO}
    values {ug NO}
    params {
      ppb_div 1.247
    }
    equation {$ug / $ppb_div}
    clamp {min 0 max 50000}}

  "kalibracja HCL ppb" {
    slot_name {HCL}
    values {ug HCL}
    params {
      ppb_div 1.46
    }
    equation {$ug / $ppb_div}
    clamp {min 0 max 50000}}

  "kalibracja HCN ppb" {
    slot_name {HCN}
    values {ug HCN}
    params {
      ppb_div 1.1
    }
    equation {$ug / $ppb_div}
    clamp {min 0 max 50000}}

  "kalibracja NH3 ppb" {
    slot_name {NH3}
    values {ug NH3}
    params {
      ppb_div 0.7079
    }
    equation {$ug / $ppb_div}
    clamp {min 0 max 50000}}

  "kalibracja H2S ppb" {
    slot_name {H2S}
    values {ug H2S}
    params {
      ppb_div 1.4166
    }
    equation {$ug / $ppb_div}
    clamp {min 0 max 50000}}
	
	  "raw PM1" {
    slot_name {PM1_raw}
    values {RAW US3_SPS30_PM01}
    params {}
    equation {$RAW}
    clamp {min 0 max 50000}}

  "raw PM25" {
    slot_name {PM25_raw}
    values {RAW US3_SPS30_PM25}
    params {}
    equation {$RAW}
    clamp {min 0 max 50000}}

  "raw PM10" {
    slot_name {PM10_raw}
    values {RAW US3_SPS30_PM10}
    params {}
    equation {$RAW}
    clamp {min 0 max 50000}}

  "raw PM1_2" {
    slot_name {PM1_2_raw}
    values {RAW US0_PMSA003_PM01A}
    params {}
    equation {$RAW}
    clamp {min 0 max 50000}}

  "raw PM25_2" {
    slot_name {PM25_2_raw}
    values {RAW US0_PMSA003_PM25A}
    params {}
    equation {$RAW}
    clamp {min 0 max 50000}}

  "raw PM10_2" {
    slot_name {PM10_2_raw}
    values {RAW US0_PMSA003_PM10A}
    params {}
    equation {$RAW}
    clamp {min 0 max 50000}}
	

"PMS RAW pyl PM01_S ug/m3 " {
    slot_name {PMS_PM01S}
    values {RAW US0_PMSA003_PM01S}
    params {}
    equation {$RAW}
    clamp {min 0 max 1000000}}

"PMS RAW pyl PM25_S ug/m3 " {
    slot_name {PMS_PM25S}
    values {RAW US0_PMSA003_PM25S}
    params {}
    equation {$RAW}
    clamp {min 0 max 1000000}}

"PMS RAW pyl PM10_S ug/m3 " {
    slot_name {PMS_PM10S}
    values {RAW US0_PMSA003_PM10S}
    params {}
    equation {$RAW}
    clamp {min 0 max 1000000}}

"PMS RAW pyl PM01_A ug/m3 " {
    slot_name {PMS_PM01A}
    values {RAW US0_PMSA003_PM01A}
    params {}
    equation {$RAW}
    clamp {min 0 max 1000000}}

"PMS RAW pyl PM25_A ug/m3 " {
    slot_name {PMS_PM25A}
    values {RAW US0_PMSA003_PM25A}
    params {}
    equation {$RAW}
    clamp {min 0 max 1000000}}

"PMS RAW pyl PM10_A ug/m3 " {
    slot_name {PMS_PM10A}
    values {RAW US0_PMSA003_PM10A}
    params {}
    equation {$RAW}
    clamp {min 0 max 1000000}}

"PMS Ilosciowo pyl PM0.3 particles/l " {
    slot_name {PMS_PM0o3}
    values {RAW US0_PMSA003_PM0o3}
    params {
      b1 10.0
      fx_lin {[Linear_advanced $b1 $RAW 0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 1000000}}

"PMS Ilosciowo pyl PM0.5 particles/l " {
    slot_name {PMS_PM0o5}
    values {RAW US0_PMSA003_PM0o5}
    params {
      b1 10.0
      fx_lin {[Linear_advanced $b1 $RAW 0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 1000000}}
	
"PMS Ilosciowo pyl PM1.0 particles/l " {
    slot_name {PMS_PM1o0}
    values {RAW US0_PMSA003_PM1o0}
    params {
      b1 10.0
      fx_lin {[Linear_advanced $b1 $RAW 0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 1000000}}	

"PMS Ilosciowo pyl PM2.5 particles/l " {
    slot_name {PMS_PM2o5}
    values {RAW US0_PMSA003_PM2o5}
    params {
      b1 10.0
      fx_lin {[Linear_advanced $b1 $RAW 0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 1000000}}		
	
"PMS Ilosciowo pyl PM5.0 particles/l " {
    slot_name {PMS_PM5o0}
    values {RAW US0_PMSA003_PM5o0}
    params {
      b1 10.0
      fx_lin {[Linear_advanced $b1 $RAW 0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 1000000}}		
	
"PMS Ilosciowo pyl PM10 particles/l " {
    slot_name {PMS_PM10o}
    values {RAW US0_PMSA003_PM10o}
    params {
      b1 10.0
      fx_lin {[Linear_advanced $b1 $RAW 0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 1000000}}	

"SPS RAW pyl PM1 ug/m3 " {
    slot_name {SPS_PM01}
    values {RAW US3_SPS30_PM01}
    params {}
    equation {$RAW}
    clamp {min 0 max 1000000}}
	
"SPS RAW pyl PM25 ug/m3 " {
    slot_name {SPS_PM25}
    values {RAW US3_SPS30_PM25}
    params {}
    equation {$RAW}
    clamp {min 0 max 1000000}}
	
"SPS RAW pyl PM40 ug/m3 " {
    slot_name {SPS_PM40}
    values {RAW US3_SPS30_PM40}
    params {}
    equation {$RAW}
    clamp {min 0 max 1000000}}
	
"SPS RAW pyl PM10 ug/m3 " {
    slot_name {SPS_PM010}
    values {RAW US3_SPS30_PM10}
    params {}
    equation {$RAW}
    clamp {min 0 max 1000000}}

"SPS Ilosciowo pyl PM0.5 particles/l " {
    slot_name {SPS_PM0o5}
    values {RAW US3_SPS30_PM0o5}
    params {
      b1 1000.0
      fx_lin {[Linear_advanced $b1 $RAW 0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 100000000}}	
	
"SPS Ilosciowo pyl PM1.0 particles/l " {
    slot_name {SPS_PM1o0}
    values {RAW US3_SPS30_PM1o0}
    params {
      b1 1000.0
      fx_lin {[Linear_advanced $b1 $RAW 0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 100000000}}	

"SPS Ilosciowo pyl PM2.5 particles/l " {
    slot_name {SPS_PM2o5}
    values {RAW US3_SPS30_PM2o5}
    params {
      b1 1000.0
      fx_lin {[Linear_advanced $b1 $RAW 0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 100000000}}	

"SPS Ilosciowo pyl PM4.0 particles/l " {
    slot_name {SPS_PM4o0}
    values {RAW US3_SPS30_PM4o0}
    params {
      b1 1000.0
      fx_lin {[Linear_advanced $b1 $RAW 0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 100000000}}	

"SPS Ilosciowo pyl PM10 particles/l " {
    slot_name {SPS_PM10o}
    values {RAW US3_SPS30_PM10o}
    params {
      b1 1000.0
      fx_lin {[Linear_advanced $b1 $RAW 0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 100000000}}	
	
"SPS średni rozmiar cząstki nm " {
    slot_name {SPS_PSize}
    values {RAW US3_SPS30_SIZE}
    params {
      b1 1.0
      fx_lin {[Linear_advanced $b1 $RAW 0]}
    }
    equation {$fx_lin}
    clamp {min 0 max 50000}}		

 "kalibracja pylu PM10_2 ug/m3" {
    slot_name {PM10_2}
    values {RAW US0_PMSA003_PM10A}
    params {
      b1 1.0
      a  0.0
      fx_lin {[Linear_advanced $b1 $RAW $a]}
    }
    equation {$fx_lin}
    clamp {min 0 max 5000}}

 "kalibracja pylu PM25_2 ug/m3" {
    slot_name {PM25_2}
    values {RAW US0_PMSA003_PM25A}
    params {
      b1 1.0
      a  0.0
      fx_lin {[Linear_advanced $b1 $RAW $a]}
    }
    equation {$fx_lin}
    clamp {min 0 max 5000}}

 "kalibracja pylu PM1_2 ug/m3" {
    slot_name {PM1_2}
    values {RAW US0_PMSA003_PM01A}
    params {
      b1 1.0
      a  0.0
      fx_lin {[Linear_advanced $b1 $RAW $a]}
    }
    equation {$fx_lin}
    clamp {min 0 max 5000}}

 "kalibracja pylu PM10 ug/m3" {
    slot_name {PM10}
    values {RAW US3_SPS30_PM10}
    params {
      b1 1.0
      a  0.0
      fx_lin {[Linear_advanced $b1 $RAW $a]}
    }
    equation {$fx_lin}
    clamp {min 0 max 5000}}

 "kalibracja pylu PM25 ug/m3" {
    slot_name {PM25}
    values {RAW US3_SPS30_PM25}
    params {
     b1 1.0
     a  0.0
     fx_lin {[Linear_advanced $b1 $RAW $a]}
    }
    equation {$fx_lin}
    clamp {min 0 max 5000}}

  "kalibracja pylu PM1 ug/m3" {
    slot_name {PM1}
    values {RAW US3_SPS30_PM01}
    params {
      b1 1.0
      a  0.0
      fx_lin {[Linear_advanced $b1 $RAW $a]}
    }
    equation {$fx_lin}
    clamp {min 0 max 5000}}

  "RAW K1_1" { slot_name {RAW_K1_1} values {RAW K1_1} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K1_2" { slot_name {RAW_K1_2} values {RAW K1_2} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K1_3" { slot_name {RAW_K1_3} values {RAW K1_3} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K1_4" { slot_name {RAW_K1_4} values {RAW K1_4} params {} equation {$RAW} clamp {min -100000000 max 100000000}}

  "RAW K2_1" { slot_name {RAW_K2_1} values {RAW K2_1} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K2_2" { slot_name {RAW_K2_2} values {RAW K2_2} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K2_3" { slot_name {RAW_K2_3} values {RAW K2_3} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K2_4" { slot_name {RAW_K2_4} values {RAW K2_4} params {} equation {$RAW} clamp {min -100000000 max 100000000}}

  "RAW K3_1" { slot_name {RAW_K3_1} values {RAW K3_1} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K3_2" { slot_name {RAW_K3_2} values {RAW K3_2} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K3_3" { slot_name {RAW_K3_3} values {RAW K3_3} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K3_4" { slot_name {RAW_K3_4} values {RAW K3_4} params {} equation {$RAW} clamp {min -100000000 max 100000000}}

  "RAW K4_1" { slot_name {RAW_K4_1} values {RAW K4_1} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K4_2" { slot_name {RAW_K4_2} values {RAW K4_2} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K4_3" { slot_name {RAW_K4_3} values {RAW K4_3} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K4_4" { slot_name {RAW_K4_4} values {RAW K4_4} params {} equation {$RAW} clamp {min -100000000 max 100000000}}

  "RAW K5_1" { slot_name {RAW_K5_1} values {RAW K5_1} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K5_2" { slot_name {RAW_K5_2} values {RAW K5_2} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K5_3" { slot_name {RAW_K5_3} values {RAW K5_3} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K5_4" { slot_name {RAW_K5_4} values {RAW K5_4} params {} equation {$RAW} clamp {min -100000000 max 100000000}}

  "RAW K6_1" { slot_name {RAW_K6_1} values {RAW K6_1} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K6_2" { slot_name {RAW_K6_2} values {RAW K6_2} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K6_3" { slot_name {RAW_K6_3} values {RAW K6_3} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K6_4" { slot_name {RAW_K6_4} values {RAW K6_4} params {} equation {$RAW} clamp {min -100000000 max 100000000}}

  "RAW K7_1" { slot_name {RAW_K7_1} values {RAW K7_1} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K7_2" { slot_name {RAW_K7_2} values {RAW K7_2} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K7_3" { slot_name {RAW_K7_3} values {RAW K7_3} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K7_4" { slot_name {RAW_K7_4} values {RAW K7_4} params {} equation {$RAW} clamp {min -100000000 max 100000000}}

  "RAW K8_1" { slot_name {RAW_K8_1} values {RAW K8_1} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K8_2" { slot_name {RAW_K8_2} values {RAW K8_2} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K8_3" { slot_name {RAW_K8_3} values {RAW K8_3} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K8_4" { slot_name {RAW_K8_4} values {RAW K8_4} params {} equation {$RAW} clamp {min -100000000 max 100000000}}

  "RAW K9_1" { slot_name {RAW_K9_1} values {RAW K9_1} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K9_2" { slot_name {RAW_K9_2} values {RAW K9_2} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K9_3" { slot_name {RAW_K9_3} values {RAW K9_3} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K9_4" { slot_name {RAW_K9_4} values {RAW K9_4} params {} equation {$RAW} clamp {min -100000000 max 100000000}}

  "RAW K12_1" { slot_name {RAW_K12_1} values {RAW K12_1} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K12_2" { slot_name {RAW_K12_2} values {RAW K12_2} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K12_3" { slot_name {RAW_K12_3} values {RAW K12_3} params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "RAW K12_4" { slot_name {RAW_K12_4} values {RAW K12_4} params {} equation {$RAW} clamp {min -100000000 max 100000000}}

  "CUBE CO ug/m3 (alias)" { slot_name {CUBE_CO} values {V CALIBRATED_CO} params {} equation {$V} clamp {min 0 max 50000}}
  "CUBE NO ug/m3 (alias)" { slot_name {CUBE_NO} values {V CALIBRATED_NO} params {} equation {$V} clamp {min 0 max 50000}}
  "CUBE NO2 ug/m3 (alias)" { slot_name {CUBE_NO2} values {V CALIBRATED_NO2} params {} equation {$V} clamp {min 0 max 50000}}

  "CUBE H2S ug/m3 (alias)" { slot_name {CUBE_H2S} values {V CALIBRATED_H2S} params {} equation {$V} clamp {min 0 max 50000}}
  "CUBE NH3 ug/m3 (alias)" { slot_name {CUBE_NH3} values {V CALIBRATED_NH3} params {} equation {$V} clamp {min 0 max 50000}}
  "CUBE CO ppb" { slot_name {CUBE_CO_PPB} values {ug CALIBRATED_CO_PPB}  params {} equation {$ug} clamp {min -100000000 max 100000000}}
  "CUBE NO ppb" { slot_name {CUBE_NO_PPB} values {ug CALIBRATED_NO_PPB}  params {} equation {$ug} clamp {min -100000000 max 100000000}}

  "CUBE H2S ppb" { slot_name {CUBE_H2S_PPB} values {ug CALIBRATED_H2S_PPB}  params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "CUBE NH3 ppb" { slot_name {CUBE_NH3_PPB} values {ug CALIBRATED_NH3_PPB}  params {} equation {$RAW} clamp {min -100000000 max 100000000}}
  "CUBE_TGS02" { slot_name {CUBE_TGS02} values {V CALIBRATED_TGS02} params {} equation {$V} clamp {min -100000000 max 100000000}}

  "CUBE_TGS02_OHM" { slot_name {CUBE_TGS02_OHM} values {V CALIBRATED_TGS02_OHM} params {} equation {$V} clamp {min -100000000 max 100000000}}
  "scd41_valid" { slot_name {scd41_valid} values {V SCD41_VALID} params {} equation {$V} clamp {min 0 max 1}}
  "scd41_co2" { slot_name {scd41_co2} values {V SCD41_CO2} params {} equation {$V} clamp {min 0 max 1000000}}
  "scd41_temperature" { slot_name {scd41_temperature} values {V SCD41_TEMP} params {} equation {$V} clamp {min -100 max 100}}
  "scd41_humidity" { slot_name {scd41_humidity} values {V SCD41_HUMID} params {} equation {$V} clamp {min 0 max 100}}
 
  "CUBE_HCHO" { slot_name {CUBE_HCHO} values {V US0_PMS5003ST_HCHO_UG} params {} equation {$V} clamp {min 0 max 1000000}}
  "CUBE_HCHO_PPB" { slot_name {CUBE_HCHO_PPB} values {V HCHO_PPB} params {} equation {$V} clamp {min 0 max 1000000}}
  "CUBE_HCHO_VOC" { slot_name {CUBE_HCHO_VOC_PPB} values {V HCHO_VOC_PPB} params {} equation {$V} clamp {min 0 max 1000000}}
  "CUBE_TGS02_TVOC" { slot_name {CUBE_TGS02_TVOC_PPB} values {V CALIBRATED_VOC_PPB} params {} equation {$V} clamp {min 0 max 1000000}}
  "CUBE_HCHO_TVOC" { slot_name {CUBE_HCHO_TVOC_PPB} values {V HCHO_TVOC_PPB} params {} equation {$V} clamp {min 0 max 1000000}}
  "CUBE_HCHO_TEMP" { slot_name {CUBE_HCHO_TEMP} values {V HCHO_TEMP} params {} equation {$V} clamp {min -100 max 100}}
  "CUBE_HCHO_HUMID" { slot_name {CUBE_HCHO_HUMID} values {V HCHO_HUMID} params {} equation {$V} clamp {min 0 max 100}}
  "CUBE_HCHO_STATUS" { slot_name {CUBE_HCHO_STATUS} values {V HCHO_STATUS} params {} equation {$V} clamp {min 0 max 1}}

 

  "IPS_PM_01" { slot_name {IPS_PM_01} values {V IPS_PM_1} params {} equation {$V} clamp {min 0 max 1000000}}
  "IPS_PM_03" { slot_name {IPS_PM_03} values {V IPS_PM_2} params {} equation {$V} clamp {min 0 max 1000000}}
  "IPS_PM_05" { slot_name {IPS_PM_05} values {V IPS_PM_3} params {} equation {$V} clamp {min 0 max 1000000}}
  "IPS_PM_10" { slot_name {IPS_PM_10} values {V IPS_PM_4} params {} equation {$V} clamp {min 0 max 1000000}}
  "IPS_PM_25" { slot_name {IPS_PM_25} values {V IPS_PM_5} params {} equation {$V} clamp {min 0 max 1000000}}
  "IPS_PM_50" { slot_name {IPS_PM_50} values {V IPS_PM_6} params {} equation {$V} clamp {min 0 max 1000000}}
  "IPS_PM_100" { slot_name {IPS_PM_100} values {V IPS_PM_7} params {} equation {$V} clamp {min 0 max 1000000}}

 
  "battery" { slot_name {battery} values {V BATTERY_VOLTAGE} params {} equation {$V} clamp {min 0 max 20}}
  "battery_voltage" { slot_name {BATTERY_VOLTAGE} values {V BATTERY_VOLTAGE} params {} equation {$V} clamp {min 0 max 20}}
  "battery_current" { slot_name {BATTERY_CURRENT} values {I BATTERY_CURRENT} params {} equation {$I} clamp {min -10 max 10}}
  "battery_power" { slot_name {BATTERY_POWER} values {P BATTERY_POWER} params {} equation {$P} clamp {min -200 max 200}}
  "battery_charge_percent" { slot_name {BATTERY_CHARGE_PERCENT} values {P BATTERY_CHARGE_PERCENT} params {} equation {$P} clamp {min 0 max 100}}
  "cube_uptime" { slot_name {CUBE_UPTIME} values {V SYSTEM_UPTIME} params {} equation {$V} clamp {min 0 max 100000000}}
  "cube_free_heap" { slot_name {CUBE_FREE_HEAP} values {V SYSTEM_FREE_HEAP} params {} equation {$V} clamp {min 0 max 100000000}}
  "cube_wifi_signal" { slot_name {CUBE_WIFI_SIGNAL} values {V SYSTEM_WIFI_SIGNAL} params {} equation {$V} clamp {min -100 max 100}}
}


