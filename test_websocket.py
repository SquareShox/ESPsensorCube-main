#!/usr/bin/env python3
"""
Test WebSocket dla ESP Sensor Cube
Użycie: python test_websocket.py [IP_ADDRESS]
"""

import asyncio
import websockets
import json
import sys
import time
from datetime import datetime

class ESPWebSocketTester:
    def __init__(self, ip_address="192.168.1.100"):
        self.ip_address = ip_address
        self.ws_url = f"ws://{ip_address}/ws"
        self.websocket = None
        
    async def connect(self):
        """Nawiązanie połączenia WebSocket"""
        try:
            self.websocket = await websockets.connect(self.ws_url)
            print(f"✅ Połączono z {self.ws_url}")
            return True
        except Exception as e:
            print(f"❌ Błąd połączenia: {e}")
            return False
    
    async def send_command(self, command):
        """Wysłanie komendy i oczekiwanie na odpowiedź"""
        if not self.websocket:
            print("❌ Brak połączenia WebSocket")
            return None
            
        try:
            await self.websocket.send(json.dumps(command))
            response = await self.websocket.recv()
            return json.loads(response)
        except Exception as e:
            print(f"❌ Błąd komunikacji: {e}")
            return None
    
    async def test_status(self):
        """Test komendy status"""
        print("\n🔍 Test: Status systemu")
        response = await self.send_command({"cmd": "status"})
        if response:
            print(f"✅ Uptime: {response.get('uptime', 'N/A')}s")
            print(f"✅ Free Heap: {response.get('freeHeap', 'N/A')} bytes")
            print(f"✅ WiFi RSSI: {response.get('wifiRSSI', 'N/A')} dBm")
            print(f"✅ WiFi Connected: {response.get('wifiConnected', 'N/A')}")
            
            sensors = response.get('sensors', {})
            print("📊 Status sensorów:")
            for sensor, status in sensors.items():
                status_icon = "✅" if status else "❌"
                print(f"   {status_icon} {sensor}: {status}")
    
    async def test_sensor_data(self):
        """Test pobierania danych sensorów"""
        print("\n📊 Test: Dane sensorów")
        
        # Test wszystkich sensorów
        response = await self.send_command({"cmd": "getSensorData", "sensor": "all"})
        if response and 'data' in response:
            print("✅ Dane wszystkich sensorów:")
            for sensor, data in response['data'].items():
                if data.get('valid'):
                    print(f"   📈 {sensor}: {data}")
                else:
                    print(f"   ❌ {sensor}: brak danych")
        
        # Test pojedynczych sensorów
        sensors_to_test = ["sht40", "sps30", "co2", "power", "hcho"]
        for sensor in sensors_to_test:
            response = await self.send_command({"cmd": "getSensorData", "sensor": sensor})
            if response and 'data' in response and response['data'].get('valid'):
                print(f"✅ {sensor}: {response['data']}")
            else:
                print(f"❌ {sensor}: brak danych lub błąd")
    
    async def test_history(self):
        """Test historii danych"""
        print("\n📈 Test: Historia danych")
        
        time_ranges = ["1h", "6h", "24h"]
        sensors_to_test = ["sht40", "sps30", "power"]
        
        for sensor in sensors_to_test:
            for time_range in time_ranges:
                response = await self.send_command({
                    "cmd": "getHistory",
                    "sensor": sensor,
                    "timeRange": time_range
                })
                
                if response and 'samples' in response:
                    samples = response['samples']
                    if samples > 0:
                        print(f"✅ {sensor} ({time_range}): {samples} próbek")
                    else:
                        print(f"❌ {sensor} ({time_range}): brak danych")
                else:
                    print(f"❌ {sensor} ({time_range}): błąd")
    
    async def test_averages(self):
        """Test średnich uśrednionych"""
        print("\n📊 Test: Średnie uśrednione")
        
        sensors_to_test = ["sht40", "sps30", "power"]
        avg_types = ["fast", "slow"]
        
        for sensor in sensors_to_test:
            for avg_type in avg_types:
                response = await self.send_command({
                    "cmd": "getAverages",
                    "sensor": sensor,
                    "type": avg_type
                })
                
                if response and 'data' in response:
                    data = response['data'].get(sensor, {})
                    if data.get('valid'):
                        print(f"✅ {sensor} ({avg_type}): {data}")
                    else:
                        print(f"❌ {sensor} ({avg_type}): brak danych")
                else:
                    print(f"❌ {sensor} ({avg_type}): błąd")
    
    async def test_config(self):
        """Test konfiguracji"""
        print("\n⚙️ Test: Konfiguracja")
        
        # Pobierz aktualną konfigurację
        response = await self.send_command({"cmd": "getConfig"})
        if response and 'config' in response:
            print("✅ Aktualna konfiguracja:")
            config = response['config']
            for key, value in config.items():
                print(f"   {key}: {value}")
        
        # Test zmiany konfiguracji (tylko odczyt, nie zmieniaj)
        print("\n📝 Test: Zmiana konfiguracji (tylko symulacja)")
        test_config = {
            "cmd": "setConfig",
            "enableSPS30": True,
            "enableSHT40": True
        }
        response = await self.send_command(test_config)
        if response and response.get('success'):
            print("✅ Konfiguracja zaktualizowana")
        else:
            print("❌ Błąd aktualizacji konfiguracji")
    
    async def test_system_commands(self):
        """Test komend systemowych"""
        print("\n🖥️ Test: Komendy systemowe")
        
        # Test informacji o pamięci
        response = await self.send_command({"cmd": "system", "command": "memory"})
        if response and response.get('success'):
            print(f"✅ Pamięć: {response.get('freeHeap', 'N/A')} bytes free")
            print(f"✅ PSRAM: {response.get('freePsram', 'N/A')} bytes free")
            print(f"✅ Uptime: {response.get('uptime', 'N/A')}s")
        
        # Test statusu WiFi
        response = await self.send_command({"cmd": "system", "command": "wifi"})
        if response and response.get('success'):
            print(f"✅ WiFi: {response.get('connected', 'N/A')}")
            print(f"✅ SSID: {response.get('ssid', 'N/A')}")
            print(f"✅ RSSI: {response.get('rssi', 'N/A')} dBm")
            print(f"✅ IP: {response.get('localIP', 'N/A')}")
        
        # UWAGA: Nie testujemy restart/reset w trybie testowym
        print("⚠️ Pominięto test restart/reset (bezpieczeństwo)")
    
    async def test_calibration(self):
        """Test kalibracji"""
        print("\n🔬 Test: Kalibracja")
        
        # Test statusu kalibracji
        response = await self.send_command({"cmd": "calibration", "command": "status"})
        if response and response.get('success'):
            print(f"✅ Kalibracja włączona: {response.get('enabled', 'N/A')}")
            print(f"✅ Dane kalibracji ważne: {response.get('valid', 'N/A')}")
            
            if response.get('data'):
                data = response['data']
                print("📊 Dane kalibracji:")
                for key, value in data.items():
                    if isinstance(value, (int, float)):
                        print(f"   {key}: {value}")
    
    async def monitor_realtime(self, duration=30):
        """Monitorowanie danych w czasie rzeczywistym"""
        print(f"\n📡 Monitorowanie w czasie rzeczywistym ({duration}s)")
        print("Naciśnij Ctrl+C aby przerwać")
        
        start_time = time.time()
        message_count = 0
        
        try:
            while time.time() - start_time < duration:
                try:
                    # Odbierz wiadomości broadcast
                    response = await asyncio.wait_for(self.websocket.recv(), timeout=1.0)
                    data = json.loads(response)
                    message_count += 1
                    
                    if data.get('cmd') == 'update':
                        timestamp = datetime.now().strftime("%H:%M:%S")
                        sensors = data.get('sensors', {})
                        
                        print(f"\n[{timestamp}] Aktualizacja #{message_count}:")
                        for sensor, sensor_data in sensors.items():
                            if sensor_data.get('valid'):
                                if sensor == 'sht40':
                                    temp = sensor_data.get('temperature', 'N/A')
                                    hum = sensor_data.get('humidity', 'N/A')
                                    print(f"   🌡️ SHT40: {temp}°C, {hum}%")
                                elif sensor == 'sps30':
                                    pm25 = sensor_data.get('pm2_5', 'N/A')
                                    print(f"   💨 SPS30 PM2.5: {pm25} µg/m³")
                                elif sensor == 'co2':
                                    co2 = sensor_data.get('co2', 'N/A')
                                    print(f"   🌿 CO2: {co2} ppm")
                                elif sensor == 'power':
                                    power = sensor_data.get('power', 'N/A')
                                    print(f"   ⚡ Moc: {power} mW")
                                elif sensor == 'hcho':
                                    hcho = sensor_data.get('hcho', 'N/A')
                                    print(f"   🧪 HCHO: {hcho} mg/m³")
                    
                except asyncio.TimeoutError:
                    # Brak wiadomości w timeout - to normalne
                    pass
                except Exception as e:
                    print(f"❌ Błąd odbioru: {e}")
                    break
                    
        except KeyboardInterrupt:
            print("\n⏹️ Przerwano monitorowanie")
        
        print(f"📊 Odebrano {message_count} wiadomości w {duration}s")
    
    async def run_all_tests(self):
        """Uruchomienie wszystkich testów"""
        print("🚀 Rozpoczęcie testów WebSocket ESP Sensor Cube")
        print(f"📍 Adres: {self.ws_url}")
        
        if not await self.connect():
            return
        
        try:
            # Czekaj na powitalną wiadomość
            await asyncio.sleep(1)
            
            # Uruchom testy
            await self.test_status()
            await asyncio.sleep(1)
            
            await self.test_sensor_data()
            await asyncio.sleep(1)
            
            await self.test_history()
            await asyncio.sleep(1)
            
            await self.test_averages()
            await asyncio.sleep(1)
            
            await self.test_config()
            await asyncio.sleep(1)
            
            await self.test_system_commands()
            await asyncio.sleep(1)
            
            await self.test_calibration()
            await asyncio.sleep(1)
            
            # Monitorowanie w czasie rzeczywistym
            await self.monitor_realtime(30)
            
        except Exception as e:
            print(f"❌ Błąd podczas testów: {e}")
        finally:
            if self.websocket:
                await self.websocket.close()
                print("🔌 Połączenie zamknięte")

async def main():
    # Sprawdź argumenty linii komend
    ip_address = sys.argv[1] if len(sys.argv) > 1 else "192.168.1.100"
    
    print("ESP Sensor Cube - WebSocket Tester")
    print("=" * 50)
    
    tester = ESPWebSocketTester(ip_address)
    await tester.run_all_tests()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n👋 Test przerwany przez użytkownika")
    except Exception as e:
        print(f"❌ Błąd główny: {e}") 