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
      <option value="ina219">INA219 (Moc)</option>
      <option value="sht40">SHT40 (Temperatura/Wilgotność)</option>
      <option value="scd41">SCD41 (CO2)</option>
      <option value="hcho">HCHO (VOC)</option>
      <option value="i2c">I2C Sensors</option>
      <option value="mcp3424">MCP3424 (ADC)</option>
      <option value="ads1110">ADS1110 (ADC)</option>
      <option value="solar">Solar Sensor</option>
      <option value="opcn3">OPCN3 (Particle Counter)</option>
      <option value="calibrated">Kalibracja (Gazy)</option>
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

// Pakietowanie danych
let currentPackets = {};
let allData = {};

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
      
      if (data.cmd === 'history') {
        handleHistoryResponse(data);
      } else {
        updateStatusBar(data);
      }
    } catch (error) {
      console.error('Error parsing WebSocket data:', error);
    }
  };
}

function updateStatusBar(data) {
  document.getElementById('update-status').textContent = `Ostatnia aktualizacja: ${new Date().toLocaleTimeString('pl-PL')}`;
  
  // Debug info
  console.log('Status bar update:', data.cmd, data);
  
  if (data.history) {
    const memoryPercent = data.history.memoryUsed > 0 ? Math.round((data.history.memoryUsed / data.history.memoryBudget) * 100) : 0;
    document.getElementById('history-status').textContent = 
      `Historia: ${data.history.enabled ? '✅' : '❌'} (${memoryPercent}% pamięci)`;
  }
  
  // Show connection quality
  if (data.success === false) {
    document.getElementById('connection-status').textContent = 'Błąd odpowiedzi ⚠️';
  }
}

