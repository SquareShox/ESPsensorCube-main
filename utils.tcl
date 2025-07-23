# -----------------------------------------------------------------------------
proc tlog_start_file {filename} {
  set ::tlog_file [open $filename w]
}
proc tlog_stop_file {} {
  close $::tlog_file
  unset ::tlog_file
}
# funkcje logujace ------------------------------------------------------------
proc get_timestamp {} {
  if {$::timestamps} {
    set time_as_ms [clock milliseconds]
    set time_as_s [expr $time_as_ms / 1000]
    set current_ms [expr $time_as_ms % 1000]
    return "[clock format $time_as_s -format "%Y-%m-%d_%H:%M:%S"].[format "%03d" $current_ms] [tsv::get dust         counter]"
  } else {
    return "[tsv::get dust counter]"
  }
}
# standardowe logowanie pojedynczego komunikatu
proc tlog {msg} {
  set timestamp [get_timestamp]
  if {[info exists ::tlog_file]} {
    puts $::tlog_file "$timestamp $msg"
  }
  puts "[get_timestamp] $msg"
}
# logowanie hurtowe
proc tllog {header msg} {
  foreach line $msg {
    if {[string length $line] > 0} {
      tlog "$header: $line"
    }
  }
}
# logowanie slownika kolumnami - dla lepszej czytelnosci logow
proc tlog_dict {header dictionary} {
  set val_index 0
  set val_string ""
  set timestamp [get_timestamp]
  set string_dump  "$timestamp $header: +[string repeat - 58]+\n"

  foreach {k v} $dictionary {
    incr val_index
    set val_string "${val_string} [format "%17s %-10s" $k $v]"
    if {$val_index == 2} {
      set string_dump "$string_dump$timestamp $header: |$val_string|\n"
      set val_string ""
      set val_index 0
    }
  }
  if {$val_index > 0} {
    set string_dump "$string_dump$timestamp $header: |[format "%-58s" $val_string]|\n"
  }
  puts -nonewline "$string_dump"
  tlog "$header: +[string repeat - 58]+"
}
# logowanie uzaleznione od flagi DEBUG
proc dlog {msg} {
  if {$::debug} {
    foreach line [split $msg "\n"] {
      if {[string length $line] > 0} {
        tlog "DBG $line"
      }
    }
  }
}
# -----------------------------------------------------------------------------
# funkcje zwiazane z MODBUSem -------------------------------------------------
# odczyt standardowego identyfikatora
proc readIdentifier {fd addr} {
  if {[catch {
    set data [modbus::03 $fd $addr 0xa000 32]
    tlog "Device id: address [format "0x%02X" $addr]: [string trim $data]"
  } msg]} {
    tlog "Error reading device id: address [format "0x%02X" $addr]"
  }
}
proc readSensors {fd addr type} {
  set sensors [list 0xE001 0xE002 0xE003 0xE004 0xE005 0xE006 0xE007 0xE008 0xE009 0xE00A 0xE00B 0xE00C]
  set index 0
  foreach reg $sensors {
    incr index
    if {[catch {
      set id [modbus::03 $fd $addr $reg 64]
      set id [string trimright $id \xFF]
      if {[string length $id] == 0} {
        tlog "I2C sensor $index: not installed or missing EEPROM"
      } else {
        tlog "I2C sensor $index: $id"
      }
    } msg]} {
      dlog "I2C error reading sensor: $index"
    }
  }
}
# zapis do pamieci urzadzenia
proc sendDevice10Retry {fd addr beg dataHex {retry 3}} {
  set msg "error"
  set data [unhex $dataHex]
  set len [expr {[string length $data]/2 } ]
  while {($retry > 0) && ([string length $msg] > 0)} {
    dlog "MODBUS write ($retry $fd $addr $beg): [hex $data]"
    if {[catch {
      modbus::10 $fd $addr $beg $len $data
    } msg]} {
      dlog "MODBUS write error: $msg"
    }
    after 100
    incr retry -1
  }
  return $msg
}
# odczyt rejestrow urzadzenia
proc readDevice03 {fd addr beg count formatString regList} {
  set outputVal [list]
  set data [modbus::03 $fd $addr $beg $count]
  binary scan $data $formatString {*}$regList
  foreach regItem $regList {
    set outputVal [concat $outputVal [set $regItem]]
  }
  return $outputVal
}
# -----------------------------------------------------------------------------
# funkcje zwiazane z obsluga i interpretacja pomiarow -------------------------
# konwersja konfiguracji na slownik pomiarow
proc ramMapDict {text start} {
  set scanFormat ""
  set labelsList [list]
  set convParams [dict create]
  set regNo 0
  foreach param [split [string trim $text] "\n"] {
    set param [string trim $param]
    if {[string length $param] > 1} {
      lassign $param format name div
      if {$div eq ""} {set div 1}
      append scanFormat $format
      lappend labelsList $name
      dict set convParams $name $div
      incr regNo
      if {$format eq "I"} {incr regNo}
    }
  }
  set result [dict create start $start regNo $regNo scanFormat $scanFormat labelsList $labelsList convParams $convParams]
  return $result
}
# odczyt mapowanych rejestrow, tworzenie slownika zebranych wartosci
proc readDeviceSlots {fd ADDR params} {
  set START_REG [dict get $params start]
  set regNo [dict get $params regNo]
  set scanFormat [dict get $params scanFormat]
  set labelsList [dict get $params labelsList]
  set convParams [dict get $params convParams]
  set result [dict create]
  if {[catch {
    set rawValues [readDevice03 $fd $ADDR $START_REG $regNo $scanFormat $labelsList]
    set result [assignValuesToLabels $labelsList $rawValues $convParams]
    set result [oneWireCorrection $result]
    dlog "DEV read ok: ($ADDR)"
  } msg]} {
    dlog "DEV read error: ($ADDR)"
  }
  return $result
}
# przypisanie wartosci do slownikowych kluczy
proc assignValuesToLabels {labels values convParams} {
  set result [dict create]
  foreach label $labels value $values {
    if {($label ne "ex") && ([string length $label] > 0)} {
      set badValue 0
      if {([dict get $convParams $label] eq "A")} {
        set confReg [expr $value & 0x000000FF]
        dlog "$label conf: [format "%02X" $confReg]"
        if {$confReg == 0 || $confReg == 255} {
          set badValue 1
          dlog "$label error: minmax detected"
        } else {
          if {[string match *_1 $label] && [expr $confReg & 0x60] != 0x00} {
            set badValue 1
            dlog "$label error: channel 0 missmatch detected"
          }
          if {[string match *_2 $label] && [expr $confReg & 0x60] != 0x20} {
            set badValue 1
            dlog "$label error: channel 1 missmatch detected"
          }
          if {[string match *_3 $label] && [expr $confReg & 0x60] != 0x40} {
            set badValue 1
            dlog "$label error: channel 1 missmatch detected"
          }
          if {[string match *_4 $label] && [expr $confReg & 0x60] != 0x60} {
            set badValue 1
            dlog "$label error: channel 1 missmatch detected"
          }
        }
        set rawVal [expr $value >> 8]
        set gain [expr $confReg & 0x03]
        if {($gain == 0)} {
          set shiftValue [expr $rawVal * 4.0]
        } elseif {($gain == 1)} {
          set shiftValue [expr $rawVal * 2.0]
        } elseif {($gain == 2)} {
          set shiftValue [expr $rawVal * 1.0]
        } else {
          set shiftValue [expr $rawVal * 0.5]
        }
        set convVal [format "%.2f" $shiftValue]
      } elseif {([dict get $convParams $label] eq "H")} {
        set convVal [format "%04X" $value]
        set label "HEX_$label"
      } elseif {([dict get $convParams $label] eq "HH")} {
        set convVal [format "%08X" $value]
        set label "HEX_$label"
      } elseif {([dict get $convParams $label] eq "B")} {
        set convVal [format "%016b" $value]
        set label "BIN_$label"
      } else {
        set convVal [format "%.2f" [expr $value * [dict get $convParams $label]]]
      }
      if {!$badValue} {
        dict set result $label $convVal
        dlog "$label: $convVal"
      }
    }
  }
  return $result
}
# korekcja odczytow 1-wire
proc oneWireCorrection {values} {
  dict for {key value} $values {
    if {[string match ONEW_*_TEMP $key]} {
      dlog "1-WIRRE correction for $key"
      if {$value > 125.0} {
        set newTemp [expr ((~int(($value*100))+1) & 0x0FFF) * -0.01]
        dlog "1-WIRRE correction for $key > 125 ---> $newTemp"
        dict set values $key $newTemp
      }
    }
  }
  return $values
}
# polaczenie wskaznikow cisnienia
proc pressAddMili {raws {suffix ""}} {
  set press [expr [dict get $raws "I2C_PRESS_${suffix}"] + [dict get $raws "I2C_PRESS_MILI_${suffix}"]]
  return [format "%.2f" $press]
}
# -----------------------------------------------------------------------------
# funkcje zwiazane z obsluga czujnikow pylu -----------------------------------
# pms crc
proc pmsChecksum {bufferBin} {
  binary scan $bufferBin "cu*" ldata
  set byteNo  [llength $ldata]
  set sumPMS  0
  set sumCalc 0
  set sumPMS [expr [lindex $ldata end] + [lindex $ldata end-1]*256 ]
  set i 0
  foreach v $ldata {
    if {$i < ($byteNo-2)} {
      set sumCalc [expr $sumCalc + $v]
    }
    incr i
  }
  if {$sumCalc == $sumPMS} {
    dlog "PMS checksum ok: sum $sumCalc pms $sumPMS"
    return 0
  } else {
    dlog "PMS checksum error: sum $sumCalc pms $sumPMS"
    return 1
  }
}
# pmc crc
proc pmcChecksum {bufferBin} {
  binary scan $bufferBin "cu*" framedata
  set csum 0
  set sumPMC [lindex $framedata 55]
  foreach b $framedata {
    set csum [expr $csum + $b]
  }
  set csum [expr $csum - $sumPMC]
  set csum [expr (256 - $csum) & 0xFF]
  if {$csum == $sumPMC} {
    dlog "PMC checksum ok: sum $csum pmc $sumPMC"
    return 0
  } else {
    dlog "PMC checksum error: sum $csum pmc $sumPMC"
    return 1
  }
}
# sps crc
proc spsChecksum {bufferBin} {
  dlog "SPS checksum check PLACEHOLDER"
  return 0
}
# sds crc
proc sdsChecksumAndTail {bufferBin} {
  binary scan $bufferBin "c c cu*" hdr cmd ldata
  set byteNo  8
  set sumCalc 0
  set sumSDS  [lindex $ldata 6]
  set tailSDS [lindex $ldata 7]
  set TAIL    0xAB
  set i 0
  foreach v $ldata {
    if {$i < ($byteNo-2)} {
      set sumCalc [expr $sumCalc + $v]
    }
    incr i
  }
  set sumCalc [expr $sumCalc & 0xff]
  if {$sumCalc == $sumSDS && $tailSDS == $TAIL} {
    dlog "SDS checksum ok: sum $sumCalc sds $sumSDS tail [format 0x%02X $tailSDS] ($TAIL)"
    return 0
  } else {
    dlog "SDS checksum error: sum $sumCalc sds $sumSDS tail [format 0x%02X $tailSDS] ($TAIL)"
    return 1
  }
}
# hcho crc
proc hchoChecksum {bufferBin} {
  binary scan $bufferBin "cu*" framedata
  set csum 0
  set sumHCHO [lindex $framedata 15]
  foreach b $framedata {
    set csum [expr $csum + $b]
  }
  set csum [expr $csum - $sumHCHO]
  set csum [expr (256 - $csum) & 0xFF]
  if {$csum == $sumHCHO} {
    dlog "HCHO checksum ok: sum $csum hcho $sumHCHO"
    return 0
  } else {
    dlog "HCHO checksum error: sum $csum hcho $sumHCHO"
    return 1
  }
}
# mapowanie odczytanych poziomow pm10 na jakos powietrza
proc airQualityForLed {currentValues airQuality} {
  set recorded_pm10 0
  set recorded_pm25 0
  set air_quality 0
  # Iteruj po wszystkich znanych kombinacjach parametrow PM10/PM25
  # Pierwszenstwo ma pierwsza, znaleziona para
  set found 0
  set slot_pm10 PM10
  set slot_pm25 PM25
  if {[dict exists $currentValues $slot_pm10] && [dict exists $currentValues $slot_pm25]} {
    dlog "AQMS slot found $slot_pm10 $slot_pm25"
    set recorded_pm10 [dict get $currentValues $slot_pm10]
    set recorded_pm25 [dict get $currentValues $slot_pm25]
  } else {
    dlog "AQMS slot missing $slot_pm10 $slot_pm25"
    foreach {unum} {US0_ US1_ US2_} {
      foreach {stype} {PMS7003_ PMSA003_ PMS5003S_} {
        set slot_pm10 $unum${stype}PM10A
        set slot_pm25 $unum${stype}PM25A
        if {[dict exists $currentValues $slot_pm10] && [dict exists $currentValues $slot_pm25]} {
          dlog "AQMS slot found $slot_pm10 $slot_pm25"
          set recorded_pm10 [dict get $currentValues $slot_pm10]
          set recorded_pm25 [dict get $currentValues $slot_pm25]
          set found 1
          break
        } else {
          dlog "AQMS slot missing $slot_pm10 $slot_pm25"
        }
      }
      if {$found == 1} {
        break
      }
    }
  }

  foreach quality_bin [lsort -integer [dict keys $airQuality]] {
    set bin_pm10 [dict get $airQuality $quality_bin -pm10]
    set bin_pm25 [dict get $airQuality $quality_bin -pm25]
    if {($recorded_pm10 > $bin_pm10) || ($recorded_pm25 > $bin_pm25)} {
      set air_quality $quality_bin
    }
  }
  dlog "AQMS PM10 $recorded_pm10 PM25 $recorded_pm25 INDEX $air_quality"
  dlog "AQMS RAW |[dict get $airQuality $air_quality]|"
  return [dict get $airQuality $air_quality]
}
# -----------------------------------------------------------------------------
# funkcje pomocnicze ----------------------------------------------------------
# odczyt i aktualizacja licznika uruchomien uslugi
proc getCntFromFileAndIncr {} {
  set file_name "counter"
  if {[catch {
    if {[file exists $file_name]} {
      set file [open $file_name r]
      set cnt [read $file]
      close $file
      tlog "CNT file: |$file_name| present"
      tlog "CNT previous value: |$cnt|"
    } else {
      set cnt 0
      tlog "CNT file: |$file_name| missing"
      tlog "CNT value reset"
    }
    incr cnt
    set file [open $file_name w+]
    puts -nonewline $file $cnt
    close $file
    tlog "CNT value: $cnt"
  } msg]} {
    set cnt 0
    tlog "CNT error |$msg|"
    tlog "CNT value reset"
  }
  return $cnt
}
# krotkie funkcje
proc timestampFormat {ts {format "%Y-%m-%d %H:%M:%S"}} {
  return [clock format $ts -format $format]
}
proc timestampFormatGMT {ts {format "%Y-%m-%d %H:%M:%S"}} {
  return [clock format $ts -format $format -gmt 1]
}
proc hex {char} {
    binary scan $char H* out
    return $out
}
proc hex2 {char} {
  binary scan x$char xH* out
  return $out
}
proc abs {line} {
  set out ""
  set x \x80
  foreach c [split $line {}] {
    append out [expr {([string is print $c] && ($c < $x) && ($c >= " "))?$c:"\[[hex $c]\]" }]
  }
  return $out
}
# aproksymacja liniowa dla termostatowania (do wywalenia)
proc HeaterPWMLinear {tempDiff PWMmax treshold offset} {
  set PWMSet 0
  if { $tempDiff >= 0} {
    set PWMSet 0
  } elseif {$tempDiff < -$treshold} {
    set PWMSet $PWMmax
  } else {
    set PWMSet [ expr { round ( (-$tempDiff) * $PWMmax / $treshold + $PWMmax * $offset ) } ]
  }
  if { $PWMSet > $PWMmax } { set PWMSet $PWMmax }
  return $PWMSet
}
# okreslenie identyfikatora czujnika na podstawie okreslonego typu

