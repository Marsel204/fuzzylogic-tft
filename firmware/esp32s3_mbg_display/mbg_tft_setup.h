#pragma once
// Force-include this setup for ALL C++ translation units, including TFT_eSPI.
// scripts/build_tft.sh supplies the required compiler flag without editing the library.
#ifndef MBG_TFT_GUIDE_SETUP
#error "Build this sketch with scripts/build_tft.sh (project TFT pin configuration)."
#endif
#define USER_SETUP_LOADED
#if defined(MBG_TFT_ST7789)
#define ST7789_DRIVER
#elif defined(MBG_TFT_GUITION)
#define ILI9341_2_DRIVER
#else
#define ILI9341_DRIVER
#endif
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#if defined(MBG_TFT_GUITION)
// GUITION JC2432W328C: shared display/SD SPI and capacitive I2C touch.
#define USE_HSPI_PORT
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_MISO 12
#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST -1
#define TFT_BL 21
#define TFT_BACKLIGHT_ON HIGH
#define SPI_FREQUENCY 55000000
#define SPI_READ_FREQUENCY 20000000
#else
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_MISO 13
#define TFT_CS 10
#define TFT_DC 14
#define TFT_RST 15
#define TFT_BL 16
#ifndef MBG_TFT_NO_TOUCH
#define TOUCH_CS 17
#define MBG_TOUCH_IRQ 18
#endif
#define SPI_FREQUENCY 40000000
#define SPI_READ_FREQUENCY 16000000
#define SPI_TOUCH_FREQUENCY 2500000
#endif
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
