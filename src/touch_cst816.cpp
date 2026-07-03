// CST816 capacitive touch over I2C (Arduino). Protocol ported from the vendor's
// esp_lcd_touch_cst816s.c (see context/JC3636W518_LVGL_DEMO/vendor_ref/ and its NOTES.md):
// read 5 bytes from reg 0x02 (num, xh|pad, xl, yh|pad, yl), chip ID at reg 0xA7.
#include "touch.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>

#define CST816_ADDR      0x15
#define CST816_REG_DATA  0x02
#define CST816_REG_CHIPID 0xA7
#define CST816_DATA_LEN  5        // num, xh|pad, xl, yh|pad, yl

// Read `len` bytes from an 8-bit register.
static bool cst_read_reg(uint8_t reg, uint8_t *data, uint8_t len) {
    Wire.beginTransmission(CST816_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((uint8_t)CST816_ADDR, len) < len) return false;
    for (uint8_t i = 0; i < len; ++i) data[i] = Wire.read();
    return true;
}

bool touch_begin() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000);

    // hardware reset (vendor reference driver uses 200 ms low / 200 ms high)
    pinMode(PIN_TP_RST, OUTPUT);
    digitalWrite(PIN_TP_RST, LOW);
    delay(200);
    digitalWrite(PIN_TP_RST, HIGH);
    delay(200);
    pinMode(PIN_TP_INT, INPUT);

    uint8_t id = 0;
    if (cst_read_reg(CST816_REG_CHIPID, &id, 1)) {
        Serial.printf("[touch] CST816 responding (id=0x%02X)\n", id);
    } else {
        Serial.println("[touch] CST816 not responding yet (will keep polling)");
    }
    return true;
}

bool touch_read(uint16_t *ox, uint16_t *oy) {
    uint8_t d[CST816_DATA_LEN];
    if (!cst_read_reg(CST816_REG_DATA, d, CST816_DATA_LEN)) return false;

    const uint8_t num = d[0];
    if (num == 0) return false;

    uint16_t x = ((uint16_t)(d[1] & 0x0F) << 8) | d[2];
    uint16_t y = ((uint16_t)(d[3] & 0x0F) << 8) | d[4];

    if (x > SCREEN_W - 1) x = SCREEN_W - 1;
    if (y > SCREEN_H - 1) y = SCREEN_H - 1;
    if (TP_MIRROR_X) x = (SCREEN_W - 1) - x;
    if (TP_MIRROR_Y) y = (SCREEN_H - 1) - y;

    *ox = x;
    *oy = y;
    return true;
}
