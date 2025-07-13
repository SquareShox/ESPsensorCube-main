# Procedury kompensacji czujnikow elektrochemicznych (oddzielne wersje A4 i B4)
proc A4_O3 {T} {
  set TS [expr (int($T/10) * 10)]
  set TE [expr $TS + 10]
  set TCS [expr {$TS <= 40} ? 0.18 : 2.87]
  set TCE [expr {$TE <= 40} ? 0.18 : 2.87]
  return [expr $TCS + (($TCE - $TCS) * (($T - $TS) / 10.0))]
}
proc B4_O3 {T} {
  set TS [expr (int($T/10) * 10)]
  set TE [expr $TS + 10]
  set TCS [expr {$TS <= 10} ? -0.90 : {$TS <= 20} ? -0.35 : {$TS <= 30} ? 0.50 : {$TS <= 40} ? 1.15 : 1.80]
  set TCE [expr {$TE <= 10} ? -0.90 : {$TE <= 20} ? -0.35 : {$TE <= 30} ? 0.50 : {$TE <= 40} ? 1.15 : 1.80]
  return [expr $TCS + (($TCE - $TCS) * (($T - $TS) / 10.0))]
}
proc A4_SO2 {T} {
  set TS [expr (int($T/10) * 10)]
  set TE [expr $TS + 10]
  set TCS [expr {$TS <= 10} ? 0.85 : {$TS <= 20} ? 1.15 : {$TS <= 30} ? 1.45 : {$TS <= 40} ? 1.75 : 1.95]
  set TCE [expr {$TE <= 10} ? 0.85 : {$TE <= 20} ? 1.15 : {$TE <= 30} ? 1.45 : {$TE <= 40} ? 1.75 : 1.95]
  return [expr $TCS + (($TCE - $TCS) * (($T - $TS) / 10.0))]
}
proc B4_SO2 {T} {
  return 1.0
}
proc A4_NO2 {T} {
  set TS [expr (int($T/10) * 10)]
  set TE [expr $TS + 10]
  set TCS [expr {$TS <= 30} ? 1.18 : {$TS <= 40} ? 2.00 : 2.70]
  set TCE [expr {$TE <= 30} ? 1.18 : {$TE <= 40} ? 2.00 : 2.70]
  return [expr $TCS + (($TCE - $TCS) * (($T - $TS) / 10.0))]
}
proc B4_NO2 {T} {
  return 0.70
}
proc A4_NO {T} {
  set TS [expr (int($T/10) * 10)]
  set TE [expr $TS + 10]
  set TCS [expr {$TS <= 0} ? 1.13 : 1.37]
  set TCE [expr {$TE <= 0} ? 1.13 : 1.37]
  return [expr $TCS + (($TCE - $TCS) * (($T - $TS) / 10.0))]
}
proc B4_NO {T} {
  set TS [expr (int($T/10) * 10)]
  set TE [expr $TS + 10]
  set TCS [expr {$TS <= -30} ? 0.75 : {$TS <= -20} ? 1.05 : {$TS <= -10} ? 1.35 : {$TS <= 0} ? 1.65 : {$TS <= 10} ? 1.95 : {$TS <= 20} ? 1.80 : {$TS <= 30} ? 1.65 : {$TS <= 40} ? 1.50 : 1.35]
  set TCE [expr {$TE <= -30} ? 0.75 : {$TE <= -20} ? 1.05 : {$TE <= -10} ? 1.35 : {$TE <= 0} ? 1.65 : {$TE <= 10} ? 1.95 : {$TE <= 20} ? 1.80 : {$TE <= 30} ? 1.65 : {$TE <= 40} ? 1.50 : 1.35]
  return [expr $TCS + (($TCE - $TCS) * (($T - $TS) / 10.0))]
}
proc A4_CO {T} {
  set TS [expr (int($T/10) * 10)]
  set TE [expr $TS + 10]
  set TCS [expr {$TS <= -30} ? 1.40 : {$TS <= -20} ? 1.03 : {$TS <= -10} ? 0.85 : {$TS <= 0} ? 0.62 : {$TS <= 10} ? 0.30 : {$TS <= 20} ? 0.03 : {$TS <= 30} ? -0.25 : {$TS <= 40} ? -0.48 : -0.80]
  set TCE [expr {$TE <= -30} ? 1.40 : {$TE <= -20} ? 1.03 : {$TE <= -10} ? 0.85 : {$TE <= 0} ? 0.62 : {$TE <= 10} ? 0.30 : {$TE <= 20} ? 0.03 : {$TE <= 30} ? -0.25 : {$TE <= 40} ? -0.48 : -0.80]
  return [expr $TCS + (($TCE - $TCS) * (($T - $TS) / 10.0))]
}
proc B4_CO {T} {
  set TS [expr (int($T/10) * 10)]
  set TE [expr $TS + 10]
  set TCS [expr {$TS <= 0} ? 1.20 : {$TS <= 10} ? 1.05 : {$TS <= 20} ? -0.32 : {$TS <= 30} ? -1.07 : {$TS <= 40} ? -3.00 : -2.40]
  set TCE [expr {$TE <= 0} ? 1.20 : {$TE <= 10} ? 1.05 : {$TE <= 20} ? -0.32 : {$TE <= 30} ? -1.07 : {$TE <= 40} ? -3.00 : -2.40]
  return [expr $TCS + (($TCE - $TCS) * (($T - $TS) / 10.0))]
}
proc A4_H2S {T} {
 return 0.65
}
proc B4_H2S {T} {
  set TS [expr (int($T/10) * 10)]
  set TE [expr $TS + 10]
  set TCS [expr {$TS <= 0} ? 1.85 : -1.80]
  set TCE [expr {$TE <= 0} ? 1.85 : -1.80]
  return [expr $TCS + (($TCE - $TCS) * (($T - $TS) / 10.0))]
}
# Procedury obliczania temperatury i napiecia w czujnikach B4
proc B4_T {T V} {
  set To 25.0
  set B 3380.0
  set Ro 10
  set Rs 33
  set k 3.2
  set CoK 273.15
  return [expr (1/( 1/($To + $CoK) + 1/$B * log($Rs/($k*$V/$T-1)/$Ro)) - $CoK)]
}
proc B4_mV {V} {
  set lsb 3.90625
  set k 3.2
  return [expr $k *$lsb * $V / 1000.0]
}
# Procedury obliczania temperatury i napiecia w czujnikach TGS
proc TGS_T {C1 C2} {
  set To 25.0
  set B 3380.0
  set Ro 10
  set Rs 33
  set k 3.2
  set CoK 273.15
  return [expr (1/( 1/($To + $CoK) + 1/$B * log($C1/$C2)) - $CoK)]
}
proc TGS_mV {C4} {
  set lsb 3.90625
  set k 4.2
  return [expr $k *$lsb * $C4 / 1500.0]
}
# Procedury obliczania temperatury PID
proc PID_T {C2} {
  set To 25.0
  set B 3380.0
  set Ro 10
  set Rs 10
  set Vcc 844800
  set CoK 273.15
  return [expr (1/(1/($To + $CoK) + 1/$B * log($Rs/($Vcc/$C2-1)/$Ro)) - $CoK)]
}
# Procedura obliczania napięcia PID
proc PID_mV {C4} {
  set lsb 3.90625
  set k 2.0
  return [expr $k *$lsb * $C4 / 1000.0]
}
# Procedury do korekcji liniowej
proc Linear_basic {mul RAW add} {
  return [expr {$mul * $RAW + $add}]
}
proc Linear_advanced {mul RAW add} {
  set a2_abs [expr {2 * abs($add)}]
  set fx_nom [Linear_basic $mul $RAW $add]
  set fx_alt [expr {$add == 0} ? ($mul * $RAW) : ($mul * $RAW)/(1 - ($add/$a2_abs))]
  return [expr {$fx_nom >= $a2_abs} ? $fx_nom : $fx_alt]
}
# Procedura obliczania odpowiedzi TGS w ppm
proc TGS_ppm {C3 C4 RL A B C b1 a} {
  set R [expr {($RL * $C4 / $C3) - 100.0}]
  set Sr [expr {$A/($R + $C) + $B}]
  return [expr {$b1 * $Sr + $a}]
}

