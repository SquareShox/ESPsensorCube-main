#ifndef NETWORK_CONFIG_HTML_H
#define NETWORK_CONFIG_HTML_H
const char *network_config_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP Sensor Cube - Konfiguracja Sieci</title>
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
    max-width: 800px;
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
  }

  .wifi-icon { background: linear-gradient(45deg, #2196f3, #03a9f4); }
  .network-icon { background: linear-gradient(45deg, #4caf50, #8bc34a); }
  .status-icon { background: linear-gradient(45deg, #9c27b0, #e91e63); }

  .form-group {
    margin-bottom: 20px;
  }

  .form-label {
    display: block;
    font-weight: bold;
    color: #333;
    margin-bottom: 8px;
  }

  .form-input {
    width: 100%;
    padding: 12px;
    border: 2px solid #ddd;
    border-radius: 8px;
    font-size: 16px;
    transition: border-color 0.3s ease;
  }

  .form-input:focus {
    outline: none;
    border-color: #667eea;
    box-shadow: 0 0 10px rgba(102, 126, 234, 0.3);
  }

  .form-input:disabled {
    background-color: #f5f5f5;
    color: #666;
  }

  .radio-group {
    display: flex;
    gap: 20px;
    margin-bottom: 20px;
  }

  .radio-option {
    display: flex;
    align-items: center;
    cursor: pointer;
  }

  .radio-option input[type="radio"] {
    margin-right: 8px;
    transform: scale(1.2);
  }

  .radio-option label {
    font-weight: bold;
    color: #333;
    cursor: pointer;
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
    margin: 10px 5px;
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

  /* Configuration Styles */
  .config-sections {
    display: grid;
    gap: 15px;
  }

  .config-section {
    background: rgba(255, 255, 255, 0.05);
    border-radius: 8px;
    padding: 15px;
    border: 1px solid rgba(255, 255, 255, 0.1);
  }

  .config-section h4 {
    margin: 0 0 10px 0;
    color: #3498db;
    font-size: 14px;
    font-weight: 600;
  }

  .config-checkbox {
    display: flex;
    align-items: center;
    margin: 8px 0;
    cursor: pointer;
    font-size: 13px;
    color: #ecf0f1;
    position: relative;
    padding-left: 25px;
  }

  .config-checkbox input[type="checkbox"] {
    position: absolute;
    opacity: 0;
    cursor: pointer;
    height: 0;
    width: 0;
  }

  .config-checkbox .checkmark {
    position: absolute;
    left: 0;
    top: 0;
    height: 18px;
    width: 18px;
    background-color: rgba(255, 255, 255, 0.1);
    border: 2px solid #3498db;
    border-radius: 3px;
    transition: all 0.2s ease;
  }

  .config-checkbox:hover input ~ .checkmark {
    background-color: rgba(52, 152, 219, 0.2);
  }

  .config-checkbox input:checked ~ .checkmark {
    background-color: #3498db;
    border-color: #3498db;
  }

  .config-checkbox .checkmark:after {
    content: "";
    position: absolute;
    display: none;
  }

  .config-checkbox input:checked ~ .checkmark:after {
    display: block;
  }

  .config-checkbox .checkmark:after {
    left: 5px;
    top: 2px;
    width: 4px;
    height: 8px;
    border: solid white;
    border-width: 0 2px 2px 0;
    transform: rotate(45deg);
  }

  .config-input-group {
    display: flex;
    gap: 10px;
    align-items: center;
    margin: 10px 0;
  }

  .config-input {
    flex: 1;
    padding: 8px 12px;
    border: 1px solid #3498db;
    border-radius: 4px;
    background: rgba(255, 255, 255, 0.1);
    color: #ecf0f1;
    font-size: 13px;
  }

  .config-input:focus {
    outline: none;
    border-color: #2980b9;
    background: rgba(255, 255, 255, 0.15);
  }

  .config-buttons {
    display: flex;
    flex-direction: column;
    gap: 10px;
  }

  .config-buttons .btn {
    margin: 0;
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

  @media (max-width: 768px) {
    .config-container {
      margin: 10px;
      padding: 20px;
    }
    
    .header h1 {
      font-size: 2em;
    }
    
    .radio-group {
      flex-direction: column;
      gap: 10px;
    }
  }
</style>
<script src="/common.js"></script>
</head>
<body>
  <div class="header">
    <h1>🌐 ESP Sensor Cube - Konfiguracja Sieci</h1>
    <p>Zarządzanie ustawieniami WiFi i sieci</p>
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

    <!-- WiFi Configuration Section -->
    <div class="config-section">
      <div class="section-title">
        <div class="section-icon wifi-icon">📶</div>
        Konfiguracja WiFi
      </div>
      
      <div class="form-group">
        <label class="form-label">SSID (Nazwa sieci):</label>
        <input type="text" id="wifi-ssid" class="form-input" placeholder="Wprowadź nazwę sieci WiFi">
      </div>
      
      <div class="form-group">
        <label class="form-label">Hasło:</label>
        <input type="password" id="wifi-password" class="form-input" placeholder="Wprowadź hasło WiFi">
      </div>
      
      <button class="btn btn-success" onclick="saveWiFiConfig()">
        💾 Zapisz WiFi
      </button>
      
      <button class="btn btn-warning" onclick="testWiFiConnection()">
        🔍 Testuj Połączenie
      </button>
    </div>

    <!-- Network Configuration Section -->
    <div class="config-section">
      <div class="section-title">
        <div class="section-icon network-icon">🌐</div>
        Konfiguracja Sieci
      </div>
      
      <div class="radio-group">
        <div class="radio-option">
          <input type="radio" id="dhcp" name="ip-mode" value="dhcp" checked>
          <label for="dhcp">DHCP (Automatyczny)</label>
        </div>
        <div class="radio-option">
          <input type="radio" id="static" name="ip-mode" value="static">
          <label for="static">Statyczny IP</label>
        </div>
      </div>
      
      <div id="static-ip-fields" style="display: none;">
        <div class="form-group">
          <label class="form-label">Statyczny IP:</label>
          <input type="text" id="static-ip" class="form-input" placeholder="192.168.1.100">
        </div>
        
        <div class="form-group">
          <label class="form-label">Brama (Gateway):</label>
          <input type="text" id="gateway" class="form-input" placeholder="192.168.1.1">
        </div>
        
        <div class="form-group">
          <label class="form-label">Maska podsieci:</label>
          <input type="text" id="subnet" class="form-input" placeholder="255.255.255.0">
        </div>
        
        <div class="form-group">
          <label class="form-label">DNS 1:</label>
          <input type="text" id="dns1" class="form-input" placeholder="8.8.8.8">
        </div>
        
        <div class="form-group">
          <label class="form-label">DNS 2:</label>
          <input type="text" id="dns2" class="form-input" placeholder="8.8.4.4">
        </div>
      </div>
      
      <button class="btn btn-success" onclick="saveNetworkConfig()">
        💾 Zapisz Sieć
      </button>
      
      <button class="btn btn-warning" onclick="applyNetworkConfig()">
        🔄 Zastosuj Konfigurację
      </button>
    </div>

    <!-- Current Status Section -->
    <div class="config-section">
      <div class="section-title">
        <div class="section-icon status-icon">📊</div>
        Aktualny Status
      </div>
      
      <div class="status-text">
        <strong>Połączenie WiFi:</strong> <span id="wifi-status">Sprawdzanie...</span>
        <span class="status-indicator" id="wifi-indicator"></span><br>
        <strong>SSID:</strong> <span id="current-ssid">--</span><br>
        <strong>IP Adres:</strong> <span id="current-ip">--</span><br>
        <strong>Sygnał:</strong> <span id="wifi-signal">--</span> dBm<br>
        <strong>Tryb IP:</strong> <span id="ip-mode">--</span>
      </div>
      
      <button class="btn btn-warning" onclick="refreshStatus()">
        🔄 Odśwież Status
      </button>
      
      <button class="btn btn-danger" onclick="resetConfig()">
        🗑️ Resetuj Konfigurację
      </button>
    </div>
  </div>

<script>
let ws;

function connectWebSocket() {
  ws = new WebSocket(`ws://${window.location.host}/ws`);
  ws.onopen = function() {
    console.log('WebSocket connected');
    loadCurrentConfig();
  };
  ws.onclose = function() {
    console.log('WebSocket disconnected, reconnecting...');
    setTimeout(connectWebSocket, 5000);
  };
  ws.onmessage = function(event) {
    try {
      const data = JSON.parse(event.data);
      console.log('Network config WebSocket message:', data.cmd, data);
      
      if (data.cmd === "networkConfig") {
        updateNetworkConfigDisplay(data);
      } else if (data.cmd === "setWiFiConfig") {
        if (data.success) {
          showAlert(data.message || 'Konfiguracja WiFi zapisana pomyślnie', 'success');
        } else {
          showAlert(data.error || 'Błąd zapisu konfiguracji WiFi', 'error');
        }
      } else if (data.cmd === "setNetworkConfig") {
        if (data.success) {
          showAlert(data.message || 'Konfiguracja sieci zapisana pomyślnie', 'success');
        } else {
          showAlert(data.error || 'Błąd zapisu konfiguracji sieci', 'error');
        }
      } else if (data.cmd === "testWiFi") {
        if (data.wifiConnected) {
          showAlert(`WiFi połączony: ${data.ssid} (${data.localIP})`, 'success');
        } else {
          showAlert('WiFi nie połączony', 'error');
        }
      } else if (data.cmd === "applyNetworkConfig") {
        if (data.success) {
          showAlert(data.message || 'Konfiguracja sieci zastosowana', 'success');
        } else {
          showAlert(data.error || 'Błąd zastosowania konfiguracji', 'error');
        }
      } else if (data.cmd === "resetNetworkConfig") {
        if (data.success) {
          showAlert(data.message || 'Konfiguracja zresetowana', 'success');
          // Reload config after reset
          setTimeout(() => loadCurrentConfig(), 1000);
        } else {
          showAlert(data.error || 'Błąd resetowania konfiguracji', 'error');
        }
      }
    } catch (error) {
      console.error('Error parsing WebSocket data:', error);
    }
  };
}


function loadCurrentConfig() {
  console.log('Loading current network config...');
  if (ws && ws.readyState === WebSocket.OPEN) {
    const command = {cmd: "getNetworkConfig"};
    console.log('Sending command:', command);
    ws.send(JSON.stringify(command));
  } else {
    console.log('WebSocket not connected, state:', ws ? ws.readyState : 'no ws');
    showAlert('Brak połączenia WebSocket!', 'error');
  }
}

function updateNetworkConfigDisplay(data) {
  console.log('Updating network config display:', data);
  
  // Update WiFi fields
  document.getElementById('wifi-ssid').value = data.wifiSSID || '';
  document.getElementById('wifi-password').value = data.wifiPassword || '';
  
  // Update network fields
  document.getElementById('static-ip').value = data.staticIP || '192.168.1.100';
  document.getElementById('gateway').value = data.gateway || '192.168.1.1';
  document.getElementById('subnet').value = data.subnet || '255.255.255.0';
  document.getElementById('dns1').value = data.dns1 || '8.8.8.8';
  document.getElementById('dns2').value = data.dns2 || '8.8.4.4';
  
  // Update radio buttons
  if (data.useDHCP) {
    document.getElementById('dhcp').checked = true;
    document.getElementById('static-ip-fields').style.display = 'none';
  } else {
    document.getElementById('static').checked = true;
    document.getElementById('static-ip-fields').style.display = 'block';
  }
  
  // Update status
  document.getElementById('wifi-status').textContent = data.wifiConnected ? 'Połączony' : 'Rozłączony';
  document.getElementById('wifi-indicator').className = `status-indicator ${data.wifiConnected ? 'status-ok' : 'status-error'}`;
  document.getElementById('current-ssid').textContent = data.currentSSID || '--';
  document.getElementById('current-ip').textContent = data.currentIP || '--';
  document.getElementById('wifi-signal').textContent = data.wifiSignal || '--';
  document.getElementById('ip-mode').textContent = data.useDHCP ? 'DHCP' : 'Statyczny';
}

function saveWiFiConfig() {
  const ssid = document.getElementById('wifi-ssid').value.trim();
  const password = document.getElementById('wifi-password').value;
  
  console.log('Saving WiFi config:', { ssid, password: password ? '***' : 'empty' });
  
  if (!ssid) {
    showAlert('Wprowadź nazwę sieci WiFi!', 'error');
    return;
  }
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    const command = {
      cmd: "setWiFiConfig",
      ssid: ssid,
      password: password
    };
    console.log('Sending WiFi config command:', command);
    ws.send(JSON.stringify(command));
    showAlert('Konfiguracja WiFi wysłana...', 'info');
  } else {
    console.log('WebSocket not connected, state:', ws ? ws.readyState : 'no ws');
    showAlert('Brak połączenia WebSocket!', 'error');
  }
}

function saveNetworkConfig() {
  const useDHCP = document.getElementById('dhcp').checked;
  const staticIP = document.getElementById('static-ip').value.trim();
  const gateway = document.getElementById('gateway').value.trim();
  const subnet = document.getElementById('subnet').value.trim();
  const dns1 = document.getElementById('dns1').value.trim();
  const dns2 = document.getElementById('dns2').value.trim();
  
  console.log('Saving network config:', { useDHCP, staticIP, gateway, subnet, dns1, dns2 });
  
  if (!useDHCP) {
    if (!staticIP || !gateway || !subnet) {
      showAlert('Wypełnij wszystkie pola dla statycznego IP!', 'error');
      return;
    }
  }
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    const command = {
      cmd: "setNetworkConfig",
      useDHCP: useDHCP,
      staticIP: staticIP,
      gateway: gateway,
      subnet: subnet,
      dns1: dns1,
      dns2: dns2
    };
    console.log('Sending network config command:', command);
    ws.send(JSON.stringify(command));
    showAlert('Konfiguracja sieci wysłana...', 'info');
  } else {
    console.log('WebSocket not connected, state:', ws ? ws.readyState : 'no ws');
    showAlert('Brak połączenia WebSocket!', 'error');
  }
}

function testWiFiConnection() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({cmd: "testWiFi"}));
    showAlert('Testowanie połączenia WiFi...', 'info');
  } else {
    showAlert('Brak połączenia WebSocket!', 'error');
  }
}

function applyNetworkConfig() {
  if (confirm('Czy na pewno chcesz zastosować nową konfigurację sieci? System może się zrestartować.')) {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({cmd: "applyNetworkConfig"}));
      showAlert('Stosowanie konfiguracji sieci...', 'info');
    } else {
      showAlert('Brak połączenia WebSocket!', 'error');
    }
  }
}

function refreshStatus() {
  loadCurrentConfig();
  showAlert('Status odświeżony', 'success');
}

function resetConfig() {
  if (confirm('Czy na pewno chcesz zresetować całą konfigurację sieci? To usunie wszystkie zapisane ustawienia.')) {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({cmd: "resetNetworkConfig"}));
      showAlert('Resetowanie konfiguracji...', 'info');
    } else {
      showAlert('Brak połączenia WebSocket!', 'error');
    }
  }
}

// Event listeners for radio buttons
document.getElementById('dhcp').addEventListener('change', function() {
  document.getElementById('static-ip-fields').style.display = 'none';
});

document.getElementById('static').addEventListener('change', function() {
  document.getElementById('static-ip-fields').style.display = 'block';
});

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
#endif // NETWORK_CONFIG_HTML_H
