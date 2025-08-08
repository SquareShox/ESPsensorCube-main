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

  /* Fan Control Styles */
  .control-section {
    margin-bottom: 25px;
    padding: 20px;
    background: #f8f9fa;
    border-radius: 10px;
    border-left: 4px solid #667eea;
  }

  .control-section h4 {
    margin-bottom: 15px;
    color: #333;
    font-size: 1.2em;
  }

  .control-buttons {
    display: flex;
    gap: 10px;
    flex-wrap: wrap;
    justify-content: center;
    margin-bottom: 15px;
  }

  .control-buttons .btn {
    flex: 1;
    min-width: 120px;
    margin: 5px;
  }

  .speed-control {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
    margin: 15px 0;
  }

  .speed-control label {
    font-weight: bold;
    color: #333;
  }

  .speed-control input[type="range"] {
    width: 100%;
    max-width: 300px;
    height: 8px;
    border-radius: 5px;
    background: #ddd;
    outline: none;
    -webkit-appearance: none;
  }

  .speed-control input[type="range"]::-webkit-slider-thumb {
    -webkit-appearance: none;
    appearance: none;
    width: 20px;
    height: 20px;
    border-radius: 50%;
    background: #667eea;
    cursor: pointer;
    box-shadow: 0 2px 6px rgba(0,0,0,0.3);
  }

  .speed-control input[type="range"]::-moz-range-thumb {
    width: 20px;
    height: 20px;
    border-radius: 50%;
    background: #667eea;
    cursor: pointer;
    border: none;
    box-shadow: 0 2px 6px rgba(0,0,0,0.3);
  }

  .sleep-controls {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 15px;
    margin-bottom: 15px;
  }

  .sleep-input {
    display: flex;
    flex-direction: column;
    gap: 5px;
  }

  .sleep-input label {
    font-weight: bold;
    color: #333;
    font-size: 0.9em;
  }

  .sleep-input-field {
    padding: 8px 12px;
    border: 2px solid #ddd;
    border-radius: 8px;
    font-size: 1em;
    transition: border-color 0.3s ease;
  }

  .sleep-input-field:focus {
    outline: none;
    border-color: #667eea;
    box-shadow: 0 0 8px rgba(102, 126, 234, 0.3);
  }

  .status-section {
    background: #e8f4f8;
    padding: 15px;
    border-radius: 8px;
    border-left: 4px solid #2196f3;
  }

  .status-section h4 {
    margin-bottom: 10px;
    color: #1976d2;
  }

  /* Configuration Styles */
  .config-sections {
    display: flex;
    flex-direction: column;
    gap: 20px;
  }

  .config-section {
    background: #f8f9fa;
    padding: 20px;
    border-radius: 10px;
    border-left: 4px solid #4caf50;
  }

  .config-section h4 {
    margin-bottom: 15px;
    color: #2e7d32;
    font-size: 1.1em;
    display: flex;
    align-items: center;
    gap: 8px;
  }

  .config-checkbox {
    display: flex;
    align-items: center;
    margin-bottom: 10px;
    cursor: pointer;
    padding: 8px;
    border-radius: 6px;
    transition: background-color 0.2s ease;
  }

  .config-checkbox:hover {
    background-color: rgba(76, 175, 80, 0.1);
  }

  .config-checkbox input[type="checkbox"] {
    margin-right: 10px;
    width: 16px;
    height: 16px;
    cursor: pointer;
  }

  .config-checkbox .checkmark {
    margin-left: 5px;
    flex: 1;
    font-weight: 500;
    color: #333;
  }

  .config-buttons {
    display: flex;
    gap: 10px;
    flex-wrap: wrap;
    justify-content: center;
    margin-top: 15px;
  }

  .config-buttons .btn {
    flex: 1;
    min-width: 150px;
  }

  .setting-group {
    margin-bottom: 15px;
  }

  .setting-label {
    display: flex;
    align-items: center;
    margin-bottom: 8px;
    font-weight: bold;
    color: #333;
  }

  .setting-checkbox {
    margin-right: 10px;
    width: 16px;
    height: 16px;
  }

  .setting-input {
    width: 100%;
    padding: 10px 15px;
    border: 2px solid #ddd;
    border-radius: 8px;
    font-size: 1em;
    transition: border-color 0.3s ease;
  }

  .setting-input:focus {
    outline: none;
    border-color: #667eea;
    box-shadow: 0 0 8px rgba(102, 126, 234, 0.3);
  }

  .btn-primary {
    background: linear-gradient(45deg, #667eea, #764ba2);
  }

  .btn-info {
    background: linear-gradient(45deg, #17a2b8, #20c997);
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

    .control-buttons {
      flex-direction: column;
    }

    .sleep-controls {
      grid-template-columns: 1fr;
    }

    .config-buttons {
      flex-direction: column;
    }

    .control-buttons .btn,
    .config-buttons .btn {
      min-width: auto;
      width: 100%;
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
    <a href="/mcp3424">🔌 MCP3424</a>
  </div>

  <div class="main-container">
    <!-- Firmware Update Card -->
    <div class="control-card">
      <div class="card-header">
        <div class="card-icon update-icon">📦</div>
        <div class="card-title">Aktualizacja Systemu</div>
      </div>
      <div class="card-description">
        Aktualizacja firmware (.bin) lub filesystem (.bin) urządzenia
      </div>
      
      <!-- Firmware Update Section -->
      <div class="control-section">
        <h4>🔧 Aktualizacja Firmware</h4>
        <p style="color: #666; font-size: 0.9em; margin-bottom: 15px;">
          Wybierz plik .bin z nową wersją firmware do aktualizacji systemu
        </p>
        
        <form method="POST" action="/update" enctype="multipart/form-data" id="firmwareUpdateForm">
          <div class="file-input-container">
            <input type="file" name="update" class="file-input" accept=".bin" id="firmwareFile">
          </div>
          
          <button type="submit" class="btn btn-warning" id="firmwareUpdateBtn" disabled>
            📤 Aktualizuj Firmware
          </button>
        </form>
        
        <div class="progress-bar" id="firmwareProgressBar">
          <div class="progress-fill" id="firmwareProgressFill"></div>
        </div>
        
        <div class="status-text" id="firmwareUpdateStatus">
          Status: Gotowy do aktualizacji firmware
        </div>
      </div>
      
      <!-- Filesystem Update Section -->
      <div class="control-section">
        <h4>💾 Aktualizacja Filesystem</h4>
        <p style="color: #666; font-size: 0.9em; margin-bottom: 15px;">
          Wybierz plik .bin z nową wersją filesystem (LittleFS) do aktualizacji
        </p>
        
        <form method="POST" action="/updatefs" enctype="multipart/form-data" id="filesystemUpdateForm">
          <div class="file-input-container">
            <input type="file" name="update" class="file-input" accept=".bin" id="filesystemFile">
          </div>
          
          <button type="submit" class="btn btn-info" id="filesystemUpdateBtn" disabled>
            💾 Aktualizuj Filesystem
          </button>
        </form>
        
        <div class="progress-bar" id="filesystemProgressBar">
          <div class="progress-fill" id="filesystemProgressFill"></div>
        </div>
        
        <div class="status-text" id="filesystemUpdateStatus">
          Status: Gotowy do aktualizacji filesystem
        </div>
      </div>
      
      <!-- Update Info -->
      <div class="status-section">
        <h4>ℹ️ Informacje o Aktualizacji</h4>
        <div class="status-text">
          <strong>Firmware:</strong> Aktualizuje kod programu urządzenia<br>
          <strong>Filesystem:</strong> Aktualizuje pliki konfiguracyjne i dane<br>
          <strong>Uwaga:</strong> Aktualizacja firmware wymaga restartu systemu
        </div>
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
        🔌 MCP3424 ADC
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
        <input type="password" id="pushbulletToken" class="setting-input" placeholder="Wprowadź token Pushbullet">
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
        <strong>Device ID:</strong> <span id="deviceID">--</span><br>
        <strong>Uptime:</strong> <span id="uptime">--</span><br>
        <strong>Free Heap:</strong> <span id="freeHeap">--</span> KB<br>
        <strong>WiFi Signal:</strong> <span id="wifiSignal">--</span> dBm<br>
        <strong>Filesystem:</strong> <span id="filesystemInfo">--</span><br>
        <strong>Last Update:</strong> <span id="lastUpdate">--</span>
      </div>
      
      <!-- Device ID Configuration -->
      <div class="control-section">
        <h4>🆔 Konfiguracja Device ID</h4>
        <div class="input-group">
          <input type="text" id="deviceIDInput" placeholder="Wprowadź Device ID" 
                 class="form-control" style="margin-bottom: 10px;">
          <button class="btn btn-primary" onclick="updateDeviceID()">
            💾 Zapisz Device ID
          </button>
        </div>
      </div>
      
      <!-- Network Flag Control -->
      <div class="control-section">
        <h4>🌐 Flaga Sieci</h4>
        <div class="control-buttons">
          <button class="btn btn-success" onclick="setNetworkFlag(true)">
            ✅ Włącz Flagę Sieci
          </button>
          <button class="btn btn-danger" onclick="setNetworkFlag(false)">
            ❌ Wyłącz Flagę Sieci
          </button>
        </div>
        <div class="status-text" style="margin-top: 10px;">
          <strong>Status flagi:</strong> <span id="networkFlagStatus">--</span>
        </div>
      </div>
      
      <button class="btn btn-warning" id="refreshInfo">
        🔄 Odśwież Info
      </button>
      
      <button class="btn btn-info" onclick="getFilesystemInfo()">
        💾 Sprawdź Filesystem
      </button>
    </div>

    <!-- Fan Control Card -->
    <div class="control-card">
      <div class="card-header">
        <div class="card-icon system-icon">🌀</div>
        <div class="card-title">Sterowanie Wentylatorem i GLine</div>
      </div>
      <div class="card-description">
        Kontrola wentylatora, GLine (router) i tryb sleep<br>
        <small style="color: #ff9800;"><strong>⚠️ Uwaga:</strong> Sterowanie wentylatorem musi być włączone w konfiguracji systemu poniżej</small>
      </div>
      
      <!-- Fan Control -->
      <div class="control-section">
        <h4>🌀 Wentylator</h4>
        <div class="control-buttons">
          <button class="btn btn-success" onclick="sendFanCommand('fan_on')">
            🌀 Włącz Wentylator
          </button>
          <button class="btn btn-danger" onclick="sendFanCommand('fan_off')">
            ⏹️ Wyłącz Wentylator
          </button>
        </div>
        
        <div class="speed-control">
          <label>Prędkość: <span id="fanSpeedValue">50</span>% <small style="color: #666;">(0% = wyłączony)</small></label>
          <input type="range" id="fanSpeedSlider" min="0" max="100" value="50" 
                 oninput="updateFanSpeed(this.value)" 
                 onmouseup="sendFanCommand('fan_speed', parseInt(this.value))"
                 ontouchend="sendFanCommand('fan_speed', parseInt(this.value))">
          <button class="btn btn-primary" onclick="setFanSpeed()" style="margin-top: 10px; min-width: 150px;">
            ⚡ Ustaw Prędkość
          </button>
        </div>
      </div>
      
      <!-- GLine Control -->
      <div class="control-section">
        <h4>⚡ GLine (Router)</h4>
        <div class="control-buttons">
          <button class="btn btn-success" onclick="sendFanCommand('gline_on')">
            ⚡ Włącz GLine
          </button>
          <button class="btn btn-danger" onclick="sendFanCommand('gline_off')">
            🔌 Wyłącz GLine
          </button>
        </div>
      </div>
      
      <!-- Sleep Mode -->
      <div class="control-section">
        <h4>😴 Tryb Sleep</h4>
        <div class="sleep-controls">
          <div class="sleep-input">
            <label>Opóźnienie (s):</label>
            <input type="number" id="sleepDelay" min="0" max="3600" value="60" class="sleep-input-field">
          </div>
          <div class="sleep-input">
            <label>Czas trwania (s):</label>
            <input type="number" id="sleepDuration" min="10" max="7200" value="300" class="sleep-input-field">
          </div>
        </div>
        <div class="control-buttons">
          <button class="btn btn-warning" onclick="startSleepMode()">
            😴 Start Sleep
          </button>
          <button class="btn btn-success" onclick="sendFanCommand('sleep_stop')">
            ⏰ Stop Sleep
          </button>
        </div>
      </div>
      
      <!-- Status Display -->
      <div class="status-section">
        <h4>📊 Status</h4>
        <div class="status-text" id="fanStatus">
          <strong>Wentylator:</strong> <span id="fanState">--</span><br>
          <strong>Prędkość:</strong> <span id="fanDutyCycle">--</span>%<br>
          <strong>RPM:</strong> <span id="fanRPM">--</span><br>
          <strong>GLine:</strong> <span id="glineState">--</span><br>
          <strong>Sleep Mode:</strong> <span id="sleepState">--</span>
        </div>
        <button class="btn btn-primary" onclick="getFanStatus()">
          🔄 Odśwież Status
        </button>
      </div>
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
            <input type="checkbox" id="enableSHT30" onchange="updateSystemConfig('enableSHT30', this.checked)">
            <span class="checkmark"></span>
            SHT30 (Temperatura/Wilgotność)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableBME280" onchange="updateSystemConfig('enableBME280', this.checked)">
            <span class="checkmark"></span>
            BME280 (Temperatura/Wilgotność/Ciśnienie)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableSCD41" onchange="updateSystemConfig('enableSCD41', this.checked)">
            <span class="checkmark"></span>
            SCD41 (CO2/Temperatura/Wilgotność)
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
            MCP3424 (ADC 18-bit)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableADS1110" onchange="updateSystemConfig('enableADS1110', this.checked)">
            <span class="checkmark"></span>
            ADS1110 (ADC 16-bit)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableSolarSensor" onchange="updateSystemConfig('enableSolarSensor', this.checked)">
            <span class="checkmark"></span>
            Solar (Panel słoneczny)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableOPCN3Sensor" onchange="updateSystemConfig('enableOPCN3Sensor', this.checked)">
            <span class="checkmark"></span>
            OPCN3 (Czujnik pyłu optyczny)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableIPS" onchange="updateSystemConfig('enableIPS', this.checked)">
            <span class="checkmark"></span>
            IPS (Czujnik pyłu UART)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableIPSDebug" onchange="updateSystemConfig('enableIPSDebug', this.checked)">
            <span class="checkmark"></span>
            IPS Debug Mode
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableI2CSensors" onchange="updateSystemConfig('enableI2CSensors', this.checked)">
            <span class="checkmark"></span>
            I2C Sensors (Ogólne I2C)
          </label>
          <label class="config-checkbox">
            <input type="checkbox" id="enableFan" onchange="updateSystemConfig('enableFan', this.checked)">
            <span class="checkmark"></span>
            Wentylator i GLine (Kontrola PWM)
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
        
        <!-- Zaawansowane -->
        <div class="config-section">
          <h4>🔧 Zaawansowane</h4>
          <label class="config-checkbox">
            <input type="checkbox" id="autoReset" onchange="updateSystemConfig('autoReset', this.checked)">
            <span class="checkmark"></span>
            Auto Reset (Automatyczny restart systemu)
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

    <!-- Calibration Constants Card -->
    <div class="control-card">
      <div class="card-header">
        <div class="card-icon charts-icon">🔧</div>
        <div class="card-title">Stałe Kalibracyjne</div>
      </div>
      <div class="card-description">
        Zarządzanie stałymi kalibracyjnymi z pliku JSON
      </div>
      
      <div class="config-sections">
        <div class="config-section">
          <h4>📊 Zarządzanie Stałymi</h4>
          <div class="config-buttons">
            <button class="btn btn-primary" onclick="loadCalibrationConstants()">
              📥 Wczytaj Stałe
            </button>
            <button class="btn btn-success" onclick="saveCalibrationConstants()">
              💾 Zapisz Stałe
            </button>
            <button class="btn btn-info" onclick="downloadCalibrationConstants()">
              📤 Pobierz Plik
            </button>
            <button class="btn btn-warning" onclick="resetCalibrationConstants()">
              🔄 Resetuj do Domyślnych
            </button>
          </div>
        </div>
        
        <div class="config-section">
          <h4>📝 Edytor Stałych</h4>
          <div style="margin-bottom: 15px;">
            <textarea id="calibConstantsEditor" rows="15" style="width: 100%; font-family: monospace; font-size: 12px; padding: 10px; border: 2px solid #ddd; border-radius: 8px;" placeholder="Wklej tutaj JSON ze stałymi kalibracyjnymi..."></textarea>
          </div>
          <div class="config-buttons">
            <button class="btn btn-primary" onclick="applyCalibrationConstants()">
              ✅ Zastosuj Zmiany
            </button>
            <button class="btn btn-secondary" onclick="formatCalibrationConstants()">
              🎨 Formatuj JSON
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
      } else if (data.cmd === 'setNetworkFlag') {
        // Handle network flag response
        if (data.success) {
          showAlert(data.message || 'Flaga sieci zaktualizowana', 'success');
          // Update status display
          document.getElementById('networkFlagStatus').textContent = data.enabled ? 'WŁĄCZONA' : 'WYŁĄCZONA';
        } else {
          showAlert('Błąd aktualizacji flagi sieci: ' + (data.error || 'Nieznany błąd'), 'error');
        }
      } else if (data.cmd === 'fan_status' || data.cmd === 'fan_on' || data.cmd === 'fan_off' || 
                 data.cmd === 'fan_speed' || data.cmd === 'gline_on' || data.cmd === 'gline_off' ||
                 data.cmd === 'sleep' || data.cmd === 'sleep_stop') {
        // Handle fan control commands
        updateFanStatusDisplay(data);
      } else if (data.cmd === 'restart' || data.cmd === 'lowPowerOn' || data.cmd === 'lowPowerOff' ||
                 data.cmd === 'pushbulletTest' || data.cmd === 'pushbulletBatteryTest') {
        // Handle other system commands
        if (data.success) {
          showAlert(data.message || `Komenda ${data.cmd} wykonana pomyślnie`, 'success');
        } else {
          showAlert(data.error || `Błąd wykonania komendy ${data.cmd}`, 'error');
        }
      } else if (data.cmd === 'filesystemInfo') {
        // Handle filesystem info response
        if (data.success) {
          document.getElementById('filesystemInfo').textContent = data.filesystemInfo || '--';
          showAlert('Informacje o filesystem zaktualizowane', 'success');
        } else {
          showAlert('Błąd pobierania informacji o filesystem: ' + (data.error || 'Nieznany błąd'), 'error');
        }
      } else if (data.cmd === 'system') {
        // Handle legacy system commands (backward compatibility)
        updateFanStatusDisplay(data);
        
        // Also handle other system messages
        if (data.command === 'memory') {
          updateSystemInfo(data);
        }
      } else {
        // Handle other messages (status, sensor data, etc.)
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
    const deviceID = data.DeviceID || (data.data && data.data.DeviceID) || '--';
    const filesystemInfo = data.filesystemInfo || (data.data && data.data.filesystemInfo) || '--';
    
    document.getElementById('deviceID').textContent = deviceID;
    document.getElementById('deviceIDInput').value = deviceID; // Fill the input field
    document.getElementById('uptime').textContent = formatUptime(uptime);
    document.getElementById('freeHeap').textContent = Math.round(freeHeap / 1024);
    document.getElementById('wifiSignal').textContent = wifiSignal;
    document.getElementById('filesystemInfo').textContent = filesystemInfo;
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
        cmd: command  // Bezpośrednie użycie komendy
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
      cmd: "pushbulletTest"  // Bezpośrednie użycie komendy
    }));
  }
}

