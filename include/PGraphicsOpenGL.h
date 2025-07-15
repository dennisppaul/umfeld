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

#include "UmfeldSDLOpenGL.h" // TODO move to cpp implementation

#include "UmfeldFunctionsAdditional.h"
#include "PGraphics.h"
#include "UFont.h"

#ifndef PGRAPHICS_OPENGL_DO_NOT_CHECK_ERRORS
#define GL_CALL(func) \
    func;             \
    checkOpenGLError(#func)
#else
#define GL_CALL(func) func;
#endif

namespace umfeld {

    // TODO @maybe move the functions below into the class above

    struct OpenGLCapabilities;
    static constexpr int OPENGL_PROFILE_NONE          = -1;
    static constexpr int OPENGL_PROFILE_CORE          = 1;
    static constexpr int OPENGL_PROFILE_COMPATIBILITY = 2;

    inline double depth_range = 10000; // TODO this should be configurable … maybe in `reset_matrices()`

    inline std::string getOpenGLErrorString(const GLenum error) {
        switch (error) {
            case GL_NO_ERROR: return "No error";
            case GL_INVALID_ENUM: return "Invalid enum (GL_INVALID_ENUM)";
            case GL_INVALID_VALUE: return "Invalid value (GL_INVALID_VALUE)";
            case GL_INVALID_OPERATION: return "Invalid operation (GL_INVALID_OPERATION)";
            case GL_OUT_OF_MEMORY: return "Out of memory (GL_OUT_OF_MEMORY)";
            case GL_INVALID_FRAMEBUFFER_OPERATION: return "Invalid framebuffer operation (GL_INVALID_FRAMEBUFFER_OPERATION)";
#ifndef OPENGL_ES_3_0
            case GL_STACK_OVERFLOW: return "Stack overflow (GL_STACK_OVERFLOW)";
            case GL_STACK_UNDERFLOW: return "Stack underflow (GL_STACK_UNDERFLOW)";
#endif
            default: return "Unknown OpenGL error";
        }
    }

    inline void checkOpenGLError(const std::string& functionName) {
#ifndef PGRAPHICS_OPENGL_DO_NOT_CHECK_ERRORS
        GLenum opengl_error;
        while ((opengl_error = glGetError()) != GL_NO_ERROR) {
            warning("[OpenGL Error] @", functionName, ": ", getOpenGLErrorString(opengl_error));
        }
#endif
    }

    static std::string fl(const std::string& text) {
        constexpr size_t column_width = DEFAULT_CONSOLE_LABEL_WIDTH;
        return format_label(text, column_width);
    }

    static void get_OpenGL_version(int& major, int& minor) {
        const auto versionStr = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        if (!versionStr) {
            major = 0;
            minor = 0;
            return;
        }

#ifdef OPENGL_ES_3_0
        const std::regex versionRegex(R"(OpenGL ES (\d+)\.(\d+))");
        std::cmatch      match;
        if (std::regex_search(versionStr, match, versionRegex) && match.size() >= 3) {
            major = std::stoi(match[1].str());
            minor = std::stoi(match[2].str());
        }
#else
        sscanf(versionStr, "%d.%d", &major, &minor);
#endif
    }

    /* opengl capabilities */

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

    inline OpenGLCapabilities open_gl_capabilities;

    void query_opengl_capabilities(OpenGLCapabilities& capabilities);

    inline void printOpenGLInfo(OpenGLCapabilities& capabilities) {
        const GLubyte* version         = glGetString(GL_VERSION);
        const GLubyte* renderer        = glGetString(GL_RENDERER);
        const GLubyte* vendor          = glGetString(GL_VENDOR);
        const GLubyte* shadingLanguage = glGetString(GL_SHADING_LANGUAGE_VERSION);
        get_OpenGL_version(capabilities.version_major, capabilities.version_minor);

        console(fl("OpenGL Version"), version, " (", capabilities.version_major, ".", capabilities.version_minor, ")");
        console(fl("Renderer"), renderer);
        console(fl("Vendor"), vendor);
        console(fl("GLSL Version"), shadingLanguage);

        std::string profile_str = "none ( pre 3.2 )";
        capabilities.profile    = OPENGL_PROFILE_NONE;

#ifdef OPENGL_3_3_CORE
        if (capabilities.version_major > 2) {
            int profile = 0;
            glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
            if (profile & GL_CONTEXT_CORE_PROFILE_BIT) {
                profile_str = "core";
                console("OpenGL Core Profile detected. Deprecated functions are not available.");
                capabilities.profile = OPENGL_PROFILE_CORE;
            }
            if (profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) {
                profile_str          = "compatibility ( legacy functions available )";
                capabilities.profile = OPENGL_PROFILE_COMPATIBILITY;
            }
        }
#endif // OPENGL_3_3_CORE
        console(fl("Profile"), profile_str);
    }

    inline void query_opengl_capabilities(OpenGLCapabilities& capabilities) {
        console(separator());
        console("OPENGL CAPABILITIES");
        console(separator());

        printOpenGLInfo(capabilities);

        console(separator(false));

        // NOTE line and point capabilities queries are not available in OpenGL ES 3.0
#ifndef OPENGL_ES_3_0
        GLfloat line_size_range[2]{};
        glGetFloatv(GL_LINE_WIDTH_RANGE, line_size_range);
        capabilities.line_size_min = line_size_range[0];
        capabilities.line_size_max = line_size_range[1];
        console(fl("line size min"), capabilities.line_size_min);
        console(fl("line size max"), capabilities.line_size_max);
        if (capabilities.line_size_min == 1.0f && capabilities.line_size_max == 1.0f) {
            console(fl("line support"), "since min and max line size is 1.0");
            console(fl(""), "it is likely that lines are not");
            console(fl(""), "properly supported.");
        }

        GLfloat line_size_granularity{0};
        glGetFloatv(GL_LINE_WIDTH_GRANULARITY, &line_size_granularity);
        console(fl("point size granularity"), line_size_granularity);
        capabilities.line_size_granularity = line_size_granularity;

        GLfloat point_size_range[2]{};
        glGetFloatv(GL_POINT_SIZE_RANGE, point_size_range);
        capabilities.point_size_min = point_size_range[0];
        capabilities.point_size_max = point_size_range[1];
        console(fl("point size min"), capabilities.point_size_min);
        console(fl("point size max"), capabilities.point_size_max);

        GLfloat point_size_granularity{0};
        glGetFloatv(GL_POINT_SIZE_GRANULARITY, &point_size_granularity);
        console(fl("point size granularity"), point_size_granularity);
        capabilities.point_size_granularity = point_size_granularity;

        console(separator());
#endif
    }

    inline GLint get_draw_mode(const int shape) {
        // TODO separate between client-side data storage ( vertices, native_opengl_shape etcetera ) and OpenGL implementation ( maybe create a vertex_buffer class )
        // TODO add shapes drawing
        int _shape = shape;
        switch (shape) {
            case TRIANGLES:
                _shape = GL_TRIANGLES;
                break;
            case TRIANGLE_STRIP:
                _shape = GL_TRIANGLE_STRIP;
                break;
            case TRIANGLE_FAN:
                _shape = GL_TRIANGLE_FAN;
                break;
            case QUADS:
#ifndef OPENGL_ES_3_0
                _shape = GL_QUADS;
#else
                warning("QUADS not supported in this OpenGL version");
#endif
                break;
            case QUAD_STRIP:
#ifdef OPENGL_2_0
                _shape = GL_QUAD_STRIP;
#else
                warning("QUAD_STRIP not supported in this OpenGL version");
#endif
                break;
            case POLYGON:
#ifdef OPENGL_2_0
                _shape = GL_POLYGON;
#else
                warning("QUAD_STRIP not supported in this OpenGL version");
#endif
                break;
            case POINTS:
                _shape = GL_POINTS;
                break;
            case LINES:
                _shape = GL_LINES;
                break;
            case LINE_STRIP:
                _shape = GL_LINE_STRIP;
                break;
            default:
                _shape = shape;
        }
        return _shape;
    }

    // TODO clean this up … move methods to implementation file
    class PGraphicsOpenGL : public PGraphics {
    public:
        ~PGraphicsOpenGL() override = default;

        void set_default_graphics_state() override {
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            blendMode(BLEND);
        }

        virtual void store_fbo_state()   = 0;
        virtual void restore_fbo_state() = 0;
        virtual void bind_fbo()          = 0;
        virtual void finish_fbo()        = 0;

        void beginDraw() override {
            PGraphics::beginDraw();
            if (render_to_offscreen) {
                // store_fbo_state(); // TODO moved to v33 + v20
                bind_fbo();
            }
            glViewport(0, 0, framebuffer.width, framebuffer.height);
        }

        void endDraw() override {
            PGraphics::endDraw();
            if (render_to_offscreen) {
                restore_fbo_state();
                finish_fbo();
            }
        }

        // Common method to bind a framebuffer texture
        void bind_framebuffer_texture() const {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, framebuffer.texture_id);
        }

        void blendMode(const int mode) override {
            glEnable(GL_BLEND);
            switch (mode) {
                case REPLACE:
                    glBlendEquation(GL_FUNC_ADD);
                    glBlendFunc(GL_ONE, GL_ZERO);
                    break;
                case BLEND:
                    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                                        GL_ONE, GL_ONE);
                    break;
                case ADD:
                    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE,
                                        GL_ONE, GL_ONE);
                    break;
                case SUBTRACT:
                    glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE,
                                        GL_ONE, GL_ONE);
                    break;
                case LIGHTEST:
                    glBlendEquationSeparate(GL_MAX, GL_FUNC_ADD);
                    glBlendFuncSeparate(GL_ONE, GL_ONE,
                                        GL_ONE, GL_ONE);
                    break;
                case DARKEST:
                    glBlendEquationSeparate(GL_MIN, GL_FUNC_ADD);
                    glBlendFuncSeparate(GL_ONE, GL_ONE,
                                        GL_ONE, GL_ONE);
                    break;
                case MULTIPLY:
                    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                    glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR,
                                        GL_ONE, GL_ONE);
                    break;
                case SCREEN:
                    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                    glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ONE,
                                        GL_ONE, GL_ONE);
                    break;
                case EXCLUSION:
                    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                    glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR,
                                        GL_ONE, GL_ONE);
                    break;
                // not possible in fixed-function blending
                case DIFFERENCE_BLEND:
                case OVERLAY:
                case HARD_LIGHT:
                case SOFT_LIGHT:
                case DODGE:
                case BURN:
                    // optionally: issue a warning here
                    glBlendEquation(GL_FUNC_ADD);
                    glBlendFunc(GL_ONE, GL_ZERO); // fallback: REPLACE
                    break;
                default:
                    // fallback: BLEND
                    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                                        GL_ONE, GL_ONE);
                    break;
            }
        }

        /* --- interface --- */

        void        init(uint32_t* pixels, int width, int height) override                                                              = 0;
        void        upload_texture(PImage* img, const uint32_t* pixel_data, int width, int height, int offset_x, int offset_y) override = 0;
        void        download_texture(PImage* img) override                                                                              = 0;
        std::string name() override                                                                                                     = 0;

        /* --- additional methods --- */

        static bool OGL_read_framebuffer(const FrameBufferObject& framebuffer, std::vector<unsigned char>& pixels) {
            const int _width  = framebuffer.width;
            const int _height = framebuffer.height;
            pixels.resize(_width * _height * DEFAULT_BYTES_PER_PIXELS);
            glPixelStorei(GL_PACK_ALIGNMENT, 4);
            glReadPixels(0, 0, _width, _height,
                         UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                         UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                         pixels.data());
            return true;
        }

        bool OGL_generate_and_upload_image_as_texture(PImage* image) {
            if (image == nullptr) {
                error_in_function("image is nullptr");
                return false;
            }

            if (image->pixels == nullptr) {
                error_in_function("pixel data is nullptr");
                return false;
            }

            if (width <= 0 || height <= 0) {
                error_in_function("invalid width or height");
                return false;
            }

            // generate texture ID
            GLuint mTextureID;
            glGenTextures(1, &mTextureID);

            if (mTextureID == 0) {
                error_in_function("texture ID generation failed");
                return false;
            }

            image->texture_id           = static_cast<int>(mTextureID);
            const int tmp_bound_texture = texture_id_current;
            IMPL_bind_texture(image->texture_id);

            // set texture parameters
            if (image->get_auto_generate_mipmap()) {
                texture_wrap(CLAMP_TO_EDGE);
                texture_filter(MIPMAP);
            } else {
                texture_wrap(CLAMP_TO_EDGE);
                texture_filter(LINEAR);
            }

            // load image data
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         UMFELD_DEFAULT_INTERNAL_PIXEL_FORMAT,
                         static_cast<GLint>(image->width),
                         static_cast<GLint>(image->height),
                         0,
                         UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                         UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                         image->pixels);

            if (image->get_auto_generate_mipmap()) {
                glGenerateMipmap(GL_TEXTURE_2D); // NOTE this works on macOS … but might not work on all platforms
            }

            IMPL_bind_texture(tmp_bound_texture);
            return true;
        }

    protected:
        UFont debug_font;
    }; // class PGraphicsOpenGL
} // namespace umfeld