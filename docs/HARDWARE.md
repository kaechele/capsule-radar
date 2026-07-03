# Hardware — supported boards

Two boards are supported side by side (see `platformio.ini` for the `[env:...]` per board,
and `src/board_waveshare_amoled175.h` / `src/board_jc3636w518.h` for the pin maps that
`src/config.h` picks between via the `-DBOARD_*` build flag).

## Waveshare ESP32-S3-Touch-AMOLED-1.75

### Board
- **MCU**: ESP32-S3R8 (Xtensa LX7 dual-core @240 MHz), 512 KB SRAM, **8 MB PSRAM**, **16 MB flash**.
- **Wireless**: 2.4 GHz WiFi (b/g/n) + Bluetooth 5 (LE), onboard antenna (IPEX option).
- **Display**: 1.75" AMOLED, **466×466**, 16.7M colors, driver **CO5300** over **QSPI**. Brightness set via panel command (no PWM backlight line).
- **Touch**: **CST9217** capacitive, **I2C**. (Some units carry FT3168 — detect/abstract.)
- **IMU**: **QMI8658** 6-axis (accel + gyro), I2C.
- **RTC**: **PCF85063**, I2C (addr 0x51), kept alive by the PMIC.
- **PMIC**: **AXP2101**, I2C (addr 0x34), LiPo charge + rails.
- **Audio**: **ES8311** codec + onboard speaker (MX1.25 header); dual-mic array (ES7210 on the -C variant) with echo cancellation.
- **Storage**: microSD (TF) slot.
- **Buttons**: PWR + BOOT (programmable).
- **Expansion**: 8-pin header = 3×GPIO + 1×UART, plus reserved I2C/IO pads.
- **GPS** (only on the `-G` variant): LC76G + external ceramic antenna.

### Pin map
#### Verified (from the board's ESPHome definition)
| Signal            | GPIO | Notes                         |
|-------------------|------|-------------------------------|
| LCD CS            | 12   | CO5300, QSPI                  |
| LCD RST           | 39   | CO5300                        |
| Touch INT         | 11   | CST9217                       |
| Touch RST         | 40   | CST9217                       |
| Touch transform   | —    | mirror_x = true, mirror_y = true |

#### To confirm from the official Waveshare Arduino demo (do NOT guess)
| Signal              | GPIO | Where to find it                 |
|---------------------|------|----------------------------------|
| LCD QSPI SCLK       | ?    | demo `01_HelloWorld` (Arduino_GFX_DataBus) |
| LCD QSPI D0..D3     | ?    | demo `01_HelloWorld`             |
| I2C SDA (shared)    | ?    | any LVGL+sensor demo (`03`/`04`) |
| I2C SCL (shared)    | ?    | any LVGL+sensor demo             |
| Speaker / I2S (ES8311) | ? | demo `08_ES8311`                 |

Touch, IMU, RTC, PMIC share one I2C bus. Pull pins from the demos into `src/config.h`; the placeholders there are `-1`.

### I2C addresses (typical)
- CST9217 touch: 0x15 (verify)
- QMI8658 IMU: 0x6B (or 0x6A)
- PCF85063 RTC: 0x51
- AXP2101 PMIC: 0x34

### Reference
Waveshare wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.75
ESPHome device page (pin source): https://devices.esphome.io/devices/waveshare-esp32-s3-touch-amoled-175/

## GUITION JC3636W518

### Board
- **MCU**: ESP32-S3, dual-core @240 MHz, 512 KB SRAM, **8 MB PSRAM**, **16 MB flash** — same
  memory config as the Waveshare board above.
- **Display**: 1.8" round TFT, **360×360**, IPS, driver **ST77916** over **QSPI**. Backlight
  is PWM (LEDC) on a dedicated GPIO — unlike the Waveshare board's CO5300, there is no panel
  brightness command.
- **Touch**: **CST816** capacitive, I2C (its own dedicated bus — not shared with anything
  else on this board).
- **Audio**: TI **PCM5100APWR** DAC, I2S output only (no I2C control bus). **Not yet driven
  by firmware** — pins are mapped in `board_jc3636w518.h` but no I2S driver exists yet
  (`audio.cpp`'s ES8311 driver simply finds nothing at its I2C address and no-ops, which is
  the intended current behavior on this board).
- **No RTC, no IMU, no battery/PMIC** on this board — `rtc_begin()`/`imu_begin()`/
  `battery_begin()` already handle absent hardware gracefully (same runtime pattern used for
  the Waveshare board's optional GPS variant), so no firmware changes were needed for these.
- **Storage**: microSD (TF) slot (pins defined, not wired up in firmware).
- **Buttons**: BOOT (GPIO0 strapping pin).

### Pin map (from the vendor's `pincfg.h`)
| Signal            | GPIO | Notes                                  |
|-------------------|------|-----------------------------------------|
| LCD CS            | 10   | ST77916, QSPI                           |
| LCD RST           | 47   | ST77916                                 |
| LCD Backlight     | 15   | PWM (LEDC), not a panel command         |
| LCD QSPI SCLK     | 9    |                                          |
| LCD QSPI D0..D3   | 11, 12, 13, 14 |                                    |
| Touch INT         | 41   | CST816                                  |
| Touch RST         | 40   | CST816                                  |
| Touch I2C SDA/SCL | 7 / 8 | dedicated bus, not shared with anything else |
| I2S BCLK / LRCLK / DOUT | 18 / 16 / 17 | PCM5100 (driver not yet implemented) |
| PCM5100 XSMT (mute) | 48 | **active-LOW** mute — opposite polarity of the Waveshare board's PA-enable pin; a future audio driver must not reuse that logic verbatim |
| BOOT button       | 0    | strapping pin, same caveat as any ESP32 |

### I2C addresses
- CST816 touch: 0x15

### Reference
Vendor spec sheet, schematics and the exact ST77916 init sequence used by this firmware are
kept locally in `context/JC3636W518_LVGL_DEMO/` (git-ignored — see that folder's `NOTES.md`
for details and provenance). That folder is not part of the repository; if you're setting up
a fresh clone and need those references, they'd need to be re-sourced from the board vendor.
