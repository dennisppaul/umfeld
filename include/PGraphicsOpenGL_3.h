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

#include "PGraphicsOpenGL.h"

namespace umfeld {
    class VertexBuffer;
    class PGraphicsOpenGL_3 final : public PGraphicsOpenGL {
    public:
        explicit PGraphicsOpenGL_3(bool render_to_offscreen);

        /* --- OpenGL 3.3 specific implementation of shared methods --- */

        void render_framebuffer_to_screen(bool use_blit = false) override;
        bool read_framebuffer(std::vector<unsigned char>& pixels) override;
        void store_fbo_state() override;
        void restore_fbo_state() override;
        void bind_fbo() override;
        void finish_fbo() override {}

        void upload_texture(PImage* img, const uint32_t* pixel_data, int width, int height, int offset_x, int offset_y) override;
        void download_texture(PImage* img) override;
        void upload_colorbuffer(uint32_t* pixels) override;
        void download_colorbuffer(uint32_t* pixels) override;

        void beginDraw() override;
        void endDraw() override;
        void texture(PImage* img) override;

        void init(uint32_t* pixels, int width, int height) override;
        void resize(int new_width, int height) override;

        std::string name() override {
#if defined(OPENGL_ES_3_0)
            return "PGraphicsOpenGL_ES_3_0";
#elif defined(OPENGL_3_3_CORE)
            return "PGraphicsOpenGL_3";
#else
            return "Unknown";
#endif
        }

        void hint(uint16_t property) override;

        /* --- standard drawing functions --- */

        // void     mesh(VertexBuffer* mesh_shape) override;
        void        shader(PShader* shader) override;
        PShader*    loadShader(const std::string& vertex_code, const std::string& fragment_code, const std::string& geometry_code = "") override;
        void        resetShader() override;
        void        lights() override;
        void        noLights() override;
        void        ambientLight(float r, float g, float b, float x = 0, float y = 0, float z = 0) override;
        void        directionalLight(float r, float g, float b, float nx, float ny, float nz) override;
        void        pointLight(float r, float g, float b, float x, float y, float z) override;
        void        spotLight(float r, float g, float b, float x, float y, float z, float nx, float ny, float nz, float angle, float concentration) override;
        void        lightFalloff(float constant, float linear, float quadratic) override;
        void        lightSpecular(float r, float g, float b) override;
        void        ambient(float r, float g, float b) override;
        void        specular(float r, float g, float b) override;
        void        emissive(float r, float g, float b) override;
        void        shininess(float s) override;
        static void OGL3_add_line_quad(const Vertex& p0, const Vertex& p1, float thickness, std::vector<Vertex>& out);
        static void OGL3_add_line_quad_and_bevel(const Vertex& p0, const Vertex& p1, const Vertex& p2, float thickness, std::vector<Vertex>& out);

    private:
        struct RenderBatch {
            int      start_index;
            int      num_vertices;
            uint32_t texture_id;

            RenderBatch(const int start, const int count, const uint32_t texID)
                : start_index(start), num_vertices(count), texture_id(texID) {}
        };

        static constexpr bool    CREATE_SRGB_FBO                        = false; // TODO sRGB is WIP
        static constexpr bool    RENDER_POINT_AS_CIRCLE                 = true;
        static constexpr bool    RENDER_PRIMITVES_AS_SHAPES             = true;
        static constexpr uint8_t NUM_FILL_VERTEX_ATTRIBUTES_XYZ_RGBA_UV = 9;
        static constexpr uint8_t NUM_STROKE_VERTEX_ATTRIBUTES_XYZ_RGBA  = 7;
        PShader*                 shader_fullscreen_texture{nullptr};
        int32_t                  previously_bound_read_FBO = 0;
        int32_t                  previously_bound_draw_FBO = 0;
        int32_t                  previous_viewport[4]{};
        int32_t                  previous_shader{0};


        /* --- lights --- */

        void setLightPosition(int num, float x, float y, float z, bool directional);
        void setLightNormal(int num, float dx, float dy, float dz);
        void setLightAmbient(int num, float r, float g, float b);
        void setNoLightAmbient(int num);
        void setLightDiffuse(int num, float r, float g, float b);
        void setNoLightDiffuse(int num);
        void setLightSpecular(int num, float r, float g, float b);
        void setNoLightSpecular(int num);
        void setLightFalloff(int num, float constant, float linear, float quadratic);
        void setNoLightFalloff(int num);
        void setLightSpot(int num, float angle, float concentration);
        void setNoLightSpot(int num);

        /* --- OpenGL 3.3 specific methods --- */

        // void        OGL3_update_shader_matrices(PShader* shader) const;
        // static void OGL3_reset_shader_matrices(PShader* shader);
        void OGL3_flip_pixel_buffer(uint32_t* pixels);
        void OGL3_draw_fullscreen_texture(uint32_t texture_id) const;
    };
} // namespace umfeld
