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

#include <iostream>
#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "earcut.hpp"
#include "polypartition.h"
#include "clipper2/clipper.h"

#include "Umfeld.h"
#include "UmfeldFunctionsGraphics.h"
#include "UmfeldFunctionsAdditional.h"
#include "PGraphics.h"
#include "Vertex.h"
#include "Geometry.h"
#include "UShapeRenderer.h"
#include "VertexBuffer.h"

using namespace umfeld;

PGraphics::PGraphics() : PImage(0, 0), debug_font(new UFont()) {
    flip_y_texcoords = true;
    PGraphics::fill_f(1.0f);
    PGraphics::stroke_f(0.0f);
    PGraphics::ellipseDetail(ELLIPSE_DETAIL_DEFAULT);
    generate_box(box_fill_vertices_LUT, true);
    generate_box(box_stroke_vertices_LUT, false);
    generate_sphere(sphere_vertices_LUT, sphere_u_resolution, sphere_u_resolution);
}

void PGraphics::beginDraw() {
    reset_mvp_matrices();
}

void PGraphics::endDraw() {
    flush();
    restore_mvp_matrices();
    lights_enabled     = false;
    current_shape.mode = POLYGON;
    texture_stack_top  = nullptr;
    texture_stack_used = false;
    resetShader();
}

void PGraphics::shader(PShader* shader) {
    current_custom_shader = shader;
    if (shader == nullptr) {
        resetShader();
    }
}

void PGraphics::resetShader() {
    current_custom_shader = nullptr;
}

void PGraphics::flush() {
    if (shape_renderer) {
        shape_renderer->flush(view_matrix, projection_matrix);
    }
}

void PGraphics::mesh(VertexBuffer* mesh_shape) {
    if (shape_renderer == nullptr || mesh_shape == nullptr) {
        return;
    }
    UShape s;
    // NOTE ignore 'mode'
    s.filled = true;
    // NOTE ignore 'vertices'
    s.model_matrix = model_matrix;
    s.transparent  = mesh_shape->get_transparent();
    // NOTE ignore 'closed'
    s.texture_id    = get_current_texture_id();
    s.light_enabled = lights_enabled; // TODO not properly supported WIP
    if (lights_enabled) {
        s.lighting = lightingState;
    }
    s.shader        = current_custom_shader;
    s.vertex_buffer = mesh_shape;
    shape_renderer->submit_shape(s);
    if (render_mode == RENDER_MODE_IMMEDIATELY) {
        flush();
    }
}

void PGraphics::hint(const uint16_t property) {
    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    switch (property) {
        case ENABLE_DEPTH_TEST: {
            hint_force_enable_depth_test = true;
        } break;
        case DISABLE_DEPTH_TEST: {
            hint_force_enable_depth_test = false;
        } break;
    }
}

int PGraphics::displayDensity() {
    return pixel_density;
}

void PGraphics::pixelDensity(const int density) {
    if (init_properties_locked) {
        resize(width, height);
    }
    pixel_density = density;
}

void PGraphics::stroke_properties(const float stroke_join_round_resolution,
                                  const float stroke_cap_round_resolution,
                                  const float stroke_join_miter_max_angle) {
    this->current_stroke_state.stroke_join_round_resolution = stroke_join_round_resolution;
    this->current_stroke_state.stroke_cap_round_resolution  = stroke_cap_round_resolution;
    this->current_stroke_state.stroke_join_miter_max_angle  = stroke_join_miter_max_angle;
}

// ## Color

// ### Creating & Reading

float PGraphics::alpha(const color_t color) const {
    float alpha = static_cast<float>((color & 0x000000FF) >> 0) / 255.0f;
    alpha       = map(alpha, 0.0, 1.0, 0.0, color_mode_state.range.a);
    return alpha;
}

float PGraphics::blue(const color_t color) const {
    float blue = static_cast<float>((color & 0x0000FF00) >> 8) / 255.0f;
    blue       = map(blue, 0.0, 1.0, 0.0, color_mode_state.range.b);
    return blue;
}

float PGraphics::green(const color_t color) const {
    float green = static_cast<float>((color & 0x00FF0000) >> 16) / 255.0f;
    green       = map(green, 0.0, 1.0, 0.0, color_mode_state.range.g);
    return green;
}

float PGraphics::red(const color_t color) const {
    float red = static_cast<float>((color & 0xFF000000) >> 24) / 255.0f;
    red       = map(red, 0.0, 1.0, 0.0, color_mode_state.range.r);
    return red;
}

float PGraphics::hue(const uint32_t color) const {
    const float r = static_cast<float>((color & 0xFF000000) >> 24) / 255.0f;
    const float g = static_cast<float>((color & 0x00FF0000) >> 16) / 255.0f;
    const float b = static_cast<float>((color & 0x0000FF00) >> 8) / 255.0f;
    float       h, s, v;
    rgb_to_hsb_f(r, g, b, h, s, v);
    h = map(h, 0.0, 1.0, 0.0, color_mode_state.range.r);
    return h;
}

float PGraphics::saturation(const uint32_t color) const {
    const float r = static_cast<float>((color & 0xFF000000) >> 24) / 255.0f;
    const float g = static_cast<float>((color & 0x00FF0000) >> 16) / 255.0f;
    const float b = static_cast<float>((color & 0x0000FF00) >> 8) / 255.0f;
    float       h, s, v;
    rgb_to_hsb_f(r, g, b, h, s, v);
    s = map(s, 0.0, 1.0, 0.0, color_mode_state.range.g);
    return s;
}


float PGraphics::brightness(const uint32_t color) const {
    const float r = static_cast<float>((color & 0xFF000000) >> 24) / 255.0f;
    const float g = static_cast<float>((color & 0x00FF0000) >> 16) / 255.0f;
    const float b = static_cast<float>((color & 0x0000FF00) >> 8) / 255.0f;
    float       h, s, v;
    rgb_to_hsb_f(r, g, b, h, s, v);
    v = map(v, 0.0, 1.0, 0.0, color_mode_state.range.b);
    return v;
}

color_t PGraphics::color(const float gray) {
    return color(gray, gray, gray, color_mode_state.range.a);
}

color_t PGraphics::color(const float gray, const float alpha) {
    return color(gray, gray, gray, alpha);
}

color_t PGraphics::color(const float v1, const float v2, const float v3) {
    return color(v1, v2, v3, color_mode_state.range.a);
}

color_t PGraphics::color(const float v1, const float v2, const float v3, const float alpha) {
    glm::vec4 _color;
    interpret_color_mode(_color, v1, v2, v3, alpha);
    return color_f(_color.r, _color.g, _color.b, _color.a);
}

color_t PGraphics::color_f(const float brightness, const float alpha) {
    return color_f(brightness, brightness, brightness, alpha);
}

color_t PGraphics::color_f(const float r, const float g, const float b) {
    return color_f(r, g, b, 1.0);
}

color_t PGraphics::color_f(const float r, const float g, const float b, const float a) {
    return static_cast<uint32_t>(a * 255) << 24 |
           static_cast<uint32_t>(b * 255) << 16 |
           static_cast<uint32_t>(g * 255) << 8 |
           static_cast<uint32_t>(r * 255);
}

color_t PGraphics::lerpColor(const color_t c1, const color_t c2, float amt) const {
    amt = std::clamp(amt, 0.0f, 1.0f);
    glm::vec4 cn1;
    color_unpack_f(c1, cn1.r, cn1.g, cn1.b, cn1.a);
    glm::vec4 cn2;
    color_unpack_f(c2, cn2.r, cn2.g, cn2.b, cn2.a);
    glm::vec4 ci;

    ci.r = cn1.r * (1 - amt) + cn2.r * amt;
    ci.g = cn1.g * (1 - amt) + cn2.g * amt;
    ci.b = cn1.b * (1 - amt) + cn2.b * amt;
    ci.a = cn1.a * (1 - amt) + cn2.a * amt;

    return color_pack_f(ci.r, ci.g, ci.b, ci.a);
}

// ### Setting

void PGraphics::background(const float gray) {
    background(gray, gray, gray, 1.0);
}