function testBatteryNotification() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      cmd: "pushbulletBatteryTest"  // Bezpośrednie użycie komendy
    }));
  }
}



function refreshSystemInfo() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({cmd: "status"}));
  }
}

function getFilesystemInfo() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({cmd: "filesystemInfo"}));
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
  if (config.enableSHT30 !== undefined) {
    document.getElementById('enableSHT30').checked = config.enableSHT30;
  }
  if (config.enableBME280 !== undefined) {
    document.getElementById('enableBME280').checked = config.enableBME280;
  }
  if (config.enableSCD41 !== undefined) {
    document.getElementById('enableSCD41').checked = config.enableSCD41;
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
  if (config.enableADS1110 !== undefined) {
    document.getElementById('enableADS1110').checked = config.enableADS1110;
  }
  if (config.enableSolarSensor !== undefined) {
    document.getElementById('enableSolarSensor').checked = config.enableSolarSensor;
  }
  if (config.enableOPCN3Sensor !== undefined) {
    document.getElementById('enableOPCN3Sensor').checked = config.enableOPCN3Sensor;
  }
  if (config.enableIPS !== undefined) {
    document.getElementById('enableIPS').checked = config.enableIPS;
  }
  if (config.enableIPSDebug !== undefined) {
    document.getElementById('enableIPSDebug').checked = config.enableIPSDebug;
  }
  if (config.enableI2CSensors !== undefined) {
    document.getElementById('enableI2CSensors').checked = config.enableI2CSensors;
  }
  if (config.enableFan !== undefined) {
    document.getElementById('enableFan').checked = config.enableFan;
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
  // Zaawansowane
  if (config.autoReset !== undefined) {
    document.getElementById('autoReset').checked = config.autoReset;
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
      enableSHT30: document.getElementById('enableSHT30').checked,
      enableBME280: document.getElementById('enableBME280').checked,
      enableSCD41: document.getElementById('enableSCD41').checked,
      enableHCHO: document.getElementById('enableHCHO').checked,
      enableINA219: document.getElementById('enableINA219').checked,
      enableMCP3424: document.getElementById('enableMCP3424').checked,
      enableADS1110: document.getElementById('enableADS1110').checked,
      enableSolarSensor: document.getElementById('enableSolarSensor').checked,
      enableOPCN3Sensor: document.getElementById('enableOPCN3Sensor').checked,
      enableIPS: document.getElementById('enableIPS').checked,
      enableIPSDebug: document.getElementById('enableIPSDebug').checked,
      enableI2CSensors: document.getElementById('enableI2CSensors').checked,
      enableFan: document.getElementById('enableFan').checked,
      
      // Komunikacja
      enableWiFi: document.getElementById('enableWiFi').checked,
      enableWebServer: document.getElementById('enableWebServer').checked,
      enableModbus: document.getElementById('enableModbus').checked,
      
      // Tryb Pracy
      lowPowerMode: document.getElementById('lowPowerModeMain').checked,
      // Zaawansowane
      autoReset: document.getElementById('autoReset').checked
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
        cmd: 'restart'  // Bezpośrednie użycie komendy restart
      }));
      showAlert('System zostanie zrestartowany...', 'warning');
    } else {
      showAlert('Brak połączenia WebSocket', 'error');
    }
  }
}

