# Hardware

Capsule Radar supports two Waveshare round-AMOLED boards, which share the SoC and the
466×466 CO5300 panel, and the GUITION JC3636W518, the same SoC with a smaller 360×360
ST77916 TFT. Their **pin maps have nothing in common** — flashing one board's image onto
another gives a black screen and a silent I2C bus. (The experimental Waveshare LCD 2.1 is
described in the README.)

Pin maps live one-per-board in [`../src/boards/`](../src/boards/) and are selected by a
build flag (see [`../src/config.h`](../src/config.h)):

| Board | PlatformIO env | Build flag | Header |
|---|---|---|---|
| ESP32-S3-Touch-AMOLED-1.75 (reference) | `esp32-s3-amoled-175` | *(default)* | `board_amoled_175.h` |
| ESP32-S3-Touch-AMOLED-1.43 | `esp32-s3-amoled-143` | `-DBOARD_AMOLED_143` | `board_amoled_143.h` |
| GUITION JC3636W518 | `esp32-s3-jc3636w518` | `-DBOARD_JC3636W518` | `board_jc3636w518.h` |

---

## ESP32-S3-Touch-AMOLED-1.75 (reference board)

- **MCU**: ESP32-S3R8 (Xtensa LX7 dual-core @240 MHz), 512 KB SRAM, **8 MB PSRAM**, **16 MB flash**.
- **Wireless**: 2.4 GHz WiFi (b/g/n) + Bluetooth 5 (LE), onboard antenna (IPEX option).
- **Display**: 1.75" AMOLED, **466×466**, driver **CO5300** over **QSPI**. Brightness via panel command (no PWM backlight line).
- **Touch**: **CST9217** capacitive, I2C.
- **IMU**: **QMI8658** 6-axis, I2C. **RTC**: **PCF85063**, I2C.
- **PMIC**: **AXP2101**, I2C — LiPo charge + rails.
- **Audio**: **ES8311** codec + onboard speaker; dual-mic array (ES7210 on the -C variant).
- **Storage**: microSD (TF). **Buttons**: PWR + BOOT. **GPS** (`-G` variant only): LC76G.

| Signal | GPIO | | Signal | GPIO |
|---|---|---|---|---|
| LCD CS | 12 | | I2C SDA | 15 |
| LCD RST | 39 | | I2C SCL | 14 |
| LCD QSPI SCLK | 38 | | I2S MCLK | 42 |
| LCD QSPI D0–D3 | 4, 5, 6, 7 | | I2S BCLK | 9 |
| Touch INT | 11 | | I2S LRCLK | 45 |
| Touch RST | 40 | | I2S DOUT / DIN | 8 / 10 |
| Touch transform | mirror_x + mirror_y | | Speaker amp enable | 46 |

Panel column gap **6**, row gap 0, QSPI clock 80 MHz.
I2C: CST9217 `0x5A`, QMI8658 `0x6B`, PCF85063 `0x51`, AXP2101 `0x34`, ES8311 `0x18`.

Sources: the board's ESPHome definition, the Waveshare board definition in xiaozhi-esp32,
and a working Arduino_GFX port for this panel.

---

## ESP32-S3-Touch-AMOLED-1.43

Same 466×466 CO5300 panel, 8 MB PSRAM / 16 MB flash. Differences that matter:

- **Touch is an FT3168**, not a CST9217, at a different address and with a different protocol.
- **No AXP2101 and no ES8311** — no battery reporting and no alert pings. Both compile out
  via `BOARD_HAS_PMIC` / `BOARD_HAS_AUDIO`. This is not merely cosmetic: the 1.75's I2S
  BCLK/DIN (GPIO 9/10) are *this* board's QSPI CS and SCLK, so leaving audio enabled here
  would drive the panel's own bus lines.
- The FT3168 has **no reset line of its own**; it is tied to the panel reset (GPIO 21) and
  does not appear on I2C at all until the display has been initialised. `display::begin()`
  therefore must keep `gfx->begin()` ahead of `touch_begin()`.
- IMU and RTC are the same parts at the same addresses as the 1.75.

| Signal | GPIO | | Signal | GPIO |
|---|---|---|---|---|
| LCD CS | 9 | | I2C SDA | 47 |
| LCD RST | 21 | | I2C SCL | 48 |
| LCD QSPI SCLK | 10 | | Touch INT / RST | not broken out |
| LCD QSPI D0–D3 | 11, 12, 13, 14 | | Touch transform | none (raw coords match) |
| Battery ADC | 4 (unused — see below) | | IMU INT / RTC INT | 8 / 15 |

