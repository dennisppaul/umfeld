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

using namespace umfeld;

PGraphics::PGraphics() : PImage(0, 0) {
    flip_y_texcoords = true;
    PGraphics::fill(1.0f);
    PGraphics::stroke(0.0f);
    PGraphics::ellipseDetail(ELLIPSE_DETAIL_DEFAULT);
    generate_box(box_vertices_LUT);
    generate_sphere(sphere_vertices_LUT, sphere_u_resolution, sphere_u_resolution);
}

void PGraphics::beginDraw() {
    reset_mvp_matrices();
    blendMode(BLEND);
}

void PGraphics::endDraw() {
    restore_mvp_matrices();
}

void PGraphics::hint(const uint16_t property) {}

int PGraphics::displayDensity() {
    return pixel_density;
}

void PGraphics::pixelDensity(const int density) {
    static bool emitted_warning = false;
    if (!emitted_warning && init_properties_locked) {
        warning("`pixelDensity()` should not be set after context is created. use `retina_support` in settings instead to control pixel density.");
        emitted_warning = true;
    }
    pixel_density = density;
}

void PGraphics::stroke_properties(const float stroke_join_round_resolution,
                                  const float stroke_cap_round_resolution,
                                  const float stroke_join_miter_max_angle) {
    this->stroke_join_round_resolution = stroke_join_round_resolution;
    this->stroke_cap_round_resolution  = stroke_cap_round_resolution;
    this->stroke_join_miter_max_angle  = stroke_join_miter_max_angle;
}

void PGraphics::background(PImage* img) {
    background(0, 0, 0, 0);
    fill(1);
    image(img, 0, 0, framebuffer.width, framebuffer.height);
}

void PGraphics::background(const float a) {
    background(a, a, a);
}

void PGraphics::background(const float a, const float b, const float c, const float d) {
    static bool emitted_warning = false;
    if (!emitted_warning) {
        if (a < 0 || a > 1 || b < 0 || b > 1 || c < 0 || c > 1 || d < 0 || d > 1) {
            warning("`background()` values should be in range [0, 1].");
        }
        emitted_warning = true;
    }
    IMPL_background(a, b, c, d);
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

/* --- color, stroke, and fill --- */

void PGraphics::fill(const float r, const float g, const float b, const float alpha) {
    static bool emitted_warning = false;
    if (!emitted_warning) {
        if (r < 0 || r > 1 || g < 0 || g > 1 || b < 0 || b > 1 || alpha < 0 || alpha > 1) {
            warning("`fill()` values should be in range [0, 1].");
        }
        emitted_warning = true;
    }

    color_fill.r      = r;
    color_fill.g      = g;
    color_fill.b      = b;
    color_fill.a      = alpha;
    color_fill.active = true;
}

void PGraphics::fill(const float gray, const float alpha) {
    fill(gray, gray, gray, alpha);
}

void PGraphics::fill_color(const uint32_t c) {
    color_unpack(c, color_fill.r, color_fill.g, color_fill.b, color_fill.a);
    color_fill.active = true;
}

void PGraphics::noFill() {
    color_fill.active = false;
}

void PGraphics::stroke(const float r, const float g, const float b, const float alpha) {
    static bool emitted_warning = false;
    if (!emitted_warning) {
        if (r < 0 || r > 1 || g < 0 || g > 1 || b < 0 || b > 1 || alpha < 0 || alpha > 1) {
            warning("`stroke()` values should be in range [0, 1].");
        }
        emitted_warning = true;
    }

    color_stroke.r      = r;
    color_stroke.g      = g;
    color_stroke.b      = b;
    color_stroke.a      = alpha;
    color_stroke.active = true;
}

void PGraphics::stroke(const float gray, const float alpha) {
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

void PGraphics::stroke_color(const uint32_t c) {
    color_unpack(c, color_stroke.r, color_stroke.g, color_stroke.b, color_stroke.a);
    color_stroke.active = true;
}

void PGraphics::stroke(const float a) {
    stroke(a, a, a);
}

void PGraphics::noStroke() {
    color_stroke.active = false;
}

void PGraphics::strokeWeight(const float weight) {
    stroke_weight = weight;
}

/**
 *  can be MITER, BEVEL, ROUND, NONE, BEVEL_FAST or MITER_FAST
 * @param join
 */
void PGraphics::strokeJoin(const int join) {
    stroke_join_mode = join;
}

/**
 * can be PROJECT, ROUND, POINTED or SQUARE
 * @param cap
 */
void PGraphics::strokeCap(const int cap) {
    stroke_cap_mode = cap;
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
        stroke_weight // assuming you have this
    });
}

