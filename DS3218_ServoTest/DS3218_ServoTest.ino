/*
 * DS3218 Servo Test Sketch — v2 with Soft Motion
 * ================================================
 * Board:  ESP32-S3 (Waveshare ESP32-S3-DEV-KIT-NxR8)
 * PCB:    ESP32 ROR Controller v3.1 (JLCPCB-003)
 * Pin:    IO10 (via H4 header)
 * Servo:  Miuzei DS3218 20KG, 270° variant
 *
 * DS3218 270° Specifications:
 *   - Pulse width:  500 µs (0°) to 2500 µs (270°)
 *   - Frequency:    50 Hz (20 ms period)
 *   - Neutral:      1500 µs (135°)
 *   - Voltage:      4.8–6.8V (powered from 5V rail via K7805-2000R3)
 *   - Signal:       3.3V logic accepted
 *
 * Motion Modes:
 *   - Direct:    Immediate positioning (servo moves at full speed)
 *   - Ramp:      Linear interpolation at constant angular velocity
 *   - S-curve:   Smooth acceleration/deceleration using sigmoid function
 *
 * Usage:
 *   Open Serial Monitor at 115200 baud. Type 'help' for commands.
 */

#include <ESP32Servo.h>
#include <math.h>

// ── Pin Configuration ──────────────────────────────────────────────
#define SERVO_PIN       10        // IO10 on H4 header

// ── DS3218 270° Servo Parameters ───────────────────────────────────
#define SERVO_MIN_US    500       // Pulse width at 0°
#define SERVO_MAX_US    2500      // Pulse width at 270°
#define SERVO_MAX_DEG   270       // Total travel
#define SERVO_FREQ_HZ   50       // Standard servo frequency
#define STEP_DEG         30       // Increment size for direct positioning

// ── Motion Parameters ──────────────────────────────────────────────
#define MOTION_INTERVAL_MS  20    // Update interval (ms) — 50 Hz update rate
                                  // Matches servo PWM frame period

#define RAMP_SPEED_DEG_S    45.0f // Linear ramp speed (degrees per second)
                                  // 45°/s → full 270° travel in 6 seconds

#define SCURVE_DURATION_MS  4000  // Total S-curve transit time (ms)
                                  // Applies regardless of travel distance,
                                  // so short moves are gentler than long ones.
                                  // Adjust per your mechanical requirements.

#define SCURVE_STEEPNESS    6.0f  // Sigmoid steepness factor (higher = sharper
                                  // transition at midpoint, more time at
                                  // min/max speed). 6.0 is a good default.
                                  // Range: 4.0 (gentle) to 10.0 (aggressive)

Servo myServo;
float currentAngle = 0.0f;       // Track current position as float for smooth motion

// ════════════════════════════════════════════════════════════════════
//  CORE MOTION FUNCTIONS
//  These accept arbitrary start/end angles in either direction.
//  Reuse these directly when integrating into your final firmware.
// ════════════════════════════════════════════════════════════════════

// ── Float version of map() for precision ───────────────────────────
float mapf(float x, float inMin, float inMax, float outMin, float outMax) {
  return outMin + (x - inMin) * (outMax - outMin) / (inMax - inMin);
}

// ── Convert degrees to microseconds ────────────────────────────────
int degreesToMicroseconds(float degrees) {
  return (int)mapf(degrees, 0.0f, (float)SERVO_MAX_DEG,
                   (float)SERVO_MIN_US, (float)SERVO_MAX_US);
}

// ── Set servo position (float degrees) ─────────────────────────────
void setServoAngle(float angle) {
  angle = constrain(angle, 0.0f, (float)SERVO_MAX_DEG);
  myServo.writeMicroseconds(degreesToMicroseconds(angle));
  currentAngle = angle;
}

// ── Sigmoid function for S-curve ───────────────────────────────────
//    Returns 0.0–1.0 for input t = 0.0–1.0
float sigmoid(float t, float steepness) {
  return 1.0f / (1.0f + expf(-steepness * (t - 0.5f)));
}

// ── Normalized sigmoid (maps exactly 0→0 and 1→1) ─────────────────
float normalizedSigmoid(float t, float steepness) {
  float s0 = sigmoid(0.0f, steepness);
  float s1 = sigmoid(1.0f, steepness);
  return (sigmoid(t, steepness) - s0) / (s1 - s0);
}

