# Project Report — Voice Control Home Automation using ESP32

**Department of ECE**
Presented by: P Girish (2300040110), G Mukhesh Kumar (2300040145), P Mahesh (2300040186)
Guided by: Dr. N. Prabakaran, Associate Professor, Dept. of ECE

## 1. Objective
- Design and implement a smart home automation system using ESP32 and IoT.
- Remotely control home appliances (lights, fan, door) over the internet.
- Enable real-time, low-latency communication between devices using MQTT.
- Add voice control so appliances can be operated hands-free.
- Keep the system low-cost, energy-efficient, and reasonably secure.

## 2. Problem Statement
Home appliances are traditionally controlled with manual switches, which is
inconvenient and can waste energy when devices are left on unnecessarily.
This project builds a low-cost system that lets a user monitor and control
appliances remotely over the internet, and additionally by voice, using an
ESP32 microcontroller and the MQTT protocol.

## 3. System Architecture
```
 [Voice / Mobile / Web App]
         |  (speech -> text -> MQTT publish, over WebSocket+TLS)
         v
 [HiveMQ Cloud MQTT Broker]  <-- TLS (port 8883/8884) -->
         ^
         |  (subscribe: home/+/set, home/+/get)
         |  (publish : home/+/state, retained)
         v
     [ESP32 DevKit]
         |
         v
 [4-Channel Relay Module]
         |
         v
 [Bedroom light | Hall light | Fan | Door lock]
```

## 4. Hardware Components
- ESP32 microcontroller
- 4-channel relay module
- Light bulbs (bedroom and hall)
- Fan / DC motor
- Door lock / motor
- 5V power supply
- Jumper wires, breadboard (optional)
- WiFi router / internet connection

## 5. Software Components
- Arduino IDE + ESP32 board package
- PubSubClient (MQTT client library)
- HiveMQ Cloud (managed MQTT broker, TLS)
- A browser-based voice control client (Web Speech API + MQTT.js over WebSocket) — added in this revision, see `web-app/`

## 6. Working / Methodology
1. **Initialization**: ESP32 connects to WiFi, then opens a TLS connection to the HiveMQ Cloud broker and subscribes to command topics.
2. **Command input**: The user speaks a command ("turn on the bedroom light") into the web app; the browser's speech recognition converts it to text, which is matched to a device + action and published to `home/<device>/set`.
3. **Delivery**: HiveMQ instantly routes the message to the subscribed ESP32.
4. **Actuation**: The ESP32 parses the topic/payload and drives the matching GPIO pin.
5. **Switching**: The relay module switches the higher-current appliance load from the ESP32's low-voltage GPIO signal.
6. **Feedback**: The ESP32 publishes the new state back to `home/<device>/state` (retained), so the app reflects the true hardware state, even after reconnecting.

## 7. Enhancements Made in This Revision
See `ENHANCEMENTS.md` in the project root for the full list and rationale —
summary: credentials externalized, non-blocking reconnect logic, OTA
firmware updates, mDNS hostname, retained state sync, and an actual voice
input client (the original sketch had no voice input path).

## 8. Conclusion
This project demonstrates a working smart home automation system built on
ESP32 and MQTT, extended with a genuine voice-control front end. It remains
low-cost and simple to reproduce while giving practical, hands-on exposure
to IoT protocols, embedded firmware design, and secure credential handling.

## 9. Future Scope
- Add sensors (PIR, temperature, gas) for automatic/reactive automation.
- Add power consumption analytics per device.
- Multi-user access control with per-user permissions.
- Integrate with smart grid / demand-response signaling.
- Replace `setInsecure()` TLS with full certificate pinning for production use.
