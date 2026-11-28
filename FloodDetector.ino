
#include <ESP32Servo.h>

// Water sensors
#define WATER_1 32
#define WATER_2 33
#define WATER_3 34

// Servo motor
#define SERVO_PIN 25

// Configuration
const int WATER_THRESHOLD = 2000;

Servo floodServo;

bool servoActivated = false;

bool isWet(int value) {
  return value > WATER_THRESHOLD;
}

void setup() {
  Serial.begin(115200);

  pinMode(WATER_1, INPUT);
  pinMode(WATER_2, INPUT);
  pinMode(WATER_3, INPUT);

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

  // Both sensors must exceed 2000
  bool waterDetected =
    isWet(sensor1) && isWet(sensor2);

  bool thirdSensorWet = isWet(sensor3);

  Serial.println("\n--- FLOOD STATUS ---");

  Serial.printf("Water sensor 1: %d\n", sensor1);
  Serial.printf("Water sensor 2: %d\n", sensor2);
  Serial.printf("Water sensor 3: %d\n", sensor3);

  Serial.printf("Water detected: %s\n",
    waterDetected ? "YES" : "NO");

  Serial.printf("Third sensor wet: %s\n",
    thirdSensorWet ? "YES" : "NO");

  if (waterDetected && !servoActivated) {
    servoActivated = true;
    floodServo.write(90);

    Serial.println("FLOOD DETECTED!");
  }
  else if (!waterDetected && servoActivated) {
    servoActivated = false;
    floodServo.write(0);

    Serial.println("Water level returned to normal.");
  }

  Serial.printf("Servo: %d degrees\n",
    servoActivated ? 90 : 0);

  delay(300);
}
