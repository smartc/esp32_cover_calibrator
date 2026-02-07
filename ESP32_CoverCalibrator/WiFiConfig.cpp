/**
 * WiFiConfig.cpp - Implementation of WiFi Configuration Management
 */

#include "WiFiConfig.h"

// WiFi reconnection settings
const unsigned long WIFI_RECONNECT_TIMEOUT = 30000;  // 30 seconds max

// ============================================================================
// Constructor
// ============================================================================

WiFiConfig::WiFiConfig(WebServer& server)
    : webServer(server),
      apMode(false),
      apStartTime(0),
      wifiReconnecting(false),
      wifiReconnectStartTime(0)
{
    memset(ssid, 0, SSID_SIZE);
    memset(password, 0, PASSWORD_SIZE);
}

// ============================================================================
// Initialization
// ============================================================================

void WiFiConfig::begin() {
    DEBUG_PRINTLN("Initializing WiFi Configuration...");

    // Load credentials from preferences
    loadCredentials();

    // Setup web interface endpoints
    webServer.on("/config", HTTP_GET, [this]() { handleConfigPage(); });
    webServer.on("/config", HTTP_POST, [this]() { handleSaveConfig(); });
    webServer.on("/wifi/scan", HTTP_GET, [this]() { handleWiFiScan(); });

    // Initialize WiFi connection
    initWiFi();

    // Start web server
    webServer.begin();
    DEBUG_PRINTF("Web server started on port %d\n", ALPACA_PORT);
}

// ============================================================================
// Configuration Management
// ============================================================================

void WiFiConfig::loadCredentials() {
    preferences.begin("wifi", true);  // Read-only
    preferences.getString("ssid", ssid, SSID_SIZE);
    preferences.getString("password", password, PASSWORD_SIZE);
    preferences.end();

    // If no credentials stored, use defaults
    if (strlen(ssid) == 0) {
        strncpy(ssid, DEFAULT_WIFI_SSID, SSID_SIZE - 1);
        strncpy(password, DEFAULT_WIFI_PASSWORD, PASSWORD_SIZE - 1);
        DEBUG_PRINTLN("No stored WiFi credentials, using defaults");
    } else {
        DEBUG_PRINTF("Loaded WiFi credentials for: %s\n", ssid);
    }
}

void WiFiConfig::saveCredentials(const char* newSSID, const char* newPassword) {
    preferences.begin("wifi", false);  // Read-write
    preferences.putString("ssid", newSSID);
    preferences.putString("password", newPassword);
    preferences.end();

    strncpy(ssid, newSSID, SSID_SIZE - 1);
    strncpy(password, newPassword, PASSWORD_SIZE - 1);

    DEBUG_PRINTF("Saved WiFi credentials for: %s\n", ssid);
}

// ============================================================================
// WiFi Management
// ============================================================================

void WiFiConfig::initWiFi() {
    DEBUG_PRINTLN("Initializing WiFi...");

    // Clean up any existing WiFi state
    WiFi.disconnect(true);
    WiFi.softAPdisconnect(true);
    delay(100);

    // Check if we have valid credentials
    if (strlen(ssid) > 0 && strcmp(ssid, "YOUR_SSID") != 0) {
        DEBUG_PRINTF("Connecting to WiFi network: %s\n", ssid);

        // Disable Power Saving Mode for better reliability
        WiFi.setSleep(WIFI_PS_NONE);

        // Set WiFi mode to station
        WiFi.mode(WIFI_STA);
        delay(100);

        WiFi.begin(ssid, password);

        // Wait up to 30 seconds for connection
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 60) {
            delay(500);
            DEBUG_PRINT(".");
            attempts++;
        }
        DEBUG_PRINTLN("");

        if (WiFi.status() == WL_CONNECTED) {
            DEBUG_PRINTLN("WiFi connected successfully!");
            DEBUG_PRINTF("IP address: %s\n", WiFi.localIP().toString().c_str());
            DEBUG_PRINTF("Signal strength: %d dBm\n", WiFi.RSSI());
            apMode = false;
        } else {
            DEBUG_PRINTF("Failed to connect to WiFi (status: %d), starting AP mode\n", WiFi.status());
            startAPMode();
        }
    } else {
        DEBUG_PRINTLN("No valid WiFi credentials configured, starting AP mode");
        startAPMode();
    }
}

