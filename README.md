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

- `XiaoZigbeeXYLight.ino` - main Arduino sketch and WS2812B effects
- `ZigbeeXYLight.h` - custom endpoint class
- `ZigbeeXYLight.cpp` - Zigbee clusters and attribute handling
- `zha_quirks/remco_ws2812b.py` - Home Assistant ZHA v2 custom quirk

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

## Home Assistant / ZHA pairing

The light supports On/off, Brightness and XY/RGB color. Expected Home Assistant attribute:

```yaml
supported_color_modes:
  - xy
```

There should be no `color_temp` mode.

## WS2812B effects

Firmware effects:

| Value | Effect |
|---:|---|
| 0 | Solid |
| 1 | Rainbow |
| 2 | Rainbow Cycle |
| 3 | Color Wipe |
| 4 | Theater Chase |
| 5 | Pulse |

Effect speed is 1-100.

The firmware exposes manufacturer-specific cluster `0xFC00` on endpoint 10:

- attribute `0x0000`: effect
- attribute `0x0001`: effect speed

## ZHA custom quirk

The repository contains `zha_quirks/remco_ws2812b.py`. It matches:

- Manufacturer: `Remco`
- Model: `XIAO-C6-WS2812B-XY-FX-v2`
- Endpoint: 10
- Cluster: `0xFC00`

The quirk is intended to expose an **LED effect** select entity and **Effect speed** number entity in Home Assistant.

### Install

Copy:

```text
zha_quirks/remco_ws2812b.py
```

to:

```text
/config/custom_zha_quirks/remco_ws2812b.py
```

Then add/update Home Assistant `configuration.yaml`:

```yaml
zha:
  custom_quirks_path: /config/custom_zha_quirks/
```

Restart Home Assistant completely after installing or changing the quirk.

If the device was already paired before the quirk was installed, remove it from ZHA, factory-reset the ESP32-C6 and pair it again so the device signature and entities are rebuilt.

## Factory reset

Hold the XIAO **BOOT** button for 3 seconds. The current firmware explicitly uses GPIO9 for the XIAO ESP32-C6 BOOT button.

While holding the button, the WS2812B strip flashes red. At 3 seconds it stays red and `Zigbee.factoryReset()` erases the Zigbee network data.

## License

MIT
