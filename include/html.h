
const char *update_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<style>
  body {
    background-color: #ff00ff;
    background-image: linear-gradient(45deg, #ffcc00, #00ffcc, #cc00ff);
    background-size: 200% 200%;
    animation: gradient 5s infinite;
    font-family: 'Arial', sans-serif;
  }

  @keyframes gradient {
    0% {
      background-position: 0% 50%;
    }
    50% {
      background-position: 100% 50%;
    }
    100% {
      background-position: 0% 50%;
    }
  }

  form, .webserial-link, .auto-reset {
    margin: 50px;
    padding: 50px;
    background-color: rgba(255, 255, 255, 0.8);
    border-radius: 25px;
    box-shadow: 0 0 10px 0 rgba(0, 0, 0, 0.5);
    text-align: center;
  }

  input[type="file"], .webserial-btn, #toggleAutoReset {
    color: #000;
    background-color: #ff69b4;
    border: 2px solid #000;
    padding: 20px;
    cursor: pointer;
    font-weight: bold;
    margin-top: 20px;
    width: 80%;
    max-width: 300px;
    display: inline-block; /* Centruje przycisk w kontenerze */
    text-align: center;
  }

  input[type="submit"], .webserial-btn, #toggleAutoReset {
    background-color: #009688;
    color: #fff;
    border: none;
    border-radius: 25px;
    font-size: 1.2em;
    cursor: pointer;
    box-shadow: 0 5px 15px rgba(0, 0, 0, 0.4);
    transition: transform 0.2s, background-color 0.3s ease;
    width: auto; /* Dostosowuje szerokość do treści */
    padding: 15px 45px;
  }

  input[type="submit"]:hover, .webserial-btn:hover, #toggleAutoReset:hover {
    transform: scale(1.05);
    background-color: #00796b;
  }

  #autoResetState {
    font-weight: bold;
    margin-top: 20px;
  }
</style>
</head>
<body>
<form method="POST" action="/update" enctype="multipart/form-data">
<input type="file" name="update">
<input type="submit" value="Update Firmware">
</form>

<div class="auto-reset">
<button id="toggleAutoReset">Toggle Auto Reset</button>
<p id="autoResetState">Auto Reset is </p>
</div>

<!-- Navigation Links -->
<div class="webserial-link">
<a href="/dashboard" class="webserial-btn"> open Live Dashboard</a>
</div>

<div class="webserial-link">
<a href="/webserial" class="webserial-btn"> Open WebSerial</a>
</div>
<script>
function updateAutoResetState(state) {
  document.getElementById("autoResetState").innerHTML = "Auto Reset is " + (state == "true" ? "Enabled" : "Disabled");
}

document.getElementById("toggleAutoReset").addEventListener("click", function() {
  var xhr = new XMLHttpRequest();
  xhr.open("POST", "/toggleAutoReset", true);
  xhr.onreadystatechange = function() {
    if (xhr.readyState == 4 && xhr.status == 200) {
      updateAutoResetState(xhr.responseText);
    }
  }
  xhr.send();
});

window.onload = function() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/getAutoReset", true);
  xhr.onreadystatechange = function() {
    if (xhr.readyState == 4 && xhr.status == 200) {
      updateAutoResetState(xhr.responseText);
    }
  }
  xhr.send();
};
</script>
<script>
var source = new EventSource("/events");
source.addEventListener('serialData', function(event) {
  document.getElementById("espData").innerHTML = "ESP Data: " + event.data;
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
    <a href="/">🔧 Update Firmware</a>
    <a href="/dashboard">📊 Dashboard</a>
    <a href="/webserial">💻 WebSerial</a>
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
        <div class="data-item">
          <div class="data-label">CO2</div>
          <div class="data-value" id="env-co2">--<span class="data-unit">ppm</span></div>
        </div>
      </div>
      <div class="last-update" id="i2c-update">Ostatnia aktualizacja: --</div>
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
  </div>

<script>
let eventSource;
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

function connectEventSource() {
  if (eventSource) {
    eventSource.close();
  }
  
  eventSource = new EventSource('/events');
  
  eventSource.addEventListener('sensorData', function(event) {
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
        updateValue('pm1', data.opcn3.pm1 || 0, 1);
        updateValue('pm25', data.opcn3.pm2_5 || 0, 1);
        updateValue('pm10', data.opcn3.pm10 || 0, 1);
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
        updateValue('env-co2', data.i2c.co2 || 0, 0);
        updateStatus('i2c-status', true);
        document.getElementById('i2c-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      } else {
        updateStatus('i2c-status', false);
      }
      
      // SPS30 particle sensor data
      if (data.sps30 && data.sps30.valid) {
        updateValue('sps30-pm1', data.sps30.pm1_0 || 0, 1);
        updateValue('sps30-pm25', data.sps30.pm2_5 || 0, 1);
        updateValue('sps30-pm4', data.sps30.pm4_0 || 0, 1);
        updateValue('sps30-pm10', data.sps30.pm10 || 0, 1);
        updateValue('sps30-size', data.sps30.typical_particle_size || 0, 1);
        updateValue('sps30-nc05', data.sps30.nc0_5 || 0, 1);
        updateValue('sps30-nc1', data.sps30.nc1_0 || 0, 1);
        updateValue('sps30-nc25', data.sps30.nc2_5 || 0, 1);
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
      
    } catch (error) {
      console.error('Error parsing sensor data:', error);
    }
  });
  
  eventSource.addEventListener('error', function(event) {
    console.error('EventSource error:', event);
    setTimeout(connectEventSource, 5000); // Reconnect after 5 seconds
  });
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
  connectEventSource();
});

// Cleanup on page unload
window.addEventListener('beforeunload', function() {
  if (eventSource) {
    eventSource.close();
  }
});
</script>
</body>
</html>
)rawliteral";