// GUITION JC3636W518 bring-up: ST77916 360x360 round panel via Arduino_GFX (QSPI) + LVGL.
// Pins come from config.h (board_jc3636w518.h). Structured like display.cpp (the
// Waveshare board's driver) — same LVGL plumbing and rotation handling, different panel
// bus/init/backlight. See touch.h for the shared touch API this file drives LVGL's
// indev from (touch_cst816.cpp provides the implementation for this board).
#include "display.h"
#include "config.h"
#include "radar_view.h"
#include "ui.h"
#include "touch.h"

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include <esp_heap_caps.h>

// --- ST77916 init table -------------------------------------------------------
// Translated from context/JC3636W518_LVGL_DEMO/vendor_ref/lcd_init_cmd_st77916.txt (the
// vendor's own working demo init sequence for THIS exact panel) into Arduino_GFX's
// operation-stream format. Do NOT swap this for Arduino_ST77916's built-in default
// tables (st77916_150_init_operations / st77916_180_init_operations) — neither matches
// this panel's register values (checked byte-by-byte against the vendor table).
// The final three commands (0x21 INVON, 0x11 SLPOUT, 0x29 DISPON) are emitted as
// WRITE_COMMAND_8 with no data phase, matching how Arduino_ST77916's own built-in tables
// handle these same three commands, even though the vendor's raw dump includes a
// (likely spurious) trailing 0x00 data byte for each.
static const uint8_t st77916_jc3636w518_init_ops[] = {
    BEGIN_WRITE,
    WRITE_C8_D8, 0xF0, 0x28,
    WRITE_C8_D8, 0xF2, 0x28,
    WRITE_C8_D8, 0x73, 0xF0,
    WRITE_C8_D8, 0x7C, 0xD1,
    WRITE_C8_D8, 0x83, 0xE0,
    WRITE_C8_D8, 0x84, 0x61,
    WRITE_C8_D8, 0xF2, 0x82,
    WRITE_C8_D8, 0xF0, 0x00,
    WRITE_C8_D8, 0xF0, 0x01,
    WRITE_C8_D8, 0xF1, 0x01,
    WRITE_C8_D8, 0xB0, 0x56,
    WRITE_C8_D8, 0xB1, 0x4D,
    WRITE_C8_D8, 0xB2, 0x24,
    WRITE_C8_D8, 0xB4, 0x87,
    WRITE_C8_D8, 0xB5, 0x44,
    WRITE_C8_D8, 0xB6, 0x8B,
    WRITE_C8_D8, 0xB7, 0x40,
    WRITE_C8_D8, 0xB8, 0x86,
    WRITE_C8_D8, 0xBA, 0x00,
    WRITE_C8_D8, 0xBB, 0x08,
    WRITE_C8_D8, 0xBC, 0x08,
    WRITE_C8_D8, 0xBD, 0x00,
    WRITE_C8_D8, 0xC0, 0x80,
    WRITE_C8_D8, 0xC1, 0x10,
    WRITE_C8_D8, 0xC2, 0x37,
    WRITE_C8_D8, 0xC3, 0x80,
    WRITE_C8_D8, 0xC4, 0x10,
    WRITE_C8_D8, 0xC5, 0x37,
    WRITE_C8_D8, 0xC6, 0xA9,
    WRITE_C8_D8, 0xC7, 0x41,
    WRITE_C8_D8, 0xC8, 0x01,
    WRITE_C8_D8, 0xC9, 0xA9,
    WRITE_C8_D8, 0xCA, 0x41,
    WRITE_C8_D8, 0xCB, 0x01,
    WRITE_C8_D8, 0xD0, 0x91,
    WRITE_C8_D8, 0xD1, 0x68,
    WRITE_C8_D8, 0xD2, 0x68,
    WRITE_C8_D16, 0xF5, 0x00, 0xA5,
    WRITE_C8_D8, 0xDD, 0x4F,
    WRITE_C8_D8, 0xDE, 0x4F,
    WRITE_C8_D8, 0xF1, 0x10,
    WRITE_C8_D8, 0xF0, 0x00,
    WRITE_C8_D8, 0xF0, 0x02,
    WRITE_COMMAND_8, 0xE0,
    WRITE_BYTES, 14,
    0xF0, 0x0A, 0x10, 0x09, 0x09, 0x36, 0x35, 0x33,
    0x4A, 0x29, 0x15, 0x15, 0x2E, 0x34,
    WRITE_COMMAND_8, 0xE1,
    WRITE_BYTES, 14,
    0xF0, 0x0A, 0x0F, 0x08, 0x08, 0x05, 0x34, 0x33,
    0x4A, 0x39, 0x15, 0x15, 0x2D, 0x33,
    WRITE_C8_D8, 0xF0, 0x10,
    WRITE_C8_D8, 0xF3, 0x10,
    WRITE_C8_D8, 0xE0, 0x07,
    WRITE_C8_D8, 0xE1, 0x00,
    WRITE_C8_D8, 0xE2, 0x00,
    WRITE_C8_D8, 0xE3, 0x00,
    WRITE_C8_D8, 0xE4, 0xE0,
    WRITE_C8_D8, 0xE5, 0x06,
    WRITE_C8_D8, 0xE6, 0x21,
    WRITE_C8_D8, 0xE7, 0x01,
    WRITE_C8_D8, 0xE8, 0x05,
    WRITE_C8_D8, 0xE9, 0x02,
    WRITE_C8_D8, 0xEA, 0xDA,
    WRITE_C8_D8, 0xEB, 0x00,
    WRITE_C8_D8, 0xEC, 0x00,
    WRITE_C8_D8, 0xED, 0x0F,
    WRITE_C8_D8, 0xEE, 0x00,
    WRITE_C8_D8, 0xEF, 0x00,
    WRITE_C8_D8, 0xF8, 0x00,
    WRITE_C8_D8, 0xF9, 0x00,
    WRITE_C8_D8, 0xFA, 0x00,
    WRITE_C8_D8, 0xFB, 0x00,
    WRITE_C8_D8, 0xFC, 0x00,
    WRITE_C8_D8, 0xFD, 0x00,
    WRITE_C8_D8, 0xFE, 0x00,
    WRITE_C8_D8, 0xFF, 0x00,
    WRITE_C8_D8, 0x60, 0x40,
    WRITE_C8_D8, 0x61, 0x04,
    WRITE_C8_D8, 0x62, 0x00,
    WRITE_C8_D8, 0x63, 0x42,
    WRITE_C8_D8, 0x64, 0xD9,
    WRITE_C8_D8, 0x65, 0x00,
    WRITE_C8_D8, 0x66, 0x00,
    WRITE_C8_D8, 0x67, 0x00,
    WRITE_C8_D8, 0x68, 0x00,
    WRITE_C8_D8, 0x69, 0x00,
    WRITE_C8_D8, 0x6A, 0x00,
    WRITE_C8_D8, 0x6B, 0x00,
    WRITE_C8_D8, 0x70, 0x40,
    WRITE_C8_D8, 0x71, 0x03,
    WRITE_C8_D8, 0x72, 0x00,
    WRITE_C8_D8, 0x73, 0x42,
    WRITE_C8_D8, 0x74, 0xD8,
    WRITE_C8_D8, 0x75, 0x00,
    WRITE_C8_D8, 0x76, 0x00,
    WRITE_C8_D8, 0x77, 0x00,
    WRITE_C8_D8, 0x78, 0x00,
    WRITE_C8_D8, 0x79, 0x00,
    WRITE_C8_D8, 0x7A, 0x00,
    WRITE_C8_D8, 0x7B, 0x00,
    WRITE_C8_D8, 0x80, 0x48,
    WRITE_C8_D8, 0x81, 0x00,
    WRITE_C8_D8, 0x82, 0x06,
    WRITE_C8_D8, 0x83, 0x02,
    WRITE_C8_D8, 0x84, 0xD6,
    WRITE_C8_D8, 0x85, 0x04,
    WRITE_C8_D8, 0x86, 0x00,
    WRITE_C8_D8, 0x87, 0x00,
    WRITE_C8_D8, 0x88, 0x48,
    WRITE_C8_D8, 0x89, 0x00,
    WRITE_C8_D8, 0x8A, 0x08,
    WRITE_C8_D8, 0x8B, 0x02,
    WRITE_C8_D8, 0x8C, 0xD8,
    WRITE_C8_D8, 0x8D, 0x04,
    WRITE_C8_D8, 0x8E, 0x00,
    WRITE_C8_D8, 0x8F, 0x00,
    WRITE_C8_D8, 0x90, 0x48,
    WRITE_C8_D8, 0x91, 0x00,
    WRITE_C8_D8, 0x92, 0x0A,
    WRITE_C8_D8, 0x93, 0x02,
    WRITE_C8_D8, 0x94, 0xDA,
    WRITE_C8_D8, 0x95, 0x04,
    WRITE_C8_D8, 0x96, 0x00,
    WRITE_C8_D8, 0x97, 0x00,
    WRITE_C8_D8, 0x98, 0x48,
    WRITE_C8_D8, 0x99, 0x00,
    WRITE_C8_D8, 0x9A, 0x0C,
    WRITE_C8_D8, 0x9B, 0x02,
    WRITE_C8_D8, 0x9C, 0xDC,
    WRITE_C8_D8, 0x9D, 0x04,
    WRITE_C8_D8, 0x9E, 0x00,
    WRITE_C8_D8, 0x9F, 0x00,
    WRITE_C8_D8, 0xA0, 0x48,
    WRITE_C8_D8, 0xA1, 0x00,
    WRITE_C8_D8, 0xA2, 0x05,
    WRITE_C8_D8, 0xA3, 0x02,
    WRITE_C8_D8, 0xA4, 0xD5,
    WRITE_C8_D8, 0xA5, 0x04,
    WRITE_C8_D8, 0xA6, 0x00,
    WRITE_C8_D8, 0xA7, 0x00,
    WRITE_C8_D8, 0xA8, 0x48,
    WRITE_C8_D8, 0xA9, 0x00,
    WRITE_C8_D8, 0xAA, 0x07,
    WRITE_C8_D8, 0xAB, 0x02,
    WRITE_C8_D8, 0xAC, 0xD7,
    WRITE_C8_D8, 0xAD, 0x04,
    WRITE_C8_D8, 0xAE, 0x00,
    WRITE_C8_D8, 0xAF, 0x00,
    WRITE_C8_D8, 0xB0, 0x48,
    WRITE_C8_D8, 0xB1, 0x00,
    WRITE_C8_D8, 0xB2, 0x09,
    WRITE_C8_D8, 0xB3, 0x02,
    WRITE_C8_D8, 0xB4, 0xD9,
    WRITE_C8_D8, 0xB5, 0x04,
    WRITE_C8_D8, 0xB6, 0x00,
    WRITE_C8_D8, 0xB7, 0x00,
    WRITE_C8_D8, 0xB8, 0x48,
    WRITE_C8_D8, 0xB9, 0x00,
    WRITE_C8_D8, 0xBA, 0x0B,
    WRITE_C8_D8, 0xBB, 0x02,
    WRITE_C8_D8, 0xBC, 0xDB,
    WRITE_C8_D8, 0xBD, 0x04,
    WRITE_C8_D8, 0xBE, 0x00,
    WRITE_C8_D8, 0xBF, 0x00,
    WRITE_C8_D8, 0xC0, 0x10,
    WRITE_C8_D8, 0xC1, 0x47,
    WRITE_C8_D8, 0xC2, 0x56,
    WRITE_C8_D8, 0xC3, 0x65,
    WRITE_C8_D8, 0xC4, 0x74,
    WRITE_C8_D8, 0xC5, 0x88,
    WRITE_C8_D8, 0xC6, 0x99,
    WRITE_C8_D8, 0xC7, 0x01,
    WRITE_C8_D8, 0xC8, 0xBB,
    WRITE_C8_D8, 0xC9, 0xAA,
    WRITE_C8_D8, 0xD0, 0x10,
    WRITE_C8_D8, 0xD1, 0x47,
    WRITE_C8_D8, 0xD2, 0x56,
    WRITE_C8_D8, 0xD3, 0x65,
    WRITE_C8_D8, 0xD4, 0x74,
    WRITE_C8_D8, 0xD5, 0x88,
    WRITE_C8_D8, 0xD6, 0x99,
    WRITE_C8_D8, 0xD7, 0x01,
    WRITE_C8_D8, 0xD8, 0xBB,
    WRITE_C8_D8, 0xD9, 0xAA,
    WRITE_C8_D8, 0xF3, 0x01,
    WRITE_C8_D8, 0xF0, 0x00,
    WRITE_C8_D8, 0x3A, 0x55,   // COLMOD: 16-bit/pixel (RGB565). Missing from the vendor's raw
                               // dump but present in Arduino_ST77916's own built-in tables right
                               // here — without it the panel stays in its default ~18-bit mode
                               // ("RGB 262K color" per the datasheet) while we stream RGB565,
                               // which desyncs the pixel byte-boundaries (sheared/duplicated
                               // pixels, garbled colors).
    WRITE_COMMAND_8, 0x21,
    WRITE_COMMAND_8, 0x11,
    END_WRITE,
    DELAY, 120,
    BEGIN_WRITE,
    WRITE_COMMAND_8, 0x29,
    END_WRITE,
};

