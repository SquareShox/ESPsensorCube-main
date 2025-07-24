#ifndef CHARTS_HTML_H
#define CHARTS_HTML_H

const char *charts_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP Sensor Cube - Wykresy</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<script src="https://cdn.jsdelivr.net/npm/chartjs-adapter-date-fns"></script>
<style>
  * {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
  }

  body {
    font-family: 'Arial', sans-serif;
    background: linear-gradient(135deg, #2c3e50 0%, #34495e 100%);
    min-height: 100vh;
    padding: 20px;
    color: white;
  }

  .header {
    text-align: center;
    color: white;
    margin-bottom: 30px;
  }

  .header h1 {
    font-size: 2.5em;
    text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
  }

  .nav {
    text-align: center;
    margin-bottom: 30px;
  }

  .nav a {
    color: white;
    text-decoration: none;
    margin: 0 15px;
    padding: 10px 20px;
    border: 2px solid white;
    border-radius: 25px;
    transition: all 0.3s ease;
  }

  .nav a:hover {
    background-color: white;
    color: #2c3e50;
  }

  .controls {
    text-align: center;
    margin-bottom: 30px;
  }

  .controls select, .controls button {
    margin: 0 10px;
    padding: 10px 15px;
    border: none;
    border-radius: 5px;
    font-size: 14px;
  }

  .controls button {
    background: linear-gradient(45deg, #3498db, #2980b9);
    color: white;
    cursor: pointer;
    transition: all 0.3s ease;
  }

  .controls button:hover {
    background: linear-gradient(45deg, #2980b9, #1f4e79);
    transform: scale(1.05);
  }

  .charts-container {
    max-width: 1400px;
    margin: 0 auto;
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(600px, 1fr));
    gap: 30px;
  }

  .chart-card {
    background: rgba(255, 255, 255, 0.95);
    border-radius: 15px;
    padding: 25px;
    box-shadow: 0 10px 30px rgba(0,0,0,0.2);
    color: #333;
  }

  .chart-title {
    font-size: 1.3em;
    font-weight: bold;
    margin-bottom: 20px;
    text-align: center;
    color: #2c3e50;
    border-bottom: 2px solid #ecf0f1;
    padding-bottom: 10px;
  }

  .chart-container {
    position: relative;
    height: 300px;
    margin-bottom: 15px;
  }

  .chart-info {
    text-align: center;
    font-size: 0.9em;
    color: #666;
  }

  .loading {
    text-align: center;
    padding: 50px;
    font-size: 1.2em;
    color: #666;
  }

  .no-data {
    text-align: center;
    padding: 50px;
    font-size: 1.1em;
    color: #999;
  }

  .status-bar {
    text-align: center;
    padding: 15px;
    background: rgba(255, 255, 255, 0.1);
    border-radius: 10px;
    margin-bottom: 20px;
    color: white;
  }

  /* Configuration Panel Styles */
  .config-section {
    background: #f8f9fa;
    border-radius: 10px;
    padding: 20px;
    border: 1px solid #e9ecef;
  }

  .config-section h4 {
    margin-bottom: 15px;
    color: #2c3e50;
    font-size: 1.1em;
    border-bottom: 2px solid #3498db;
    padding-bottom: 8px;
  }

  .config-checkbox {
    display: flex;
    align-items: center;
    margin-bottom: 12px;
    cursor: pointer;
    font-size: 0.95em;
  }

  .config-checkbox input[type="checkbox"] {
    margin-right: 10px;
    width: 18px;
    height: 18px;
    accent-color: #3498db;
  }

  .config-input-group {
    display: flex;
    margin-top: 10px;
    gap: 10px;
  }

  .config-input {
    flex: 1;
    padding: 8px 12px;
    border: 1px solid #ddd;
    border-radius: 5px;
    font-size: 14px;
  }

  .btn {
    padding: 8px 16px;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    font-size: 14px;
    transition: all 0.3s ease;
  }

  .btn-primary {
    background: linear-gradient(45deg, #3498db, #2980b9);
    color: white;
  }

  .btn-primary:hover {
    background: linear-gradient(45deg, #2980b9, #1f4e79);
    transform: scale(1.05);
  }

  .btn-secondary {
    background: linear-gradient(45deg, #95a5a6, #7f8c8d);
    color: white;
  }

  .btn-secondary:hover {
    background: linear-gradient(45deg, #7f8c8d, #6c7b7d);
    transform: scale(1.05);
  }

  .btn-danger {
    background: linear-gradient(45deg, #e74c3c, #c0392b);
    color: white;
  }

  .btn-danger:hover {
    background: linear-gradient(45deg, #c0392b, #a93226);
    transform: scale(1.05);
  }

  @media (max-width: 768px) {
    .charts-container {
      grid-template-columns: 1fr;
    }
    
    .header h1 {
      font-size: 2em;
    }
    
    .chart-container {
      height: 250px;
    }

    #configPanel {
      margin: 0 10px 30px 10px !important;
      padding: 15px !important;
    }

    #configPanel > div {
      grid-template-columns: 1fr !important;
    }
  }
</style>
<script src="/common.js"></script>
</head>
<body>
  <div class="header">
    <h1>📈 Wykresy Pomiarów</h1>
    <p>Historia danych z czujników - JEDEN CZUJNIK NA RAZ</p>
  </div>

  <div class="nav">
    <a href="/">🔧 Panel Sterowania</a>
    <a href="/dashboard">📊 Dashboard</a>
    <a href="/charts">📈 Wykresy</a>
    <a href="/network">🌐 Sieć</a>
    <a href="/mcp3424">🔌 MCP3424</a>
  </div>

  <div class="status-bar" id="status-bar">
    <span id="connection-status">Łączenie...</span> | 
    <span id="history-status">Historia: --</span> | 
    <span id="update-status">Ostatnia aktualizacja: --</span>
  </div>

  <div class="controls">
    <label style="color: white;">Zakres czasu:</label>
    <select id="timeRange">
      <option value="1h">Ostatnia godzina</option>
      <option value="6h">Ostatnie 6 godzin</option>
      <option value="24h">Ostatnie 24 godziny</option>
    </select>
    
    <label style="color: white;">Typ próbek:</label>
    <select id="sampleType">
      <option value="fast">Szybkie (10s)</option>
      <option value="slow">Wolne (5min)</option>
    </select>
    
    <label style="color: white;">Czujnik:</label>
    <select id="sensorType">
      <option value="sps30">SPS30 (Pyłki)</option>
      <option value="ips">IPS (Pyłki)</option>
      <option value="power">INA219 (Moc)</option>
      <option value="sht40">SHT40 (Temperatura/Wilgotność)</option>
      <option value="co2">CO2 (SCD41)</option>
      <option value="hcho">HCHO (VOC)</option>
      <option value="battery">Battery (Bateria)</option>
      <option value="calibration">Kalibracja (Gazy)</option>
      <option value="fan">Wentylator</option>
    </select>
    
    <button onclick="updateCharts()">🔄 Odśwież</button>
    <button onclick="toggleAutoUpdate()">⏰ Auto: <span id="auto-status">OFF</span></button>
    <button onclick="toggleConfigPanel()">⚙️ Konfiguracja</button>
    <button onclick="window.location.href='/network'" style="background: linear-gradient(45deg, #4caf50, #8bc34a);">🌐 Sieć</button>
    <button onclick="window.location.href='/mcp3424'" style="background: linear-gradient(45deg, #ff9800, #ffc107);">🔌 MCP3424</button>
  </div>

  <!-- Panel Konfiguracji -->
  <div id="configPanel" style="display: none; max-width: 1200px; margin: 0 auto 30px auto; background: rgba(255, 255, 255, 0.95); border-radius: 15px; padding: 25px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); color: #333;">
    <h3 style="text-align: center; margin-bottom: 25px; color: #2c3e50; font-size: 1.5em;">⚙️ Konfiguracja Systemu</h3>
    
    <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 25px;">
      
             <!-- Historia i Dane -->
       <div class="config-section">
         <h4>📊 Historia i Dane</h4>
         <label class="config-checkbox">
           <input type="checkbox" id="enableHistoryChart" onchange="updateSystemConfigChart('enableHistory', this.checked)">
           <span class="checkmark"></span>
           Włącz zapis historii danych (PSRAM)
         </label>
       </div>

             <!-- Czujniki Podstawowe -->
       <div class="config-section">
         <h4>🌡️ Czujniki Podstawowe</h4>
         <label class="config-checkbox">
           <input type="checkbox" id="enableSPS30Chart" onchange="updateSystemConfigChart('enableSPS30', this.checked)">
           <span class="checkmark"></span>
           SPS30 (Sensirion Particle Sensor)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableSHT40Chart" onchange="updateSystemConfigChart('enableSHT40', this.checked)">
           <span class="checkmark"></span>
           SHT40 (Temp/Humidity)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableSHT30Chart" onchange="updateSystemConfigChart('enableSHT30', this.checked)">
           <span class="checkmark"></span>
           SHT30 (Temp/Humidity)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableBME280Chart" onchange="updateSystemConfigChart('enableBME280', this.checked)">
           <span class="checkmark"></span>
           BME280 (Temp/Humidity/Pressure)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableSCD41Chart" onchange="updateSystemConfigChart('enableSCD41', this.checked)">
           <span class="checkmark"></span>
           SCD41 (CO2 Sensor)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableHCHOChart" onchange="updateSystemConfigChart('enableHCHO', this.checked)">
           <span class="checkmark"></span>
           HCHO (CB-HCHO-V4 Formaldehyde)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableINA219Chart" onchange="updateSystemConfigChart('enableINA219', this.checked)">
           <span class="checkmark"></span>
           INA219 (Current/Voltage Sensor)
         </label>
       </div>

       <!-- Czujniki Zaawansowane -->
       <div class="config-section">
         <h4>🔬 Czujniki Zaawansowane</h4>
         <label class="config-checkbox">
           <input type="checkbox" id="enableI2CSensorsChart" onchange="updateSystemConfigChart('enableI2CSensors', this.checked)">
           <span class="checkmark"></span>
           I2C Sensors (General)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableMCP3424Chart" onchange="updateSystemConfigChart('enableMCP3424', this.checked)">
           <span class="checkmark"></span>
           MCP3424 (18-bit ADC Converter)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableADS1110Chart" onchange="updateSystemConfigChart('enableADS1110', this.checked)">
           <span class="checkmark"></span>
           ADS1110 (16-bit ADC Converter)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableSolarSensorChart" onchange="updateSystemConfigChart('enableSolarSensor', this.checked)">
           <span class="checkmark"></span>
           Solar Sensor (Same pins as IPS)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableOPCN3SensorChart" onchange="updateSystemConfigChart('enableOPCN3Sensor', this.checked)">
           <span class="checkmark"></span>
           OPCN3 Sensor (Particle Counter)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableIPSChart" onchange="updateSystemConfigChart('enableIPS', this.checked)">
           <span class="checkmark"></span>
           IPS (UART Particle Sensor)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableIPSDebugChart" onchange="updateSystemConfigChart('enableIPSDebug', this.checked)">
           <span class="checkmark"></span>
           IPS Debug Mode
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableSHT30Chart" onchange="updateSystemConfigChart('enableSHT30', this.checked)">
           <span class="checkmark"></span>
           SHT30 (Temp/Humidity)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableBME280Chart" onchange="updateSystemConfigChart('enableBME280', this.checked)">
           <span class="checkmark"></span>
           BME280 (Temp/Humidity/Pressure)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableSCD41Chart" onchange="updateSystemConfigChart('enableSCD41', this.checked)">
           <span class="checkmark"></span>
           SCD41 (CO2 Sensor)
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableOPCN3SensorChart" onchange="updateSystemConfigChart('enableOPCN3Sensor', this.checked)">
           <span class="checkmark"></span>
           OPCN3 Sensor (Particle Counter)
         </label>
       </div>

      <!-- Komunikacja -->
      <div class="config-section">
        <h4>📡 Komunikacja</h4>
        <label class="config-checkbox">
          <input type="checkbox" id="enableWiFiChart" onchange="updateSystemConfigChart('enableWiFi', this.checked)">
          <span class="checkmark"></span>
          WiFi
        </label>
        <label class="config-checkbox">
          <input type="checkbox" id="enableWebServerChart" onchange="updateSystemConfigChart('enableWebServer', this.checked)">
          <span class="checkmark"></span>
          Serwer WWW
        </label>
        <label class="config-checkbox">
          <input type="checkbox" id="enableModbusChart" onchange="updateSystemConfigChart('enableModbus', this.checked)">
          <span class="checkmark"></span>
          Modbus
        </label>
      </div>

      <!-- Powiadomienia -->
      <div class="config-section">
        <h4>🔔 Powiadomienia</h4>
        <label class="config-checkbox">
          <input type="checkbox" id="enablePushbulletChart" onchange="updateSystemConfigChart('enablePushbullet', this.checked)">
          <span class="checkmark"></span>
          Powiadomienia Pushbullet
        </label>
        <div class="config-input-group">
          <input type="text" id="pushbulletTokenChart" placeholder="Token Pushbullet" 
                 onchange="updateSystemConfigChart('pushbulletToken', this.value)" 
                 class="config-input">
          <button onclick="testPushbulletChart()" class="btn btn-secondary">🧪 Test</button>
        </div>
      </div>

             <!-- Tryb Pracy i Funkcje -->
       <div class="config-section">
         <h4>⚡ Tryb Pracy i Funkcje</h4>
         <label class="config-checkbox">
           <input type="checkbox" id="lowPowerModeChart" onchange="updateSystemConfigChart('lowPowerMode', this.checked)">
           <span class="checkmark"></span>
           Tryb niskiego poboru energii
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="autoResetChart" onchange="updateSystemConfigChart('autoReset', this.checked)">
           <span class="checkmark"></span>
           Automatyczny reset
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="useAveragedDataChart" onchange="updateSystemConfigChart('useAveragedData', this.checked)">
           <span class="checkmark"></span>
           Używaj uśrednionych danych
         </label>
         <label class="config-checkbox">
           <input type="checkbox" id="enableFanChart" onchange="updateSystemConfigChart('enableFan', this.checked)">
           <span class="checkmark"></span>
           Fan Control System (PWM/Tacho/GLine)
         </label>
       </div>

    </div>

    <!-- Akcje -->
    <div style="text-align: center; margin-top: 30px; border-top: 1px solid #ddd; padding-top: 20px;">
      <button onclick="saveAllConfigChart()" class="btn btn-primary" style="margin: 0 10px;">💾 Ustaw Konfigurację</button>
      <button onclick="loadSystemConfigChart()" class="btn btn-secondary" style="margin: 0 10px;">🔄 Odśwież Konfigurację</button>
      <button onclick="testPushbulletChart()" class="btn btn-secondary" style="margin: 0 10px;">🧪 Test Pushbullet</button>
      <button onclick="testBatteryNotificationChart()" class="btn btn-secondary" style="margin: 0 10px;">🔋 Test Baterii</button>
      <button onclick="restartSystemChart()" class="btn btn-danger" style="margin: 0 10px;">🔄 Restart Systemu</button>
    </div>
  </div>

  <div class="charts-container" id="charts-container">
    <div class="loading">📊 Wybierz czujnik i czas...</div>
  </div>

<script>
let charts = {};
let autoUpdate = false;
let autoUpdateInterval;
let ws;

function connectWebSocket() {
  ws = new WebSocket(`ws://${window.location.host}/ws`);
  ws.onopen = function() {
    document.getElementById('connection-status').textContent = 'Połączono ✅';
  };
  ws.onclose = function() {
    document.getElementById('connection-status').textContent = 'Rozłączono ❌';
    setTimeout(connectWebSocket, 5000);
  };
  ws.onmessage = function(event) {
    try {
      const data = JSON.parse(event.data);
      console.log('WebSocket message received:', data.cmd, data);
      
      // Handle different WebSocket commands
      if (data.cmd === 'history') {
        console.log('History command received, calling handleHistoryResponse');
        // Handle history data from WebSocket
        handleHistoryResponse(data);
      } else if (data.cmd === 'getConfig') {
        console.log('Config response received:', data);
        // Handle configuration response
        if (data.success && data.config) {
          console.log('Updating config display with:', data.config);
          updateSystemConfigDisplayChart(data.config);
        } else {
          console.error('Config response error:', data);
        }
      } else if (data.cmd === 'setConfig') {
        console.log('Config update response:', data);
        // Handle configuration update response
        if (data.success) {
          console.log('Configuration updated successfully');
        } else {
          console.error('Configuration update failed:', data.error);
        }
      } else {
        console.log('Non-history command, updating status bar');
        // Update status bar for other messages
        updateStatusBar(data);
      }
    } catch (error) {
      console.error('Error parsing WebSocket data:', error);
    }
  };
}

function updateStatusBar(data) {
  document.getElementById('update-status').textContent = `Ostatnia aktualizacja: ${new Date().toLocaleTimeString('pl-PL')}`;
  
  // Handle both old and new format
  const history = data.history || (data.data && data.data.history);
  if (history) {
    const memoryPercent = history.memoryUsed > 0 ? Math.round((history.memoryUsed / history.memoryBudget) * 100) : 0;
    document.getElementById('history-status').textContent = 
      `Historia: ${history.enabled ? '✅' : '❌'} (${memoryPercent}% pamięci)`;
  }
  
  // Update low power mode status if available
  if (data.lowPowerMode !== undefined) {
    updateLowPowerModeStatus(data.lowPowerMode);
  } else if (data.data && data.data.config && data.data.config.lowPowerMode !== undefined) {
    updateLowPowerModeStatus(data.data.config.lowPowerMode);
  }
}

// Global variable to store pending history request
let pendingHistoryRequest = null;

function updateLowPowerModeStatus(enabled) {
  const element = document.getElementById("lowPowerModeState");
  if (element) {
    element.innerHTML = `Low Power Mode: <span style="color: ${enabled ? '#ff9800' : '#4caf50'}">${enabled ? 'Włączony' : 'Wyłączony'}</span>`;
  }
}

function toggleLowPowerMode() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    // Get current status first
    ws.send(JSON.stringify({cmd: "status"}));
    
    setTimeout(() => {
      // Toggle based on current state
      const command = document.getElementById("lowPowerModeState").textContent.includes("Włączony") ? 
        "lowPowerOff" : "lowPowerOn";
      
      ws.send(JSON.stringify({
        cmd: "system",
        command: command
      }));
    }, 100);
  }
}

function testPushbullet() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      cmd: "system",
      command: "pushbulletTest"
    }));
  }
}

