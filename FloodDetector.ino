
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <ArduinoOTA.h>
#include <ESP32Servo.h>

// =====================================
// WIFI CONFIGURATION
// =====================================

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Fallback Access Point
const char* AP_SSID = "FLOOD-DETECTOR";
const char* AP_PASSWORD = "flood1234";

// =====================================
// PIN CONFIGURATION
// =====================================

#define WATER_1 32
#define WATER_2 33
#define WATER_3 34

#define SERVO_PIN 25

// =====================================
// SYSTEM CONFIGURATION
// =====================================

const int WATER_THRESHOLD = 2000;

const int SERVO_OFF = 0;
const int SERVO_ON = 90;

const char* OTA_HOSTNAME = "flood-detector";

Servo floodServo;
WebServer server(80);

// =====================================
// GLOBAL VARIABLES
// =====================================

int sensor1 = 0;
int sensor2 = 0;
int sensor3 = 0;

bool waterDetected = false;
bool thirdSensorWet = false;
bool servoActivated = false;

bool apMode = false;
bool updateSuccess = false;

// =====================================
// WEBSITE
// =====================================

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">

<title>Flood Detector V1</title>

<style>
* {
  box-sizing: border-box;
}

body {
  margin: 0;
  padding: 20px;
  background: #101827;
  color: white;
  font-family: Arial, sans-serif;
}

.container {
  max-width: 850px;
  margin: auto;
}

h1 {
  color: #38bdf8;
}

.subtitle {
  color: #94a3b8;
}

.card {
  background: #1e293b;
  padding: 20px;
  margin: 15px 0;
  border-radius: 12px;
}

.grid {
  display: grid;
  grid-template-columns: repeat(auto-fit,minmax(180px,1fr));
  gap: 12px;
}

.value {
  font-size: 32px;
  font-weight: bold;
  margin-top: 12px;
}

.status {
  font-size: 23px;
  font-weight: bold;
}

button {
  background: #0284c7;
  color: white;
  border: none;
  padding: 12px 18px;
  border-radius: 8px;
  cursor: pointer;
  margin-top: 10px;
}

input {
  max-width: 100%;
  color: white;
  margin: 12px 0;
}

.green {
  color: #4ade80;
}

.red {
  color: #f87171;
}

footer {
  color: #64748b;
  text-align: center;
  margin-top: 30px;
}
</style>
</head>

<body>

<div class="container">

<h1>FLOOD DETECTOR</h1>
<p class="subtitle">SYSTEM CONTROL CENTER | VERSION 1.0</p>

<div class="card">
<h2>System Status</h2>
<div id="flood" class="status">Loading...</div>
<p>Servo: <span id="servo">--</span></p>
<p>Connection: <span id="connection">--</span></p>
</div>

<h2>Water Sensors</h2>

<div class="grid">

<div class="card">
<h3>Sensor 1</h3>
<div class="value" id="s1">--</div>
<p id="s1status">Waiting...</p>
</div>

<div class="card">
<h3>Sensor 2</h3>
<div class="value" id="s2">--</div>
<p id="s2status">Waiting...</p>
</div>

<div class="card">
<h3>Sensor 3</h3>
<div class="value" id="s3">--</div>
<p id="s3status">Waiting...</p>
</div>

</div>

<div class="card">
<h2>Flood Detection</h2>
<p>Activation threshold: 2000</p>
<p>Required: Sensor 1 AND Sensor 2</p>
<p>Servo activation: 90 degrees</p>
<p>Servo reset: 0 degrees</p>
</div>

<div class="card">
<h2>Wireless Firmware Update</h2>

<p>Upload a compiled ESP32 firmware (.bin) file.</p>

<form method="POST" action="/update"
enctype="multipart/form-data">

<input type="file" name="firmware"
accept=".bin" required>

<br>

<button type="submit">UPLOAD FIRMWARE</button>

</form>

<p id="otaInfo">
Keep the ESP32 powered during updates.
</p>
</div>

<div class="card">
<h2>Network Information</h2>
<p>IP Address: <span id="ip">--</span></p>
<p>Wi-Fi Mode: <span id="wifi">--</span></p>
<p>OTA Hostname: flood-detector</p>
</div>

<footer>
RAGEBLOCKS | FLOOD DETECTOR V1
</footer>

</div>

<script>

async function refreshStatus() {

try {

const response = await fetch('/status');
const data = await response.json();

document.getElementById('s1').textContent = data.sensor1;
document.getElementById('s2').textContent = data.sensor2;
document.getElementById('s3').textContent = data.sensor3;

document.getElementById('s1status').textContent =
data.sensor1 > 2000 ? 'WET' : 'NORMAL';

document.getElementById('s2status').textContent =
data.sensor2 > 2000 ? 'WET' : 'NORMAL';

document.getElementById('s3status').textContent =
data.sensor3 > 2000 ? 'WET' : 'NORMAL';

const flood = document.getElementById('flood');

flood.textContent = data.flood ? 'FLOOD DETECTED!' : 'NORMAL';

flood.className = 'status ' +
(data.flood ? 'red' : 'green');

document.getElementById('servo').textContent =
data.servo ? '90° ACTIVE' : '0° INACTIVE';

document.getElementById('connection').textContent =
'CONNECTED';

document.getElementById('ip').textContent = data.ip;

document.getElementById('wifi').textContent =
data.ap ? 'ACCESS POINT' : 'WIFI CLIENT';

} catch(error) {

document.getElementById('connection').textContent =
'DISCONNECTED';

}

}

setInterval(refreshStatus, 1000);
refreshStatus();

</script>

</body>
</html>
)rawliteral";

