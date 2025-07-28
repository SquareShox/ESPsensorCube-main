#ifndef MCP3424_CONFIG_HTML_H
#define MCP3424_CONFIG_HTML_H
const char *mcp3424_config_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP Sensor Cube - Konfiguracja MCP3424</title>
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

  .config-container {
    max-width: 1000px;
    margin: 0 auto;
    background: rgba(255, 255, 255, 0.95);
    border-radius: 20px;
    padding: 30px;
    box-shadow: 0 10px 30px rgba(0,0,0,0.2);
  }

  .config-section {
    margin-bottom: 30px;
    padding: 20px;
    border: 2px solid #f0f0f0;
    border-radius: 15px;
    background: #fafafa;
  }

  .section-title {
    font-size: 1.4em;
    font-weight: bold;
    color: #333;
    margin-bottom: 20px;
    display: flex;
    align-items: center;
  }

  .section-icon {
    width: 30px;
    height: 30px;
    border-radius: 50%;
    margin-right: 10px;
    display: flex;
    align-items: center;
    justify-content: center;
    color: white;
    font-weight: bold;
    font-size: 16px;
    background: linear-gradient(45deg, #ff9800, #ffc107);
  }

  .device-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    gap: 20px;
    margin-bottom: 20px;
  }

  .device-card {
    background: white;
    border: 2px solid #e0e0e0;
    border-radius: 15px;
    padding: 20px;
    transition: all 0.3s ease;
  }

  .device-card:hover {
    border-color: #667eea;
    box-shadow: 0 5px 15px rgba(0,0,0,0.1);
  }

  .device-card.active {
    border-color: #4caf50;
    background: #f8fff8;
  }

  .device-header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-bottom: 15px;
    padding-bottom: 10px;
    border-bottom: 1px solid #eee;
  }

  .device-title {
    font-weight: bold;
    font-size: 1.1em;
    color: #333;
  }

  .device-index {
    background: #667eea;
    color: white;
    padding: 5px 10px;
    border-radius: 15px;
    font-size: 0.9em;
    font-weight: bold;
  }

  .form-group {
    margin-bottom: 15px;
  }

  .form-label {
    display: block;
    font-weight: bold;
    color: #333;
    margin-bottom: 5px;
  }

  .form-input, .form-select {
    width: 100%;
    padding: 10px;
    border: 2px solid #ddd;
    border-radius: 8px;
    font-size: 14px;
    transition: border-color 0.3s ease;
  }

  .form-input:focus, .form-select:focus {
    outline: none;
    border-color: #667eea;
    box-shadow: 0 0 10px rgba(102, 126, 234, 0.3);
  }

  .form-input:disabled {
    background-color: #f5f5f5;
    color: #666;
  }

  .form-input.found {
    border-color: #4caf50 !important;
    background-color: #f8fff8;
  }

  .form-input.not-found {
    border-color: #ff9800 !important;
    background-color: #fff8f0;
  }

  .form-input.invalid {
    border-color: #f44336 !important;
    background-color: #fff0f0;
  }

  .checkbox-group {
    display: flex;
    align-items: center;
    margin-top: 10px;
  }

  .checkbox-group input[type="checkbox"] {
    margin-right: 8px;
    transform: scale(1.2);
  }

  .checkbox-group label {
    font-weight: bold;
    color: #333;
    cursor: pointer;
  }

  .btn {
    background: linear-gradient(45deg, #667eea, #764ba2);
    color: white;
    border: none;
    border-radius: 25px;
    padding: 12px 25px;
    font-size: 1em;
    font-weight: bold;
    cursor: pointer;
    transition: all 0.3s ease;
    margin: 5px;
  }

  .btn:hover {
    transform: translateY(-2px);
    box-shadow: 0 8px 25px rgba(0,0,0,0.3);
  }

  .btn-success {
    background: linear-gradient(45deg, #4caf50, #8bc34a);
  }

  .btn-danger {
    background: linear-gradient(45deg, #f44336, #ff5722);
  }

  .btn-warning {
    background: linear-gradient(45deg, #ff9800, #ffc107);
  }

  .alert {
    padding: 15px;
    border-radius: 8px;
    margin-bottom: 20px;
    font-weight: bold;
  }

  .alert-success {
    background-color: #d4edda;
    color: #155724;
    border: 1px solid #c3e6cb;
  }

  .alert-error {
    background-color: #f8d7da;
    color: #721c24;
    border: 1px solid #f5c6cb;
  }

  .alert-warning {
    background-color: #fff3cd;
    color: #856404;
    border: 1px solid #ffeaa7;
  }

  .info-box {
    background: #e3f2fd;
    border: 1px solid #2196f3;
    border-radius: 10px;
    padding: 15px;
    margin-bottom: 20px;
  }

  .info-box h3 {
    color: #1976d2;
    margin-bottom: 10px;
  }

  .info-box ul {
    margin-left: 20px;
  }

  .info-box li {
    margin-bottom: 5px;
    color: #333;
  }

  @media (max-width: 768px) {
    .config-container {
      margin: 10px;
      padding: 20px;
    }
    
    .header h1 {
      font-size: 2em;
    }
    
    .device-grid {
      grid-template-columns: 1fr;
    }
  }
</style>
<script src="/common.js"></script>
</head>
<body>
  <div class="header">
    <h1>🔌 ESP Sensor Cube - Konfiguracja MCP3424</h1>
    <p>Zarządzanie przypisaniem urządzeń MCP3424 do typów gazów</p>
  </div>

  <div class="nav">
    <a href="/">🔧 Panel Sterowania</a>
    <a href="/dashboard">📊 Dashboard</a>
    <a href="/charts">📈 Wykresy</a>
    <a href="/network">🌐 Sieć</a>
    <a href="/mcp3424">🔌 MCP3424</a>
  </div>

  <div class="config-container">
    <div id="alert-container"></div>

    <!-- Info Section -->
    <div class="info-box">
      <h3>📋 Informacje o konfiguracji MCP3424</h3>
      <ul>
        <li><strong>Device 0-4:</strong> Czujniki elektrochemiczne (NO, O3, NO2, CO, SO2)</li>
        <li><strong>Device 5-7:</strong> Czujniki TGS (TGS1, TGS2, TGS3)</li>
        <li><strong>Channel 0:</strong> Working Electrode (WRK)</li>
        <li><strong>Channel 1:</strong> Auxiliary Electrode (AUX)</li>
        <li><strong>Channel 2:</strong> Temperature (TEMP) / TGS C3</li>
        <li><strong>Channel 3:</strong> Supply Voltage (VCC) / TGS C4</li>
        <li><strong>Adresy I2C:</strong> 0x68-0x6F (domyślnie: Device 0=0x68, Device 1=0x6A, itd.)</li>
        <li><strong>Walidacja:</strong> Każdy adres I2C i typ gazu musi być unikalny</li>
        <li><strong>Status adresów:</strong> Zielone = znaleziono, Pomarańczowe = nie znaleziono, Czerwone = błąd</li>
      </ul>
    </div>

    <!-- Device Configuration Section -->
    <div class="config-section">
      <div class="section-title">
        <div class="section-icon">🔌</div>
        Konfiguracja Urządzeń MCP3424
      </div>
      
      <div id="device-grid" class="device-grid">
        <!-- Devices will be loaded here -->
      </div>
      
      <div style="text-align: center; margin-top: 20px;">
        <button class="btn btn-success" onclick="saveMCP3424Config()">
          💾 Zapisz Konfigurację
        </button>
        
        <button class="btn btn-warning" onclick="resetMCP3424Config()">
          🔄 Resetuj do Domyślnych
        </button>
        
        <button class="btn" onclick="loadMCP3424Config()">
          🔄 Odśwież Konfigurację
        </button>
        
        <button class="btn btn-warning" onclick="scanI2CAddresses()">
          🔍 Skanuj Adresy I2C
        </button>
        
        <button class="btn btn-success" onclick="autoAssignAddresses()">
          🎯 Automatyczne Przypisanie
        </button>
      </div>
    </div>
  </div>

<script>
let ws;
let currentConfig = { devices: [] };

function connectWebSocket() {
  ws = new WebSocket(`ws://${window.location.host}/ws`);
  ws.onopen = function() {
    console.log('WebSocket connected');
    loadMCP3424Config();
  };
  ws.onclose = function() {
    console.log('WebSocket disconnected, reconnecting...');
    setTimeout(connectWebSocket, 5000);
  };
  ws.onmessage = function(event) {
    try {
      const data = JSON.parse(event.data);
      console.log('WebSocket received:', data);
      
      if (data.cmd === "mcp3424Config") {
        console.log('MCP3424 config received:', data.config);
        updateMCP3424ConfigDisplay(data.config);
      } else if (data.cmd === "i2cScanResult") {
        console.log('I2C scan result received:', data);
        if (data.success) {
          lastScanResults = data.foundAddresses || []; // Store scan results
          showAlert(`Skanowanie I2C zakończone. Znaleziono ${data.foundAddresses ? data.foundAddresses.length : 0} urządzeń.`, 'success');
          // Update the display with found addresses
          if (data.foundAddresses && data.foundAddresses.length > 0) {
            updateWithFoundAddresses(data.foundAddresses);
          }
        } else {
          showAlert('Błąd podczas skanowania I2C: ' + (data.error || 'Nieznany błąd'), 'error');
          lastScanResults = []; // Clear scan results on error
        }
      } else if (data.cmd === "setMCP3424Config") {
        if (data.success) {
          showAlert('Konfiguracja MCP3424 została zapisana pomyślnie!', 'success');
        } else {
          showAlert('Błąd podczas zapisywania konfiguracji MCP3424: ' + (data.error || 'Nieznany błąd'), 'error');
        }
      } else if (data.cmd === "resetMCP3424Config") {
        if (data.success) {
          showAlert('Konfiguracja MCP3424 została zresetowana do ustawień domyślnych!', 'success');
          loadMCP3424Config(); // Reload the configuration
        } else {
          showAlert('Błąd podczas resetowania konfiguracji MCP3424: ' + (data.error || 'Nieznany błąd'), 'error');
        }
      }
    } catch (error) {
      console.error('Error parsing WebSocket data:', error);
    }
  };
}


function loadMCP3424Config() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    console.log('Requesting MCP3424 config...');
    ws.send(JSON.stringify({cmd: "getMCP3424Config"}));
    
    // Set timeout - if no response in 3 seconds, show default cards
    setTimeout(() => {
      if (document.getElementById('device-grid').children.length === 0) {
        console.log('Timeout - showing default cards');
        showAlert('Timeout połączenia - załadowano domyślną konfigurację', 'warning');
        updateMCP3424ConfigDisplay({ devices: [] });
      }
    }, 3000);
  } else {
    console.log('WebSocket not connected - showing default cards');
    showAlert('WebSocket nie połączony - załadowano domyślną konfigurację', 'warning');
    updateMCP3424ConfigDisplay({ devices: [] });
  }
}

function updateMCP3424ConfigDisplay(config) {
  console.log('Updating MCP3424 display with config:', config);
  currentConfig = config || { devices: [] };
  const grid = document.getElementById('device-grid');
  grid.innerHTML = '';
  
  // Create device cards for all 8 possible devices
  for (let i = 0; i < 8; i++) {
    const device = config && config.devices ? config.devices.find(d => d.deviceIndex === i) : null;
    const card = createDeviceCard(i, device);
    grid.appendChild(card);
  }
  
  // Show success message if we have data
  if (config && config.devices && config.devices.length > 0) {
    showAlert(`Załadowano ${config.devices.length} urządzeń`, 'success');
  } else {
    showAlert('Brak danych konfiguracyjnych - załadowano domyślną konfigurację', 'info');
  }
}

function createDeviceCard(deviceIndex, deviceData) {
  const card = document.createElement('div');
  card.className = 'device-card';
  if (deviceData && deviceData.enabled) {
    card.classList.add('active');
  }
  
  const gasTypes = ['NO', 'O3', 'NO2', 'CO', 'SO2', 'TGS1', 'TGS2', 'TGS3'];
  const descriptions = [
    'NO Sensor (K1)', 'O3 Sensor (K2)', 'NO2 Sensor (K3)', 
    'CO Sensor (K4)', 'SO2 Sensor (K5)', 'TGS Sensor 1', 
    'TGS Sensor 2', 'TGS Sensor 3'
  ];
  
  // Default I2C addresses for MCP3424 (0x68-0x6F)
  const defaultAddresses = [0x68, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70];
  const currentAddress = deviceData ? deviceData.i2cAddress : defaultAddresses[deviceIndex];
  
  card.innerHTML = `
    <div class="device-header">
      <div class="device-title">Device ${deviceIndex}</div>
      <div class="device-index">#${deviceIndex}</div>
    </div>
    
    <div class="form-group">
      <label class="form-label">Adres I2C:</label>
      <input type="text" class="form-input" id="i2c-address-${deviceIndex}" 
             value="0x${currentAddress.toString(16).toUpperCase()}"
             placeholder="0x68"
             pattern="0x[0-9A-Fa-f]{2}"
             title="Format: 0x68-0x6F"
             onchange="updateDeviceConfig(${deviceIndex})">
      <small style="color: #666; font-size: 0.9em;">Format: 0x68-0x6F (domyślnie: 0x${defaultAddresses[deviceIndex].toString(16).toUpperCase()})</small>
    </div>
    
    <div class="form-group">
      <label class="form-label">Typ gazu:</label>
      <select class="form-select" id="gas-type-${deviceIndex}" onchange="updateDeviceConfig(${deviceIndex})">
        <option value="">-- Wybierz typ gazu --</option>
        ${gasTypes.map((type, idx) => 
          `<option value="${type}" ${deviceData && deviceData.gasType === type ? 'selected' : ''}>
            ${type} - ${descriptions[idx]}
          </option>`
        ).join('')}
      </select>
    </div>
    
    <div class="form-group">
      <label class="form-label">Opis:</label>
      <input type="text" class="form-input" id="description-${deviceIndex}" 
             value="${deviceData ? deviceData.description : descriptions[deviceIndex] || ''}"
             placeholder="Opis urządzenia"
             onchange="updateDeviceConfig(${deviceIndex})">
    </div>
    
    <div class="checkbox-group">
      <input type="checkbox" id="enabled-${deviceIndex}" 
             ${deviceData && deviceData.enabled ? 'checked' : ''}
             onchange="updateDeviceConfig(${deviceIndex})">
      <label for="enabled-${deviceIndex}">Aktywne</label>
      ${deviceData && deviceData.autoDetected ? '<span style="color: #4caf50; font-size: 0.9em; margin-left: 10px;">🔍 Auto-wykryte</span>' : ''}
    </div>
  `;
  
  return card;
}

function updateDeviceConfig(deviceIndex) {
  const gasType = document.getElementById(`gas-type-${deviceIndex}`).value;
  const description = document.getElementById(`description-${deviceIndex}`).value;
  const enabled = document.getElementById(`enabled-${deviceIndex}`).checked;
  
  // Parse I2C address
  let i2cAddress = 0x68; // default
  const addressInput = document.getElementById(`i2c-address-${deviceIndex}`);
  const addressValue = addressInput.value.trim();
  
  if (addressValue) {
    // Validate I2C address format
    const addressMatch = addressValue.match(/^0x([0-9A-Fa-f]{2})$/);
    if (addressMatch) {
      i2cAddress = parseInt(addressMatch[1], 16);
      // Validate range (0x68-0x6F typical for MCP3424)
      if (i2cAddress < 0x68 || i2cAddress > 0x6F) {
        showAlert(`Adres I2C 0x${i2cAddress.toString(16).toUpperCase()} jest poza typowym zakresem MCP3424 (0x68-0x6F)`, 'warning');
        addressInput.classList.remove('found', 'not-found');
        addressInput.classList.add('invalid');
      } else {
        addressInput.classList.remove('invalid', 'not-found');
        addressInput.classList.add('found');
      }
    } else {
      showAlert(`Nieprawidłowy format adresu I2C: ${addressValue}. Użyj formatu 0x68-0x6F`, 'error');
      addressInput.classList.remove('found', 'not-found');
      addressInput.classList.add('invalid');
      return;
    }
  } else {
    // Use default address based on device index
    const defaultAddresses = [0x68, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70];
    i2cAddress = defaultAddresses[deviceIndex];
    addressInput.value = `0x${i2cAddress.toString(16).toUpperCase()}`;
    addressInput.classList.remove('found', 'not-found', 'invalid');
  }
  
  // Update current config
  let device = currentConfig.devices.find(d => d.deviceIndex === deviceIndex);
  if (!device) {
    device = { deviceIndex: deviceIndex };
    currentConfig.devices.push(device);
  }
  
  device.i2cAddress = i2cAddress;
  device.gasType = gasType;
  device.description = description;
  device.enabled = enabled;
  
  // Update card appearance
  const card = document.querySelector(`#gas-type-${deviceIndex}`).closest('.device-card');
  if (enabled && gasType) {
    card.classList.add('active');
  } else {
    card.classList.remove('active');
  }
}

function saveMCP3424Config() {
  // Filter out empty devices
  const devices = currentConfig.devices.filter(d => d.gasType && d.gasType.trim() !== '');
  
  if (devices.length === 0) {
    showAlert('Wybierz przynajmniej jedno urządzenie!', 'error');
    return;
  }
  
  // Check for duplicate gas types
  const gasTypes = devices.map(d => d.gasType);
  const uniqueGasTypes = [...new Set(gasTypes)];
  if (gasTypes.length !== uniqueGasTypes.length) {
    showAlert('Każdy typ gazu może być przypisany tylko do jednego urządzenia!', 'error');
    return;
  }
  
  // Check for duplicate I2C addresses
  const addresses = devices.map(d => d.i2cAddress);
  const uniqueAddresses = [...new Set(addresses)];
  if (addresses.length !== uniqueAddresses.length) {
    showAlert('Każdy adres I2C może być przypisany tylko do jednego urządzenia!', 'error');
    return;
  }
  
  // Validate I2C addresses are in valid range
  for (const device of devices) {
    if (device.i2cAddress < 0x68 || device.i2cAddress > 0x6F) {
      showAlert(`Adres I2C 0x${device.i2cAddress.toString(16).toUpperCase()} dla urządzenia ${device.deviceIndex} jest poza typowym zakresem MCP3424 (0x68-0x6F)`, 'error');
      return;
    }
  }
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      cmd: "setMCP3424Config",
      devices: devices
    }));
    showAlert('Konfiguracja MCP3424 wysłana...', 'info');
  } else {
    showAlert('Brak połączenia WebSocket!', 'error');
  }
}

