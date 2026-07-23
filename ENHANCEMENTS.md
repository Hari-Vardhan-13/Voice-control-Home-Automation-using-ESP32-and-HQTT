# Enhancements Over the Original Sketch

The original `Voice_control_home_automation.ino` was a solid working proof
of concept: ESP32 + MQTT + relay switching. Here's exactly what changed and
why, so you can explain each choice if asked in a review/viva.

| # | Area | Original | Now | Why it matters |
|---|---|---|---|---|
| 1 | Credentials | WiFi + MQTT username/password hardcoded in the `.ino` | Moved to `secrets.h` (gitignored), template provided as `secrets.h.example` | Hardcoded secrets get leaked the moment the code is shared, uploaded to GitHub, or pasted anywhere. Any reviewer will flag this immediately. |
| 2 | Reconnect logic | `while (!mqtt.connected())` with blocking `delay(3000)` inside `loop()` | Non-blocking, timestamp-based retry (`millis()`) | The original design freezes the entire board (no button presses, no other logic) for seconds at a time while reconnecting. Non-blocking code stays responsive. |
| 3 | Voice input | None — the sketch only reacts to MQTT ON/OFF; nothing captured a voice command | Added `web-app/index.html`: browser speech recognition → MQTT publish | The project is titled "Voice Control" but had no voice input path at all. This closes that gap with something you can actually demo live. |
| 4 | State sync | No retained messages — a newly opened app has no idea if the fan is on or off | State topics published with `retained = true`; ESP32 republishes all states on every reconnect | Without this, your UI can show "OFF" while the fan is actually running. |
| 5 | Firmware updates | USB cable required for every re-flash | `ArduinoOTA` support added, ESP32 reachable at `esp32-home.local` | Useful once the board is wired into a wall/enclosure and no longer convenient to unplug. |
| 6 | Status visibility | No way to tell if the board is online without opening Serial Monitor | Optional status LED (GPIO 15): solid = connected, blinking = trying to reconnect | Handy for a live demo/viva — visible proof the board is alive. |
| 7 | Extensibility | Devices hardcoded as `if/else` chain | Devices defined in a `DeviceCfg[]` array with per-device relay polarity | Adding a 5th appliance is now a one-line array entry instead of editing multiple functions. |
| 8 | Commands supported | `ON` / `OFF` only | `ON` / `OFF` / `TOGGLE`, plus a `get` topic to query state | `TOGGLE` maps naturally to a single voice phrase like "toggle the fan." |

## Still worth doing before a "production" deployment (not done here, flagged for awareness)
- Replace `secureClient.setInsecure()` with a pinned CA certificate
  (`setCACert()`) — `setInsecure()` skips TLS certificate verification,
  which is fine for a classroom demo but not for anything real.
- Add authentication/PIN confirmation before actuating the door lock
  specifically, since it's a security-relevant actuator.
- Move relay outputs behind a watchdog timer (`esp_task_wdt`) so a firmware
  hang can't leave an appliance stuck ON.
