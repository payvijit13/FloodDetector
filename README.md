🌊 ESP32 Flood Detector

An ESP32-based flood detection prototype that detects water presence, measures water depth and activates a servo motor when the water level exceeds a configured threshold.

Hardware

- 1× ESP32 development board
- 3× Water sensors (VCC, GND, Signal)
- 1× HC-SR04 ultrasonic sensor
- 1× Servo motor
- Jumper wires
- External 5V power supply for servo

How It Works

1. Water sensors 1 and 2 detect water presence.
2. The HC-SR04 measures the distance to the water surface.
3. The ESP32 calculates the water depth.
4. When water depth reaches 55 mm, the servo rotates to 90°.
5. When water depth falls to 45 mm or below, the servo returns to 0°.
6. Water sensor 3 provides an additional water-presence reading.

Water sensor 3 S GPIO 34
HC-SR04 TRIG   GPIO 26
HC-SR04 ECHO   GPIO 27*
Servo Signal   GPIO 25

Power and ground
All 3 water sensors: VCC → 3.3V, GND → GND.
HC-SR04: VCC → 5V, GND → GND.
Servo: external regulated 5V supply; connect its GND to ESP32 GND.

Important: HC-SR04 ECHO outputs 5V. Use a voltage divider before connecting it to GPIO 27.

Thresholds

Parameter| Value
Activation depth| 55 mm
Reset depth| 45 mm
Servo activation angle| 90°
Servo resting angle| 0°
Default sensor mounting height| 200 mm

Software Requirements

- Arduino IDE
- ESP32 Arduino Core
- ESP32Servo library

Installation

1. Install the ESP32 board package in Arduino IDE.
2. Install the ESP32Servo library.
3. Open "FloodDetector.ino".
4. Select your ESP32 board.
5. Select the correct serial port.
6. Upload the sketch.
7. Open Serial Monitor at 115200 baud.

Power & Safety

- Do not power the servo directly from the ESP32 3.3V output.
- Use a suitable regulated external 5V supply.
- Connect external supply GND to ESP32 GND.
- Use a voltage divider on the HC-SR04 Echo output before connecting it to GPIO 27.
- Keep electronics protected from water.

Project Status

Prototype / Development

- Three water sensors configured.
- Ultrasonic water-depth measurement implemented.
- One servo motor configured.
- 55 mm activation threshold implemented.
- 45 mm reset threshold implemented.

Sensor 3 is currently monitored but does not participate in the servo activation condition.

Sensor thresholds and ultrasonic mounting height require calibration.

This prototype is not a certified flood-warning or life-safety system.

---

RAGEBLOCKS © 2026
