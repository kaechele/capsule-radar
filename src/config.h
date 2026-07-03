#pragma once
// Capsule Radar — build & user configuration.
//
// This file holds tunables shared by every supported board. Screen dimensions, pin maps
// and I2C addresses are board-specific and live in board_<name>.h, selected below by the
// -DBOARD_* flag set in platformio.ini for the active [env:...].

#define FW_VERSION "1.3.17"   // shown on the web config page + Stats screen; bump on release

// ---------- Home location (default: Dénia, Spain) ----------
// Overridable at runtime via the captive portal (stored in NVS).
#define HOME_LAT_DEFAULT   38.8409
#define HOME_LON_DEFAULT    0.1059

// ---------- Radar ----------
#define RANGE_KM_DEFAULT    30.0f          // display range (outer ring). Query is wider, see ADSB_QUERY_KM
#define ADSB_QUERY_KM       50.0f          // feed query radius (> display: off-range traffic shows as edge arrows)
static const float RANGE_STEPS_KM[] = {10.0f, 20.0f, 30.0f, 50.0f, 100.0f};
#define POLL_INTERVAL_MS    2000           // be gentle with the free API (>=1000)
#define POLL_INTERVAL_BATTERY_MS 5000      // slower polling when running on battery
#define MOTION_INTERP       1              // 1 = glyphs glide between polls; 0 = snap to new pos
#define AC_STALE_MS         15000          // drop aircraft not refreshed in this long
#define TZ_STR              "CET-1CEST,M3.5.0,M10.5.0/3"  // POSIX TZ (Spain) for local time/date

// ---------- ADS-B API (free, non-commercial) ----------
#define ADSB_PRIMARY_HOST   "api.airplanes.live"   // GET /v2/point/{lat}/{lon}/{radius_nm}
#define ADSB_FALLBACK_HOST  "api.adsb.lol"          // same readsb format
#define ADSB_USER_AGENT     "CapsuleRadar/1.0 (ESP32-S3 hobby; +https://github.com/socquique/capsule-radar)"
#define ADSB_HTTPS_INSECURE 1               // 1 = setInsecure() (hobby). 0 = use pinned root CA.
#define ADSB_MAX_AIRCRAFT   60              // hard cap parsed per poll (protect RAM in busy areas)

// ---------- Debug ----------
#define DEBUG_MEM           0               // 1 = print a [mem] heap/fps line every 5s on serial

// ---------- Board-specific screen/pins/I2C ----------
// Edit pins in the board_*.h files below: replace any -1 with the value from the vendor
// factory demo (see docs/HARDWARE.md). Do NOT guess them.
#if defined(BOARD_JC3636W518)
#  include "board_jc3636w518.h"
#elif defined(BOARD_WAVESHARE_AMOLED175)
#  include "board_waveshare_amoled175.h"
#else
#  error "config.h: define -DBOARD_WAVESHARE_AMOLED175 or -DBOARD_JC3636W518 in platformio.ini"
#endif
