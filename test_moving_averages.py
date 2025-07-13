#!/usr/bin/env python3
"""
Test script for ESP32 Sensor Cube moving averages system.
Supports the new register format with timestamp and data type information.
"""

import serial
import struct
import time
import argparse
import sys

try:
    import modbus_tk.defines as cst
    from modbus_tk import modbus_rtu
    import modbus_tk.modbus_rtu as rtu
except ImportError:
    print("Error: modbus_tk library not found. Install with: pip install modbus_tk")
    sys.exit(1)

# Modbus configuration
MODBUS_SLAVE_ID = 1
MODBUS_PORT = 'COM3'  # Adjust for your system
MODBUS_BAUDRATE = 19200

# Register addresses - new format
SOLAR_BASE = 0
OPCN3_BASE = 50
I2C_BASE = 100
IPS_BASE = 150
MCP3424_BASE = 200
ADS1110_BASE = 350
INA219_BASE = 400
SPS30_BASE = 450

# Control registers
DATA_TYPE_REG = 90
COMMAND_REG = 91
AUTO_RESET_REG = 92
MOVING_AVG_STATUS_REG = 93

class SensorCubeClient:
    def __init__(self, port=MODBUS_PORT, baudrate=MODBUS_BAUDRATE, slave_id=MODBUS_SLAVE_ID):
        self.port = port
        self.baudrate = baudrate
        self.slave_id = slave_id
        self.master = None
        
    def connect(self):
        """Connect to the Modbus device"""
        try:
            self.master = rtu.RtuMaster(serial.Serial(port=self.port, baudrate=self.baudrate, 
                                                     bytesize=8, parity='N', stopbits=1, xonxoff=0))
            self.master.set_timeout(2.0)
            self.master.set_verbose(True)
            print(f"Connected to {self.port} at {self.baudrate} baud")
            return True
        except Exception as e:
            print(f"Failed to connect: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from the Modbus device"""
        if self.master:
            self.master.close()
            print("Disconnected")
    
    def read_sensor_header(self, base_reg):
        """Read common header from any sensor (status, data type, timestamp)"""
        try:
            data = self.master.execute(self.slave_id, cst.READ_HOLDING_REGISTERS, base_reg, 4)
            status = data[0]
            data_type = data[1]
            timestamp = (data[3] << 16) | data[2]  # Combine upper and lower 16 bits
            
            data_type_names = {0: "Current", 1: "Fast Avg (10s)", 2: "Slow Avg (5min)"}
            data_type_name = data_type_names.get(data_type, f"Unknown ({data_type})")
            
            return {
                'status': status,
                'data_type': data_type,
                'data_type_name': data_type_name,
                'timestamp': timestamp,
                'timestamp_ms': timestamp
            }
        except Exception as e:
            print(f"Error reading sensor header at {base_reg}: {e}")
            return None
    
    def read_solar_data(self):
        """Read solar sensor data"""
        header = self.read_sensor_header(SOLAR_BASE)
        if not header:
            return None
            
        try:
            # Read solar-specific data starting from register 4
            data = self.master.execute(self.slave_id, cst.READ_HOLDING_REGISTERS, SOLAR_BASE + 4, 19)
            
            result = header.copy()
            result.update({
                'sensor_name': 'Solar',
                'pid': f"0x{data[0]:04X}",
                'firmware': data[1],
                'serial': data[2],
                'battery_voltage': data[3],  # mV
                'battery_current': data[4],  # mA (signed)
                'panel_voltage': data[5],    # mV
                'panel_power': data[6],      # mW
                'charge_state': data[7],
                'mppt_state': data[8],
                'or_register': f"0x{data[9]:04X}",
                'error_flags': data[10],
                'load_state': 'ON' if data[11] else 'OFF',
                'load_current': data[12]     # mA
            })
            return result
        except Exception as e:
            print(f"Error reading solar data: {e}")
            return None
    
    def read_i2c_data(self):
        """Read I2C sensor data"""
        header = self.read_sensor_header(I2C_BASE)
        if not header:
            return None
            
        try:
            # Read I2C-specific data starting from register 4
            data = self.master.execute(self.slave_id, cst.READ_HOLDING_REGISTERS, I2C_BASE + 4, 5)
            
            result = header.copy()
            result.update({
                'sensor_name': 'I2C',
                'temperature': data[0] / 100.0,  # °C
                'humidity': data[1] / 100.0,     # %
                'pressure': data[2] / 10.0,      # hPa
                'co2': data[3],                  # ppm
                'sensor_type': data[4]
            })
            return result
        except Exception as e:
            print(f"Error reading I2C data: {e}")
            return None
    
    def read_sps30_data(self):
        """Read SPS30 sensor data"""
        header = self.read_sensor_header(SPS30_BASE)
        if not header:
            return None
            
        try:
            # Read SPS30-specific data starting from register 4
            data = self.master.execute(self.slave_id, cst.READ_HOLDING_REGISTERS, SPS30_BASE + 4, 11)
            
            result = header.copy()
            result.update({
                'sensor_name': 'SPS30',
                'pm1_0': data[0] / 1000.0,   # µg/m³
                'pm2_5': data[1] / 1000.0,   # µg/m³
                'pm4_0': data[2] / 1000.0,   # µg/m³
                'pm10': data[3] / 1000.0,    # µg/m³
                'nc0_5': data[4] / 1000.0,   # #/cm³
                'nc1_0': data[5] / 1000.0,   # #/cm³
                'nc2_5': data[6] / 1000.0,   # #/cm³
                'nc4_0': data[7] / 1000.0,   # #/cm³
                'nc10': data[8] / 1000.0,    # #/cm³
                'typical_size': data[9] / 1000.0,  # µm
                'age_seconds': data[10]
            })
            return result
        except Exception as e:
            print(f"Error reading SPS30 data: {e}")
            return None
    
    def read_ina219_data(self):
        """Read INA219 power monitor data"""
        header = self.read_sensor_header(INA219_BASE)
        if not header:
            return None
            
        try:
            # Read INA219-specific data starting from register 4
            data = self.master.execute(self.slave_id, cst.READ_HOLDING_REGISTERS, INA219_BASE + 4, 5)
            
            result = header.copy()
            result.update({
                'sensor_name': 'INA219',
                'bus_voltage': data[0],      # mV
                'shunt_voltage': data[1] / 10.0,  # mV (0.1mV precision)
                'current': data[2],          # mA
                'power': data[3],            # mW
                'age_seconds': data[4]
            })
            return result
        except Exception as e:
            print(f"Error reading INA219 data: {e}")
            return None
    
    def get_current_data_type(self):
        """Get current data type setting"""
        try:
            data_type = self.master.execute(self.slave_id, cst.READ_HOLDING_REGISTERS, DATA_TYPE_REG, 1)[0]
            data_type_names = {0: "Current", 1: "Fast Avg (10s)", 2: "Slow Avg (5min)"}
            return data_type, data_type_names.get(data_type, f"Unknown ({data_type})")
        except Exception as e:
            print(f"Error reading data type: {e}")
            return None, None
    
    def set_data_type(self, data_type):
        """Set data type (0=current, 1=fast avg, 2=slow avg)"""
        try:
            self.master.execute(self.slave_id, cst.WRITE_SINGLE_REGISTER, DATA_TYPE_REG, data_type)
            print(f"Data type set to {data_type}")
            return True
        except Exception as e:
            print(f"Error setting data type: {e}")
            return False
    
    def send_command(self, command):
        """Send system command"""
        try:
            self.master.execute(self.slave_id, cst.WRITE_SINGLE_REGISTER, COMMAND_REG, command)
            print(f"Command {command} sent")
            return True
        except Exception as e:
            print(f"Error sending command: {e}")
            return False
    
    def read_all_sensors(self):
        """Read data from all available sensors"""
        sensors = []
        
        # Read each sensor type
        for sensor_func in [self.read_solar_data, self.read_i2c_data, self.read_sps30_data, self.read_ina219_data]:
            data = sensor_func()
            if data and data['status']:  # Only include if sensor is active
                sensors.append(data)
        
        return sensors

def test_basic_communication(client):
    """Test basic Modbus communication"""
    print("\n=== Basic Communication Test ===")
    
    if not client.connect():
        return False
    
    try:
        # Try to read the data type register
        data_type, data_type_name = client.get_current_data_type()
        if data_type is not None:
            print(f"✓ Communication OK - Current data type: {data_type_name}")
            return True
        else:
            print("✗ Failed to read data type register")
            return False
    except Exception as e:
        print(f"✗ Communication failed: {e}")
        return False

def test_moving_averages(client):
    """Test moving averages system"""
    print("\n=== Moving Averages Test ===")
    
    if not client.connect():
        return
    
    try:
        print("Testing moving averages functionality...")
        
        # Read sensors in each mode
        for data_type, name in [(0, "Current"), (1, "Fast Avg (10s)"), (2, "Slow Avg (5min)")]:
            print(f"\n--- {name} Data ---")
            client.set_data_type(data_type)
            time.sleep(1)  # Allow time for data type change
            
            sensors = client.read_all_sensors()
            
            if sensors:
                for sensor in sensors:
                    print(f"{sensor['sensor_name']:>8}: Status={sensor['status']}, "
                          f"Type={sensor['data_type_name']}, "
                          f"Timestamp={sensor['timestamp']}")
                    
                    # Print key sensor values
                    if sensor['sensor_name'] == 'Solar' and sensor['status']:
                        print(f"         Battery: {sensor['battery_voltage']}mV, {sensor['battery_current']}mA")
                    elif sensor['sensor_name'] == 'I2C' and sensor['status']:
                        print(f"         Temp: {sensor['temperature']:.2f}°C, Humidity: {sensor['humidity']:.2f}%")
                    elif sensor['sensor_name'] == 'SPS30' and sensor['status']:
                        print(f"         PM2.5: {sensor['pm2_5']:.3f}µg/m³, PM10: {sensor['pm10']:.3f}µg/m³")
                    elif sensor['sensor_name'] == 'INA219' and sensor['status']:
                        print(f"         Bus: {sensor['bus_voltage']}mV, Current: {sensor['current']}mA")
            else:
                print("No active sensors found")
        
        print("\n✓ Moving averages test completed")
        
    except Exception as e:
        print(f"✗ Moving averages test failed: {e}")
    
    finally:
        client.disconnect()

def demo_data_switching(client):
    """Demonstrate switching between data types with value comparison"""
    print("\n=== Data Type Switching Demo ===")
    
    if not client.connect():
        return
    
    try:
        print("Demonstrating data type switching with value comparison...")
        
        # Collect data from all three modes
        results = {}
        
        for data_type, name in [(0, "Current"), (1, "Fast Avg"), (2, "Slow Avg")]:
            print(f"\nSwitching to {name}...")
            client.set_data_type(data_type)
            time.sleep(2)  # Allow time for data type change
            
            sensors = client.read_all_sensors()
            results[name] = sensors
            
            if sensors:
                print(f"Found {len(sensors)} active sensors in {name} mode")
            else:
                print(f"No active sensors in {name} mode")
        
        # Compare values across data types
        print("\n--- Value Comparison ---")
        sensor_names = set()
        for mode_sensors in results.values():
            if mode_sensors:
                sensor_names.update(s['sensor_name'] for s in mode_sensors)
        
        for sensor_name in sensor_names:
            print(f"\n{sensor_name} Sensor:")
            for mode_name, mode_sensors in results.items():
                sensor_data = None
                if mode_sensors:
                    sensor_data = next((s for s in mode_sensors if s['sensor_name'] == sensor_name), None)
                
                if sensor_data and sensor_data['status']:
                    print(f"  {mode_name:>12}: Active (timestamp: {sensor_data['timestamp']})")
                    
                    # Show specific values for comparison
                    if sensor_name == 'I2C':
                        print(f"               Temp: {sensor_data['temperature']:.2f}°C, "
                              f"Humidity: {sensor_data['humidity']:.2f}%, "
                              f"CO2: {sensor_data['co2']}ppm")
                    elif sensor_name == 'Solar':
                        print(f"               Battery: {sensor_data['battery_voltage']}mV, "
                              f"Current: {sensor_data['battery_current']}mA")
                    elif sensor_name == 'SPS30':
                        print(f"               PM2.5: {sensor_data['pm2_5']:.3f}µg/m³, "
                              f"PM10: {sensor_data['pm10']:.3f}µg/m³")
                    elif sensor_name == 'INA219':
                        print(f"               Bus: {sensor_data['bus_voltage']}mV, "
                              f"Current: {sensor_data['current']}mA, "
                              f"Power: {sensor_data['power']}mW")
                else:
                    print(f"  {mode_name:>12}: Inactive or no data")
        
        print("\n✓ Data type switching demo completed")
        
    except Exception as e:
        print(f"✗ Data switching demo failed: {e}")
    
    finally:
        client.disconnect()

def monitor_continuous(client, data_type=0):
    """Continuously monitor sensor data"""
    print(f"\n=== Continuous Monitoring (Data Type: {data_type}) ===")
    print("Press Ctrl+C to stop...")
    
    if not client.connect():
        return
    
    try:
        client.set_data_type(data_type)
        time.sleep(1)
        
        while True:
            current_type, current_type_name = client.get_current_data_type()
            print(f"\n--- {time.strftime('%H:%M:%S')} - {current_type_name} ---")
            
            sensors = client.read_all_sensors()
            
            if sensors:
                for sensor in sensors:
                    if sensor['status']:
                        print(f"{sensor['sensor_name']:>8}: ", end="")
                        
                        if sensor['sensor_name'] == 'Solar':
                            print(f"Battery: {sensor['battery_voltage']}mV/{sensor['battery_current']}mA, "
                                  f"Panel: {sensor['panel_voltage']}mV/{sensor['panel_power']}mW")
                        elif sensor['sensor_name'] == 'I2C':
                            print(f"T:{sensor['temperature']:.2f}°C, H:{sensor['humidity']:.2f}%, "
                                  f"P:{sensor['pressure']:.1f}hPa, CO2:{sensor['co2']}ppm")
                        elif sensor['sensor_name'] == 'SPS30':
                            print(f"PM1.0:{sensor['pm1_0']:.3f}, PM2.5:{sensor['pm2_5']:.3f}, "
                                  f"PM10:{sensor['pm10']:.3f}µg/m³")
                        elif sensor['sensor_name'] == 'INA219':
                            print(f"Bus:{sensor['bus_voltage']}mV, I:{sensor['current']}mA, "
                                  f"P:{sensor['power']}mW")
                    else:
                        print(f"{sensor['sensor_name']:>8}: Inactive")
            else:
                print("No active sensors")
            
            time.sleep(5)  # Update every 5 seconds
            
    except KeyboardInterrupt:
        print("\nMonitoring stopped by user")
    except Exception as e:
        print(f"\nMonitoring error: {e}")
    finally:
        client.disconnect()

def send_system_command(client, command):
    """Send a system command"""
    print(f"\n=== Sending System Command {command} ===")
    
    if not client.connect():
        return
    
    try:
        # Map command names to numbers
        command_map = {
            'opcn3_read': 1,
            'system_reset': 2,
            'toggle_auto_reset': 3,
            'ips_read': 4,
            'ips_debug_on': 5,
            'ips_debug_off': 6,
            'set_current': 7,
            'set_fast_avg': 8,
            'set_slow_avg': 9,
            'print_avg_status': 10,
            'cycle_data_type': 11
        }
        
        if isinstance(command, str):
            command_num = command_map.get(command)
            if command_num is None:
                print(f"Unknown command: {command}")
                print(f"Available commands: {list(command_map.keys())}")
                return
        else:
            command_num = command
        
        print(f"Sending command {command_num}...")
        client.send_command(command_num)
        
        # For data type commands, verify the change
        if command_num in [7, 8, 9, 11]:
            time.sleep(1)
            data_type, data_type_name = client.get_current_data_type()
            if data_type is not None:
                print(f"Current data type after command: {data_type_name}")
        
        print("✓ Command sent successfully")
        
    except Exception as e:
        print(f"✗ Command failed: {e}")
    finally:
        client.disconnect()

def main():
    parser = argparse.ArgumentParser(description='ESP32 Sensor Cube Test Script with Moving Averages')
    parser.add_argument('--port', default=MODBUS_PORT, help='Serial port (default: COM3)')
    parser.add_argument('--baudrate', type=int, default=MODBUS_BAUDRATE, help='Baud rate (default: 19200)')
    parser.add_argument('--slave-id', type=int, default=MODBUS_SLAVE_ID, help='Modbus slave ID (default: 1)')
    
    # Test modes
    parser.add_argument('--test-basic', action='store_true', help='Test basic communication')
    parser.add_argument('--test-averages', action='store_true', help='Test moving averages system')
    parser.add_argument('--demo-switching', action='store_true', help='Demo data type switching with value comparison')
    parser.add_argument('--monitor', action='store_true', help='Continuously monitor sensors')
    parser.add_argument('--datatype', type=int, choices=[0, 1, 2], default=0, 
                       help='Data type for monitoring (0=current, 1=fast avg, 2=slow avg)')
    
    # Command mode
    parser.add_argument('--command', help='Send system command (see available commands in code)')
    
    args = parser.parse_args()
    
    print(f"ESP32 Sensor Cube Test Script")
    print(f"Connecting to {args.port} at {args.baudrate} baud, slave ID {args.slave_id}")
    
    client = SensorCubeClient(args.port, args.baudrate, args.slave_id)
    
    try:
        if args.test_basic:
            test_basic_communication(client)
        elif args.test_averages:
            test_moving_averages(client)
        elif args.demo_switching:
            demo_data_switching(client)
        elif args.monitor:
            monitor_continuous(client, args.datatype)
        elif args.command:
            send_system_command(client, args.command)
        else:
            # Default: run basic test
            if test_basic_communication(client):
                print("\nBasic test passed! Try:")
                print("  --test-averages     : Test moving averages")
                print("  --demo-switching    : Demo data type switching")
                print("  --monitor           : Continuous monitoring")
                print("  --command <cmd>     : Send system command")
    except KeyboardInterrupt:
        print("\nStopped by user")
    except Exception as e:
        print(f"Unexpected error: {e}")

if __name__ == "__main__":
    main() 