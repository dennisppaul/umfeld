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

#include "UmfeldSDLOpenGL.h" // TODO move to cpp implementation
#include "Umfeld.h"
#include "PGraphicsOpenGL.h"

namespace umfeld {

    void OGL_draw_pre();

    static void center_display(SDL_Window* window) {
        int mDisplayLocation;
        if (display == DEFAULT) {
            mDisplayLocation = SDL_WINDOWPOS_CENTERED;
        } else {
            int mNumDisplays = 0;
            SDL_GetDisplays(&mNumDisplays);
            if (display >= mNumDisplays) {
                error("display index '", display, "' out of range. ",
                      mNumDisplays, " display", mNumDisplays > 1 ? "s are" : " is",
                      " available. using default display.");
                mDisplayLocation = SDL_WINDOWPOS_CENTERED;
            } else {
                mDisplayLocation = SDL_WINDOWPOS_CENTERED_DISPLAY(display);
            }
        }
        SDL_SetWindowPosition(window, mDisplayLocation, mDisplayLocation);
    }

    inline bool OGL_init(SDL_Window*&   window,
                         SDL_GLContext& gl_context,
                         const int      major_version,
                         const int      minor_version,
                         const int      profile) {
        /* setup opengl */

#ifndef OPENGL_ES_3_0
#ifdef SYSTEM_MACOS
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // always required on Mac?
#endif
#endif
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major_version);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor_version);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        if (antialiasing > 0) {
            // TODO querry number of available buffers and samples
            //      code below this always returns 0 :(
            // GLint available_buffers, available_samples;
            // SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &available_buffers);
            // SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &available_samples);
            // console(fl("antialiasing"), "enabled with ", available_buffers, " buffers and ", available_samples, " samples per pixel");
            // antialiasing = antialiasing > available_buffers ? available_buffers : antialiasing;

            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, antialiasing);
            console(fl("antialiasing"), antialiasing, "x");
        } else {
            console(fl("antialiasing"), "disabled");
        }

        /* window */

        SDL_WindowFlags flags = SDL_WINDOW_OPENGL;
        window                = SDL_CreateWindow(DEFAULT_WINDOW_TITLE,
                                                 static_cast<int>(umfeld::width),  // TODO would be better if this is passed as a parameter
                                                 static_cast<int>(umfeld::height), // TODO would be better if this is passed as a parameter
                                                 get_SDL_WindowFlags(flags));

        if (window == nullptr) {
            error("Couldn't create window: ", SDL_GetError());
            return false;
        }

        center_display(window);

        /* create opengl context */

        gl_context = SDL_GL_CreateContext(window);
        if (gl_context == nullptr) {
            error("Couldn't create OpenGL context: ", SDL_GetError());
            SDL_DestroyWindow(window);
            return false;
        }

        SDL_GL_MakeCurrent(window, gl_context);
        SDL_GL_SetSwapInterval(vsync ? 1 : 0);

        /* display window */

        SDL_ShowWindow(window);

        /* initialize GLAD */
// TODO maybe move to subsystem
#if defined(OPENGL_3_3_CORE) || defined(OPENGL_2_0)
        if (!gladLoadGL(SDL_GL_GetProcAddress)) {
            error("Failed to load OpenGL with GLAD");
            SDL_GL_DestroyContext(gl_context);
            SDL_DestroyWindow(window);
            return false;
        }
#elif defined(OPENGL_ES_3_0)
        if (!gladLoadGLES2(SDL_GL_GetProcAddress)) {
            error("Failed to load OpenGL with GLAD");
            SDL_GL_DestroyContext(gl_context);
            SDL_DestroyWindow(window);
            return false;
        }
#else
#error "Unsupported OpenGL version. Please define OPENGL_3_3_CORE or OPENGL_2_0 or OPENGL_ES_30."
#endif

        PGraphicsOpenGL::OpenGLCapabilities open_gl_capabilities;
        PGraphicsOpenGL::OGL_query_capabilities(open_gl_capabilities);
        PGraphicsOpenGL::OGL_check_error("SUBSYSTEM_GRAPHICS_OPENGL::init(end)");

        return true;
    }

    inline void OGL_setup_pre(SDL_Window*& window) {
        if (window == nullptr) {
            return;
        }

        if (g == nullptr) {
            return;
        }

        int        framebuffer_width;
        int        framebuffer_height;
        const bool success = SDL_GetWindowSizeInPixels(window, &framebuffer_width, &framebuffer_height);
        if (!success) {
            warning("Failed to get window size in pixels.");
        }
        const float pixel_density = SDL_GetWindowPixelDensity(window);

        console(fl("renderer"), g->name());
        console(fl("render to offscreen"), g->render_to_offscreen ? "true" : "false");
        console(fl("framebuffer size"), framebuffer_width, " x ", framebuffer_height, " px");
        console(fl("graphics size"), width, " x ", height, " px");
        console(fl("pixel_density"), pixel_density, (width != framebuffer_width) && (pixel_density <= 1) ? " "
#if UMFELD_DEBUG_PIXEL_DENSITY_FRAME_BUFFER
                                                                                                           "( pixel_density and framebuffer size do not align )"
#else
                                                                                                           ""
#endif
                                                                                                         : "");
        g->pixelDensity(pixel_density); // NOTE setting pixel density from actual configuration

        g->set_auto_generate_mipmap(false);
        g->init(nullptr, framebuffer_width, framebuffer_height);
        g->width  = width;
        g->height = height;
        g->lock_init_properties(true);

        g->set_default_graphics_state();

        OGL_draw_pre();
    }

    inline void OGL_setup_post() {
        // draw_post(); // TODO revive this once it is fully integrated
    }

    inline void OGL_draw_pre() {
        if (g == nullptr) {
            return;
        }
        g->beginDraw();
    }

    inline void OGL_draw_post(SDL_Window*& window, const bool blit_framebuffer_object_to_screenbuffer) {
        if (window == nullptr || g == nullptr) {
            return;
        }

        g->endDraw();

        if (g->render_to_offscreen && g->framebuffer.id > 0) {
            g->render_framebuffer_to_screen(blit_framebuffer_object_to_screenbuffer);
        }
        SDL_GL_SwapWindow(window);
    }

} // namespace umfeld