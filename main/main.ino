/**
 * main.ino - ESP32 Cover Calibrator Main Sketch
 *
 * Telescope Cover Calibrator with DS3218 Servo Motor
 * - Serial (USB) Interface - ASCOM compliant
 * - ASCOM Alpaca REST API Interface
 * - S-Curve motion control for smooth operation
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
 *
 * Interfaces:
 *   - Serial: 115200 baud, ASCOM standard commands
 *   - Alpaca: REST API on port 11111
 *
 * Author: DIY Astronomy
 * Version: 1.0.0
 * Date: 2026-02-07
 */

#include "Config.h"
#include "CoverCalibrator.h"
#include "ASCOMAlpaca.h"
#include "SerialInterface.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

CoverCalibrator coverCalibrator;
ASCOMAlpaca alpacaServer(coverCalibrator);
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

    // Initialize ASCOM Alpaca server
    DEBUG_PRINTLN("Initializing ASCOM Alpaca server...");
    alpacaServer.begin();
    if (alpacaServer.isWiFiConnected()) {
        DEBUG_PRINTLN("ASCOM Alpaca server initialized");
        DEBUG_PRINTF("Server address: http://%s:%d\n",
                     alpacaServer.getIPAddress().c_str(), ALPACA_PORT);
    } else {
        DEBUG_PRINTLN("WARNING: WiFi not connected - Alpaca server unavailable");
        DEBUG_PRINTLN("Serial interface will still work");
    }
    DEBUG_PRINTLN();

    DEBUG_PRINTLN("══════════════════════════════════════════════════");
    DEBUG_PRINTLN("   Initialization Complete");
    DEBUG_PRINTLN("══════════════════════════════════════════════════");
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("Device ready. Waiting for commands...");
    DEBUG_PRINTLN();

    // Show status on startup
    delay(500);
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    // Update all subsystems
    coverCalibrator.update();      // Update servo motion control
    alpacaServer.update();         // Handle HTTP requests
    serialInterface.update();      // Handle serial commands

    // Small delay to prevent CPU hogging
    delay(1);
}
