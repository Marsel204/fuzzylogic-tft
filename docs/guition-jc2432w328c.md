# GUITION JC2432W328C hardware configuration

This configuration is for the capacitive GUITION JC2432W328C with the classic ESP32-D0WD-V3, CH340 USB-UART bridge, 4 MB flash, and a 240x320 panel rotated to 320x240 landscape. The source is the board's supplied complete integration guide. Use FQBN `esp32:esp32:esp32`.

| Peripheral | Signal | GPIO |
| --- | --- | --- |
| TFT (HSPI) | MOSI | 13 |
| TFT (HSPI) | MISO | 12 |
| TFT (HSPI) | SCK | 14 |
| TFT | CS | 15 |
| TFT | DC | 2 |
| TFT | RESET | -1 (tied to system reset) |
| Backlight | Channel 1 / Channel 2 | 21 / 27, active HIGH |
| CST820/816 touch | SDA / SCL | 33 / 32 |
| CST820/816 touch | RESET | 25, active LOW |
| CST820/816 touch | I2C address | `0x15` |
| RGB LED | Red / Green / Blue | 4 / 16 / 17, active LOW |
| MicroSD | CS | 5 (held HIGH while unused) |
| MicroSD | MOSI / MISO / SCK | 13 / 12 / 14 (shared with TFT) |
| LDR | Analog input | 34 |
| Audio | DAC output | 26 |
| BOOT button | Input | 0, active LOW |

The supplied guide lists both GPIO21 for touch interrupt and for a backlight channel. The firmware keeps it as a backlight output and polls touch over I2C; it does not attach an interrupt to that pin. Touch reset pulses LOW for 20 ms then HIGH for 50 ms.

The default build uses `ILI9341_2_DRIVER`, HSPI at 55 MHz, and reads at 20 MHz. For the alternative ST7789 controller, use `./scripts/build_tft.sh --board guition --controller st7789`.

This board has **capacitive** touch, with no XPT2046 populated. Do not enable `TOUCH_CS` or the resistive four-corner calibration. Coordinates from CST registers `0x02` through `0x06` are mapped into landscape orientation; release events, out-of-range coordinates, incomplete reads, and failed I2C transactions are ignored.

Signals are 3.3V logic. GPIO2, 5, 12, and 15 are board-assigned strapping pins; preserve the onboard wiring. The ESP32-S3 GPIO map in the other integration guide must not be used on this board.
