
// Forward declarations - charts_html moved to chart.h

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
      
      <div class="status-text">
        <span class="status-indicator status-ok"></span>
        System gotowy
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
  </div>

<script>
let ws;

function connectWebSocket() {
  ws = new WebSocket(`ws://${window.location.host}/ws`);
  ws.onopen = function() {
    console.log('WebSocket connected');
    updateSystemInfo();
  };
  ws.onclose = function() {
    console.log('WebSocket disconnected, reconnecting...');
    setTimeout(connectWebSocket, 5000);
  };
  ws.onmessage = function(event) {
    try {
      const data = JSON.parse(event.data);
      updateSystemInfo(data);
    } catch (error) {
      console.error('Error parsing WebSocket data:', error);
    }
  };
}

function updateSystemInfo(data = null) {
  if (data) {
    document.getElementById('uptime').textContent = formatUptime(data.uptime || 0);
    document.getElementById('freeHeap').textContent = Math.round((data.freeHeap || 0) / 1024);
    document.getElementById('wifiSignal').textContent = data.wifiSignal || 0;
    document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString('pl-PL');
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
document.getElementById('refreshInfo').addEventListener('click', refreshSystemInfo);

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
});

window.addEventListener('beforeunload', function() {
  if (ws) ws.close();
});
</script>
</body>
</html>
)rawliteral";

