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

#if defined(OPENGL_3_3_CORE) || defined(OPENGL_ES_3_0)

#include <vector>

#include "UmfeldSDLOpenGL.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Umfeld.h"
#include "PGraphicsOpenGLConstants.h"
#include "PGraphicsOpenGL.h"
#include "PGraphicsOpenGL_3.h"
#include "Vertex.h"
#include "Geometry.h"
#include "VertexBuffer.h"
#include "PShader.h"
#include "ShaderSourceColor.h"
#include "ShaderSourceColorLights.h"
#include "ShaderSourceFullscreen.h"
#include "ShaderSourceLine.h"
#include "ShaderSourcePoint.h"
#include "ShaderSourceTexture.h"
#include "ShaderSourceTextureLights.h"
#include "UShapeRendererOpenGL_3.h"

#if UMFELD_DEBUG_PGRAPHICS_OPENGL_3_ERRORS
#define UMFELD_PGRAPHICS_OPENGL_3_CHECK_ERRORS(msg) \
    do {                                            \
        PGraphicsOpenGL::OGL_check_error(msg);      \
    } while (0)
#else
#define UMFELD_PGRAPHICS_OPENGL_3_CHECK_ERRORS(msg)
#endif

using namespace umfeld;

PGraphicsOpenGL_3::PGraphicsOpenGL_3(const bool render_to_offscreen) : PImage(0, 0) {
    this->render_to_offscreen = render_to_offscreen;
    PGraphicsOpenGL::blendMode(BLEND); // NOTE set blend mode once
}

void PGraphicsOpenGL_3::OGL3_add_line_quad(const Vertex& p0, const Vertex& p1, float thickness, std::vector<Vertex>& out) {
#define OPTIMIZE_LINE_QUAD
#ifdef OPTIMIZE_LINE_QUAD
    // glm::vec3 dir = glm::normalize(p1 - p0);
    glm::vec3 dir = p1.position - p0.position; // NOTE no need to normalize, the shader will do it

    // The shader will use this direction to compute screen-space offset
    glm::aligned_vec4 normal(dir, thickness);
    // glm::aligned_vec4 color(0.5f, 0.85f, 1.0f, 1.0f); // or pass in if needed

    // glm::aligned_vec4 pos0(p0.position);
    // glm::aligned_vec4 pos1(p1.position);

    // These 6 vertices form two triangles of the line quad
    Vertex v0, v1, v2, v3;

    v0.position = p0.position;
    v1.position = p1.position;
    v2.position = p0.position;
    v3.position = p1.position;

    v0.normal = normal;
    v1.normal = normal;
    normal.w  = -thickness;
    v2.normal = normal;
    v3.normal = normal;

    v0.color = p0.color;
    v1.color = p1.color;
    v2.color = p0.color;
    v3.color = p1.color;

    // Add first triangle
    out.push_back(v0);
    out.push_back(v1);
    out.push_back(v2);

    // Add second triangle
    out.push_back(v2);
    out.push_back(v1);
    out.push_back(v3);
#else
    // Calculate direction vector (shader will normalize)
    const glm::vec3 dir = p1.position - p0.position;

    // Pre-allocate space for 6 vertices to avoid multiple reallocations
    out.reserve(out.size() + 6);

    // Create normals with positive and negative thickness
    const glm::aligned_vec4 normal_pos(dir, thickness);
    const glm::aligned_vec4 normal_neg(dir, -thickness);

    // Emplace vertices directly to avoid temporary objects
    // First triangle: v0, v1, v2
    out.emplace_back(p0.position, normal_pos, p0.color);
    out.emplace_back(p1.position, normal_pos, p1.color);
    out.emplace_back(p0.position, normal_neg, p0.color);

    // Second triangle: v2, v1, v3
    out.emplace_back(p0.position, normal_neg, p0.color);
    out.emplace_back(p1.position, normal_pos, p1.color);
    out.emplace_back(p1.position, normal_neg, p1.color);
#endif
}