// File input handling for firmware update
document.getElementById('firmwareFile').addEventListener('change', function(e) {
  const file = e.target.files[0];
  const updateBtn = document.getElementById('firmwareUpdateBtn');
  
  if (file && file.name.endsWith('.bin')) {
    updateBtn.disabled = false;
    document.getElementById('firmwareUpdateStatus').innerHTML = `Wybrano: ${file.name}`;
  } else {
    updateBtn.disabled = true;
    document.getElementById('firmwareUpdateStatus').innerHTML = 'Status: Wybierz plik .bin';
  }
});

// File input handling for filesystem update
document.getElementById('filesystemFile').addEventListener('change', function(e) {
  const file = e.target.files[0];
  const updateBtn = document.getElementById('filesystemUpdateBtn');
  
  if (file && file.name.endsWith('.bin')) {
    updateBtn.disabled = false;
    document.getElementById('filesystemUpdateStatus').innerHTML = `Wybrano: ${file.name}`;
  } else {
    updateBtn.disabled = true;
    document.getElementById('filesystemUpdateStatus').innerHTML = 'Status: Wybierz plik .bin';
  }
});

// Firmware update form handling
document.getElementById('firmwareUpdateForm').addEventListener('submit', function(e) {
  const progressBar = document.getElementById('firmwareProgressBar');
  const progressFill = document.getElementById('firmwareProgressFill');
  const updateStatus = document.getElementById('firmwareUpdateStatus');
  
  progressBar.style.display = 'block';
  updateStatus.innerHTML = 'Status: Aktualizacja firmware w toku...';
  
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

// Filesystem update form handling
document.getElementById('filesystemUpdateForm').addEventListener('submit', function(e) {
  const progressBar = document.getElementById('filesystemProgressBar');
  const progressFill = document.getElementById('filesystemProgressFill');
  const updateStatus = document.getElementById('filesystemUpdateStatus');
  
  progressBar.style.display = 'block';
  updateStatus.innerHTML = 'Status: Aktualizacja filesystem w toku...';
  
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

// Fan Control Functions
function sendFanCommand(command, value = null) {
  console.log('=== sendFanCommand() called ===');
  console.log('command:', command);
  console.log('value:', value);
  console.log('value type:', typeof value);
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    const message = {
      cmd: command  // Bezpośrednie użycie komendy jako cmd
    };
    
    if (value !== null) {
      message.value = value;
      console.log('Added value to message:', message.value);
    }
    
    // For sleep command, add delay and duration
    if (command === 'sleep') {
      message.delay = parseInt(document.getElementById('sleepDelay').value) || 60;
      message.duration = parseInt(document.getElementById('sleepDuration').value) || 300;
    }
    
    console.log('Final message object:', message);
    console.log('JSON string:', JSON.stringify(message));
    
    ws.send(JSON.stringify(message));
    
    // Show loading feedback
    showAlert(`Wykonuję komendę: ${command}`, 'info');
    
    // Refresh status after command
    setTimeout(() => {
      getFanStatus();
    }, 1000);
  } else {
    showAlert('Brak połączenia WebSocket', 'error');
  }
}

function updateFanSpeed(value) {
  document.getElementById('fanSpeedValue').textContent = value;
}

function setFanSpeed() {
  console.log('=== setFanSpeed() called ===');
  
  const slider = document.getElementById('fanSpeedSlider');
  console.log('Slider element:', slider);
  
  const speed = slider.value;
  console.log('Slider raw value:', speed);
  console.log('Slider value type:', typeof speed);
  
  const speedInt = parseInt(speed);
  console.log('Parsed to int:', speedInt);
  
  sendFanCommand('fan_speed', speedInt);
}

function startSleepMode() {
  const delay = parseInt(document.getElementById('sleepDelay').value) || 60;
  const duration = parseInt(document.getElementById('sleepDuration').value) || 300;
  
  if (confirm(`Uruchomić tryb sleep z opóźnieniem ${delay}s i czasem trwania ${duration}s?`)) {
    sendFanCommand('sleep');
  }
}

function getFanStatus() {
  console.log('Getting fan status...');
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      cmd: 'fan_status'  // Bezpośrednie użycie komendy
    }));
  } else {
    showAlert('Brak połączenia WebSocket', 'error');
  }
}

