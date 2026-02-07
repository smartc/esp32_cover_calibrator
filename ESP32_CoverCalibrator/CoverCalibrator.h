/**
 * CoverCalibrator.h - Cover Calibrator Device Controller
 *
 * Controls a telescope cover with DS3218 servo motor using S-curve motion
 * Provides ASCOM-compliant interface for cover and calibrator control
 */

#ifndef COVER_CALIBRATOR_H
#define COVER_CALIBRATOR_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include "Config.h"

// ============================================================================
// ASCOM ENUMERATIONS
// ============================================================================

// ASCOM CoverStatus enumeration
enum CoverStatus {
    COVER_NOT_PRESENT = 0,
    COVER_CLOSED = 1,
    COVER_MOVING = 2,
    COVER_OPEN = 3,
    COVER_UNKNOWN = 4,
    COVER_ERROR = 5
};

// ASCOM CalibratorStatus enumeration
enum CalibratorStatus {
    CALIBRATOR_NOT_PRESENT = 0,
    CALIBRATOR_OFF = 1,
    CALIBRATOR_NOT_READY = 2,
    CALIBRATOR_READY = 3,
    CALIBRATOR_UNKNOWN = 4,
    CALIBRATOR_ERROR = 5
};

// ============================================================================
// COVER CALIBRATOR CLASS
// ============================================================================

class CoverCalibrator {
public:
    // Constructor
    CoverCalibrator();

    // Initialization
    void begin();
    void update();  // Call in main loop to handle motion updates

    // ========================================================================
    // ASCOM COMMON METHODS
    // ========================================================================

    bool isConnected() const { return connected; }
    void setConnected(bool state);
    String getName() const { return DEVICE_NAME; }
    String getDescription() const { return DRIVER_INFO; }
    String getDriverInfo() const { return DRIVER_INFO; }
    String getDriverVersion() const { return DRIVER_VERSION; }

    // ========================================================================
    // COVER CONTROL METHODS
    // ========================================================================

    // Cover commands
    void openCover();
    void closeCover();
    void haltCover();

    // Cover status
    CoverStatus getCoverState() const { return coverState; }
    bool isCoverMoving() const { return coverState == COVER_MOVING; }

    // Cover configuration
    void setCoverAngles(float closedAngle, float openAngle);
    float getClosedAngle() const { return coverClosedAngle; }
    float getOpenAngle() const { return coverOpenAngle; }

    // Persistent storage
    void loadConfiguration();
    void saveConfiguration();
    void savePosition();

    // ========================================================================
    // CALIBRATOR CONTROL METHODS (Flat Panel - Not Yet Implemented)
    // ========================================================================

    // Calibrator commands
    void calibratorOn(int brightness);
    void calibratorOff();

    // Calibrator status
    CalibratorStatus getCalibratorState() const { return calibratorState; }
    int getBrightness() const { return currentBrightness; }
    int getMaxBrightness() const { return MAX_BRIGHTNESS; }

    // ========================================================================
    // SERVO CONTROL METHODS
    // ========================================================================

    float getCurrentAngle() const { return currentAngle; }
    float getTargetAngle() const { return targetAngle; }

private:
    // ========================================================================
    // SERVO MOTION FUNCTIONS (from sample code)
    // ========================================================================

    // Utility functions
    float mapf(float x, float in_min, float in_max, float out_min, float out_max);
    int degreesToMicroseconds(float angle);
    void setServoAngle(float angle);

    // S-Curve motion control
    float sigmoid(float t, float steepness);
    float normalizedSigmoid(float t, float steepness);
    void startScurveMotion(float startAngle, float endAngle, unsigned long durationMs, float steepness);
    void updateScurveMotion();  // Called from update() to progress motion

    // ========================================================================
    // MEMBER VARIABLES
    // ========================================================================

    // Servo hardware
    Servo servo;
    float currentAngle;
    float targetAngle;

    // Cover configuration
    float coverClosedAngle;
    float coverOpenAngle;

    // Cover state
    CoverStatus coverState;
    bool connected;

    // Calibrator state (flat panel - not yet implemented)
    CalibratorStatus calibratorState;
    int currentBrightness;

    // S-Curve motion state
    bool isMoving;
    float motionStartAngle;
    float motionEndAngle;
    unsigned long motionStartTime;
    unsigned long motionDuration;
    float motionSteepness;

    // Timeout handling
    unsigned long motionTimeoutStart;
    bool motionTimedOut;

    // Persistent storage
    Preferences preferences;
};

#endif // COVER_CALIBRATOR_H