void PGraphicsOpenGL_3::OGL3_add_line_quad_and_bevel(const Vertex& p0, const Vertex& p1, const Vertex& p2, float thickness, std::vector<Vertex>& out) {
    glm::vec3         dir = p1.position - p0.position; // NOTE no need to normalize, the shader will do it
    glm::aligned_vec4 normal(dir, thickness);
    Vertex            v0, v1, v2, v3;

    v0.position = p0.position;
    v1.position = p1.position;
    v2.position = p0.position;
    v3.position = p1.position;

    v0.color = p0.color;
    v1.color = p1.color;
    v2.color = p0.color;
    v3.color = p1.color;

    v0.normal = normal;
    v1.normal = normal;
    normal.w  = -thickness;
    v2.normal = normal;
    v3.normal = normal;

    // Add first triangle
    out.push_back(v0);
    out.push_back(v1);
    out.push_back(v2);

    // Add second triangle
    out.push_back(v2);
    out.push_back(v1);
    out.push_back(v3);

    // add bevel triangles
    glm::vec3         dir_next = p2.position - p1.position;
    glm::aligned_vec4 normal_prev(dir, thickness);
    glm::aligned_vec4 normal_next(dir_next, thickness);

    // Bevel triangle 1: junction point + two positive extensions
    Vertex bv0, bv1, bv2;
    bv0.position = p1.position;
    bv1.position = p1.position;
    bv2.position = p1.position;

    bv0.color = p1.color;
    bv1.color = p1.color;
    bv2.color = p1.color;

    bv0.normal = glm::vec4(0, 0, 0, 0); // Junction point (no offset)
    bv1.normal = normal_prev;           // Extension along previous line
    bv2.normal = normal_next;           // Extension along next line

    out.push_back(bv0);
    out.push_back(bv1);
    out.push_back(bv2);

    // Bevel triangle 2: junction point + two negative extensions
    Vertex bv3, bv4, bv5;
    bv3.position = p1.position;
    bv4.position = p1.position;
    bv5.position = p1.position;

    bv3.color = p1.color;
    bv4.color = p1.color;
    bv5.color = p1.color;

    normal_prev.w = -thickness;
    normal_next.w = -thickness;

    bv3.normal = glm::vec4(0, 0, 0, 0); // Junction point (no offset)
    bv4.normal = normal_next;           // Extension along next line (negative side)
    bv5.normal = normal_prev;           // Extension along previous line (negative side)

    out.push_back(bv3);
    out.push_back(bv4);
    out.push_back(bv5);
}

/* --- UTILITIES --- */

void PGraphicsOpenGL_3::beginDraw() {
    if (render_to_offscreen) {
        store_fbo_state();
    }
    noLights();
    resetShader();
    PGraphicsOpenGL::beginDraw();
    texture(nullptr);
}

void PGraphicsOpenGL_3::endDraw() {
    PGraphicsOpenGL::endDraw();
}

void PGraphicsOpenGL_3::texture(PImage* img) {
    PGraphics::texture(img);
    // enable and
    if (img != nullptr) {
        texture_update_and_bind(img);
    } else {
        texture_update_and_bind(nullptr);
    }
}

void PGraphicsOpenGL_3::render_framebuffer_to_screen(const bool use_blit) {
    if (use_blit) {
        // NOTE modern OpenGL framebuffer rendering method
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.id);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, framebuffer.width, framebuffer.height,
                          0, 0, framebuffer.width, framebuffer.height,
                          GL_COLOR_BUFFER_BIT, GL_LINEAR); // TODO maybe GL_NEAREST is enough
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    } else {
        warning("TODO only blitting supported atm … `render_framebuffer_to_screen` needs to implement this ... may re-use existing shader");
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // glDisable(GL_DEPTH_TEST);
        // glDisable(GL_BLEND);
        //
        // glUseProgram(shaderProgram);
        // glBindVertexArray(screenVAO);
        //
        // bind_framebuffer_texture();
        // glUniform1i(glGetUniformLocation(shaderProgram, "screenTexture"), 0);
        //
        // glDrawArrays(GL_TRIANGLES, 0, 6);
        //
        // glBindVertexArray(0);
        // glUseProgram(0);
    }
}

void PGraphicsOpenGL_3::hint(const uint16_t property) {
    PGraphics::hint(property);
    switch (property) {
        case ENABLE_SMOOTH_LINES:
#ifndef OPENGL_ES_3_0
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
#endif
            break;
        case DISABLE_SMOOTH_LINES:
#ifndef OPENGL_ES_3_0
            glDisable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
#endif
            break;
        default:
            break;
    }
}

void PGraphicsOpenGL_3::upload_texture(PImage*         img,
                                       const uint32_t* pixel_data,
                                       const int       width,
                                       const int       height,
                                       const int       offset_x,
                                       const int       offset_y) {
    if (img == nullptr) {
        error_in_function("image is nullptr.");
        return;
    }

    if (pixel_data == nullptr) {
        error_in_function("pixel data is nullptr");
        return;
    }

    // check if the provided width, height, and offsets are within the valid range
    if (width <= 0 || height <= 0) {
        error_in_function("invalid width or height");
        return;
    }

    if (offset_x < 0 || offset_y < 0 || offset_x + width > img->width || offset_y + height > img->height) {
        error_in_function("parameters exceed image dimensions");
        return;
    }

    if (img->texture_id < TEXTURE_VALID_ID) {
        OGL_generate_and_upload_image_as_texture(img); // NOTE texture binding and unbinding is handled here properly
        console_in_function(": texture has not been initialized yet … trying to initialize");
        if (img->texture_id < TEXTURE_VALID_ID) {
            error_in_function("failed to create texture");
            return;
        }
        console("texture is now initialized.");
        if (offset_x > 0 || offset_y > 0) {
            console_in_function(": offset was ignored (WIP)");
        }
        return; // NOTE this should be fine, as the texture is now initialized
    }

    const int tmp_bound_texture = get_current_texture_id();
    OGL_bind_texture(img->texture_id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0, offset_x, offset_y,
                    width, height,
                    UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                    UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                    pixel_data);

    if (img->get_auto_generate_mipmap()) {
        glGenerateMipmap(GL_TEXTURE_2D); // NOTE this works on macOS … but might not work on all platforms
    }

    OGL_bind_texture(tmp_bound_texture);
}

