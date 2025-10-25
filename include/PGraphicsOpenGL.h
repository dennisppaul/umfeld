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

#include <regex>
#include <string>

#include "PGraphics.h"

namespace umfeld {

#if UMFELD_DEBUG_CHECK_OPENGL_ERROR
#define CHECK_OPENGL_ERROR_FUNC(func) \
    func;                             \
    PGraphicsOpenGL::OGL_check_error(#func)
#else
#define CHECK_OPENGL_ERROR_FUNC(func) func;
#endif

#if UMFELD_DEBUG_CHECK_OPENGL_ERROR
#define CHECK_OPENGL_ERROR_BLOCK(NAME, CODE)                                         \
    do {                                                                             \
        bool   __ogl_err_found = false;                                              \
        GLenum __ogl_err;                                                            \
        while ((__ogl_err = glGetError()) != GL_NO_ERROR) {                          \
            warning("[OpenGL Error BEFORE] @", __func__, ": ",                       \
                    NAME, " -> ", PGraphicsOpenGL::OGL_get_error_string(__ogl_err)); \
            __ogl_err_found = true;                                                  \
        }                                                                            \
        CODE while ((__ogl_err = glGetError()) != GL_NO_ERROR) {                     \
            if (!__ogl_err_found)                                                    \
                warning("--> " NAME);                                                \
            warning("[OpenGL Error AFTER] @", __func__, ": ",                        \
                    NAME, " -> ", PGraphicsOpenGL::OGL_get_error_string(__ogl_err)); \
            __ogl_err_found = true;                                                  \
        }                                                                            \
        if (__ogl_err_found)                                                         \
            warning("<-- " NAME);                                                    \
    } while (0)
#else
#define CHECK_OPENGL_ERROR_BLOCK(NAME, CODE) \
    do { CODE } while (0)
#endif

    class UFont;

    class PGraphicsOpenGL : public PGraphics {
    public:
        struct OpenGLCapabilities {
            int   version_major{0};
            int   version_minor{0};
            int   profile{0};
            float line_size_min{0};
            float line_size_max{0};
            float line_size_granularity{0};
            float point_size_min{0};
            float point_size_max{0};
            float point_size_granularity{0};
        };

        ~PGraphicsOpenGL() override = default;

        void set_default_graphics_state() override;

        virtual void store_fbo_state()   = 0;
        virtual void restore_fbo_state() = 0;
        virtual void bind_fbo()          = 0;
        virtual void finish_fbo()        = 0;

        /* --- interface --- */

        void        init(uint32_t* pixels, int width, int height) override                                                              = 0;
        void        upload_texture(PImage* img, const uint32_t* pixel_data, int width, int height, int offset_x, int offset_y) override = 0;
        void        download_texture(PImage* img) override                                                                              = 0;
        std::string name() override                                                                                                     = 0;

        /* --- extended functionality --- */

        void background_f(const float a, const float b, const float c, const float d) override;
        void beginDraw() override;
        void endDraw() override;
        void bind_framebuffer_texture() const;
        void blendMode(BlendMode mode) override;
        void texture_filter(TextureFilter filter) override;
        void texture_wrap(TextureWrap wrap, glm::vec4 color_stroke) override;
        int  texture_update_and_bind(PImage* img) override;

        /* --- OpenGL specific methods --- */

        static void        OGL_bind_texture(int texture_id);
        static bool        OGL_read_framebuffer(const FrameBufferObject& framebuffer, std::vector<unsigned char>& pixels);
        static bool        OGL_generate_and_upload_image_as_texture(PImage* image);
        static void        OGL_texture_filter(TextureFilter filter);
        static void        OGL_texture_wrap(TextureWrap wrap, glm::vec4 color_stroke);
        static void        OGL_check_error(const std::string& functionName);
        static void        OGL_query_capabilities(OpenGLCapabilities& capabilities);
        static uint32_t    OGL_get_draw_mode(int shape);
        static void        OGL_get_version(int& major, int& minor);
        static std::string OGL_get_error_string(uint32_t error);
        static void        OGL_print_info(OpenGLCapabilities& capabilities);
        static void        OGL_enable_depth_testing();
        static void        OGL_disable_depth_testing();
        static void        OGL_enable_depth_buffer_writing();
        static void        OGL_disable_depth_buffer_writing();
        static uint32_t    OGL_get_uniform_location(const uint32_t& id, const char* uniform_name);
        static bool        OGL_evaluate_shader_uniforms(const std::string& shader_name, const ShaderUniforms& uniforms);

    protected:
        static constexpr int DEFAULT_ACTIVE_TEXTURE_UNIT  = 0;
        static constexpr int OPENGL_PROFILE_NONE          = -1;
        static constexpr int OPENGL_PROFILE_CORE          = 1;
        static constexpr int OPENGL_PROFILE_COMPATIBILITY = 2;
        double               depth_range                  = 10000; // TODO this should be configurable … maybe in `reset_matrices()`
        UFont                debug_font;
    }; // class PGraphicsOpenGL
} // namespace umfeld