function updateFanStatusDisplay(data) {
  console.log('Updating fan status display:', data);
  
  if (data.cmd === 'fan_status') {
    // Obsługa odpowiedzi fan_status
    document.getElementById('fanState').textContent = data.enabled ? 'Włączony' : 'Wyłączony';
    document.getElementById('fanDutyCycle').textContent = data.dutyCycle || '--';
    document.getElementById('fanRPM').textContent = data.rpm || '--';
    document.getElementById('glineState').textContent = data.glineEnabled ? 'Włączony' : 'Wyłączony';
    
    // Update sleep mode status
    if (data.sleepMode) {
      const sleepEnd = new Date(data.sleepEndTime * 1000); // Convert from epoch
      document.getElementById('sleepState').textContent = `Aktywny do ${sleepEnd.toLocaleTimeString()}`;
    } else {
      document.getElementById('sleepState').textContent = 'Nieaktywny';
    }
    
    // Update slider position
    if (data.dutyCycle !== undefined) {
      document.getElementById('fanSpeedSlider').value = data.dutyCycle;
      document.getElementById('fanSpeedValue').textContent = data.dutyCycle;
    }
  } else if (data.cmd === 'fan_speed' || data.cmd === 'fan_on' || data.cmd === 'fan_off' ||
             data.cmd === 'gline_on' || data.cmd === 'gline_off' || 
             data.cmd === 'sleep' || data.cmd === 'sleep_stop') {
    // Obsługa odpowiedzi komend fan
    if (data.success) {
      showAlert(data.message || `Komenda ${data.cmd} wykonana pomyślnie`, 'success');
      
      // Update specific UI elements based on command
      if (data.cmd === 'fan_speed' && data.speed !== undefined) {
        document.getElementById('fanSpeedSlider').value = data.speed;
        document.getElementById('fanSpeedValue').textContent = data.speed;
      }
    } else {
      showAlert(data.error || `Błąd wykonania komendy ${data.cmd}`, 'error');
    }
  } else if (data.cmd === 'system') {
    // Fallback dla starych komend system (jeśli są jeszcze używane)
    updateFanStatusDisplay_Legacy(data);
  }
}