// ────────────────────────────────────────────────────────────────────
//  moveRamp() — Linear ramp between any two angles
//
//  Moves at a constant angular velocity. Duration is proportional
//  to the distance traveled. Works in either direction.
//
//  Parameters:
//    startAngle  — starting position in degrees
//    endAngle    — target position in degrees
//    speedDegS   — angular velocity in degrees/second
// ────────────────────────────────────────────────────────────────────
void moveRamp(float startAngle, float endAngle, float speedDegS) {
  float distance = endAngle - startAngle;
  float absDistance = fabs(distance);

  if (absDistance < 0.1f) return;

  float durationMs = (absDistance / speedDegS) * 1000.0f;
  unsigned long totalSteps = (unsigned long)(durationMs / MOTION_INTERVAL_MS);
  if (totalSteps < 1) totalSteps = 1;

  Serial.printf("  Ramp: %.0f° → %.0f°  |  Speed: %.0f°/s  |  Duration: %.0f ms  |  Steps: %lu\n",
                startAngle, endAngle, speedDegS, durationMs, totalSteps);

  setServoAngle(startAngle);
  delay(MOTION_INTERVAL_MS);

  unsigned long startTime = millis();
  float prevAngle = startAngle;

  for (unsigned long step = 1; step <= totalSteps; step++) {
    float t = (float)step / (float)totalSteps;
    float angle = startAngle + (distance * t);
    setServoAngle(angle);

    // Report at ~30° boundaries
    int prevBucket = (int)(prevAngle / 30.0f);
    int currBucket = (int)(angle / 30.0f);
    if (prevBucket != currBucket || step == totalSteps) {
      Serial.printf("    %6.1f°  |  %4d µs  |  t=%.2f\n",
                    angle, degreesToMicroseconds(angle), t);
    }
    prevAngle = angle;

    delay(MOTION_INTERVAL_MS);
  }

  setServoAngle(endAngle);
  Serial.printf("  Complete. Elapsed: %lu ms\n", millis() - startTime);
}

// ────────────────────────────────────────────────────────────────────
//  moveScurve() — S-curve (sigmoid) between any two angles
//
//  Uses a normalized sigmoid for smooth acceleration/deceleration.
//  Fixed duration regardless of distance — shorter moves are gentler.
//  Works in either direction.
//
//  Parameters:
//    startAngle  — starting position in degrees
//    endAngle    — target position in degrees
//    durationMs  — total move time in milliseconds
//    steepness   — sigmoid steepness (4.0=gentle, 10.0=aggressive)
// ────────────────────────────────────────────────────────────────────
void moveScurve(float startAngle, float endAngle, unsigned long durationMs, float steepness) {
  float distance = endAngle - startAngle;

  if (fabs(distance) < 0.1f) return;

  unsigned long totalSteps = durationMs / MOTION_INTERVAL_MS;
  if (totalSteps < 1) totalSteps = 1;

  Serial.printf("  S-curve: %.0f° → %.0f°  |  Duration: %lu ms  |  Steepness: %.1f  |  Steps: %lu\n",
                startAngle, endAngle, durationMs, steepness, totalSteps);

  setServoAngle(startAngle);
  delay(MOTION_INTERVAL_MS);

  unsigned long startTime = millis();
  float prevAngle = startAngle;

  for (unsigned long step = 1; step <= totalSteps; step++) {
    float t = (float)step / (float)totalSteps;
    float s = normalizedSigmoid(t, steepness);
    float angle = startAngle + (distance * s);
    setServoAngle(angle);

    // Report at ~30° boundaries
    int prevBucket = (int)(prevAngle / 30.0f);
    int currBucket = (int)(angle / 30.0f);
    if (prevBucket != currBucket || step == totalSteps) {
      float instantSpeed = fabs(angle - prevAngle) / ((float)MOTION_INTERVAL_MS / 1000.0f);
      Serial.printf("    %6.1f°  |  %4d µs  |  t=%.2f  s=%.3f  |  ~%.0f°/s\n",
                    angle, degreesToMicroseconds(angle), t, s, instantSpeed);
    }
    prevAngle = angle;

    delay(MOTION_INTERVAL_MS);
  }

  setServoAngle(endAngle);
  Serial.printf("  Complete. Elapsed: %lu ms\n", millis() - startTime);
}