function toggleConfigPanel() {
  console.log('toggleConfigPanel called');
  const panel = document.getElementById('configPanel');
  console.log('Panel element:', panel);
  console.log('Current display style:', panel ? panel.style.display : 'element not found');
  
  if (panel) {
    if (panel.style.display === 'none' || panel.style.display === '') {
      console.log('Showing config panel');
      panel.style.display = 'block';
      loadSystemConfigChart(); // Load current config when opening
    } else {
      console.log('Hiding config panel');
      panel.style.display = 'none';
    }
  } else {
    console.error('Config panel element not found!');
    alert('Błąd: Panel konfiguracji nie został znaleziony!');
  }
}

function updateSystemConfigChart(configKey, value) {
  console.log('Updating config:', configKey, '=', value);
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    const message = {
      cmd: 'setConfig',
      [configKey]: value
    };
    ws.send(JSON.stringify(message));
  } else {
    console.error('WebSocket not connected');
  }
}

function loadSystemConfigChart() {
  console.log('Loading system configuration...');
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    console.log('Sending getConfig command...');
    ws.send(JSON.stringify({
      cmd: 'getConfig'
    }));
  } else {
    console.error('WebSocket not connected for config load, state:', ws ? ws.readyState : 'no ws');
  }
}

function updateSystemConfigDisplayChart(config) {
  console.log('Updating config display with:', config);
  
  // Historia i Dane
  if (config.enableHistory !== undefined) {
    document.getElementById('enableHistoryChart').checked = config.enableHistory;
  }
  if (config.useAveragedData !== undefined) {
    document.getElementById('useAveragedDataChart').checked = config.useAveragedData;
  }
  
     // Czujniki Podstawowe
   if (config.enableSPS30 !== undefined) {
     document.getElementById('enableSPS30Chart').checked = config.enableSPS30;
   }
   if (config.enableSHT40 !== undefined) {
     document.getElementById('enableSHT40Chart').checked = config.enableSHT40;
   }
   if (config.enableSHT30 !== undefined) {
     document.getElementById('enableSHT30Chart').checked = config.enableSHT30;
   }
   if (config.enableBME280 !== undefined) {
     document.getElementById('enableBME280Chart').checked = config.enableBME280;
   }
   if (config.enableSCD41 !== undefined) {
     document.getElementById('enableSCD41Chart').checked = config.enableSCD41;
   }
   if (config.enableHCHO !== undefined) {
     document.getElementById('enableHCHOChart').checked = config.enableHCHO;
   }
   if (config.enableINA219 !== undefined) {
     document.getElementById('enableINA219Chart').checked = config.enableINA219;
   }
   
   // Czujniki Zaawansowane
   if (config.enableI2CSensors !== undefined) {
     document.getElementById('enableI2CSensorsChart').checked = config.enableI2CSensors;
   }
   if (config.enableMCP3424 !== undefined) {
     document.getElementById('enableMCP3424Chart').checked = config.enableMCP3424;
   }
   if (config.enableADS1110 !== undefined) {
     document.getElementById('enableADS1110Chart').checked = config.enableADS1110;
   }
   if (config.enableSolarSensor !== undefined) {
     document.getElementById('enableSolarSensorChart').checked = config.enableSolarSensor;
   }
   if (config.enableOPCN3Sensor !== undefined) {
     document.getElementById('enableOPCN3SensorChart').checked = config.enableOPCN3Sensor;
   }
   if (config.enableIPS !== undefined) {
     document.getElementById('enableIPSChart').checked = config.enableIPS;
   }
   if (config.enableIPSDebug !== undefined) {
     document.getElementById('enableIPSDebugChart').checked = config.enableIPSDebug;
   }
  
  // Komunikacja
  if (config.enableWiFi !== undefined) {
    document.getElementById('enableWiFiChart').checked = config.enableWiFi;
  }
  if (config.enableWebServer !== undefined) {
    document.getElementById('enableWebServerChart').checked = config.enableWebServer;
  }
  if (config.enableModbus !== undefined) {
    document.getElementById('enableModbusChart').checked = config.enableModbus;
  }
  
  // Powiadomienia
  if (config.enablePushbullet !== undefined) {
    document.getElementById('enablePushbulletChart').checked = config.enablePushbullet;
  }
  if (config.pushbulletToken !== undefined) {
    document.getElementById('pushbulletTokenChart').value = config.pushbulletToken;
  }
  
     // Tryb Pracy i Funkcje
   if (config.lowPowerMode !== undefined) {
     document.getElementById('lowPowerModeChart').checked = config.lowPowerMode;
   }
   if (config.autoReset !== undefined) {
     document.getElementById('autoResetChart').checked = config.autoReset;
   }
   if (config.useAveragedData !== undefined) {
     document.getElementById('useAveragedDataChart').checked = config.useAveragedData;
   }
   if (config.enableFan !== undefined) {
     document.getElementById('enableFanChart').checked = config.enableFan;
   }
}

