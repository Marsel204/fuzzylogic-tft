# TFT freshness inspector

Light interface for a 240x320 ILI9341 panel, rotated to 320x240 landscape, with an XPT2046 resistive touchscreen. This is the concrete configuration in `14_HARDWARE_SENSOR_TFT_INTEGRATION_GUIDE.md`. The interface currently uses simulated sensor values and labels every inspection screen as DEMO.

The screens provide ingredient selection, live sensor cards, evaluation, and a result with a short diagnosis. **UJI ULANG** returns to the same ingredient's probe screen. **MENU** returns to ingredient selection. **CETAK LOG** reprints the completed inspection over Serial; it does not imply that a log file was saved on the device. Indeterminate inspections show `--` and request another measurement.

## Wiring

| Signal | ESP32-S3 GPIO |
| --- | --- |
| TFT MOSI / SDI | 11 |
| TFT SCK | 12 |
| TFT MISO / SDO | 13 |
| TFT CS | 10 |
| TFT DC / RS | 14 |
| TFT RESET | 15 |
| TFT backlight | 16 |
| XPT2046 CS | 17 |
| XPT2046 IRQ | 18 (optional; touch is polled) |
| I2C SDA / SCL | 8 / 9 |

Use the module's specified supply and a common ground; the GPIO signal levels are 3.3V. The display uses digital SPI on GPIO 11–18, while analog sensors use ADC1 on GPIO 1, 2, and 4.

## Build

For the **GUITION JC2432W328C** with the classic ESP32-D0WD-V3 and capacitive CST820/816 touch, use:

```bash
./scripts/build_tft.sh --board guition
arduino-cli upload --port /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --input-dir bin/tft-guition-ili9341 firmware/esp32s3_mbg_display
```

This board configuration follows the [GUITION pin map](../../docs/guition-jc2432w328c.md): MOSI=13, MISO=12, SCK=14, CS=15, DC=2, RESET=-1, backlights=21/27; CST I2C SDA=33, SCL=32, RESET=25, address=0x15. It uses the guide's `ILI9341_2_DRIVER`, HSPI, and active-low RGB indicators. The unused SD card is deselected on GPIO5. GPIO21 stays an output for backlight; touch is polled over I2C. Capacitive touch does **not** use XPT2046 or four-corner calibration. Add `--controller st7789` only for the alternative display controller, or `--no-touch` to disable touch.

For the separate **ESP32-S3** with the pin map above:

Install the ESP32 Arduino core and TFT_eSPI library, then run from the project directory:

```bash
./scripts/build_tft.sh
```

The script configures **both the sketch and TFT_eSPI library** with the project pins. No global Arduino library setup files need editing. Compiled output is placed in `bin/tft-ili9341`.

For the guide's alternative ST7789 240x320 panel:

```bash
./scripts/build_tft.sh --controller st7789
```

For a display without touch, add `--no-touch` and use the Serial menu at 115200 baud. The UI depends on the documented 240x320 panel size; other ST7789 sizes require a different layout and setup.

## Touch setup

On the first boot, touch the four calibration markers. Landscape calibration is saved in ESP32 NVS and reused after restart. Send `t` over Serial to calibrate again. Calibration requires the XPT2046 touch controller to be connected; build with `--no-touch` if the display has no touchscreen. Each press triggers one action, and a release is required before the next action.

Serial shortcuts: `1`–`7` select an ingredient, `f/s/z/h` choose a simulation scenario, `e` evaluates, `r` returns to the menu, `a` reprints the last receipt, and `d` toggles the demonstration cycle.

## Verify and preview

```bash
make test PYTHON=/path/to/python-with-pytest
python scripts/preview_tft.py
```

The preview uses the actual firmware drawing commands and installed TFT_eSPI bitmap fonts (Pillow required). Host tests cover touch navigation, held touches, calibration reuse, text bounds, and inspection snapshots. Firmware compilation validates the configured controller and pins; physical display and touch operation must be checked on the connected device.
