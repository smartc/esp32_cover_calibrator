/**
 * ESP32_CoverCalibrator.ino - ESP32 Cover Calibrator Main Sketch
 *
 * Telescope Cover Calibrator with DS3218 Servo Motor
 * - Serial (USB) Interface - ASCOM compliant
 * - ASCOM Alpaca REST API Interface
 * - S-Curve motion control for smooth operation
 * - Dynamic WiFi configuration with AP mode
 *
 * Hardware:
 *   - Board: ESP32-S3 (Waveshare ESP32-S3-DEV-KIT-NxR8)
 *   - PCB: ESP32 ROR Controller v3.1 (JLCPCB-003)
 *   - Servo: Miuzei DS3218 20KG, 270° variant
 *   - Pin: IO10 (via H4 header)
 *
 * Functionality:
 *   - Open/Close telescope cover with configurable angles
 *   - Default: 90° (closed) to 180° (open)
 *   - Future: 0° to 270° range
 *   - Flat panel control (not yet implemented)
 *   - Dynamic WiFi configuration via web interface
 *
 * Interfaces:
 *   - Serial: 115200 baud, ASCOM standard commands
 *   - Alpaca: REST API on port 11111
 *   - Web UI: WiFi configuration at http://[ip]/config
 *
 * WiFi Configuration:
 *   - On first boot, device starts in AP mode (SSID: CoverCalibrator_Setup)
 *   - Connect to AP and navigate to http://192.168.4.1/config
 *   - Configure WiFi credentials via web interface
 *   - Credentials stored in ESP32 Preferences (persistent)
 *
 * Author: DIY Astronomy
 * Version: 1.1.0
 * Date: 2026-02-07
 */

#include <WebServer.h>
#include "Config.h"
#include "CoverCalibrator.h"
#include "WiFiConfig.h"
#include "ASCOMAlpaca.h"
#include "SerialInterface.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

WebServer webServer(ALPACA_PORT);            // Shared web server for all interfaces
CoverCalibrator coverCalibrator;
WiFiConfig wifiConfig(webServer);
ASCOMAlpaca alpacaServer(coverCalibrator, webServer);
SerialInterface serialInterface(coverCalibrator);

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    // Initialize serial interface first for debugging output
    serialInterface.begin();

    DEBUG_PRINTLN();
    DEBUG_PRINTLN("══════════════════════════════════════════════════");
    DEBUG_PRINTLN("   ESP32 Cover Calibrator Initialization");
    DEBUG_PRINTLN("══════════════════════════════════════════════════");
    DEBUG_PRINTLN();

    // Initialize cover calibrator hardware
    DEBUG_PRINTLN("Initializing cover calibrator...");
    coverCalibrator.begin();
    DEBUG_PRINTLN("Cover calibrator initialized");
    DEBUG_PRINTLN();

    // Initialize WiFi configuration (loads credentials and connects)
    DEBUG_PRINTLN("Initializing WiFi...");
    wifiConfig.begin();

    if (wifiConfig.isAPMode()) {
        DEBUG_PRINTLN();
        DEBUG_PRINTLN("═══════════════════════════════════════════════");
        DEBUG_PRINTLN("    AP MODE - WiFi Configuration Required");
        DEBUG_PRINTLN("═══════════════════════════════════════════════");
        DEBUG_PRINTF("  SSID: %s\n", AP_SSID);
        DEBUG_PRINTF("  Password: %s\n", AP_PASSWORD);
        DEBUG_PRINTF("  IP: %s\n", wifiConfig.getIPAddress().c_str());
        DEBUG_PRINTLN();
        DEBUG_PRINTLN("  Connect to this network and navigate to:");
        DEBUG_PRINTF("  http://%s/config\n", wifiConfig.getIPAddress().c_str());
        DEBUG_PRINTLN("═══════════════════════════════════════════════");
        DEBUG_PRINTLN();
    } else if (wifiConfig.isConnected()) {
        DEBUG_PRINTLN("WiFi connected successfully");
        DEBUG_PRINTF("IP address: %s\n", wifiConfig.getIPAddress().c_str());
        DEBUG_PRINTF("Signal strength: %d dBm\n", wifiConfig.getRSSI());
    }
    DEBUG_PRINTLN();

    // Initialize ASCOM Alpaca server
    DEBUG_PRINTLN("Initializing ASCOM Alpaca server...");
    alpacaServer.begin();
    DEBUG_PRINTLN("ASCOM Alpaca server initialized");
    if (wifiConfig.isConnected()) {
        DEBUG_PRINTF("Server address: http://%s:%d\n",
                     wifiConfig.getIPAddress().c_str(), ALPACA_PORT);
        DEBUG_PRINTF("Configuration: http://%s/config\n",
                     wifiConfig.getIPAddress().c_str());
    }
    DEBUG_PRINTLN();

    DEBUG_PRINTLN("══════════════════════════════════════════════════");
    DEBUG_PRINTLN("   Initialization Complete");
    DEBUG_PRINTLN("══════════════════════════════════════════════════");
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("Device ready. Waiting for commands...");
    if (!wifiConfig.isConnected() && !wifiConfig.isAPMode()) {
        DEBUG_PRINTLN("NOTE: WiFi not connected - Serial interface available");
    }
    DEBUG_PRINTLN();

    // Show status on startup
    delay(500);
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    // Handle WiFi connection (non-blocking)
    wifiConfig.handleWiFi();

    // Update all subsystems
    coverCalibrator.update();      // Update servo motion control
    webServer.handleClient();      // Handle HTTP requests (Alpaca + Config)
    serialInterface.update();      // Handle serial commands

    // Small delay to prevent CPU hogging
    delay(1);
}
