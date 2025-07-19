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
    <a href="/">🔧 Update</a>
    <a href="/dashboard">📊 Dashboard</a>
    <a href="/charts">📈 Wykresy</a>
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
    
    <label style="color: white;">Czujnik:</label>
    <select id="sensorType">
      <option value="i2c">Środowiskowe (I2C)</option>
      <option value="solar">Solar</option>
      <option value="sps30">SPS30 (Pyłki)</option>
      <option value="power">INA219 (Moc)</option>
      <option value="hcho">HCHO (VOC)</option>
    </select>
    
    <button onclick="updateCharts()">🔄 Odśwież</button>
    <button onclick="toggleAutoUpdate()">⏰ Auto: <span id="auto-status">OFF</span></button>
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
      updateStatusBar(data);
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
  
  return historyData.map(entry => ({
    x: new Date(entry.timestamp),
    y: parseFloat(entry.data[valueKey]) || entry.data[valueKey]
  })).filter(point => point.y !== undefined && point.y !== null && !isNaN(point.y));
}

async function fetchHistoryData(sensor, timeRange) {
  try {
    const response = await fetch(`/api/history?sensor=${sensor}&timeRange=${timeRange}`);
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }
    
    const result = await response.json();
    
    if (result.error) {
      throw new Error(result.error);
    }
    
    return result;
  } catch (error) {
    console.error('Error fetching history data:', error);
    return { 
      error: error.message, 
      data: [], 
      totalSamples: 0 
    };
  }
}

async function updateCharts() {
  const timeRange = document.getElementById('timeRange').value;
  const sensorType = document.getElementById('sensorType').value;
  const container = document.getElementById('charts-container');
  
  // Clear existing charts
  Object.values(charts).forEach(chart => chart.destroy());
  charts = {};
  container.innerHTML = '<div class="loading">📊 Ładowanie danych z ' + sensorType + '...</div>';
  
  try {
    const historyData = await fetchHistoryData(sensorType, timeRange);
    
    if (historyData.error) {
      container.innerHTML = `<div class="no-data">❌ Błąd: ${historyData.error}</div>`;
      return;
    }
    
    if (!historyData.data || historyData.data.length === 0) {
      container.innerHTML = '<div class="no-data">📭 Brak danych dla wybranego czujnika</div>';
      return;
    }
    
    // Create charts based on sensor type
    if (sensorType === 'i2c') {
      createEnvironmentalCharts(historyData.data);
    } else if (sensorType === 'solar') {
      createSolarCharts(historyData.data);
    } else if (sensorType === 'sps30') {
      createAirQualityCharts(historyData.data);
    } else if (sensorType === 'power') {
      createPowerCharts(historyData.data);
    } else if (sensorType === 'hcho') {
      createHCHOCharts(historyData.data);
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
    statsDiv.innerHTML = `📊 Załadowano ${historyData.totalSamples} próbek z ${sensorType}`;
    container.appendChild(statsDiv);
    
  } catch (error) {
    console.error('Error updating charts:', error);
    container.innerHTML = '<div class="no-data">❌ Błąd podczas ładowania danych</div>';
  }
}

function createEnvironmentalCharts(data) {
  const container = document.getElementById('charts-container');
  
  // Temperature & Humidity
  const tempHumCard = document.createElement('div');
  tempHumCard.className = 'chart-card';
  tempHumCard.innerHTML = `
    <div class="chart-title">🌡️ Temperatura i Wilgotność</div>
    <div class="chart-container" id="temp-hum-chart"></div>
    <div class="chart-info">Dane z czujników I2C</div>
  `;
  container.appendChild(tempHumCard);
  
  charts.tempHum = createChart('temp-hum-chart', 'Temperatura i Wilgotność', [
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
    }
  ]);
  
  // Pressure & CO2
  const pressureCard = document.createElement('div');
  pressureCard.className = 'chart-card';
  pressureCard.innerHTML = `
    <div class="chart-title">🌍 Ciśnienie i CO2</div>
    <div class="chart-container" id="pressure-chart"></div>
    <div class="chart-info">Dane atmosferyczne</div>
  `;
  container.appendChild(pressureCard);
  
  charts.pressure = createChart('pressure-chart', 'Ciśnienie i CO2', [
    {
      label: 'Ciśnienie (hPa)',
      data: formatChartData(data, 'pressure'),
      borderColor: '#9b59b6',
      backgroundColor: '#9b59b620',
      tension: 0.1
    },
    {
      label: 'CO2 (ppm)',
      data: formatChartData(data, 'co2'),
      borderColor: '#2ecc71',
      backgroundColor: '#2ecc7120',
      tension: 0.1
    }
  ]);
}

function createSolarCharts(data) {
  const container = document.getElementById('charts-container');
  
  const solarCard = document.createElement('div');
  solarCard.className = 'chart-card';
  solarCard.innerHTML = `
    <div class="chart-title">☀️ Monitor Solarny</div>
    <div class="chart-container" id="solar-chart"></div>
    <div class="chart-info">Napięcie, prąd i moc PV</div>
  `;
  container.appendChild(solarCard);
  
  charts.solar = createChart('solar-chart', 'Parametry Solarne', [
    {
      label: 'Napięcie (V)',
      data: formatChartData(data, 'V'),
      borderColor: '#f39c12',
      backgroundColor: '#f39c1220',
      tension: 0.1
    },
    {
      label: 'Prąd (A)',
      data: formatChartData(data, 'I'),
      borderColor: '#e67e22',
      backgroundColor: '#e67e2220',
      tension: 0.1
    },
    {
      label: 'Moc PV (W)',
      data: formatChartData(data, 'PPV'),
      borderColor: '#27ae60',
      backgroundColor: '#27ae6020',
      tension: 0.1
    }
  ]);
}

function createAirQualityCharts(data) {
  const container = document.getElementById('charts-container');
  
  const airCard = document.createElement('div');
  airCard.className = 'chart-card';
  airCard.innerHTML = `
    <div class="chart-title">💨 Jakość Powietrza (SPS30)</div>
    <div class="chart-container" id="air-chart"></div>
    <div class="chart-info">Cząstki PM w µg/m³</div>
  `;
  container.appendChild(airCard);
  
  charts.air = createChart('air-chart', 'Cząstki PM', [
    {
      label: 'PM1.0',
      data: formatChartData(data, 'pm1_0'),
      borderColor: '#34495e',
      backgroundColor: '#34495e20',
      tension: 0.1
    },
    {
      label: 'PM2.5',
      data: formatChartData(data, 'pm2_5'),
      borderColor: '#e74c3c',
      backgroundColor: '#e74c3c20',
      tension: 0.1
    },
    {
      label: 'PM10',
      data: formatChartData(data, 'pm10'),
      borderColor: '#95a5a6',
      backgroundColor: '#95a5a620',
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
  
  charts.hcho = createChart('hcho-chart', 'HCHO i VOC', [
    {
      label: 'HCHO (mg/m³)',
      data: formatChartData(data, 'hcho'),
      borderColor: '#8e44ad',
      backgroundColor: '#8e44ad20',
      tension: 0.1
    },
    {
      label: 'VOC (mg/m³)',
      data: formatChartData(data, 'voc'),
      borderColor: '#d35400',
      backgroundColor: '#d3540020',
      tension: 0.1
    },
    {
      label: 'TVOC (mg/m³)',
      data: formatChartData(data, 'tvoc'),
      borderColor: '#16a085',
      backgroundColor: '#16a08520',
      tension: 0.1
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