// ════════════════════════════════════════════════════════════════════
//  DEMO ROUTINES
// ════════════════════════════════════════════════════════════════════

// Helper: return to 0° using ramp before a demo
void returnToZero() {
  if (fabs(currentAngle) > 0.5f) {
    Serial.println("  Returning to 0°...");
    moveRamp(currentAngle, 0.0f, RAMP_SPEED_DEG_S * 1.5f);  // Slightly faster return
    delay(500);
  }
}

void demoRamp() {
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║       LINEAR RAMP DEMO               ║");
  Serial.println("╚══════════════════════════════════════╝\n");

  Serial.println("── Forward: 0° → 270° ──");
  moveRamp(0.0f, 270.0f, RAMP_SPEED_DEG_S);

  Serial.println("\n  Pausing 1.5s at 270°...\n");
  delay(1500);

  Serial.println("── Reverse: 270° → 0° ──");
  moveRamp(270.0f, 0.0f, RAMP_SPEED_DEG_S);

  Serial.println("\n  Ramp demo complete.\n");
}

void demoScurve() {
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║       S-CURVE DEMO                   ║");
  Serial.println("╚══════════════════════════════════════╝\n");

  Serial.println("── Forward: 0° → 270° ──");
  moveScurve(0.0f, 270.0f, SCURVE_DURATION_MS, SCURVE_STEEPNESS);

  Serial.println("\n  Pausing 1.5s at 270°...\n");
  delay(1500);

  Serial.println("── Reverse: 270° → 0° ──");
  moveScurve(270.0f, 0.0f, SCURVE_DURATION_MS, SCURVE_STEEPNESS);

  Serial.println("\n  S-curve demo complete.\n");
}

void demoBoth() {
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   COMPARISON: RAMP vs S-CURVE        ║");
  Serial.println("╚══════════════════════════════════════╝\n");

  Serial.println("─── Part 1: Linear Ramp ───");
  moveRamp(0.0f, 270.0f, RAMP_SPEED_DEG_S);
  delay(1500);
  moveRamp(270.0f, 0.0f, RAMP_SPEED_DEG_S);

  Serial.println("\n  Pausing 2s before S-curve...\n");
  delay(2000);

  Serial.println("─── Part 2: S-Curve ───");
  moveScurve(0.0f, 270.0f, SCURVE_DURATION_MS, SCURVE_STEEPNESS);
  delay(1500);
  moveScurve(270.0f, 0.0f, SCURVE_DURATION_MS, SCURVE_STEEPNESS);

  Serial.println("\n  Comparison demo complete.\n");
}

// ════════════════════════════════════════════════════════════════════
//  DIRECT POSITIONING (from v1)
// ════════════════════════════════════════════════════════════════════

void moveDirectToAngle(int targetAngle) {
  if (targetAngle < 0 || targetAngle > SERVO_MAX_DEG) {
    Serial.printf("  ERROR: Angle %d° is out of range (0–%d)\n", targetAngle, SERVO_MAX_DEG);
    return;
  }
  int pulseUs = degreesToMicroseconds((float)targetAngle);
  Serial.printf("  Direct: %.0f° → %d°  |  Pulse: %d µs\n",
                currentAngle, targetAngle, pulseUs);
  setServoAngle((float)targetAngle);
}

void sweepDirect() {
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║       DIRECT SWEEP: 0° → 270° → 0°  ║");
  Serial.println("╚══════════════════════════════════════╝\n");

  Serial.println("── Forward ──");
  for (int angle = 0; angle <= SERVO_MAX_DEG; angle += STEP_DEG) {
    moveDirectToAngle(angle);
    delay(800);
  }
  Serial.println("\n  Pausing at 270°...\n");
  delay(1000);

  Serial.println("── Reverse ──");
  for (int angle = SERVO_MAX_DEG; angle >= 0; angle -= STEP_DEG) {
    moveDirectToAngle(angle);
    delay(800);
  }
  Serial.println("\n  Direct sweep complete.\n");
}

// ════════════════════════════════════════════════════════════════════
//  MENU & INPUT HANDLING
// ════════════════════════════════════════════════════════════════════