void PGraphics::popStyle() {
    if (style_stack.empty()) {
        return;
    }

    const StyleState s = style_stack.top();
    style_stack.pop();

    color_stroke  = s.stroke;
    color_fill    = s.fill;
    stroke_weight = s.strokeWeight;
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
            vertex(arcPoints.front().x, arcPoints.front().y);
            for (const auto& pt: arcPoints) {
                vertex(pt.x, pt.y);
            }
            vertex(arcPoints.back().x, arcPoints.back().y);
            vertex(x, y); // close back to center
            end_shape_closed = true;
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
    texture(img);
    rect(x, y, w, h, img->flip_y_texcoords);
    pop_texture_id();
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

PFont* PGraphics::loadFont(const std::string& file, const float size) {
    const std::string absolute_path = resolveDataPath(file);
    if (!file_exists(absolute_path)) {
        error("loadFont() failed! file not found: '", file, "'. the 'sketchPath()' is currently set to '", sketchPath(), "'. looking for file at: '", absolute_path, "'");
        return nullptr;
    }
    return new PFont(absolute_path, size);
}

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
    IMPL_set_texture(img);
}

void PGraphics::point(const float x, const float y, const float z) {
    beginShape(POINTS);
    vertex(x, y, z);
    endShape();
}

void PGraphics::pointSize(const float size) {
    point_size = size;
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
    beginShape(TRIANGLES);
    for (const auto& v: box_vertices_LUT) {
        vertex(Vertex{glm::vec4(v.position.x * width, v.position.y * height, v.position.z * depth, v.position.w),
                      v.color,
                      v.tex_coord,
                      v.normal});
    }
    endShape();
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

void PGraphics::triangulate_line_strip_vertex(const std::vector<Vertex>& line_strip,
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
                           stroke_weight,
                           stroke_join_mode,
                           stroke_cap_mode,
                           stroke_join_round_resolution,
                           stroke_cap_round_resolution,
                           stroke_join_miter_max_angle,
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

// void PGraphics::beginCamera() {
//     in_camera_block        = true;
//     temp_view_matrix       = view_matrix;
//     temp_projection_matrix = projection_matrix;
// }
//
// void PGraphics::endCamera() {
//     if (in_camera_block) {
//         view_matrix       = temp_view_matrix;
//         projection_matrix = temp_projection_matrix;
//         in_camera_block   = false;
//     }
// }

void PGraphics::camera(const float eyeX, const float eyeY, const float eyeZ,
                       const float centerX, const float centerY, const float centerZ,
                       const float upX, const float upY, const float upZ) {
    const glm::vec3 eye(eyeX, eyeY, eyeZ);
    const glm::vec3 center(centerX, centerY, centerZ);
    const glm::vec3 up(upX, upY, upZ);

    if (in_camera_block) {
        temp_view_matrix = glm::lookAt(eye, center, up);
    } else {
        view_matrix = glm::lookAt(eye, center, up);
    }
}

void PGraphics::camera() {
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
        projection_matrix = proj;
    }
}

void PGraphics::ortho(const float left, const float right, const float bottom, const float top, const float near, const float far) {
    const glm::mat4 proj = glm::ortho(left, right, bottom, top, near, far);
    if (in_camera_block) {
        temp_projection_matrix = proj;
    } else {
        projection_matrix = proj;
    }
}

void PGraphics::perspective(const float fovy, const float aspect, const float near, const float far) {
    const glm::mat4 proj = glm::perspective(fovy, aspect, near, far);
    if (in_camera_block) {
        temp_projection_matrix = proj;
    } else {
        projection_matrix = proj;
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
    shape_mode_cache = shape;
    shape_has_begun  = true;
}


void PGraphics::endShape(const bool close_shape) {
    /*
     * OpenGL ES 3.1 is stricter:
     *
     * 1. No GL_LINES, GL_LINE_STRIP, or GL_LINE_LOOP support in core spec!
     * 2. No glLineWidth support at all.
     * 3. Only GL_TRIANGLES, GL_TRIANGLE_STRIP, and GL_TRIANGLE_FAN are guaranteed.
     *
     * i.e GL_LINES + GL_LINE_STRIP must be emulated
     */
    process_collected_fill_vertices();
    process_collected_stroke_vertices(close_shape);
    shape_fill_vertex_buffer.clear();
    shape_stroke_vertex_buffer.clear();
    shape_has_begun = false;
}

void PGraphics::emit_shape_fill_triangles(std::vector<Vertex>& triangle_vertices) {
    if (triangle_emitter_callback) {
        triangle_emitter_callback(triangle_vertices);
    }
    IMPL_emit_shape_fill_triangles(triangle_vertices);
}

void PGraphics::emit_shape_stroke_line_strip(std::vector<Vertex>& line_strip_vertices, const bool line_strip_closed) {
    if (stroke_emitter_callback) {
        stroke_emitter_callback(line_strip_vertices, line_strip_closed);
    }
    IMPL_emit_shape_stroke_line_strip(line_strip_vertices, line_strip_closed);
}

void PGraphics::emit_shape_stroke_points(std::vector<Vertex>& point_vertices, float point_size) {
    // TODO maybe add a callback for points or send strokes with one vertex TBD?
    // if (stroke_emitter_callback) {
    //     stroke_emitter_callback(point_vertices);
    // }
    IMPL_emit_shape_stroke_points(point_vertices, point_size);
}

void PGraphics::process_collected_fill_vertices() {
    const int tmp_shape_mode_cache = shape_mode_cache;
    if (!shape_fill_vertex_buffer.empty()) {
        switch (tmp_shape_mode_cache) {
            case POINTS:
            case LINES:
            case LINE_STRIP:
                break;
            case TRIANGLES:
                emit_shape_fill_triangles(shape_fill_vertex_buffer);
                break;
            case TRIANGLE_FAN: {
                std::vector<Vertex> vertices_fill_quads = convertTriangleFanToTriangles(shape_fill_vertex_buffer);
                emit_shape_fill_triangles(vertices_fill_quads);
            } break;
            case QUAD_STRIP: // NOTE does this just work?!?
            case TRIANGLE_STRIP: {
                std::vector<Vertex> vertices_fill_quads = convertTriangleStripToTriangles(shape_fill_vertex_buffer);
                emit_shape_fill_triangles(vertices_fill_quads);
            } break;
            case QUADS: {
                std::vector<Vertex> vertices_fill_quads = convertQuadsToTriangles(shape_fill_vertex_buffer);
                emit_shape_fill_triangles(vertices_fill_quads);
            } break;
            default:
            case POLYGON: {
                std::vector<Vertex> vertices_fill_polygon;
                if (polygon_triangulation_strategy == POLYGON_TRIANGULATION_FASTER) {
                    // EARCUT :: supports concave polygons, textures but no holes or self-intersection
                    vertices_fill_polygon = triangulate_faster(shape_fill_vertex_buffer);
                } else if (polygon_triangulation_strategy == POLYGON_TRIANGULATION_BETTER) {
                    // LIBTESS2 :: supports concave polygons, textures, holes and self-intersection but no textures
                    vertices_fill_polygon = triangulate_good(shape_fill_vertex_buffer);
                } else if (polygon_triangulation_strategy == POLYGON_TRIANGULATION_MID) {
                    // POLYPARTITION + CLIPPER2 // TODO maybe remove this option
                    vertices_fill_polygon = triangulate_better_quality(shape_fill_vertex_buffer);
                }
                emit_shape_fill_triangles(vertices_fill_polygon);
                // TODO what if polygon has only 3 ( triangle ) or 4 vertices ( quad )? could shortcut … here
            } break;
        }
    }
}

void PGraphics::process_collected_stroke_vertices(const bool close_shape) {
    const int tmp_shape_mode_cache = shape_mode_cache;
    if (!shape_stroke_vertex_buffer.empty()) {
        if (tmp_shape_mode_cache == POINTS) {
            // TODO STROKE_RENDER_MODE_NATIVE is handled in renderer
            // TODO decide if point is a fill shape or it s own shape
            // if (point_render_mode == POINT_RENDER_MODE_NATIVE) {
            //     // TODO does this still work under macOS? it renders squares … maybe texturize them
            //     // TODO @OpenGLES3.1 replace with circle or textured quad
            //     const float tmp_point_size = std::max(std::min(point_size, open_gl_capabilities.point_size_max), open_gl_capabilities.point_size_min);
            //     glPointSize(tmp_point_size);
            //     OGL_tranform_model_matrix_and_render_vertex_buffer(IM_primitive_shape, GL_POINTS, shape_stroke_vertex_buffer);
            // }
            if (point_render_mode == POINT_RENDER_MODE_TRIANGULATE) {
                std::vector<Vertex> line_vertices = convertPointsToTriangles(shape_stroke_vertex_buffer, point_size);
                emit_shape_fill_triangles(line_vertices);
            } else if (point_render_mode == POINT_RENDER_MODE_SHADER) {
                emit_shape_stroke_points(shape_stroke_vertex_buffer, point_size);
            }
            return; // NOTE strokes are rendered as points ... exit early
        }

        switch (tmp_shape_mode_cache) {
            case LINES: {
                const int buffer_size = shape_stroke_vertex_buffer.size() / 2 * 2;
                for (int i = 0; i < buffer_size; i += 2) {
                    std::vector line = {shape_stroke_vertex_buffer[i], shape_stroke_vertex_buffer[i + 1]};
                    emit_shape_stroke_line_strip(line, false);
                }
            } break;
            case TRIANGLE_FAN: {
                const std::vector<Vertex> _triangle_vertices = convertTriangleFanToTriangles(shape_stroke_vertex_buffer);
                const int                 buffer_size        = _triangle_vertices.size() / 3 * 3;
                for (int i = 0; i < buffer_size; i += 3) {
                    std::vector line = {_triangle_vertices[i], _triangle_vertices[i + 1], _triangle_vertices[i + 2]};
                    emit_shape_stroke_line_strip(line, true);
                }
            } break;
            case TRIANGLES: {
                const int buffer_size = shape_stroke_vertex_buffer.size() / 3 * 3;
                for (int i = 0; i < buffer_size; i += 3) {
                    std::vector line = {shape_stroke_vertex_buffer[i], shape_stroke_vertex_buffer[i + 1], shape_stroke_vertex_buffer[i + 2]};
                    emit_shape_stroke_line_strip(line, true);
                }
            } break;
            case TRIANGLE_STRIP: {
                const std::vector<Vertex> _triangle_vertices = convertTriangleStripToTriangles(shape_stroke_vertex_buffer);
                const int                 buffer_size        = _triangle_vertices.size() / 3 * 3;
                for (int i = 0; i < buffer_size; i += 3) {
                    std::vector line = {_triangle_vertices[i], _triangle_vertices[i + 1], _triangle_vertices[i + 2]};
                    emit_shape_stroke_line_strip(line, true);
                }
            } break;
            case QUAD_STRIP: {
                const std::vector<Vertex> _quad_vertices = convertQuadStripToQuads(shape_stroke_vertex_buffer);
                const int                 buffer_size    = _quad_vertices.size() / 4 * 4;
                for (int i = 0; i < buffer_size; i += 4) {
                    std::vector line = {_quad_vertices[i], _quad_vertices[i + 1], _quad_vertices[i + 2], _quad_vertices[i + 3]};
                    emit_shape_stroke_line_strip(line, true);
                }
            } break;
            case LINE_STRIP: {
                emit_shape_stroke_line_strip(shape_stroke_vertex_buffer, false);
            } break;
            case QUADS: {
                const int buffer_size = shape_stroke_vertex_buffer.size() / 4 * 4;
                for (int i = 0; i < buffer_size; i += 4) {
                    std::vector line = {shape_stroke_vertex_buffer[i], shape_stroke_vertex_buffer[i + 1], shape_stroke_vertex_buffer[i + 2], shape_stroke_vertex_buffer[i + 3]};
                    emit_shape_stroke_line_strip(line, true);
                }
            }
            default:
            case POLYGON: {
                emit_shape_stroke_line_strip(shape_stroke_vertex_buffer, close_shape);
            } break;
        }
    }
}

void PGraphics::vertex(const float x, const float y, const float z) {
    vertex(x, y, z, 0, 0);
}

void PGraphics::vertex(const float x, const float y, const float z, const float u, const float v) {
    if (!color_stroke.active && !color_fill.active) {
        return;
    }

    const glm::vec3 position{x, y, z};

    if (color_stroke.active) {
        const glm::vec4 strokeColor = as_vec4(color_stroke);
        shape_stroke_vertex_buffer.emplace_back(position, strokeColor, glm::vec3{u, v, 0.0f}, current_normal);
    }

    if (color_fill.active) {
        const glm::vec4 fillColor = as_vec4(color_fill);
        shape_fill_vertex_buffer.emplace_back(position, fillColor, glm::vec3{u, v, 0.0f}, current_normal);
    }
}
void PGraphics::vertex(Vertex v) {
    if (!color_stroke.active && !color_fill.active) {
        return;
    }

    if (color_stroke.active) {
        const glm::vec4 strokeColor = as_vec4(color_stroke);
        shape_stroke_vertex_buffer.emplace_back(v.position, strokeColor, v.tex_coord, v.normal);
    }

    if (color_fill.active) {
        const glm::vec4 fillColor = as_vec4(color_fill);
        // TODO maybe use v.color instead of color_fill?
        // shape_fill_vertex_buffer.emplace_back(v);
        shape_fill_vertex_buffer.emplace_back(v.position, fillColor, v.tex_coord, v.normal);
    }
}
