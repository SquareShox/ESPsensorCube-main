#ifndef UPDATE_HTML_H
#define UPDATE_HTML_H
const char *update_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP Sensor Cube - Panel Sterowania</title>
<style>
  * {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
  }

  body {
    font-family: 'Arial', sans-serif;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    min-height: 100vh;
    padding: 20px;
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

  .header p {
    font-size: 1.2em;
    opacity: 0.9;
  }

  .nav {
    text-align: center;
    margin-bottom: 30px;
  }

  .nav a {
    color: white;
    text-decoration: none;
    margin: 0 15px;
    padding: 12px 25px;
    border: 2px solid white;
    border-radius: 25px;
    transition: all 0.3s ease;
    font-weight: bold;
  }

  .nav a:hover {
    background-color: white;
    color: #667eea;
    transform: translateY(-2px);
    box-shadow: 0 5px 15px rgba(0,0,0,0.2);
  }

  .main-container {
    max-width: 1200px;
    margin: 0 auto;
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
    gap: 25px;
  }

  .control-card {
    background: rgba(255, 255, 255, 0.95);
    border-radius: 20px;
    padding: 30px;
    box-shadow: 0 10px 30px rgba(0,0,0,0.2);
    transition: transform 0.3s ease;
    text-align: center;
  }

  .control-card:hover {
    transform: translateY(-5px);
  }

  .card-header {
    display: flex;
    align-items: center;
    justify-content: center;
    margin-bottom: 25px;
    padding-bottom: 15px;
    border-bottom: 3px solid #f0f0f0;
  }

  .card-icon {
    width: 50px;
    height: 50px;
    border-radius: 50%;
    margin-right: 15px;
    display: flex;
    align-items: center;
    justify-content: center;
    color: white;
    font-weight: bold;
    font-size: 24px;
  }

  .update-icon { background: linear-gradient(45deg, #ff9800, #ffc107); }
  .system-icon { background: linear-gradient(45deg, #2196f3, #03a9f4); }
  .dashboard-icon { background: linear-gradient(45deg, #4caf50, #8bc34a); }
  .charts-icon { background: linear-gradient(45deg, #9c27b0, #e91e63); }
  .notification-icon { background: linear-gradient(45deg, #e91e63, #f06292); }

  .card-title {
    font-size: 1.4em;
    font-weight: bold;
    color: #333;
  }

  .card-description {
    color: #666;
    margin-bottom: 25px;
    line-height: 1.5;
  }

  .file-input-container {
    position: relative;
    margin-bottom: 20px;
  }

  .file-input {
    width: 100%;
    padding: 15px;
    border: 2px dashed #ddd;
    border-radius: 10px;
    background: #f9f9f9;
    cursor: pointer;
    transition: all 0.3s ease;
  }

  .file-input:hover {
    border-color: #667eea;
    background: #f0f8ff;
  }

  .file-input:focus {
    outline: none;
    border-color: #667eea;
    box-shadow: 0 0 10px rgba(102, 126, 234, 0.3);
  }

  .btn {
    background: linear-gradient(45deg, #667eea, #764ba2);
    color: white;
    border: none;
    border-radius: 25px;
    padding: 15px 30px;
    font-size: 1.1em;
    font-weight: bold;
    cursor: pointer;
    transition: all 0.3s ease;
    text-decoration: none;
    display: inline-block;
    margin: 10px;
    min-width: 200px;
  }

  .btn:hover {
    transform: translateY(-2px);
    box-shadow: 0 8px 25px rgba(0,0,0,0.3);
  }

  .btn-danger {
    background: linear-gradient(45deg, #f44336, #ff5722);
  }

  .btn-success {
    background: linear-gradient(45deg, #4caf50, #8bc34a);
  }

  .btn-warning {
    background: linear-gradient(45deg, #ff9800, #ffc107);
  }

  .status-indicator {
    width: 12px;
    height: 12px;
    border-radius: 50%;
    display: inline-block;
    margin-left: 10px;
    animation: pulse 2s infinite;
  }

  .status-ok { background-color: #4caf50; }
  .status-error { background-color: #f44336; }

  @keyframes pulse {
    0% { opacity: 1; }
    50% { opacity: 0.5; }
    100% { opacity: 1; }
  }

  .status-text {
    font-weight: bold;
    margin-top: 15px;
    padding: 10px;
    border-radius: 10px;
    background: #f8f9fa;
  }

  .progress-bar {
    width: 100%;
    height: 20px;
    background: #f0f0f0;
    border-radius: 10px;
    overflow: hidden;
    margin: 15px 0;
    display: none;
  }

  .progress-fill {
    height: 100%;
    background: linear-gradient(45deg, #4caf50, #8bc34a);
    width: 0%;
    transition: width 0.3s ease;
  }

  @media (max-width: 768px) {
    .main-container {
      grid-template-columns: 1fr;
    }
    
    .header h1 {
      font-size: 2em;
    }
    
    .nav a {
      display: block;
      margin: 10px 0;
    }
  }
</style>
<script src="/common.js"></script>
</head>
<body>
  <div class="header">
    <h1>🔧 ESP Sensor Cube - Panel Sterowania</h1>
    <p>Zarządzanie systemem i aktualizacje</p>
  </div>

  <div class="nav">
    <a href="/">🔧 Panel Sterowania</a>
    <a href="/dashboard">📊 Dashboard</a>
    <a href="/charts">📈 Wykresy</a>
    <a href="/network">🌐 Sieć</a>
  </div>

  <div class="main-container">
    <!-- Firmware Update Card -->
    <div class="control-card">
      <div class="card-header">
        <div class="card-icon update-icon">📦</div>
        <div class="card-title">Aktualizacja Firmware</div>
      </div>
      <div class="card-description">
        Wybierz plik .bin z nową wersją firmware do aktualizacji systemu
      </div>
      
      <form method="POST" action="/update" enctype="multipart/form-data" id="updateForm">
        <div class="file-input-container">
          <input type="file" name="update" class="file-input" accept=".bin" id="firmwareFile">
        </div>
        
        <button type="submit" class="btn btn-warning" id="updateBtn" disabled>
          📤 Aktualizuj Firmware
        </button>
</form>

      <div class="progress-bar" id="progressBar">
        <div class="progress-fill" id="progressFill"></div>
</div>

      <div class="status-text" id="updateStatus">
        Status: Gotowy do aktualizacji
      </div>
</div>

    <!-- System Control Card -->
    <div class="control-card">
      <div class="card-header">
        <div class="card-icon system-icon">⚙️</div>
        <div class="card-title">Kontrola Systemu</div>
</div>
      <div class="card-description">
        Zarządzanie ustawieniami systemu i kontrolą automatycznego resetu
      </div>
      
      <button class="btn btn-success" id="toggleAutoReset">
        🔄 Toggle Auto Reset
      </button>
      
      <div class="status-text" id="autoResetState">
        Auto Reset: Sprawdzanie...
      </div>
      
      <button class="btn btn-danger" id="restartBtn">
        🔄 Restart Systemu
      </button>
      
      <div class="status-text" id="systemStatus">
        Status systemu: Sprawdzanie...
      </div>
    </div>

    <!-- Quick Access Card -->
    <div class="control-card">
      <div class="card-header">
        <div class="card-icon dashboard-icon">🚀</div>
        <div class="card-title">Szybki Dostęp</div>
      </div>
      <div class="card-description">
        Przejdź bezpośrednio do monitorowania i analizy danych
      </div>
      
      <a href="/dashboard" class="btn btn-success">
        📊 Live Dashboard
      </a>
      
      <a href="/charts" class="btn btn-success">
        📈 Wykresy Historyczne
      </a>
      
      <a href="/network" class="btn btn-warning">
        🌐 Konfiguracja Sieci
      </a>
      
      <a href="/mcp3424" class="btn btn-warning">
        🔌 Konfiguracja MCP3424
      </a>
      
      <div class="status-text">
        <span class="status-indicator status-ok"></span>
        System gotowy
      </div>
    </div>

    <!-- Pushbullet Settings Card -->
    <div class="control-card">
      <div class="card-header">
        <div class="card-icon notification-icon">📱</div>
        <div class="card-title">Powiadomienia Pushbullet</div>
      </div>
      <div class="card-description">
        Konfiguracja powiadomień push na telefon
      </div>
      
      <div class="setting-group">
        <label class="setting-label">
          <input type="checkbox" id="enablePushbullet" class="setting-checkbox">
          Włącz powiadomienia Pushbullet
        </label>
      </div>
      
      <div class="setting-group">
        <label class="setting-label">Token Pushbullet:</label>
        <input type="text" id="pushbulletToken" class="setting-input" placeholder="Wprowadź token Pushbullet">
      </div>
      
      <button class="btn btn-primary" onclick="savePushbulletConfig()">
        💾 Zapisz Ustawienia
      </button>
      
      <button class="btn btn-info" onclick="testPushbullet()">
        📤 Test Powiadomienia
      </button>
      
      <button class="btn btn-warning" onclick="testBatteryNotification()">
        🔋 Test Powiadomienia Baterii
      </button>
      
      <div class="status-text" id="pushbulletStatus">
        Status: Sprawdzanie...
      </div>
    </div>

    <!-- System Info Card -->
    <div class="control-card">
      <div class="card-header">
        <div class="card-icon charts-icon">ℹ️</div>
        <div class="card-title">Informacje Systemowe</div>
      </div>
      <div class="card-description">
        Aktualne informacje o stanie systemu i zasobach
      </div>
      
      <div class="status-text" id="systemInfo">
        <strong>Uptime:</strong> <span id="uptime">--</span><br>
        <strong>Free Heap:</strong> <span id="freeHeap">--</span> KB<br>
        <strong>WiFi Signal:</strong> <span id="wifiSignal">--</span> dBm<br>
        <strong>Last Update:</strong> <span id="lastUpdate">--</span>
      </div>
      
      <button class="btn btn-warning" id="refreshInfo">
        🔄 Odśwież Info
      </button>
    </div>

    <!-- System Configuration Card -->
    <div class="control-card">
      <div class="card-header">
        <div class="card-icon charts-icon">⚙️</div>
        <div class="card-title">Konfiguracja Systemu</div>
      </div>
      <div class="card-description">
        Szybka konfiguracja głównych funkcji systemu
      </div>
      
      <div class="config-sections">
        <!-- Historia i Dane -->
        <div class="config-section">
          <h4>📊 Historia i Dane</h4>
          <label class="config-checkbox">
            <input type="checkbox" id="enableHistory" onchange="updateSystemConfig('enableHistory', this.checked)">
            <span class="checkmark"></span>
            Włącz zapis historii danych
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="useAveragedData" onchange="updateSystemConfig('useAveragedData', this.checked)">
            <span class="checkmark"></span>
            Używaj uśrednionych danych
          </label>
        </div>
        
        <!-- Czujniki -->
        <div class="config-section">
          <h4>🔬 Czujniki</h4>
          <label class="config-checkbox">
            <input type="checkbox" id="enableSPS30" onchange="updateSystemConfig('enableSPS30', this.checked)">
            <span class="checkmark"></span>
            SPS30 (Pył)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableSHT40" onchange="updateSystemConfig('enableSHT40', this.checked)">
            <span class="checkmark"></span>
            SHT40 (Temperatura/Wilgotność)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableHCHO" onchange="updateSystemConfig('enableHCHO', this.checked)">
            <span class="checkmark"></span>
            HCHO (Formaldehyd)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableINA219" onchange="updateSystemConfig('enableINA219', this.checked)">
            <span class="checkmark"></span>
            INA219 (Monitoring zasilania)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableMCP3424" onchange="updateSystemConfig('enableMCP3424', this.checked)">
            <span class="checkmark"></span>
            MCP3424 (ADC)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableSolarSensor" onchange="updateSystemConfig('enableSolarSensor', this.checked)">
            <span class="checkmark"></span>
            Solar (Panel słoneczny)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableIPS" onchange="updateSystemConfig('enableIPS', this.checked)">
            <span class="checkmark"></span>
            IPS (Czujnik pyłu)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableI2CSensors" onchange="updateSystemConfig('enableI2CSensors', this.checked)">
            <span class="checkmark"></span>
            I2C Sensors (SCD41 CO2)
          </label>
        </div>
        
        <!-- Komunikacja -->
        <div class="config-section">
          <h4>📡 Komunikacja</h4>
          <label class="config-checkbox">
            <input type="checkbox" id="enableWiFi" onchange="updateSystemConfig('enableWiFi', this.checked)">
            <span class="checkmark"></span>
            WiFi
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableWebServer" onchange="updateSystemConfig('enableWebServer', this.checked)">
            <span class="checkmark"></span>
            Serwer WWW
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableModbus" onchange="updateSystemConfig('enableModbus', this.checked)">
            <span class="checkmark"></span>
            Modbus
          </label>
        </div>
        

        
        <!-- Tryb Pracy -->
        <div class="config-section">
          <h4>⚡ Tryb Pracy</h4>
          <label class="config-checkbox">
            <input type="checkbox" id="lowPowerModeMain" onchange="updateSystemConfig('lowPowerMode', this.checked)">
            <span class="checkmark"></span>
            Tryb niskiego poboru energii
          </label>
        </div>
        
        <!-- Akcje -->
        <div class="config-section">
          <h4>🎛️ Akcje</h4>
          <div class="config-buttons">
            <button class="btn btn-primary" onclick="saveAllConfigMain()">
              💾 Ustaw Konfigurację
            </button>
            <button class="btn btn-warning" onclick="loadSystemConfigMain()">
              🔄 Odśwież Konfigurację
            </button>
            <button class="btn btn-danger" onclick="restartSystemMain()">
              🔄 Restart Systemu
            </button>
          </div>
        </div>
      </div>
    </div>
  </div>

<script>
let ws;

function connectWebSocket() {
  ws = new WebSocket(`ws://${window.location.host}/ws`);
  ws.onopen = function() {
    console.log('WebSocket connected');
    updateSystemInfo();
    loadSystemConfigMain(); // Load configuration on connect
  };
  ws.onclose = function() {
    console.log('WebSocket disconnected, reconnecting...');
    setTimeout(connectWebSocket, 5000);
  };
  ws.onmessage = function(event) {
    try {
      const data = JSON.parse(event.data);
      console.log('WebSocket message received:', data.cmd, data);
      
      if (data.cmd === 'getConfig') {
        // Handle config response
        if (data.config) {
          updatePushbulletConfig(data.config);
          updateSystemConfigDisplay(data.config);
        }
      } else if (data.cmd === 'setConfig') {
        // Handle config update response
        if (data.success) {
          showAlert('Konfiguracja zaktualizowana pomyślnie', 'success');
          // Reload config to update display
          loadSystemConfigMain();
        } else {
          showAlert('Błąd aktualizacji konfiguracji: ' + (data.error || 'Nieznany błąd'), 'error');
        }
      } else {
        // Handle other messages
        updateSystemInfo(data);
      }
    } catch (error) {
      console.error('Error parsing WebSocket data:', error);
    }
  };
}

function updateSystemInfo(data = null) {
  if (data) {
    // Handle both old and new format
    const uptime = data.uptime || (data.data && data.data.uptime) || 0;
    const freeHeap = data.freeHeap || (data.data && data.data.freeHeap) || 0;
    const wifiSignal = data.wifiSignal || (data.data && data.data.wifiSignal) || 0;
    
    document.getElementById('uptime').textContent = formatUptime(uptime);
    document.getElementById('freeHeap').textContent = Math.round(freeHeap / 1024);
    document.getElementById('wifiSignal').textContent = wifiSignal;
    document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString('pl-PL');
    
    // Update low power mode status if available
    if (data.lowPowerMode !== undefined) {
      updateLowPowerModeStatus(data.lowPowerMode);
    } else if (data.data && data.data.config && data.data.config.lowPowerMode !== undefined) {
      updateLowPowerModeStatus(data.data.config.lowPowerMode);
    }
    
    // Update Pushbullet config if available
    if (data.enablePushbullet !== undefined || data.pushbulletToken !== undefined) {
      updatePushbulletConfig(data);
    }
  }
}

function formatUptime(seconds) {
  const days = Math.floor(seconds / 86400);
  const hours = Math.floor((seconds % 86400) / 3600);
  const mins = Math.floor((seconds % 3600) / 60);
  
  if (days > 0) return `${days}d ${hours}h`;
  if (hours > 0) return `${hours}h ${mins}m`;
  return `${mins}m`;
}

function updateAutoResetState(state) {
  const element = document.getElementById("autoResetState");
  const isEnabled = state === "true";
  element.innerHTML = `Auto Reset: <span style="color: ${isEnabled ? '#4caf50' : '#f44336'}">${isEnabled ? 'Włączony' : 'Wyłączony'}</span>`;
}

function updateLowPowerModeStatus(enabled) {
  const element = document.getElementById("lowPowerModeState");
  if (element) {
    element.innerHTML = `Low Power Mode: <span style="color: ${enabled ? '#ff9800' : '#4caf50'}">${enabled ? 'Włączony' : 'Wyłączony'}</span>`;
  }
}

function toggleAutoReset() {
  const xhr = new XMLHttpRequest();
  xhr.open("POST", "/toggleAutoReset", true);
  xhr.onreadystatechange = function() {
    if (xhr.readyState == 4 && xhr.status == 200) {
      updateAutoResetState(xhr.responseText);
    }
  }
  xhr.send();
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

function savePushbulletConfig() {
  const enablePushbullet = document.getElementById('enablePushbullet').checked;
  const pushbulletToken = document.getElementById('pushbulletToken').value;
  const statusElement = document.getElementById('pushbulletStatus');
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    const command = {
      cmd: "setConfig",
      enablePushbullet: enablePushbullet,
      pushbulletToken: pushbulletToken
    };
    
    console.log('Saving Pushbullet config:', command);
    ws.send(JSON.stringify(command));
    
    statusElement.innerHTML = 'Status: Zapisywanie...';
    
    // Listen for response
    const originalOnMessage = ws.onmessage;
    ws.onmessage = function(event) {
      try {
        const data = JSON.parse(event.data);
        if (data.cmd === 'setConfig') {
          if (data.success) {
            statusElement.innerHTML = 'Status: ✅ Ustawienia zapisane';
            showAlert('Ustawienia Pushbullet zostały zapisane', 'success');
          } else {
            statusElement.innerHTML = 'Status: ❌ Błąd zapisu';
            showAlert('Błąd zapisu ustawień: ' + (data.error || 'Nieznany błąd'), 'error');
          }
          ws.onmessage = originalOnMessage; // Restore original handler
        }
      } catch (error) {
        console.error('Error parsing response:', error);
        ws.onmessage = originalOnMessage; // Restore original handler
      }
    };
  } else {
    statusElement.innerHTML = 'Status: ❌ Brak połączenia WebSocket';
  }
}

function loadPushbulletConfig() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({cmd: "getConfig"}));
  }
}

function updatePushbulletConfig(data) {
  if (data.enablePushbullet !== undefined) {
    document.getElementById('enablePushbullet').checked = data.enablePushbullet;
  }
  if (data.pushbulletToken !== undefined) {
    document.getElementById('pushbulletToken').value = data.pushbulletToken;
  }
  
  const statusElement = document.getElementById('pushbulletStatus');
  if (data.enablePushbullet) {
    statusElement.innerHTML = 'Status: ✅ Powiadomienia włączone';
  } else {
    statusElement.innerHTML = 'Status: ⚠️ Powiadomienia wyłączone';
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

function testBatteryNotification() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      cmd: "system",
      command: "pushbulletBatteryTest"
    }));
  }
}

function restartSystem() {
  if (confirm('Czy na pewno chcesz zrestartować system?')) {
    const xhr = new XMLHttpRequest();
    xhr.open("POST", "/restart", true);
    xhr.send();
    document.getElementById('systemStatus').innerHTML = 'System restartuje się...';
  }
}

function refreshSystemInfo() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({cmd: "status"}));
  }
}

// System Configuration Functions
function updateSystemConfig(configKey, value) {
  console.log('Updating system config:', configKey, '=', value);
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    const config = {};
    config[configKey] = value;
    
    ws.send(JSON.stringify({
      cmd: 'setConfig',
      ...config
    }));
    
    // Show loading feedback
    showAlert(`Aktualizowanie ${configKey}...`, 'info');
  } else {
    showAlert('Brak połączenia WebSocket', 'error');
  }
}

function loadSystemConfigMain() {
  console.log('Loading system configuration...');
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({cmd: 'getConfig'}));
    showAlert('Ładowanie konfiguracji...', 'info');
  } else {
    showAlert('Brak połączenia WebSocket', 'error');
  }
}

function updateSystemConfigDisplay(config) {
  console.log('Updating system config display:', config);
  
  // Historia i Dane
  if (config.enableHistory !== undefined) {
    document.getElementById('enableHistory').checked = config.enableHistory;
  }
  if (config.useAveragedData !== undefined) {
    document.getElementById('useAveragedData').checked = config.useAveragedData;
  }
  
  // Czujniki
  if (config.enableSPS30 !== undefined) {
    document.getElementById('enableSPS30').checked = config.enableSPS30;
  }
  if (config.enableSHT40 !== undefined) {
    document.getElementById('enableSHT40').checked = config.enableSHT40;
  }
  if (config.enableHCHO !== undefined) {
    document.getElementById('enableHCHO').checked = config.enableHCHO;
  }
  if (config.enableINA219 !== undefined) {
    document.getElementById('enableINA219').checked = config.enableINA219;
  }
  if (config.enableMCP3424 !== undefined) {
    document.getElementById('enableMCP3424').checked = config.enableMCP3424;
  }
  if (config.enableSolarSensor !== undefined) {
    document.getElementById('enableSolarSensor').checked = config.enableSolarSensor;
  }
  if (config.enableIPS !== undefined) {
    document.getElementById('enableIPS').checked = config.enableIPS;
  }
  if (config.enableI2CSensors !== undefined) {
    document.getElementById('enableI2CSensors').checked = config.enableI2CSensors;
  }
  
  // Komunikacja
  if (config.enableWiFi !== undefined) {
    document.getElementById('enableWiFi').checked = config.enableWiFi;
  }
  if (config.enableWebServer !== undefined) {
    document.getElementById('enableWebServer').checked = config.enableWebServer;
  }
  if (config.enableModbus !== undefined) {
    document.getElementById('enableModbus').checked = config.enableModbus;
  }
  
  // Tryb Pracy
  if (config.lowPowerMode !== undefined) {
    document.getElementById('lowPowerModeMain').checked = config.lowPowerMode;
  }
}

function saveAllConfigMain() {
  console.log('Saving all system configuration...');
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    // Zbierz wszystkie ustawienia z formularza
    const config = {
      // Historia i Dane
      enableHistory: document.getElementById('enableHistory').checked,
      useAveragedData: document.getElementById('useAveragedData').checked,
      
      // Czujniki
      enableSPS30: document.getElementById('enableSPS30').checked,
      enableSHT40: document.getElementById('enableSHT40').checked,
      enableHCHO: document.getElementById('enableHCHO').checked,
      enableINA219: document.getElementById('enableINA219').checked,
      enableMCP3424: document.getElementById('enableMCP3424').checked,
      enableSolarSensor: document.getElementById('enableSolarSensor').checked,
      enableIPS: document.getElementById('enableIPS').checked,
      enableI2CSensors: document.getElementById('enableI2CSensors').checked,
      
      // Komunikacja
      enableWiFi: document.getElementById('enableWiFi').checked,
      enableWebServer: document.getElementById('enableWebServer').checked,
      enableModbus: document.getElementById('enableModbus').checked,
      
      // Tryb Pracy
      lowPowerMode: document.getElementById('lowPowerModeMain').checked
    };
    
    ws.send(JSON.stringify({
      cmd: 'setConfig',
      ...config
    }));
    
    showAlert('Zapisywanie konfiguracji...', 'info');
  } else {
    showAlert('Brak połączenia WebSocket', 'error');
  }
}



function restartSystemMain() {
  if (confirm('Czy na pewno chcesz zrestartować system?')) {
    console.log('Restarting system...');
    
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({
        cmd: 'system',
        command: 'restart'
      }));
      showAlert('System zostanie zrestartowany...', 'warning');
    } else {
      showAlert('Brak połączenia WebSocket', 'error');
    }
  }
}

// File input handling
document.getElementById('firmwareFile').addEventListener('change', function(e) {
  const file = e.target.files[0];
  const updateBtn = document.getElementById('updateBtn');
  
  if (file && file.name.endsWith('.bin')) {
    updateBtn.disabled = false;
    document.getElementById('updateStatus').innerHTML = `Wybrano: ${file.name}`;
  } else {
    updateBtn.disabled = true;
    document.getElementById('updateStatus').innerHTML = 'Status: Wybierz plik .bin';
  }
});

// Update form handling
document.getElementById('updateForm').addEventListener('submit', function(e) {
  const progressBar = document.getElementById('progressBar');
  const progressFill = document.getElementById('progressFill');
  const updateStatus = document.getElementById('updateStatus');
  
  progressBar.style.display = 'block';
  updateStatus.innerHTML = 'Status: Aktualizacja w toku...';
  
  // Simulate progress (in real implementation, this would come from server)
  let progress = 0;
  const progressInterval = setInterval(() => {
    progress += Math.random() * 10;
    if (progress > 90) progress = 90;
    progressFill.style.width = progress + '%';
  }, 500);
  
  // Clear interval after form submission
  setTimeout(() => clearInterval(progressInterval), 5000);
});

// Event listeners
document.getElementById('toggleAutoReset').addEventListener('click', toggleAutoReset);
document.getElementById('restartBtn').addEventListener('click', restartSystem);
document.getElementById('refreshInfo').addEventListener('click', function() {
  refreshSystemInfo();
  loadSystemConfigMain(); // Also refresh configuration
});

// Initialize
window.addEventListener('load', function() {
  connectWebSocket();
  
  // Get initial auto reset state
  const xhr = new XMLHttpRequest();
  xhr.open("GET", "/getAutoReset", true);
  xhr.onreadystatechange = function() {
    if (xhr.readyState == 4 && xhr.status == 200) {
      updateAutoResetState(xhr.responseText);
    }
  }
  xhr.send();
  
  // Load Pushbullet config after WebSocket connects
  setTimeout(() => {
    loadPushbulletConfig();
  }, 2000);
});

window.addEventListener('beforeunload', function() {
  if (ws) ws.close();
});
</script>
</body>
</html>
)rawliteral";
#endif // UPDATE_HTML_H