function saveAllConfigChart() {
  console.log('Saving all configuration...');
  
     const config = {
     // Historia i Dane
     enableHistory: document.getElementById('enableHistoryChart').checked,
     
     // Czujniki Podstawowe
     enableSPS30: document.getElementById('enableSPS30Chart').checked,
     enableSHT40: document.getElementById('enableSHT40Chart').checked,
     enableSHT30: document.getElementById('enableSHT30Chart').checked,
     enableBME280: document.getElementById('enableBME280Chart').checked,
     enableSCD41: document.getElementById('enableSCD41Chart').checked,
     enableHCHO: document.getElementById('enableHCHOChart').checked,
     enableINA219: document.getElementById('enableINA219Chart').checked,
     
     // Czujniki Zaawansowane
     enableI2CSensors: document.getElementById('enableI2CSensorsChart').checked,
     enableMCP3424: document.getElementById('enableMCP3424Chart').checked,
     enableADS1110: document.getElementById('enableADS1110Chart').checked,
     enableSolarSensor: document.getElementById('enableSolarSensorChart').checked,
     enableOPCN3Sensor: document.getElementById('enableOPCN3SensorChart').checked,
     enableIPS: document.getElementById('enableIPSChart').checked,
     enableIPSDebug: document.getElementById('enableIPSDebugChart').checked,
     
     // Komunikacja
     enableWiFi: document.getElementById('enableWiFiChart').checked,
     enableWebServer: document.getElementById('enableWebServerChart').checked,
     enableModbus: document.getElementById('enableModbusChart').checked,
     
     // Powiadomienia
     enablePushbullet: document.getElementById('enablePushbulletChart').checked,
     pushbulletToken: document.getElementById('pushbulletTokenChart').value,
     
     // Tryb Pracy i Funkcje
     lowPowerMode: document.getElementById('lowPowerModeChart').checked,
     autoReset: document.getElementById('autoResetChart').checked,
     useAveragedData: document.getElementById('useAveragedDataChart').checked,
     enableFan: document.getElementById('enableFanChart').checked
   };
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    const message = Object.assign({cmd: 'setConfig'}, config);
    ws.send(JSON.stringify(message));
    
    // Visual feedback
    const button = document.querySelector('button[onclick="saveAllConfigChart()"]');
    const originalText = button.innerHTML;
    button.innerHTML = '✅ Zapisano!';
    button.style.background = 'linear-gradient(45deg, #27ae60, #2ecc71)';
    
    setTimeout(() => {
      button.innerHTML = originalText;
      button.style.background = 'linear-gradient(45deg, #3498db, #2980b9)';
    }, 2000);
    
  } else {
    console.error('WebSocket not connected for config save');
    alert('Błąd: Brak połączenia WebSocket');
  }
}

