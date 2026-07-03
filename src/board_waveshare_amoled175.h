#pragma once
// Waveshare ESP32-S3-Touch-AMOLED-1.75 — screen, pin map, I2C addresses.
// Included from config.h when -DBOARD_WAVESHARE_AMOLED175 is set. Do NOT include directly.

// ---------- Screen (CO5300 AMOLED) ----------
#define SCREEN_W            466
#define SCREEN_H            466
#define SCREEN_CX           233
#define SCREEN_CY           233
#define RADAR_R_OUTER_PX    218            // outer ring radius in pixels
#define LV_COLOR_DEPTH_BITS 16
#define LCD_COL_OFFSET      6              // CO5300 column (x) gap on this panel (esp_lcd set_gap 0x06)
#define LCD_ROW_OFFSET      0              // no row (y) gap
#define LCD_QSPI_HZ         80000000       // CO5300 QSPI clock (vendor uses 40 MHz; 80 = faster, verify no artifacts)
#define BRIGHTNESS_DEFAULT  200            // 0..255, panel brightness via cmd 0x51
#define BRIGHTNESS_IDLE     25             // dimmed after no touch for IDLE_DIM_MS
#define IDLE_DIM_MS         20000          // dim the screen after this long without a touch

// ---------- Pin map ----------
// VERIFIED (ESPHome def, cross-checked against the Waveshare board definition in
// xiaozhi-esp32 and a working Arduino_GFX port for this exact panel):
#define PIN_LCD_CS          12
#define PIN_LCD_RST         39
#define PIN_TP_INT          11
#define PIN_TP_RST          40
#define TP_MIRROR_X         true
#define TP_MIRROR_Y         true

// CONFIRMED — CO5300 QSPI databus (LCD_CS=12, LCD_RST=39 above match too):
#define PIN_LCD_SCLK        38             // QSPI PCLK
#define PIN_LCD_D0          4
#define PIN_LCD_D1          5
#define PIN_LCD_D2          6
#define PIN_LCD_D3          7

// CONFIRMED — shared I2C bus (touch + IMU + RTC + PMIC + audio codec):
#define PIN_I2C_SDA         15
#define PIN_I2C_SCL         14

// CONFIRMED — ES8311 codec over I2S (M4 alert ping). MCLK/DIN/PA included for completeness:
#define PIN_I2S_MCLK        42
#define PIN_I2S_BCLK        9
#define PIN_I2S_LRCLK       45             // a.k.a. WS
#define PIN_I2S_DOUT        8              // ESP32 -> codec (speaker)
#define PIN_I2S_DIN         10             // codec -> ESP32 (mics)
#define PIN_AUDIO_PA        46             // speaker amp enable
#define PIN_BOOT_BUTTON     0              // BOOT button (held on boot = captive portal, later)

// I2C addresses:
#define I2C_ADDR_TOUCH      0x5A           // CST9217 (corrected from vendor driver; was 0x15)
#define I2C_ADDR_IMU        0x6B
#define I2C_ADDR_RTC        0x51
#define I2C_ADDR_PMIC       0x34

// Safety net: should never fire now that pins are filled in. Keeps future edits honest.
#if (PIN_LCD_SCLK < 0) || (PIN_I2C_SDA < 0)
#  error "board_waveshare_amoled175.h: QSPI/I2C pins are back to placeholders (-1). Restore the real values."
#endif
