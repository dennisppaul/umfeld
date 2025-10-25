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

#include <cstdint>
#include <limits>
#include <string>

// ReSharper disable once CppUnusedIncludeDirective
#include "UmfeldDefines.h"
#include "UmfeldVersion.h"

#include "SDL3/SDL_scancode.h"

namespace umfeld {
    typedef uint32_t color_32;
    typedef uint32_t color_t;

#ifndef UMFELD_APP_NAME
    static constexpr auto DEFAULT_WINDOW_TITLE = "Umfeld";
#else
    static constexpr auto DEFAULT_WINDOW_TITLE = UMFELD_APP_NAME;
#endif
    static constexpr int      DEFAULT                       = -1;
    static constexpr int      NOT_INITIALIZED               = -1;
    static constexpr int      DISPLAY_WIDTH                 = -1;
    static constexpr int      DISPLAY_HEIGHT                = -1;
    static constexpr int      DEFAULT_CONSOLE_WIDTH         = 78;
    static constexpr int      DEFAULT_CONSOLE_LABEL_WIDTH   = 39;
    static constexpr int      DEFAULT_WINDOW_WIDTH          = 1024;
    static constexpr int      DEFAULT_WINDOW_HEIGHT         = 768;
    static constexpr int      DEFAULT_PIXEL_FORMAT_RGBA     = 0;
    static constexpr int      DEFAULT_FRAME_RATE            = 60;
    static constexpr float    DEFAULT_CAMERA_FOV_RADIANS    = 1.04719755f; // glm::radians(60.f));
    static constexpr int      DEFAULT_AUDIO_DEVICE          = -1;
    static constexpr auto     DEFAULT_AUDIO_DEVICE_NAME     = "";
    static constexpr auto     DEFAULT_AUDIO_DEVICE_NOT_USED = "NOOP";
    static constexpr uint32_t DEFAULT_SAMPLE_RATE           = 0xFFFFFFFF;
    static constexpr uint32_t DEFAULT_AUDIO_BUFFER_SIZE     = 0xFFFFFFFF;
    static constexpr int8_t   DEFAULT_INPUT_CHANNELS        = -1;
    static constexpr int8_t   DEFAULT_OUTPUT_CHANNELS       = -1;
    static constexpr bool     DEFAULT_AUDIO_RUN_IN_THREAD   = false;
    static constexpr bool     DEFAULT_UPDATE_RUN_IN_THREAD  = false;
    static constexpr int      DEFAULT_BYTES_PER_PIXELS      = 4;
    static constexpr int      DEFAULT_SPHERE_RESOLUTION     = 15;
    static constexpr uint32_t DEFAULT_BACKGROUND_COLOR      = 0x202020FF;
    static constexpr int      AUDIO_DEVICE_FIND_BY_NAME     = -2;
    static constexpr int      AUDIO_DEVICE_NOT_FOUND        = -3;
    static constexpr int      AUDIO_UNIT_NOT_INITIALIZED    = -4;
    static constexpr int      BUFFER_SIZE_UNDEFINED         = -1;
    static constexpr uint32_t TEXTURE_PROPERTY_UNDEFINED    = -1;
    static constexpr int      TEXTURE_NOT_GENERATED         = -1;
    static constexpr int      TEXTURE_NOT_UPDATED           = -2;
    static constexpr int      TEXTURE_VALID_ID              = 1;
    static constexpr int      TEXTURE_NONE                  = 0;
    static constexpr float    PI                            = 3.14159265358979323846f;
    static constexpr float    HALF_PI                       = PI / 2;
    static constexpr float    QUARTER_PI                    = PI / 4;
    static constexpr float    TWO_PI                        = PI * 2;
    static constexpr float    TAU                           = TWO_PI;
    static constexpr bool     CLOSE                         = true;
    static constexpr bool     NOT_CLOSED                    = false;

    enum Location {
        RIGHT = SDL_SCANCODE_RIGHT,
        LEFT  = SDL_SCANCODE_LEFT,
        DOWN  = SDL_SCANCODE_DOWN,
        UP    = SDL_SCANCODE_UP,
        CENTER,
        MIDDLE,
    };