void PGraphicsOpenGL_3::download_texture(PImage* img) {
    if (img == nullptr) {
        error_in_function("image is nullptr");
        return;
    }
    if (img->pixels == nullptr) {
        error_in_function("pixel data is nullptr");
        return;
    }
    if (img->texture_id < TEXTURE_VALID_ID) {
        error_in_function("texture has not been initialized yet");
        return;
    }

#ifndef OPENGL_ES_3_0
    const int tmp_bound_texture = get_current_texture_id();
    OGL_bind_texture(img->texture_id);
    glGetTexImage(GL_TEXTURE_2D, 0,
                  UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                  UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                  img->pixels);
    OGL_bind_texture(tmp_bound_texture);
#else
    static bool emit_warning = true;
    if (emit_warning) {
        emit_warning = false;
        warning("PGraphics / `download_texture` not implemented for OpenGL ES 3.0");
    }
#endif
}

void PGraphicsOpenGL_3::init(uint32_t* pixels, const int width, const int height) {
    const int msaa_samples = antialiasing; // TODO not cool to take this from Umfeld

    this->width        = width;
    this->height       = height;
    framebuffer.width  = width;
    framebuffer.height = height;
    framebuffer.msaa   = render_to_offscreen && msaa_samples > 0;

    constexpr uint32_t internal_target_format = CREATE_SRGB_FBO ? GL_SRGB8_ALPHA8 : UMFELD_DEFAULT_INTERNAL_PIXEL_FORMAT;

    if (render_to_offscreen) {
        glGenFramebuffers(1, &framebuffer.id);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
        glGenTextures(1, &framebuffer.texture_id);

#ifdef OPENGL_ES_3_0
        if (framebuffer.msaa) {
            warning("MSAA not supported in OpenGL ES 3.0 ... disabling MSAA.");
            framebuffer.msaa = false;
        }
#endif

        if (framebuffer.msaa) {
            console("using multisample anti-aliasing (MSAA)");

            GLint maxSamples;
            glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
            console(format_label("Max supported MSAA samples"), maxSamples);

            GLuint    msaaDepthBuffer;
            const int samples = std::min(msaa_samples, maxSamples); // Number of MSAA samples
            console(format_label("number of used MSAA samples"), samples);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebuffer.texture_id); // NOTE no need to use `OGL_bind_texture()`
            UMFELD_PGRAPHICS_OPENGL_3_CHECK_ERRORS("glBindTexture");
#ifndef OPENGL_ES_3_0
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                                    samples,
                                    internal_target_format,
                                    framebuffer.width,
                                    framebuffer.height,
                                    GL_TRUE);
#endif
            UMFELD_PGRAPHICS_OPENGL_3_CHECK_ERRORS("glTexImage2DMultisample");
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D_MULTISAMPLE,
                                   framebuffer.texture_id, 0);
            UMFELD_PGRAPHICS_OPENGL_3_CHECK_ERRORS("glFramebufferTexture2D");
            // Create Multisampled Depth Buffer
            glGenRenderbuffers(1, &msaaDepthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthBuffer);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                             samples,
                                             GL_DEPTH24_STENCIL8,
                                             framebuffer.width,
                                             framebuffer.height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER,
                                      msaaDepthBuffer);
        } else {
            glBindTexture(GL_TEXTURE_2D, framebuffer.texture_id); // NOTE no need to use `OGL_bind_texture()`
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         internal_target_format,
                         framebuffer.width,
                         framebuffer.height,
                         0,
                         UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                         UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                         nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D,
                                   framebuffer.texture_id,
                                   0);
            GLuint depthBuffer;
            glGenRenderbuffers(1, &depthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebuffer.width, framebuffer.height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            error_in_function("framebuffer is not complete!");
        }

        glViewport(0, 0, framebuffer.width, framebuffer.height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (framebuffer.msaa) {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0); // NOTE no need to use `OGL_bind_texture()`
        } else {
            glBindTexture(GL_TEXTURE_2D, 0); // NOTE no need to use `OGL_bind_texture()`
        }
        texture_id = framebuffer.texture_id; // TODO maybe get rid of one of the texture_id variables
    } else {
        GLuint _buffer_texture_id;
        glGenTextures(1, &_buffer_texture_id);
        glBindTexture(GL_TEXTURE_2D, _buffer_texture_id); // NOTE no need to use `OGL_bind_texture()`
        glTexImage2D(GL_TEXTURE_2D, 0,
                     internal_target_format,
                     width, height,
                     0,
                     UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                     UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        texture_id = _buffer_texture_id;
    }

    /* initialize shape renderer */
    const auto            shape_renderer_ogl3 = new UShapeRendererOpenGL_3();
    std::vector<PShader*> shader_batch_programs(NUM_SHADER_PROGRAMS);
    shader_batch_programs[SHADER_PROGRAM_COLOR]          = loadShader(shader_source_color.get_vertex_source(), shader_source_color.get_fragment_source());
    shader_batch_programs[SHADER_PROGRAM_TEXTURE]        = loadShader(shader_source_texture.get_vertex_source(), shader_source_texture.get_fragment_source());
    shader_batch_programs[SHADER_PROGRAM_COLOR_LIGHTS]   = loadShader(shader_source_color_lights.get_vertex_source(), shader_source_color_lights.get_fragment_source());
    shader_batch_programs[SHADER_PROGRAM_TEXTURE_LIGHTS] = loadShader(shader_source_texture_lights.get_vertex_source(), shader_source_texture_lights.get_fragment_source());
    shader_batch_programs[SHADER_PROGRAM_POINT]          = loadShader(shader_source_point.get_vertex_source(), shader_source_point.get_fragment_source());
    shader_batch_programs[SHADER_PROGRAM_LINE]           = loadShader(shader_source_line.get_vertex_source(), shader_source_line.get_fragment_source());
    shape_renderer_ogl3->init(this, shader_batch_programs);
    shape_renderer = shape_renderer_ogl3;

    shader_fullscreen_texture = loadShader(shader_source_fullscreen.get_vertex_source(), shader_source_fullscreen.get_fragment_source());

    if constexpr (sizeof(Vertex) != 64) {
        // ReSharper disable once CppDFAUnreachableCode
        warning("Vertex struct must be 64 bytes");
    }
}

