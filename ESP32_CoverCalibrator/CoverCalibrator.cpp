/**
 * CoverCalibrator.cpp - Implementation of Cover Calibrator Device Controller
 */

#include "CoverCalibrator.h"
#include <math.h>

// ============================================================================
// CONSTRUCTOR
// ============================================================================

CoverCalibrator::CoverCalibrator()
    : currentAngle(0.0f),
      targetAngle(0.0f),
      coverClosedAngle(COVER_CLOSED_ANGLE),
      coverOpenAngle(COVER_OPEN_ANGLE),
      coverState(COVER_UNKNOWN),
      connected(false),
      calibratorState(CALIBRATOR_NOT_PRESENT),
      currentBrightness(0),
      isMoving(false),
      motionStartAngle(0.0f),
      motionEndAngle(0.0f),
      motionStartTime(0),
      motionDuration(0),
      motionSteepness(SCURVE_STEEPNESS),
      motionTimeoutStart(0),
      motionTimedOut(false)
{
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void CoverCalibrator::begin() {
    DEBUG_PRINTLN("Initializing CoverCalibrator...");

    // Load configuration from preferences (angles and last position)
    loadConfiguration();

    // Attach servo
    ESP32PWM::allocateTimer(0);
    servo.setPeriodHertz(SERVO_FREQ_HZ);
    servo.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);

    // IMPORTANT: Do NOT move servo on startup!
    // Just set the servo to the last known position without physically moving it
    // This prevents unwanted movement on power-up
    servo.writeMicroseconds(degreesToMicroseconds(currentAngle));

    // Determine cover state based on last position
    if (fabs(currentAngle - coverClosedAngle) < 1.0f) {
        coverState = COVER_CLOSED;
    } else if (fabs(currentAngle - coverOpenAngle) < 1.0f) {
        coverState = COVER_OPEN;
    } else {
        coverState = COVER_UNKNOWN;  // Partially open/closed
    }

    DEBUG_PRINTF("CoverCalibrator initialized. Closed: %.1f°, Open: %.1f°, Current: %.1f°\n",
                 coverClosedAngle, coverOpenAngle, currentAngle);
    DEBUG_PRINTF("Cover state on startup: %d (%s)\n", coverState,
                 coverState == COVER_CLOSED ? "CLOSED" :
                 coverState == COVER_OPEN ? "OPEN" : "UNKNOWN");
}

// ============================================================================
// UPDATE - Call in main loop
// ============================================================================

void CoverCalibrator::update() {
    if (isMoving) {
        updateScurveMotion();

        // Check for timeout
        if (millis() - motionTimeoutStart > COVER_MOVE_TIMEOUT_MS) {
            DEBUG_PRINTLN("ERROR: Cover motion timeout!");
            isMoving = false;
            coverState = COVER_ERROR;
            motionTimedOut = true;
        }
    }
}

// ============================================================================
// CONNECTION MANAGEMENT
// ============================================================================

void CoverCalibrator::setConnected(bool state) {
    connected = state;
    if (connected) {
        DEBUG_PRINTLN("Device connected");
        // Update cover state based on current position
        if (fabs(currentAngle - coverClosedAngle) < 1.0f) {
            coverState = COVER_CLOSED;
        } else if (fabs(currentAngle - coverOpenAngle) < 1.0f) {
            coverState = COVER_OPEN;
        } else {
            coverState = COVER_UNKNOWN;
        }
    } else {
        DEBUG_PRINTLN("Device disconnected");
    }
}

// ============================================================================
// COVER CONTROL METHODS
// ============================================================================

void CoverCalibrator::openCover() {
    if (!connected) {
        DEBUG_PRINTLN("ERROR: Cannot open cover - device not connected");
        return;
    }

    if (coverState == COVER_OPEN) {
        DEBUG_PRINTLN("Cover is already open");
        return;
    }

    DEBUG_PRINTF("Opening cover: %.1f° → %.1f°\n", currentAngle, coverOpenAngle);
    coverState = COVER_MOVING;
    startScurveMotion(currentAngle, coverOpenAngle, SCURVE_DURATION_MS, SCURVE_STEEPNESS);
}

void CoverCalibrator::closeCover() {
    if (!connected) {
        DEBUG_PRINTLN("ERROR: Cannot close cover - device not connected");
        return;
    }

    if (coverState == COVER_CLOSED) {
        DEBUG_PRINTLN("Cover is already closed");
        return;
    }

    DEBUG_PRINTF("Closing cover: %.1f° → %.1f°\n", currentAngle, coverClosedAngle);
    coverState = COVER_MOVING;
    startScurveMotion(currentAngle, coverClosedAngle, SCURVE_DURATION_MS, SCURVE_STEEPNESS);
}

void CoverCalibrator::haltCover() {
    if (isMoving) {
        DEBUG_PRINTLN("Halting cover motion");
        isMoving = false;
        targetAngle = currentAngle;

        // Save halted position
        savePosition();

        // Determine new cover state based on position
        if (fabs(currentAngle - coverClosedAngle) < 1.0f) {
            coverState = COVER_CLOSED;
        } else if (fabs(currentAngle - coverOpenAngle) < 1.0f) {
            coverState = COVER_OPEN;
        } else {
            coverState = COVER_UNKNOWN;
        }
    }
}

