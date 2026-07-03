#pragma once
// GUITION JC3636W518 — screen, pin map, I2C addresses.
// Included from config.h when -DBOARD_JC3636W518 is set. Do NOT include directly.
// Pins/chip facts sourced from context/JC3636W518_LVGL_DEMO/ (pincfg.h + vendor_ref/) —
// see that folder's NOTES.md for the full derivation. Board has no RTC/IMU/battery/PMIC;
// imu_begin()/rtc_begin()/battery_begin() already no-op gracefully when those chips aren't
// found, so I2C_ADDR_IMU/RTC/PMIC below are inert placeholders, not real devices.

// ---------- Screen (ST77916, round 360x360) ----------
#define SCREEN_W            360
#define SCREEN_H            360
#define SCREEN_CX           180
#define SCREEN_CY           180
#define RADAR_R_OUTER_PX    168            // outer ring radius in pixels (scaled from the 466px board: 218/233*180)
#define LV_COLOR_DEPTH_BITS 16
#define LCD_COL_OFFSET      0              // no known gap quirk for this panel; verify on hardware
#define LCD_ROW_OFFSET      0
#define LCD_QSPI_HZ         50000000       // vendor demo's speed; raise later if stable
#define BRIGHTNESS_DEFAULT  200            // 0..255, PWM duty via PIN_LCD_BL (no panel brightness command on this chip)
#define BRIGHTNESS_IDLE     25
#define IDLE_DIM_MS         20000

// ---------- Pin map (from context/JC3636W518_LVGL_DEMO/pincfg.h) ----------
#define PIN_LCD_CS          10
#define PIN_LCD_RST         47
#define PIN_LCD_BL          15             // PWM backlight (LEDC) — no equivalent on the Waveshare board
#define PIN_TP_INT          41
#define PIN_TP_RST          40
#define TP_MIRROR_X         false          // vendor demo ships with no swap/mirror; verify on hardware
#define TP_MIRROR_Y         false

// QSPI databus:
#define PIN_LCD_SCLK        9
#define PIN_LCD_D0          11
#define PIN_LCD_D1          12
#define PIN_LCD_D2          13
#define PIN_LCD_D3          14

// Shared I2C bus (touch only — no IMU/RTC/PMIC on this board):
#define PIN_I2C_SDA         7
#define PIN_I2C_SCL         8

// PCM5100 I2S (audio driver deferred — pins defined for when it's implemented):
#define PIN_I2S_MCLK        -1             // not connected; verify DAC SCK/self-clock strapping (see NOTES.md)
#define PIN_I2S_BCLK        18
#define PIN_I2S_LRCLK       16             // a.k.a. WS
#define PIN_I2S_DOUT        17             // ESP32 -> DAC
#define PIN_I2S_DIN         -1             // no mic wired up in this driver's scope
#define PIN_AUDIO_PA        48             // PCM5100 XSMT — ACTIVE-LOW mute (opposite polarity of the
                                            // Waveshare board's active-HIGH PA-enable; a real driver must
                                            // invert the existing play_cue() HIGH/LOW logic, not reuse it as-is)
#define PIN_BOOT_BUTTON     0              // BOOT button (GPIO0 strapping pin, same caveat as any ESP32)

// I2C addresses:
#define I2C_ADDR_TOUCH      0x15           // CST816
#define I2C_ADDR_IMU        0x6B           // inert: no IMU on this board
#define I2C_ADDR_RTC        0x51           // inert: no RTC on this board
#define I2C_ADDR_PMIC       0x34           // inert: no PMIC on this board

#if (PIN_LCD_SCLK < 0) || (PIN_I2C_SDA < 0)
#  error "board_jc3636w518.h: QSPI/I2C pins are back to placeholders (-1). Restore the real values."
#endif
