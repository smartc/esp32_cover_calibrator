/**
 * Config.h - Configuration constants for ESP32 Cover Calibrator
 *
 * Hardware: ESP32-S3-DEV-KIT with DS3218 Servo
 * PCB: ESP32 ROR Controller v3.1
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

// Servo Pin Configuration
#define SERVO_PIN           10          // IO10 via H4 header

// Servo PWM Parameters
#define SERVO_MIN_US        500         // Minimum pulse width (0°)
#define SERVO_MAX_US        2500        // Maximum pulse width (270°)
#define SERVO_MAX_DEG       270         // Maximum servo angle
#define SERVO_FREQ_HZ       50          // PWM frequency (50 Hz = 20ms period)

// ============================================================================
// COVER CALIBRATOR CONFIGURATION
// ============================================================================

// Cover Position Angles (default range: 90-180, future: 0-270)
#define COVER_CLOSED_ANGLE  90.0f       // Fully closed position
#define COVER_OPEN_ANGLE    180.0f      // Fully open position

// Motion Parameters
#define MOTION_INTERVAL_MS  20          // Update interval for motion control
#define SCURVE_DURATION_MS  4000        // S-curve motion duration (4 seconds)
#define SCURVE_STEEPNESS    6.0f        // S-curve steepness (4.0-10.0)

// Timeout Settings
#define COVER_MOVE_TIMEOUT_MS   10000   // Maximum time for cover movement

// ============================================================================
// SERIAL INTERFACE CONFIGURATION
// ============================================================================

#define SERIAL_BAUD_RATE    115200      // Serial communication baud rate

// ============================================================================
// ASCOM ALPACA CONFIGURATION
// ============================================================================

// WiFi Configuration (stored in Preferences, these are defaults)
#define DEFAULT_WIFI_SSID       "YOUR_SSID"     // Default WiFi SSID (will use AP mode if not changed)
#define DEFAULT_WIFI_PASSWORD   "YOUR_PASSWORD" // Default WiFi password
#define SSID_SIZE               32              // Maximum SSID length
#define PASSWORD_SIZE           64              // Maximum password length

// Access Point Configuration (for initial setup or WiFi failure)
#define AP_SSID                 "CoverCalibrator_Setup"  // AP mode SSID
#define AP_PASSWORD             "covercal123"            // AP mode password (min 8 chars)
#define AP_TIMEOUT              300000                   // AP timeout (5 minutes)

// Network Configuration
#define ALPACA_PORT         80                  // HTTP port (ASCOM clients will be configured to use this)
#define ALPACA_DISCOVERY_PORT   32227           // ASCOM discovery port

// Device Information
#define DEVICE_NAME         "ESP32 Cover Calibrator"
#define DEVICE_TYPE         "CoverCalibrator"
#define DEVICE_NUMBER       0
#define MANUFACTURER        "DIY Astronomy"
#define MANUFACTURER_VERSION    "1.0.0"
#define DEVICE_VERSION      "1.0.0"
#define DRIVER_INFO         "ESP32-based ASCOM Cover Calibrator with DS3218 Servo"
#define DRIVER_VERSION      "1.0.0"
#define UNIQUE_ID           "12345678-1234-1234-1234-123456789012"  // UUID format

// API Configuration
#define API_VERSION_MAJOR   1
#define API_VERSION_MINOR   0

// ============================================================================
// FLAT PANEL CONFIGURATION (NOT YET IMPLEMENTED)
// ============================================================================

#define FLAT_PANEL_ENABLED  false       // Flat panel feature flag
#define MAX_BRIGHTNESS      255         // Maximum brightness value

// ============================================================================
// DEBUG CONFIGURATION
// ============================================================================

#define DEBUG_ENABLED       true        // Enable debug output
#define DEBUG_SERIAL        Serial      // Debug output serial port

#if DEBUG_ENABLED
  #define DEBUG_PRINT(x)    DEBUG_SERIAL.print(x)
  #define DEBUG_PRINTLN(x)  DEBUG_SERIAL.println(x)
  #define DEBUG_PRINTF(...) DEBUG_SERIAL.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H