#----------------------------------------------
# Procedury do obsługi nowych plytek TGS i PID
#----------------------------------------------
# Procedura obliczania odpowiedzi TGS w ppm dla v4
proc TGSv4_ppm {C3 C4 RL A B C b1 a} {
  set R [expr {$RL * ($C4 - $C3) / ($C4 + $C3)}]
  set Sr [expr {$A/($R + $C) + $B}]
  return [expr {$b1 * $Sr + $a}]
}
# Procedura obliczania rezystancji  TGS w OHM v4
proc TGSv4_ohm {C3 C4 RL} {
  set R [expr {$RL * ($C4 - $C3) / ($C4 + $C3)}]
  return [expr {$R}]
}
# Procedura obliczania rezystancji RL w TGS w OHM v4 do celów kalibracji RL
proc TGSv4_RL_ohm {C3 C4 RX} {
  set RL [expr {$RX * ($C4 + $C3) / ($C4 - $C3)}]
  return [expr {$RL}]
}
# Procedura obliczania nap. dla TGS v4
proc TGSv4_mV {C4} {
  set lsb 3.90625
  set k 5.0
  return [expr $k *$lsb * $C4 / 1000.0]
}

# Procedura obliczania nap. dla TGS v4
proc TGSv4_mV_FIX {C3 C4} {
  set lsb 3.90625
  set k 5.0
  return [expr $k *$lsb * ($C3 + $C4) / 1000.0]
}


