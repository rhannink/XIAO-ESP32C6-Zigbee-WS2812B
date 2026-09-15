"""ZHA quirk for Remco XIAO ESP32-C6 WS2812B XY light effects."""

import enum

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
    """Manufacturer-specific WS2812B effect cluster."""

    cluster_id = EFFECT_CLUSTER_ID
    ep_attribute = "remco_effects"

    attributes = {
        EFFECT_ATTR_ID: foundation.ZCLAttributeDef(
            id=EFFECT_ATTR_ID,
            name="effect",
            type=LedEffect,
            access=foundation.ZCLAttributeAccess.Read
            | foundation.ZCLAttributeAccess.Write,
            is_manufacturer_specific=True,
        ),
        SPEED_ATTR_ID: foundation.ZCLAttributeDef(
            id=SPEED_ATTR_ID,
            name="effect_speed",
            type=t.uint8_t,
            access=foundation.ZCLAttributeAccess.Read
            | foundation.ZCLAttributeAccess.Write,
            is_manufacturer_specific=True,
        ),
    }


(
    QuirkBuilder("Remco", "XIAO-C6-WS2812B-XY-FX-v2")
    .replaces(RemcoEffectCluster, endpoint_id=ENDPOINT_ID)
    .enum(
        attribute_name="effect",
        enum_class=LedEffect,
        cluster_id=EFFECT_CLUSTER_ID,
        endpoint_id=ENDPOINT_ID,
        fallback_name="LED effect",
    )
    .number(
        attribute_name="effect_speed",
        cluster_id=EFFECT_CLUSTER_ID,
        endpoint_id=ENDPOINT_ID,
        min_value=1,
        max_value=100,
        step=1,
        fallback_name="Effect speed",
    )
    .add_to_registry()
)
