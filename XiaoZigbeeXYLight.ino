#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

#ifndef ZIGBEE_MODE_ED
#error "Select Tools -> Zigbee Mode -> Zigbee ED (end device)"
#endif

#include "Zigbee.h"
#include "ZigbeeXYLight.h"

#define LED_PIN          2
#define LED_COUNT        16
#define ZIGBEE_ENDPOINT  10
#define BUTTON_PIN       BOOT_PIN
#define RESET_TIME_MS    3000
#define BLINK_TIME_MS    250

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
ZigbeeXYLight zbLight(ZIGBEE_ENDPOINT);

void setStripColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void clearStrip() {
  strip.clear();
  strip.show();
}

void xyToRGB(uint16_t rawX, uint16_t rawY, uint8_t &red, uint8_t &green, uint8_t &blue) {
  float x = rawX / 65535.0f;
  float y = rawY / 65535.0f;

  if (y <= 0.0001f) {
    red = green = blue = 0;
    return;
  }

  float z = 1.0f - x - y;
  float Y = 1.0f;
  float X = (Y / y) * x;
  float Z = (Y / y) * z;

  float r = X * 1.656492f - Y * 0.354851f - Z * 0.255038f;
  float g = -X * 0.707196f + Y * 1.655397f + Z * 0.036152f;
  float b = X * 0.051713f - Y * 0.121364f + Z * 1.011530f;

  if (r < 0) r = 0;
  if (g < 0) g = 0;
  if (b < 0) b = 0;

  float maxValue = max(r, max(g, b));
  if (maxValue > 1.0f) {
    r /= maxValue;
    g /= maxValue;
    b /= maxValue;
  }

  r = (r <= 0.0031308f) ? 12.92f * r : 1.055f * powf(r, 1.0f / 2.4f) - 0.055f;
  g = (g <= 0.0031308f) ? 12.92f * g : 1.055f * powf(g, 1.0f / 2.4f) - 0.055f;
  b = (b <= 0.0031308f) ? 12.92f * b : 1.055f * powf(b, 1.0f / 2.4f) - 0.055f;

  red = constrain((int)(r * 255.0f), 0, 255);
  green = constrain((int)(g * 255.0f), 0, 255);
  blue = constrain((int)(b * 255.0f), 0, 255);
}

void lightChanged(bool state, uint8_t level, uint16_t x, uint16_t y) {
  Serial.println("\n----- Zigbee update -----");
  Serial.printf("State: %s\n", state ? "ON" : "OFF");
  Serial.printf("Level: %u\n", level);
  Serial.printf("X: %u Y: %u\n", x, y);

  if (!state) {
    clearStrip();
    return;
  }

  uint8_t r, g, b;
  xyToRGB(x, y, r, g, b);

  float brightness = min((float)level / 254.0f, 1.0f);
  r = (uint8_t)(r * brightness);
  g = (uint8_t)(g * brightness);
  b = (uint8_t)(b * brightness);

  Serial.printf("RGB: %u, %u, %u\n", r, g, b);
  setStripColor(r, g, b);
}

void restoreLight() {
  lightChanged(zbLight.getLightState(), zbLight.getLightLevel(), zbLight.getLightX(), zbLight.getLightY());
}

void performFactoryReset() {
  Serial.println("\n==========================");
  Serial.println(" ZIGBEE FACTORY RESET");
  Serial.println("==========================");

  setStripColor(255, 0, 0);
  delay(1000);
  clearStrip();

  Serial.println("Erasing Zigbee network data...");
  Zigbee.factoryReset();
  delay(3000);
  ESP.restart();
}

void handleButton() {
  if (digitalRead(BUTTON_PIN) != LOW) return;

  delay(30);
  if (digitalRead(BUTTON_PIN) != LOW) return;

  unsigned long start = millis();
  unsigned long lastBlink = 0;
  bool blink = false;

  Serial.println("BOOT pressed - hold 3 seconds for Zigbee factory reset");

  while (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long now = millis();

    if (now - lastBlink >= BLINK_TIME_MS) {
      lastBlink = now;
      blink = !blink;
      if (blink) setStripColor(255, 0, 0);
      else clearStrip();
    }

    if (now - start >= RESET_TIME_MS) {
      performFactoryReset();
      return;
    }

    delay(10);
  }

  Serial.println("BOOT released - reset cancelled");
  restoreLight();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==========================");
  Serial.println(" XIAO ESP32-C6");
  Serial.println(" Zigbee WS2812B XY Light");
  Serial.println("==========================");

  strip.begin();
  clearStrip();
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  zbLight.onLightChange(lightChanged);
  zbLight.setManufacturerAndModel("Remco", "XIAO-C6-WS2812B-XY-v1");

  Serial.println("Adding custom XY endpoint...");
  Zigbee.addEndpoint(&zbLight);

  Serial.println("Starting Zigbee...");
  if (!Zigbee.begin()) {
    Serial.println("ERROR: Zigbee start failed");
    delay(2000);
    ESP.restart();
  }

  Serial.println("Zigbee started. Waiting for network...");
}

void loop() {
  static bool previousConnected = false;
  bool connected = Zigbee.connected();

  if (connected != previousConnected) {
    Serial.println(connected ? "ZIGBEE CONNECTED" : "ZIGBEE DISCONNECTED");
    previousConnected = connected;
  }

  handleButton();
  delay(50);
}
