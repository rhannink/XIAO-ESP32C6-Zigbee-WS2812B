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

uint16_t effectFrame = 0;
unsigned long lastEffectUpdate = 0;
uint8_t baseR = 255, baseG = 255, baseB = 255;

uint8_t scale8(uint8_t value, uint8_t level) {
  return (uint16_t)value * min((uint16_t)level, (uint16_t)254) / 254;
}

void setStripColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < LED_COUNT; i++) strip.setPixelColor(i, strip.Color(r, g, b));
  strip.show();
}

void clearStrip() { strip.clear(); strip.show(); }

void xyToRGB(uint16_t rawX, uint16_t rawY, uint8_t &red, uint8_t &green, uint8_t &blue) {
  float x = rawX / 65535.0f, y = rawY / 65535.0f;
  if (y <= 0.0001f) { red = green = blue = 0; return; }
  float z = 1.0f - x - y, Y = 1.0f, X = (Y / y) * x, Z = (Y / y) * z;
  float r = X * 1.656492f - Y * 0.354851f - Z * 0.255038f;
  float g = -X * 0.707196f + Y * 1.655397f + Z * 0.036152f;
  float b = X * 0.051713f - Y * 0.121364f + Z * 1.011530f;
  r = max(r, 0.0f); g = max(g, 0.0f); b = max(b, 0.0f);
  float m = max(r, max(g, b));
  if (m > 1.0f) { r /= m; g /= m; b /= m; }
  r = (r <= .0031308f) ? 12.92f*r : 1.055f*powf(r,1/2.4f)-.055f;
  g = (g <= .0031308f) ? 12.92f*g : 1.055f*powf(g,1/2.4f)-.055f;
  b = (b <= .0031308f) ? 12.92f*b : 1.055f*powf(b,1/2.4f)-.055f;
  red=constrain((int)(r*255),0,255); green=constrain((int)(g*255),0,255); blue=constrain((int)(b*255),0,255);
}

uint32_t wheel(byte pos, uint8_t level) {
  pos = 255 - pos;
  uint8_t r,g,b;
  if (pos < 85) { r=255-pos*3; g=0; b=pos*3; }
  else if (pos < 170) { pos-=85; r=0; g=pos*3; b=255-pos*3; }
  else { pos-=170; r=pos*3; g=255-pos*3; b=0; }
  return strip.Color(scale8(r,level),scale8(g,level),scale8(b,level));
}

void showSolid() {
  uint8_t level = zbLight.getLightLevel();
  setStripColor(scale8(baseR,level), scale8(baseG,level), scale8(baseB,level));
}

void lightChanged(bool state, uint8_t level, uint16_t x, uint16_t y) {
  if (!state) { clearStrip(); return; }
  xyToRGB(x,y,baseR,baseG,baseB);
  effectFrame = 0;
  if (zbLight.getEffect() == EFFECT_SOLID) showSolid();
}

uint16_t effectInterval() {
  // 1 = slow (~200 ms), 100 = fast (~15 ms)
  uint8_t speed = constrain(zbLight.getEffectSpeed(), 1, 100);
  return 202 - (speed * 187 / 100);
}

void updateEffect() {
  if (!zbLight.getLightState()) return;
  uint8_t effect = zbLight.getEffect();
  if (effect == EFFECT_SOLID) return;

  unsigned long now = millis();
  if (now - lastEffectUpdate < effectInterval()) return;
  lastEffectUpdate = now;
  uint8_t level = zbLight.getLightLevel();

  switch (effect) {
    case EFFECT_RAINBOW:
      setStripColor(
        scale8((uint8_t)(wheel(effectFrame & 255,254) >> 16),level),
        scale8((uint8_t)(wheel(effectFrame & 255,254) >> 8),level),
        scale8((uint8_t)wheel(effectFrame & 255,254),level));
      break;

    case EFFECT_RAINBOW_CYCLE:
      for (uint16_t i=0;i<LED_COUNT;i++)
        strip.setPixelColor(i,wheel(((i*256/LED_COUNT)+effectFrame)&255,level));
      strip.show();
      break;

    case EFFECT_COLOR_WIPE: {
      strip.clear();
      uint16_t upto = effectFrame % (LED_COUNT + 1);
      uint32_t c=strip.Color(scale8(baseR,level),scale8(baseG,level),scale8(baseB,level));
      for(uint16_t i=0;i<upto;i++) strip.setPixelColor(i,c);
      strip.show();
      break;
    }

    case EFFECT_THEATER_CHASE: {
      strip.clear();
      uint32_t c=strip.Color(scale8(baseR,level),scale8(baseG,level),scale8(baseB,level));
      for(uint16_t i=effectFrame%3;i<LED_COUNT;i+=3) strip.setPixelColor(i,c);
      strip.show();
      break;
    }

    case EFFECT_PULSE: {
      uint8_t phase=effectFrame&0xFF;
      uint8_t pulse=(phase<128)?phase*2:(255-phase)*2;
      uint8_t pulseLevel=(uint16_t)level*pulse/255;
      setStripColor(scale8(baseR,pulseLevel),scale8(baseG,pulseLevel),scale8(baseB,pulseLevel));
      break;
    }
  }
  effectFrame++;
}

void restoreLight() { lightChanged(zbLight.getLightState(),zbLight.getLightLevel(),zbLight.getLightX(),zbLight.getLightY()); }

void performFactoryReset() {
  setStripColor(255,0,0); delay(1000); clearStrip();
  Serial.println("Erasing Zigbee network data...");
  Zigbee.factoryReset(); delay(3000); ESP.restart();
}

void handleButton() {
  if (digitalRead(BUTTON_PIN)!=LOW) return;
  delay(30); if(digitalRead(BUTTON_PIN)!=LOW) return;
  unsigned long start=millis(),lastBlink=0; bool blink=false;
  while(digitalRead(BUTTON_PIN)==LOW) {
    unsigned long now=millis();
    if(now-lastBlink>=BLINK_TIME_MS) { lastBlink=now; blink=!blink; if(blink)setStripColor(255,0,0); else clearStrip(); }
    if(now-start>=RESET_TIME_MS) { performFactoryReset(); return; }
    delay(10);
  }
  restoreLight();
}

void setup() {
  Serial.begin(115200); delay(1000);
  strip.begin(); clearStrip(); pinMode(BUTTON_PIN,INPUT_PULLUP);
  zbLight.onLightChange(lightChanged);
  zbLight.setManufacturerAndModel("Remco","XIAO-C6-WS2812B-XY-FX-v1");
  Zigbee.addEndpoint(&zbLight);
  if(!Zigbee.begin()) { Serial.println("ERROR: Zigbee start failed"); delay(2000); ESP.restart(); }
  Serial.println("Zigbee XY light + WS2812B effects started");
}

void loop() {
  handleButton();
  updateEffect();
  delay(2);
}