void printMenu() {
  Serial.println("┌──────────────────────────────────────────┐");
  Serial.println("│   DS3218 Servo Test v2 — Commands         │");
  Serial.println("├──────────────────────────────────────────┤");
  Serial.println("│  Direct Positioning:                      │");
  Serial.println("│    0–270     Move to angle (30° steps)    │");
  Serial.println("│    sweep     Direct sweep 0→270→0         │");
  Serial.println("│    center    Move to 135° (neutral)       │");
  Serial.println("│                                           │");
  Serial.println("│  Soft Motion Demos:                       │");
  Serial.println("│    ramp      Linear ramp 0→270→0          │");
  Serial.println("│    scurve    S-curve 0→270→0              │");
  Serial.println("│    compare   Run ramp then S-curve        │");
  Serial.println("│                                           │");
  Serial.println("│  Info:                                    │");
  Serial.println("│    pos       Current position & pulse     │");
  Serial.println("│    help      Show this menu               │");
  Serial.println("└──────────────────────────────────────────┘");
  Serial.println();
}

bool isValidStep(int angle) {
  return (angle >= 0 && angle <= SERVO_MAX_DEG && angle % STEP_DEG == 0);
}

// ════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("\n");
  Serial.println("══════════════════════════════════════════");
  Serial.println("  DS3218 Servo Test v2 — ESP32 ROR v3.1");
  Serial.printf("  Pin: IO%d  |  Range: 0–%d°\n", SERVO_PIN, SERVO_MAX_DEG);
  Serial.printf("  Pulse: %d–%d µs  |  Freq: %d Hz\n", SERVO_MIN_US, SERVO_MAX_US, SERVO_FREQ_HZ);
  Serial.println("  ──────────────────────────────────────");
  Serial.printf("  Ramp speed:      %.0f°/s\n", RAMP_SPEED_DEG_S);
  Serial.printf("  S-curve time:    %d ms\n", SCURVE_DURATION_MS);
  Serial.printf("  S-curve steep:   %.1f\n", SCURVE_STEEPNESS);
  Serial.printf("  Update rate:     %d ms (%d Hz)\n", MOTION_INTERVAL_MS, 1000 / MOTION_INTERVAL_MS);
  Serial.println("══════════════════════════════════════════\n");

  // Attach servo
  ESP32PWM::allocateTimer(0);
  myServo.setPeriodHertz(SERVO_FREQ_HZ);
  myServo.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);

  // Initialize to 0°
  Serial.println("  Initializing servo to 0°...\n");
  setServoAngle(0.0f);
  delay(1000);

  printMenu();
  Serial.print("> ");
}

// ════════════════════════════════════════════════════════════════════
void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toLowerCase();

    if (input.length() == 0) {
      Serial.print("> ");
      return;
    }

    Serial.printf("  Input: \"%s\"\n", input.c_str());

    if (input == "sweep") {
      sweepDirect();
    }
    else if (input == "ramp") {
      returnToZero();
      demoRamp();
    }
    else if (input == "scurve") {
      returnToZero();
      demoScurve();
    }
    else if (input == "compare") {
      returnToZero();
      demoBoth();
    }
    else if (input == "center") {
      moveDirectToAngle(SERVO_MAX_DEG / 2);
      Serial.println();
    }
    else if (input == "pos") {
      Serial.printf("  Position: %.1f°  |  Pulse: %d µs\n\n",
                    currentAngle, degreesToMicroseconds(currentAngle));
    }
    else if (input == "help") {
      Serial.println();
      printMenu();
    }
    else {
      int angle = input.toInt();
      if (input != "0" && angle == 0) {
        Serial.printf("  Unknown command: \"%s\" — type 'help'\n\n", input.c_str());
      }
      else if (!isValidStep(angle)) {
        Serial.printf("  Invalid angle: %d° — use 0–%d in %d° steps\n",
                      angle, SERVO_MAX_DEG, STEP_DEG);
        Serial.print("  Valid: ");
        for (int a = 0; a <= SERVO_MAX_DEG; a += STEP_DEG) {
          Serial.printf("%d", a);
          if (a < SERVO_MAX_DEG) Serial.print(", ");
        }
        Serial.println("\n");
      }
      else {
        moveDirectToAngle(angle);
        Serial.println();
      }
    }

    Serial.print("> ");
  }
}
