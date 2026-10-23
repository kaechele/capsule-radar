// CST816 capacitive touch over I2C (Arduino). TOUCH_DRIVER_CST816 boards only (JC3636W518).
// Protocol ported from the vendor's esp_lcd_touch_cst816s.c: read 5 bytes from register
// 0x02 -> {points, xh, xl, yh, yl} with 12-bit coordinates; chip ID at 0xA7. Same family
// as the LCD 2.1's CST820, but with the reset on a SoC GPIO and the data block at 0x02.
#include "touch.h"
#include "config.h"

#if TOUCH_DRIVER_CST816
#include <Arduino.h>
#include <Wire.h>

#define CST816_REG_DATA   0x02   // points, xh|flags, xl, yh|id, yl
#define CST816_REG_CHIPID 0xA7
#define CST816_DATA_LEN   5

static bool cst_read(uint8_t reg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission((uint8_t)I2C_ADDR_TOUCH);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;   // repeated start
    if (Wire.requestFrom((uint8_t)I2C_ADDR_TOUCH, len) < len) return false;
    for (uint8_t i = 0; i < len; ++i) buf[i] = Wire.read();
    return true;
}

bool touch_begin() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000);

    // hardware reset (the vendor driver holds each level for 200 ms)
    pinMode(PIN_TP_RST, OUTPUT);
    digitalWrite(PIN_TP_RST, LOW);
    delay(200);
    digitalWrite(PIN_TP_RST, HIGH);
    delay(200);
    pinMode(PIN_TP_INT, INPUT);

    uint8_t id = 0;
    if (cst_read(CST816_REG_CHIPID, &id, 1)) Serial.printf("[touch] CST816 chipID=0x%02X\n", id);
    else                                     Serial.println("[touch] CST816 not responding yet (will keep polling)");
    return true;
}

TouchRead touch_read(uint16_t *ox, uint16_t *oy) {
    uint8_t d[CST816_DATA_LEN];
    // The CST816S can stop answering while it sleeps between touches; a failed read is
    // "unknown" (TOUCH_NODATA), not a lift.
    if (!cst_read(CST816_REG_DATA, d, CST816_DATA_LEN)) return TOUCH_NODATA;
    if (d[0] == 0) return TOUCH_UP;

    uint16_t x = (uint16_t)(((d[1] & 0x0F) << 8) | d[2]);
    uint16_t y = (uint16_t)(((d[3] & 0x0F) << 8) | d[4]);

    if (x > SCREEN_W - 1) x = SCREEN_W - 1;
    if (y > SCREEN_H - 1) y = SCREEN_H - 1;
    if (TP_MIRROR_X) x = (SCREEN_W - 1) - x;
    if (TP_MIRROR_Y) y = (SCREEN_H - 1) - y;

    *ox = x;
    *oy = y;
    return TOUCH_DOWN;
}

#endif  // TOUCH_DRIVER_CST816
