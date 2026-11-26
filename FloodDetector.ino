#include <ESP32Servo.h>

// ---------- WATER SENSOR PINS ----------
const int WATER_SENSOR_PINS[4] = {
  32,  // Water sensor 1
  33,  // Water sensor 2
  34,  // Water sensor 3 (monitoring only)
  35   // Water sensor 4 (monitoring only)
};

// ---------- ULTRASONIC SENSOR PINS ----------
const int TRIG_PIN = 26;
const int ECHO_PIN = 27;

// ---------- SERVO PINS ----------
const int SERVO_PINS[4] = {
  13,  // Servo 1: flood gate/actuator
  14,  // Servo 2: reserved
  25,  // Servo 3: reserved
  23   // Servo 4: reserved
};

Servo servos[4];

// ---------- CALIBRATION ----------
const int WATER_THRESHOLD = 1500; // Adjust after checking Serial Monitor
const bool WET_WHEN_ABOVE = true; // Set false if wet readings are lower

const float SENSOR_HEIGHT_MM = 200.0; // Measure and change this!
const float ACTIVATE_DEPTH_MM = 55.0;
const float RESET_DEPTH_MM = 45.0;

// ---------- STATE ----------
bool servo1Activated = false;

bool isWet(int reading) {
  return WET_WHEN_ABOVE
           ? reading >= WATER_THRESHOLD
           : reading <= WATER_THRESHOLD;
}

float readWaterDepthMM() {
  // Send a 10 microsecond trigger pulse.
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Timeout after 30 ms to avoid waiting indefinitely.
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);

  if (duration == 0) {
    return -1.0; // No valid echo
  }

  // Speed of sound is approximately 0.343 mm per microsecond.
  float distanceMM = (duration * 0.343f) / 2.0f;

  if (distanceMM <= 0 || distanceMM > SENSOR_HEIGHT_MM + 100.0f) {
    return -1.0;
  }

  float depthMM = SENSOR_HEIGHT_MM - distanceMM;

  if (depthMM < 0) {
    depthMM = 0;
  }

  return depthMM;
}

void setServo1(bool activate) {
  if (activate) {
    servos[0].write(90);
    servo1Activated = true;
    Serial.println("ACTION: Servo 1 -> 90 degrees");
  } else {
    servos[0].write(0);
    servo1Activated = false;
    Serial.println("ACTION: Servo 1 -> 0 degrees");
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  for (int i = 0; i < 4; i++) {
    servos[i].setPeriodHertz(50);
    servos[i].attach(SERVO_PINS[i], 500, 2400);
    servos[i].write(0);
  }

  Serial.println();
  Serial.println("================================");
  Serial.println("       FLOOD DETECTOR V1");
  Serial.println("================================");
  Serial.println("Waiting for water...");
}

void loop() {
  int waterReadings[4];
  bool waterStates[4];

  for (int i = 0; i < 4; i++) {
    waterReadings[i] = analogRead(WATER_SENSOR_PINS[i]);
    waterStates[i] = isWet(waterReadings[i]);
  }

  // The first two sensors are the water-presence confirmation pair.
  bool waterDetected = waterStates[0] && waterStates[1];

  float depthMM = readWaterDepthMM();

  Serial.println("--------------------------------");

  for (int i = 0; i < 4; i++) {
    Serial.print("Water sensor ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(waterReadings[i]);
    Serial.print(" | ");
    Serial.println(waterStates[i] ? "WET" : "DRY");
  }

  Serial.print("Water presence confirmed: ");
  Serial.println(waterDetected ? "YES" : "NO");

  if (depthMM >= 0) {
    Serial.print("Measured water depth: ");
    Serial.print(depthMM, 1);
    Serial.println(" mm");
  } else {
    Serial.println("Ultrasonic reading: NO VALID ECHO");
  }

  // Fail-safe behavior: no confirmed water or no valid depth reading
  // returns Servo 1 to its resting position.
  if (!waterDetected || depthMM < 0) {
    if (servo1Activated) {
      setServo1(false);
    }
  } else if (!servo1Activated && depthMM >= ACTIVATE_DEPTH_MM) {
    setServo1(true);
    Serial.println("WARNING: WATER DEPTH THRESHOLD REACHED");
  } else if (servo1Activated && depthMM <= RESET_DEPTH_MM) {
    setServo1(false);
    Serial.println("Water level has dropped below reset threshold");
  }

  Serial.print("Servo 1 state: ");
  Serial.println(servo1Activated ? "90 degrees" : "0 degrees");
  Serial.println("Servos 2, 3 and 4 remain at 0 degrees.");

  delay(300);
}
