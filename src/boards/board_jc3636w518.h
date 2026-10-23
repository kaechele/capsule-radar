#pragma once
// Board: GUITION JC3636W518.
// ESP32-S3 (8 MB PSRAM / 16 MB flash), ST77916 360x360 round TFT (QSPI) with a PWM
// backlight, CST816 touch on its own I2C bus, PCM5100 I2S DAC. No PMIC, RTC or IMU.
//
// Pins come from GUITION's LVGL demo for this board (pincfg.h) and its schematics, and
// were confirmed on hardware. imu_begin() / rtc_begin() find nothing on the touch bus
// and stay off, so I2C_ADDR_IMU / I2C_ADDR_RTC below only satisfy the shared code.

#define BOARD_NAME          "GUITION JC3636W518"
#define BOARD_PIO_ENV       "esp32-s3-jc3636w518"   // used in the OTA hint printed at boot

// ---------- Screen (ST77916 TFT, 360x360 round) ----------
#define SCREEN_W            360
#define SCREEN_H            360
#define SCREEN_CX           180
#define SCREEN_CY           180
#define RADAR_R_OUTER_PX    168            // outer ring radius in pixels (218/233 of the 466 layout)

// ---------- Panel ----------
#define PANEL_ST77916       1              // display.cpp: Arduino_ST77916 + st77916_init.h
#define PIN_LCD_CS          10
#define PIN_LCD_RST         47
#define PIN_LCD_SCLK        9              // QSPI PCLK
#define PIN_LCD_D0          11
#define PIN_LCD_D1          12
#define PIN_LCD_D2          13
#define PIN_LCD_D3          14
#define LCD_COL_OFFSET      0
#define LCD_ROW_OFFSET      0
#define LCD_QSPI_HZ         80000000       // the vendor demo runs 50 MHz; 80 verified on hardware

// ---------- Backlight (PWM; the ST77916 has no brightness command) ----------
#define PIN_LCD_BL          15
#define LCD_BL_PWM_FREQ     5000
#define LCD_BL_PWM_BITS     8

// ---------- Touch (dedicated I2C bus, nothing else on it) ----------
#define PIN_I2C_SDA         7
#define PIN_I2C_SCL         8

#define TOUCH_DRIVER_CST816 1
#define I2C_ADDR_TOUCH      0x15
#define PIN_TP_INT          41
#define PIN_TP_RST          40
#define TP_MIRROR_X         false
#define TP_MIRROR_Y         false

// ---------- Peripherals ----------
#define BOARD_HAS_PMIC      0
#define BOARD_HAS_AUDIO     0              // PCM5100 DAC present, driver not written yet (see below)
#define I2C_ADDR_IMU        0x6B           // no IMU on this board; probed, never answers
#define I2C_ADDR_RTC        0x51           // no RTC on this board; probed, never answers

// PCM5100 over I2S, for when an audio driver exists. Its XSMT mute line is ACTIVE LOW,
// the opposite of the Waveshare boards' amp enable, so audio.cpp's PIN_AUDIO_PA
// HIGH/LOW logic cannot be reused as it is.
#define PIN_I2S_MCLK        -1             // the vendor demo leaves MCLK undriven; check R38 before relying on that
#define PIN_I2S_BCLK        18
#define PIN_I2S_LRCLK       16             // a.k.a. WS
#define PIN_I2S_DOUT        17             // ESP32 -> DAC
#define PIN_I2S_DIN         -1
#define PIN_AUDIO_PA        48             // PCM5100 XSMT (active low)

#define PIN_BOOT_BUTTON     0              // BOOT button
