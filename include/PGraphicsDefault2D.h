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

#include <SDL3/SDL.h>

#include "PGraphics.h"

namespace umfeld {

    class PGraphicsDefault2D final : public PGraphics {
        void set_color_f(const ColorState& c) const {
            if (renderer == nullptr) {
                return;
            }
            SDL_SetRenderDrawColorFloat(renderer, c.r, c.g, c.b, c.a);
        }

    public:
        explicit PGraphicsDefault2D(SDL_Renderer* renderer) : renderer(renderer) {}

        void strokeWeight(float weight) override {}

        void background_f(const float a, const float b, const float c, const float d = 1.0f) override {
            if (renderer == nullptr) {
                return;
            }
            SDL_SetRenderDrawColorFloat(renderer, a, b, c, d);
            SDL_RenderClear(renderer);
        }

        void rect(const float x, const float y, const float width, const float height) override {
            if (renderer == nullptr) {
                return;
            }
            SDL_FRect r;
            r.x = x;
            r.y = y;
            r.w = width;
            r.h = height;
            if (color_stroke.active) {
                set_color_f(color_stroke);
                SDL_RenderRect(renderer, &r);
            }
            if (color_fill.active) {
                set_color_f(color_fill);
                SDL_RenderFillRect(renderer, &r);
            }
        }

        void ellipse(float x, float y, float width, float height) override {}
        void circle(float x, float y, float radius) override {}

        void line(const float x1, const float y1, const float x2, const float y2) override {
            if (renderer == nullptr) {
                return;
            }
            if (color_stroke.active) {
                set_color_f(color_stroke);
                SDL_RenderLine(renderer, x1, y1, x2, y2);
            }
        }

        void line(float x1, float y1, float z1, float x2, float y2, float z2) override {
            // TODO implement
        }

        void triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) override {
            // TODO implement
        }

        void bezier(float x1, float y1, float x2, float y2,
                    float x3, float y3, float x4, float y4) override {}
        void bezier(float x1, float y1, float z1, float x2, float y2, float z2,
                    float x3, float y3, float z3, float x4, float y4, float z4) override {}
        void bezierDetail(int detail) override {}

        void pointSize(float point_size) override {}

        void point(const float x, const float y, float z = 0.0f) override {
            if (renderer == nullptr) {
                return;
            }
            if (color_stroke.active) {
                set_color_f(color_stroke);
                SDL_RenderPoint(renderer, x, y);
            }
        }

        void beginShape(int shape = POLYGON) override {}
        void endShape(bool close_shape = false) override {
            // SDL_RenderGeometryRaw(SDL_Renderer *renderer,
            //                                    SDL_Texture *texture,
            //                                    const float *xy, int xy_stride,
            //                                    const SDL_FColor *color, int color_stride,
            //                                    const float *uv, int uv_stride,
            //                                    int num_vertices,
            //                                    const void *indices, int num_indices, int size_indices);
        }

        void   vertex(float x, float y, float z = 0.0f) override {}
        void   vertex(float x, float y, float z, float u, float v) override {}
        // PFont* loadFont(const std::string& file, float size) override { return nullptr; }
        void   textFont(PFont* font) override {}
        void   textSize(float size) override {}

        void text(const char* value, const float x, const float y, float z = 0.0f) override {
            if (renderer == nullptr) {
                return;
            }
            if (color_fill.active) {
                set_color_f(color_fill);
                SDL_RenderDebugText(renderer, x, y, value);
            }
        }

        float textWidth(const std::string& text) override { return 0; }

        // PImage* loadImage(const std::string& filename, bool use_relative_path) override {
        //     // auto* img = new PImage(filename);
        //     //
        //     // // Create SDL texture (RGBA channels)
        //     // SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, TEXTURE_WIDTH, TEXTURE_HEIGHT);
        //     // if (!texture) {
        //     //     std::cerr << "Failed to create texture! SDL_Error: " << SDL_GetError() << std::endl;
        //     // }
        //     //
        //     // // Upload pixel data to the texture
        //     // SDL_UpdateTexture(texture, NULL, pixels, img->width * sizeof(uint32_t));
        //     //
        //     // // Draw the texture
        //     // SDL_RenderTexture(renderer, texture, NULL, NULL); // Fullscreen
        //     //
        //     return nullptr;
        // }

        void image(PImage* img, float x, float y, float w, float h) override {}
        void image(PImage* img, float x, float y) override {}
        void texture(PImage* img) override {}
        void popMatrix() override {}
        void pushMatrix() override {}
        void translate(float x, float y, float z = 0.0f) override {}
        void rotateX(float angle) override {}
        void rotateY(float angle) override {}
        void rotateZ(float angle) override {}
        void rotate(float angle) override {}
        void rotate(float angle, float x, float y, float z) override {}
        void scale(float x) override {}
        void scale(float x, float y) override {}
        void scale(float x, float y, float z) override {}
        void pixelDensity(int density) override {}
        void hint(uint16_t property) override {}
        void text_str(const std::string& text, float x, float y, float z = 0.0f) override {}
        void beginDraw() override {}
        void endDraw() override {}
        void mesh(VertexBuffer* mesh_shape) override { /* TODO implement */ }

        void init(uint32_t* pixels, const int width, const int height) override {
            this->width  = width;
            this->height = height;
            // framebuffer.width  = width;
            // framebuffer.height = height;
        }

    private:
        static constexpr int ELLIPSE_NUM_SEGMENTS = 32;

        SDL_Renderer* renderer;

        [[maybe_unused]] float fPointSize             = 1;
        [[maybe_unused]] float fStrokeWeight          = 1;
        [[maybe_unused]] bool  fEnabledTextureInShape = false;
        [[maybe_unused]] bool  fShapeBegun            = false;
        [[maybe_unused]] int   fEllipseDetail         = 32;
        [[maybe_unused]] int   fBezierDetail          = 20;
        [[maybe_unused]] int   fPixelDensity          = 1;
        [[maybe_unused]] int   fPreviousFBO{};
    };
} // namespace umfeld
