# Circuit & Wiring Guide

## Components
| Component | Qty | Notes |
|---|---|---|
| ESP32 DevKit board | 1 | Any ESP32-WROOM based dev board |
| 4-channel relay module | 1 | 5V, opto-isolated recommended |
| Bedroom light (bulb + holder) | 1 | Switched load |
| Hall light (bulb + holder) | 1 | Switched load |
| Fan / DC motor | 1 | Switched load |
| Door lock / motor | 1 | Switched load |
| 5V power supply | 1 | For relay module (ESP32 can run off USB while testing) |
| Jumper wires | ~10 | |
| Breadboard | 1 | Optional, for prototyping only |

## GPIO Map
| Function | ESP32 Pin | Relay Channel |
|---|---|---|
| Bedroom light | GPIO 2 | Relay 1 |
| Hall light | GPIO 4 | Relay 2 |
| Fan | GPIO 5 | Relay 3 |
| Door lock | GPIO 18 | Relay 4 |
| Status LED (optional) | GPIO 15 | External LED + 220Ω resistor to GND |

## Wiring Steps
1. Connect ESP32 **GND** to relay module **GND**.
2. Connect ESP32 **5V (VIN)** or an external 5V supply to the relay module **VCC**.
   > Do not power 4 relays directly from the ESP32's onboard 3.3V regulator — use
   > an external 5V supply or the VIN pin fed from USB, otherwise you risk brownouts.
3. Connect GPIO 2, 4, 5, 18 to the IN1–IN4 pins on the relay module.
4. Wire each relay's COM/NO (normally open) terminals in series with the live
   wire of the appliance it controls. **Mains wiring carries lethal voltage —
   have a qualified person do this part, or use low-voltage DC loads (motor/LED)
   for a safe classroom demo instead of real mains appliances.**
5. Optional: wire an LED + 220Ω resistor from GPIO 15 to GND as a connection
   status indicator (solid = WiFi+MQTT connected, blinking = reconnecting).

## Relay Logic Note
Some relay modules are **active-LOW** (a LOW signal energizes the relay),
others are **active-HIGH**. The firmware already accounts for this per-device
in the `devices[]` array (`activeLow` field) — bedroom/hall default to
active-LOW, fan/door default to active-HIGH, matching common 4-channel relay
boards. Check your specific relay module's datasheet and adjust if your
relays click at the wrong moment.
