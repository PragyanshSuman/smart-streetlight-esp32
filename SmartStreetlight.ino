/*
 *  SMART STREETLIGHT  -  Automatic Day/Night & Motion-Based Control
 *  Board : ESP32 DevKit (ESP32-D0WD)        Arduino core 3.x
 *
 *  Three operating states (see slide 8 of the presentation):
 *    DAY            -> LDR bright                 -> lamp   0 %
 *    NIGHT / IDLE   -> LDR dark, no motion        -> lamp  20 %
 *    NIGHT / MOTION -> LDR dark, PIR triggered    -> lamp 100 %  (held 15 s)
 *
 *  Wiring - NO external resistors needed (see WIRING.md):
 *    LDR  : 3V3 --[LDR]-- GPIO32   (internal ~45k pull-down acts as the divider)
 *    PIR  : VCC -> VIN(5V), GND -> GND, OUT -> GPIO27
 *    LAMP : GPIO25 --|>|-- GND     (the "streetlight", PWM-limited current)
 *    MOTION LED : GPIO26 --|>|-- GND   (optional indicator)
 */

// ---------- pins ----------
const int PIN_LDR        = 32;   // ADC1_CH4, has internal pull-down
const int PIN_PIR        = 27;
const int PIN_LAMP       = 25;   // PWM
const int PIN_MOTION_LED = 26;   // simple on/off indicator

// ---------- tuning ----------
int   NIGHT_LEVEL        = 1200;  // ADC (0-4095). Below this = night. Adjust from serial readings.
const int   HYSTERESIS   = 150;   // avoids flicker right at the threshold
const unsigned long HOLD_MS   = 15000;  // keep full brightness 15 s after last motion
const int   PWM_OFF      = 0;
const int   PWM_FULL     = 180;   // "100 %" - capped so a resistor-less LED stays safe
const int   PWM_DIM      = PWM_FULL / 5;   // 20 %
const unsigned long REPORT_MS = 1000;   // serial status interval

// ---------- optional MQTT (set to 1 and fill in credentials) ----------
#define ENABLE_MQTT 0
#if ENABLE_MQTT
  #include <WiFi.h>
  #include <PubSubClient.h>
  const char* WIFI_SSID = "YOUR_WIFI";
  const char* WIFI_PASS = "YOUR_PASSWORD";
  const char* MQTT_HOST = "broker.hivemq.com";
  const int   MQTT_PORT = 1883;
  WiFiClient   wifiClient;
  PubSubClient mqtt(wifiClient);
#endif

// ---------- state ----------
bool          isNight       = false;
unsigned long holdUntil     = 0;
int           currentPwm    = -1;
unsigned long lastReport    = 0;
const char*   stateName     = "BOOT";

void setLamp(int pwm) {
  if (pwm != currentPwm) {
    analogWrite(PIN_LAMP, pwm);          // core 3.x: LEDC-backed PWM
    currentPwm = pwm;
  }
}

#if ENABLE_MQTT
void mqttConnect() {
  if (WiFi.status() != WL_CONNECTED || mqtt.connected()) return;
  if (mqtt.connect("streetlight-01")) mqtt.subscribe("streetlight/01/cmd");
}
void mqttPublish(int lux, bool motion, int pwm) {
  if (!mqtt.connected()) return;
  char buf[16];
  mqtt.publish("streetlight/01/lux",        itoa(lux, buf, 10));
  mqtt.publish("streetlight/01/motion",     motion ? "1" : "0");
  mqtt.publish("streetlight/01/brightness", itoa(pwm * 100 / PWM_FULL, buf, 10));
  mqtt.publish("streetlight/01/status",     stateName);
}
#endif

void setup() {
  Serial.begin(115200);
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_LDR, INPUT_PULLDOWN);   // replaces the external 10k resistor
  pinMode(PIN_MOTION_LED, OUTPUT);
  pinMode(PIN_LAMP, OUTPUT);
  analogReadResolution(12);
  setLamp(PWM_OFF);

  Serial.println();
  Serial.println("=== SMART STREETLIGHT (ESP32) ===");
  Serial.println("PIR warming up for 30 s - keep still...");
  for (int i = 30; i > 0; i -= 5) { Serial.printf("  %d s\n", i); delay(5000); }
  Serial.println("Ready.");
  Serial.println("Columns: LDR(adc)  night?  PIR  lamp%  state");

#if ENABLE_MQTT
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
#endif
}

void loop() {
  // 1. read sensors
  int  ldr    = analogRead(PIN_LDR);       // higher = brighter (LDR on the 3V3 side)
  bool motion = digitalRead(PIN_PIR) == HIGH;
  digitalWrite(PIN_MOTION_LED, motion);

  // 2. day / night with hysteresis
  if (!isNight && ldr < NIGHT_LEVEL - HYSTERESIS) isNight = true;
  if ( isNight && ldr > NIGHT_LEVEL + HYSTERESIS) isNight = false;

  // 3. decide brightness
  int pwm;
  if (!isNight) {
    pwm = PWM_OFF;  stateName = "DAY";
  } else {
    if (motion) holdUntil = millis() + HOLD_MS;
    if (millis() < holdUntil) { pwm = PWM_FULL; stateName = "NIGHT/MOTION"; }
    else                      { pwm = PWM_DIM;  stateName = "NIGHT/IDLE";   }
  }
  setLamp(pwm);

  // 4. report
  if (millis() - lastReport >= REPORT_MS) {
    lastReport = millis();
    Serial.printf("LDR=%4d  night=%d  PIR=%d  lamp=%3d%%  %s\n",
                  ldr, isNight, motion, pwm * 100 / PWM_FULL, stateName);
#if ENABLE_MQTT
    mqttConnect(); mqtt.loop();
    mqttPublish(ldr, motion, pwm);
#endif
  }
  delay(50);
}
