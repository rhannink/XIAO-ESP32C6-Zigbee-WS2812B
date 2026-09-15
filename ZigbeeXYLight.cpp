#include "ZigbeeXYLight.h"

#if CONFIG_ZB_ENABLED

ZigbeeXYLight::ZigbeeXYLight(uint8_t endpoint) : ZigbeeEP(endpoint) {
  _device_id = ESP_ZB_HA_COLOR_DIMMABLE_LIGHT_DEVICE_ID;
  _state = false;
  _level = 254;
  _x = 20494;
  _y = 21561;
  _callback = nullptr;

  _cluster_list = esp_zb_zcl_cluster_list_create();

  esp_zb_basic_cluster_cfg_t basic_cfg = {};
  basic_cfg.zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE;
  basic_cfg.power_source = ESP_ZB_ZCL_BASIC_POWER_SOURCE_DEFAULT_VALUE;
  esp_zb_attribute_list_t *basic_cluster = esp_zb_basic_cluster_create(&basic_cfg);
  esp_zb_cluster_list_add_basic_cluster(_cluster_list, basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

  esp_zb_identify_cluster_cfg_t identify_cfg = {};
  identify_cfg.identify_time = 0;
  esp_zb_attribute_list_t *identify_cluster = esp_zb_identify_cluster_create(&identify_cfg);
  esp_zb_cluster_list_add_identify_cluster(_cluster_list, identify_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

  esp_zb_on_off_cluster_cfg_t onoff_cfg = {};
  onoff_cfg.on_off = false;
  esp_zb_attribute_list_t *onoff_cluster = esp_zb_on_off_cluster_create(&onoff_cfg);
  esp_zb_cluster_list_add_on_off_cluster(_cluster_list, onoff_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

  esp_zb_level_cluster_cfg_t level_cfg = {};
  level_cfg.current_level = 254;
  esp_zb_attribute_list_t *level_cluster = esp_zb_level_cluster_create(&level_cfg);
  esp_zb_cluster_list_add_level_cluster(_cluster_list, level_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

  // Custom Color Control cluster: XY only. No Color Temperature attributes.
  esp_zb_color_cluster_cfg_t color_cfg = {};
  color_cfg.current_x = _x;
  color_cfg.current_y = _y;
  color_cfg.color_mode = ZIGBEE_COLOR_MODE_CURRENT_X_Y;
  color_cfg.options = 0;
  color_cfg.enhanced_color_mode = ZIGBEE_COLOR_MODE_CURRENT_X_Y;
  color_cfg.color_capabilities = ZIGBEE_COLOR_CAPABILITY_X_Y;

  esp_zb_attribute_list_t *color_cluster = esp_zb_color_control_cluster_create(&color_cfg);
  esp_zb_cluster_list_add_color_control_cluster(_cluster_list, color_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

  _ep_config = {
    .endpoint = _endpoint,
    .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
    .app_device_id = ESP_ZB_HA_COLOR_DIMMABLE_LIGHT_DEVICE_ID,
    .app_device_version = 1
  };
}

void ZigbeeXYLight::onLightChange(xyLightCallback callback) {
  _callback = callback;
}

bool ZigbeeXYLight::getLightState() const { return _state; }
uint8_t ZigbeeXYLight::getLightLevel() const { return _level; }
uint16_t ZigbeeXYLight::getLightX() const { return _x; }
uint16_t ZigbeeXYLight::getLightY() const { return _y; }

void ZigbeeXYLight::lightChanged() {
  if (_callback) {
    _callback(_state, _level, _x, _y);
  }
}

void ZigbeeXYLight::zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) {
  if (!message) return;

  if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) {
    if (message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID &&
        message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL) {
      _state = *(bool *)message->attribute.data.value;
      lightChanged();
    }
    return;
  }

  if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL) {
    if (message->attribute.id == ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID &&
        message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_U8) {
      _level = *(uint8_t *)message->attribute.data.value;
      lightChanged();
    }
    return;
  }

  if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_COLOR_CONTROL) {
    if (message->attribute.id == ESP_ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_X_ID &&
        message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_U16) {
      _x = *(uint16_t *)message->attribute.data.value;
      lightChanged();
      return;
    }

    if (message->attribute.id == ESP_ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_Y_ID &&
        message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_U16) {
      _y = *(uint16_t *)message->attribute.data.value;
      lightChanged();
      return;
    }
  }
}

#endif