function testPushbulletChart() {
  console.log('Testing Pushbullet from chart page...');
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      cmd: 'system',
      command: 'pushbulletTest'
    }));
    
    // Visual feedback
    const button = document.querySelector('button[onclick="testPushbulletChart()"]');
    const originalText = button.innerHTML;
    button.innerHTML = '📤 Wysyłanie...';
    
    setTimeout(() => {
      button.innerHTML = originalText;
    }, 3000);
  } else {
    alert('Brak połączenia WebSocket');
  }
}

function testBatteryNotificationChart() {
  console.log('Testing battery notification from chart page...');
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      cmd: 'system',
      command: 'pushbulletBatteryTest'
    }));
    
    // Visual feedback
    const button = document.querySelector('button[onclick="testBatteryNotificationChart()"]');
    const originalText = button.innerHTML;
    button.innerHTML = '📤 Wysyłanie...';
    
    setTimeout(() => {
      button.innerHTML = originalText;
    }, 3000);
  } else {
    alert('Brak połączenia WebSocket');
  }
}

function restartSystemChart() {
  if (confirm('Czy na pewno chcesz zrestartować system?')) {
    console.log('Restarting system from chart page...');
    
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({
        cmd: 'system',
        command: 'restart'
      }));
      
      // Visual feedback
      const button = document.querySelector('button[onclick="restartSystemChart()"]');
      button.innerHTML = '🔄 Restartowanie...';
      button.disabled = true;
      
      // Show countdown
      setTimeout(() => {
        alert('System zostanie zrestartowany za 3 sekundy...');
      }, 1000);
      
    } else {
      alert('Brak połączenia WebSocket');
    }
  }
}

