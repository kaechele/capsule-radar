#pragma once
// Scales a pixel value authored against the original 466x466 mockup/layout down (or up)
// proportionally to the active board's SCREEN_W. Layout geometry only (sizes, offsets,
// radii) — NOT font sizes: LVGL fonts are fixed bitmap sizes and can't be scaled by a
// multiply. A smaller screen may need a smaller pre-built font chosen explicitly instead.
#include "config.h"
#include <lvgl.h>
#include <math.h>

#define UI_SCALE(v) ((lv_coord_t)lroundf((v) * (float)SCREEN_W / 466.0f))
