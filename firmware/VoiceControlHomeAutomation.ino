/*
  Voice Control Home Automation using ESP32 + MQTT (HiveMQ Cloud)
  ------------------------------------------------------------------
  Department of ECE — Embedded Prototype
  Base project: Voice_control_home_automation.ino
  Enhanced by: restructured for reliability, security, and maintainability

  WHAT CHANGED FROM THE ORIGINAL SKETCH
  --------------------------------------
  1. Credentials moved out of this file into secrets.h (not hardcoded).
  2. Non-blocking WiFi + MQTT reconnect (no more delay()-blocked loop()).
  3. mDNS hostname so the board is reachable as esp32-home.local
  4. OTA (Over-The-Air) updates -- re-flash the ESP32 over WiFi, no USB cable.
  5. Retained MQTT state topics so the app/voice client shows correct
     ON/OFF status immediately after reconnecting or reopening the app.
  6. A "home/<device>/get" request topic so the app can ask the ESP32
     to republish current state on demand.
  7. Basic input validation + a status heartbeat topic for debugging.

  LIBRARIES REQUIRED (Arduino IDE > Tools > Manage Libraries):
    - PubSubClient      by Nick O'Leary
    - ESP32 board package (via Boards Manager)
  Built-in (no install needed): WiFi, WiFiClientSecure, ESPmDNS,
  ArduinoOTA, Preferences

  BEFORE UPLOADING:
    Copy secrets.h.example -> secrets.h and fill in your WiFi + MQTT
    credentials. Do not upload/commit secrets.h anywhere public.
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

#include "secrets.h"   // WIFI_SSID, WIFI_PASS, MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASS, CLIENT_ID

// ---------------- PIN MAP ----------------
#define PIN_BEDROOM 2
#define PIN_HALL    4
#define PIN_FAN     5
#define PIN_DOOR    18
#define PIN_STATUS_LED 15   // optional onboard/external LED: solid = connected, blinking = trying to connect

// Some relay modules are "active LOW" (LOW = relay energized).
// Set this per device to match your hardware. true = active LOW relay.
struct DeviceCfg {
  const char* name;
  int pin;
  bool activeLow;
};

DeviceCfg devices[] = {
  { "bedroom", PIN_BEDROOM, true  },
  { "hall",    PIN_HALL,    true  },
  { "fan",     PIN_FAN,     false },
  { "door",    PIN_DOOR,    false },
};
const int NUM_DEVICES = sizeof(devices) / sizeof(devices[0]);

// Track last known state in RAM so we can answer "get" requests and
// republish on reconnect without guessing.
bool deviceState[4] = { false, false, false, false }; // false = OFF

WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

unsigned long lastWifiAttempt = 0;
unsigned long lastMqttAttempt = 0;
unsigned long lastHeartbeat   = 0;
const unsigned long WIFI_RETRY_MS = 5000;
const unsigned long MQTT_RETRY_MS = 3000;
const unsigned long HEARTBEAT_MS  = 30000;

// ---------------- HELPERS ----------------

int findDeviceIndex(const String& name) {
  for (int i = 0; i < NUM_DEVICES; i++) {
    if (name == devices[i].name) return i;
  }
  return -1;
}

void applyState(int idx, bool on) {
  DeviceCfg& d = devices[idx];
  int level;
  if (d.activeLow) {
    level = on ? LOW : HIGH;
  } else {
    level = on ? HIGH : LOW;
  }
  digitalWrite(d.pin, level);
  deviceState[idx] = on;
}

void publishState(int idx, bool retained = true) {
  String topic = "home/" + String(devices[idx].name) + "/state";
  String payload = deviceState[idx] ? "ON" : "OFF";
  mqtt.publish(topic.c_str(), payload.c_str(), retained);
}

void publishAllStates() {
  for (int i = 0; i < NUM_DEVICES; i++) publishState(i);
}

void publishHeartbeat() {
  mqtt.publish("home/esp32/heartbeat", String(millis() / 1000).c_str());
}

// ---------------- MQTT CALLBACK ----------------

void callback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  String msg;
  msg.reserve(length);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  // Expect topics shaped like: home/<device>/set  or  home/<device>/get
  int a = t.indexOf('/');
  int b = t.indexOf('/', a + 1);
  if (a == -1 || b == -1) return;

  String device = t.substring(a + 1, b);
  String action = t.substring(b + 1);

  int idx = findDeviceIndex(device);
  if (idx == -1) {
    Serial.println("Unknown device: " + device);
    return;
  }

  if (action == "set") {
    msg.trim();
    msg.toUpperCase();
    if (msg == "ON") {
      applyState(idx, true);
      publishState(idx);
    } else if (msg == "OFF") {
      applyState(idx, false);
      publishState(idx);
    } else if (msg == "TOGGLE") {
      applyState(idx, !deviceState[idx]);
      publishState(idx);
    } else {
      Serial.println("Unrecognized command: " + msg);
    }
  } else if (action == "get") {
    publishState(idx);
  }
}

// ---------------- CONNECTION HANDLING (non-blocking) ----------------

void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("esp32-home");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void handleWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  unsigned long now = millis();
  if (now - lastWifiAttempt >= WIFI_RETRY_MS) {
    lastWifiAttempt = now;
    Serial.println("WiFi not connected, retrying...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }
}

void handleMqtt() {
  if (mqtt.connected()) {
    mqtt.loop();
    return;
  }
  unsigned long now = millis();
  if (now - lastMqttAttempt < MQTT_RETRY_MS) return;
  lastMqttAttempt = now;

  Serial.print("Connecting to MQTT... ");
  if (mqtt.connect(CLIENT_ID, MQTT_USER, MQTT_PASS)) {
    Serial.println("connected");
    mqtt.subscribe("home/+/set");
    mqtt.subscribe("home/+/get");
    publishAllStates();       // sync app/voice client with real hardware state
  } else {
    Serial.print("failed, rc=");
    Serial.println(mqtt.state());
  }
}

void updateStatusLed() {
  bool ok = WiFi.status() == WL_CONNECTED && mqtt.connected();
  digitalWrite(PIN_STATUS_LED, ok ? HIGH : (millis() / 300) % 2); // solid vs blinking
}

// ---------------- SETUP ----------------

void setup() {
  Serial.begin(115200);
  delay(200);

  for (int i = 0; i < NUM_DEVICES; i++) {
    pinMode(devices[i].pin, OUTPUT);
    applyState(i, false); // start with everything OFF and pins in a known state
  }
  pinMode(PIN_STATUS_LED, OUTPUT);

  setupWifi();
  Serial.print("Connecting to WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    Serial.print(".");
    delay(300);
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " connected" : " timed out, will retry in loop()");

  // NOTE: setInsecure() skips TLS certificate verification. It's the
  // common approach for student/prototype projects but is NOT secure
  // for production. For a hardened build, load the broker's CA
  // certificate with secureClient.setCACert(...) instead.
  secureClient.setInsecure();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(512);
  mqtt.setCallback(callback);

  // mDNS: reach the board at http://esp32-home.local for OTA discovery
  if (MDNS.begin("esp32-home")) {
    Serial.println("mDNS responder started: esp32-home.local");
  }

  // OTA updates
  ArduinoOTA.setHostname("esp32-home");
  ArduinoOTA.onStart([]() { Serial.println("OTA update starting..."); });
  ArduinoOTA.onEnd([]()   { Serial.println("OTA update complete."); });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]\n", error);
  });
  ArduinoOTA.begin();

  Serial.println("Setup complete.");
}

// ---------------- LOOP ----------------

void loop() {
  handleWifi();
  handleMqtt();
  ArduinoOTA.handle();
  updateStatusLed();

  unsigned long now = millis();
  if (mqtt.connected() && now - lastHeartbeat >= HEARTBEAT_MS) {
    lastHeartbeat = now;
    publishHeartbeat();
  }
}