void PGraphicsOpenGL_3::resize(const int new_width, const int new_height) {
    if (new_width <= 0 || new_height <= 0) {
        error_in_function("invalid size for resize: ", new_width, " x ", new_height);
        return;
    }

    // Save previous GL state if we manage an offscreen FBO
    if (render_to_offscreen) {
        store_fbo_state();
    }

    // Update logical sizes
    this->width        = new_width;
    this->height       = new_height;
    framebuffer.width  = new_width;
    framebuffer.height = new_height;

    constexpr uint32_t internal_target_format = CREATE_SRGB_FBO ? GL_SRGB8_ALPHA8 : UMFELD_DEFAULT_INTERNAL_PIXEL_FORMAT;

    // Recreate color target(s)
    if (render_to_offscreen) {
        // Recreate framebuffer resources
        if (framebuffer.id == 0) {
            glGenFramebuffers(1, &framebuffer.id);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);

        // Recreate color texture
        if (framebuffer.texture_id == 0) {
            glGenTextures(1, &framebuffer.texture_id);
        }

#ifdef OPENGL_ES_3_0
        if (framebuffer.msaa) {
            warning("MSAA not supported in OpenGL ES 3.0 ... disabling MSAA on resize.");
            framebuffer.msaa = false;
        }
#endif

        if (framebuffer.msaa) {
#ifndef OPENGL_ES_3_0
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebuffer.texture_id);
            GLint maxSamples;
            glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
            const int samples = std::min(antialiasing, maxSamples);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                                    samples,
                                    internal_target_format,
                                    framebuffer.width,
                                    framebuffer.height,
                                    GL_TRUE);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D_MULTISAMPLE,
                                   framebuffer.texture_id, 0);
            // Depth/stencil renderbuffer (recreate each resize)
            GLuint msaaDepthBuffer;
            glGenRenderbuffers(1, &msaaDepthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthBuffer);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                             samples,
                                             GL_DEPTH24_STENCIL8,
                                             framebuffer.width,
                                             framebuffer.height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER,
                                      msaaDepthBuffer);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
#endif
        } else {
            glBindTexture(GL_TEXTURE_2D, framebuffer.texture_id);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         internal_target_format,
                         framebuffer.width,
                         framebuffer.height,
                         0,
                         UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                         UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                         nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D,
                                   framebuffer.texture_id,
                                   0);

            // Depth/stencil renderbuffer (recreate each resize)
            GLuint depthBuffer;
            glGenRenderbuffers(1, &depthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebuffer.width, framebuffer.height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            error_in_function("framebuffer is not complete after resize!");
        }

        // Clear new target and set viewport to new size
        glViewport(0, 0, framebuffer.width, framebuffer.height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Unbind FBO; restore previous state
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        texture_id = framebuffer.texture_id;
        restore_fbo_state();
    } else {
        // On-screen: recreate backing texture used for blits/draws
        if (texture_id == 0) {
            GLuint _buffer_texture_id;
            glGenTextures(1, &_buffer_texture_id);
            texture_id = _buffer_texture_id;
        }
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0,
                     internal_target_format,
                     new_width, new_height,
                     0,
                     UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                     UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Update viewport for immediate rendering paths
        glViewport(0, 0, new_width, new_height);
    }

    // Optional: update projection dependent data if needed (client code may also call perspective/ortho)
    // resetShader(); // ensure no stale program bound if caller expects default state
}