# Procedura obliczania offsetu PID_PH
proc PID_PHmV {C4} {
  set lsb 3.90625
  set k 1.0
  return [expr $k *$lsb * $C4 / 1000.0]
}
# Procedury obliczania temperatury PID_PH
proc PID_PH_T {C1 C2} {
  set To 25.0
  set B 3380.0
  set CoK 273.15
  return [expr (1/( 1/($To + $CoK) + 1/$B * log($C1/$C2)) - $CoK)]
}
proc TGSv4_ppm_FIX {C3 C4 RL A B C b1 a} {
  set R [expr {$RL * (2*$C3 + $C4) / ($C4)}]
  set Sr [expr {$A/($R + $C) + $B}]
  return [expr {$b1 * $Sr + $a}]
}
# Procedura obliczania rezystancji  TGS w OHM v4
proc TGSv4_ohm_FIX {C3 C4 RL} {
  set R [expr {$RL * (2*$C3 + $C4) / ($C4)}]
  return [expr {$R}]
}

# -----------------------------------------------------------------------------
# Procedura obslugi przeliczen

proc map_calib_dict {source_dict calibration_dict} {
  set results_dict [dict create]
  # iteruj po wpisach slownika mapowania
  foreach {id record} $calibration_dict {
    dlog "CLB: ----------------------------------------"
    if {[ catch {
      # odczyt pojedynczego rekordu
      set name [dict get $record "slot_name"]
      set source_values [dict get $record "values"]
      set calibration_parameters [dict get $record "params"]
      set clamp_parameters [dict get $record "clamp"]
      # odczyt ograniczen min/max
      set eqMin [dict get $clamp_parameters "min"]
      set eqMax [dict get $clamp_parameters "max"]
      # odczyt funkcji przeksztalcajacej
      set transformation [dict get $record "equation"]
      dlog "CLB: $id - \[$name\] <$eqMin:$eqMax>"
      # mapowanie parametrow przeliczen na zmienne i dane ze slownika
      foreach {var_name var_value} $source_values {
        if {[catch {
          set slot_value [dict get $source_dict $var_value]
          dlog "CLB:  $var_name - $var_value -> $slot_value"
          set $var_name $slot_value
        } errmsg ]} {
          set slot_value [dict get $results_dict $var_value]
          dlog "CLB:  $var_name - $var_value -> $slot_value"
          set $var_name $slot_value
        }
      }
      foreach {var_name var_value} $calibration_parameters {
        set tmp_value [format "%.10f" [expr $var_value]]
        if {$tmp_value != $var_value} {
          dlog "CLB:  $var_name - $var_value -> $tmp_value"
        } else {
          dlog "CLB:  $var_name - $var_value"
        }
        set $var_name $tmp_value
      }
    } errmsg ]} {
      #dlog $errmsg
      dlog "CLB: parsing ERROR - $id"
      dlog "CLB: ----------------------------------------"
      continue
    }
    if {[catch {
      # obliczenie
      set raw_result [format "%.2f" [expr $transformation]]
      # obciecie wyniku
      set result [format "%.2f" [expr max(min($raw_result, $eqMax), $eqMin)]]
      # aktualizacja slownika rezultatow
      foreach {sn} $name {
        dict set results_dict $sn $result
      }
      # wypisanie danych
      dlog "CLB: equation: $transformation"
      dlog "CLB: equation: $transformation"
      dlog "CLB: equation: [subst $transformation]"
      if {$raw_result != $result} {
        dlog "CLB: result: $raw_result -> $result"
      } else {
        dlog "CLB: result: $result"
      }
      dlog "CLB: ----------------------------------------"
      dlog "[format "CLB: %-30s --> %-15s = %.2f" $id $name $result]"
    } errmsg ]} {
      dlog "CLB: math ERROR - $id"
      dlog "CLB: ----------------------------------------"
    }
  }
  return $results_dict
}


proc pm_fix {dict_name pm1_name pm25_name pm10_name} {
    # Check if PM25 exists in the dictionary
    upvar 1 $dict_name local_dict
    if {[dict exists $local_dict $pm25_name]} {
        set pm10_offset 0.270
        set pm1_offset  0.273
        set pm25_calib [dict get $local_dict $pm25_name]

        # If PM25 exists, check for PM10
        if {[dict exists $local_dict $pm10_name]} {
            set pm10_calib [dict get $local_dict $pm10_name]
            set pm10_fix $pm10_calib

            if {$pm10_calib <= $pm25_calib + $pm10_offset} {
                dlog "PM10 fix"
                set pm10_fix [format "%.2f" [expr $pm25_calib + $pm10_offset * (1 + 1 * (rand() - 0.5))]]
            }
            dict set local_dict "${pm10_name}_calib" $pm10_calib
            dict set local_dict $pm10_name $pm10_fix
        } else {
            dlog "NO PM10"
        }

        # If PM25 exists, check for PM1
        if {[dict exists $local_dict $pm1_name]} {
            set pm1_calib [dict get $local_dict $pm1_name]
            set pm1_fix $pm1_calib

            if {$pm1_calib >= $pm25_calib - $pm1_offset} {
                dlog "PM1 fix"
                set pm1_fix [format "%.2f" [expr max(($pm25_calib - $pm1_offset * (1 + 1 * (rand() - 0.5))),0)]]
            }
            dict set local_dict "${pm1_name}_calib" $pm1_calib
            dict set local_dict $pm1_name $pm1_fix
        } else {
            dlog "NO PM1"
        }
    } else {
        dlog "NO PM25"
    }
}

       