// Legacy function for backward compatibility
function updateFanStatusDisplay_Legacy(data) {
  if (data.command === 'fan_status') {
    document.getElementById('fanState').textContent = data.enabled ? 'Włączony' : 'Wyłączony';
    document.getElementById('fanDutyCycle').textContent = data.dutyCycle || '--';
    document.getElementById('fanRPM').textContent = data.rpm || '--';
    document.getElementById('glineState').textContent = data.glineEnabled ? 'Włączony' : 'Wyłączony';
    
    if (data.sleepMode) {
      const sleepEnd = new Date(data.sleepEndTime);
      document.getElementById('sleepState').textContent = `Aktywny do ${sleepEnd.toLocaleTimeString()}`;
    } else {
      document.getElementById('sleepState').textContent = 'Nieaktywny';
    }
    
    if (data.dutyCycle !== undefined) {
      document.getElementById('fanSpeedSlider').value = data.dutyCycle;
      document.getElementById('fanSpeedValue').textContent = data.dutyCycle;
    }
  }
}

// Alert/notification function
function showAlert(message, type = 'info') {
  // Create alert element if it doesn't exist
  let alertContainer = document.getElementById('alertContainer');
  if (!alertContainer) {
    alertContainer = document.createElement('div');
    alertContainer.id = 'alertContainer';
    alertContainer.style.cssText = `
      position: fixed;
      top: 20px;
      right: 20px;
      z-index: 10000;
      max-width: 300px;
    `;
    document.body.appendChild(alertContainer);
  }
  
  // Create alert
  const alert = document.createElement('div');
  alert.style.cssText = `
    padding: 15px;
    margin-bottom: 10px;
    border-radius: 8px;
    color: white;
    font-weight: bold;
    opacity: 0;
    transform: translateX(100%);
    transition: all 0.3s ease;
    box-shadow: 0 4px 12px rgba(0,0,0,0.3);
  `;
  
  // Set color based on type
  switch (type) {
    case 'success':
      alert.style.background = 'linear-gradient(45deg, #4caf50, #8bc34a)';
      break;
    case 'error':
      alert.style.background = 'linear-gradient(45deg, #f44336, #ff5722)';
      break;
    case 'warning':
      alert.style.background = 'linear-gradient(45deg, #ff9800, #ffc107)';
      break;
    default:
      alert.style.background = 'linear-gradient(45deg, #2196f3, #03a9f4)';
  }
  
  alert.textContent = message;
  alertContainer.appendChild(alert);
  
  // Animate in
  setTimeout(() => {
    alert.style.opacity = '1';
    alert.style.transform = 'translateX(0)';
  }, 100);
  
  // Remove after 3 seconds
  setTimeout(() => {
    alert.style.opacity = '0';
    alert.style.transform = 'translateX(100%)';
    setTimeout(() => {
      if (alert.parentNode) {
        alert.parentNode.removeChild(alert);
      }
    }, 300);
  }, 3000);
}