function testBatteryNotification() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      cmd: "system",
      command: "pushbulletBatteryTest"
    }));
  }
}

function handleHistoryResponse(data) {
  console.log('Handling history response:', data);
  console.log('Data success:', data.success);
  console.log('Data error:', data.error);
  console.log('Data data:', data.data);
  console.log('Data samples:', data.samples);
  console.log('pendingHistoryRequest before check:', pendingHistoryRequest);
  
  if (pendingHistoryRequest) {
    const { sensorType, timeRange, sampleType, container } = pendingHistoryRequest;
    pendingHistoryRequest = null;
    
    console.log('Pending request:', { sensorType, timeRange, sampleType });
    console.log('Container:', container);
    
    if (data.error) {
      console.log('History error detected:', data.error);
      container.innerHTML = `<div class="no-data">❌ Błąd: ${data.error}</div>`;
      return;
    }
    
    if (!data.data || data.data.length === 0) {
      console.log('No data array or empty data');
      container.innerHTML = '<div class="no-data">📭 Brak danych dla wybranego czujnika</div>';
      return;
    }
    
         // Debug: sprawdź strukturę danych
     console.log('Data received:', data.data.length, 'samples');
     if (data.data.length > 0) {
       console.log('First sample structure:', data.data[0]);
       console.log('Sample keys:', Object.keys(data.data[0]));
       if (data.data[0].data) {
         console.log('Sample data keys:', Object.keys(data.data[0].data));
       }
       console.log('Last sample structure:', data.data[data.data.length - 1]);
     }
    
    // Create charts based on sensor type
    if (sensorType === 'sps30') {
      createSPS30Charts(data.data);
    } else if (sensorType === 'ips') {
      createIPSCharts(data.data);
    } else if (sensorType === 'power') {
      createPowerCharts(data.data);
    } else if (sensorType === 'sht40') {
      createSHT40Charts(data.data);
    } else if (sensorType === 'co2') {
      createCO2Charts(data.data);
    } else if (sensorType === 'hcho') {
      createHCHOCharts(data.data);
    } else if (sensorType === 'battery') {
      createBatteryCharts(data.data);
    } else if (sensorType === 'calibration') {
      createCalibrationCharts(data.data);
    } else if (sensorType === 'fan') {
      createFanCharts(data.data);
    }
    
    // Remove loading message
    const loading = container.querySelector('.loading');
    if (loading) loading.remove();
    
    // Show stats
    const statsDiv = document.createElement('div');
    statsDiv.className = 'chart-info';
    statsDiv.style.textAlign = 'center';
    statsDiv.style.color = 'white';
    statsDiv.style.marginTop = '20px';
    const sampleTypeText = sampleType === 'fast' ? 'szybkich (10s)' : 'wolnych (5min)';
    statsDiv.innerHTML = `📊 Załadowano ${data.samples} próbek ${sampleTypeText} z ${sensorType}`;
    container.appendChild(statsDiv);
    
    console.log('Charts created successfully');
  } else {
    console.log('No pending request found - response ignored');
  }
}

function createChart(containerId, title, datasets) {
  const canvas = document.createElement('canvas');
  document.getElementById(containerId).appendChild(canvas);
  
  const ctx = canvas.getContext('2d');
  return new Chart(ctx, {
    type: 'line',
    data: {
      datasets: datasets
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        x: {
          type: 'time',
          time: {
            unit: 'minute',
            displayFormats: {
              minute: 'HH:mm'
            }
          },
          title: {
            display: true,
            text: 'Czas'
          }
        },
        y: {
          beginAtZero: false,
          title: {
            display: true,
            text: 'Wartość'
          }
        }
      },
      plugins: {
        title: {
          display: true,
          text: title,
          font: {
            size: 16
          }
        },
        legend: {
          display: true,
          position: 'top'
        }
      },
      interaction: {
        intersect: false,
        mode: 'index'
      }
    }
  });
}

