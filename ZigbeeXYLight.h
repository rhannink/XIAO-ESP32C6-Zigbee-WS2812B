#pragma once

#include <Arduino.h>
#include "Zigbee.h"

#if CONFIG_ZB_ENABLED

typedef void (*xyLightCallback)(bool state, uint8_t level, uint16_t x, uint16_t y);

class ZigbeeXYLight : public ZigbeeEP {
public:
  ZigbeeXYLight(uint8_t endpoint);

  void onLightChange(xyLightCallback callback);

  bool getLightState() const;
  uint8_t getLightLevel() const;
  uint16_t getLightX() const;
  uint16_t getLightY() const;

  void zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) override;

private:
  bool _state;
  uint8_t _level;
  uint16_t _x;
  uint16_t _y;
  xyLightCallback _callback;

  void lightChanged();
};

#endif