    enum ShapeMode {
        INHERIT = 0x10,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
        QUADS,
        QUAD_STRIP,
        POLYGON,
        POINTS,
        LINES,
        LINE_STRIP,
        LINE_LOOP
    };
    enum ArcMode {
        OPEN = 0x19,
        CHORD,
        PIE
    };
    enum RectEllipseMode {
        CORNER = 0x20,
        CORNERS,
        // CENTER,
        RADIUS
    };
    enum TextAlign {
        // LEFT = 0xB0, // x
        // CENTER,
        // RIGHT,
        TOP = 0xB0, // y
        BOTTOM,
        // CENTER,
        BASELINE
    };
    enum StrokeJoin {
        BEVEL = 0x30,
        MITER,
        ROUND,
        NONE,
        BEVEL_FAST,
        MITER_FAST
    };
    enum StrokeCap {
        SQUARE = 0x40,
        PROJECT,
        // ROUND,
        POINTED
    };
    enum StrokeRenderMode {
        STROKE_RENDER_MODE_TRIANGULATE_2D = 0x50,
        STROKE_RENDER_MODE_TUBE_3D,
        STROKE_RENDER_MODE_NATIVE,
        STROKE_RENDER_MODE_LINE_SHADER,
        STROKE_RENDER_MODE_BARYCENTRIC_SHADER,
        STROKE_RENDER_MODE_GEOMETRY_SHADER
    };
    enum PointRenderMode {
        POINT_RENDER_MODE_NATIVE = 0x60,
        POINT_RENDER_MODE_TRIANGULATE,
        POINT_RENDER_MODE_POINT_SHADER,
        POINT_RENDER_MODE_GEOMETRY_SHADER
    };
    enum PolygonTriangulation {
        POLYGON_TRIANGULATION_FASTER = 0x70,
        POLYGON_TRIANGULATION_BETTER,
        POLYGON_TRIANGULATION_MID
    };
    enum BlendMode {
        BLEND = 0x80,
        ADD,
        SUBTRACT,
        LIGHTEST,
        DARKEST,
        MULTIPLY,
        SCREEN,
        EXCLUSION,
        REPLACE,
        DIFFERENCE_BLEND, // not implemented
        OVERLAY,          // not implemented
        HARD_LIGHT,       // not implemented
        SOFT_LIGHT,       // not implemented
        DODGE,            // not implemented
        BURN              // not implemented
    };
    enum TextureFilter {
        NEAREST = 0xC0, // nearest neighbor
        LINEAR,         // bilinear
        MIPMAP          // mipmap with bilinear
    };
    enum TextureWrap {
        REPEAT = 0xD0,   // repeat texture
        CLAMP_TO_EDGE,   // clamp to edge
        CLAMP_TO_BORDER, // clamp to border (requires OpenGL 3.2+)
        MIRRORED_REPEAT  // mirrored repeat
    };
    enum RenderMode {
        RENDER_MODE_SORTED_BY_Z_ORDER = 0x90,   // render sorted by z-order in batches ( fast, transparent shapes are sorted, textured are rendered in batches )
        RENDER_MODE_SORTED_BY_SUBMISSION_ORDER, // render sorted by submission order ( slow, good for 2D, shapes are renderered one by one )
        RENDER_MODE_IMMEDIATELY                 // render immediately ( slowest, shapes are rendered at call to `endShape()` )
    };
    enum Hint {
        ENABLE_SMOOTH_LINES = 0xA0,
        DISABLE_SMOOTH_LINES,
        ENABLE_DEPTH_TEST,
        DISABLE_DEPTH_TEST
    };
    enum Renderer {
        RENDERER_DEFAULT = DEFAULT,          // default renderer based on platform and configuration
        OPENGL           = RENDERER_DEFAULT, // default renderer ... @deprecated in processing
        P3D              = 0xB0,             // default renderer + 3D profile
        P2D,                                 // default renderer + 2D profile
        RENDERER_OPENGL_3_3_CORE,            // core profile
        RENDERER_OPENGL_2_0,                 // fixed-function pipeline
        RENDERER_OPENGL_ES_3_0,              // iOS + Android + RPI4b+5
        RENDERER_SDL_2D,
        RENDERER_TERMINAL,
        RENDERER_TEMPLATE,
        RENDERER_CUSTOM,
    };
#if defined(OPENGL_3_3_CORE)
    inline Renderer RENDERER_OPEN_GL = RENDERER_OPENGL_3_3_CORE; // default OpenGL renderer
#elif defined(OPENGL_2_0)
    inline Renderer RENDERER_OPEN_GL = RENDERER_OPENGL_2_0; // default OpenGL renderer
#elif defined(OPENGL_ES_3_0)
    inline Renderer RENDERER_OPEN_GL = RENDERER_OPENGL_ES_3_0; // default OpenGL renderer
#endif
    enum Exporter {
        OBJ,
        PDF
    };
    enum WindingOrder {
        CW, // Clockwise
        CCW // Counter-Clockwise
    };
    enum Random {
        FAST = 0xE0,  // Linear Congruential Generator (LCG) a fast normalized random float generator using 24-bit resolution for best for speed, but less uniform distribution
        XOR_SHIFT_32, // Xorshift32 (Marsaglia) a Xorshift32-based normalized float generator with 24-bit precision variant for better distribution
        PCG,          // Permuted Congruential Generator (PCG) good quality, fast, small state. Designed by Melissa O’Neill. Very popular and widely recommended for general-purpose PRNG.
        WYRAND        // WyRand very fast (among the fastest non-cryptographic PRNGs). good statistical quality (passes PractRand up to 32 TB).
    };
    enum Profile {
        PROFILE_2D,
        PROFILE_3D
    };
    enum ShaderProgramType {
        SHADER_PROGRAM_COLOR,
        SHADER_PROGRAM_TEXTURE,
        SHADER_PROGRAM_COLOR_LIGHTS,
        SHADER_PROGRAM_TEXTURE_LIGHTS,
        SHADER_PROGRAM_POINT, // TODO implement
        SHADER_PROGRAM_LINE,
        NUM_SHADER_PROGRAMS
    };
    enum SampleAcquisitionMode {
        AUDIO_BLOCK,
        AUDIO_PER_SAMPLE_NO_INPUT,
        AUDIO_PER_SAMPLE_WITH_INPUT
    };
    enum ColorMode {
        RGB,
        RGBA,
        HSB
    };
} // namespace umfeld