function formatChartData(historyData, valueKey, label, color) {
  if (!historyData || !historyData.length) return [];
  
  console.log(`Formatting chart data for key: ${valueKey}, entries: ${historyData.length}`);
  console.log('First entry:', historyData[0]);
  console.log('Last entry:', historyData[historyData.length - 1]);
  
  const formattedData = historyData.map((entry, index) => {
    // Use the timestamp directly from the entry
    let timestamp = entry.timestamp;
    
    // If timestamp looks like millis() (small number), convert to proper epoch time
    if (timestamp < 1600000000000) {
      // Convert millis to epoch time - assume it's system uptime
      const now = Date.now();
      const systemUptimeMs = timestamp;
      timestamp = now - (Date.now() % 1000000000) + systemUptimeMs;
    }
    
    // Get the value from data object
    const value = entry.data ? entry.data[valueKey] : entry[valueKey];
    const parsedValue = parseFloat(value);
    
    console.log(`Entry ${index} for ${valueKey}:`, {
      originalTimestamp: entry.timestamp,
      convertedTimestamp: timestamp,
      dateTime: entry.dateTime,
      value: value,
      parsedValue: parsedValue
    });
    
    return {
      x: new Date(timestamp),
      y: isNaN(parsedValue) ? null : parsedValue
    };
  }).filter(point => point.y !== null && point.y !== undefined);
  
  // Sort by timestamp to ensure proper order
  formattedData.sort((a, b) => a.x.getTime() - b.x.getTime());
  
  console.log(`Formatted ${formattedData.length} valid points for ${valueKey}`);
  return formattedData;
}



async function updateCharts() {
  const timeRange = document.getElementById('timeRange').value;
  const sampleType = document.getElementById('sampleType').value;
  const sensorType = document.getElementById('sensorType').value;
  const container = document.getElementById('charts-container');
  
  console.log('=== updateCharts called ===');
  console.log('Updating charts:', { timeRange, sampleType, sensorType });
  console.log('Container element:', container);
  console.log('Previous pendingHistoryRequest:', pendingHistoryRequest);
  
  // Don't update if there's a pending request
  if (pendingHistoryRequest) {
    console.log('Skipping update - pending request exists');
    return;
  }
  
  // Clear existing charts
  Object.values(charts).forEach(chart => chart.destroy());
  charts = {};
  container.innerHTML = '<div class="loading">📊 Ładowanie danych z ' + sensorType + '...</div>';
  
  // Store pending request
  pendingHistoryRequest = { sensorType, timeRange, sampleType, container };
  console.log('Set pendingHistoryRequest:', pendingHistoryRequest);
  
  // Send WebSocket command
  if (ws && ws.readyState === WebSocket.OPEN) {
    const command = {
      cmd: 'getHistory',
      sensor: sensorType,
      timeRange: timeRange,
      sampleType: sampleType
    };
    console.log('Sending WebSocket command:', command);
    ws.send(JSON.stringify(command));
    
    // Set timeout to reset pending request if no response
    setTimeout(() => {
      if (pendingHistoryRequest) {
        console.log('Timeout - resetting pending request');
        pendingHistoryRequest = null;
        container.innerHTML = '<div class="no-data">⏰ Timeout - brak odpowiedzi</div>';
      }
    }, 10000); // 10 second timeout
  } else {
    console.log('WebSocket not connected, state:', ws ? ws.readyState : 'no ws');
    container.innerHTML = '<div class="no-data">❌ Brak połączenia WebSocket</div>';
    pendingHistoryRequest = null; // Reset on connection error
  }
}

function createSPS30Charts(data) {
  const container = document.getElementById('charts-container');
  
  // PM Mass Concentration - tylko PM2.5 i PM10
  const pmCard = document.createElement('div');
  pmCard.className = 'chart-card';
  pmCard.innerHTML = `
    <div class="chart-title">🌫️ Stężenie Pyłu (SPS30)</div>
    <div class="chart-container" id="pm-chart"></div>
    <div class="chart-info">PM2.5, PM10 (µg/m³)</div>
  `;
  container.appendChild(pmCard);
  
  charts.pm = createChart('pm-chart', 'Stężenie Pyłu', [
    {
      label: 'PM2.5 (µg/m³)',
      data: formatChartData(data, 'PM25'),
      borderColor: '#f39c12',
      backgroundColor: '#f39c1220',
      tension: 0.1
    },
    {
      label: 'PM10 (µg/m³)',
      data: formatChartData(data, 'PM10'),
      borderColor: '#d35400',
      backgroundColor: '#d3540020',
      tension: 0.1
    }
  ]);
}

function createIPSCharts(data) {
  const container = document.getElementById('charts-container');
  
  // PM Mass Concentration
  const pmCard = document.createElement('div');
  pmCard.className = 'chart-card';
  pmCard.innerHTML = `
    <div class="chart-title">🌫️ Stężenie Pyłu (IPS)</div>
    <div class="chart-container" id="ips-pm-chart"></div>
    <div class="chart-info">PM0.1, PM0.3, PM0.5, PM1.0, PM2.5, PM5.0, PM10 (µg/m³)</div>
  `;
  container.appendChild(pmCard);
  
  charts.ipsPm = createChart('ips-pm-chart', 'Stężenie Pyłu IPS', [
    {
      label: 'PM0.1 (µg/m³)',
      data: formatChartData(data, 'pm_1'),
      borderColor: '#e74c3c',
      backgroundColor: '#e74c3c20',
      tension: 0.1
    },
    {
      label: 'PM0.3 (µg/m³)',
      data: formatChartData(data, 'pm_2'),
      borderColor: '#f39c12',
      backgroundColor: '#f39c1220',
      tension: 0.1
    },
    {
      label: 'PM0.5 (µg/m³)',
      data: formatChartData(data, 'pm_3'),
      borderColor: '#e67e22',
      backgroundColor: '#e67e2220',
      tension: 0.1
    },
    {
      label: 'PM1.0 (µg/m³)',
      data: formatChartData(data, 'pm_4'),
      borderColor: '#d35400',
      backgroundColor: '#d3540020',
      tension: 0.1
    },
    {
      label: 'PM2.5 (µg/m³)',
      data: formatChartData(data, 'pm_5'),
      borderColor: '#8e44ad',
      backgroundColor: '#8e44ad20',
      tension: 0.1
    },
    {
      label: 'PM5.0 (µg/m³)',
      data: formatChartData(data, 'pm_6'),
      borderColor: '#2c3e50',
      backgroundColor: '#2c3e5020',
      tension: 0.1
    },
    {
      label: 'PM10 (µg/m³)',
      data: formatChartData(data, 'pm_7'),
      borderColor: '#34495e',
      backgroundColor: '#34495e20',
      tension: 0.1
    }
  ]);
  
  // Particle Count
  const pcCard = document.createElement('div');
  pcCard.className = 'chart-card';
  pcCard.innerHTML = `
    <div class="chart-title">🔢 Liczba Cząstek (IPS)</div>
    <div class="chart-container" id="ips-pc-chart"></div>
    <div class="chart-info">PC0.1, PC0.3, PC0.5, PC1.0, PC2.5, PC5.0, PC10 (#/cm³)</div>
  `;
  container.appendChild(pcCard);
  
  charts.ipsPc = createChart('ips-pc-chart', 'Liczba Cząstek IPS', [
    {
      label: 'PC0.1 (#/cm³)',
      data: formatChartData(data, 'pc_1'),
      borderColor: '#3498db',
      backgroundColor: '#3498db20',
      tension: 0.1
    },
    {
      label: 'PC0.3 (#/cm³)',
      data: formatChartData(data, 'pc_2'),
      borderColor: '#2980b9',
      backgroundColor: '#2980b920',
      tension: 0.1
    },
    {
      label: 'PC0.5 (#/cm³)',
      data: formatChartData(data, 'pc_3'),
      borderColor: '#1abc9c',
      backgroundColor: '#1abc9c20',
      tension: 0.1
    },
    {
      label: 'PC1.0 (#/cm³)',
      data: formatChartData(data, 'pc_4'),
      borderColor: '#16a085',
      backgroundColor: '#16a08520',
      tension: 0.1
    },
    {
      label: 'PC2.5 (#/cm³)',
      data: formatChartData(data, 'pc_5'),
      borderColor: '#27ae60',
      backgroundColor: '#27ae6020',
      tension: 0.1
    },
    {
      label: 'PC5.0 (#/cm³)',
      data: formatChartData(data, 'pc_6'),
      borderColor: '#2ecc71',
      backgroundColor: '#2ecc7120',
      tension: 0.1
    },
    {
      label: 'PC10 (#/cm³)',
      data: formatChartData(data, 'pc_7'),
      borderColor: '#55a3ff',
      backgroundColor: '#55a3ff20',
      tension: 0.1
    }
  ]);
}

