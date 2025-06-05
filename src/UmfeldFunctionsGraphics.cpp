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
    void background(const float a) {
        if (g == nullptr) {
            return;
        }
        g->background(a);
    }

    void background(const float a, const float b, const float c, const float d) {
        if (g == nullptr) {
            return;
        }
        g->background(a, b, c, d);
    }

    void background(PImage* img) {
        if (g == nullptr) {
            return;
        }
        g->background(img);
    }

    void beginShape(const int shape) {
        if (g == nullptr) {
            return;
        }
        g->beginShape(shape);
    }

    void endShape(const bool close_shape) {
        if (g == nullptr) {
            return;
        }
        g->endShape(close_shape);
    }

    void bezier(const float x1, const float y1, const float x2, const float y2, const float x3, const float y3, const float x4, const float y4) {
        if (g == nullptr) {
            return;
        }
        g->bezier(x1, y1, x2, y2, x3, y3, x4, y4);
    }

    void bezier(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2, const float x3, const float y3, const float z3, const float x4, const float y4, const float z4) {
        if (g == nullptr) {
            return;
        }
        g->bezier(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
    }

    void bezierDetail(const int detail) {
        if (g == nullptr) {
            return;
        }
        g->bezierDetail(detail);
    }

    void pointSize(const float point_size) {
        if (g == nullptr) {
            return;
        }
        g->pointSize(point_size);
    }

    void arc(const float x, const float y, const float width, const float height, const float start, const float stop, const int mode) {
        if (g == nullptr) {
            return;
        }
        g->arc(x, y, width, height, start, stop, mode);
    }

    void circle(const float x, const float y, const float diameter) {
        if (g == nullptr) {
            return;
        }
        g->circle(x, y, diameter);
    }

    void ellipse(const float x, const float y, const float width, const float height) {
        if (g == nullptr) {
            return;
        }
        g->ellipse(x, y, width, height);
    }

    void ellipseDetail(const int detail) {
        if (g == nullptr) {
            return;
        }
        g->ellipseDetail(detail);
    }

    void fill(const float r, const float g, const float b, const float a) {
        if (umfeld::g == nullptr) {
            return;
        }
        umfeld::g->fill(r, g, b, a);
    }

    void fill(const float brightness, const float a) {
        if (g == nullptr) {
            return;
        }
        g->fill(brightness, a);
    }

    void fill(const float a) {
        if (g == nullptr) {
            return;
        }
        g->fill(a);
    }

    void fill_color(const uint32_t c) {
        if (g == nullptr) {
            return;
        }
        g->fill_color(c);
    }

    void noFill() {
        if (g == nullptr) {
            return;
        }
        g->noFill();
    }

    void image(PImage* img, const float x, const float y, const float w, const float h) {
        if (g == nullptr) {
            return;
        }
        g->image(img, x, y, w, h);
    }

    void image(PImage* img, const float x, const float y) {
        if (g == nullptr) {
            return;
        }
        g->image(img, x, y);
    }

    void texture(PImage* img) {
        if (g == nullptr) {
            return;
        }
        g->texture(img);
    }

    PImage* loadImage(const std::string& filename, const bool use_relative_path) {
        if (g == nullptr) {
            return nullptr;
        }
        return g->loadImage(filename, use_relative_path);
    }

    void line(const float x1, const float y1, const float x2, const float y2) {
        if (g == nullptr) {
            return;
        }
        g->line(x1, y1, x2, y2);
    }

    void line(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2) {
        if (g == nullptr) {
            return;
        }
        g->line(x1, y1, z1, x2, y2, z2);
    }

    void triangle(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2, const float x3, const float y3, const float z3) {
        if (g == nullptr) {
            return;
        }
        g->triangle(x1, y1, z1, x2, y2, z2, x3, y3, z3);
    }

    void quad(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2, const float x3, const float y3, const float z3, const float x4, const float y4, const float z4) {
        if (g == nullptr) {
            return;
        }
        g->quad(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
    }

    void point(const float x, const float y, const float z) {
        if (g == nullptr) {
            return;
        }
        g->point(x, y, z);
    }

    void rect(const float x, const float y, const float width, const float height) {
        if (g == nullptr) {
            return;
        }
        g->rect(x, y, width, height);
    }

    void square(const float x, const float y, const float extent) {
        if (g == nullptr) {
            return;
        }
        g->square(x, y, extent);
    }

    void stroke(const float r, const float g, const float b, const float a) {
        if (umfeld::g == nullptr) {
            return;
        }
        umfeld::g->stroke(r, g, b, a);
    }

    void stroke(const float brightness, const float a) {
        if (g == nullptr) {
            return;
        }
        g->stroke(brightness, a);
    }

    void stroke(const float a) {
        if (g == nullptr) {
            return;
        }
        g->stroke(a);
    }

    void stroke_color(const uint32_t c) {
        if (g == nullptr) {
            return;
        }
        g->stroke_color(c);
    }

    void noStroke() {
        if (g == nullptr) {
            return;
        }
        g->noStroke();
    }

    void strokeWeight(const float weight) {
        if (g == nullptr) {
            return;
        }
        g->strokeWeight(weight);
    }

    /**
     *  can be MITER, BEVEL, ROUND, NONE, BEVEL_FAST or MITER_FAST
     * @param join
     */
    void strokeJoin(const int join) {
        if (g == nullptr) {
            return;
        }
        g->strokeJoin(join);
    }

    /**
     * can be PROJECT, ROUND, POINTED or SQUARE
     * @param cap
     */
    void strokeCap(const int cap) {
        if (g == nullptr) {
            return;
        }
        g->strokeCap(cap);
    }

    void vertex(const float x, const float y, const float z) {
        if (g == nullptr) {
            return;
        }
        g->vertex(x, y, z);
    }

    void vertex(const float x, const float y, const float z, const float u, const float v) {
        if (g == nullptr) {
            return;
        }
        g->vertex(x, y, z, u, v);
    }

    PFont* loadFont(const std::string& file, const float size) {
        if (g == nullptr) {
            error("`loadFont` is only available after `settings()` has finished");
            return nullptr;
        }
        return g->loadFont(file, size);
    }

    void textFont(PFont* font) {
        if (g == nullptr) {
            return;
        }
        g->textFont(font);
    }

    void textSize(const float size) {
        if (g == nullptr) {
            return;
        }
        g->textSize(size);
    }

    void text(const char* value, const float x, const float y, const float z) {
        text(std::string(value), x, y, z);
    }

    void text(const std::string& text, const float x, const float y, const float z) {
        if (g == nullptr) {
            return;
        }
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
        if (g == nullptr) {
            return;
        }
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
        if (g == nullptr) {
            return;
        }
        g->textLeading(leading);
    }

    void popMatrix() {
        if (g == nullptr) {
            return;
        }
        g->popMatrix();
    }

    void pushMatrix() {
        if (g == nullptr) {
            return;
        }
        g->pushMatrix();
    }

    void translate(const float x, const float y, const float z) {
        if (g == nullptr) {
            return;
        }
        g->translate(x, y, z);
    }

    void rotateX(const float angle) {
        if (g == nullptr) {
            return;
        }
        g->rotateX(angle);
    }

    void rotateY(const float angle) {
        if (g == nullptr) {
            return;
        }
        g->rotateY(angle);
    }

    void rotateZ(const float angle) {
        if (g == nullptr) {
            return;
        }
        g->rotateZ(angle);
    }

    void rotate(const float angle) {
        if (g == nullptr) {
            return;
        }
        g->rotate(angle);
    }

    void rotate(const float angle, const float x, const float y, const float z) {
        if (g == nullptr) {
            return;
        }
        g->rotate(angle, x, y, z);
    }

    void scale(const float x) {
        if (g == nullptr) {
            return;
        }
        g->scale(x);
    }

    void scale(const float x, const float y) {
        if (g == nullptr) {
            return;
        }
        g->scale(x, y);
    }

    void scale(const float x, const float y, const float z) {
        if (g == nullptr) {
            return;
        }
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
        if (g == nullptr) {
            return;
        }
        g->rectMode(mode);
    }

    void ellipseMode(const int mode) {
        if (g == nullptr) {
            return;
        }
        g->ellipseMode(mode);
    }

    void hint(const uint16_t property) {
        if (g == nullptr) {
            return;
        }
        g->hint(property);
    }

    void box(const float size) {
        if (g == nullptr) {
            return;
        }
        g->box(size);
    }

    void box(const float width, const float height, const float depth) {
        if (g == nullptr) {
            return;
        }
        g->box(width, height, depth);
    }

    void sphere(const float size) {
        if (g == nullptr) {
            return;
        }
        g->sphere(size);
    }

    void sphere(const float width, const float height, const float depth) {
        if (g == nullptr) {
            return;
        }
        g->sphere(width, height, depth);
    }

    void sphereDetail(const int ures, const int vres) {
        if (g == nullptr) {
            return;
        }
        g->sphereDetail(ures, vres);
    }

    void sphereDetail(const int res) {
        if (g == nullptr) {
            return;
        }
        g->sphereDetail(res);
    }

    void mesh(VertexBuffer* mesh_shape) {
        if (g == nullptr) {
            return;
        }
        g->mesh(mesh_shape);
    }

    void shader(PShader* shader) {
        if (g == nullptr) {
            return;
        }
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
        if (g == nullptr) {
            return;
        }
        g->resetShader();
    }

    void normal(const float x, const float y, const float z, const float w) {
        if (g == nullptr) {
            return;
        }
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
        if (g == nullptr) {
            return;
        }
        g->camera(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);
    }

    void camera() {
        if (g == nullptr) {
            return;
        }
        g->camera();
    }

    void frustum(const float left, const float right, const float bottom, const float top, const float near, const float far) {
        if (g == nullptr) {
            return;
        }
        g->frustum(left, right, bottom, top, near, far);
    }

    void ortho(const float left, const float right, const float bottom, const float top, const float near, const float far) {
        if (g == nullptr) {
            return;
        }
        g->ortho(left, right, bottom, top, near, far);
    }

    void perspective(const float fovYDegrees, const float aspect, const float near, const float far) {
        if (g == nullptr) {
            return;
        }
        g->perspective(fovYDegrees, aspect, near, far);
    }

    void printCamera() {
        if (g == nullptr) {
            return;
        }
        g->printCamera();
    }

    void printProjection() {
        if (g == nullptr) {
            return;
        }
        g->printProjection();
    }

    void lights() {
        if (g == nullptr) {
            return;
        }
        g->lights();
    }

    /* additional */

    void debug_text(const std::string& text, const float x, const float y) {
        if (g == nullptr) {
            return;
        }
        g->debug_text(text, x, y);
    }
} // namespace umfeld