function resetMCP3424Config() {
  if (confirm('Czy na pewno chcesz zresetować konfigurację MCP3424 do ustawień domyślnych?')) {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({cmd: "resetMCP3424Config"}));
      showAlert('Resetowanie konfiguracji MCP3424...', 'info');
    } else {
      showAlert('Brak połączenia WebSocket!', 'error');
    }
  }
}

function scanI2CAddresses() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({cmd: "scanI2CAddresses"}));
    showAlert('Skanowanie adresów I2C...', 'info');
  } else {
    showAlert('Brak połączenia WebSocket!', 'error');
  }
}

function updateWithFoundAddresses(foundAddresses) {
  // Update device cards with found addresses
  for (let i = 0; i < 8; i++) {
    const addressInput = document.getElementById(`i2c-address-${i}`);
    if (addressInput) {
      // Parse current address from input field
      const addressValue = addressInput.value.trim();
      let currentAddress = null;
      
      const addressMatch = addressValue.match(/^0x([0-9A-Fa-f]{2})$/);
      if (addressMatch) {
        currentAddress = parseInt(addressMatch[1], 16);
      }
      
      if (currentAddress !== null) {
        // Check if this address was found during scan
        const wasFound = foundAddresses.includes(currentAddress);
        
        if (wasFound) {
          addressInput.classList.remove('not-found', 'invalid');
          addressInput.classList.add('found');
          addressInput.title = `✅ Znaleziono urządzenie na adresie 0x${currentAddress.toString(16).toUpperCase()}`;
        } else {
          addressInput.classList.remove('found', 'invalid');
          addressInput.classList.add('not-found');
          addressInput.title = `❌ Nie znaleziono urządzenia na adresie 0x${currentAddress.toString(16).toUpperCase()}`;
        }
      }
    }
  }
  
  // Show summary
  const foundCount = foundAddresses.length;
  const totalExpected = 8;
  showAlert(`Skanowanie zakończone: ${foundCount}/${totalExpected} adresów I2C znaleziono. Zielone obramowanie = znaleziono, pomarańczowe = nie znaleziono.`, 'info');
}