function createSHT40Charts(data) {
  const container = document.getElementById('charts-container');
  
  const sht40Card = document.createElement('div');
  sht40Card.className = 'chart-card';
  sht40Card.innerHTML = `
    <div class="chart-title">🌡️ SHT40 (Temperatura/Wilgotność)</div>
    <div class="chart-container" id="sht40-chart"></div>
    <div class="chart-info">Temperatura (°C) i Wilgotność (%)</div>
  `;
  container.appendChild(sht40Card);
  
  charts.sht40 = createChart('sht40-chart', 'SHT40 - Temperatura i Wilgotność', [
    {
      label: 'Temperatura (°C)',
      data: formatChartData(data, 'temperature'),
      borderColor: '#e74c3c',
      backgroundColor: '#e74c3c20',
      tension: 0.1
    },
    {
      label: 'Wilgotność (%)',
      data: formatChartData(data, 'humidity'),
      borderColor: '#3498db',
      backgroundColor: '#3498db20',
      tension: 0.1
    },
    {
      label: 'Ciśnienie (kPa)',
      data: formatChartData(data, 'pressure'),
      borderColor: '#9b59b6',
      backgroundColor: '#9b59b620',
      tension: 0.1
    }
  ]);
}

function createCO2Charts(data) {
  const container = document.getElementById('charts-container');
  
  const co2Card = document.createElement('div');
  co2Card.className = 'chart-card';
  co2Card.innerHTML = `
    <div class="chart-title">🌬️ CO2 (SCD41)</div>
    <div class="chart-container" id="co2-chart"></div>
    <div class="chart-info">Stężenie CO2 (ppm)</div>
  `;
  container.appendChild(co2Card);
  
  charts.co2 = createChart('co2-chart', 'CO2 - Stężenie CO2', [
    {
      label: 'CO2 (ppm)',
      data: formatChartData(data, 'co2'),
      borderColor: '#27ae60',
      backgroundColor: '#27ae6020',
      tension: 0.1
    }
  ]);
}

function createCalibrationCharts(data) {
  const container = document.getElementById('charts-container');
  
  // Gases (ug/m3)
  const gasesCard = document.createElement('div');
  gasesCard.className = 'chart-card';
  gasesCard.innerHTML = `
    <div class="chart-title">🧪 Gazy (µg/m³)</div>
    <div class="chart-container" id="gases-chart"></div>
    <div class="chart-info">CO, NO, NO2, O3, SO2, H2S, NH3</div>
  `;
  container.appendChild(gasesCard);
  
  charts.gases = createChart('gases-chart', 'Stężenie Gazów (µg/m³)', [
    {
      label: 'CO (µg/m³)',
      data: formatChartData(data, 'CO'),
      borderColor: '#e74c3c',
      backgroundColor: '#e74c3c20',
      tension: 0.1
    },
    {
      label: 'NO (µg/m³)',
      data: formatChartData(data, 'NO'),
      borderColor: '#f39c12',
      backgroundColor: '#f39c1220',
      tension: 0.1
    },
    {
      label: 'NO2 (µg/m³)',
      data: formatChartData(data, 'NO2'),
      borderColor: '#e67e22',
      backgroundColor: '#e67e2220',
      tension: 0.1
    },
    {
      label: 'O3 (µg/m³)',
      data: formatChartData(data, 'O3'),
      borderColor: '#3498db',
      backgroundColor: '#3498db20',
      tension: 0.1
    },
    {
      label: 'SO2 (µg/m³)',
      data: formatChartData(data, 'SO2'),
      borderColor: '#9b59b6',
      backgroundColor: '#9b59b620',
      tension: 0.1
    },
    {
      label: 'H2S (µg/m³)',
      data: formatChartData(data, 'H2S'),
      borderColor: '#2c3e50',
      backgroundColor: '#2c3e5020',
      tension: 0.1
    },
    {
      label: 'NH3 (µg/m³)',
      data: formatChartData(data, 'NH3'),
      borderColor: '#27ae60',
      backgroundColor: '#27ae6020',
      tension: 0.1
    }
  ]);
  
  // Special sensors
  const specialCard = document.createElement('div');
  specialCard.className = 'chart-card';
  specialCard.innerHTML = `
    <div class="chart-title">🔬 Czujniki Specjalne</div>
    <div class="chart-container" id="special-chart"></div>
    <div class="chart-info">HCHO, PID, VOC</div>
  `;
  container.appendChild(specialCard);
  
  charts.special = createChart('special-chart', 'Czujniki Specjalne', [
    {
      label: 'HCHO (ppb)',
      data: formatChartData(data, 'HCHO'),
      borderColor: '#8e44ad',
      backgroundColor: '#8e44ad20',
      tension: 0.1
    },
    {
      label: 'PID',
      data: formatChartData(data, 'PID'),
      borderColor: '#d35400',
      backgroundColor: '#d3540020',
      tension: 0.1
    },
    {
      label: 'VOC (µg/m³)',
      data: formatChartData(data, 'VOC'),
      borderColor: '#16a085',
      backgroundColor: '#16a08520',
      tension: 0.1
    }
  ]);
}

