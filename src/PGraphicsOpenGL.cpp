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

#include "PGraphicsOpenGL.h"

#include "UmfeldSDLOpenGL.h"

#include "Umfeld.h"
#include "UmfeldFunctionsGraphics.h"
#include "PGraphicsOpenGLConstants.h"
#include "UShapeRenderer.h"

namespace umfeld {
    void PGraphicsOpenGL::background_f(const float a, const float b, const float c, const float d) {
        PGraphics::background_f(a, b, c, d);

        GLboolean previous_depth_mask;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &previous_depth_mask);
        if (!previous_depth_mask) {
            glDepthMask(GL_TRUE);
        }

        glClearColor(a, b, c, d);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!previous_depth_mask) {
            glDepthMask(previous_depth_mask);
        }
    }

    void PGraphicsOpenGL::beginDraw() {
        PGraphics::beginDraw();
        if (render_to_offscreen) {
            bind_fbo();
        }
        glViewport(0, 0, framebuffer.width, framebuffer.height);
    }

    void PGraphicsOpenGL::endDraw() {
        PGraphics::endDraw();
        if (render_to_offscreen) {
            restore_fbo_state();
            finish_fbo();
        }
    }

    void PGraphicsOpenGL::set_default_graphics_state() {
        background(DEFAULT_BACKGROUND_COLOR);
        blendMode(BLEND);
    }

    void PGraphicsOpenGL::bind_framebuffer_texture() const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer.texture_id);
    }

    void PGraphicsOpenGL::blendMode(const BlendMode mode) {
        PGraphics::blendMode(mode);
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

    void PGraphicsOpenGL::OGL_bind_texture(const int texture_id) {
        glActiveTexture(GL_TEXTURE0 + DEFAULT_ACTIVE_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        // OPTIMIZE by also passing the current texture ID i.e `texture_id_current`
        // if (texture_id != texture_id_current) {
        //     glActiveTexture(GL_TEXTURE0 + DEFAULT_ACTIVE_TEXTURE_UNIT);
        //     glBindTexture(GL_TEXTURE_2D, texture_id);
        //     glBindTexture(GL_TEXTURE_2D, texture_id_current);
        // }
    }

    bool PGraphicsOpenGL::OGL_read_framebuffer(const FrameBufferObject& framebuffer, std::vector<unsigned char>& pixels) {
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

    bool PGraphicsOpenGL::OGL_generate_and_upload_image_as_texture(PImage* image) {
        if (image == nullptr) {
            error_in_function("image is nullptr");
            return false;
        }

        if (image->pixels == nullptr) {
            error_in_function("pixel data is nullptr");
            return false;
        }

        if (image->width <= 0 || image->height <= 0) {
            error_in_function("invalid width or height");
            return false;
        }

        // generate texture ID + bin texture
        GLuint mTextureID;
        glGenTextures(1, &mTextureID);

        if (mTextureID == 0) {
            error_in_function("texture ID generation failed");
            return false;
        }
        image->texture_id = static_cast<int>(mTextureID);
        OGL_bind_texture(image->texture_id);

        // set texture parameters
        if (image->get_auto_generate_mipmap()) {
            OGL_texture_wrap(CLAMP_TO_EDGE, glm::vec4(0, 0, 0, 0));
            OGL_texture_filter(MIPMAP);
        } else {
            OGL_texture_wrap(CLAMP_TO_EDGE, glm::vec4(0, 0, 0, 0));
            OGL_texture_filter(LINEAR);
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

        return true;
    }

    void PGraphicsOpenGL::OGL_texture_filter(const TextureFilter filter) {
        switch (filter) {
            case NEAREST:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                break;
            case LINEAR:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                break;
            case MIPMAP:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                break;
            default:
                error_in_function("unknown texture filter type");
                break;
        }
    }

    void PGraphicsOpenGL::OGL_texture_wrap(const TextureWrap wrap, const glm::vec4 color_stroke) {
        switch (wrap) {
            case REPEAT:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                break;
            case CLAMP_TO_EDGE:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                break;
            case MIRRORED_REPEAT:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                break;
            case CLAMP_TO_BORDER:
#ifndef OPENGL_ES_3_0
                // NOTE this is not supported in OpenGL ES 3.0
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                    const float borderColor[] = {color_stroke.r, color_stroke.g, color_stroke.b, color_stroke.a};
                    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
                }
#endif
                break;
            default:
                error_in_function("unknown texture wrap type");
                break;
        }
    }

    /**
     * handles initial generation of texture and upload of pixel data to GPU.
     * also updates filter and wrap settings if required.
     * @param img image to update and bind as texture.
     * @return texture ID associated with the image, or TEXTURE_NONE if the image is null or texture generation failed.
     */
    int PGraphicsOpenGL::texture_update_and_bind(PImage* img) {
        if (img == nullptr) {
            OGL_bind_texture(TEXTURE_NONE);
            return TEXTURE_NONE;
        }
        /* upload and bind texture */
        if (img->texture_id == TEXTURE_NOT_GENERATED) {
            const bool success = OGL_generate_and_upload_image_as_texture(img);
            if (!success || img->texture_id == TEXTURE_NOT_GENERATED) {
                error_in_function("cannot create texture from image.");
                OGL_bind_texture(TEXTURE_NONE);
                return TEXTURE_NONE;
            }
        } else {
            OGL_bind_texture(img->texture_id);
        }
        /* filter */
        if (img->is_texture_filter_dirty()) {
            img->set_texture_filter_clean();
            OGL_texture_filter(img->get_texture_filter());
        }
        /* wrap */
        if (img->is_texture_wrap_dirty()) {
            img->set_texture_wrap_clean();
            OGL_texture_wrap(img->get_texture_wrap(), static_cast<glm::vec4>(color_stroke));
        }
        return img->texture_id;
    }

    void PGraphicsOpenGL::texture_filter(const TextureFilter filter) {
        if (current_texture != nullptr) {
            texture_update_and_bind(current_texture);
            current_texture->set_texture_filter(filter);
            OGL_texture_filter(filter);
            current_texture->set_texture_filter_clean();
        }
    }

    void PGraphicsOpenGL::texture_wrap(const TextureWrap wrap, const glm::vec4 color_stroke) {
        if (current_texture != nullptr) {
            texture_update_and_bind(current_texture);
            current_texture->set_texture_wrap(wrap);
            OGL_texture_wrap(wrap, color_stroke);
            current_texture->set_texture_wrap_clean();
        }
    }

    std::string PGraphicsOpenGL::OGL_get_error_string(const GLenum error) {
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

    void PGraphicsOpenGL::OGL_check_error(const std::string& functionName) {
#if UMFELD_DEBUG_CHECK_OPENGL_ERROR
        GLenum opengl_error;
        while ((opengl_error = glGetError()) != GL_NO_ERROR) {
            warning("[OpenGL Error] @", functionName, ": ", OGL_get_error_string(opengl_error));
        }
#endif
    }

    void PGraphicsOpenGL::OGL_get_version(int& major, int& minor) {
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

    void PGraphicsOpenGL::OGL_print_info(OpenGLCapabilities& capabilities) {
        const GLubyte* version         = glGetString(GL_VERSION);
        const GLubyte* renderer        = glGetString(GL_RENDERER);
        const GLubyte* vendor          = glGetString(GL_VENDOR);
        const GLubyte* shadingLanguage = glGetString(GL_SHADING_LANGUAGE_VERSION);
        OGL_get_version(capabilities.version_major, capabilities.version_minor);

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
                profile_str          = "core";
                capabilities.profile = OPENGL_PROFILE_CORE;
            }
            if (profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) {
                profile_str          = "compatibility ( legacy functions available )";
                capabilities.profile = OPENGL_PROFILE_COMPATIBILITY;
            }
        }
#endif // OPENGL_3_3_CORE
        console(fl("Profile"), profile_str);
        if (capabilities.profile == OPENGL_PROFILE_CORE) {
            console(fl(""), "OpenGL Core Profile detected.");
            console(fl(""), "Deprecated functions are not available.");
        }
    }

    void PGraphicsOpenGL::OGL_query_capabilities(OpenGLCapabilities& capabilities) {
        console(separator());
        console("OPENGL CAPABILITIES");
        console(separator());

        OGL_print_info(capabilities);

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
            console(fl(""), "lines support is probably only rudimentary.");
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

    uint32_t PGraphicsOpenGL::OGL_get_draw_mode(const int shape) {
        // NOTE primitives supported in OpenGL ES 3.0:
        //      GL_POINTS - Individual points
        //      GL_LINES - Pairs of vertices forming line segments
        //      GL_LINE_STRIP - Connected line segments
        //      GL_LINE_LOOP - Connected line segments forming a closed loop
        //      GL_TRIANGLES - Groups of 3 vertices forming triangles
        //      GL_TRIANGLE_STRIP - Connected triangles sharing vertices
        //      GL_TRIANGLE_FAN - Triangles sharing a common vertex
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
            case LINE_LOOP:
                _shape = GL_LINE_LOOP;
                break;
            default:
                _shape = shape;
        }
        return _shape;
    }

    void PGraphicsOpenGL::OGL_enable_depth_testing() {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL); // allow equal depths to pass ( `GL_LESS` is default )
    }

    void PGraphicsOpenGL::OGL_disable_depth_testing() {
        glDisable(GL_DEPTH_TEST);
    }

    void PGraphicsOpenGL::OGL_enable_depth_buffer_writing() {
        glDepthMask(GL_TRUE);
    }

    void PGraphicsOpenGL::OGL_disable_depth_buffer_writing() {
        glDepthMask(GL_FALSE);
    }

    uint32_t PGraphicsOpenGL::OGL_get_uniform_location(const uint32_t& id, const char* uniform_name) {
        const uint32_t location = glGetUniformLocation(id, uniform_name);
        if (location == GL_INVALID_INDEX) {
            return ShaderUniforms::NOT_FOUND;
        }
        return location;
    }

    bool PGraphicsOpenGL::OGL_evaluate_shader_uniforms(const std::string& shader_name, const ShaderUniforms& uniforms) {
        bool valid = true;
        if (uniforms.u_model_matrix.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform '", uniforms.u_model_matrix.name, "' not found");
            valid = false;
        }
        if (uniforms.u_view_matrix.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'u_view_matrix' not found");
            valid = false;
        }
        if (uniforms.u_projection_matrix.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'u_projection_matrix' not found");
            valid = false;
        }
        if (uniforms.u_view_projection_matrix.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform '", uniforms.u_view_projection_matrix.name, "' not found");
            valid = false;
        }
        /* texture shader */
        if (uniforms.u_texture_unit.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'u_texture_unit' not found");
            valid = false;
        }
        /* lighting shader */
        // TODO consider "normalMatrix as Transform" ... see model_matrix
        // if (uniforms.normalMatrix == ShaderUniforms::NOT_FOUND) {
        //     warning(shader_name, ": uniform 'normalMatrix' not found");
        //     valid = false;
        // }
        if (uniforms.ambient.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'ambient' not found");
            valid = false;
        }
        if (uniforms.specular.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'specular' not found");
            valid = false;
        }
        if (uniforms.emissive.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'emissive' not found");
            valid = false;
        }
        if (uniforms.shininess.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'shininess' not found");
            valid = false;
        }
        if (uniforms.lightCount.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'lightCount' not found");
            valid = false;
        }
        if (uniforms.lightPosition.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'lightPosition' not found");
            valid = false;
        }
        if (uniforms.lightNormal.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'lightNormal' not found");
            valid = false;
        }
        if (uniforms.lightAmbient.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'lightAmbient' not found");
            valid = false;
        }
        if (uniforms.lightDiffuse.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'lightDiffuse' not found");
            valid = false;
        }
        if (uniforms.lightSpecular.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'lightSpecular' not found");
            valid = false;
        }
        if (uniforms.lightFalloff.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'lightFalloff' not found");
            valid = false;
        }
        if (uniforms.lightSpot.id == ShaderUniforms::NOT_FOUND) {
            warning(shader_name, ": uniform 'lightSpot' not found");
            valid = false;
        }
        return valid;
    }
} // namespace umfeld
