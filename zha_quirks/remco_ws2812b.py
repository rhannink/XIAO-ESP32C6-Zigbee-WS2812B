"""ZHA quirk for Remco XIAO ESP32-C6 WS2812B XY light effects."""

import zigpy.types as t
from zigpy.quirks import CustomCluster
from zigpy.quirks.v2 import QuirkBuilder
from zigpy.zcl import foundation

EFFECT_CLUSTER_ID = 0xFC00
EFFECT_ATTR_ID = 0x0000
SPEED_ATTR_ID = 0x0001
ENDPOINT_ID = 10


class LedEffect(t.enum8):
    SOLID = 0
    RAINBOW = 1
    RAINBOW_CYCLE = 2
    COLOR_WIPE = 3
    THEATER_CHASE = 4
    PULSE = 5


class RemcoEffectCluster(CustomCluster):
    """Custom WS2812B effect cluster implemented by the ESP32-C6 firmware.

    The cluster ID itself is manufacturer-specific (0xFC00), but the firmware
    registers attributes 0x0000 and 0x0001 as normal custom-cluster attributes.
    Therefore the ZCL writes must NOT set the manufacturer-specific attribute
    flag/manufacturer code.
    """

    cluster_id = EFFECT_CLUSTER_ID
    ep_attribute = "remco_effects"

    attributes = {
        EFFECT_ATTR_ID: foundation.ZCLAttributeDef(
            id=EFFECT_ATTR_ID,
            name="effect",
            type=LedEffect,
            access=foundation.ZCLAttributeAccess.Read
            | foundation.ZCLAttributeAccess.Write,
            is_manufacturer_specific=False,
        ),
        SPEED_ATTR_ID: foundation.ZCLAttributeDef(
            id=SPEED_ATTR_ID,
            name="effect_speed",
            type=t.uint8_t,
            access=foundation.ZCLAttributeAccess.Read
            | foundation.ZCLAttributeAccess.Write,
            is_manufacturer_specific=False,
        ),
    }


(
    QuirkBuilder("Remco", "XIAO-C6-WS2812B-XY-FX-v1")
    .replaces(RemcoEffectCluster, endpoint_id=ENDPOINT_ID)
    .enum(
        attribute_name="effect",
        enum_class=LedEffect,
        cluster_id=EFFECT_CLUSTER_ID,
        endpoint_id=ENDPOINT_ID,
        translation_key="led_effect",
        fallback_name="LED effect",
    )
    .number(
        attribute_name="effect_speed",
        cluster_id=EFFECT_CLUSTER_ID,
        endpoint_id=ENDPOINT_ID,
        min_value=1,
        max_value=100,
        step=1,
        translation_key="effect_speed",
        fallback_name="Effect speed",
    )
    .add_to_registry()
)
