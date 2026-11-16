# Smart Streetlight - wiring (ESP32 DevKit on COM12)

Parts you have: ESP32, PIR (HC-SR501), LDR, LEDs, jumper cables, breadboard.
**No resistors are needed** - the sketch uses the ESP32's internal pull-down for the
LDR, and caps the LED PWM so the LEDs are safe straight off the GPIO pins.

Put the ESP32 across the middle gap of the breadboard so each pin gets its own row.

| Part | Pin on part | ESP32 pin | Notes |
|------|-------------|-----------|-------|
| LDR | either leg | 3V3 | LDR has no polarity |
| LDR | other leg | GPIO32 | internal pull-down does the voltage-divider job |
| PIR | VCC | VIN (5V) | PIR needs 5 V; its OUT is 3.3 V so it is safe |
| PIR | GND | GND | |
| PIR | OUT | GPIO27 | middle pin on most HC-SR501 boards |
| Lamp LED | long leg (+) | GPIO25 | this is the "streetlight" (PWM dimmed) |
| Lamp LED | short leg (-) | GND | |
| Motion LED (optional) | long leg (+) | GPIO26 | lights while PIR sees motion |
| Motion LED | short leg (-) | GND | |

Tips
- Use one breadboard rail as GND and connect it to any ESP32 GND pin once.
- If you only have one LED, skip the motion LED.
- Prefer a white or blue LED for the lamp if you have one (safest with no resistor).
  Red/green also work; the sketch keeps the PWM at 70 % max for that reason.

## Test it
1. Serial Monitor at 115200 baud.
2. Wait the 30 s PIR warm-up (printed on screen).
3. Cover the LDR with your finger -> `night=1`, lamp goes dim (20 %).
4. Wave in front of PIR -> lamp 100 % for 15 s, then back to 20 %.
5. Uncover LDR -> `night=0`, lamp off.

If night never switches: note the `LDR=` number with the LDR covered and uncovered,
and set `NIGHT_LEVEL` in the sketch roughly midway between the two values.
