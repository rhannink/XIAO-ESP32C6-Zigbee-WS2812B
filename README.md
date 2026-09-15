# XIAO ESP32-C6 Zigbee WS2812B

Arduino project for a Seeed Studio XIAO ESP32-C6 controlling a 16-pixel WS2812B LED strip and exposing it to Home Assistant/ZHA as a Zigbee XY color light.

## Why a custom Zigbee endpoint?

Arduino-ESP32 3.3.11 `ZigbeeColorDimmableLight` exposes Color Temperature attributes even for an RGB/XY-only lamp. In ZHA this can result in an invalid `min_color_temp_kelvin: 15` and the error:

```
Failed to convert color_temp_mireds=66666 ... to zigpy.types.basic.uint16_t
```

This project uses a custom `ZigbeeEP` endpoint with On/Off, Level Control and XY Color Control, but no Color Temperature attributes. Home Assistant should therefore expose `supported_color_modes: xy`.

## Hardware

- Seeed Studio XIAO ESP32-C6
- 16 x WS2812B LEDs
- WS2812B DIN connected to GPIO 2
- External 5 V supply recommended for the LED strip
- Common ground between XIAO and LED power supply
- Recommended: 330 ohm resistor in series with DIN
- Recommended: 470-1000 uF capacitor across strip 5 V/GND

## Software

Tested target configuration:

- Arduino IDE
- esp32 by Espressif Systems: **3.3.11**
- Adafruit NeoPixel library
- Board: XIAO ESP32C6
- Zigbee Mode: **Zigbee ED (End Device)**
- Zigbee-compatible partition scheme

## Files

- `XiaoZigbeeXYLight.ino` - main Arduino sketch and WS2812B driver
- `ZigbeeXYLight.h` - custom endpoint class
- `ZigbeeXYLight.cpp` - Zigbee clusters and attribute handling

## Wiring

```
XIAO ESP32-C6               WS2812B

GPIO 2 ---- 330R ---------- DIN
GND ----------------------- GND

External 5V --------------- +5V
External GND -------------- GND
              |
              +------------ XIAO GND
```

Do not power a full LED strip directly from a GPIO pin.

## Home Assistant / ZHA pairing

1. Flash the sketch.
2. In Home Assistant open ZHA and select **Add device**.
3. Power/restart the XIAO.
4. Wait for `ZIGBEE CONNECTED` in Serial Monitor at 115200 baud.

The light should support:

- On/off
- Brightness
- XY/RGB color

Expected Home Assistant attribute:

```yaml
supported_color_modes:
  - xy
```

There should be no `color_temp` mode.

## Factory reset

Hold the XIAO **BOOT** button for 3 seconds.

While holding the button, the WS2812B strip flashes red. After 3 seconds it stays red briefly and the Zigbee network data is erased with `Zigbee.factoryReset()`.

If the button is released before 3 seconds, the reset is cancelled and the previous light state is restored.

## Re-pairing after endpoint changes

ZHA caches the endpoint and cluster information discovered during pairing. After changing the Zigbee endpoint configuration:

1. Remove the old device from ZHA.
2. Flash the new firmware.
3. Perform the 3-second BOOT factory reset.
4. Pair the device again as a new ZHA device.

## License

MIT