// --- Arduino_GFX panel -------------------------------------------------------
static Arduino_DataBus *s_bus = nullptr;
static Arduino_ST77916 *s_gfx = nullptr;

// --- LVGL plumbing -----------------------------------------------------------
#define LVGL_BUF_LINES 40    // partial draw-buffer height (lines); kept in fast internal RAM
static lv_disp_draw_buf_t s_draw_buf;
static lv_disp_drv_t      s_disp_drv;
static lv_indev_drv_t     s_indev_drv;
static lv_color_t        *s_buf1 = nullptr;
static lv_color_t        *s_buf2 = nullptr;

static volatile uint32_t s_frameCount = 0;   // rendered frames (last-flush), for FPS measurement
uint32_t display_frames() { return s_frameCount; }

static volatile uint8_t s_rot = 0;           // display rotation: 0/1/2/3 = 0°/90°/180°/270°
static lv_color_t *s_rotBuf = nullptr;       // PSRAM scratch for 90/270° transpose (see begin())

// LVGL -> panel, applying the chosen rotation while pushing. Same approach as the
// Waveshare board's display.cpp (parametric on SCREEN_W/H, so it ports unchanged).
static void flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *px) {
    const int w = (int)(area->x2 - area->x1 + 1);
    const int h = (int)(area->y2 - area->y1 + 1);
    lv_color_t *out = px;
    int16_t  dx = area->x1, dy = area->y1;
    uint16_t dw = (uint16_t)w, dh = (uint16_t)h;

    switch (s_rot) {
        case 2:  // 180°
            for (int i = 0, j = w * h - 1; i < j; ++i, --j) { lv_color_t t = px[i]; px[i] = px[j]; px[j] = t; }
            dx = (int16_t)(SCREEN_W - 1 - area->x2);
            dy = (int16_t)(SCREEN_H - 1 - area->y2);
            break;
        case 1:  // 90° CW
            if (s_rotBuf) {
                for (int j = 0; j < h; ++j)
                    for (int i = 0; i < w; ++i)
                        s_rotBuf[i * h + (h - 1 - j)] = px[j * w + i];
                out = s_rotBuf; dw = (uint16_t)h; dh = (uint16_t)w;
                dx = (int16_t)(SCREEN_H - 1 - area->y2); dy = area->x1;
            }
            break;
        case 3:  // 270° CW
            if (s_rotBuf) {
                for (int j = 0; j < h; ++j)
                    for (int i = 0; i < w; ++i)
                        s_rotBuf[(w - 1 - i) * h + j] = px[j * w + i];
                out = s_rotBuf; dw = (uint16_t)h; dh = (uint16_t)w;
                dx = area->y1; dy = (int16_t)(SCREEN_W - 1 - area->x2);
            }
            break;
        default: break;  // 0°
    }
#if (LV_COLOR_16_SWAP != 0)
    s_gfx->draw16bitBeRGBBitmap(dx, dy, (uint16_t *)out, dw, dh);
#else
    s_gfx->draw16bitRGBBitmap(dx, dy, (uint16_t *)out, dw, dh);
#endif
    if (lv_disp_flush_is_last(drv)) s_frameCount++;
    lv_disp_flush_ready(drv);
}

