/*
 * Umfeld
 *
 * This file is part of the *Umfeld* library (https://github.com/dennisppaul/umfeld).
 * Copyright (c) 2025 Dennis P Paul.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define ENABLE_UMFELD_CALLBACK_LOGGING 0

#if ENABLE_UMFELD_CALLBACK_LOGGING
#define LOG_CALLBACK_MSG(msg) SDL_Log(msg)
#else
#define LOG_CALLBACK_MSG(msg) ((void) 0)
#endif

// // TODO remove WEAK define asap
// #ifndef SYSTEM_WIN32
// #ifndef WEAK
// #define WEAK __attribute__((weak))
// #endif
// #else
// #define WEAK
// #endif

#ifndef UMFELD_FUNC_WEAK
#define UMFELD_FUNC_WEAK __attribute__((weak))
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* --- DEFAULTS --- */

#define DEFAULT_WINDOW_TITLE "Umfeld"

/* --- UMFELD_WINDOW_TITLE --- */

#ifndef UMFELD_WINDOW_TITLE // can be set in `CMakeLists.txt` // TODO consider moving this to settings?
#define UMFELD_WINDOW_TITLE DEFAULT_WINDOW_TITLE
#endif

#ifndef UMFELD_PRINT_ERRORS
#define UMFELD_PRINT_ERRORS TRUE
#endif
#ifndef UMFELD_PRINT_WARNINGS
#define UMFELD_PRINT_WARNINGS TRUE
#endif
#ifndef UMFELD_PRINT_CONSOLE
#define UMFELD_PRINT_CONSOLE TRUE
#endif

/* --- TOOLS --- */

#ifndef RGBA
#define RGBA(r, g, b, a) (((uint32_t) (a) << 24) | ((uint32_t) (b) << 16) | ((uint32_t) (g) << 8) | ((uint32_t) (r)))
#endif
#ifndef HSBA
#define HSBA(h, s, b, a) ({                         \
    float _h = (h) * 360.0f, _s = (s), _b = (b);    \
    float _r, _g, _bb, _f, _p, _q, _t;              \
    int   _i = (int) (_h / 60.0f) % 6;              \
    _f       = (_h / 60.0f) - _i;                   \
    _p       = _b * (1.0f - _s);                    \
    _q       = _b * (1.0f - _f * _s);               \
    _t       = _b * (1.0f - (1.0f - _f) * _s);      \
    switch (_i) {                                   \
        case 0: _r = _b, _g = _t, _bb = _p; break;  \
        case 1: _r = _q, _g = _b, _bb = _p; break;  \
        case 2: _r = _p, _g = _b, _bb = _t; break;  \
        case 3: _r = _p, _g = _q, _bb = _b; break;  \
        case 4: _r = _t, _g = _p, _bb = _b; break;  \
        default: _r = _b, _g = _p, _bb = _q; break; \
    }                                               \
    ((uint32_t) ((uint8_t) ((a) * 255.0f) << 24) |  \
     (uint32_t) ((uint8_t) (_bb * 255.0f) << 16) |  \
     (uint32_t) ((uint8_t) (_g * 255.0f) << 8) |    \
     (uint32_t) ((uint8_t) (_r * 255.0f)));         \
})
#endif // HSBA

#define TIME_FUNCTION_MS(fn) time_function_ms([&]() { fn; })