# Konfiguracja szczegolowa (musi byc zgodna z 'sensorsNames')
# 0 - PMSA003, PMS7003, SDS
# 1 - CBHCHO
# 2 - PM2009, PM2012, PM3006T
# 3 - SPS30
# 4 - PMS5003ST
proc pmsType2Id {pms_type} {
  set id 0
  switch -nocase -exact $pms_type {
    pmsa003 -
    pms7003 -
    sds {
      set id 0
    }
    pms5003st {
      set id 4
    }
    pm2009 -
    pm2012 -
    pm3006t {
      set id 2
    }
    sps30 {
      set id 3
    }
    cbhcho {
      set id 1
    }
    default {
      set id 99
    }
  }
  tlog "PMS id for $pms_type is $id"
  return $id
}

proc i2cType2Config {i2c_type gain} {
  set i2c_cfg "00000000"
  switch -nocase -exact $i2c_type {
    adc2 {
      set i2c_cfg "D020$gain"
    }
    adc4 {
      set i2c_cfg "D028$gain"
    }
    bme280 {
      set i2c_cfg "00100000"
    }
    bme280_pcb {
      set i2c_cfg "77100000"
    }
    lux7700 {
      set i2c_cfg "00300000"
    }
    scd {
      set i2c_cfg "${gain}0000"
    }
    default {
      set i2c_cfg "00000000"
    }
  }
  return $i2c_cfg
}
# -----------------------------------------------------------------------------