let lastScanResults = []; // Store last scan results

function showAlert(message, type) {
  const container = document.getElementById('alert-container');
  const alert = document.createElement('div');
  alert.className = `alert alert-${type === 'success' ? 'success' : type === 'error' ? 'error' : type === 'info' ? 'warning' : 'warning'}`;
  alert.innerHTML = message;
  
  container.innerHTML = ''; // Clear previous alerts
  container.appendChild(alert);
  
  // Auto-hide after 5 seconds for success/info messages
  if (type === 'success' || type === 'info') {
    setTimeout(() => {
      if (alert.parentNode) {
        alert.parentNode.removeChild(alert);
      }
    }, 5000);
  }
}

function autoAssignAddresses() {
  if (lastScanResults.length === 0) {
    showAlert('Najpierw wykonaj skanowanie I2C aby znaleźć dostępne urządzenia!', 'warning');
    return;
  }
  
  const defaultAddresses = [0x68, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70];
  let assignedCount = 0;

  for (let i = 0; i < 8 && assignedCount < lastScanResults.length; i++) {
    const addressInput = document.getElementById(`i2c-address-${i}`);
    if (addressInput && lastScanResults[assignedCount] !== undefined) {
      const foundAddress = lastScanResults[assignedCount];
      addressInput.value = `0x${foundAddress.toString(16).toUpperCase()}`;
      addressInput.classList.remove('not-found', 'invalid');
      addressInput.classList.add('found');
      addressInput.title = `✅ Automatycznie przypisano znaleziony adres 0x${foundAddress.toString(16).toUpperCase()}`;
      
      // Update the configuration
      updateDeviceConfig(i);
      assignedCount++;
    }
  }
  
  // Reset remaining addresses to defaults if no more found addresses
  for (let i = assignedCount; i < 8; i++) {
    const addressInput = document.getElementById(`i2c-address-${i}`);
    if (addressInput) {
      const defaultAddress = defaultAddresses[i];
      addressInput.value = `0x${defaultAddress.toString(16).toUpperCase()}`;
      addressInput.classList.remove('found', 'not-found', 'invalid');
      addressInput.title = `Domyślny adres 0x${defaultAddress.toString(16).toUpperCase()}`;
      updateDeviceConfig(i);
    }
  }
  
  showAlert(`Automatycznie przypisano ${assignedCount} znalezionych adresów I2C.`, 'success');
}

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
#endif // MCP3424_CONFIG_HTML_H
