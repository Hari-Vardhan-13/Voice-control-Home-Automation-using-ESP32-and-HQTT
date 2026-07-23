# MQTT Topic Reference

Broker: HiveMQ Cloud (or any TLS-enabled MQTT broker)
Port: 8883 (TLS, used by the ESP32) / 8884 (WebSocket + TLS, used by the web app)

## Topics

| Topic | Direction | Payload | Purpose |
|---|---|---|---|
| `home/<device>/set` | App/Voice → ESP32 | `ON`, `OFF`, `TOGGLE` | Command a device |
| `home/<device>/get` | App/Voice → ESP32 | (empty) | Request current state |
| `home/<device>/state` | ESP32 → App/Voice | `ON`, `OFF` | Current state (retained) |
| `home/esp32/heartbeat` | ESP32 → App/Voice | seconds since boot | Liveness check, published every 30s |

`<device>` is one of: `bedroom`, `hall`, `fan`, `door`

## Examples

Turn the bedroom light on:
```
Topic:   home/bedroom/set
Payload: ON
```

Ask the fan's current state:
```
Topic:   home/fan/get
Payload: (empty)
```

The ESP32 responds on the retained state topic:
```
Topic:   home/fan/state
Payload: OFF
```

## Why retained messages matter
State topics are published with the **retained** flag. This means when the
web app (or any new subscriber) connects and subscribes to `home/+/state`,
it immediately receives the last known value from the broker — no need to
wait for the ESP32 to republish. This keeps the UI accurate even after a
phone reconnects or a browser tab is reopened.

## Testing without hardware
You can simulate the ESP32 side using any MQTT client (e.g. MQTT Explorer,
`mosquitto_pub`/`mosquitto_sub`, or HiveMQ's own web client) to publish to
`home/<device>/set` and confirm your web app reacts correctly, before the
firmware is even flashed.
