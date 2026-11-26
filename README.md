🌊 ESP32 Flood Detector

An ESP32-based flood detection prototype using three water sensors, an ultrasonic distance sensor, and one servo motor.

🔧 Hardware Required

- 1× ESP32 development board
- 3× 3-pin water sensors (VCC, GND, S)
- 1× HC-SR04 ultrasonic sensor
- 1× Servo motor
- Jumper wires
- Breadboard (optional)
- External regulated 5V supply for the servo
- Resistors for an HC-SR04 ECHO voltage divider

📌 Pin Connections

1. Water Sensors

Each water sensor has three pins: VCC, GND, and S (Signal).

Water Sensor| VCC| GND| Signal
Sensor 1| ESP32 3.3V| ESP32 GND| GPIO 32
Sensor 2| ESP32 3.3V| ESP32 GND| GPIO 33
Sensor 3| ESP32 3.3V| ESP32 GND| GPIO 34

Note: GPIO 34 is input-only, which is suitable for reading the sensor.

2. HC-SR04 Ultrasonic Sensor

HC-SR04 Pin| Connection
VCC| ESP32 5V / VIN
GND| ESP32 GND
TRIG| ESP32 GPIO 26
ECHO| ESP32 GPIO 27 through a voltage divider

⚠️ Important — ECHO voltage protection

The HC-SR04 ECHO pin can output 5V, while ESP32 GPIO pins are not 5V tolerant.

Use a voltage divider between ECHO and GPIO 27.

Example:

- HC-SR04 ECHO → 1kΩ resistor → GPIO 27
- GPIO 27 → 2kΩ resistor → GND

This reduces the 5V signal to approximately 3.3V.

3. Servo Motor

Servo Wire| Connection
Signal (usually orange/yellow)| ESP32 GPIO 25
VCC (usually red)| External regulated 5V
GND (usually brown/black)| External supply GND

Important:

- Do not power the servo from the ESP32 3.3V pin.
- Connect the external supply GND to ESP32 GND.
- Use a suitable regulated supply for the servo.

🔌 Complete GPIO Summary

Component| Function| ESP32 GPIO
Water Sensor 1| Analog input| 32
Water Sensor 2| Analog input| 33
Water Sensor 3| Analog input| 34
HC-SR04| TRIG| 26
HC-SR04| ECHO| 27 (through voltage divider)
Servo Motor| Signal| 25

⚙️ How It Works

1. Water sensors 1 and 2 detect water presence.
2. The HC-SR04 measures the distance between the sensor and the water surface.
3. The ESP32 calculates the water depth.
4. When the water depth reaches 55 mm, the servo rotates to 90°.
5. When the water depth falls to 45 mm or below, the servo returns to 0°.
6. Water sensor 3 provides an additional water-presence reading and is displayed in the Serial Monitor. It does not currently control servo activation.

📏 Detection Configuration

Setting| Value
Servo activation depth| 55 mm
Servo reset depth| 45 mm
Servo activated angle| 90°
Servo default angle| 0°
Default sensor mounting height| 200 mm
Serial baud rate| 115200

Sensor mounting height: "SENSOR_HEIGHT_MM" must be adjusted to the actual distance from the ultrasonic sensor face to the container bottom.

Water depth is calculated as:

"Water Depth = Sensor Mounting Height - Measured Distance"

💻 Software Requirements

- Arduino IDE
- ESP32 board package
- ESP32Servo library

🚀 Upload Instructions

1. Open "FloodDetector.ino" in Arduino IDE.
2. Install the ESP32Servo library.
3. Select your ESP32 board.
4. Select the correct serial port.
5. Upload the sketch.
6. Open Serial Monitor at 115200 baud.

🧪 Calibration

- Check the raw water sensor readings in Serial Monitor.
- Adjust "WATER_THRESHOLD" according to the actual sensor readings.
- If the sensor readings decrease when wet, change "WET_WHEN_ABOVE" to "false".
- Adjust "SENSOR_HEIGHT_MM" to match the physical installation.
- Test the servo's 0° and 90° positions before connecting it to any mechanism.

⚠️ Safety Notes

- Never connect the HC-SR04 ECHO output directly to an ESP32 GPIO.
- Keep the ESP32 and all exposed electronics protected from water.
- Use a suitable regulated external supply for the servo.
- Ensure all grounds are connected together.
- This is a prototype, not a certified flood-warning or life-safety system.

📂 Project Structure

RageBOT/
├── FloodDetector.ino
└── README.md

📌 Project Status

Flood Detector V1 — Prototype

- ESP32 controller
- Three water sensors
- One HC-SR04 ultrasonic sensor
- One servo motor
- Automatic servo activation based on measured water depth

Water sensor calibration and real-world hardware testing are still required.

---

RAGEBLOCKS © 2026