void PGraphics::background(const float gray, const float alpha) {
    background(gray, gray, gray, alpha);
}

void PGraphics::background(const float v1, const float v2, const float v3) {
    background(v1, v2, v3, color_mode_state.range.a);
}

void PGraphics::background(const float v1, const float v2, const float v3, const float alpha) {
    glm::vec4 _color;
    interpret_color_mode(_color, v1, v2, v3, alpha);
    background_f(_color.r, _color.g, _color.b, _color.a);
}


void PGraphics::background(PImage* img) {
    background_f(0, 0, 0, 0);
    fill_f(1);
    image(img, 0, 0, framebuffer.width, framebuffer.height);
}

void PGraphics::background_f(const float a) {
    background_f(a, a, a);
}

void PGraphics::background_f(const float a, const float b, const float c, const float d) {
    static bool emitted_warning = false;
    if (!emitted_warning) {
        if (a < 0 || a > 1 || b < 0 || b > 1 || c < 0 || c > 1 || d < 0 || d > 1) {
            warning("`background_f()` values should be in range [0, 1].");
        }
        emitted_warning = true;
    }
}

void PGraphics::background_color(const color_t color) {
    glm::vec4 _color;
    color_unpack_f(color, _color.r, _color.g, _color.b, _color.a);
    background_f(_color.r, _color.g, _color.b, _color.a);
}

void PGraphics::colorMode(const ColorMode mode, const float max) {
    color_mode_state.mode    = mode;
    color_mode_state.range.r = max;
    color_mode_state.range.g = max;
    color_mode_state.range.b = max;
    color_mode_state.range.a = max;
}

void PGraphics::colorMode(const ColorMode mode, const float max1, const float max2, const float max3) {
    color_mode_state.mode    = mode;
    color_mode_state.range.r = max1;
    color_mode_state.range.g = max2;
    color_mode_state.range.b = max3;
}

void PGraphics::colorMode(const ColorMode mode, const float max1, const float max2, const float max3, const float maxA) {
    color_mode_state.mode    = mode;
    color_mode_state.range.r = max1;
    color_mode_state.range.g = max2;
    color_mode_state.range.b = max3;
    color_mode_state.range.a = maxA;
}

void PGraphics::fill(const float gray) {
    fill(gray, gray, gray, color_mode_state.range.a);
}

void PGraphics::fill(const float gray, const float alpha) {
    fill(gray, gray, gray, alpha);
}

void PGraphics::fill(const float v1, const float v2, const float v3) {
    fill(v1, v2, v3, color_mode_state.range.a);
}

void PGraphics::fill(const float v1, const float v2, const float v3, const float alpha) {
    interpret_color_mode(color_fill, v1, v2, v3, alpha);
    color_fill.active = true;
}

void PGraphics::fill_f(const float gray, const float alpha) {
    fill_f(gray, gray, gray, alpha);
}

void PGraphics::fill_f(const float r, const float g, const float b, const float alpha) {
    static bool emitted_warning = false;
    if (!emitted_warning) {
        if (r < 0 || r > 1 || g < 0 || g > 1 || b < 0 || b > 1 || alpha < 0 || alpha > 1) {
            warning("`fill_f()` values should be in range [0, 1].");
        }
        emitted_warning = true;
    }
    color_fill.r      = r;
    color_fill.g      = g;
    color_fill.b      = b;
    color_fill.a      = alpha;
    color_fill.active = true;
}

void PGraphics::fill_color(const color_t c) {
    color_unpack_f(c, color_fill.r, color_fill.g, color_fill.b, color_fill.a);
    color_fill.active = true;
}

void PGraphics::noFill() {
    color_fill.active = false;
}

void PGraphics::stroke(const float gray) {
    stroke(gray, gray, gray, color_mode_state.range.a);
}

void PGraphics::stroke(const float gray, const float alpha) {
    stroke(gray, gray, gray, alpha);
}

void PGraphics::stroke(const float v1, const float v2, const float v3) {
    stroke(v1, v2, v3, color_mode_state.range.a);
}

void PGraphics::stroke(const float v1, const float v2, const float v3, const float alpha) {
    interpret_color_mode(color_stroke, v1, v2, v3, alpha);
    color_stroke.active = true;
}

void PGraphics::stroke_f(const float r, const float g, const float b, const float alpha) {
    static bool emitted_warning = false;
    if (!emitted_warning) {
        if (r < 0 || r > 1 || g < 0 || g > 1 || b < 0 || b > 1 || alpha < 0 || alpha > 1) {
            warning("`stroke_f()` values should be in range [0, 1].");
        }
        emitted_warning = true;
    }

    color_stroke.r      = r;
    color_stroke.g      = g;
    color_stroke.b      = b;
    color_stroke.a      = alpha;
    color_stroke.active = true;
}

void PGraphics::stroke_f(const float gray, const float alpha) {
    static bool emitted_warning = false;
    if (!emitted_warning) {
        if (gray < 0 || gray > 1 || alpha < 0 || alpha > 1) {
            warning("`stroke()` values should be in range [0, 1].");
        }
        emitted_warning = true;
    }

    color_stroke.r      = gray;
    color_stroke.g      = gray;
    color_stroke.b      = gray;
    color_stroke.a      = alpha;
    color_stroke.active = true;
}

void PGraphics::stroke_color(const color_t c) {
    color_unpack_f(c, color_stroke.r, color_stroke.g, color_stroke.b, color_stroke.a);
    color_stroke.active = true;
}

void PGraphics::noStroke() {
    color_stroke.active = false;
}

/* --- transform matrices --- */

void PGraphics::popMatrix() {
    if (!model_matrix_stack.empty()) {
        model_matrix = model_matrix_stack.back();
        model_matrix_stack.pop_back();
    }
}

void PGraphics::pushMatrix() {
    model_matrix_stack.push_back(model_matrix);
}

void PGraphics::resetMatrix() { // NOTE this just resets the model view matrix
    model_matrix = glm::mat4(1.0f);
}

void PGraphics::printMatrix(const glm::mat4& matrix) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << matrix[j][i] << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void PGraphics::printMatrix() {
    printMatrix(model_matrix);
}

void PGraphics::translate(const float x, const float y, const float z) {
    model_matrix       = glm::translate(model_matrix, glm::vec3(x, y, z));
    model_matrix_dirty = true;
}

void PGraphics::rotateX(const float angle) {
    model_matrix       = glm::rotate(model_matrix, angle, glm::vec3(1.0f, 0.0f, 0.0f));
    model_matrix_dirty = true;
}

