/**
 * WiFiConfig.h - WiFi Configuration Management
 *
 * Handles dynamic WiFi configuration with:
 * - Preferences storage for SSID/password
 * - Access Point mode for initial setup
 * - Non-blocking WiFi reconnection
 * - Web interface for configuration
 */

#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "Config.h"

// ============================================================================
// WiFi Configuration Class
// ============================================================================

class WiFiConfig {
public:
    WiFiConfig(WebServer& server);

    // Initialization
    void begin();
    void handleWiFi();  // Call in main loop for non-blocking operation

    // Configuration management
    void loadCredentials();
    void saveCredentials(const char* newSSID, const char* newPassword);

    // Status
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    bool isAPMode() const { return apMode; }
    String getSSID() const { return String(ssid); }
    String getIPAddress() const;
    int getRSSI() const { return WiFi.RSSI(); }

private:
    // WiFi management
    void initWiFi();
    void startAPMode();
    void handleReconnection();

    // Web interface handlers
    void handleConfigPage();
    void handleSaveConfig();
    void handleWiFiScan();

    // Member variables
    WebServer& webServer;
    Preferences preferences;

    char ssid[SSID_SIZE];
    char password[PASSWORD_SIZE];

    bool apMode;
    unsigned long apStartTime;

    bool wifiReconnecting;
    unsigned long wifiReconnectStartTime;
};

#endif // WIFI_CONFIG_H