function handleHistoryResponse(data) {
  console.log('Handling history response:', data);
  
  if (data.error) {
    console.log('History error detected:', data.error);
    document.getElementById('charts-container').innerHTML = `<div class="no-data">❌ Błąd: ${data.error}</div>`;
    return;
  }
  
  // Sprawdź czy to automatyczny tryb
  const autoMode = data.autoMode || false;
  
  // Informacje o pakietach
  const packetIndex = data.packetIndex || 0;
  const totalPackets = data.totalPackets || 1;
  const hasMorePackets = data.hasMorePackets || false;
  const totalAvailableSamples = data.totalAvailableSamples || 0;
  
  console.log(`Pakiet ${packetIndex + 1}/${totalPackets}, dostępne próbki: ${totalAvailableSamples}, więcej pakietów: ${hasMorePackets}, autoMode: ${autoMode}`);
  
  if (!data.data || data.data.length === 0) {
    if (packetIndex === 0) {
      console.log('No data array or empty data');
      const errorMsg = !data.data ? 'Brak pola data w odpowiedzi' : 'Pusta tablica danych';
      document.getElementById('charts-container').innerHTML = `<div class="no-data">📭 ${errorMsg}<br><small>Sensor: ${data.sensor}, zakres: ${data.timeRange}, typ: ${data.sampleType}</small></div>`;
      return;
    }
  }
  
  const sensorKey = `${data.sensor}_${data.timeRange}_${data.sampleType}`;
  
  // Inicjalizuj storage dla tego sensora jeśli nie istnieje
  if (!allData[sensorKey]) {
    allData[sensorKey] = [];
    currentPackets[sensorKey] = {
      received: new Set(),
      totalPackets: totalPackets,
      sensor: data.sensor,
      timeRange: data.timeRange,
      sampleType: data.sampleType
    };
  }
  
  // Dodaj dane z tego pakietu
  if (data.data && data.data.length > 0) {
    allData[sensorKey] = allData[sensorKey].concat(data.data);
    currentPackets[sensorKey].received.add(packetIndex);
    console.log(`Pakiet ${packetIndex} dodany. Łącznie próbek: ${allData[sensorKey].length}`);
  }
  
  // Sprawdź czy mamy wszystkie pakiety lub to pierwszy pakiet
  const receivedPackets = currentPackets[sensorKey].received.size;
  const shouldCreateCharts = (receivedPackets >= totalPackets) || (packetIndex === 0 && !hasMorePackets);
  
  if (shouldCreateCharts) {
    console.log(`Tworzenie wykresów z ${allData[sensorKey].length} próbek`);
    
    // Log first sample structure for debugging
    if (allData[sensorKey].length > 0) {
      console.log('First sample structure:', allData[sensorKey][0]);
    }
    
    const sensorType = document.getElementById('sensorType').value;
  
    // Create charts based on sensor type
    if (sensorType === 'sps30') {
      createSPS30Charts(allData[sensorKey]);
    } else if (sensorType === 'ips') {
      createIPSCharts(allData[sensorKey]);
    } else if (sensorType === 'ina219') {
      createINA219Charts(allData[sensorKey]);
    } else if (sensorType === 'sht40') {
      createSHT40Charts(allData[sensorKey]);
    } else if (sensorType === 'scd41') {
      createSCD41Charts(allData[sensorKey]);
    } else if (sensorType === 'hcho') {
      createHCHOCharts(allData[sensorKey]);
    } else if (sensorType === 'i2c') {
      createI2CCharts(allData[sensorKey]);
    } else if (sensorType === 'mcp3424') {
      createMCP3424Charts(allData[sensorKey]);
    } else if (sensorType === 'ads1110') {
      createADS1110Charts(allData[sensorKey]);
    } else if (sensorType === 'solar') {
      createSolarCharts(allData[sensorKey]);
    } else if (sensorType === 'opcn3') {
      createOPCN3Charts(allData[sensorKey]);
    } else if (sensorType === 'calibrated') {
      createCalibratedCharts(allData[sensorKey]);
    } else {
      console.error('Unknown sensor type:', sensorType);
      const container = document.getElementById('charts-container');
      container.innerHTML = '<div class="no-data">❌ Nieznany typ czujnika: ' + sensorType + '</div>';
      return;
    }
    
    // Show stats
    const statsDiv = document.createElement('div');
    statsDiv.className = 'chart-info';
    statsDiv.style.textAlign = 'center';
    statsDiv.style.color = 'white';
    statsDiv.style.marginTop = '20px';
    const sampleTypeText = document.getElementById('sampleType').value === 'fast' ? 'szybkich (10s)' : 'wolnych (5min)';
    const modeText = autoMode ? 'automatycznie' : 'manualnie';
    statsDiv.innerHTML = `📊 Załadowano ${allData[sensorKey].length} próbek ${sampleTypeText} z ${sensorType} (${modeText}, pakiety: ${receivedPackets}/${totalPackets})`;
    document.getElementById('charts-container').appendChild(statsDiv);
    
    console.log('Charts created successfully');
    
    // Wyczyść dane po utworzeniu wykresów
    delete allData[sensorKey];
    delete currentPackets[sensorKey];
    
  } else if (hasMorePackets && !autoMode) {
    // Żądaj kolejnego pakietu TYLKO jeśli nie ma autoMode (stary sposób)
    console.log(`Żądam pakietu ${packetIndex + 1}/${totalPackets} (manual mode)`);
    requestNextPacket(data.sensor, data.timeRange, data.sampleType, packetIndex + 1, data.fromTime, data.toTime);
    
    // Pokaż postęp
    const container = document.getElementById('charts-container');
    const progress = Math.round((receivedPackets / totalPackets) * 100);
    container.innerHTML = `<div class="loading">📊 Ładowanie danych... ${progress}% (pakiet ${receivedPackets}/${totalPackets})</div>`;
  } else if (hasMorePackets && autoMode) {
    // W autoMode tylko pokaż postęp - serwer sam wyśle kolejne pakiety
    const container = document.getElementById('charts-container');
    const progress = Math.round((receivedPackets / totalPackets) * 100);
    container.innerHTML = `<div class="loading">📊 Automatyczne ładowanie... ${progress}% (pakiet ${receivedPackets}/${totalPackets})</div>`;
    console.log(`Waiting for auto packet ${packetIndex + 1}/${totalPackets}`);
  }
}

