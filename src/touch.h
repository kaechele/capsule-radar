#pragma once
// Common touch API shared by every board's touch driver (touch_cst9217.cpp /
// touch_cst816.cpp). Only one implementation is compiled per env (see
// platformio.ini's build_src_filter) — callers just include this header and never
// the board-specific one, so display.cpp stays board-agnostic.
#include <stdint.h>

bool touch_begin();                       // I2C + hardware reset; logs comms status
bool touch_read(uint16_t *x, uint16_t *y); // true if currently pressed (x,y in screen px)
