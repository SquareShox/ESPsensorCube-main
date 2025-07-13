#!/usr/bin/env python3
"""
ESP32 Modbus RTU Tester
Testuje komunikację Modbus z ESP32 przez port serial
"""

import serial
import struct
import time
import threading
import argparse
import json
from datetime import datetime
from dataclasses import dataclass
from typing import Dict, List, Optional
import statistics

@dataclass
class ModbusResponse:
    """Data class for Modbus response"""
    timestamp: float
    address: int
    register_count: int
    data: List[int]
    response_time: float
    success: bool
    error_msg: str = ""

class ModbusTester:
    """ESP32 Modbus RTU Communication Tester"""
    
    def __init__(self, port: str, baudrate: int = 38400, slave_id: int = 30):
        self.port = port
        self.baudrate = baudrate
        self.slave_id = slave_id
        self.serial_conn = None
        self.running = False
        self.responses: List[ModbusResponse] = []
        self.error_count = 0
        self.success_count = 0
        
        # Modbus register definitions from ESP32
        self.register_map = {
            "solar_data": {"start": 0, "count": 40, "name": "Solar Sensor Data"},
            "opcn3_data": {"start": 40, "count": 50, "name": "OPCN3 Particle Sensor"},
            "i2c_data": {"start": 90, "count": 6, "name": "I2C Sensors"},
            "ips_data": {"start": 96, "count": 25, "name": "IPS Particle Sensor"},
            "bin_counts": {"start": 46, "count": 24, "name": "OPCN3 Bin Counts"},
            "commands": {"start": 91, "count": 2, "name": "Command Registers"}
        }
    
    def calculate_crc16(self, data: bytes) -> int:
        """Calculate Modbus CRC16"""
        crc = 0xFFFF
        for byte in data:
            crc ^= byte
            for _ in range(8):
                if crc & 0x0001:
                    crc = ((crc >> 1) ^ 0xA001) & 0xFFFF
                else:
                    crc = (crc >> 1) & 0xFFFF
        return crc
    
    def create_modbus_request(self, function_code: int, start_address: int, register_count: int) -> bytes:
        """Create Modbus RTU request frame"""
        # Slave ID + Function Code + Start Address + Register Count
        frame = struct.pack('>BBHH', self.slave_id, function_code, start_address, register_count)
        
        # Calculate and append CRC
        crc = self.calculate_crc16(frame)
        frame += struct.pack('<H', crc)
        
        return frame
    
    def parse_modbus_response(self, response: bytes) -> Optional[List[int]]:
        """Parse Modbus RTU response frame"""
        if len(response) < 5:
            return None
        
        # Extract slave ID, function code, byte count
        slave_id = response[0]
        function_code = response[1]
        byte_count = response[2]
        
        if slave_id != self.slave_id:
            raise ValueError(f"Wrong slave ID: expected {self.slave_id}, got {slave_id}")
        
        if function_code & 0x80:  # Error response
            error_code = response[2]
            raise ValueError(f"Modbus error code: {error_code}")
        
        # Extract data bytes
        data_end = 3 + byte_count
        if len(response) < data_end + 2:
            raise ValueError("Response too short")
        
        data_bytes = response[3:data_end]
        
        # Verify CRC
        expected_crc = struct.unpack('<H', response[data_end:data_end+2])[0]
        calculated_crc = self.calculate_crc16(response[:data_end])
        
        if expected_crc != calculated_crc:
            raise ValueError(f"CRC mismatch: expected {expected_crc:04X}, got {calculated_crc:04X}")
        
        # Convert bytes to 16-bit registers
        registers = []
        for i in range(0, len(data_bytes), 2):
            register_value = struct.unpack('>H', data_bytes[i:i+2])[0]
            registers.append(register_value)
        
        return registers
    
    def connect(self) -> bool:
        """Connect to serial port"""
        try:
            self.serial_conn = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=2.0
            )
            print(f"✅ Połączono z {self.port} @ {self.baudrate} baud")
            return True
        except Exception as e:
            print(f"❌ Błąd połączenia: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from serial port"""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            print("🔌 Rozłączono")
    
    def read_registers(self, start_address: int, register_count: int) -> ModbusResponse:
        """Read holding registers from ESP32"""
        start_time = time.time()
        
        try:
            # Create and send request
            request = self.create_modbus_request(0x03, start_address, register_count)  # Function 03: Read Holding Registers
            
            self.serial_conn.write(request)
            
            # Read response with timeout
            response_data = b""
            timeout_start = time.time()
            
            while len(response_data) < 5 and (time.time() - timeout_start) < 2.0:
                if self.serial_conn.in_waiting > 0:
                    response_data += self.serial_conn.read(self.serial_conn.in_waiting)
                time.sleep(0.001)
            
            # Calculate expected response length
            if len(response_data) >= 3:
                byte_count = response_data[2]
                expected_length = 3 + byte_count + 2  # Header + Data + CRC
                
                while len(response_data) < expected_length and (time.time() - timeout_start) < 2.0:
                    if self.serial_conn.in_waiting > 0:
                        response_data += self.serial_conn.read(self.serial_conn.in_waiting)
                    time.sleep(0.001)
            
            response_time = time.time() - start_time
            
            # Parse response
            registers = self.parse_modbus_response(response_data)
            
            if registers is not None:
                self.success_count += 1
                return ModbusResponse(
                    timestamp=time.time(),
                    address=start_address,
                    register_count=register_count,
                    data=registers,
                    response_time=response_time,
                    success=True
                )
            else:
                raise ValueError("Failed to parse response")
                
        except Exception as e:
            self.error_count += 1
            return ModbusResponse(
                timestamp=time.time(),
                address=start_address,
                register_count=register_count,
                data=[],
                response_time=time.time() - start_time,
                success=False,
                error_msg=str(e)
            )
    
    def test_single_read(self, register_group: str) -> ModbusResponse:
        """Test single register group read"""
        if register_group not in self.register_map:
            raise ValueError(f"Unknown register group: {register_group}")
        
        reg_info = self.register_map[register_group]
        print(f"📊 Odczytuję {reg_info['name']} (rejestry {reg_info['start']}-{reg_info['start']+reg_info['count']-1})")
        
        response = self.read_registers(reg_info['start'], reg_info['count'])
        
        if response.success:
            print(f"✅ Sukces! Czas odpowiedzi: {response.response_time*1000:.1f}ms")
            print(f"📦 Dane: {len(response.data)} rejestrów")
            
            # Display specific data based on register group
            self.display_register_data(register_group, response.data)
        else:
            print(f"❌ Błąd: {response.error_msg}")
            print(f"⏱️ Czas: {response.response_time*1000:.1f}ms")
        
        self.responses.append(response)
        return response
    
    def display_register_data(self, register_group: str, data: List[int]):
        """Display formatted register data"""
        if register_group == "solar_data" and len(data) >= 4:
            print(f"   Status: {'OK' if data[0] == 1 else 'ERROR'}")
            print(f"   Device ID: {data[1]}")
            print(f"   Timestamp: {data[3]}")
            
        elif register_group == "opcn3_data" and len(data) >= 40:
            print(f"   Status: {'OK' if data[0] == 1 else 'ERROR'}")
            print(f"   Temperatura: {data[4]/100:.1f}°C")
            print(f"   Wilgotność: {data[5]/100:.1f}%")
            if len(data) >= 40:
                pm_values = data[37:40]  # PM1, PM2.5, PM10
                print(f"   PM1: {pm_values[0]/100:.1f} μg/m³")
                print(f"   PM2.5: {pm_values[1]/100:.1f} μg/m³")  
                print(f"   PM10: {pm_values[2]/100:.1f} μg/m³")
                
        elif register_group == "bin_counts" and len(data) >= 24:
            print(f"   Bin Counts (pierwsze 8 binów):")
            for i in range(min(8, len(data))):
                print(f"     Bin {i}: {data[i]} cząstek")
            total_particles = sum(data)
            print(f"   Suma wszystkich cząstek: {total_particles}")
            
        elif register_group == "i2c_data" and len(data) >= 6:
            print(f"   Status: {'OK' if data[0] == 1 else 'ERROR'}")
            print(f"   Temperatura: {data[1]/100:.1f}°C")
            print(f"   Wilgotność: {data[2]/100:.1f}%")
            sensor_types = {1: "SHT30", 2: "BME280", 3: "SCD40"}
            sensor_type = sensor_types.get(data[5], f"Unknown({data[5]})")
            print(f"   Typ czujnika: {sensor_type}")
            
        elif register_group == "ips_data" and len(data) >= 20:
            print(f"   Status: {'OK' if data[0] == 1 else 'ERROR'}")
            print(f"   Device ID: {data[1]}")
            print(f"   Timestamp: {data[3]}")
            
            # PC Values (7 values, each as 2 16-bit registers)
            print(f"   PC Values (liczba cząstek):")
            pc_ranges = ["0.1μm", "0.3μm", "0.5μm", "1.0μm", "2.5μm", "5.0μm", "10μm"]
            for i in range(7):
                if 4 + i*2 + 1 < len(data):
                    # Combine two 16-bit registers into 32-bit value
                    pc_value = (data[4 + i*2 + 1] << 16) | data[4 + i*2]
                    print(f"     {pc_ranges[i]}: {pc_value}")
            
            # PM Values (7 values, scaled by 1000)
            print(f"   PM Values (masa cząstek):")
            pm_start = 18 if len(data) > 18 else len(data)
            for i in range(min(7, len(data) - pm_start)):
                if pm_start + i < len(data):
                    pm_value = data[pm_start + i] / 1000.0
                    print(f"     PM{pc_ranges[i]}: {pm_value:.3f} μg/m³")
    
    def continuous_test(self, interval: float = 1.0, duration: float = 60.0):
        """Run continuous testing"""
        print(f"🔄 Start testu ciągłego: {duration}s, interwał: {interval}s")
        
        start_time = time.time()
        test_count = 0
        
        try:
            while time.time() - start_time < duration:
                test_count += 1
                print(f"\n--- Test #{test_count} ---")
                
                # Test all register groups
                for group_name in ["solar_data", "opcn3_data", "i2c_data", "ips_data"]:
                    self.test_single_read(group_name)
                    time.sleep(0.1)  # Small delay between reads
                
                # Wait for next cycle
                time.sleep(max(0, interval - 0.3))  # Adjust for processing time
                
        except KeyboardInterrupt:
            print("\n⏹️ Test przerwany przez użytkownika")
        
        self.print_statistics()
    
    def print_statistics(self):
        """Print test statistics"""
        if not self.responses:
            print("📊 Brak danych do analizy")
            return
        
        successful_responses = [r for r in self.responses if r.success]
        failed_responses = [r for r in self.responses if not r.success]
        
        if successful_responses:
            response_times = [r.response_time * 1000 for r in successful_responses]  # Convert to ms
            
            print("\n" + "="*50)
            print("📈 STATYSTYKI TESTÓW")
            print("="*50)
            print(f"Całkowita liczba testów: {len(self.responses)}")
            print(f"Udane: {len(successful_responses)} ({len(successful_responses)/len(self.responses)*100:.1f}%)")
            print(f"Nieudane: {len(failed_responses)} ({len(failed_responses)/len(self.responses)*100:.1f}%)")
            print(f"\nCzasy odpowiedzi (ms):")
            print(f"  Średni: {statistics.mean(response_times):.1f}")
            print(f"  Mediana: {statistics.median(response_times):.1f}")
            print(f"  Min: {min(response_times):.1f}")
            print(f"  Max: {max(response_times):.1f}")
            if len(response_times) > 1:
                print(f"  Odchylenie std: {statistics.stdev(response_times):.1f}")
        
        if failed_responses:
            print(f"\n❌ Błędy:")
            error_types = {}
            for response in failed_responses:
                error_msg = response.error_msg
                error_types[error_msg] = error_types.get(error_msg, 0) + 1
            
            for error, count in error_types.items():
                print(f"  {error}: {count}x")
    
    def save_results(self, filename: str):
        """Save test results to JSON file"""
        data = {
            "test_info": {
                "port": self.port,
                "baudrate": self.baudrate,
                "slave_id": self.slave_id,
                "timestamp": datetime.now().isoformat(),
                "total_tests": len(self.responses),
                "success_count": self.success_count,
                "error_count": self.error_count
            },
            "responses": [
                {
                    "timestamp": r.timestamp,
                    "address": r.address,
                    "register_count": r.register_count,
                    "data": r.data,
                    "response_time": r.response_time,
                    "success": r.success,
                    "error_msg": r.error_msg
                }
                for r in self.responses
            ]
        }
        
        with open(filename, 'w') as f:
            json.dump(data, f, indent=2)
        
        print(f"💾 Wyniki zapisane do: {filename}")

def main():
    parser = argparse.ArgumentParser(description="ESP32 Modbus RTU Tester")
    parser.add_argument("port", help="Port serial (np. COM3 lub /dev/ttyUSB0)")
    parser.add_argument("--baudrate", type=int, default=38400, help="Prędkość transmisji (domyślnie: 19200)")
    parser.add_argument("--slave-id", type=int, default=30, help="Modbus Slave ID (domyślnie: 30)")
    parser.add_argument("--test", choices=["single", "continuous"], default="single", help="Typ testu")
    parser.add_argument("--group", choices=["solar_data", "opcn3_data", "i2c_data", "ips_data", "bin_counts", "all"], 
                       default="all", help="Grupa rejestrów do testu")
    parser.add_argument("--duration", type=float, default=60.0, help="Czas trwania testu ciągłego (s)")
    parser.add_argument("--interval", type=float, default=1.0, help="Interwał w teście ciągłym (s)")
    parser.add_argument("--save", help="Zapisz wyniki do pliku JSON")
    
    args = parser.parse_args()
    
    # Create tester instance
    tester = ModbusTester(args.port, args.baudrate, args.slave_id)
    
    try:
        # Connect to serial port
        if not tester.connect():
            return 1
        
        print(f"🚀 ESP32 Modbus Tester - Start!")
        print(f"Port: {args.port}, Baudrate: {args.baudrate}, Slave ID: {args.slave_id}")
        
        if args.test == "single":
            if args.group == "all":
                for group in ["solar_data", "opcn3_data", "i2c_data", "ips_data", "bin_counts"]:
                    print(f"\n--- Testing {group} ---")
                    tester.test_single_read(group)
                    time.sleep(0.5)
            else:
                tester.test_single_read(args.group)
        
        elif args.test == "continuous":
            tester.continuous_test(args.interval, args.duration)
        
        # Save results if requested
        if args.save:
            tester.save_results(args.save)
        
        tester.print_statistics()
        
    except KeyboardInterrupt:
        print("\n⏹️ Program przerwany")
    except Exception as e:
        print(f"❌ Błąd: {e}")
        return 1
    finally:
        tester.disconnect()
    
    return 0

if __name__ == "__main__":
    exit(main()) 