void PGraphics::rotateY(const float angle) {
    model_matrix       = glm::rotate(model_matrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    model_matrix_dirty = true;
}

void PGraphics::rotateZ(const float angle) {
    model_matrix       = glm::rotate(model_matrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    model_matrix_dirty = true;
}

void PGraphics::rotate(const float angle) {
    model_matrix       = glm::rotate(model_matrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    model_matrix_dirty = true;
}

void PGraphics::rotate(const float angle, const float x, const float y, const float z) {
    model_matrix       = glm::rotate(model_matrix, angle, glm::vec3(x, y, z));
    model_matrix_dirty = true;
}

void PGraphics::scale(const float x) {
    model_matrix       = glm::scale(model_matrix, glm::vec3(x, x, x));
    model_matrix_dirty = true;
}

void PGraphics::scale(const float x, const float y) {
    model_matrix       = glm::scale(model_matrix, glm::vec3(x, y, 1));
    model_matrix_dirty = true;
}

void PGraphics::scale(const float x, const float y, const float z) {
    model_matrix       = glm::scale(model_matrix, glm::vec3(x, y, z));
    model_matrix_dirty = true;
}

void PGraphics::strokeWeight(const float weight) {
    current_stroke_state.stroke_weight = weight;
}

/**
 *  can be MITER, BEVEL, ROUND, NONE, BEVEL_FAST or MITER_FAST
 * @param join
 */
void PGraphics::strokeJoin(const int join) {
    current_stroke_state.stroke_join_mode = join;
}

/**
 * can be PROJECT, ROUND, POINTED or SQUARE
 * @param cap
 */
void PGraphics::strokeCap(const int cap) {
    current_stroke_state.stroke_cap_mode = cap;
}

void PGraphics::rectMode(const int mode) {
    rect_mode = mode;
}

void PGraphics::ellipseMode(const int mode) {
    ellipse_mode = mode;
}

void PGraphics::ellipseDetail(const int detail) {
    if (ellipse_detail == detail) {
        return;
    }
    if (detail < ELLIPSE_DETAIL_MIN) {
        return;
    }
    ellipse_detail = detail;
    resize_ellipse_points_LUT();
}

void PGraphics::bezier(const float x1, const float y1,
                       const float x2, const float y2,
                       const float x3, const float y3,
                       const float x4, const float y4) {
    if (!color_stroke.active) {
        return;
    }

    if (bezier_detail < 2) {
        return;
    }

    const int   segments = bezier_detail;
    const float step     = 1.0f / static_cast<float>(segments);

    beginShape(LINE_STRIP);
    for (int i = 0; i < segments + 1; ++i) {
        const float t = static_cast<float>(i) * step;
        const float u = 1.0f - t;

        const float b0 = u * u * u;
        const float b1 = 3 * u * u * t;
        const float b2 = 3 * u * t * t;
        const float b3 = t * t * t;

        const float x = b0 * x1 + b1 * x2 + b2 * x3 + b3 * x4;
        const float y = b0 * y1 + b1 * y2 + b2 * y3 + b3 * y4;

        vertex(x, y);
    }
    endShape();
}

void PGraphics::bezier(const float x1, const float y1, const float z1,
                       const float x2, const float y2, const float z2,
                       const float x3, const float y3, const float z3,
                       const float x4, const float y4, const float z4) {
    if (!color_stroke.active) {
        return;
    }
    if (bezier_detail < 2) {
        return;
    }

    const int   segments = bezier_detail;
    const float step     = 1.0f / static_cast<float>(segments);

    beginShape(LINE_STRIP);
    for (int i = 0; i < segments + 1; ++i) {
        const float t = static_cast<float>(i) * step;
        const float u = 1.0f - t;

        const float b0 = u * u * u;
        const float b1 = 3 * u * u * t;
        const float b2 = 3 * u * t * t;
        const float b3 = t * t * t;

        const float x = b0 * x1 + b1 * x2 + b2 * x3 + b3 * x4;
        const float y = b0 * y1 + b1 * y2 + b2 * y3 + b3 * y4;
        const float z = b0 * z1 + b1 * z2 + b2 * z3 + b3 * z4;

        vertex(x, y, z);
    }
    endShape();
}

void PGraphics::bezierDetail(const int detail) {
    bezier_detail = detail;
}

static glm::vec2 hermite(const float t, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& m1, const glm::vec2& m2) {
    const float t2 = t * t;
    const float t3 = t2 * t;

    return (2.0f * t3 - 3.0f * t2 + 1.0f) * p1 +
           (t3 - 2.0f * t2 + t) * m1 +
           (-2.0f * t3 + 3.0f * t2) * p2 +
           (t3 - t2) * m2;
}

void PGraphics::curve(const float x1, const float y1,
                      const float x2, const float y2,
                      const float x3, const float y3,
                      const float x4, const float y4) {
    const glm::vec2 p1(x1, y1);
    const glm::vec2 p2(x2, y2);
    const glm::vec2 p3(x3, y3);
    const glm::vec2 p4(x4, y4);

    const glm::vec2 m1 = (1.0f - curve_tightness) * 0.5f * (p3 - p1);
    const glm::vec2 m2 = (1.0f - curve_tightness) * 0.5f * (p4 - p2);

    const int segments = curve_detail;
    glm::vec2 prev     = p2;

    for (int i = 1; i <= segments; ++i) {
        const float     t  = i / static_cast<float>(segments);
        const glm::vec2 pt = hermite(t, p2, p3, m1, m2);
        line(prev.x, prev.y, pt.x, pt.y); // your existing line() function
        prev = pt;
    }
}

static glm::vec3 hermite(const float t, const glm::vec3& p1, const glm::vec3& p2,
                         const glm::vec3& m1, const glm::vec3& m2) {
    const float t2 = t * t;
    const float t3 = t2 * t;

    return (2.0f * t3 - 3.0f * t2 + 1.0f) * p1 +
           (t3 - 2.0f * t2 + t) * m1 +
           (-2.0f * t3 + 3.0f * t2) * p2 +
           (t3 - t2) * m2;
}

void PGraphics::curve(const float x1, const float y1, const float z1,
                      const float x2, const float y2, const float z2,
                      const float x3, const float y3, const float z3,
                      const float x4, const float y4, const float z4) {
    const glm::vec3 p1(x1, y1, z1);
    const glm::vec3 p2(x2, y2, z2);
    const glm::vec3 p3(x3, y3, z3);
    const glm::vec3 p4(x4, y4, z4);

    const glm::vec3 m1 = (1.0f - curve_tightness) * 0.5f * (p3 - p1);
    const glm::vec3 m2 = (1.0f - curve_tightness) * 0.5f * (p4 - p2);

    const int segments = curve_detail;
    glm::vec3 prev     = p2;

    for (int i = 1; i <= segments; ++i) {
        const float     t  = i / static_cast<float>(segments);
        const glm::vec3 pt = hermite(t, p2, p3, m1, m2);
        line(prev.x, prev.y, prev.z, pt.x, pt.y, pt.z); // 3D line function
        prev = pt;
    }
}

void PGraphics::curveDetail(const int detail) {
    curve_detail = detail;
}

void PGraphics::curveTightness(const float tightness) {
    curve_tightness = tightness;
}

// std::vector<glm::vec3> curve_vertices;
// void PGraphics::beginShape() {
//     curve_vertices.clear();
//     // other shape setup
// }
// void PGraphics::curveVertex(float x, float y, float z) {
//     curve_vertices.emplace_back(x, y, z);
//
//     if (curve_vertices.size() < 4) return;
//
//     const glm::vec3& p1 = curve_vertices[curve_vertices.size() - 4];
//     const glm::vec3& p2 = curve_vertices[curve_vertices.size() - 3];
//     const glm::vec3& p3 = curve_vertices[curve_vertices.size() - 2];
//     const glm::vec3& p4 = curve_vertices[curve_vertices.size() - 1];
//
//     const glm::vec3 m1 = (1.0f - curve_tightness) * 0.5f * (p3 - p1);
//     const glm::vec3 m2 = (1.0f - curve_tightness) * 0.5f * (p4 - p2);
//
//     const int segments = curve_detail;
//     glm::vec3 prev     = p2;
//
//     for (int i = 1; i <= segments; ++i) {
//         const float     t  = i / float(segments);
//         const glm::vec3 pt = hermite(t, p2, p3, m1, m2);
//         line(prev.x, prev.y, prev.z, pt.x, pt.y, pt.z);
//         prev = pt;
//     }
// }

void PGraphics::pushStyle() {
    style_stack.push(StyleState{
        color_stroke,
        color_fill,
        current_stroke_state.stroke_weight // assuming you have this
    });
}

void PGraphics::popStyle() {
    if (style_stack.empty()) {
        return;
    }

    const StyleState s = style_stack.top();
    style_stack.pop();

    color_stroke                       = s.stroke;
    color_fill                         = s.fill;
    current_stroke_state.stroke_weight = s.strokeWeight;
}

// TODO implement LUT similar to ellipse_points_LUT
void PGraphics::arc(const float x, const float y,
                    const float w, const float h,
                    const float start, const float stop,
                    const int mode) {
    const int   segments  = arc_detail;
    const float angleStep = (stop - start) / static_cast<float>(segments);

    std::vector<glm::vec2> arcPoints;
    for (int i = 0; i <= segments; i++) {
        const float angle = start + i * angleStep;
        const float vx    = x + (w / 2.0f) * cos(angle);
        const float vy    = y + (h / 2.0f) * sin(angle);
        arcPoints.emplace_back(vx, vy);
    }

    // === fill pass ===
    if (color_fill.active) {
        pushStyle();
        color_stroke.active = false; // disable stroke for fill

        if (mode == PIE) {
            beginShape(TRIANGLE_FAN);
            vertex(x, y); // center
            for (const auto& pt: arcPoints) {
                vertex(pt.x, pt.y);
            }
            endShape(true);
        } else if (mode == OPEN || mode == CHORD) {
            beginShape();
            for (const auto& pt: arcPoints) {
                vertex(pt.x, pt.y);
            }
            if (mode == CHORD) {
                vertex(arcPoints.front().x, arcPoints.front().y);
            }
            endShape(true);
        }

        popStyle();
    }

    // === stroke pass ===
    if (color_stroke.active) {
        pushStyle();
        color_fill.active = false; // disable fill for stroke

        bool end_shape_closed = false;
        beginShape();
        if (mode == PIE) {
            // NOTE outline is not closed to comply with Processing behavior
            // vertex(arcPoints.front().x, arcPoints.front().y);
            for (const auto& pt: arcPoints) {
                vertex(pt.x, pt.y);
            }
            // vertex(arcPoints.back().x, arcPoints.back().y);
            // vertex(x, y); // close back to center
            end_shape_closed = false;
        } else if (mode == CHORD) {
            for (const auto& pt: arcPoints) {
                vertex(pt.x, pt.y);
            }
            vertex(arcPoints.front().x, arcPoints.front().y); // chord edge
        } else if (mode == OPEN) {
            for (const auto& pt: arcPoints) {
                vertex(pt.x, pt.y);
            }
        }
        endShape(end_shape_closed);

        popStyle();
    }
}

void PGraphics::arcDetail(const int detail) {
    arc_detail = detail;
}

void PGraphics::ellipse(const float a, const float b, const float c, const float d) {
    if (!color_fill.active && !color_stroke.active) {
        return;
    }

    float cx, cy, width, height;

    switch (ellipse_mode) {
        case CENTER:
            cx     = a;
            cy     = b;
            width  = c;
            height = d;
            break;
        case RADIUS:
            cx     = a;
            cy     = b;
            width  = c * 2.0f;
            height = d * 2.0f;
            break;
        case CORNER:
            cx     = a + c * 0.5f;
            cy     = b + d * 0.5f;
            width  = c;
            height = d;
            break;
        case CORNERS:
            cx     = (a + c) * 0.5f;
            cy     = (b + d) * 0.5f;
            width  = std::abs(c - a);
            height = std::abs(d - b);
            break;
        default:
            // fallback to CENTER mode if unknown
            cx     = a;
            cy     = b;
            width  = c;
            height = d;
            break;
    }

    const float radiusX = width * 0.5f;
    const float radiusY = height * 0.5f;

    std::vector<glm::vec3> points;
    points.reserve(ellipse_detail + 1);

    // TODO: recompute LUT if ellipse_detail changes
    float i_f = 0.0f;
    for (int i = 0; i <= ellipse_detail; ++i, i_f += 1.0f) {
        points.emplace_back(cx + radiusX * ellipse_points_LUT[i].x,
                            cy + radiusY * ellipse_points_LUT[i].y,
                            0.0f);
    }

    // TODO optimize ellipse by drawing it as TRIANGLE_FAN
    beginShape(POLYGON);
    points.pop_back(); // remove duplicate point if LUT loops
    for (const auto& p: points) {
        vertex(p.x, p.y, p.z);
    }
    endShape(CLOSE);
}

void PGraphics::image(PImage* img, const float x, const float y, float w, float h) {
    if (!color_fill.active) {
        return;
    }

    if (img == nullptr) {
        error("img is null");
        return;
    }

    if (w < 0) {
        w = img->width;
    }
    if (h < 0) {
        h = img->height;
    }

    const bool _stroke_active = color_stroke.active;
    noStroke();
    push_texture_id();
    push_force_transparent();
    shape_force_transparent = true;
    texture(img);
    rect(x, y, w, h, img->flip_y_texcoords);
    pop_texture_id();
    pop_force_transparent();
    color_stroke.active = _stroke_active;
}

void PGraphics::image(PImage* img, const float x, const float y) {
    image(img, x, y, img->width, img->height);
}

void PGraphics::circle(const float x, const float y, const float diameter) {
    ellipse(x, y, diameter, diameter);
}

/* --- load image --- */

// PImage* PGraphics::loadImage(const std::string& file, const bool use_relative_path) {
//     const std::string absolute_path = use_relative_path ? file : sketchPath() + file;
//     if (!file_exists(absolute_path)) {
//         error("loadImage() failed! file not found: '", file, "'. the 'sketchPath()' is currently set to '", sketchPath(), "'. looking for file at: '", absolute_path, "'");
//         return nullptr;
//     }
//     return new PImage(absolute_path);
// }

// const std::vector<std::string> supportedProtocols{};
//
// struct MemoryBuffer {
//     std::vector<unsigned char> data;
// };
//
// static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
//     size_t        totalSize = size * nmemb;
//     MemoryBuffer* mem       = static_cast<MemoryBuffer*>(userp);
//     mem->data.insert(mem->data.end(), (unsigned char*) contents, (unsigned char*) contents + totalSize);
//     return totalSize;
// }
//
// unsigned char* loadImageFromFile(const std::string& filepath, int* width, int* height, int* channels) {
//     return stbi_load(filepath.c_str(), width, height, channels, 0);
// }
//
// unsigned char* loadImageFromURL(const std::string& url, int* width, int* height, int* channels) {
//     static std::vector<std::string> supportedProtocols;
//
//     // Query supported protocols only once
//     if (supportedProtocols.empty()) {
//         const curl_version_info_data* info = curl_version_info(CURLVERSION_NOW);
//         if (info && info->protocols) {
//             for (const char* const* proto = info->protocols; *proto != nullptr; ++proto) {
//                 supportedProtocols.emplace_back(*proto);
//             }
//         }
//     }
//
//     // Optional: skip early if protocol is unsupported
//     auto schemeEnd = url.find("://");
//     if (schemeEnd != std::string::npos) {
//         std::string scheme = url.substr(0, schemeEnd);
//         auto        found  = std::find(supportedProtocols.begin(), supportedProtocols.end(), scheme);
//         if (found == supportedProtocols.end()) {
//             std::cerr << "Protocol not supported by libcurl: " << scheme << "\n";
//             return nullptr;
//         }
//     }
//
//     // Proceed with blocking download
//     CURL* curl = curl_easy_init();
//     if (!curl) {
//         return nullptr;
//     }
//
//     MemoryBuffer buffer;
//
//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
//     curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
//
//     CURLcode res = curl_easy_perform(curl);
//     curl_easy_cleanup(curl);
//
//     if (res != CURLE_OK) {
//         std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
//         return nullptr;
//     }
//
//     return stbi_load_from_memory(buffer.data.data(), buffer.data.size(), width, height, channels, 0);
// }
//
// unsigned char* loadImage(const std::string& location, int* width, int* height, int* channels) {
//     // Query and cache supported protocols once
//     static std::vector<std::string> curlProtocols;
//     if (curlProtocols.empty()) {
//         curl_version_info_data* info = curl_version_info(CURLVERSION_NOW);
//         if (info && info->protocols) {
//             for (const char* const* proto = info->protocols; *proto != nullptr; ++proto) {
//                 curlProtocols.emplace_back(*proto);
//             }
//         }
//     }
//
//     // Extract scheme (before ://)
//     auto schemeEnd = location.find("://");
//     if (schemeEnd != std::string::npos) {
//         std::string scheme = location.substr(0, schemeEnd);
//
//         if (scheme == "file") {
//             // Handle file:// specially — map to local file
// #if defined(_WIN32)
//             const std::string prefix = "file:///";
//             if (location.rfind(prefix, 0) == 0) {
//                 std::string path = location.substr(prefix.size());
//                 std::replace(path.begin(), path.end(), '/', '\\');
//                 return loadImageFromFile(path, width, height, channels);
//             }
// #else
//             std::string path = location.substr(7); // strip file://
//             return loadImageFromFile(path, width, height, channels);
// #endif
//         }
//
//         // Check if scheme is supported by libcurl
//         if (std::find(curlProtocols.begin(), curlProtocols.end(), scheme) != curlProtocols.end()) {
//             return loadImageFromURL(location, width, height, channels);
//         } else {
//             std::cerr << "Unsupported protocol: " << scheme << "\n";
//             return nullptr;
//         }
//     }
//
//     // No scheme → assume it's a local path
//     return loadImageFromFile(location, width, height, channels);
// }

// PFont* PGraphics::loadFont(const std::string& file, const float size) {
//     const std::string absolute_path = resolve_data_path(file);
//     if (!file_exists(absolute_path)) {
//         error("loadFont() failed! file not found: '", file, "'. the 'sketchPath()' is currently set to '", sketchPath(), "'. looking for file at: '", absolute_path, "'");
//         return nullptr;
//     }
//     return new PFont(absolute_path, size);
// }

void PGraphics::textFont(PFont* font) {
    current_font = font;
}

void PGraphics::textSize(const float size) {
    if (current_font == nullptr) {
        return;
    }
    current_font->textSize(size);
}

void PGraphics::text(const char* value, const float x, const float y, const float z) {
    text_str(value, x, y, z);
}

float PGraphics::textWidth(const std::string& text) {
    if (current_font == nullptr) {
        return 0;
    }
    return current_font->textWidth(text.c_str());
}

void PGraphics::textAlign(const int alignX, const int alignY) {
    if (current_font == nullptr) {
        return;
    }
    current_font->textAlign(alignX, alignY);
}

float PGraphics::textAscent() {
    if (current_font == nullptr) {
        return 0;
    }
    return current_font->textAscent();
}

float PGraphics::textDescent() {
    if (current_font == nullptr) {
        return 0;
    }
    return current_font->textDescent();
}

void PGraphics::textLeading(const float leading) {
    if (current_font == nullptr) {
        return;
    }
    current_font->textLeading(leading);
}

void PGraphics::text_str(const std::string& text, const float x, const float y, const float z) {
    if (current_font == nullptr) {
        return;
    }
    if (!color_fill.active) {
        return;
    }

    current_font->draw(this, text, x, y, z);
}

void PGraphics::texture(PImage* img) {
    current_texture = img;
}

void PGraphics::point(const float x, const float y, const float z) {
    beginShape(POINTS);
    vertex(x, y, z);
    endShape();
}

void PGraphics::pointSize(const float size) {
    current_stroke_state.point_weight = size;
}

void PGraphics::line(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2) {
    if (!color_stroke.active) {
        return;
    }
    beginShape(LINES);
    vertex(x1, y1, z1);
    vertex(x2, y2, z2);
    endShape();
}

void PGraphics::line(const float x1, const float y1, const float x2, const float y2) {
    line(x1, y1, 0, x2, y2, 0);
}

void PGraphics::triangle(const float x1, const float y1, const float z1,
                         const float x2, const float y2, const float z2,
                         const float x3, const float y3, const float z3) {
    beginShape(TRIANGLES);
    vertex(x1, y1, z1);
    vertex(x2, y2, z2);
    vertex(x3, y3, z3);
    endShape();
}

void PGraphics::quad(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2, const float x3, const float y3, const float z3, const float x4, const float y4, const float z4) {
    beginShape(QUADS);
    vertex(x1, y1, z1, 1, 1);
    vertex(x2, y2, z2, 0, 1);
    vertex(x3, y3, z3, 0, 0);
    vertex(x4, y4, z4, 1, 0);
    endShape();
}

void PGraphics::rect(const float x, const float y, const float width, const float height, const bool flip_y_texcoords) {
    // TODO not too happy about the texture flip option …
    //      maybe this can be handled elsewhere
    //      e.g in a tex_coord matrix … in shader
    //      ( currently there is no such matrix in use )
    if (!color_stroke.active && !color_fill.active) {
        return;
    }

    // compute rectangle corners using glm::vec2
    glm::vec2 p1, p2;
    switch (rect_mode) {
        case CORNERS:
            p1 = {x, y};
            p2 = {width, height};
            break;
        case CENTER:
            p1 = {x - width * 0.5f, y - height * 0.5f};
            p2 = {x + width * 0.5f, y + height * 0.5f};
            break;
        case RADIUS:
            p1 = {x - width, y - height};
            p2 = {x + width, y + height};
            break;
        case CORNER:
        default:
            p1 = {x, y};
            p2 = {x + width, y + height};
            break;
    }

    // define rectangle vertices (shared for fill and stroke)
    static constexpr uint8_t                  NUM_VERTICES  = 4;
    const std::array<glm::vec3, NUM_VERTICES> rect_vertices = {
        glm::vec3{p1.x, p1.y, 0},
        glm::vec3{p2.x, p1.y, 0},
        glm::vec3{p2.x, p2.y, 0},
        glm::vec3{p1.x, p2.y, 0}};
    const std::array<glm::vec2, NUM_VERTICES> rect_tex_coords = {
        glm::vec2{0, flip_y_texcoords ? 1 : 0},
        glm::vec2{1, flip_y_texcoords ? 1 : 0},
        glm::vec2{1, flip_y_texcoords ? 0 : 1},
        glm::vec2{0, flip_y_texcoords ? 0 : 1}};

    beginShape(QUADS);
    for (int i = 0; i < rect_vertices.size(); ++i) {
        vertex_vec(rect_vertices[i], rect_tex_coords[i]);
    }
    endShape();
}

void PGraphics::rect(const float x, const float y, const float width, const float height) {
    rect(x, y, width, height, false);
}

void PGraphics::box(const float width, const float height, const float depth) {
    pushMatrix();
    scale(width, height, depth);
    /* fill */
    if (color_fill.active) {
        current_shape.started          = true;
        current_shape.mode             = TRIANGLES;
        shape_fill_vertex_buffer       = box_fill_vertices_LUT; // bulk copy
        const glm::vec4 fill_color_vec = as_vec4(color_fill);
        for (auto& v: shape_fill_vertex_buffer) { v.color = fill_color_vec; }
        submit_fill_shape(true, shape_force_transparent);
        shape_fill_vertex_buffer.clear();
        current_shape.started = false;
    }
    /* stroke */
    if (color_stroke.active) {
        current_shape.started            = true;
        current_shape.mode               = QUADS;
        shape_stroke_vertex_buffer       = box_stroke_vertices_LUT; // bulk copy
        const glm::vec4 stroke_color_vec = as_vec4(color_stroke);
        for (auto& v: shape_stroke_vertex_buffer) { v.color = stroke_color_vec; }
        submit_stroke_shape(true, shape_force_transparent);
        shape_stroke_vertex_buffer.clear();
        current_shape.started = false;
    }
    popMatrix();
    current_shape.reset();
}

void PGraphics::sphere(const float width, const float height, const float depth) {
    pushMatrix();
    scale(width, height, depth);
    beginShape(TRIANGLES);
    for (const auto& v: sphere_vertices_LUT) {
        vertex(v);
    }
    endShape();
    popMatrix();
    // beginShape(TRIANGLES);
    // for (const auto& v: sphere_vertices_LUT) {
    //     vertex(Vertex{glm::vec4(v.position.x * width, v.position.y * height, v.position.z * depth, v.position.w),
    //                   v.color,
    //                   v.tex_coord,
    //                   v.normal});
    // }
    // endShape();
}

void PGraphics::sphereDetail(const int ures, const int vres) {
    sphere_u_resolution = std::max(3, ures); // longitudinal (around)
    sphere_v_resolution = std::max(2, vres); // latitudinal (top to bottom)
    sphere_vertices_LUT.clear();
    generate_sphere(sphere_vertices_LUT, sphere_u_resolution, sphere_u_resolution);
}

void PGraphics::resize_ellipse_points_LUT() {
    if (ellipse_detail < ELLIPSE_DETAIL_MIN) {
        return;
    }
    ellipse_points_LUT.clear();
    ellipse_points_LUT.resize(ellipse_detail + 1); // Resize instead of reserve

    const float deltaTheta = (2.0f * PI) / static_cast<float>(ellipse_detail);

    for (int i = 0; i <= ellipse_detail; ++i) {
        const float theta     = deltaTheta * static_cast<float>(i);
        ellipse_points_LUT[i] = {std::cos(theta), std::sin(theta)};
    }
}

/* --- triangulation --- */

// TODO move to Geometry or Triangulation

// EARCUT

std::vector<Vertex> PGraphics::triangulate_faster(const std::vector<Vertex>& vertices) {
    std::vector<std::vector<std::array<float, 2>>> polygon;
    polygon.emplace_back(); // Outer boundary

    // convert Vertex to 2D channels for earcut (ignore z)
    for (const auto& v: vertices) {
        polygon[0].push_back({v.position.x, v.position.y});
    }

    // perform triangulation
    const std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);
    std::vector<Vertex>         triangleList;

    for (size_t i = 0; i < indices.size(); i++) {
        const int index = indices[i];
        if (i < indices.size()) {
            triangleList.push_back(vertices[index]);
        }
    }
    return triangleList;
}

// POLYPARTITION + CLIPPER2

Clipper2Lib::PathD convertToClipperPath(const std::vector<Vertex>& vertices) {
    Clipper2Lib::PathD path;
    for (const auto& v: vertices) {
        path.push_back({v.position.x, v.position.y});
    }
    return path;
}

std::vector<TPPLPoly> convertToPolyPartition(const Clipper2Lib::PathsD& paths) {
    std::vector<TPPLPoly> polys;
    for (const auto& path: paths) {
        TPPLPoly poly;
        poly.Init(path.size());
        for (size_t i = 0; i < path.size(); ++i) {
            poly[i].x = path[i].x;
            poly[i].y = path[i].y;
        }
        polys.push_back(poly);
    }
    return polys;
}

std::vector<Vertex> PGraphics::triangulate_good(const std::vector<Vertex>& vertices) {
    const std::vector<Vertex> triangles = triangulator.triangulate(vertices, Triangulator::Winding::WINDING_ODD);
    return triangles;
}

std::vector<Vertex> PGraphics::triangulate_better_quality(const std::vector<Vertex>& vertices) {
    const glm::vec4 first_color  = vertices[0].color;
    const glm::vec4 first_normal = vertices[0].normal;

    TPPLPartition partitioner;

    // use Clipper2 to resolve self-intersections
    const Clipper2Lib::PathsD inputPaths = {convertToClipperPath(vertices)};
    const Clipper2Lib::PathsD fixedPaths = Clipper2Lib::Union(inputPaths, Clipper2Lib::FillRule::NonZero);

    if (fixedPaths.empty()) {
        std::cerr << "Clipper2 failed to fix the polygon!" << std::endl;
        return {};
    }

    // convert to PolyPartition channels
    std::vector<TPPLPoly> convexPolygons;
    for (auto& poly: convertToPolyPartition(fixedPaths)) {
        std::list<TPPLPoly> convexParts;
        if (!partitioner.ConvexPartition_HM(&poly, &convexParts)) {
            std::cerr << "Convex partitioning failed!" << std::endl;
            continue;
        }
        convexPolygons.insert(convexPolygons.end(), convexParts.begin(), convexParts.end());
    }

    // triangulate each convex part
    std::vector<Vertex> triangleList;
    for (auto& part: convexPolygons) {
        std::list<TPPLPoly> triangles;
        if (!partitioner.Triangulate_EC(&part, &triangles)) {
            std::cerr << "Triangulation failed for a convex part!" << std::endl;
            continue;
        }

        // extract triangle vertices
        for (const auto& tri: triangles) {
            for (int i = 0; i < 3; ++i) {
                triangleList.emplace_back(glm::vec3(tri[i].x, tri[i].y, 0.0f),
                                          first_color,
                                          glm::vec3(0.0f, 0.0f, 0.0f),
                                          first_normal);
            }
        }
    }
    return triangleList;
}

void PGraphics::reset_mvp_matrices() {
    /* model_matrix */
    resetMatrix();
    /* projection_matrix */
    in_camera_block = false;
    perspective(DEFAULT_CAMERA_FOV_RADIANS, width / height, 0.1f, depth_range);
    projection_matrix[1][1] *= -1.0f;
    /* view_matrix */
    PGraphics::camera();

    // orthographic projection NOTE use framebuffer.width and framebuffer.height
    // projection_matrix_2D = glm::ortho(0.0f, static_cast<float>(framebuffer.width), static_cast<float>(framebuffer.height), 0.0f);

    model_matrix_dirty = false;
}

void PGraphics::restore_mvp_matrices() {
    // NOTE nothing to restore here
}

void PGraphics::to_screen_space(glm::vec3& model_position) const {
    glm::vec4 clipPos = projection_matrix * view_matrix * model_matrix * glm::vec4(model_position, 1.0f);

    // perspective divide
    if (clipPos.w != 0.0f) {
        clipPos.x /= clipPos.w;
        clipPos.y /= clipPos.w;
        clipPos.z /= clipPos.w;
    }

    // NDC → screen
    const float screenX = (clipPos.x * 0.5f + 0.5f) * width;
    const float screenY = (1.0f - (clipPos.y * 0.5f + 0.5f)) * height; // flip Y
    const float screenZ = clipPos.z;                                   // depth, optional

    model_position = glm::vec3(screenX, screenY, screenZ);
}

void PGraphics::to_world_space(glm::vec3& model_position) const {
    model_position = model_matrix * glm::vec4(model_position, 1.0f);
}

static glm::vec3 world_to_screen(
    const glm::vec3& P, // world-space point
    const glm::mat4& mvp,
    const int        screen_width,
    const int        screen_height) {
    const glm::vec4 clip_space = mvp * glm::vec4(P, 1.0f);

    if (clip_space.w == 0.0f) {
        return glm::vec3(-1.0f); // error
    }

    // Perspective divide to get NDC
    const glm::vec3 ndc = glm::vec3(clip_space) / clip_space.w;

    // NDC to screen
    glm::vec3 screen;
    screen.x = (ndc.x * 0.5f + 0.5f) * screen_width;
    screen.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * screen_height; // flip Y
    screen.z = ndc.z;                                          // optional, -1 (near) to 1 (far)

    return screen;
}

// OPTIMIZE this and store MVP ( and variants ) globally
void PGraphics::triangulate_line_strip_vertex(const glm::mat4&           model_matrix,
                                              const std::vector<Vertex>& line_strip,
                                              const StrokeState&         stroke,
                                              const bool                 close_shape,
                                              std::vector<Vertex>&       line_vertices) const {
    const glm::vec4        color  = line_strip[0].color;
    const glm::vec4        normal = line_strip[0].normal;
    std::vector<glm::vec2> points(line_strip.size());
    std::vector<glm::vec2> triangles;

    const glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;
    for (int i = 0; i < line_strip.size(); ++i) {
        // glm::vec3 _position = line_strip[i].position;
        // to_screen_space(_position);
        points[i] = world_to_screen(line_strip[i].position, mvp, width, height);
    }

    triangulate_line_strip(points,
                           close_shape,
                           stroke.stroke_weight,
                           stroke.stroke_join_mode,
                           stroke.stroke_cap_mode,
                           stroke.stroke_join_round_resolution,
                           stroke.stroke_cap_round_resolution,
                           stroke.stroke_join_miter_max_angle,
                           triangles);
    line_vertices.reserve(triangles.size());
    for (const auto& triangle: triangles) {
        line_vertices.emplace_back(glm::vec3(triangle, 0.0f), color, glm::vec3(0.0f, 0.0f, 0.0f), normal);
    }
}

void PGraphics::normal(const float x, const float y, const float z, const float w) {
    current_normal.x = x;
    current_normal.y = y;
    current_normal.z = z;
    current_normal.w = w;
}

void PGraphics::beginCamera() {
    in_camera_block        = true;
    temp_view_matrix       = view_matrix;
    temp_projection_matrix = projection_matrix;
}

void PGraphics::endCamera() {
    if (in_camera_block) {
        update_view_matrix(temp_view_matrix);
        update_projection_matrix(projection_matrix);
        in_camera_block = false;
    }
}

void PGraphics::auto_flush_on_matrix_change() {
    if (auto_flush) {
        flush();
        resetMatrix();
    }
}

void PGraphics::update_view_matrix(const glm::mat4& view) {
    auto_flush_on_matrix_change();
    view_matrix = view;
}

void PGraphics::update_projection_matrix(const glm::mat4& proj) {
    auto_flush_on_matrix_change();
    projection_matrix = proj;
}

void PGraphics::camera(const float eyeX, const float eyeY, const float eyeZ,
                       const float centerX, const float centerY, const float centerZ,
                       const float upX, const float upY, const float upZ) {
    const glm::vec3 eye(eyeX, eyeY, eyeZ);
    const glm::vec3 center(centerX, centerY, centerZ);
    const glm::vec3 up(upX, upY, upZ);

    if (in_camera_block) {
        temp_view_matrix = glm::lookAt(eye, center, up);
    } else {
        const glm::mat4 view = glm::lookAt(eye, center, up);
        update_view_matrix(view);
    }
}

void PGraphics::camera() {
    auto_flush_on_matrix_change();
    // from https://processing.org/reference/camera_.html with FOV 60°
    // camera(width / 2.0, height / 2.0, (height / 2.0) / tan(PI * 30.0 / 180.0),
    //        width / 2.0, height / 2.0, 0,
    //        0, 1, 0);
    constexpr float fov            = DEFAULT_CAMERA_FOV_RADIANS;
    const float     cameraDistance = (height / 2.0f) / tan(fov / 2.0f);
    PGraphics::camera(width / 2.0, height / 2.0, cameraDistance,
                      width / 2.0, height / 2.0, 0,
                      0, 1, 0);
}

void PGraphics::frustum(const float left, const float right, const float bottom, const float top, const float near, const float far) {
    const glm::mat4 proj = glm::frustum(left, right, bottom, top, near, far);
    if (in_camera_block) {
        temp_projection_matrix = proj;
    } else {
        update_projection_matrix(proj);
    }
}

void PGraphics::ortho(const float left, const float right, const float bottom, const float top, const float near, const float far) {
    const glm::mat4 proj = glm::ortho(left, right, bottom, top, near, far);
    if (in_camera_block) {
        temp_projection_matrix = proj;
    } else {
        update_projection_matrix(proj);
    }
}

void PGraphics::perspective(const float fovy, const float aspect, const float near, const float far) {
    const glm::mat4 proj = glm::perspective(fovy, aspect, near, far);
    if (in_camera_block) {
        temp_projection_matrix = proj;
    } else {
        update_projection_matrix(proj);
    }
}

void PGraphics::printCamera() {
    const glm::mat4& mat = in_camera_block ? temp_view_matrix : view_matrix;
    printMatrix(mat);
}

void PGraphics::printProjection() {
    const glm::mat4& mat = in_camera_block ? temp_projection_matrix : projection_matrix;
    printMatrix(mat);
}

/* --- shape --- */

void PGraphics::beginShape(const int shape) {
    shape_fill_vertex_buffer.clear();
    shape_stroke_vertex_buffer.clear();
    current_shape.mode = static_cast<ShapeMode>(shape);
}

void PGraphics::vertex(const float x, const float y, const float z) {
    vertex(x, y, z, 0, 0);
}

static bool shape_can_have_fill(const int shape_mode) {
    return shape_mode != POINTS &&
           shape_mode != LINES &&
           shape_mode != LINE_STRIP;
}

void PGraphics::vertex(const float x, const float y, const float z, const float u, const float v) {
    if (!color_stroke.active && !color_fill.active) {
        return;
    }

    const glm::vec3 position{x, y, z};
    const glm::vec3 tex_coord{u, v, 0.0f};

    if (color_stroke.active) {
        shape_stroke_vertex_buffer.emplace_back(position, as_vec4(color_stroke), tex_coord, current_normal);
    }

    if (color_fill.active && shape_can_have_fill(current_shape.mode)) {
        shape_fill_vertex_buffer.emplace_back(position, as_vec4(color_fill), tex_coord, current_normal);
    }
}

void PGraphics::vertex(const Vertex& v) {
    if (!color_stroke.active && !color_fill.active) {
        return;
    }
    // TODO maybe use v.color instead of color_fill? 'shape_fill_vertex_buffer.emplace_back(v);'
    if (color_stroke.active) {
        shape_stroke_vertex_buffer.emplace_back(v.position, as_vec4(color_stroke), v.tex_coord, v.normal);
    }
    if (color_fill.active && shape_can_have_fill(current_shape.mode)) {
        shape_fill_vertex_buffer.emplace_back(v.position, as_vec4(color_fill), v.tex_coord, v.normal);
    }
}

int PGraphics::get_current_texture_id() const { return current_texture == nullptr ? TEXTURE_NONE : current_texture->texture_id; }

void PGraphics::submit_stroke_shape(const bool closed, const bool force_transparent) const {
    const bool _force_transparent = stroke_render_mode == STROKE_RENDER_MODE_TRIANGULATE_2D ? true : force_transparent;
    if (!shape_stroke_vertex_buffer.empty()) {
        UShape s;
        s.mode         = current_shape.mode;
        s.stroke       = current_stroke_state;
        s.filled       = false;
        s.vertices     = shape_stroke_vertex_buffer;
        s.model_matrix = model_matrix;
        s.transparent  = _force_transparent ? true : has_transparent_vertices(shape_stroke_vertex_buffer);
        s.closed       = closed;
        s.texture_id   = get_current_texture_id();
        s.shader       = current_custom_shader;
        shape_renderer->submit_shape(s);
    }
}

void PGraphics::submit_fill_shape(const bool closed, const bool force_transparent) const {
    if (shape_renderer != nullptr && !shape_fill_vertex_buffer.empty()) {
        UShape s;
        // NOTE no need to copy stroke info for filled shape
        s.mode          = current_shape.mode;
        s.filled        = true;
        s.vertices      = shape_fill_vertex_buffer;
        s.model_matrix  = model_matrix;
        s.transparent   = force_transparent ? true : has_transparent_vertices(shape_fill_vertex_buffer);
        s.closed        = closed;
        s.texture_id    = get_current_texture_id();
        s.light_enabled = lights_enabled;
        // NOTE only copy lighting state if lights are enabled
        if (lights_enabled) {
            s.lighting = lightingState;
        }
        s.shader = current_custom_shader;
        shape_renderer->submit_shape(s);
    }
}

void PGraphics::endShape(const bool closed) {
    if (shape_renderer != nullptr) {
        submit_fill_shape(closed, shape_force_transparent);
        submit_stroke_shape(closed, shape_force_transparent);
        if (render_mode == RENDER_MODE_IMMEDIATELY) {
            flush();
        }
    }
    shape_fill_vertex_buffer.clear();
    shape_stroke_vertex_buffer.clear();
    current_shape.reset();
}

void PGraphics::debug_text(const std::string& text, const float x, const float y) {
    if (shape_renderer == nullptr || debug_font == nullptr) {
        return;
    }
    UShape s;
    s.mode = TRIANGLES;
    // NOTE ignore 'stroke'
    s.filled = true;
    s.vertices.reserve(text.size() * 6);
    debug_font->generate(s.vertices, text, x, y, glm::vec4(color_fill));
    s.model_matrix  = model_matrix;
    s.transparent   = true;
    s.closed        = false;
    s.texture_id    = texture_update_and_bind(debug_font->atlas());
    s.light_enabled = false;
    // NOTE ignore 'lighting'
    // NOTE ignore 'shader'
    // NOTE ignore 'vertex_buffer'
    shape_renderer->submit_shape(s);
    if (render_mode == RENDER_MODE_IMMEDIATELY) {
        flush();
    }
}

void PGraphics::emit_shape_fill_triangles(std::vector<Vertex>& triangle_vertices) {
    if (triangle_emitter_callback) {
        triangle_emitter_callback(triangle_vertices);
    }
    // IMPL_emit_shape_fill_triangles(triangle_vertices);
}

void PGraphics::emit_shape_stroke_line_strip(std::vector<Vertex>& line_strip_vertices, const bool line_strip_closed) {
    if (stroke_emitter_callback) {
        stroke_emitter_callback(line_strip_vertices, line_strip_closed);
    }
    // IMPL_emit_shape_stroke_line_strip(line_strip_vertices, line_strip_closed);
}

void PGraphics::emit_shape_stroke_points(std::vector<Vertex>& point_vertices, const float point_size) {
    warning_in_function_once("emit_shape_stroke_points() is not implemented yet!");
    // TODO maybe add a callback for points or send strokes with one vertex TBD?
    // if (point_emitter_callback) {
    //     point_emitter_callback(point_vertices);
    // }
    // IMPL_emit_shape_stroke_points(point_vertices, point_size);
}

void PGraphics::convert_stroke_shape_to_line_strip(UShape& s, std::vector<UShape>& shapes) {
    std::vector<Vertex> shape_stroke_vertex_buffer = s.vertices;
    const int           shape_mode_cache           = s.mode;

    if (!shape_stroke_vertex_buffer.empty()) {
        // helper struct to configure vertex group processing
        struct VertexGroupConfig {
            int                        vertices_per_group;
            bool                       is_closed;
            const std::vector<Vertex>* vertex_source;
        };

        // helper function to process vertex groups
        auto processVertexGroup = [&](const VertexGroupConfig& config) {
            const int buffer_size = config.vertex_source->size() / config.vertices_per_group * config.vertices_per_group;
            shapes.reserve(buffer_size / config.vertices_per_group);
            for (int i = 0; i < buffer_size; i += config.vertices_per_group) {
                std::vector<Vertex> vertices;
                vertices.reserve(config.vertices_per_group);
                for (int j = 0; j < config.vertices_per_group; ++j) {
                    vertices.push_back((*config.vertex_source)[i + j]);
                }
                UShape ns{
                    .mode         = LINE_STRIP,
                    .stroke       = s.stroke,
                    .filled       = s.filled,
                    .vertices     = std::move(vertices),
                    .model_matrix = s.model_matrix,
                    .transparent  = s.transparent,
                    .closed       = config.is_closed,
                    .texture_id   = s.texture_id};
                shapes.emplace_back(std::move(ns));
            }
        };
        switch (shape_mode_cache) {
            case LINES: {
                const VertexGroupConfig config{2, false, &shape_stroke_vertex_buffer};
                processVertexGroup(config);
            } break;
            case TRIANGLE_FAN: {
                const std::vector<Vertex> converted_vertices = convertTriangleFanToTriangles(shape_stroke_vertex_buffer);
                const VertexGroupConfig   config{3, true, &converted_vertices};
                processVertexGroup(config);
            } break;
            case TRIANGLES: {
                const VertexGroupConfig config{3, true, &shape_stroke_vertex_buffer};
                processVertexGroup(config);
            } break;
            case TRIANGLE_STRIP: {
                const std::vector<Vertex> converted_vertices = convertTriangleStripToTriangles(shape_stroke_vertex_buffer);
                const VertexGroupConfig   config{3, true, &converted_vertices};
                processVertexGroup(config);
            } break;
            case QUAD_STRIP: {
                const std::vector<Vertex> converted_vertices = convertQuadStripToQuads(shape_stroke_vertex_buffer);
                const VertexGroupConfig   config{4, true, &converted_vertices};
                processVertexGroup(config);
            } break;
            case QUADS: {
                const VertexGroupConfig config{4, true, &shape_stroke_vertex_buffer};
                processVertexGroup(config);
            } break;
            default:
            case LINE_STRIP:
            case POLYGON:
                shapes.emplace_back(std::move(s));
                s.mode = LINE_STRIP;
                break;
        }
    }
}

void PGraphics::convert_fill_shape_to_triangles(UShape& s) const {
    // NOTE used by UShapeRendererOpenGL_3::convert_fill_shape_to_triangles
    const std::vector<Vertex> shape_fill_vertex_buffer = s.vertices;
    const int                 shape_mode_cache         = s.mode;
    // TODO what if polygon has only 3 ( triangle ) or 4 vertices ( quad )? could shortcut … here
    if (!shape_fill_vertex_buffer.empty()) {
        switch (shape_mode_cache) {
            case POINTS:
            case LINES:
            case LINE_STRIP:
                warning_in_function_once("points, lines, line_strip should not be processed here.");
            case TRIANGLES:
                break;
            case TRIANGLE_FAN: {
                std::vector<Vertex> vertices_filled_triangles = convertTriangleFanToTriangles(shape_fill_vertex_buffer);
                s.vertices                                    = std::move(vertices_filled_triangles);
            } break;
            case QUAD_STRIP: // NOTE does this just work?!?
            case TRIANGLE_STRIP: {
                std::vector<Vertex> vertices_filled_triangles = convertTriangleStripToTriangles(shape_fill_vertex_buffer);
                s.vertices                                    = std::move(vertices_filled_triangles);
            } break;
            case QUADS: {
                std::vector<Vertex> vertices_filled_triangles = convertQuadsToTriangles(shape_fill_vertex_buffer);
                s.vertices                                    = std::move(vertices_filled_triangles);
            } break;
            case POLYGON:
            default: {
                // NOTE default: POLYGON_TRIANGULATION_BETTER
                std::vector<Vertex> vertices_filled_triangles;
                if (polygon_triangulation_strategy == POLYGON_TRIANGULATION_FASTER) {
                    // EARCUT :: supports concave polygons, textures but no holes or self-intersection
                    vertices_filled_triangles = triangulate_faster(shape_fill_vertex_buffer);
                } else if (polygon_triangulation_strategy == POLYGON_TRIANGULATION_BETTER) {
                    // LIBTESS2 :: supports concave polygons, textures, holes and self-intersection but no textures
                    vertices_filled_triangles = triangulate_good(shape_fill_vertex_buffer);
                } else if (polygon_triangulation_strategy == POLYGON_TRIANGULATION_MID) {
                    // POLYPARTITION + CLIPPER2 // TODO maybe remove this option
                    vertices_filled_triangles = triangulate_better_quality(shape_fill_vertex_buffer);
                }
                s.vertices = std::move(vertices_filled_triangles);
            } break;
        }
        s.mode = TRIANGLES;
    }
}

void PGraphics::set_shader_program(PShader* shader, const ShaderProgramType shader_role) {
    if (shape_renderer != nullptr) {
        shape_renderer->set_shader_program(shader, shader_role);
    }
}

void PGraphics::interpret_color_mode(glm::vec4& color, const float v1, const float v2, const float v3, const float alpha) const {
    // internal color mode is RGBA normalized i.e `colorMode(RGB, 1.0, 1.0, 1.0, 1.0)`
    if (color_mode_state.mode == RGB || color_mode_state.mode == RGBA) {
        // normalize RGBA color
        color.r = map(v1, 0.0, color_mode_state.range.r, 0.0, 1.0);
        color.g = map(v2, 0.0, color_mode_state.range.g, 0.0, 1.0);
        color.b = map(v3, 0.0, color_mode_state.range.b, 0.0, 1.0);
        color.a = map(alpha, 0.0, color_mode_state.range.a, 0.0, 1.0);
        return;
    }
    if (color_mode_state.mode == HSB) {
        // *normalize* HSB + alpha color
        const float h = map(v1, 0.0, color_mode_state.range.r, 0.0, 360.0);  // h
        const float s = map(v2, 0.0, color_mode_state.range.g, 0.0, 1.0);    // s
        const float v = map(v3, 0.0, color_mode_state.range.b, 0.0, 1.0);    // b
        color.a       = map(alpha, 0.0, color_mode_state.range.a, 0.0, 1.0); // a
        hsb_to_rgb_f(h, s, v, color.r, color.g, color.b);
    }
}