Panel column gap **6** (same as the 1.75), row gap 0, QSPI clock 40 MHz (the vendor rate;
80 MHz is untested here).
I2C: FT3168 `0x38` (chip-id register `0xA3` reads `0x64`), QMI8658 `0x6B`, PCF85063 `0x51`.

Battery: there is a divided battery voltage on GPIO 4, but the divider ratio is not
documented, so the firmware reports "no battery" rather than an invented percentage.

**Sources.** Pins come from the Arduino-ESP32 core's own board variant
(`variants/waveshare_esp32_s3_touch_amoled_143/pins_arduino.h`, shared with the 1.64), and
each was then confirmed on hardware: an I2C scan on 47/48 answers `0x51` + `0x6B` + `0x38`,
and the panel drives correctly on CS=9 / SCK=10 / D0–D3=11–14 / RST=21. The touch orientation
was verified with an on-screen dot tracking a fingertip, and the column gap by nudging a
full-panel rim-ring test card one pixel at a time until it centred.

> Do **not** "correct" these against the Waveshare wiki pin table: that table is misaligned
> and lists an I2C SCL of GPIO 49, which does not exist on an ESP32-S3.

---

## GUITION JC3636W518

Same ESP32-S3 with 8 MB PSRAM / 16 MB flash, but a different panel, touch controller and
peripheral set:

- **Display**: 1.8" round IPS TFT, **360×360**, driver **ST77916** over **QSPI** — the same
  Arduino_GFX bus as the CO5300, so `display.cpp` drives it (`PANEL_ST77916`), with the
  vendor's init sequence in `src/st77916_init.h`. That sequence needed one addition: COLMOD
  `0x3A = 0x55` (RGB565). Without it the panel stays in its 18-bit default and the picture
  shears. The ST77916 has **no brightness command**: the backlight is PWM on GPIO 15.
- **Touch**: **CST816** capacitive at `0x15`, on its **own I2C bus** (nothing else on it).
- **Audio**: TI **PCM5100** I2S DAC, no control bus. **No driver yet** (`BOARD_HAS_AUDIO` 0),
  so no alert pings. Its XSMT mute line (GPIO 48) is **active low**, the opposite of the
  Waveshare boards' amp enable, so `audio.cpp` cannot reuse its `PIN_AUDIO_PA` logic as is.
- **No PMIC, RTC or IMU**: no battery readout, no face-down sleep; time comes from NTP.
- **Storage**: microSD slot (not used by the firmware). **Buttons**: BOOT.

| Signal | GPIO | | Signal | GPIO |
|---|---|---|---|---|
| LCD CS | 10 | | Touch I2C SDA / SCL | 7 / 8 |
| LCD RST | 47 | | Touch INT / RST | 41 / 40 |
| LCD QSPI SCLK | 9 | | I2S BCLK / LRCLK / DOUT | 18 / 16 / 17 |
| LCD QSPI D0–D3 | 11, 12, 13, 14 | | PCM5100 XSMT (mute) | 48, active low |
| LCD backlight (PWM) | 15 | | Touch transform | none |

Panel gap 0 / 0, QSPI clock 80 MHz (the vendor demo runs 50 MHz).

The UI was laid out for 466 px. On this panel `UI_SCALE()` (`src/ui_scale.h`) scales the
layout geometry and `src/ui_font.h` picks smaller fonts; `pio run -e native-jc3636w518 -t exec`
shows the result in the simulator.

**Sources.** GUITION's LVGL demo for this board (`pincfg.h`, the ST77916 init dump and the
CST816S driver) and its schematics; pins confirmed on hardware.

---

## Adding another board

1. Copy a header in `src/boards/`, fill in the pins **from the vendor demo or the Arduino
   core board variant** — never guess them.
2. Add an `[env:…]` in `platformio.ini` that `extends = amoled_common` and sets `-DBOARD_…`.
3. Set `BOARD_HAS_PMIC` / `BOARD_HAS_AUDIO` / `TOUCH_DRIVER_…` for what is actually fitted;
   add a touch driver alongside `touch_cst9217.cpp` / `touch_ft3168.cpp` /
   `touch_cst816.cpp` if needed. Define the panel geometry (`SCREEN_W`/`SCREEN_H`/
   `SCREEN_CX`/`SCREEN_CY`/`RADAR_R_OUTER_PX`) in the board header; a panel smaller than
   466 px gets the UI scaled down automatically.
4. Confirm on hardware before committing: I2C scan, a full-panel test pattern for the gap,
   and a touch dot that lands under your fingertip.
5. Add the env to `.github/workflows/release.yml` so it ships with every release.

## Reference
- 1.75 wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.75
- 1.43 wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43
- ESPHome device page (1.75 pin source): https://devices.esphome.io/devices/waveshare-esp32-s3-touch-amoled-175/