// Device ID and Network Flag Functions
function updateDeviceID() {
  const deviceID = document.getElementById('deviceIDInput').value.trim();
  if (!deviceID) {
    showAlert('Wprowadź Device ID', 'error');
    return;
  }
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    const message = {
      cmd: 'setConfig',
      DeviceID: deviceID
    };
    ws.send(JSON.stringify(message));
    showAlert('Wysyłanie Device ID...', 'info');
  } else {
    showAlert('WebSocket nie jest połączony', 'error');
  }
}

function setNetworkFlag(enabled) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    const message = {
      cmd: 'setNetworkFlag',
      enabled: enabled
    };
    ws.send(JSON.stringify(message));
    showAlert(`Flaga sieci ${enabled ? 'włączona' : 'wyłączona'}`, 'info');
    
    // Update status display
    document.getElementById('networkFlagStatus').textContent = enabled ? 'WŁĄCZONA' : 'WYŁĄCZONA';
  } else {
    showAlert('WebSocket nie jest połączony', 'error');
  }
}

// Event listeners
document.getElementById('toggleAutoReset').addEventListener('click', toggleAutoReset);
document.getElementById('restartBtn').addEventListener('click', restartSystemMain);
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
  
  // Load Pushbullet config and fan status after WebSocket connects
  setTimeout(() => {
    loadPushbulletConfig();
    getFanStatus(); // Load initial fan status
    getFilesystemInfo(); // Load initial filesystem info
  }, 2000);
});

