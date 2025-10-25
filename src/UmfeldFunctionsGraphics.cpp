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

#include <string>
#include <sstream>

#include "Umfeld.h"
#include "UmfeldFunctionsGraphics.h"
#include "ShaderSource.h"

namespace umfeld {
    void background(const float gray) {
        if (g == nullptr) { return; }
        g->background(gray);
    }

    void background(const float r, const float g, const float b, const float a) {
        if (umfeld::g == nullptr) { return; }
        umfeld::g->background(r, g, b, a);
    }

    void background(PImage* img) {
        if (g == nullptr) { return; }
        g->background(img);
    }

    void beginShape(const int shape) {
        if (g == nullptr) { return; }
        g->beginShape(shape);
    }

    void endShape(const bool close_shape) {
        if (g == nullptr) { return; }
        g->endShape(close_shape);
    }

    void bezier(const float x1, const float y1, const float x2, const float y2, const float x3, const float y3, const float x4, const float y4) {
        if (g == nullptr) { return; }
        g->bezier(x1, y1, x2, y2, x3, y3, x4, y4);
    }

    void bezier(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2, const float x3, const float y3, const float z3, const float x4, const float y4, const float z4) {
        if (g == nullptr) { return; }
        g->bezier(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
    }

    void bezierDetail(const int detail) {
        if (g == nullptr) { return; }
        g->bezierDetail(detail);
    }

    void pointSize(const float point_size) {
        if (g == nullptr) { return; }
        g->pointSize(point_size);
    }

    void arc(const float x, const float y, const float width, const float height, const float start, const float stop, const int mode) {
        if (g == nullptr) { return; }
        g->arc(x, y, width, height, start, stop, mode);
    }

    void circle(const float x, const float y, const float diameter) {
        if (g == nullptr) { return; }
        g->circle(x, y, diameter);
    }

    void ellipse(const float x, const float y, const float width, const float height) {
        if (g == nullptr) { return; }
        g->ellipse(x, y, width, height);
    }

    void ellipseDetail(const int detail) {
        if (g == nullptr) { return; }
        g->ellipseDetail(detail);
    }

    void fill(const float r, const float g, const float b, const float a) {
        if (umfeld::g == nullptr) {
            return;
        }
        umfeld::g->fill(r, g, b, a);
    }

    void fill(const float brightness, const float alpha) {
        if (g == nullptr) { return; }
        g->fill(brightness, alpha);
    }

    void fill(const float brightness) {
        if (g == nullptr) { return; }
        g->fill(brightness);
    }

    void fill_color(const color_t c) {
        if (g == nullptr) { return; }
        g->fill_color(c);
    }

    void noFill() {
        if (g == nullptr) { return; }
        g->noFill();
    }

    void image(PImage* img, const float x, const float y, const float w, const float h) {
        if (g == nullptr) { return; }
        g->image(img, x, y, w, h);
    }

    void image(PImage* img, const float x, const float y) {
        if (g == nullptr) { return; }
        g->image(img, x, y);
    }

    void texture(PImage* img) {
        if (g == nullptr) { return; }
        g->texture(img);
    }

    void line(const float x1, const float y1, const float x2, const float y2) {
        if (g == nullptr) { return; }
        g->line(x1, y1, x2, y2);
    }

    void line(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2) {
        if (g == nullptr) { return; }
        g->line(x1, y1, z1, x2, y2, z2);
    }

    void triangle(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2, const float x3, const float y3, const float z3) {
        if (g == nullptr) { return; }
        g->triangle(x1, y1, z1, x2, y2, z2, x3, y3, z3);
    }

    void quad(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2, const float x3, const float y3, const float z3, const float x4, const float y4, const float z4) {
        if (g == nullptr) { return; }
        g->quad(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
    }

    void point(const float x, const float y, const float z) {
        if (g == nullptr) { return; }
        g->point(x, y, z);
    }

    void rect(const float x, const float y, const float width, const float height) {
        if (g == nullptr) { return; }
        g->rect(x, y, width, height);
    }

    void square(const float x, const float y, const float extent) {
        if (g == nullptr) { return; }
        g->square(x, y, extent);
    }

    void stroke(const float r, const float g, const float b, const float a) {
        if (umfeld::g == nullptr) {
            return;
        }
        umfeld::g->stroke(r, g, b, a);
    }

    void stroke(const float brightness, const float alpha) {
        if (g == nullptr) { return; }
        g->stroke(brightness, alpha);
    }

    void stroke(const float brightness) {
        if (g == nullptr) { return; }
        g->stroke(brightness);
    }

    void stroke_color(const color_t c) {
        if (g == nullptr) { return; }
        g->stroke_color(c);
    }

    void noStroke() {
        if (g == nullptr) { return; }
        g->noStroke();
    }

    void strokeWeight(const float weight) {
        if (g == nullptr) { return; }
        g->strokeWeight(weight);
    }

    /**
     *  can be MITER, BEVEL, ROUND, NONE, BEVEL_FAST or MITER_FAST
     * @param join
     */
    void strokeJoin(const int join) {
        if (g == nullptr) { return; }
        g->strokeJoin(join);
    }

    /**
     * can be PROJECT, ROUND, POINTED or SQUARE
     * @param cap
     */
    void strokeCap(const int cap) {
        if (g == nullptr) { return; }
        g->strokeCap(cap);
    }

    void vertex(const float x, const float y, const float z) {
        if (g == nullptr) { return; }
        g->vertex(x, y, z);
    }

    void vertex(const float x, const float y, const float z, const float u, const float v) {
        if (g == nullptr) { return; }
        g->vertex(x, y, z, u, v);
    }

    // PFont* loadFont(const std::string& file, const float size) {
    //     if (g == nullptr) {
    //         error("`loadFont` is only available after `settings()` has finished");
    //         return nullptr;
    //     }
    //     return g->loadFont(file, size);
    // }

    void textFont(PFont* font) {
        if (g == nullptr) { return; }
        g->textFont(font);
    }

    void textSize(const float size) {
        if (g == nullptr) { return; }
        g->textSize(size);
    }

    void text(const char* value, const float x, const float y, const float z) {
        text(std::string(value), x, y, z);
    }

    void text(const std::string& text, const float x, const float y, const float z) {
        if (g == nullptr) { return; }
        g->text(text, x, y, z);
    }

    float textWidth(const char c) {
        const auto text = std::string(1, c);
        return textWidth(text);
    }

    float textWidth(const std::string& text) {
        if (g == nullptr) {
            return 0.0f;
        }
        return g->textWidth(text);
    }

    void textAlign(const int alignX, const int alignY) {
        if (g == nullptr) { return; }
        g->textAlign(alignX, alignY);
    }

    float textAscent() {
        if (g == nullptr) {
            return 0.0f;
        }
        return g->textAscent();
    }

    float textDescent() {
        if (g == nullptr) {
            return 0.0f;
        }
        return g->textDescent();
    }

    void textLeading(const float leading) {
        if (g == nullptr) { return; }
        g->textLeading(leading);
    }

    void popMatrix() {
        if (g == nullptr) { return; }
        g->popMatrix();
    }

    void pushMatrix() {
        if (g == nullptr) { return; }
        g->pushMatrix();
    }

    void translate(const float x, const float y, const float z) {
        if (g == nullptr) { return; }
        g->translate(x, y, z);
    }

    void rotateX(const float angle) {
        if (g == nullptr) { return; }
        g->rotateX(angle);
    }

    void rotateY(const float angle) {
        if (g == nullptr) { return; }
        g->rotateY(angle);
    }

    void rotateZ(const float angle) {
        if (g == nullptr) { return; }
        g->rotateZ(angle);
    }

    void rotate(const float angle) {
        if (g == nullptr) { return; }
        g->rotate(angle);
    }

    void rotate(const float angle, const float x, const float y, const float z) {
        if (g == nullptr) { return; }
        g->rotate(angle, x, y, z);
    }

    void scale(const float x) {
        if (g == nullptr) { return; }
        g->scale(x);
    }

    void scale(const float x, const float y) {
        if (g == nullptr) { return; }
        g->scale(x, y);
    }

    void scale(const float x, const float y, const float z) {
        if (g == nullptr) { return; }
        g->scale(x, y, z);
    }

    void pixelDensity(const int density) {
        if (g == nullptr) {
            error("`pixelDensity` is only available after `settings()` has finished");
            return;
        }
        g->pixelDensity(density);
    }

    void rectMode(const int mode) {
        if (g == nullptr) { return; }
        g->rectMode(mode);
    }

    void ellipseMode(const int mode) {
        if (g == nullptr) { return; }
        g->ellipseMode(mode);
    }

    void blendMode(const BlendMode mode) {
        if (g == nullptr) { return; }
        g->blendMode(mode);
    }

    void hint(const uint16_t property) {
        if (g == nullptr) { return; }
        g->hint(property);
    }

    void box(const float size) {
        if (g == nullptr) { return; }
        g->box(size);
    }

    void box(const float width, const float height, const float depth) {
        if (g == nullptr) { return; }
        g->box(width, height, depth);
    }

    void sphere(const float size) {
        if (g == nullptr) { return; }
        g->sphere(size);
    }

    void sphere(const float width, const float height, const float depth) {
        if (g == nullptr) { return; }
        g->sphere(width, height, depth);
    }

    void sphereDetail(const int ures, const int vres) {
        if (g == nullptr) { return; }
        g->sphereDetail(ures, vres);
    }

    void sphereDetail(const int res) {
        if (g == nullptr) { return; }
        g->sphereDetail(res);
    }

    void mesh(VertexBuffer* mesh_shape) {
        if (g == nullptr) { return; }
        g->mesh(mesh_shape);
    }

    void shader(PShader* shader) {
        if (g == nullptr) { return; }
        g->shader(shader);
    }

    PShader* loadShader(const std::string& vertex_code, const std::string& fragment_code, const std::string& geometry_code) {
        if (g == nullptr) {
            error("`loadShader` is only available after `settings()` has finished");
            return nullptr;
        }
        return g->loadShader(vertex_code, fragment_code, geometry_code);
    }

    PShader* loadShader(const ShaderSource& shader_source) {
        return loadShader(shader_source.vertex, shader_source.fragment, shader_source.geometry);
    }

    void resetShader() {
        if (g == nullptr) { return; }
        g->resetShader();
    }

    void normal(const float x, const float y, const float z, const float w) {
        if (g == nullptr) { return; }
        g->normal(x, y, z, w);
    }

    // void beginCamera() {
    //     if (g == nullptr) {
    //         return;
    //     }
    //     g->beginCamera();
    // }
    //
    // void endCamera() {
    //     if (g == nullptr) {
    //         return;
    //     }
    //     g->endCamera();
    // }

    void camera(const float eyeX, const float eyeY, const float eyeZ, const float centerX, const float centerY, const float centerZ, const float upX, const float upY, const float upZ) {
        if (g == nullptr) { return; }
        g->camera(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);
    }

    void camera() {
        if (g == nullptr) { return; }
        g->camera();
    }

    void frustum(const float left, const float right, const float bottom, const float top, const float near, const float far) {
        if (g == nullptr) { return; }
        g->frustum(left, right, bottom, top, near, far);
    }

    void ortho(const float left, const float right, const float bottom, const float top, const float near, const float far) {
        if (g == nullptr) { return; }
        g->ortho(left, right, bottom, top, near, far);
    }

    void perspective(const float fovYDegrees, const float aspect, const float near, const float far) {
        if (g == nullptr) { return; }
        g->perspective(fovYDegrees, aspect, near, far);
    }

    void printCamera() {
        if (g == nullptr) { return; }
        g->printCamera();
    }

    void printProjection() {
        if (g == nullptr) { return; }
        g->printProjection();
    }

    void lights() {
        if (g == nullptr) { return; }
        g->lights();
    }

    void noLights() {
        if (g == nullptr) { return; }
        g->noLights();
    }

    /* additional */

    void flush() {
        if (g == nullptr) { return; }
        g->flush();
    }

    void debug_text(const std::string& text, const float x, const float y) {
        if (g == nullptr) { return; }
        g->debug_text(text, x, y);
    }

    void texture_filter(const TextureFilter filter) {
        if (g == nullptr) { return; }
        g->texture_filter(filter);
    }

    void texture_wrap(const TextureWrap wrap) {
        if (g == nullptr) { return; }
        g->texture_wrap(wrap);
    }

    void loadPixels(const bool update_logical_buffer) {
        if (g == nullptr) { return; }

        if (g->pixels == nullptr || pixels == nullptr) {
            error_in_function("pixels is null, cannot load pixels.");
            return;
        }

        g->download_colorbuffer(g->pixels);

        const int d      = g->displayDensity();
        const int phys_w = width * d;

        if (!update_logical_buffer || d <= 1) {
            return;
        }

        // NOTE down-scale pixels from physical to logical size
        //      `g->pixels` ( physical size (w*d*h*d) ) to `pixels` ( logical size (w*h) )

        const int width_i  = static_cast<int>(width);
        const int height_i = static_cast<int>(height);

        if (update_logical_buffer) {
            if (d == 2) {
                for (int y = 0; y < height_i; ++y) {
                    for (int x = 0; x < width_i; ++x) {
                        const uint32_t* p  = &g->pixels[(y * 2) * phys_w + x * 2];
                        const uint32_t  c0 = p[0];
                        const uint32_t  c1 = p[1];
                        const uint32_t  c2 = p[phys_w];
                        const uint32_t  c3 = p[phys_w + 1];

                        const uint32_t r = ((c0 >> 16) & 0xFF) + ((c1 >> 16) & 0xFF) +
                                           ((c2 >> 16) & 0xFF) + ((c3 >> 16) & 0xFF);
                        const uint32_t g_ = ((c0 >> 8) & 0xFF) + ((c1 >> 8) & 0xFF) +
                                            ((c2 >> 8) & 0xFF) + ((c3 >> 8) & 0xFF);
                        const uint32_t b = (c0 & 0xFF) + (c1 & 0xFF) +
                                           (c2 & 0xFF) + (c3 & 0xFF);
                        const uint32_t a = ((c0 >> 24) & 0xFF) + ((c1 >> 24) & 0xFF) +
                                           ((c2 >> 24) & 0xFF) + ((c3 >> 24) & 0xFF);
                        pixels[y * width_i + x] = ((a / 4) << 24) | ((r / 4) << 16) | ((g_ / 4) << 8) | (b / 4);
                    }
                }
            } else if (d > 2) {
                for (int y = 0; y < height_i; ++y) {
                    for (int x = 0; x < width_i; ++x) {
                        uint32_t r = 0, g_ = 0, b = 0, a = 0;

                        const int base_y = y * d;
                        const int base_x = x * d;

                        for (int dy = 0; dy < d; ++dy) {
                            const uint32_t* row = &g->pixels[(base_y + dy) * phys_w];
                            for (int dx = 0; dx < d; ++dx) {
                                const uint32_t color = row[base_x + dx];
                                r += (color >> 16) & 0xFF;
                                g_ += (color >> 8) & 0xFF;
                                b += (color >> 0) & 0xFF;
                                a += (color >> 24) & 0xFF;
                            }
                        }

                        const int area = d * d;
                        r /= area;
                        g_ /= area;
                        b /= area;
                        a /= area;

                        pixels[y * width_i + x] = (a << 24) | (r << 16) | (g_ << 8) | b;
                    }
                }
            }
        }
    }

    void updatePixels(const bool update_logical_buffer) {
        if (g == nullptr) {
            return;
        }

        if (g->pixels == nullptr || pixels == nullptr) {
            error_in_function("pixels is null, cannot load pixels.");
            return;
        }

        // NOTE up-scale pixels from logical to physical size
        //      `pixels` ( logical size (w*h) ) to `g->pixels` ( physical size (w*d*h*d) )

        const int d      = g->displayDensity();
        const int phys_w = width * d;

        if (update_logical_buffer) {
            if (d == 2) {
                const int width_i = static_cast<int>(width);
                for (int y = 0; y < height; ++y) {
                    const uint32_t* src_row   = &pixels[y * width_i];
                    uint32_t*       dest_row0 = &g->pixels[(y * 2 + 0) * phys_w];
                    uint32_t*       dest_row1 = &g->pixels[(y * 2 + 1) * phys_w];
                    for (int x = 0; x < width; ++x) {
                        const uint32_t c = src_row[x];
                        const int      i = x * 2;
                        dest_row0[i + 0] = c;
                        dest_row0[i + 1] = c;
                        dest_row1[i + 0] = c;
                        dest_row1[i + 1] = c;
                    }
                }
            } else if (d > 2) {
                const int width_i = static_cast<int>(width);
                for (int y = 0; y < height; ++y) {
                    const uint32_t* src_row = &pixels[y * width_i];
                    for (int dy = 0; dy < d; ++dy) {
                        uint32_t* dest_row = &g->pixels[(y * d + dy) * phys_w];
                        for (int x = 0; x < width; ++x) {
                            const uint32_t c = src_row[x];
                            for (int dx = 0; dx < d; ++dx) {
                                dest_row[x * d + dx] = c;
                            }
                        }
                    }
                }
            }
        }

        g->upload_colorbuffer(g->pixels);
    }
} // namespace umfeld