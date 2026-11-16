# Smart Streetlight (ESP32)

Automatic day/night and motion-based streetlight using an ESP32, an LDR and a PIR sensor.

| State | Condition | Lamp |
|-------|-----------|------|
| Day | LDR bright | 0 % |
| Night / idle | LDR dark, no motion | 20 % |
| Night / motion | LDR dark, PIR triggered | 100 % for 15 s |

## Hardware
ESP32 DevKit, PIR (HC-SR501), LDR, 1-2 LEDs, jumper wires, breadboard. No resistors needed.
See [WIRING.md](WIRING.md) for the connections.

## Build and flash
Arduino IDE or arduino-cli with the `esp32:esp32` core (3.x):

```
arduino-cli compile --fqbn esp32:esp32:esp32 .
arduino-cli upload -p COMx --fqbn esp32:esp32:esp32 .
```

Open the serial monitor at 115200 baud to see live readings.

## Optional MQTT
Set `ENABLE_MQTT` to 1 in the sketch, fill in your Wi-Fi details and install the PubSubClient library.
Topics: `streetlight/01/{lux,motion,brightness,status,cmd}`.