// QSPI panels commonly require 2-pixel-aligned flush windows (even start, odd end) — the
// CO5300 (Waveshare board) needs this and ships without it produced sheared/duplicated
// pixels on partial-area updates. Applying the same safeguard here defensively.
static void rounder_cb(lv_disp_drv_t *drv, lv_area_t *area) {
    (void)drv;
    area->x1 &= ~1;
    area->y1 &= ~1;
    area->x2 |= 1;
    area->y2 |= 1;
}

// CST816 touch -> LVGL pointer. LVGL keeps the last point on release.
static void touch_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    (void)drv;
    uint16_t x, y;
    if (touch_read(&x, &y)) {
        uint16_t lx = x, ly = y;                          // map physical touch -> logical (inverse rotation)
        switch (s_rot) {
            case 1: lx = y;                            ly = (uint16_t)(SCREEN_H - 1 - x); break;
            case 2: lx = (uint16_t)(SCREEN_W - 1 - x); ly = (uint16_t)(SCREEN_H - 1 - y); break;
            case 3: lx = (uint16_t)(SCREEN_W - 1 - y); ly = x;                            break;
            default: break;
        }
        data->point.x = (lv_coord_t)lx;
        data->point.y = (lv_coord_t)ly;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

namespace display {

bool begin() {
    Serial.println("[display] init ST77916 QSPI...");
    s_bus = new Arduino_ESP32QSPI(PIN_LCD_CS, PIN_LCD_SCLK,
                                  PIN_LCD_D0, PIN_LCD_D1, PIN_LCD_D2, PIN_LCD_D3);
    s_gfx = new Arduino_ST77916(s_bus, PIN_LCD_RST, 0 /*rotation*/, true /*ips*/,
                                SCREEN_W, SCREEN_H,
                                LCD_COL_OFFSET, LCD_ROW_OFFSET, 0, 0,
                                st77916_jc3636w518_init_ops, sizeof(st77916_jc3636w518_init_ops));
    if (!s_gfx->begin(LCD_QSPI_HZ)) {
        Serial.println("[display] gfx->begin() FAILED");
        return false;
    }
    s_gfx->fillScreen(RGB565_BLACK);

    // No panel brightness command on this chip (unlike CO5300) — backlight is PWM (LEDC).
    ledcAttach(PIN_LCD_BL, 5000, 8);   // 5 kHz, 8-bit duty
    ledcWrite(PIN_LCD_BL, BRIGHTNESS_DEFAULT);
    Serial.println("[display] panel up; init LVGL...");

    lv_init();

    // Draw scratch in INTERNAL DMA RAM, same rationale as the Waveshare board's driver:
    // anti-aliased rendering into PSRAM is slow, and PSRAM is needed elsewhere (TLS).
    const size_t buf_px = (size_t)SCREEN_W * LVGL_BUF_LINES;
    s_buf1 = (lv_color_t *)heap_caps_malloc(buf_px * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    s_buf2 = nullptr;
    if (!s_buf1) {
        Serial.println("[display] internal draw buffer failed; falling back to PSRAM");
        s_buf1 = (lv_color_t *)heap_caps_malloc(buf_px * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    }
    lv_disp_draw_buf_init(&s_draw_buf, s_buf1, s_buf2, buf_px);

    // Scratch target for 90/270° rotation (deliberately PSRAM; NULL is fine, rotation just
    // falls back to un-rotated for 90/270° if PSRAM is exhausted).
    s_rotBuf = (lv_color_t *)heap_caps_malloc(buf_px * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);

    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res  = SCREEN_W;
    s_disp_drv.ver_res  = SCREEN_H;
    s_disp_drv.flush_cb = flush_cb;
    s_disp_drv.rounder_cb = rounder_cb;     // 2px-aligned flush windows (see rounder_cb above)
    s_disp_drv.draw_buf = &s_draw_buf;
    lv_disp_drv_register(&s_disp_drv);

    // Touch input (CST816) -> LVGL pointer indev (drives tap-to-inspect + swipe).
    if (touch_begin()) {
        lv_indev_drv_init(&s_indev_drv);
        s_indev_drv.type = LV_INDEV_TYPE_POINTER;
        s_indev_drv.read_cb = touch_read_cb;
        lv_indev_drv_register(&s_indev_drv);
        Serial.println("[display] CST816 touch registered");
    }

    Serial.printf("[display] PSRAM free: %u KB\n", (unsigned)(ESP.getFreePsram() / 1024));
    ui_create();
    Serial.println("[display] LVGL ready");
    return true;
}

void loop() { lv_timer_handler(); }

void setBrightness(uint8_t v) { ledcWrite(PIN_LCD_BL, v); }

void setRotation(uint8_t quarters) {
    s_rot = (uint8_t)(quarters & 3);   // 0..3 = 0°/90°/180°/270°
    lv_obj_t *scr = lv_scr_act();
    if (scr) lv_obj_invalidate(scr);   // full repaint in the new orientation
}
uint8_t rotation() { return s_rot; }

uint32_t inactiveMs() { return lv_disp_get_inactive_time(NULL); }

} // namespace display
