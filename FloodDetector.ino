
#include <ESP32Servo.h>

// Water sensors
#define WATER_1 32
#define WATER_2 33
#define WATER_3 34

// Ultrasonic sensor
#define TRIG_PIN 26
#define ECHO_PIN 27

// Servo motor
#define SERVO_PIN 25

// Configuration
const int WATER_THRESHOLD = 1500;
const bool WET_WHEN_ABOVE = true;

const float SENSOR_HEIGHT_MM = 200.0;
const float ACTIVATE_DEPTH = 55.0;
const float RESET_DEPTH = 45.0;

Servo floodServo;

bool servoActivated = false;

bool isWet(int value) {
  return WET_WHEN_ABOVE
    ? value > WATER_THRESHOLD
    : value < WATER_THRESHOLD;
}

float getWaterDepth() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration =
    pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) return -1;

  float distanceMM = duration * 0.343 / 2.0;

  return SENSOR_HEIGHT_MM - distanceMM;
}

void setup() {
  Serial.begin(115200);

  pinMode(WATER_1, INPUT);
  pinMode(WATER_2, INPUT);
  pinMode(WATER_3, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  floodServo.setPeriodHertz(50);
  floodServo.attach(SERVO_PIN, 500, 2400);
  floodServo.write(0);

  Serial.println("FLOOD DETECTOR V1");
  Serial.println("System initialized.");
}

void loop() {
  int sensor1 = analogRead(WATER_1);
  int sensor2 = analogRead(WATER_2);
  int sensor3 = analogRead(WATER_3);

  // First two sensors detect water
  bool waterDetected =
    isWet(sensor1) && isWet(sensor2);

  // Third sensor monitors water presence
  bool thirdSensorWet = isWet(sensor3);

  float depth = getWaterDepth();

  Serial.println("\n--- FLOOD STATUS ---");

  Serial.printf("Water sensor 1: %d\n", sensor1);
  Serial.printf("Water sensor 2: %d\n", sensor2);
  Serial.printf("Water sensor 3: %d\n", sensor3);

  Serial.printf("Water detected: %s\n",
    waterDetected ? "YES" : "NO");

  Serial.printf("Third sensor wet: %s\n",
    thirdSensorWet ? "YES" : "NO");

  if (depth < 0) {
    Serial.println("Ultrasonic: NO ECHO");
  } else {
    Serial.printf("Water depth: %.1f mm\n", depth);
  }

  // Activate only when both detection sensors
  // and the ultrasonic measurement are valid
  if (!waterDetected || depth < 0) {
    servoActivated = false;
    floodServo.write(0);
  }
  else if (!servoActivated && depth >= ACTIVATE_DEPTH) {
    servoActivated = true;
    floodServo.write(90);
    Serial.println("FLOOD THRESHOLD EXCEEDED!");
  }
  else if (servoActivated && depth <= RESET_DEPTH) {
    servoActivated = false;
    floodServo.write(0);
    Serial.println("Water level returned to normal.");
  }

  Serial.printf("Servo: %d degrees\n",
    servoActivated ? 90 : 0);

  delay(300);
}