/* additional */

PShader* PGraphicsOpenGL_3::loadShader(const std::string& vertex_code, const std::string& fragment_code, const std::string& geometry_code) {
    const auto shader = new PShader();
    const bool result = shader->load(vertex_code, fragment_code, geometry_code);
    if (!result) {
        error_in_function("failed to load shader: \n\n", vertex_code, "\n\n", fragment_code, "\n\n", geometry_code);
        delete shader;
        return nullptr;
    }
    return shader;
}

void PGraphicsOpenGL_3::shader(PShader* shader) {
    if (shader != nullptr) {
        glUseProgram(shader->get_program_id());
    }
    PGraphics::shader(shader);
}

void PGraphicsOpenGL_3::resetShader() {
    PGraphics::resetShader();
    glUseProgram(0);
}

bool PGraphicsOpenGL_3::read_framebuffer(std::vector<unsigned char>& pixels) {
    if (render_to_offscreen) {
        store_fbo_state();
        if (framebuffer.msaa) {
            // NOTE this is a bit tricky. when the offscreen FBO is a multisample FBO ( MSAA ) we need to resolve it first
            //      i.e blit it into the color buffer of the default framebuffer. otherwise we can just read from the
            //      offscreen FBO.
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.id);
            glBlitFramebuffer(0, 0, framebuffer.width, framebuffer.height,
                              0, 0, framebuffer.width, framebuffer.height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id); // non-MSAA FBO or default
        }
        const bool success = OGL_read_framebuffer(framebuffer, pixels);
        restore_fbo_state();
        return success;
    } else {
        const bool success = OGL_read_framebuffer(framebuffer, pixels);
        return success;
    }
}

void PGraphicsOpenGL_3::store_fbo_state() {
    if (render_to_offscreen) {
        glGetIntegerv(GL_CURRENT_PROGRAM, &previous_shader);
        glGetIntegerv(GL_VIEWPORT, previous_viewport);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previously_bound_read_FBO);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previously_bound_draw_FBO);
    } else {
        warning_in_function_once("store_fbo_state() requires render_to_offscreen to be true.");
    }
}

void PGraphicsOpenGL_3::bind_fbo() {
    if (render_to_offscreen) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
    } else {
        warning_in_function_once("bind_fbo() requires render_to_offscreen to be true.");
    }
}

void PGraphicsOpenGL_3::restore_fbo_state() {
    if (render_to_offscreen) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, previously_bound_read_FBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previously_bound_draw_FBO);
        glViewport(previous_viewport[0], previous_viewport[1], previous_viewport[2], previous_viewport[3]);
        glUseProgram(previous_shader);
    } else {
        warning_in_function_once("restore_fbo_state() requires render_to_offscreen to be true.");
    }
}

/* --- LIGHTS --- */

void PGraphicsOpenGL_3::noLights() {
    lights_enabled                             = false;
    lightingState.lightCount                   = 0;
    lightingState.currentLightSpecular         = glm::vec3(0.0f);
    lightingState.currentLightFalloffConstant  = 1.0f;
    lightingState.currentLightFalloffLinear    = 0.0f;
    lightingState.currentLightFalloffQuadratic = 0.0f;
    resetShader();
}

void PGraphicsOpenGL_3::lights() {
    lights_enabled = true;

    ambient(0.5f, 0.5f, 0.5f);
    specular(0.5f, 0.5f, 0.5f);
    emissive(0.1f, 0.1f, 0.1f);
    shininess(64.0f);
    lightFalloff(1, 0, 0);
    lightSpecular(0, 0, 0);

    ambientLight(128 / 255.0f, 128 / 255.0f, 128 / 255.0f);
    directionalLight(128 / 255.0f, 128 / 255.0f, 128 / 255.0f, 0, 0, 1); // TODO why is this (0, 0, 1) and not (0, 0, -1) as described in the documentation?
}

