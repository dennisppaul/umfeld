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

// SYSTEM

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// DEBUGGING

#define UMFELD_DEBUG_PRINT_FLUSH_SORT_BY_Z_ORDER_STATS               FALSE
#define UMFELD_DEBUG_CHECK_OPENGL_ERROR                              FALSE
#define UMFELD_DEBUG_RENDER_BATCH_WARNING_UNSUPPORTED_SHAPE_FEATURES FALSE
#define UMFELD_DEBUG_PGRAPHICS_OPENGL_3_ERRORS                       FALSE
#define UMFELD_DEBUG_VERTEX_BUFFER_DEBUG_OPENGL_ERRORS               FALSE
#define UMFELD_DEBUG_SHAPE_RENDERER_OGL_3                            FALSE
#define UMFELD_DEBUG_PIXEL_DENSITY_FRAME_BUFFER                      FALSE

// CALLBACKS

#ifndef UMFELD_FUNC_WEAK
#define UMFELD_FUNC_WEAK __attribute__((weak))
#endif

#ifndef UMFELD_SET_DEFAULT_CALLBACK
#define UMFELD_SET_DEFAULT_CALLBACK TRUE
#endif
#define ENABLE_UMFELD_CALLBACK_LOGGING FALSE

#if ENABLE_UMFELD_CALLBACK_LOGGING
#define LOG_CALLBACK_MSG(msg) warning_in_function_once(msg)
#else
#define LOG_CALLBACK_MSG(msg) ((void) 0)
#endif

/* --- CONSOLE OUTPUT --- */

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

#ifndef RGBAi
#define RGBAi(r, g, b, a) (((uint32_t) (a) << 24) | ((uint32_t) (b) << 16) | ((uint32_t) (g) << 8) | ((uint32_t) (r)))
#endif
#ifndef RGBAf
#define RGBAf(r, g, b, a) (((uint32_t) (a * 255.0f) << 24) | ((uint32_t) (b * 255.0f) << 16) | ((uint32_t) (g * 255.0f) << 8) | ((uint32_t) (r * 255.0f)))
#endif
#ifndef HSBAf
#define HSBAf(h, s, b, a) ({                        \
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
#endif // HSBAf

#define TIME_FUNCTION_MS(fn) time_function_ms([&]() { fn; })

/* --- PROFILING --- */

#ifdef ENABLE_PROFILING
#define TRACY_ALLOC
// TODO should we have includes here? maybe ok because this is usually disabled
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>
#define TRACE_SCOPE             ZoneScoped
#define TRACE_SCOPE_N(name)     ZoneScopedN(name)
#define TRACE_FRAME             FrameMark
#define TRACE_ALLOC(ptr, size)  TracyAlloc(ptr, size)
#define TRACE_FREE(ptr)         TracyFree(ptr)
#define TRACE_PLOT(name, value) TracyPlot(name, value)
#define TRACE_GPU_CONTEXT       TracyGpuContext
#define TRACE_GPU_ZONE(name)    TracyGpuZone(name)
#else
#define TRACE_SCOPE
#define TRACE_SCOPE_N(name)
#define TRACE_FRAME
#define TRACE_ALLOC(ptr, size)
#define TRACE_FREE(ptr)
#define TRACE_PLOT(name, value)
#define TRACE_GPU_CONTEXT
#define TRACE_GPU_ZONE(name)
#endif // ENABLE_PROFILING

// template for profiling blocks:
#define PROFILE_PG_OGL33
#if defined(PROFILE_PG_OGL33)
#include "UmfeldDefines.h"
#define PROFILE_PG_OGL33_SCOPE         TRACE_SCOPE
#define PROFILE_PG_OGL33_SCOPE_N(name) TRACE_SCOPE_N(name)
#else
#define PROFILE_PG_OGL33_SCOPE
#define PROFILE_PG_OGL33_SCOPE_N(name)
#endif

/* --- */