void CoverCalibrator::setCoverAngles(float closedAngle, float openAngle) {
    // Validate angles
    if (closedAngle < 0.0f || closedAngle > SERVO_MAX_DEG ||
        openAngle < 0.0f || openAngle > SERVO_MAX_DEG) {
        DEBUG_PRINTLN("ERROR: Invalid cover angles");
        return;
    }

    coverClosedAngle = closedAngle;
    coverOpenAngle = openAngle;
    saveConfiguration();  // Save to preferences
    DEBUG_PRINTF("Cover angles updated and saved. Closed: %.1f°, Open: %.1f°\n",
                 closedAngle, openAngle);
}

// ============================================================================
// PERSISTENT STORAGE METHODS
// ============================================================================

void CoverCalibrator::loadConfiguration() {
    preferences.begin("cover", true);  // Read-only

    // Load cover angles (use defaults if not found)
    coverClosedAngle = preferences.getFloat("closedAngle", COVER_CLOSED_ANGLE);
    coverOpenAngle = preferences.getFloat("openAngle", COVER_OPEN_ANGLE);

    // Load last known position (use closed angle as default)
    currentAngle = preferences.getFloat("position", coverClosedAngle);
    targetAngle = currentAngle;

    preferences.end();

    DEBUG_PRINTLN("Configuration loaded from preferences:");
    DEBUG_PRINTF("  Closed angle: %.1f°\n", coverClosedAngle);
    DEBUG_PRINTF("  Open angle: %.1f°\n", coverOpenAngle);
    DEBUG_PRINTF("  Last position: %.1f°\n", currentAngle);
}

void CoverCalibrator::saveConfiguration() {
    preferences.begin("cover", false);  // Read-write

    preferences.putFloat("closedAngle", coverClosedAngle);
    preferences.putFloat("openAngle", coverOpenAngle);

    preferences.end();

    DEBUG_PRINTLN("Configuration saved to preferences");
}

void CoverCalibrator::savePosition() {
    preferences.begin("cover", false);  // Read-write
    preferences.putFloat("position", currentAngle);
    preferences.end();

    // Only log at debug level 2 to avoid spam
    DEBUG_PRINTF(2, "Position saved: %.1f°\n", currentAngle);
}

// ============================================================================
// CALIBRATOR CONTROL METHODS (Flat Panel - Not Yet Implemented)
// ============================================================================

void CoverCalibrator::calibratorOn(int brightness) {
    DEBUG_PRINTLN("WARNING: Calibrator (flat panel) not yet implemented");
    // Future implementation will control LED panel here
    currentBrightness = constrain(brightness, 0, MAX_BRIGHTNESS);
}

void CoverCalibrator::calibratorOff() {
    DEBUG_PRINTLN("WARNING: Calibrator (flat panel) not yet implemented");
    // Future implementation will turn off LED panel here
    currentBrightness = 0;
}

// ============================================================================
// SERVO MOTION FUNCTIONS (Adapted from sample code)
// ============================================================================

float CoverCalibrator::mapf(float x, float in_min, float in_max, float out_min, float out_max) {
    return out_min + (x - in_min) * (out_max - out_min) / (in_max - in_min);
}

int CoverCalibrator::degreesToMicroseconds(float angle) {
    return (int)mapf(angle, 0.0f, (float)SERVO_MAX_DEG,
                     (float)SERVO_MIN_US, (float)SERVO_MAX_US);
}

void CoverCalibrator::setServoAngle(float angle) {
    angle = constrain(angle, 0.0f, (float)SERVO_MAX_DEG);
    servo.writeMicroseconds(degreesToMicroseconds(angle));
    currentAngle = angle;
}

// Standard sigmoid function
float CoverCalibrator::sigmoid(float t, float steepness) {
    return 1.0f / (1.0f + expf(-steepness * (t - 0.5f)));
}

// Normalized sigmoid (maps exactly 0→0 and 1→1)
float CoverCalibrator::normalizedSigmoid(float t, float steepness) {
    float s0 = sigmoid(0.0f, steepness);
    float s1 = sigmoid(1.0f, steepness);
    return (sigmoid(t, steepness) - s0) / (s1 - s0);
}

// Start S-curve motion
void CoverCalibrator::startScurveMotion(float startAngle, float endAngle,
                                        unsigned long durationMs, float steepness) {
    motionStartAngle = startAngle;
    motionEndAngle = endAngle;
    motionStartTime = millis();
    motionDuration = durationMs;
    motionSteepness = steepness;
    motionTimeoutStart = millis();
    motionTimedOut = false;
    isMoving = true;
    targetAngle = endAngle;

    DEBUG_PRINTF("S-curve motion started: %.1f° → %.1f° over %lu ms\n",
                 startAngle, endAngle, durationMs);
}

// Update S-curve motion (called from update())
void CoverCalibrator::updateScurveMotion() {
    unsigned long elapsed = millis() - motionStartTime;

    if (elapsed >= motionDuration) {
        // Motion complete
        setServoAngle(motionEndAngle);
        isMoving = false;

        // Save final position to preferences
        savePosition();

        // Update cover state
        if (fabs(motionEndAngle - coverClosedAngle) < 1.0f) {
            coverState = COVER_CLOSED;
            DEBUG_PRINTLN("Cover closed");
        } else if (fabs(motionEndAngle - coverOpenAngle) < 1.0f) {
            coverState = COVER_OPEN;
            DEBUG_PRINTLN("Cover open");
        } else {
            coverState = COVER_UNKNOWN;
        }
        return;
    }

    // Calculate current position using S-curve
    float t = (float)elapsed / (float)motionDuration;
    float s = normalizedSigmoid(t, motionSteepness);
    float distance = motionEndAngle - motionStartAngle;
    float angle = motionStartAngle + (distance * s);

    setServoAngle(angle);
}