void PGraphicsOpenGL_3::ambientLight(const float r, const float g, const float b, const float x, const float y, const float z) {
    lights_enabled = true;
    if (lightingState.lightCount >= lightingState.MAX_LIGHTS) {
        return;
    }

    lightingState.lightType[lightingState.lightCount] = LightingState::AMBIENT;

    setLightPosition(lightingState.lightCount, x, y, z, false);
    setLightNormal(lightingState.lightCount, 0, 0, 0);

    setLightAmbient(lightingState.lightCount, r, g, b);
    setNoLightDiffuse(lightingState.lightCount);
    setNoLightSpecular(lightingState.lightCount);
    setNoLightSpot(lightingState.lightCount);
    setLightFalloff(lightingState.lightCount, lightingState.currentLightFalloffConstant,
                    lightingState.currentLightFalloffLinear,
                    lightingState.currentLightFalloffQuadratic);

    lightingState.lightCount++;
}

void PGraphicsOpenGL_3::directionalLight(const float r, const float g, const float b, const float nx, const float ny, const float nz) {
    lights_enabled = true;
    if (lightingState.lightCount >= lightingState.MAX_LIGHTS) {
        return;
    }

    lightingState.lightType[lightingState.lightCount] = LightingState::DIRECTIONAL;

    setLightPosition(lightingState.lightCount, 0, 0, 0, true); // directional = true
    setLightNormal(lightingState.lightCount, nx, ny, nz);

    setNoLightAmbient(lightingState.lightCount);
    setLightDiffuse(lightingState.lightCount, r, g, b);
    setLightSpecular(lightingState.lightCount,
                     lightingState.currentLightSpecular.r,
                     lightingState.currentLightSpecular.g,
                     lightingState.currentLightSpecular.b);
    setNoLightSpot(lightingState.lightCount);
    setNoLightFalloff(lightingState.lightCount);

    lightingState.lightCount++;
}

void PGraphicsOpenGL_3::pointLight(const float r, const float g, const float b, const float x, const float y, const float z) {
    lights_enabled = true;
    if (lightingState.lightCount >= lightingState.MAX_LIGHTS) {
        return;
    }

    lightingState.lightType[lightingState.lightCount] = LightingState::POINT;

    setLightPosition(lightingState.lightCount, x, y, z, false);
    setLightNormal(lightingState.lightCount, 0, 0, 0);

    setNoLightAmbient(lightingState.lightCount);
    setLightDiffuse(lightingState.lightCount, r, g, b);
    setLightSpecular(lightingState.lightCount,
                     lightingState.currentLightSpecular.r,
                     lightingState.currentLightSpecular.g,
                     lightingState.currentLightSpecular.b);
    setNoLightSpot(lightingState.lightCount);
    setLightFalloff(lightingState.lightCount,
                    lightingState.currentLightFalloffConstant,
                    lightingState.currentLightFalloffLinear,
                    lightingState.currentLightFalloffQuadratic);

    lightingState.lightCount++;
}

void PGraphicsOpenGL_3::spotLight(const float r, const float g, const float b, const float x, const float y, const float z,
                                  const float nx, const float ny, const float nz, const float angle, const float concentration) {
    lights_enabled = true;
    if (lightingState.lightCount >= lightingState.MAX_LIGHTS) {
        return;
    }

    lightingState.lightType[lightingState.lightCount] = LightingState::SPOT;

    setLightPosition(lightingState.lightCount, x, y, z, false);
    setLightNormal(lightingState.lightCount, nx, ny, nz);

    setNoLightAmbient(lightingState.lightCount);
    setLightDiffuse(lightingState.lightCount, r, g, b);
    setLightSpecular(lightingState.lightCount,
                     lightingState.currentLightSpecular.r,
                     lightingState.currentLightSpecular.g,
                     lightingState.currentLightSpecular.b);
    setLightSpot(lightingState.lightCount, angle, concentration);
    setLightFalloff(lightingState.lightCount,
                    lightingState.currentLightFalloffConstant,
                    lightingState.currentLightFalloffLinear,
                    lightingState.currentLightFalloffQuadratic);

    lightingState.lightCount++;
}

void PGraphicsOpenGL_3::lightFalloff(const float constant, const float linear, const float quadratic) {
    lightingState.currentLightFalloffConstant  = constant;
    lightingState.currentLightFalloffLinear    = linear;
    lightingState.currentLightFalloffQuadratic = quadratic;
}

void PGraphicsOpenGL_3::lightSpecular(const float r, const float g, const float b) {
    lightingState.currentLightSpecular = glm::vec3(r, g, b);
}

void PGraphicsOpenGL_3::ambient(const float r, const float g, const float b) {
    lightingState.ambient = glm::vec4(r, g, b, 1.0f);
}

void PGraphicsOpenGL_3::specular(const float r, const float g, const float b) {
    lightingState.specular = glm::vec4(r, g, b, 1.0f);
}

void PGraphicsOpenGL_3::emissive(const float r, const float g, const float b) {
    lightingState.emissive = glm::vec4(r, g, b, 1.0f);
}

void PGraphicsOpenGL_3::shininess(const float s) {
    lightingState.shininess = s;
}