const char *dashboard_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP Sensor Cube - Dashboard</title>
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
    color: #667eea;
  }

  .dashboard {
    max-width: 1400px;
    margin: 0 auto;
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
    gap: 25px;
  }

  .sensor-card {
    background: rgba(255, 255, 255, 0.95);
    border-radius: 20px;
    padding: 25px;
    box-shadow: 0 10px 30px rgba(0,0,0,0.2);
    transition: transform 0.3s ease;
  }

  .sensor-card:hover {
    transform: translateY(-5px);
  }

  .sensor-header {
    display: flex;
    align-items: center;
    margin-bottom: 20px;
    padding-bottom: 15px;
    border-bottom: 3px solid #f0f0f0;
  }

  .sensor-icon {
    width: 40px;
    height: 40px;
    border-radius: 50%;
    margin-right: 15px;
    display: flex;
    align-items: center;
    justify-content: center;
    color: white;
    font-weight: bold;
    font-size: 18px;
  }

  .solar-icon { background: linear-gradient(45deg, #ff9800, #ffc107); }
  .air-icon { background: linear-gradient(45deg, #2196f3, #03a9f4); }
  .env-icon { background: linear-gradient(45deg, #4caf50, #8bc34a); }
  .adc-icon { background: linear-gradient(45deg, #9c27b0, #e91e63); }
  .power-icon { background: linear-gradient(45deg, #f44336, #ff5722); }
  .system-icon { background: linear-gradient(45deg, #607d8b, #9e9e9e); }

  .sensor-title {
    font-size: 1.4em;
    font-weight: bold;
    color: #333;
  }

  .sensor-data {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
    gap: 15px;
  }

  .data-item {
    text-align: center;
    padding: 15px;
    background: linear-gradient(45deg, #f8f9fa, #e9ecef);
    border-radius: 12px;
    transition: all 0.3s ease;
  }

  .data-item:hover {
    background: linear-gradient(45deg, #e9ecef, #dee2e6);
  }

  .data-label {
    font-size: 0.9em;
    color: #666;
    margin-bottom: 8px;
    font-weight: 500;
  }

  .data-value {
    font-size: 1.8em;
    font-weight: bold;
    color: #333;
  }

  .data-unit {
    font-size: 0.8em;
    color: #888;
    margin-left: 5px;
  }

  .status-indicator {
    width: 12px;
    height: 12px;
    border-radius: 50%;
    margin-left: auto;
    animation: pulse 2s infinite;
  }

  .status-ok { background-color: #4caf50; }
  .status-error { background-color: #f44336; }

  @keyframes pulse {
    0% { opacity: 1; }
    50% { opacity: 0.5; }
    100% { opacity: 1; }
  }

  .last-update {
    text-align: center;
    margin-top: 15px;
    font-size: 0.85em;
    color: #666;
  }

  @media (max-width: 768px) {
    .dashboard {
      grid-template-columns: 1fr;
    }
    
    .header h1 {
      font-size: 2em;
    }
    
    .sensor-data {
      grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
    }
  }
</style>
</head>
<body>
  <div class="header">
    <h1>🔬 ESP Sensor Cube Dashboard</h1>
    <p>Pomiary w czasie rzeczywistym</p>
  </div>

  <div class="nav">
    <a href="/">🔧 Panel Sterowania</a>
    <a href="/dashboard">📊 Dashboard</a>
    <a href="/charts">📈 Wykresy</a>
  </div>

  <div class="dashboard">
    <!-- System Status Card - Always visible -->
    <div class="sensor-card" id="system-card">
      <div class="sensor-header">
        <div class="sensor-icon system-icon">⚙️</div>
        <div class="sensor-title">System Status</div>
        <div class="status-indicator" id="system-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">Uptime</div>
          <div class="data-value" id="uptime">--<span class="data-unit">s</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Free Heap</div>
          <div class="data-value" id="free-heap">--<span class="data-unit">KB</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">WiFi Signal</div>
          <div class="data-value" id="wifi-signal">--<span class="data-unit">dBm</span></div>
        </div>
      </div>
      <div class="last-update" id="system-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- Solar Sensor Card -->
    <div class="sensor-card" id="solar-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon solar-icon">☀️</div>
        <div class="sensor-title">Solar Monitor</div>
        <div class="status-indicator" id="solar-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">Napięcie</div>
          <div class="data-value" id="solar-voltage">--<span class="data-unit">V</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Prąd</div>
          <div class="data-value" id="solar-current">--<span class="data-unit">A</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PV Voltage</div>
          <div class="data-value" id="solar-vpv">--<span class="data-unit">V</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PV Power</div>
          <div class="data-value" id="solar-ppv">--<span class="data-unit">W</span></div>
        </div>
      </div>
      <div class="last-update" id="solar-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- Air Quality Card -->
    <div class="sensor-card" id="opcn3-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon air-icon">🌬️</div>
        <div class="sensor-title">Jakość Powietrza</div>
        <div class="status-indicator" id="opcn3-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">PM1.0</div>
          <div class="data-value" id="pm1">--<span class="data-unit">µg/m³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PM2.5</div>
          <div class="data-value" id="pm25">--<span class="data-unit">µg/m³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PM10</div>
          <div class="data-value" id="pm10">--<span class="data-unit">µg/m³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Temperatura</div>
          <div class="data-value" id="opcn3-temp">--<span class="data-unit">°C</span></div>
        </div>
      </div>
      <div class="last-update" id="opcn3-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- Environmental Sensors Card -->
    <div class="sensor-card" id="i2c-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon env-icon">🌡️</div>
        <div class="sensor-title">Czujniki Środowiskowe</div>
        <div class="status-indicator" id="i2c-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">Temperatura</div>
          <div class="data-value" id="env-temp">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Wilgotność</div>
          <div class="data-value" id="env-humidity">--<span class="data-unit">%</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Ciśnienie</div>
          <div class="data-value" id="env-pressure">--<span class="data-unit">hPa</span></div>
        </div>
      </div>
      <div class="last-update" id="i2c-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- SCD41 CO2 Sensor Card -->
    <div class="sensor-card" id="scd41-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon env-icon">🌬️</div>
        <div class="sensor-title">SCD41 CO2 Sensor</div>
        <div class="status-indicator" id="scd41-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">CO2</div>
          <div class="data-value" id="scd41-co2">--<span class="data-unit">ppm</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Temperatura</div>
          <div class="data-value" id="scd41-temp">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Wilgotność</div>
          <div class="data-value" id="scd41-humidity">--<span class="data-unit">%</span></div>
        </div>
      </div>
      <div class="last-update" id="scd41-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- SPS30 Particle Sensor Card -->
    <div class="sensor-card" id="sps30-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon air-icon">💨</div>
        <div class="sensor-title">SPS30 Cząstki PM</div>
        <div class="status-indicator" id="sps30-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">PM1.0</div>
          <div class="data-value" id="sps30-pm1">--<span class="data-unit">µg/m³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PM2.5</div>
          <div class="data-value" id="sps30-pm25">--<span class="data-unit">µg/m³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PM4.0</div>
          <div class="data-value" id="sps30-pm4">--<span class="data-unit">µg/m³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PM10</div>
          <div class="data-value" id="sps30-pm10">--<span class="data-unit">µg/m³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Rozmiar</div>
          <div class="data-value" id="sps30-size">--<span class="data-unit">µm</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">NC0.5</div>
          <div class="data-value" id="sps30-nc05">--<span class="data-unit">#/cm³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">NC1.0</div>
          <div class="data-value" id="sps30-nc1">--<span class="data-unit">#/cm³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">NC2.5</div>
          <div class="data-value" id="sps30-nc25">--<span class="data-unit">#/cm³</span></div>
        </div>
      </div>
      <div class="last-update" id="sps30-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- ADC Sensors Container (dynamic content) -->
    <div id="adc-sensors-container">
      <!-- MCP3424 devices will be dynamically inserted here -->
    </div>
    
    <!-- ADS1110 Sensor Card -->
    <div class="sensor-card" id="ads1110-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon adc-icon">⚖️</div>
        <div class="sensor-title">ADS1110 ADC</div>
        <div class="status-indicator" id="ads1110-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">Voltage</div>
          <div class="data-value" id="ads-voltage">--<span class="data-unit">V</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Data Rate</div>
          <div class="data-value" id="ads-rate">--<span class="data-unit">SPS</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Gain</div>
          <div class="data-value" id="ads-gain">--<span class="data-unit">x</span></div>
        </div>
      </div>
      <div class="last-update" id="ads1110-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- Power Monitor Card -->
    <div class="sensor-card" id="power-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon power-icon">⚡</div>
        <div class="sensor-title">Monitor Mocy INA219</div>
        <div class="status-indicator" id="power-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">Bus Voltage</div>
          <div class="data-value" id="bus-voltage">--<span class="data-unit">V</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Current</div>
          <div class="data-value" id="current">--<span class="data-unit">mA</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Power</div>
          <div class="data-value" id="power">--<span class="data-unit">mW</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Shunt</div>
          <div class="data-value" id="shunt-voltage">--<span class="data-unit">mV</span></div>
        </div>
      </div>
      <div class="last-update" id="power-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- HCHO Sensor Card -->
    <div class="sensor-card" id="hcho-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon env-icon">🧪</div>
        <div class="sensor-title">HCHO Sensor</div>
        <div class="status-indicator" id="hcho-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">HCHO</div>
          <div class="data-value" id="hcho-value">--<span class="data-unit">mg/m³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Age</div>
          <div class="data-value" id="hcho-age">--<span class="data-unit">s</span></div>
        </div>
      </div>
      <div class="last-update" id="hcho-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- IPS Sensor Card -->
    <div class="sensor-card" id="ips-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon air-icon">🌫️</div>
        <div class="sensor-title">IPS Sensor</div>
        <div class="status-indicator" id="ips-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">PC 0.3</div>
          <div class="data-value" id="ips-pc03">--<span class="data-unit">#/cm³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PC 0.5</div>
          <div class="data-value" id="ips-pc05">--<span class="data-unit">#/cm³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PC 1.0</div>
          <div class="data-value" id="ips-pc10">--<span class="data-unit">#/cm³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PC 2.5</div>
          <div class="data-value" id="ips-pc25">--<span class="data-unit">#/cm³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PC 5.0</div>
          <div class="data-value" id="ips-pc50">--<span class="data-unit">#/cm³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PC 10.0</div>
          <div class="data-value" id="ips-pc100">--<span class="data-unit">#/cm³</span></div>
        </div>
      </div>
      <div class="last-update" id="ips-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- Calibration Gases Card -->
    <div class="sensor-card" id="calib-gases-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon env-icon">🧪</div>
        <div class="sensor-title">Gazy Elektrochemiczne</div>
        <div class="status-indicator" id="calib-gases-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">CO</div>
          <div class="data-value" id="calib-co">--<span class="data-unit">ppb</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">NO</div>
          <div class="data-value" id="calib-no">--<span class="data-unit">ppb</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">NO2</div>
          <div class="data-value" id="calib-no2">--<span class="data-unit">ppb</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">O3</div>
          <div class="data-value" id="calib-o3">--<span class="data-unit">ppb</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">SO2</div>
          <div class="data-value" id="calib-so2">--<span class="data-unit">ppb</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">H2S</div>
          <div class="data-value" id="calib-h2s">--<span class="data-unit">ppb</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">NH3</div>
          <div class="data-value" id="calib-nh3">--<span class="data-unit">ppb</span></div>
        </div>
      </div>
      <div class="last-update" id="calib-gases-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- Calibration TGS Card -->
    <div class="sensor-card" id="calib-tgs-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon adc-icon">🔬</div>
        <div class="sensor-title">Czujniki TGS</div>
        <div class="status-indicator" id="calib-tgs-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">TGS02</div>
          <div class="data-value" id="calib-tgs02">--<span class="data-unit">ppm</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">TGS03</div>
          <div class="data-value" id="calib-tgs03">--<span class="data-unit">ppm</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">TGS12</div>
          <div class="data-value" id="calib-tgs12">--<span class="data-unit">ppm</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">TGS02 Ω</div>
          <div class="data-value" id="calib-tgs02-ohm">--<span class="data-unit">Ω</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">TGS03 Ω</div>
          <div class="data-value" id="calib-tgs03-ohm">--<span class="data-unit">Ω</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">TGS12 Ω</div>
          <div class="data-value" id="calib-tgs12-ohm">--<span class="data-unit">Ω</span></div>
        </div>
      </div>
      <div class="last-update" id="calib-tgs-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- Calibration Temperatures Card -->
    <div class="sensor-card" id="calib-temps-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon env-icon">🌡️</div>
        <div class="sensor-title">Temperatury Czujników</div>
        <div class="status-indicator" id="calib-temps-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">K1</div>
          <div class="data-value" id="calib-temp-k1">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">K2</div>
          <div class="data-value" id="calib-temp-k2">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">K3</div>
          <div class="data-value" id="calib-temp-k3">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">K4</div>
          <div class="data-value" id="calib-temp-k4">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">K5</div>
          <div class="data-value" id="calib-temp-k5">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">K6</div>
          <div class="data-value" id="calib-temp-k6">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">K7</div>
          <div class="data-value" id="calib-temp-k7">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">K8</div>
          <div class="data-value" id="calib-temp-k8">--<span class="data-unit">°C</span></div>
        </div>
      </div>
      <div class="last-update" id="calib-temps-update">Ostatnia aktualizacja: --</div>
    </div>
  </div>

<script>
let ws;
let lastUpdateTime = Date.now();

function formatTime(timestamp) {
  return new Date(timestamp).toLocaleTimeString('pl-PL');
}

function formatUptime(seconds) {
  const days = Math.floor(seconds / 86400);
  const hours = Math.floor((seconds % 86400) / 3600);
  const mins = Math.floor((seconds % 3600) / 60);
  const secs = Math.floor(seconds % 60);
  
  if (days > 0) return `${days}d ${hours}h`;
  if (hours > 0) return `${hours}h ${mins}m`;
  if (mins > 0) return `${mins}m ${secs}s`;
  return `${secs}s`;
}

function updateStatus(elementId, isOk) {
  const element = document.getElementById(elementId);
  if (element) {
    element.className = `status-indicator ${isOk ? 'status-ok' : 'status-error'}`;
  }
}

function updateValue(elementId, value, precision = 1) {
  const element = document.getElementById(elementId);
  if (element && value !== undefined && value !== null) {
    if (typeof value === 'number') {
      element.textContent = value.toFixed(precision);
    } else {
      element.textContent = value;
    }
  }
}

function showCard(cardId) {
  const card = document.getElementById(cardId);
  if (card) {
    card.style.display = 'block';
  }
}

function hideCard(cardId) {
  const card = document.getElementById(cardId);
  if (card) {
    card.style.display = 'none';
  }
}

function createMCP3424Card(device, address) {
  return `
    <div class="sensor-card" id="mcp3424-device-${device}">
      <div class="sensor-header">
        <div class="sensor-icon adc-icon">📈</div>
        <div class="sensor-title">MCP3424 Device ${device} (${address})</div>
        <div class="status-indicator" id="mcp3424-status-${device}"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">Channel 1</div>
          <div class="data-value" id="mcp3424-${device}-ch1">--<span class="data-unit">V</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Channel 2</div>
          <div class="data-value" id="mcp3424-${device}-ch2">--<span class="data-unit">V</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Channel 3</div>
          <div class="data-value" id="mcp3424-${device}-ch3">--<span class="data-unit">V</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Channel 4</div>
          <div class="data-value" id="mcp3424-${device}-ch4">--<span class="data-unit">V</span></div>
        </div>
      </div>
      <div class="last-update" id="mcp3424-update-${device}">Ostatnia aktualizacja: --</div>
    </div>
  `;
}

function updateMCP3424Devices(adcData) {
  const container = document.getElementById('adc-sensors-container');
  if (!container) return;
  
  if (adcData.mcp3424 && adcData.mcp3424.enabled && adcData.mcp3424.deviceCount > 0) {
    // Clear existing MCP3424 cards
    container.innerHTML = '';
    
    // Create cards for each device
    adcData.mcp3424.devices.forEach((device, index) => {
      container.innerHTML += createMCP3424Card(index, device.address);
      
      // Update values
      updateValue(`mcp3424-${index}-ch1`, device.channels.ch1, 6);
      updateValue(`mcp3424-${index}-ch2`, device.channels.ch2, 6);
      updateValue(`mcp3424-${index}-ch3`, device.channels.ch3, 6);
      updateValue(`mcp3424-${index}-ch4`, device.channels.ch4, 6);
      updateStatus(`mcp3424-status-${index}`, device.valid);
      
      const updateElement = document.getElementById(`mcp3424-update-${index}`);
      if (updateElement) {
        updateElement.textContent = `Ostatnia aktualizacja: ${formatTime(Date.now())}`;
      }
    });
  } else {
    container.innerHTML = '';
  }
}

function connectWebSocket() {
  ws = new WebSocket(`ws://${window.location.host}/ws`);
  ws.onopen = function() {
    console.log('WebSocket connected');
  };
  ws.onclose = function() {
    console.log('WebSocket disconnected, reconnecting...');
    setTimeout(connectWebSocket, 5000);
  };
  ws.onmessage = function(event) {
    try {
      const data = JSON.parse(event.data);
      lastUpdateTime = Date.now();
      
      // Show/hide cards based on sensor availability
      if (data.sensorsEnabled) {
        // Solar sensor
        if (data.sensorsEnabled.solar) {
          showCard('solar-card');
        } else {
          hideCard('solar-card');
        }
        
        // OPCN3 sensor
        if (data.sensorsEnabled.opcn3) {
          showCard('opcn3-card');
        } else {
          hideCard('opcn3-card');
        }
        
        // I2C environmental sensors
        if (data.sensorsEnabled.i2c) {
          showCard('i2c-card');
        } else {
          hideCard('i2c-card');
        }
        
        // SCD41 CO2 sensor
        if (data.sensorsEnabled.scd41) {
          showCard('scd41-card');
        } else {
          hideCard('scd41-card');
        }
        
        // SPS30 particle sensor
        if (data.sensorsEnabled.sps30) {
          showCard('sps30-card');
        } else {
          hideCard('sps30-card');
        }
        
        // ADS1110 sensor
        if (data.sensorsEnabled.ads1110) {
          showCard('ads1110-card');
        } else {
          hideCard('ads1110-card');
        }
        
        // INA219 power monitor
        if (data.sensorsEnabled.ina219) {
          showCard('power-card');
        } else {
          hideCard('power-card');
        }
        
        // HCHO sensor
        if (data.sensorsEnabled.hcho) {
          showCard('hcho-card');
        } else {
          hideCard('hcho-card');
        }
        
        // IPS sensor
        if (data.sensorsEnabled.ips) {
          showCard('ips-card');
        } else {
          hideCard('ips-card');
        }
        
        // Calibration cards
        if (data.calibration && data.calibration.enabled && data.calibration.valid) {
          showCard('calib-gases-card');
          showCard('calib-tgs-card');
          showCard('calib-temps-card');
        } else {
          hideCard('calib-gases-card');
          hideCard('calib-tgs-card');
          hideCard('calib-temps-card');
        }
      }
      
      // System status
      updateValue('uptime', formatUptime(data.uptime || 0));
      updateValue('free-heap', (data.freeHeap || 0) / 1024, 1);
      updateValue('wifi-signal', data.wifiSignal || 0, 0);
      updateStatus('system-status', true);
      document.getElementById('system-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      
      // Solar data
      if (data.solar && data.solar.valid) {
        updateValue('solar-voltage', parseFloat(data.solar.V || '0'), 2);
        updateValue('solar-current', parseFloat(data.solar.I || '0'), 3);
        updateValue('solar-vpv', parseFloat(data.solar.VPV || '0'), 2);
        updateValue('solar-ppv', parseFloat(data.solar.PPV || '0'), 1);
        updateStatus('solar-status', true);
        document.getElementById('solar-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      } else {
        updateStatus('solar-status', false);
      }
      
      // OPCN3 data
      if (data.opcn3 && data.opcn3.valid) {
        updateValue('pm1', data.opcn3.PM1 || 0, 1);
        updateValue('pm25', data.opcn3.PM25 || 0, 1);
        updateValue('pm10', data.opcn3.PM10 || 0, 1);
        updateValue('opcn3-temp', data.opcn3.temperature || 0, 1);
        updateStatus('opcn3-status', true);
        document.getElementById('opcn3-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      } else {
        updateStatus('opcn3-status', false);
      }
      
      // I2C environmental data
      if (data.i2c && data.i2c.valid) {
        updateValue('env-temp', data.i2c.temperature || 0, 1);
        updateValue('env-humidity', data.i2c.humidity || 0, 1);
        updateValue('env-pressure', data.i2c.pressure || 0, 1);
        updateStatus('i2c-status', true);
        document.getElementById('i2c-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      } else {
        updateStatus('i2c-status', false);
      }
      
      // SCD41 CO2 data
      if (data.scd41 && data.scd41.valid) {
        updateValue('scd41-co2', data.scd41.CO2 || 0, 0);
        updateValue('scd41-temp', data.scd41.temperature || 0, 1);
        updateValue('scd41-humidity', data.scd41.humidity || 0, 1);
        updateStatus('scd41-status', true);
        document.getElementById('scd41-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      } else {
        updateStatus('scd41-status', false);
      }
      
      // SPS30 particle sensor data
      if (data.sps30 && data.sps30.valid) {
        updateValue('sps30-pm1', data.sps30.PM1 || 0, 1);
        updateValue('sps30-pm25', data.sps30.PM25 || 0, 1);
        updateValue('sps30-pm4', data.sps30.PM4 || 0, 1);
        updateValue('sps30-pm10', data.sps30.PM10 || 0, 1);
        updateValue('sps30-size', data.sps30.TPS || 0, 1);
        updateValue('sps30-nc05', data.sps30.NC05 || 0, 1);
        updateValue('sps30-nc1', data.sps30.NC1 || 0, 1);
        updateValue('sps30-nc25', data.sps30.NC25 || 0, 1);
        updateStatus('sps30-status', true);
        document.getElementById('sps30-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      } else {
        updateStatus('sps30-status', false);
      }
      
      // ADC data - Update MCP3424 devices dynamically
      if (data.adc) {
        updateMCP3424Devices(data.adc);
        
        // ADS1110 data
        if (data.adc.ads1110 && data.adc.ads1110.enabled) {
          updateValue('ads-voltage', data.adc.ads1110.voltage || 0, 6);
          updateValue('ads-rate', data.adc.ads1110.dataRate || 0, 0);
          updateValue('ads-gain', data.adc.ads1110.gain || 0, 0);
          updateStatus('ads1110-status', data.adc.ads1110.valid);
          document.getElementById('ads1110-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
        } else {
          updateStatus('ads1110-status', false);
        }
      }
      
      // Power monitor data
      if (data.power && data.power.valid) {
        updateValue('bus-voltage', data.power.busVoltage || 0, 3);
        updateValue('current', data.power.current || 0, 1);
        updateValue('power', data.power.power || 0, 1);
        updateValue('shunt-voltage', data.power.shuntVoltage || 0, 2);
        updateStatus('power-status', true);
        document.getElementById('power-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      } else {
        updateStatus('power-status', false);
      }
      
      // HCHO sensor data
      if (data.hcho && data.hcho.valid) {
        updateValue('hcho-value', data.hcho.HCHO || 0, 3);
        updateValue('hcho-age', data.hcho.age || 0, 0);
        updateStatus('hcho-status', true);
        document.getElementById('hcho-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      } else {
        updateStatus('hcho-status', false);
      }
      
      // IPS sensor data
      if (data.ips && data.ips.valid) {
        updateValue('ips-pc03', data.ips.PC[0] || 0, 0);
        updateValue('ips-pc05', data.ips.PC[1] || 0, 0);
        updateValue('ips-pc10', data.ips.PC[2] || 0, 0);
        updateValue('ips-pc25', data.ips.PC[3] || 0, 0);
        updateValue('ips-pc50', data.ips.PC[4] || 0, 0);
        updateValue('ips-pc100', data.ips.PC[5] || 0, 0);
        updateStatus('ips-status', true);
        document.getElementById('ips-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      } else {
        updateStatus('ips-status', false);
      }
      
      // Calibration data
      if (data.calibration && data.calibration.enabled && data.calibration.valid) {
        // Gases (ppb)
        if (data.calibration.gases_ppb) {
          updateValue('calib-co', data.calibration.gases_ppb.CO || 0, 1);
          updateValue('calib-no', data.calibration.gases_ppb.NO || 0, 1);
          updateValue('calib-no2', data.calibration.gases_ppb.NO2 || 0, 1);
          updateValue('calib-o3', data.calibration.gases_ppb.O3 || 0, 1);
          updateValue('calib-so2', data.calibration.gases_ppb.SO2 || 0, 1);
          updateValue('calib-h2s', data.calibration.gases_ppb.H2S || 0, 1);
          updateValue('calib-nh3', data.calibration.gases_ppb.NH3 || 0, 1);
          updateStatus('calib-gases-status', true);
          document.getElementById('calib-gases-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
        } else {
          updateStatus('calib-gases-status', false);
        }
        
        // TGS sensors
        if (data.calibration.tgs) {
          updateValue('calib-tgs02', data.calibration.tgs.TGS02 || 0, 3);
          updateValue('calib-tgs03', data.calibration.tgs.TGS03 || 0, 3);
          updateValue('calib-tgs12', data.calibration.tgs.TGS12 || 0, 3);
          updateValue('calib-tgs02-ohm', data.calibration.tgs.TGS02_ohm || 0, 0);
          updateValue('calib-tgs03-ohm', data.calibration.tgs.TGS03_ohm || 0, 0);
          updateValue('calib-tgs12-ohm', data.calibration.tgs.TGS12_ohm || 0, 0);
          updateStatus('calib-tgs-status', true);
          document.getElementById('calib-tgs-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
        } else {
          updateStatus('calib-tgs-status', false);
        }
        
        // Temperatures
        if (data.calibration.temperatures) {
          updateValue('calib-temp-k1', data.calibration.temperatures.K1 || 0, 1);
          updateValue('calib-temp-k2', data.calibration.temperatures.K2 || 0, 1);
          updateValue('calib-temp-k3', data.calibration.temperatures.K3 || 0, 1);
          updateValue('calib-temp-k4', data.calibration.temperatures.K4 || 0, 1);
          updateValue('calib-temp-k5', data.calibration.temperatures.K5 || 0, 1);
          updateValue('calib-temp-k6', data.calibration.temperatures.K6 || 0, 1);
          updateValue('calib-temp-k7', data.calibration.temperatures.K7 || 0, 1);
          updateValue('calib-temp-k8', data.calibration.temperatures.K8 || 0, 1);
          updateStatus('calib-temps-status', true);
          document.getElementById('calib-temps-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
        } else {
          updateStatus('calib-temps-status', false);
        }
      } else {
        updateStatus('calib-gases-status', false);
        updateStatus('calib-tgs-status', false);
        updateStatus('calib-temps-status', false);
      }
      
    } catch (error) {
      console.error('Error parsing sensor data:', error);
    }
  };
}

// Check for connection timeout and show offline status
setInterval(function() {
  const timeSinceUpdate = Date.now() - lastUpdateTime;
  if (timeSinceUpdate > 60000) { // 1 minute timeout
    updateStatus('system-status', false);
    document.getElementById('system-update').textContent = 'Połączenie utracone - próba reconnect...';
  }
}, 10000);

// Initialize on page load
window.addEventListener('load', function() {
  connectWebSocket();
});

// Cleanup on page unload
window.addEventListener('beforeunload', function() {
  if (ws) ws.close();
});
</script>
</body>
</html>
)rawliteral";