void WiFiConfig::startAPMode() {
    DEBUG_PRINTLN("Starting AP mode...");

    // Clean up WiFi state before starting AP
    WiFi.disconnect(true);
    delay(100);

    // Set WiFi mode to AP
    WiFi.mode(WIFI_AP);
    delay(100);

    if (WiFi.softAP(AP_SSID, AP_PASSWORD)) {
        DEBUG_PRINTF("AP mode started - SSID: %s, Password: %s\n", AP_SSID, AP_PASSWORD);
        DEBUG_PRINTF("AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
        DEBUG_PRINTLN("Connect to this AP and navigate to http://192.168.4.1/config");
        apMode = true;
        apStartTime = millis();
    } else {
        DEBUG_PRINTLN("Failed to start AP mode!");
        // Try one more time after a longer delay
        delay(1000);
        if (WiFi.softAP(AP_SSID, AP_PASSWORD)) {
            DEBUG_PRINTLN("AP mode started on retry");
            DEBUG_PRINTF("AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
            apMode = true;
            apStartTime = millis();
        } else {
            DEBUG_PRINTLN("AP mode failed to start after retry!");
        }
    }
}

// ============================================================================
// Non-blocking WiFi Handling
// ============================================================================

void WiFiConfig::handleWiFi() {
    if (apMode) {
        // Check if we should exit AP mode after timeout
        if (millis() - apStartTime > AP_TIMEOUT) {
            DEBUG_PRINTLN("AP mode timeout, attempting to reconnect to WiFi");
            WiFi.softAPdisconnect(true);
            delay(100);
            initWiFi();
        }
    } else {
        // Check if WiFi connection is lost
        if (WiFi.status() != WL_CONNECTED) {
            if (!wifiReconnecting) {
                DEBUG_PRINTF("WiFi connection lost (status: %d), attempting to reconnect...\n", WiFi.status());

                WiFi.disconnect();
                delay(100);
                WiFi.begin(ssid, password);

                wifiReconnecting = true;
                wifiReconnectStartTime = millis();
            } else {
                // Check if reconnection timed out
                if (millis() - wifiReconnectStartTime > WIFI_RECONNECT_TIMEOUT) {
                    DEBUG_PRINTF("\nFailed to reconnect after %lu seconds, starting AP mode\n",
                                 WIFI_RECONNECT_TIMEOUT / 1000);
                    wifiReconnecting = false;
                    startAPMode();
                }
            }
        } else {
            // WiFi is connected
            if (wifiReconnecting) {
                DEBUG_PRINTLN("WiFi reconnected successfully!");
                DEBUG_PRINTF("IP address: %s\n", WiFi.localIP().toString().c_str());
                wifiReconnecting = false;
            }
        }
    }
}

// ============================================================================
// Web Interface Handlers
// ============================================================================

void WiFiConfig::handleConfigPage() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>WiFi Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        h1 { color: #333; border-bottom: 2px solid #4CAF50; padding-bottom: 10px; }
        h2 { color: #666; margin-top: 30px; }
        input[type=text], input[type=password], select { width: 100%; padding: 12px; margin: 8px 0; box-sizing: border-box; border: 1px solid #ddd; border-radius: 4px; }
        button { background: #4CAF50; color: white; padding: 14px 20px; margin: 10px 0; border: none; border-radius: 4px; cursor: pointer; width: 100%; font-size: 16px; }
        button:hover { background: #45a049; }
        .scan-btn { background: #2196F3; }
        .scan-btn:hover { background: #0b7dda; }
        .info { background: #e7f3ff; padding: 10px; border-left: 4px solid #2196F3; margin: 10px 0; }
        .network-list { list-style: none; padding: 0; }
        .network-list li { padding: 10px; margin: 5px 0; background: #f9f9f9; border-radius: 4px; cursor: pointer; }
        .network-list li:hover { background: #e0e0e0; }
        .signal { float: right; color: #666; }
    </style>
</head>
<body>
    <div class="container">
        <h1>📡 WiFi Configuration</h1>

        <div class="info">
            <strong>Current Status:</strong><br>
            Mode: )rawliteral" + String(apMode ? "AP Mode" : "Station Mode") + R"rawliteral(<br>
            )rawliteral" + (apMode ? String("AP SSID: " AP_SSID) : String("Connected to: ") + getSSID()) + R"rawliteral(<br>
            )rawliteral" + (apMode ? String("AP IP: ") + WiFi.softAPIP().toString() : String("IP: ") + WiFi.localIP().toString()) + R"rawliteral(
            )rawliteral" + (!apMode && isConnected() ? String("<br>Signal: ") + String(getRSSI()) + " dBm" : "") + R"rawliteral(
        </div>

        <h2>Scan for Networks</h2>
        <button class="scan-btn" onclick="scanNetworks()">Scan WiFi Networks</button>
        <ul class="network-list" id="networks"></ul>

        <h2>Configure WiFi</h2>
        <form method="POST" action="/config">
            <label for="ssid">SSID:</label>
            <input type="text" id="ssid" name="ssid" value="" placeholder="Enter WiFi SSID" required>

            <label for="password">Password:</label>
            <input type="password" id="password" name="password" value="" placeholder="Enter WiFi password" required>

            <button type="submit">Save and Connect</button>
        </form>

        <div class="info" style="margin-top: 20px;">
            <strong>Note:</strong> After saving, the device will restart and attempt to connect to the new network.
            If connection fails, it will return to AP mode after 30 seconds.
        </div>
    </div>

    <script>
        function scanNetworks() {
            document.getElementById('networks').innerHTML = '<li>Scanning...</li>';
            fetch('/wifi/scan')
                .then(response => response.json())
                .then(data => {
                    let html = '';
                    data.networks.forEach(network => {
                        html += `<li onclick="selectNetwork('${network.ssid}')">
                                   ${network.ssid} <span class="signal">${network.rssi} dBm</span>
                                 </li>`;
                    });
                    document.getElementById('networks').innerHTML = html || '<li>No networks found</li>';
                })
                .catch(err => {
                    document.getElementById('networks').innerHTML = '<li>Scan failed</li>';
                });
        }

        function selectNetwork(ssid) {
            document.getElementById('ssid').value = ssid;
            document.getElementById('password').focus();
        }
    </script>
</body>
</html>
)rawliteral";

    webServer.send(200, "text/html", html);
}

void WiFiConfig::handleSaveConfig() {
    if (webServer.hasArg("ssid") && webServer.hasArg("password")) {
        String newSSID = webServer.arg("ssid");
        String newPassword = webServer.arg("password");

        saveCredentials(newSSID.c_str(), newPassword.c_str());

        String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>WiFi Configuration Saved</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta http-equiv="refresh" content="10;url=/">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; text-align: center; padding-top: 50px; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 40px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        h1 { color: #4CAF50; }
        .spinner { border: 4px solid #f3f3f3; border-top: 4px solid #4CAF50; border-radius: 50%; width: 40px; height: 40px; animation: spin 1s linear infinite; margin: 20px auto; }
        @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
    </style>
</head>
<body>
    <div class="container">
        <h1>✓ Configuration Saved</h1>
        <div class="spinner"></div>
        <p>Connecting to: <strong>)rawliteral" + newSSID + R"rawliteral(</strong></p>
        <p>The device will restart and attempt to connect.</p>
        <p>If successful, you can access it at the new IP address.</p>
        <p>This page will redirect in 10 seconds...</p>
    </div>
</body>
</html>
)rawliteral";

        webServer.send(200, "text/html", html);

        // Restart after a short delay to allow response to be sent
        delay(2000);
        ESP.restart();
    } else {
        webServer.send(400, "text/plain", "Missing SSID or password");
    }
}

void WiFiConfig::handleWiFiScan() {
    DEBUG_PRINTLN("Scanning for WiFi networks...");

    int n = WiFi.scanNetworks();

    String json = "{\"networks\":[";
    for (int i = 0; i < n; i++) {
        if (i > 0) json += ",";
        json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    json += "]}";

    webServer.send(200, "application/json", json);
}

// ============================================================================
// Status Methods
// ============================================================================

String WiFiConfig::getIPAddress() const {
    if (apMode) {
        return WiFi.softAPIP().toString();
    } else {
        return WiFi.localIP().toString();
    }
}