void PGraphicsOpenGL_3::setLightPosition(const int num, const float x, const float y, const float z, const bool directional) {
    // TODO Transform position by current modelview matrix
    //      For now, assuming world space coordinates
    lightingState.lightPositions[num] = glm::vec4(x, y, z, directional ? 0.0f : 1.0f);
}

void PGraphicsOpenGL_3::setLightNormal(const int num, const float dx, const float dy, const float dz) {
    // NOTE normalize the direction vector
    glm::vec3 normal(dx, dy, dz);
    if (glm::length(normal) > 0.0f) {
        normal = glm::normalize(normal);
    }
    lightingState.lightNormals[num] = normal;
}

void PGraphicsOpenGL_3::setLightAmbient(const int num, const float r, const float g, const float b) {
    lightingState.lightAmbientColors[num] = glm::vec3(r, g, b);
}

void PGraphicsOpenGL_3::setNoLightAmbient(const int num) {
    lightingState.lightAmbientColors[num] = glm::vec3(0.0f);
}

void PGraphicsOpenGL_3::setLightDiffuse(const int num, const float r, const float g, const float b) {
    lightingState.lightDiffuseColors[num] = glm::vec3(r, g, b);
}

void PGraphicsOpenGL_3::setNoLightDiffuse(const int num) {
    lightingState.lightDiffuseColors[num] = glm::vec3(0.0f);
}

void PGraphicsOpenGL_3::setLightSpecular(const int num, const float r, const float g, const float b) {
    lightingState.lightSpecularColors[num] = glm::vec3(r, g, b);
}

void PGraphicsOpenGL_3::setNoLightSpecular(const int num) {
    lightingState.lightSpecularColors[num] = glm::vec3(0.0f);
}

void PGraphicsOpenGL_3::setLightFalloff(const int num, const float constant, const float linear, const float quadratic) {
    lightingState.lightFalloffCoeffs[num] = glm::vec3(constant, linear, quadratic);
}

void PGraphicsOpenGL_3::setNoLightFalloff(const int num) {
    lightingState.lightFalloffCoeffs[num] = glm::vec3(1.0f, 0.0f, 0.0f);
}

void PGraphicsOpenGL_3::setLightSpot(const int num, const float angle, const float concentration) {
    lightingState.lightSpotParams[num] = glm::vec2(std::max(0.0f, std::cos(angle)), concentration);
}

void PGraphicsOpenGL_3::setNoLightSpot(const int num) {
    lightingState.lightSpotParams[num] = glm::vec2(-1.0f, 0.0f); // -1 disables spotlight
}

void PGraphicsOpenGL_3::upload_colorbuffer(uint32_t* pixels) {
    if (pixels == nullptr) {
        error_in_function("pixels pointer is null, cannot upload color buffer.");
        return;
    }

    if (render_to_offscreen) {
        if (!framebuffer.msaa) {
            OGL3_flip_pixel_buffer(pixels);
            push_texture_id();
            OGL_bind_texture(framebuffer.texture_id);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glTexSubImage2D(GL_TEXTURE_2D,
                            0,
                            0, 0,
                            framebuffer.width,
                            framebuffer.height,
                            UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                            UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                            pixels);
            pop_texture_id();
        } else {
            // TODO there is room for optimization below this line ...
            //      i.e do not create a texture every time and maybe create a dedicated texture for intermediate MSAA rendering
            // upload pixels to intermediate non-MSAA texture
            GLuint tempTexture;
            glGenTextures(1, &tempTexture);
            glBindTexture(GL_TEXTURE_2D, tempTexture);
            glTexImage2D(GL_TEXTURE_2D, 0,
                         UMFELD_DEFAULT_INTERNAL_PIXEL_FORMAT,
                         framebuffer.width, framebuffer.height, 0,
                         UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                         UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                         pixels);

            // setup texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // bind MSAA framebuffer
            store_fbo_state();
            bind_fbo();
            glViewport(0, 0, framebuffer.width, framebuffer.height);

            // draw fullscreen quad
            push_texture_id();
            OGL_bind_texture(tempTexture);
            OGL3_draw_fullscreen_texture(tempTexture);

            pop_texture_id();

            restore_fbo_state();
            glDeleteTextures(1, &tempTexture);
        }
    } else {
        push_texture_id();
        OGL_bind_texture(texture_id);
        glTexSubImage2D(GL_TEXTURE_2D,
                        0,
                        0, 0,
                        this->framebuffer.width,
                        this->framebuffer.height,
                        // width,
                        // height,
                        UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                        UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                        pixels);
        OGL3_draw_fullscreen_texture(texture_id);
        pop_texture_id();
    }
}

