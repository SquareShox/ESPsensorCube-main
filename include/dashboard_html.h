#ifndef DASHBOARD_HTML_H
#define DASHBOARD_HTML_H
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
<script src="/common.js"></script>
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
    <a href="/network">🌐 Sieć</a>
    <a href="/mcp3424">🔌 MCP3424</a>
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
        <div class="data-item">
          <div class="data-label">NC4.0</div>
          <div class="data-value" id="sps30-nc4">--<span class="data-unit">#/cm³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">NC10</div>
          <div class="data-value" id="sps30-nc10">--<span class="data-unit">#/cm³</span></div>
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
          <div class="data-label">HCHO</div>
          <div class="data-value" id="hcho-ppb">--<span class="data-unit">ppb</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">TVOC</div>
          <div class="data-value" id="hcho-tvoc">--<span class="data-unit">mg/m³</span></div>
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
        <div class="data-item">
          <div class="data-label">VOC</div>
          <div class="data-value" id="calib-voc">--<span class="data-unit">ppb</span></div>
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

    <!-- Calibration PM Sensors Card -->
    <div class="sensor-card" id="calib-pm-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon air-icon">💨</div>
        <div class="sensor-title">Skalibrowane PM (SPS30)</div>
        <div class="status-indicator" id="calib-pm-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">PM1.0</div>
          <div class="data-value" id="calib-pm1">--<span class="data-unit">µg/m³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PM2.5</div>
          <div class="data-value" id="calib-pm25">--<span class="data-unit">µg/m³</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PM10</div>
          <div class="data-value" id="calib-pm10">--<span class="data-unit">µg/m³</span></div>
        </div>
      </div>
      <div class="last-update" id="calib-pm-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- Calibration Environmental Sensors Card -->
    <div class="sensor-card" id="calib-env-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon env-icon">🌡️</div>
        <div class="sensor-title">Skalibrowane Środowiskowe</div>
        <div class="status-indicator" id="calib-env-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">AMBIENT Temp</div>
          <div class="data-value" id="calib-ambient-temp">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">AMBIENT Humid</div>
          <div class="data-value" id="calib-ambient-humid">--<span class="data-unit">%</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">AMBIENT Press</div>
          <div class="data-value" id="calib-ambient-press">--<span class="data-unit">hPa</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">DUST Temp</div>
          <div class="data-value" id="calib-dust-temp">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">DUST Humid</div>
          <div class="data-value" id="calib-dust-humid">--<span class="data-unit">%</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">DUST Press</div>
          <div class="data-value" id="calib-dust-press">--<span class="data-unit">hPa</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">GAS Temp</div>
          <div class="data-value" id="calib-gas-temp">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">GAS Humid</div>
          <div class="data-value" id="calib-gas-humid">--<span class="data-unit">%</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">GAS Press</div>
          <div class="data-value" id="calib-gas-press">--<span class="data-unit">hPa</span></div>
        </div>
      </div>
      <div class="last-update" id="calib-env-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- Calibration CO2 Sensor Card -->
    <div class="sensor-card" id="calib-co2-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon env-icon">🌬️</div>
        <div class="sensor-title">Skalibrowany CO2 (SCD41)</div>
        <div class="status-indicator" id="calib-co2-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">CO2</div>
          <div class="data-value" id="calib-co2">--<span class="data-unit">ppm</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Temperatura</div>
          <div class="data-value" id="calib-co2-temp">--<span class="data-unit">°C</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">Wilgotność</div>
          <div class="data-value" id="calib-co2-humid">--<span class="data-unit">%</span></div>
        </div>
      </div>
      <div class="last-update" id="calib-co2-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- Calibration Special Sensors Card -->
    <div class="sensor-card" id="calib-special-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon env-icon">🔬</div>
        <div class="sensor-title">Czujniki Specjalne</div>
        <div class="status-indicator" id="calib-special-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">HCHO</div>
          <div class="data-value" id="calib-hcho">--<span class="data-unit">ppb</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PID</div>
          <div class="data-value" id="calib-pid">--<span class="data-unit">ppm</span></div>
        </div>
        <div class="data-item">
          <div class="data-label">PID mV</div>
          <div class="data-value" id="calib-pid-mv">--<span class="data-unit">mV</span></div>
        </div>
      </div>
      <div class="last-update" id="calib-special-update">Ostatnia aktualizacja: --</div>
    </div>

    <!-- Calibration ODO Card -->
    <div class="sensor-card" id="calib-odo-card" style="display: none;">
      <div class="sensor-header">
        <div class="sensor-icon env-icon">🌊</div>
        <div class="sensor-title">ODO (Dissolved Oxygen)</div>
        <div class="status-indicator" id="calib-odo-status"></div>
      </div>
      <div class="sensor-data">
        <div class="data-item">
          <div class="data-label">ODO</div>
          <div class="data-value" id="calib-odo">--<span class="data-unit">mg/L</span></div>
        </div>
      </div>
      <div class="last-update" id="calib-odo-update">Ostatnia aktualizacja: --</div>
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
  } else {
    console.log(`Failed to update ${elementId}: element=${!!element}, value=${value}`);
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
  if (!container) {
    console.log('MCP3424 container not found');
    return;
  }
  
  console.log('Updating MCP3424 devices:', adcData.mcp3424);
  
  if (adcData.mcp3424 && adcData.mcp3424.enabled && adcData.mcp3424.deviceCount > 0) {
    // Clear existing MCP3424 cards
    container.innerHTML = '';
    
    console.log('Creating cards for', adcData.mcp3424.deviceCount, 'devices');
    
    // Create cards for each device
    adcData.mcp3424.devices.forEach((device, index) => {
      console.log('Device', index, ':', device);
      container.innerHTML += createMCP3424Card(index, device.address);
      
      // Update values
      updateValue(`mcp3424-${index}-ch1`, device.channels.ch1, 1);
      updateValue(`mcp3424-${index}-ch2`, device.channels.ch2, 1);
      updateValue(`mcp3424-${index}-ch3`, device.channels.ch3, 1);
      updateValue(`mcp3424-${index}-ch4`, device.channels.ch4, 1);
      updateStatus(`mcp3424-status-${index}`, device.valid);
      
      const updateElement = document.getElementById(`mcp3424-update-${index}`);
      if (updateElement) {
        updateElement.textContent = `Ostatnia aktualizacja: ${formatTime(Date.now())}`;
      }
    });
  } else {
    console.log('MCP3424 not enabled or no devices');
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
      
      console.log('Received WebSocket data:', data);
      
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
        
        // I2C environmental sensors (SHT40 status)
        if (data.sensorsEnabled.sht40) {
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
          
          // Show special sensors cards if enabled
          if (data.calibration.config && data.calibration.config.specialSensors) {
            showCard('calib-pm-card');
            showCard('calib-env-card');
            showCard('calib-co2-card');
            showCard('calib-odo-card');
            showCard('calib-special-card');
          } else {
            hideCard('calib-pm-card');
            hideCard('calib-env-card');
            hideCard('calib-co2-card');
            hideCard('calib-odo-card');
            hideCard('calib-special-card');
          }
        } else {
          hideCard('calib-gases-card');
          hideCard('calib-tgs-card');
          hideCard('calib-temps-card');
          hideCard('calib-pm-card');
          hideCard('calib-env-card');
          hideCard('calib-co2-card');
          hideCard('calib-odo-card');
          hideCard('calib-special-card');
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
      
      // I2C environmental data (SHT40)
      if (data.sht40 && data.sht40.valid) {
        updateValue('env-temp', data.sht40.temperature, 1);
        updateValue('env-humidity', data.sht40.humidity, 1);
        updateValue('env-pressure', data.sht40.pressure, 1);
        updateStatus('i2c-status', true);
        document.getElementById('i2c-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      } else {
        updateStatus('i2c-status', false);
      }
      
      // SCD41 CO2 data
      if (data.scd41 && data.scd41.valid) {
        console.log('SCD41 data:', data.scd41);
        updateValue('scd41-co2', data.scd41.co2, 0);
        if (typeof data.scd41.temperature !== 'undefined') {
          updateValue('scd41-temp', data.scd41.temperature, 1);
        }
        if (typeof data.scd41.humidity !== 'undefined') {
          updateValue('scd41-humidity', data.scd41.humidity, 1);
        }
        updateStatus('scd41-status', true);
        document.getElementById('scd41-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      } else {
        console.log('SCD41 not valid or missing:', data.scd41);
        updateStatus('scd41-status', false);
      }
      
      // SPS30 particle sensor data
      if (data.sps30 && data.sps30.valid) {
        console.log('SPS30 data:', data.sps30);
        updateValue('sps30-pm1', data.sps30.PM1 || 0, 1);
        updateValue('sps30-pm25', data.sps30.PM25 || 0, 1);
        updateValue('sps30-pm4', data.sps30.PM4 || 0, 1);
        updateValue('sps30-pm10', data.sps30.PM10 || 0, 1);
        updateValue('sps30-size', data.sps30.TPS || 0, 1);
        updateValue('sps30-nc05', data.sps30.NC05 || 0, 1);
        updateValue('sps30-nc1', data.sps30.NC1 || 0, 1);
        updateValue('sps30-nc25', data.sps30.NC25 || 0, 1);
        updateValue('sps30-nc4', data.sps30.NC4 || 0, 1);
        updateValue('sps30-nc10', data.sps30.NC10 || 0, 1);
        updateStatus('sps30-status', true);
        document.getElementById('sps30-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
      } else {
        console.log('SPS30 not valid or missing:', data.sps30);
        updateStatus('sps30-status', false);
      }
      
      // MCP3424 devices
      if (data.mcp3424 && data.mcp3424.enabled) {
        console.log('MCP3424 data:', data.mcp3424);
        updateMCP3424Devices(data);
      }
        
        // ADS1110 data
      if (data.ads1110 && data.ads1110.enabled) {
        console.log('ADS1110 data:', data.ads1110);
        updateValue('ads-voltage', data.ads1110.voltage || 0, 6);
        updateValue('ads-rate', data.ads1110.dataRate || 0, 0);
        updateValue('ads-gain', data.ads1110.gain || 0, 0);
        updateStatus('ads1110-status', data.ads1110.valid);
          document.getElementById('ads1110-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
        } else {
        console.log('ADS1110 not valid or missing:', data.ads1110);
          updateStatus('ads1110-status', false);
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
        updateValue('hcho-value', data.hcho.hcho_mg || 0, 3);
        updateValue('hcho-ppb', data.hcho.hcho_ppb || 0, 1);
        updateValue('hcho-tvoc', data.hcho.tvoc_mg || 0, 3);
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
          updateValue('calib-voc', data.calibration.gases_ppb.VOC || 0, 1);
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
        
        // Special sensors (PM, Environmental, CO2, ODO)
        if (data.calibration.special) {
          // PM sensors
          updateValue('calib-pm1', data.calibration.special.PM1 || 0, 1);
          updateValue('calib-pm25', data.calibration.special.PM25 || 0, 1);
          updateValue('calib-pm10', data.calibration.special.PM10 || 0, 1);
          updateStatus('calib-pm-status', true);
          document.getElementById('calib-pm-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
          
          // Environmental sensors
          updateValue('calib-ambient-temp', data.calibration.special.AMBIENT_TEMP || 0, 1);
          updateValue('calib-ambient-humid', data.calibration.special.AMBIENT_HUMID || 0, 1);
          updateValue('calib-ambient-press', data.calibration.special.AMBIENT_PRESS || 0, 1);
          updateValue('calib-dust-temp', data.calibration.special.DUST_TEMP || 0, 1);
          updateValue('calib-dust-humid', data.calibration.special.DUST_HUMID || 0, 1);
          updateValue('calib-dust-press', data.calibration.special.DUST_PRESS || 0, 1);
          updateValue('calib-gas-temp', data.calibration.special.GAS_TEMP || 0, 1);
          updateValue('calib-gas-humid', data.calibration.special.GAS_HUMID || 0, 1);
          updateValue('calib-gas-press', data.calibration.special.GAS_PRESS || 0, 1);
          updateStatus('calib-env-status', true);
          document.getElementById('calib-env-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
          
          // CO2 sensor
          updateValue('calib-co2', data.calibration.special.SCD_CO2 || 0, 1);
          updateValue('calib-co2-temp', data.calibration.special.SCD_T || 0, 1);
          updateValue('calib-co2-humid', data.calibration.special.SCD_RH || 0, 1);
          updateStatus('calib-co2-status', true);
          document.getElementById('calib-co2-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
          
          // Special sensors (HCHO, PID)
          updateValue('calib-hcho', data.calibration.special.HCHO_ppb || 0, 1);
          updateValue('calib-pid', data.calibration.special.PID || 0, 3);
          updateValue('calib-pid-mv', data.calibration.special.PID_mV || 0, 1);
          updateStatus('calib-special-status', true);
          document.getElementById('calib-special-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
          
          // ODO
          updateValue('calib-odo', data.calibration.special.ODO || 0, 1);
          updateStatus('calib-odo-status', true);
          document.getElementById('calib-odo-update').textContent = `Ostatnia aktualizacja: ${formatTime(lastUpdateTime)}`;
        } else {
          updateStatus('calib-pm-status', false);
          updateStatus('calib-env-status', false);
          updateStatus('calib-co2-status', false);
          updateStatus('calib-odo-status', false);
          updateStatus('calib-special-status', false);
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
#endif // DASHBOARD_HTML_H