window.addEventListener('beforeunload', function() {
  if (ws) ws.close();
});

// Calibration Constants Functions
function loadCalibrationConstants() {
  fetch('/api/calib-constants')
    .then(response => response.json())
    .then(data => {
      const editor = document.getElementById('calibConstantsEditor');
      editor.value = JSON.stringify(data, null, 2);
      showAlert('Stałe kalibracyjne wczytane pomyślnie', 'success');
    })
    .catch(error => {
      console.error('Error loading calibration constants:', error);
      showAlert('Błąd wczytywania stałych kalibracyjnych', 'error');
    });
}

function saveCalibrationConstants() {
  const editor = document.getElementById('calibConstantsEditor');
  const jsonData = editor.value.trim();
  
  try {
    const data = JSON.parse(jsonData);
    
    fetch('/api/calib-constants', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded',
      },
      body: 'data=' + encodeURIComponent(jsonData)
    })
    .then(response => response.json())
    .then(result => {
      if (result.success) {
        showAlert('Stałe kalibracyjne zapisane pomyślnie', 'success');
      } else {
        showAlert('Błąd zapisywania stałych kalibracyjnych: ' + (result.error || 'Nieznany błąd'), 'error');
      }
    })
    .catch(error => {
      console.error('Error saving calibration constants:', error);
      showAlert('Błąd zapisywania stałych kalibracyjnych', 'error');
    });
  } catch (error) {
    showAlert('Nieprawidłowy format JSON', 'error');
  }
}

function downloadCalibrationConstants() {
  const editor = document.getElementById('calibConstantsEditor');
  const jsonData = editor.value.trim();
  
  if (!jsonData) {
    showAlert('Brak danych do pobrania', 'error');
    return;
  }
  
  try {
    const data = JSON.parse(jsonData);
    const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'calib_nums.json';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    showAlert('Plik pobrany pomyślnie', 'success');
  } catch (error) {
    showAlert('Błąd podczas pobierania pliku', 'error');
  }
}