void PGraphicsOpenGL_3::download_colorbuffer(uint32_t* pixels) {
    if (pixels == nullptr) {
        error_in_function("pixels pointer is null, cannot download color buffer.");
        return;
    }
    if (render_to_offscreen) {
        store_fbo_state();
        bind_fbo();
        if (framebuffer.msaa) {
            // OPTIMIZE there is room for optimization below this line ...
            //          i.e do not use a temporary FBO if possible but rather
            //          create it once on first call to `download_colorbuffer`
            // Step 1: Create intermediate non-MSAA FBO + texture
            GLuint tempFBO, tempTex;
            glGenFramebuffers(1, &tempFBO);
            glGenTextures(1, &tempTex);

            push_texture_id();
            OGL_bind_texture(tempTex);

            glTexImage2D(GL_TEXTURE_2D, 0,
                         UMFELD_DEFAULT_INTERNAL_PIXEL_FORMAT,
                         framebuffer.width,
                         framebuffer.height,
                         0, UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                         UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                         nullptr);

            glBindFramebuffer(GL_FRAMEBUFFER, tempFBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D,
                                   tempTex,
                                   0);

            // Step 2: Blit from MSAA FBO to non-MSAA FBO
            glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(framebuffer.id));
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempFBO);
            glBlitFramebuffer(0, 0, framebuffer.width, framebuffer.height,
                              0, 0, framebuffer.width, framebuffer.height,
                              GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);

            // Step 3: Read pixels from the tempFBO
            glBindFramebuffer(GL_FRAMEBUFFER, tempFBO);
            glPixelStorei(GL_PACK_ALIGNMENT, 4); // assuming tight RGBA8 layout
            glReadPixels(0, 0,
                         framebuffer.width,
                         framebuffer.height,
                         UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                         UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                         pixels);

            // Cleanup
            glDeleteTextures(1, &tempTex);
            glDeleteFramebuffers(1, &tempFBO);

            pop_texture_id();
        } else {
            // Direct read from FBO
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
            glPixelStorei(GL_PACK_ALIGNMENT, 4);
            glReadPixels(0, 0,
                         framebuffer.width,
                         framebuffer.height,
                         UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                         UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                         pixels);
        }
        restore_fbo_state();
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glReadPixels(0, 0, framebuffer.width, framebuffer.height,
                     UMFELD_DEFAULT_EXTERNAL_PIXEL_FORMAT,
                     UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                     pixels);
    }
    OGL3_flip_pixel_buffer(pixels);
}

void PGraphicsOpenGL_3::OGL3_flip_pixel_buffer(uint32_t* pixels) {
    const int d      = displayDensity();
    const int phys_w = width * d;
    const int phys_h = height * d;
// #define MORE_COMPATIBLE_FLIP_PIXEL_BUFFER
#ifdef MORE_COMPATIBLE_FLIP_PIXEL_BUFFER
    const auto tmp = new uint32_t[phys_w * phys_h];
    std::memcpy(tmp, pixels, phys_w * phys_h * sizeof(uint32_t));
    for (int y = 0; y < phys_h; ++y) {
        const int flipped_y = phys_h - 1 - y;
        std::memcpy(&pixels[y * phys_w],
                    &tmp[flipped_y * phys_w],
                    phys_w * sizeof(uint32_t));
    }
    delete[] tmp;
#else  // MORE_COMPATIBLE_FLIP_PIXEL_BUFFER
    const size_t sz  = phys_w * sizeof(uint32_t);
    const auto   tmp = static_cast<uint32_t*>(alloca(sz)); // stack alloc for one row
    for (int y = 0; y < phys_h / 2; ++y) {
        uint32_t* row_top = pixels + y * phys_w;
        uint32_t* row_bot = pixels + (phys_h - 1 - y) * phys_w;

        memcpy(tmp, row_top, sz);
        memcpy(row_top, row_bot, sz);
        memcpy(row_bot, tmp, sz);
    }
#endif // MORE_COMPATIBLE_FLIP_PIXEL_BUFFER
}

void PGraphicsOpenGL_3::OGL3_draw_fullscreen_texture(const GLuint texture_id) const {
    if (shader_fullscreen_texture == nullptr) {
        return;
    }
    shader_fullscreen_texture->use();
    // Ensure texture unit 0 active & bound
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    // Set sampler (assuming PShader has generic uniform setter; else raw glUniform)
    // TODO should use PShader methods
    // shader_fullscreen_texture->set_uniform("u_texture_unit", 0);
    const GLint loc = glGetUniformLocation(shader_fullscreen_texture->get_program_id(), "u_texture_unit");
    if (loc >= 0) {
        glUniform1i(loc, 0);
    }
    // VAO must be bound in core profile
    static GLuint empty_VAO = 0;
    if (empty_VAO == 0) {
        glGenVertexArrays(1, &empty_VAO);
    }
    glBindVertexArray(empty_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3); // single fullscreen triangle
    glBindVertexArray(0);
}

#endif // UMFELD_PGRAPHICS_OPENGLV33_CPP