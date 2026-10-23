#pragma once
// The UI's absolute sizes and offsets were authored against the 466x466 AMOLED panel.
// UI_SCALE(v) maps such a value onto a smaller panel in proportion to SCREEN_W, so the
// same layout fits the 360x360 JC3636W518 instead of clipping and overlapping. Geometry
// only (sizes, offsets, gaps): fonts are fixed bitmaps and are picked per board instead.
//
// Panels at least as large as the reference keep the authored pixel values unchanged
// (the 480x480 LCD 2.1 included), so this is a no-op on every board but the smaller one.
#include "config.h"
#include <lvgl.h>
#include <math.h>

#define UI_REF_W 466

#if SCREEN_W >= UI_REF_W
#  define UI_SCALE(v) ((lv_coord_t)(v))
#else
#  define UI_SCALE(v) ((lv_coord_t)lroundf((float)(v) * (float)SCREEN_W / (float)UI_REF_W))
#endif