function resetCalibrationConstants() {
  if (confirm('Czy na pewno chcesz zresetować stałe kalibracyjne do wartości domyślnych?')) {
    fetch('/api/calib-constants')
      .then(response => response.json())
      .then(data => {
        // Reset to default values (all 1.0 for multipliers, 0.0 for offsets)
        const defaults = {
          RL_TGS03: 10000.0,
          RL_TGS02: 3300.0,
          RL_TGS12: 10000.0,
          TGS03_B1: 1.0,
          TGS03_A: 0.0,
          TGS02_B1: 1.0,
          TGS02_A: 0.0,
          TGS12_B1: 1.0,
          TGS12_A: 0.0,
          CO_B0: 0.0,
          CO_B1: 1.0,
          CO_B2: 0.0,
          CO_B3: 0.0,
          NO_B0: 0.0,
          NO_B1: 1.0,
          NO_B2: 0.0,
          NO_B3: 0.0,
          NO2_B0: 0.0,
          NO2_B1: 1.0,
          NO2_B2: 0.0,
          NO2_B3: 0.0,
          O3_B0: 0.0,
          O3_B1: 1.0,
          O3_B2: 0.0,
          O3_B3: 0.0,
          O3_D: 0.0,
          SO2_B0: 0.0,
          SO2_B1: 1.0,
          SO2_B2: 0.0,
          SO2_B3: 0.0,
          NH3_B0: 0.0,
          NH3_B1: 1.0,
          NH3_B3: 0.0,
          H2S_B0: 0.0,
          H2S_B1: 1.0,
          H2S_B2: 0.0,
          H2S_B3: 0.0,
          CO_PPB_DIV: 1.0,
          NO_PPB_DIV: 1.0,
          NO2_PPB_DIV: 1.0,
          O3_PPB_DIV: 1.0,
          SO2_PPB_DIV: 1.0,
          H2S_PPB_DIV: 1.0,
          NH3_PPB_DIV: 1.0,
          PID_OFFSET: 0.0,
          PID_B: 0.0,
          PID_A: 1.0,
          PID_CF: 1.0,
          HCHO_B1: 1.0,
          HCHO_A: 0.0,
          HCHO_PPB_CF: 1.0,
          PM1_B1: 1.0,
          PM1_A: 0.0,
          PM25_B1: 1.0,
          PM25_A: 0.0,
          PM10_B1: 1.0,
          PM10_A: 0.0,
          AMBIENT_TEMP_B1: 1.0,
          AMBIENT_TEMP_A: 0.0,
          AMBIENT_HUMID_B1: 1.0,
          AMBIENT_HUMID_A: 0.0,
          AMBIENT_PRESS_B1: 1.0,
          AMBIENT_PRESS_A: 0.0,
          DUST_TEMP_B1: 1.0,
          DUST_TEMP_A: 0.0,
          DUST_HUMID_B1: 1.0,
          DUST_HUMID_A: 0.0,
          DUST_PRESS_B1: 1.0,
          DUST_PRESS_A: 0.0,
          GAS_TEMP_B1: 1.0,
          GAS_TEMP_A: 0.0,
          GAS_HUMID_B1: 1.0,
          GAS_HUMID_A: 0.0,
          GAS_PRESS_B1: 1.0,
          GAS_PRESS_A: 0.0,
          SCD_CO2_B1: 1.0,
          SCD_CO2_A: 0.0,
          SCD_TEMP_B1: 1.0,
          SCD_TEMP_A: 0.0,
          SCD_RH_B1: 1.0,
          SCD_RH_A: 0.0,
          ODO_A0: 0.0,
          ODO_A1: 1.0,
          ODO_A2: 1.0,
          ODO_A3: 1.0,
          ODO_A4: 1.0,
          ODO_A5: 1.0,
          B4_TO: 25.0,
          B4_B: 3380.0,
          B4_RO: 10.0,
          B4_RS: 33.0,
          B4_K: 3.2,
          B4_COK: 273.15,
          TGS_TO: 25.0,
          TGS_B: 3380.0,
          TGS_RO: 10.0,
          TGS_COK: 273.15,
          B4_LSB: 3.90625,
          TGS_LSB: 3.90625,
          TGS_K: 5.0,
          TEMP_MIN: -30.0,
          TEMP_MAX: 1000.0,
          VOLTAGE_MIN: 0.0,
          VOLTAGE_MAX: 7000.0,
          GAS_MIN: 0.0,
          GAS_MAX: 50000.0,
          PPB_MIN: 0.0,
          PPB_MAX: 50000.0,
          TGS_MIN: 0.0,
          TGS_MAX: 1000000.0,
          HCHO_MIN: 0.0,
          HCHO_MAX: 40000.0,
          PID_MIN: 0.0,
          PID_MAX: 40000.0,
          PM_MIN: 0.0,
          PM_MAX: 5000.0,
          ENV_TEMP_MIN: -100.0,
          ENV_TEMP_MAX: 100.0,
          ENV_HUMID_MIN: 0.0,
          ENV_HUMID_MAX: 100.0,
          ENV_PRESS_MIN: 0.0,
          ENV_PRESS_MAX: 2000.0,
          CO2_MIN: 0.0,
          CO2_MAX: 60000.0,
          ODO_MIN: 0.0,
          ODO_MAX: 1000000.0
        };
        
        const editor = document.getElementById('calibConstantsEditor');
        editor.value = JSON.stringify(defaults, null, 2);
        showAlert('Stałe kalibracyjne zresetowane do wartości domyślnych', 'success');
      })
      .catch(error => {
        console.error('Error resetting calibration constants:', error);
        showAlert('Błąd resetowania stałych kalibracyjnych', 'error');
      });
  }
}

function applyCalibrationConstants() {
  saveCalibrationConstants();
}

function formatCalibrationConstants() {
  const editor = document.getElementById('calibConstantsEditor');
  const jsonData = editor.value.trim();
  
  try {
    const data = JSON.parse(jsonData);
    editor.value = JSON.stringify(data, null, 2);
    showAlert('JSON sformatowany pomyślnie', 'success');
  } catch (error) {
    showAlert('Nieprawidłowy format JSON', 'error');
  }
}
</script>
</body>
</html>
)rawliteral";
#endif // UPDATE_HTML_H