function createChart(containerId, title, datasets) {
  // Check if Chart.js is loaded
  if (typeof Chart === 'undefined') {
    console.error('Chart.js library not loaded!');
    document.getElementById(containerId).innerHTML = '<div class="no-data">❌ Błąd: Chart.js nie załadowany</div>';
    return null;
  }
  
  const canvas = document.createElement('canvas');
  const container = document.getElementById(containerId);
  
  if (!container) {
    console.error('Chart container not found:', containerId);
    return null;
  }
  
  container.appendChild(canvas);
  
  const ctx = canvas.getContext('2d');
  
  // Count datasets with data
  const datasetsWithData = datasets.filter(dataset => dataset.data && dataset.data.length > 0);
  console.log(`Creating chart "${title}" with ${datasetsWithData.length}/${datasets.length} datasets having data`);
  
  if (datasetsWithData.length === 0) {
    container.innerHTML = '<div class="no-data">📭 Brak danych do wyświetlenia</div>';
    return null;
  }
  
  try {
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
  } catch (error) {
    console.error('Error creating chart:', error);
    container.innerHTML = '<div class="no-data">❌ Błąd tworzenia wykresu: ' + error.message + '</div>';
    return null;
  }
}

function formatChartData(historyData, valueKey, label, color) {
  if (!historyData || !historyData.length) {
    console.warn(`No history data for key: ${valueKey}`);
    return [];
  }
  
  console.log(`Formatting chart data for key: ${valueKey}, entries: ${historyData.length}`);
  
  let validEntries = 0;
  const formattedData = historyData.map((entry, index) => {
    let timestamp = entry.timestamp;
    
    // Convert epoch timestamp from seconds to milliseconds
    if (timestamp < 1600000000000) {
      // If timestamp is smaller than Jan 1, 2020 in milliseconds, assume it's in seconds
      timestamp = timestamp * 1000;
    }
    
    // Get the value from data object
    const value = entry.data ? entry.data[valueKey] : entry[valueKey];
    const parsedValue = parseFloat(value);
    
    // Debug logging for first few entries
    if (index < 3) {
      console.log(`Entry ${index}: timestamp=${entry.timestamp}, converted=${timestamp}, value=${value}, parsedValue=${parsedValue}`);
    }
    
    if (!isNaN(parsedValue)) {
      validEntries++;
    }
    
    return {
      x: new Date(timestamp),
      y: isNaN(parsedValue) ? null : parsedValue
    };
  }).filter(point => point.y !== null && point.y !== undefined);
  
  // Sort by timestamp to ensure proper order
  formattedData.sort((a, b) => a.x.getTime() - b.x.getTime());
  
  console.log(`Formatted ${formattedData.length}/${validEntries} valid points for ${valueKey}`);
  if (formattedData.length === 0) {
    console.warn(`No valid data points found for key: ${valueKey}`);
  } else if (formattedData.length > 0) {
    console.log(`Time range: ${formattedData[0].x.toISOString()} to ${formattedData[formattedData.length-1].x.toISOString()}`);
  }
  
  return formattedData;
}

function requestNextPacket(sensor, timeRange, sampleType, packetIndex, fromTime, toTime) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    const command = {
      cmd: 'getHistory',
      sensor: sensor,
      timeRange: timeRange,
      sampleType: sampleType,
      packetIndex: packetIndex,
      packetSize: 20 // Domyślny rozmiar pakietu
    };
    
    // Dodaj fromTime/toTime jeśli są dostępne
    if (fromTime && toTime) {
      command.fromTime = fromTime;
      command.toTime = toTime;
    }
    
    console.log('Requesting next packet:', command);
    ws.send(JSON.stringify(command));
  } else {
    console.error('WebSocket not available for next packet request');
  }
}