function createPowerCharts(data) {
  const container = document.getElementById('charts-container');
  
  const powerCard = document.createElement('div');
  powerCard.className = 'chart-card';
  powerCard.innerHTML = `
    <div class="chart-title">⚡ Monitor Mocy (INA219)</div>
    <div class="chart-container" id="power-chart"></div>
    <div class="chart-info">Napięcie, prąd i moc</div>
  `;
  container.appendChild(powerCard);
  
  charts.power = createChart('power-chart', 'Parametry Elektryczne', [
    {
      label: 'Bus Voltage (V)',
      data: formatChartData(data, 'busVoltage'),
      borderColor: '#f39c12',
      backgroundColor: '#f39c1220',
      tension: 0.1
    },
    {
      label: 'Current (mA)',
      data: formatChartData(data, 'current'),
      borderColor: '#e67e22',
      backgroundColor: '#e67e2220',
      tension: 0.1
    },
    {
      label: 'Power (mW)',
      data: formatChartData(data, 'power'),
      borderColor: '#27ae60',
      backgroundColor: '#27ae6020',
      tension: 0.1
    }
  ]);
}

function createHCHOCharts(data) {
  const container = document.getElementById('charts-container');
  
  const hchoCard = document.createElement('div');
  hchoCard.className = 'chart-card';
  hchoCard.innerHTML = `
    <div class="chart-title">🧪 Czujnik HCHO</div>
    <div class="chart-container" id="hcho-chart"></div>
    <div class="chart-info">Formaldehyd i VOC</div>
  `;
  container.appendChild(hchoCard);
  
  charts.hcho = createChart('hcho-chart', 'HCHO', [
    {
      label: 'HCHO (mg/m³)',
      data: formatChartData(data, 'hcho_mg'),
      borderColor: '#8e44ad',
      backgroundColor: '#8e44ad20',
      tension: 0.1
    },
    {
      label: 'HCHO (ppb)',
      data: formatChartData(data, 'hcho_ppb'),
      borderColor: '#d35400',
      backgroundColor: '#d3540020',
      tension: 0.1
    }
  ]);
}

function createBatteryCharts(data) {
  const container = document.getElementById('charts-container');
  
  const batteryCard = document.createElement('div');
  batteryCard.className = 'chart-card';
  batteryCard.innerHTML = `
    <div class="chart-title">🔋 Battery Monitor</div>
    <div class="chart-container" id="battery-chart"></div>
    <div class="chart-info">Voltage (V), Current (mA), Charge (%)</div>
  `;
  container.appendChild(batteryCard);
  
  charts.battery = createChart('battery-chart', 'Battery Status', [
    {
      label: 'Voltage (V)',
      data: formatChartData(data, 'voltage'),
      borderColor: '#4caf50',
      backgroundColor: '#4caf5020',
      tension: 0.1
    },
    {
      label: 'Current (mA)',
      data: formatChartData(data, 'current'),
      borderColor: '#ff9800',
      backgroundColor: '#ff980020',
      tension: 0.1,
      yAxisID: 'y1'
    },
    {
      label: 'Charge (%)',
      data: formatChartData(data, 'chargePercent'),
      borderColor: '#2196f3',
      backgroundColor: '#2196f320',
      tension: 0.1,
      yAxisID: 'y2'
    }
  ]);
}

function createFanCharts(data) {
  const container = document.getElementById('charts-container');
  
  // Fan Speed & RPM
  const fanCard = document.createElement('div');
  fanCard.className = 'chart-card';
  fanCard.innerHTML = `
    <div class="chart-title">💨 Sterowanie Wentylatorem</div>
    <div class="chart-container" id="fan-chart"></div>
    <div class="chart-info">Prędkość PWM i RPM</div>
  `;
  container.appendChild(fanCard);
  
  charts.fan = createChart('fan-chart', 'Parametry Wentylatora', [
    {
      label: 'Duty Cycle (%)',
      data: formatChartData(data, 'dutyCycle'),
      borderColor: '#3498db',
      backgroundColor: '#3498db20',
      tension: 0.1,
      yAxisID: 'y'
    },
    {
      label: 'RPM',
      data: formatChartData(data, 'rpm'),
      borderColor: '#e74c3c',
      backgroundColor: '#e74c3c20',
      tension: 0.1,
      yAxisID: 'y1'
    }
  ]);
  
  // GLine Router Status
  const glineCard = document.createElement('div');
  glineCard.className = 'chart-card';
  glineCard.innerHTML = `
    <div class="chart-title">🔌 Status Routera GLine</div>
    <div class="chart-container" id="gline-chart"></div>
    <div class="chart-info">Włączony/Wyłączony</div>
  `;
  container.appendChild(glineCard);
  
  charts.gline = createChart('gline-chart', 'Status GLine Router', [
    {
      label: 'GLine Enabled',
      data: formatChartData(data, 'glineEnabled'),
      borderColor: '#2ecc71',
      backgroundColor: '#2ecc7120',
      tension: 0.1,
      stepped: true
    }
  ]);
}

function toggleAutoUpdate() {
  autoUpdate = !autoUpdate;
  const button = document.getElementById('auto-status');
  
  if (autoUpdate) {
    button.textContent = 'ON';
    button.style.color = '#27ae60';
    autoUpdateInterval = setInterval(updateCharts, 30000);
  } else {
    button.textContent = 'OFF';
    button.style.color = '#e74c3c';
    if (autoUpdateInterval) {
      clearInterval(autoUpdateInterval);
    }
  }
}

// Initialize
window.addEventListener('load', function() {
  connectWebSocket();
  // Don't auto-load charts, wait for user selection
});

window.addEventListener('beforeunload', function() {
  if (ws) ws.close();
  if (autoUpdateInterval) clearInterval(autoUpdateInterval);
  Object.values(charts).forEach(chart => chart.destroy());
});
</script>
</body>
</html>
)rawliteral";

#endif // CHARTS_HTML_H 