# Voice Control Home Automation — ESP32 + MQTT

An embedded IoT mini-project: control home appliances (bedroom light, hall
light, fan, door lock) over the internet using an ESP32, a 4-channel relay
module, and the MQTT protocol via HiveMQ Cloud — with an added browser-based
voice control front end.

## 🔴 Live Demo
**https://home-automation-voice-control.netlify.app/**
This is the deployed voice-control dashboard (built with Lovable, hosted on
Netlify). Use this for your actual demo/viva. `web-app/index.html` in this
repo is kept only as a lightweight local reference version. See
`docs/deployment.md`.

## Project Structure
```
VoiceControlHomeAutomation/
├── README.md                  <- you are here
├── ENHANCEMENTS.md             <- what was improved over the original sketch, and why
├── firmware/
│   ├── VoiceControlHomeAutomation.ino
│   └── secrets.h.example       <- copy to secrets.h and fill in your WiFi/MQTT credentials
├── web-app/
│   ├── index.html              <- local reference voice control client (open in Chrome/Edge)
│   └── README.md                <- points to the live deployed app
├── docs/
│   ├── circuit_wiring.md        <- GPIO map + wiring steps
│   ├── mqtt_topics.md           <- MQTT topic reference
│   ├── project_report.md        <- written report content for submission
│   └── deployment.md            <- live Netlify app + how it relates to this repo
├── presentation/
│   └── AUTOMATION_HOME.pptx     <- original project slide deck
└── media/
    └── home_automation.mp4      <- demo video
```

## Quick Start

### 1. Firmware
1. Open `firmware/VoiceControlHomeAutomation.ino` in the Arduino IDE.
2. Install the **ESP32 board package** (Boards Manager) and the
   **PubSubClient** library (Library Manager).
3. In the `firmware/` folder, copy `secrets.h.example` → `secrets.h` and
   fill in your real WiFi SSID/password and HiveMQ Cloud host/username/password.
4. Select your ESP32 board + COM port, then Upload.
5. Open Serial Monitor at 115200 baud to confirm WiFi + MQTT connect.

### 2. Wiring
Follow `docs/circuit_wiring.md` for the GPIO-to-relay mapping and power notes.

### 3. Voice control web app
1. Open `web-app/index.html` in Chrome or Edge (desktop or Android — iOS
   Safari doesn't support the Web Speech API).
2. In HiveMQ Cloud Console, confirm the WebSocket listener is enabled
   (default port 8884) and copy the `wss://...` URL.
3. Paste that URL plus your MQTT username/password into the app and hit
   **Connect**.
4. Tap the mic button and say things like *"turn on the bedroom light"* or
   *"turn off the fan"*. You can also tap the ON/OFF buttons directly.

### 4. Reference material
- `presentation/AUTOMATION_HOME.pptx` — original slide deck for submission/viva.
- `docs/project_report.md` — written report matching the deck content, updated with this revision's changes.
- `media/home_automation.mp4` — demo video of the original build.

## What Changed From the Original Sketch
See `ENHANCEMENTS.md` for the full before/after table with reasoning —
short version: credentials are no longer hardcoded, reconnect logic is
non-blocking, OTA updates and mDNS were added, device state now syncs
reliably via retained MQTT messages, and there's a genuine voice input path
that the original code was missing.

## Security Note
Please rotate your HiveMQ Cloud MQTT password. It was hardcoded in the
original `.ino` file, which means it may already be exposed anywhere that
file has been shared (chat, forum, cloud storage, etc.). Going forward,
keep real credentials only in your local `secrets.h`, which should never be
committed to Git or shared.