async function updateCharts() {
  const timeRange = document.getElementById('timeRange').value;
  const sampleType = document.getElementById('sampleType').value;
  const sensorType = document.getElementById('sensorType').value;
  const container = document.getElementById('charts-container');
  
  if (!container) {
    console.error('Charts container not found!');
    return;
  }
  
  console.log('=== updateCharts called ===');
  console.log('Updating charts:', { timeRange, sampleType, sensorType });
  console.log('WebSocket state:', ws ? ws.readyState : 'no ws');
  
  // Clear existing charts and data
  Object.values(charts).forEach(chart => {
    if (chart && chart.destroy) {
      console.log('Destroying chart:', chart.config?.options?.plugins?.title?.text || 'unknown');
      chart.destroy();
    }
  });
  charts = {};
  
  // Wyczyść poprzednie dane
  const sensorKey = `${sensorType}_${timeRange}_${sampleType}`;
  console.log('Clearing data for key:', sensorKey);
  delete allData[sensorKey];
  delete currentPackets[sensorKey];
  
  container.innerHTML = '<div class="loading">📊 Ładowanie danych z ' + sensorType + '... (pakiet 1)</div>';
  
  // Send WebSocket command (automatyczny tryb - bez parametrów pakietowania)
  if (ws && ws.readyState === WebSocket.OPEN) {
    const command = {
      cmd: 'getHistory',
      sensor: sensorType,
      timeRange: timeRange,
      sampleType: sampleType
      // Brak packetIndex/packetSize = automatyczny tryb
    };
    console.log('Sending WebSocket command (auto mode):', command);
    ws.send(JSON.stringify(command));
  } else {
    const wsState = ws ? ['CONNECTING', 'OPEN', 'CLOSING', 'CLOSED'][ws.readyState] : 'NO_WS';
    console.log('WebSocket not connected, state:', wsState);
    container.innerHTML = '<div class="no-data">❌ Brak połączenia WebSocket<br><small>Stan: ' + wsState + '</small></div>';
  }
}

function createSPS30Charts(data) {
  const container = document.getElementById('charts-container');
  
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

function createINA219Charts(data) {
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
    }
  ]);
}

