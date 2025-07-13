#!/usr/bin/env python3
"""
Real-time ESP32 Modbus Monitor
Prosty monitor do śledzenia danych z czujników w czasie rzeczywistym
"""

import serial
import struct
import time
import sys
from datetime import datetime

class SimpleModbusMonitor:
    """Simple real-time Modbus monitor"""
    
    def __init__(self, port: str, baudrate: int = 19200, slave_id: int = 30):
        self.port = port
        self.baudrate = baudrate
        self.slave_id = slave_id
        self.serial_conn = None
    
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
    
    def create_request(self, start_address: int, register_count: int) -> bytes:
        """Create Modbus RTU read request"""
        frame = struct.pack('>BBHH', self.slave_id, 0x03, start_address, register_count)
        crc = self.calculate_crc16(frame)
        frame += struct.pack('<H', crc)
        return frame
    
    def read_response(self) -> bytes:
        """Read Modbus response from serial"""
        response = b""
        timeout_start = time.time()
        
        # Wait for initial bytes
        while len(response) < 5 and (time.time() - timeout_start) < 1.0:
            if self.serial_conn.in_waiting > 0:
                response += self.serial_conn.read(self.serial_conn.in_waiting)
            time.sleep(0.001)
        
        if len(response) >= 3:
            byte_count = response[2]
            expected_length = 3 + byte_count + 2
            
            while len(response) < expected_length and (time.time() - timeout_start) < 1.0:
                if self.serial_conn.in_waiting > 0:
                    response += self.serial_conn.read(self.serial_conn.in_waiting)
                time.sleep(0.001)
        
        return response
    
    def parse_registers(self, response: bytes) -> list:
        """Parse response to registers"""
        if len(response) < 5:
            return []
        
        byte_count = response[2]
        data_bytes = response[3:3+byte_count]
        
        registers = []
        for i in range(0, len(data_bytes), 2):
            if i + 1 < len(data_bytes):
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
                timeout=1.0
            )
            return True
        except Exception as e:
            print(f"❌ Błąd połączenia: {e}")
            return False
    
    def read_registers(self, start: int, count: int) -> list:
        """Read registers with error handling"""
        try:
            request = self.create_request(start, count)
            self.serial_conn.write(request)
            response = self.read_response()
            return self.parse_registers(response)
        except Exception:
            return []
    
    def display_data(self, solar_data, opcn3_data, i2c_data, bin_counts):
        """Display formatted sensor data"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        
        # Clear screen and move cursor to top
        print("\033[2J\033[H")
        
        print("="*80)
        print(f"🔬 ESP32 Multi-Sensor Monitor - {timestamp}")
        print("="*80)
        
        # Solar Data
        print("\n🌞 SOLAR SENSOR")
        if len(solar_data) >= 4:
            status = "🟢 OK" if solar_data[0] == 1 else "🔴 ERROR"
            print(f"Status: {status} | Device ID: {solar_data[1]} | Timestamp: {solar_data[3]}")
            if len(solar_data) >= 10:
                v = solar_data[7] if len(solar_data) > 7 else 0
                i = solar_data[8] if len(solar_data) > 8 else 0
                vpv = solar_data[9] if len(solar_data) > 9 else 0
                ppv = solar_data[10] if len(solar_data) > 10 else 0
                print(f"Voltage: {v}mV | Current: {i}mA | PV Voltage: {vpv}mV | PV Power: {ppv}W")
        else:
            print("🔴 No data")
        
        # OPCN3 Data
        print("\n🌫️  OPCN3 PARTICLE SENSOR")
        if len(opcn3_data) >= 40:
            status = "🟢 OK" if opcn3_data[0] == 1 else "🔴 ERROR"
            temp = opcn3_data[4] / 100.0 if len(opcn3_data) > 4 else 0
            humidity = opcn3_data[5] / 100.0 if len(opcn3_data) > 5 else 0
            print(f"Status: {status} | Temp: {temp:.1f}°C | Humidity: {humidity:.1f}%")
            
            if len(opcn3_data) >= 40:
                pm1 = opcn3_data[37] / 100.0
                pm25 = opcn3_data[38] / 100.0
                pm10 = opcn3_data[39] / 100.0
                print(f"PM1: {pm1:.1f} μg/m³ | PM2.5: {pm25:.1f} μg/m³ | PM10: {pm10:.1f} μg/m³")
        else:
            print("🔴 No data")
        
        # Bin Counts
        print("\n📊 PARTICLE SIZE DISTRIBUTION")
        if len(bin_counts) >= 24:
            total_particles = sum(bin_counts)
            print(f"Total particles: {total_particles}")
            
            # Display first 12 bins in two rows
            print("Bins 0-11: ", end="")
            for i in range(12):
                print(f"{bin_counts[i]:4d}", end=" ")
            print()
            
            print("Bins 12-23:", end="")
            for i in range(12, 24):
                print(f"{bin_counts[i]:4d}", end=" ")
            print()
            
            # Show largest bins
            max_bin = max(bin_counts)
            max_bin_index = bin_counts.index(max_bin)
            print(f"Largest bin: #{max_bin_index} with {max_bin} particles")
        else:
            print("🔴 No bin data")
        
        # I2C Data
        print("\n🌡️  I2C SENSORS")
        if len(i2c_data) >= 6:
            status = "🟢 OK" if i2c_data[0] == 1 else "🔴 ERROR"
            temp = i2c_data[1] / 100.0 if len(i2c_data) > 1 else 0
            humidity = i2c_data[2] / 100.0 if len(i2c_data) > 2 else 0
            sensor_types = {1: "SHT30", 2: "BME280", 3: "SCD40", 0: "None"}
            sensor_type = sensor_types.get(i2c_data[5] if len(i2c_data) > 5 else 0, "Unknown")
            print(f"Status: {status} | Sensor: {sensor_type}")
            print(f"Temperature: {temp:.1f}°C | Humidity: {humidity:.1f}%")
            
            if len(i2c_data) > 3 and i2c_data[3] > 0:
                pressure = i2c_data[3] / 10.0
                print(f"Pressure: {pressure:.1f} hPa")
            
            if len(i2c_data) > 4 and i2c_data[4] > 0:
                co2 = i2c_data[4]
                print(f"CO2: {co2} ppm")
        else:
            print("🔴 No data")
        
        print("\n" + "="*80)
        print("📡 Naciśnij Ctrl+C aby zakończyć monitoring")
    
    def monitor(self, interval: float = 2.0):
        """Start real-time monitoring"""
        if not self.connect():
            return
        
        print(f"🚀 Uruchamianie monitora na porcie {self.port}")
        print("⏳ Ładowanie danych...")
        
        try:
            while True:
                # Read all sensor data
                solar_data = self.read_registers(0, 23)    # Solar registers 0-22
                opcn3_data = self.read_registers(40, 40)   # OPCN3 registers 40-79
                i2c_data = self.read_registers(90, 6)      # I2C registers 90-95
                bin_counts = self.read_registers(46, 24)   # Bin counts 46-69
                
                # Display data
                self.display_data(solar_data, opcn3_data, i2c_data, bin_counts)
                
                time.sleep(interval)
                
        except KeyboardInterrupt:
            print("\n👋 Monitor zakończony")
        finally:
            if self.serial_conn and self.serial_conn.is_open:
                self.serial_conn.close()

def main():
    if len(sys.argv) < 2:
        print("Użycie: python modbus_monitor.py <port> [interval]")
        print("Przykład: python modbus_monitor.py COM3 1.5")
        print("Przykład: python modbus_monitor.py /dev/ttyUSB0 2.0")
        return 1
    
    port = sys.argv[1]
    interval = float(sys.argv[2]) if len(sys.argv) > 2 else 2.0
    
    monitor = SimpleModbusMonitor(port)
    monitor.monitor(interval)
    
    return 0

if __name__ == "__main__":
    exit(main()) 