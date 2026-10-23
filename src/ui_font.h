#pragma once
// Text sizes by role, resolved to a built-in Montserrat size per board. UI_SCALE only
// covers geometry: fonts are fixed bitmaps, so a smaller panel picks smaller sizes that
// keep the same four-step hierarchy instead of collapsing two roles onto one size.
// Every size used here must be enabled in include/lv_conf.h.
//
// The _BIG variants are one step up each, used by the "Large text" setting
// (ui_set_large_text(), web-configurable).
#include "config.h"
#include <lvgl.h>

#if SCREEN_W >= 466
#  define UI_FONT_TITLE  (&lv_font_montserrat_28)   // splash title, compass N, big readouts
#  define UI_FONT_LG     (&lv_font_montserrat_16)   // card/tile titles, list rows, compass S/E/W
#  define UI_FONT_MD     (&lv_font_montserrat_14)   // body text, HUD row, buttons, plane callsigns
#  define UI_FONT_SM     (&lv_font_montserrat_12)   // captions, footer, version line, plane altitudes
#  define UI_FONT_LG_BIG (&lv_font_montserrat_20)
#  define UI_FONT_MD_BIG (&lv_font_montserrat_18)
#  define UI_FONT_SM_BIG (&lv_font_montserrat_16)
#else
#  define UI_FONT_TITLE  (&lv_font_montserrat_20)
#  define UI_FONT_LG     (&lv_font_montserrat_14)
#  define UI_FONT_MD     (&lv_font_montserrat_12)
#  define UI_FONT_SM     (&lv_font_montserrat_10)
#  define UI_FONT_LG_BIG (&lv_font_montserrat_18)
#  define UI_FONT_MD_BIG (&lv_font_montserrat_16)
#  define UI_FONT_SM_BIG (&lv_font_montserrat_14)
#endif