function createSCD41Charts(data) {
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

function createI2CCharts(data) {
  const container = document.getElementById('charts-container');
  
  const i2cCard = document.createElement('div');
  i2cCard.className = 'chart-card';
  i2cCard.innerHTML = `
    <div class="chart-title">🔬 Czujniki I2C</div>
    <div class="chart-container" id="i2c-chart"></div>
    <div class="chart-info">Dane z czujników I2C</div>
  `;
  container.appendChild(i2cCard);
  
  charts.i2c = createChart('i2c-chart', 'Czujniki I2C', [
    {
      label: 'Temperature (°C)',
      data: formatChartData(data, 'temperature'),
      borderColor: '#e74c3c',
      backgroundColor: '#e74c3c20',
      tension: 0.1
    },
    {
      label: 'Humidity (%)',
      data: formatChartData(data, 'humidity'),
      borderColor: '#3498db',
      backgroundColor: '#3498db20',
      tension: 0.1
    },
    {
      label: 'Pressure (hPa)',
      data: formatChartData(data, 'pressure'),
      borderColor: '#9b59b6',
      backgroundColor: '#9b59b620',
      tension: 0.1
    }
  ]);
}

function createMCP3424Charts(data) {
  const container = document.getElementById('charts-container');
  
  const mcpCard = document.createElement('div');
  mcpCard.className = 'chart-card';
  mcpCard.innerHTML = `
    <div class="chart-title">🔌 MCP3424 (18-bit ADC)</div>
    <div class="chart-container" id="mcp3424-chart"></div>
    <div class="chart-info">Kanały K1-K8 (napięcia z konwertera ADC)</div>
  `;
  container.appendChild(mcpCard);
  
  // Create datasets for available K channels
  const datasets = [];
  const colors = ['#e74c3c', '#3498db', '#27ae60', '#f39c12', '#9b59b6', '#e67e22', '#1abc9c', '#34495e'];
  
  // Check for K1_1 through K8_4 channels
  for (let k = 1; k <= 8; k++) {
    for (let ch = 1; ch <= 4; ch++) {
      const key = `K${k}_${ch}`;
      // Check if this key exists in data
      if (data.some(entry => entry.data && entry.data[key] !== undefined)) {
        datasets.push({
          label: `${key} (V)`,
          data: formatChartData(data, key),
          borderColor: colors[(k-1) % colors.length],
          backgroundColor: colors[(k-1) % colors.length] + '20',
          tension: 0.1
        });
      }
    }
  }
  
  if (datasets.length === 0) {
    // No K channel data found, show message
    container.innerHTML = '<div class="no-data">📭 Brak danych MCP3424<br><small>Sprawdź czy urządzenia MCP3424 są podłączone i skonfigurowane</small></div>';
    return;
  }
  
  charts.mcp3424 = createChart('mcp3424-chart', 'MCP3424 - Kanały ADC', datasets);
}

function createADS1110Charts(data) {
  const container = document.getElementById('charts-container');
  
  const adsCard = document.createElement('div');
  adsCard.className = 'chart-card';
  adsCard.innerHTML = `
    <div class="chart-title">🔌 ADS1110 (16-bit ADC)</div>
    <div class="chart-container" id="ads1110-chart"></div>
    <div class="chart-info">Napięcia z konwertera ADC</div>
  `;
  container.appendChild(adsCard);
  
  charts.ads1110 = createChart('ads1110-chart', 'ADS1110 - Napięcia', [
    {
      label: 'Voltage (V)',
      data: formatChartData(data, 'voltage'),
      borderColor: '#e74c3c',
      backgroundColor: '#e74c3c20',
      tension: 0.1
    }
  ]);
}

function createSolarCharts(data) {
  const container = document.getElementById('charts-container');
  
  const solarCard = document.createElement('div');
  solarCard.className = 'chart-card';
  solarCard.innerHTML = `
    <div class="chart-title">☀️ Solar Sensor</div>
    <div class="chart-container" id="solar-chart"></div>
    <div class="chart-info">Dane z czujnika słonecznego</div>
  `;
  container.appendChild(solarCard);
  
  charts.solar = createChart('solar-chart', 'Solar Sensor', [
    {
      label: 'Voltage (V)',
      data: formatChartData(data, 'voltage'),
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

function createOPCN3Charts(data) {
  const container = document.getElementById('charts-container');
  
  const opcn3Card = document.createElement('div');
  opcn3Card.className = 'chart-card';
  opcn3Card.innerHTML = `
    <div class="chart-title">🔬 OPCN3 (Particle Counter)</div>
    <div class="chart-container" id="opcn3-chart"></div>
    <div class="chart-info">Liczba cząstek</div>
  `;
  container.appendChild(opcn3Card);
  
  charts.opcn3 = createChart('opcn3-chart', 'OPCN3 - Liczba Cząstek', [
    {
      label: 'PM1.0 (#/cm³)',
      data: formatChartData(data, 'pm1'),
      borderColor: '#e74c3c',
      backgroundColor: '#e74c3c20',
      tension: 0.1
    },
    {
      label: 'PM2.5 (#/cm³)',
      data: formatChartData(data, 'pm25'),
      borderColor: '#f39c12',
      backgroundColor: '#f39c1220',
      tension: 0.1
    },
    {
      label: 'PM10 (#/cm³)',
      data: formatChartData(data, 'pm10'),
      borderColor: '#3498db',
      backgroundColor: '#3498db20',
      tension: 0.1
    }
  ]);
}

function createCalibratedCharts(data) {
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

function clearAllData() {
  console.log('Clearing all chart data and packets');
  allData = {};
  currentPackets = {};
  
  Object.values(charts).forEach(chart => {
    if (chart && chart.destroy) {
      chart.destroy();
    }
  });
  charts = {};
}

function toggleAutoUpdate() {
  autoUpdate = !autoUpdate;
  const button = document.getElementById('auto-status');
  
  if (autoUpdate) {
    button.textContent = 'ON';
    button.style.color = '#27ae60';
    console.log('Auto-update enabled: refreshing every 30 seconds');
    autoUpdateInterval = setInterval(() => {
      console.log('Auto-update triggered');
      updateCharts();
    }, 30000);
    // Update charts immediately when auto-update is enabled
    updateCharts();
  } else {
    button.textContent = 'OFF';
    button.style.color = '#e74c3c';
    console.log('Auto-update disabled');
    if (autoUpdateInterval) {
      clearInterval(autoUpdateInterval);
      autoUpdateInterval = null;
    }
  }
}

// Initialize
window.addEventListener('load', function() {
  console.log('Charts page loaded, connecting WebSocket...');
  connectWebSocket();
  // Don't auto-load charts, wait for user selection
});

window.addEventListener('beforeunload', function() {
  console.log('Charts page unloading, cleaning up...');
  if (ws) {
    console.log('Closing WebSocket connection');
    ws.close();
  }
  if (autoUpdateInterval) {
    console.log('Clearing auto-update interval');
    clearInterval(autoUpdateInterval);
  }
  console.log('Destroying', Object.keys(charts).length, 'charts');
  Object.values(charts).forEach(chart => {
    if (chart && chart.destroy) {
      chart.destroy();
    }
  });
  clearAllData();
});
</script>
</body>
</html>
)rawliteral";

#endif // CHARTS_HTML_H 