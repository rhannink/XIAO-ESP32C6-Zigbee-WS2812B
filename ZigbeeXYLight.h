#pragma once

#include <Arduino.h>
#include "Zigbee.h"

#if CONFIG_ZB_ENABLED

#define ZIGBEE_EFFECT_CLUSTER_ID 0xFC00
#define ZIGBEE_EFFECT_ATTR_ID    0x0000
#define ZIGBEE_SPEED_ATTR_ID     0x0001

enum ZigbeeLedEffect : uint8_t {
  EFFECT_SOLID = 0,
  EFFECT_RAINBOW = 1,
  EFFECT_RAINBOW_CYCLE = 2,
  EFFECT_COLOR_WIPE = 3,
  EFFECT_THEATER_CHASE = 4,
  EFFECT_PULSE = 5
};

typedef void (*xyLightCallback)(bool state, uint8_t level, uint16_t x, uint16_t y);

class ZigbeeXYLight : public ZigbeeEP {
public:
  ZigbeeXYLight(uint8_t endpoint);

  void onLightChange(xyLightCallback callback);

  bool getLightState() const;
  uint8_t getLightLevel() const;
  uint16_t getLightX() const;
  uint16_t getLightY() const;
  uint8_t getEffect() const;
  uint8_t getEffectSpeed() const;

  void zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) override;

private:
  bool _state;
  uint8_t _level;
  uint16_t _x;
  uint16_t _y;
  uint8_t _effect;
  uint8_t _effectSpeed;
  xyLightCallback _callback;

  void lightChanged();
};

#endif
