#ifndef CHART_H
#define CHART_H

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
  }
</style>
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
      <option value="calibration">Kalibracja (Gazy)</option>
      <option value="fan">Wentylator</option>
    </select>
    
    <button onclick="updateCharts()">🔄 Odśwież</button>
    <button onclick="toggleAutoUpdate()">⏰ Auto: <span id="auto-status">OFF</span></button>
    <button onclick="window.location.href='/network'" style="background: linear-gradient(45deg, #4caf50, #8bc34a);">🌐 Sieć</button>
    <button onclick="window.location.href='/mcp3424'" style="background: linear-gradient(45deg, #ff9800, #ffc107);">🔌 MCP3424</button>
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
      
      // Handle different WebSocket commands
      if (data.cmd === 'history') {
        // Handle history data from WebSocket
        handleHistoryResponse(data);
      } else {
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
  
  if (data.history) {
    const memoryPercent = data.history.memoryUsed > 0 ? Math.round((data.history.memoryUsed / data.history.memoryBudget) * 100) : 0;
    document.getElementById('history-status').textContent = 
      `Historia: ${data.history.enabled ? '✅' : '❌'} (${memoryPercent}% pamięci)`;
  }
}

// Global variable to store pending history request
let pendingHistoryRequest = null;

function handleHistoryResponse(data) {
  if (pendingHistoryRequest) {
    const { sensorType, timeRange, sampleType, container } = pendingHistoryRequest;
    pendingHistoryRequest = null;
    
    if (data.error) {
      container.innerHTML = `<div class="no-data">❌ Błąd: ${data.error}</div>`;
      return;
    }
    
    if (!data.data || data.data.length === 0) {
      container.innerHTML = '<div class="no-data">📭 Brak danych dla wybranego czujnika</div>';
      return;
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
  
  return historyData.map(entry => {
    // Użyj timestamp bezpośrednio (jeśli to epoch time) lub konwertuj
    let timestamp;
    if (entry.timestamp > 1600000000000) { // Jeśli to już epoch time (po 2020)
      timestamp = entry.timestamp;
    } else {
      // Konwertuj millis() na epoch time
      const now = Date.now();
      const systemUptime = Math.floor(now / 1000);
      const entryTime = entry.timestamp / 1000;
      timestamp = (systemUptime - entryTime) * 1000;
    }
    
    return {
      x: new Date(timestamp),
      y: parseFloat(entry.data[valueKey]) || entry.data[valueKey]
    };
  }).filter(point => point.y !== undefined && point.y !== null && !isNaN(point.y));
}



async function updateCharts() {
  const timeRange = document.getElementById('timeRange').value;
  const sampleType = document.getElementById('sampleType').value;
  const sensorType = document.getElementById('sensorType').value;
  const container = document.getElementById('charts-container');
  
  // Clear existing charts
  Object.values(charts).forEach(chart => chart.destroy());
  charts = {};
  container.innerHTML = '<div class="loading">📊 Ładowanie danych z ' + sensorType + '...</div>';
  
  // Store pending request
  pendingHistoryRequest = { sensorType, timeRange, sampleType, container };
  
  // Send WebSocket command
  if (ws && ws.readyState === WebSocket.OPEN) {
    const command = {
      cmd: 'getHistory',
      sensor: sensorType,
      timeRange: timeRange,
      sampleType: sampleType
    };
    ws.send(JSON.stringify(command));
  } else {
    container.innerHTML = '<div class="no-data">❌ Brak połączenia WebSocket</div>';
  }
}

function createSPS30Charts(data) {
  const container = document.getElementById('charts-container');
  
  // PM Mass Concentration
  const pmCard = document.createElement('div');
  pmCard.className = 'chart-card';
  pmCard.innerHTML = `
    <div class="chart-title">🌫️ Stężenie Pyłu (SPS30)</div>
    <div class="chart-container" id="pm-chart"></div>
    <div class="chart-info">PM1.0, PM2.5, PM4.0, PM10 (µg/m³)</div>
  `;
  container.appendChild(pmCard);
  
  charts.pm = createChart('pm-chart', 'Stężenie Pyłu', [
    {
      label: 'PM1.0 (µg/m³)',
      data: formatChartData(data, 'PM1'),
      borderColor: '#e74c3c',
      backgroundColor: '#e74c3c20',
      tension: 0.1
    },
    {
      label: 'PM2.5 (µg/m³)',
      data: formatChartData(data, 'PM25'),
      borderColor: '#f39c12',
      backgroundColor: '#f39c1220',
      tension: 0.1
    },
    {
      label: 'PM4.0 (µg/m³)',
      data: formatChartData(data, 'PM4'),
      borderColor: '#e67e22',
      backgroundColor: '#e67e2220',
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
  
  // Number Concentration
  const ncCard = document.createElement('div');
  ncCard.className = 'chart-card';
  ncCard.innerHTML = `
    <div class="chart-title">🔢 Koncentracja Cząstek (SPS30)</div>
    <div class="chart-container" id="nc-chart"></div>
    <div class="chart-info">NC0.5, NC1.0, NC2.5, NC4.0, NC10 (#/cm³)</div>
  `;
  container.appendChild(ncCard);
  
  charts.nc = createChart('nc-chart', 'Koncentracja Cząstek', [
    {
      label: 'NC0.5 (#/cm³)',
      data: formatChartData(data, 'NC05'),
      borderColor: '#3498db',
      backgroundColor: '#3498db20',
      tension: 0.1
    },
    {
      label: 'NC1.0 (#/cm³)',
      data: formatChartData(data, 'NC1'),
      borderColor: '#2980b9',
      backgroundColor: '#2980b920',
      tension: 0.1
    },
    {
      label: 'NC2.5 (#/cm³)',
      data: formatChartData(data, 'NC25'),
      borderColor: '#1abc9c',
      backgroundColor: '#1abc9c20',
      tension: 0.1
    },
    {
      label: 'NC4.0 (#/cm³)',
      data: formatChartData(data, 'NC4'),
      borderColor: '#16a085',
      backgroundColor: '#16a08520',
      tension: 0.1
    },
    {
      label: 'NC10 (#/cm³)',
      data: formatChartData(data, 'NC10'),
      borderColor: '#27ae60',
      backgroundColor: '#27ae6020',
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

#endif // CHART_H 