// =====================================
// WIFI SETUP
// =====================================

void setupWiFi() {

  Serial.println("\nConnecting to Wi-Fi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 15000) {

    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {

    apMode = false;

    Serial.println("\nWi-Fi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

  } else {

    Serial.println("\nWi-Fi connection failed!");
    Serial.println("Starting fallback Access Point...");

    WiFi.disconnect();
    WiFi.mode(WIFI_AP);

    WiFi.softAP(AP_SSID, AP_PASSWORD);

    apMode = true;

    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
  }
}

// =====================================
// ARDUINO OTA
// =====================================

void setupArduinoOTA() {

  ArduinoOTA.setHostname(OTA_HOSTNAME);

  ArduinoOTA
    .onStart([]() {
      Serial.println("\nArduino OTA update started.");
    })
    .onEnd([]() {
      Serial.println("\nArduino OTA update complete.");
    })
    .onProgress([](unsigned int progress,
                   unsigned int total) {

      Serial.printf("OTA Progress: %u%%\r",
        (progress * 100) / total);
    })
    .onError([](ota_error_t error) {

      Serial.printf("OTA Error: %u\n", error);
    });

  ArduinoOTA.begin();

  Serial.println("Arduino OTA ready.");
}

// =====================================
// WEBSITE ROUTES
// =====================================

void setupWebServer() {

  server.on("/", HTTP_GET, []() {

    server.send_P(200, "text/html", webpage);

  });

  server.on("/status", HTTP_GET, []() {

    String json = "{";

    json += "\"sensor1\":" + String(sensor1) + ",";
    json += "\"sensor2\":" + String(sensor2) + ",";
    json += "\"sensor3\":" + String(sensor3) + ",";
    json += "\"flood\":" + String(waterDetected ? "true" : "false") + ",";
    json += "\"servo\":" + String(servoActivated ? "true" : "false") + ",";
    json += "\"ap\":" + String(apMode ? "true" : "false") + ",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";

    if (apMode) {
      json.replace(
        WiFi.localIP().toString(),
        WiFi.softAPIP().toString()
      );
    }

    json += "}";

    server.send(200, "application/json", json);

  });

  // Firmware upload response
  server.on("/update", HTTP_POST,

    []() {

      if (Update.hasError()) {

        server.send(500, "text/plain",
          "Firmware update failed!");

      } else {

        server.send(200, "text/plain",
          "Firmware updated successfully! Restarting...");

        delay(1000);
        ESP.restart();
      }

    },

    []() {

      HTTPUpload& upload = server.upload();

      if (upload.status == UPLOAD_FILE_START) {

        Serial.printf("Firmware upload: %s\n",
          upload.filename.c_str());

        updateSuccess =
          Update.begin(UPDATE_SIZE_UNKNOWN);

      }

      else if (upload.status == UPLOAD_FILE_WRITE) {

        if (updateSuccess) {

          if (Update.write(
                upload.buf,
                upload.currentSize
              ) != upload.currentSize) {

            updateSuccess = false;
          }
        }
      }

      else if (upload.status == UPLOAD_FILE_END) {

        if (updateSuccess) {

          updateSuccess = Update.end(true);

        }

        Serial.printf("Firmware upload finished: %u bytes\n",
          upload.totalSize);

      }

    }
  );

  server.begin();

  Serial.println("Website server started.");
}

// =====================================
// FLOOD DETECTION
// =====================================

void readSensors() {

  sensor1 = analogRead(WATER_1);
  sensor2 = analogRead(WATER_2);
  sensor3 = analogRead(WATER_3);

  waterDetected =
    sensor1 > WATER_THRESHOLD &&
    sensor2 > WATER_THRESHOLD;

  thirdSensorWet =
    sensor3 > WATER_THRESHOLD;

  if (waterDetected && !servoActivated) {

    servoActivated = true;

    floodServo.write(SERVO_ON);

    Serial.println("FLOOD DETECTED!");
    Serial.println("Servo activated.");

  }

  else if (!waterDetected && servoActivated) {

    servoActivated = false;

    floodServo.write(SERVO_OFF);

    Serial.println("Water level returned to normal.");
    Serial.println("Servo deactivated.");

  }
}

// =====================================
// SETUP
// =====================================

void setup() {

  Serial.begin(115200);

  pinMode(WATER_1, INPUT);
  pinMode(WATER_2, INPUT);
  pinMode(WATER_3, INPUT);

  analogReadResolution(12);

  floodServo.setPeriodHertz(50);
  floodServo.attach(SERVO_PIN, 500, 2400);
  floodServo.write(SERVO_OFF);

  Serial.println("\n==========================");
  Serial.println("   FLOOD DETECTOR V1");
  Serial.println("==========================");

  setupWiFi();

  setupArduinoOTA();

  setupWebServer();

  Serial.println("\nSystem initialized!");
}

// =====================================
// MAIN LOOP
// =====================================

void loop() {

  readSensors();

  ArduinoOTA.handle();

  server.handleClient();

  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 1000) {

    lastPrint = millis();

    Serial.println("\n--- FLOOD STATUS ---");

    Serial.printf("Sensor 1: %d\n", sensor1);
    Serial.printf("Sensor 2: %d\n", sensor2);
    Serial.printf("Sensor 3: %d\n", sensor3);

    Serial.printf("Water detected: %s\n",
      waterDetected ? "YES" : "NO");

    Serial.printf("Third sensor wet: %s\n",
      thirdSensorWet ? "YES" : "NO");

    Serial.printf("Servo: %d degrees\n",
      servoActivated ? SERVO_ON : SERVO_OFF);
  }

  delay(50);
}
