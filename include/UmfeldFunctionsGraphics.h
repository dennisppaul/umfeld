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

#include "UmfeldConstants.h"
#include "PGraphics.h"
#include "PImage.h"
#include "PFont.h"

namespace umfeld {

    // ## Color

    // ### Creating & Reading

    float   alpha(color_t color);
    float   blue(color_t color);
    float   brightness(color_t color);
    color_t color(float gray);
    color_t color(float gray, float alpha);
    color_t color(float v1, float v2, float v3);
    color_t color(float v1, float v2, float v3, float alpha);
    float   green(color_t color);
    float   hue(color_t color);
    color_t lerpColor(color_t c1, color_t c2, float amt);
    float   red(color_t color);
    float   saturation(color_t color);

    // NOTE color functions below ignore 'colorMode()'

    color_t color_f(float r, float g, float b, float a = 1.0f);
    color_t color_f(float brightness, float alpha = 1.0);
    color_t color_8(uint8_t gray);
    color_t color_8(uint8_t gray, uint8_t alpha);
    color_t color_8(uint8_t r, uint8_t g, uint8_t b);
    color_t color_8(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    // ### Setting

    void        background(color_t color);
    void        background(float gray);
    inline void background(const int gray) { background(static_cast<float>(gray)); }
    inline void background(const double gray) { background(static_cast<float>(gray)); }
    void        background(float gray, float alpha);
    void        background(float v1, float v2, float v3);
    void        background(float v1, float v2, float v3, float alpha);
    void        background(PImage* image);
    void        colorMode(ColorMode mode, float max);
    void        colorMode(ColorMode mode, float max1, float max2, float max3);
    void        colorMode(ColorMode mode, float max1, float max2, float max3, float maxA);
    void        noFill();
    void        fill(color_t c);
    void        fill(float gray);
    inline void fill(const int gray) { fill(static_cast<float>(gray)); }
    inline void fill(const double gray) { fill(static_cast<float>(gray)); }
    void        fill(float gray, float alpha);
    void        fill(float v1, float v2, float v3);
    void        fill(float v1, float v2, float v3, float alpha);
    void        noStroke();
    void        stroke(color_t c);
    void        stroke(float gray);
    inline void stroke(const int gray) { stroke(static_cast<float>(gray)); }
    inline void stroke(const double gray) { stroke(static_cast<float>(gray)); }
    void        stroke(float gray, float alpha);
    void        stroke(float v1, float v2, float v3);
    void        stroke(float v1, float v2, float v3, float alpha);

    // NOTE additional color functions below ignore 'colorMode()'

    void        background_f(float r, float g, float b, float a = 1.0f);
    void        background_f(float gray);
    inline void background_8(const uint8_t brightness) { background_f(static_cast<float>(brightness) / 255.0f); }
    inline void background_8(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) {
        background_f(static_cast<float>(r) / 255.0f,
                     static_cast<float>(g) / 255.0f,
                     static_cast<float>(b) / 255.0f,
                     static_cast<float>(a) / 255.0f);
    }
    void        background_color(color_t color);
    void        fill_f(float r, float g, float b, float a = 1.0f);
    void        fill_f(float brightness, float alpha = 1.0f);
    inline void fill_8(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) {
        fill_f(static_cast<float>(r) / 255.0f,
               static_cast<float>(g) / 255.0f,
               static_cast<float>(b) / 255.0f,
               static_cast<float>(a) / 255.0f);
    }
    inline void fill_8(const uint8_t brightness, const uint8_t alpha) {
        fill_f(static_cast<float>(brightness) / 255.0f, static_cast<float>(alpha) / 255.0f);
    }
    inline void fill_8(const uint8_t alpha) { fill_f(static_cast<float>(alpha) / 255.0f); }
    void        fill_color(color_t c);
    void        stroke_f(float r, float g, float b, float a = 1.0f);
    void        stroke_f(float brightness, float alpha = 1.0);
    inline void stroke_8(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) {
        stroke_f(static_cast<float>(r) / 255.0f,
                 static_cast<float>(g) / 255.0f,
                 static_cast<float>(b) / 255.0f,
                 static_cast<float>(a) / 255.0f);
    }
    inline void stroke_8(const uint8_t brightness, const uint8_t alpha) { stroke_f(static_cast<float>(brightness) / 255.0f, static_cast<float>(alpha) / 255.0f); }
    inline void stroke_8(const uint8_t alpha) { stroke_f(static_cast<float>(alpha) / 255.0f); }
    void        stroke_color(color_t c);

    // ## Shape

    void     beginShape(int shape = POLYGON);
    void     endShape(bool close_shape = false);
    void     bezier(float x1, float y1,
                    float x2, float y2,
                    float x3, float y3,
                    float x4, float y4);
    void     bezier(float x1, float y1, float z1,
                    float x2, float y2, float z2,
                    float x3, float y3, float z3,
                    float x4, float y4, float z4);
    void     bezierDetail(int detail);
    void     pointSize(float point_size);
    void     arc(float x, float y, float width, float height, float start, float stop, int mode = PIE);
    void     circle(float x, float y, float diameter);
    void     ellipse(float x, float y, float width, float height);
    void     ellipseDetail(int detail);
    void     image(PImage* img, float x, float y, float w, float h);
    void     image(PImage* img, float x, float y);
    void     texture(PImage* img = nullptr);
    void     line(float x1, float y1, float x2, float y2);
    void     line(float x1, float y1, float z1, float x2, float y2, float z2);
    void     triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);
    void     quad(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);
    void     point(float x, float y, float z = 0.0);
    void     rect(float x, float y, float width, float height);
    void     square(float x, float y, float extent);
    void     strokeWeight(float weight);
    void     strokeJoin(int join);
    void     strokeCap(int cap);
    void     vertex(float x, float y, float z = 0.0);
    void     vertex(float x, float y, float z, float u, float v);
    PFont*   loadFont(const std::string& file, float size);
    void     textFont(PFont* font);
    void     textSize(float size);
    void     text(const std::string& text, float x, float y, float z = 0.0f);
    void     text(const char* value, float x, float y, float z = 0.0f);
    float    textWidth(const std::string& text);
    float    textWidth(char c);
    void     textAlign(int alignX, int alignY = BASELINE);
    float    textAscent();
    float    textDescent();
    void     textLeading(float leading);
    void     popMatrix();
    void     pushMatrix();
    void     translate(float x, float y, float z = 0);
    void     rotateX(float angle);
    void     rotateY(float angle);
    void     rotateZ(float angle);
    void     rotate(float angle);
    void     rotate(float angle, float x, float y, float z);
    void     scale(float x);
    void     scale(float x, float y);
    void     scale(float x, float y, float z);
    void     pixelDensity(int density);
    void     blendMode(BlendMode mode);
    void     hint(uint16_t property);
    void     rectMode(int mode);
    void     ellipseMode(int mode);
    void     box(float size);
    void     box(float width, float height, float depth);
    void     sphere(float size);
    void     sphere(float width, float height, float depth);
    void     sphereDetail(int ures, int vres);
    void     sphereDetail(int res);
    void     mesh(VertexBuffer* mesh_shape = nullptr);
    void     shader(PShader* shader = nullptr);
    PShader* loadShader(const std::string& vertex_code, const std::string& fragment_code, const std::string& geometry_code = "");
    struct ShaderSource;
    PShader* loadShader(const ShaderSource& shader_source);
    void     normal(float x, float y, float z, float w = 0);
    void     beginCamera();
    void     endCamera();
    void     camera(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ);
    void     camera();
    void     frustum(float left, float right, float bottom, float top, float near, float far);
    void     ortho(float left, float right, float bottom, float top, float near, float far);
    void     perspective(float fovYDegrees, float aspect, float near, float far);
    void     printCamera();
    void     printProjection();
    void     lights();
    void     noLights();
    void     loadPixels(bool update_logical_buffer = true);
    void     updatePixels(bool update_logical_buffer = true);

    /* --- additional --- */

    void debug_text(const std::string& text, float x, float y);
    void texture_filter(TextureFilter filter);
    void texture_wrap(TextureWrap wrap);
    void flush();

    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value || std::is_same<T, char>::value, void>::type
    text(const T& value, const float x, const float y, const float z = 0.0f) {
        std::ostringstream oss;
        oss << value;
        text(oss.str(), x, y, z);
    }
} // namespace umfeld