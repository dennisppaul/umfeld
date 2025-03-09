/*
 * Umgebung
 *
 * This file is part of the *Umgebung* library (https://github.com/dennisppaul/umgebung).
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

#include "UmgebungFunctionsAdditional.h"
#include "PGraphics.h"
#include "Vertex.h"

#include <UmgebungFunctionsPGraphics.h>

using namespace umgebung;

PGraphics::PGraphics() : PImage(0, 0, 0) {
    PGraphics::fill(1.0f);
    PGraphics::stroke(0.0f);
    PGraphics::ellipseDetail(ELLIPSE_DETAIL_DEFAULT);
}

/* --- transform matrices --- */

// NOTE: done
void PGraphics::popMatrix() {
    if (!model_matrix_stack.empty()) {
        model_matrix_client = model_matrix_stack.back();
        model_matrix_stack.pop_back();
    }
}

// NOTE: done
void PGraphics::pushMatrix() {
    model_matrix_stack.push_back(model_matrix_client);
}

void PGraphics::resetMatrix() {
    model_matrix_client = glm::mat4(1.0f);
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
    printMatrix(model_matrix_client);
}

// NOTE: done
void PGraphics::translate(const float x, const float y, const float z) {
    model_matrix_client = glm::translate(model_matrix_client, glm::vec3(x, y, z));
    model_matrix_dirty  = true;
}

// NOTE: done
void PGraphics::rotateX(const float angle) {
    model_matrix_client = glm::rotate(model_matrix_client, angle, glm::vec3(1.0f, 0.0f, 0.0f));
    model_matrix_dirty  = true;
}

// NOTE: done
void PGraphics::rotateY(const float angle) {
    model_matrix_client = glm::rotate(model_matrix_client, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    model_matrix_dirty  = true;
}

// NOTE: done
void PGraphics::rotateZ(const float angle) {
    model_matrix_client = glm::rotate(model_matrix_client, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    model_matrix_dirty  = true;
}

// NOTE: done
void PGraphics::rotate(const float angle) {
    model_matrix_client = glm::rotate(model_matrix_client, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    model_matrix_dirty  = true;
}

// NOTE: done
void PGraphics::rotate(const float angle, const float x, const float y, const float z) {
    model_matrix_client = glm::rotate(model_matrix_client, angle, glm::vec3(x, y, z));
    model_matrix_dirty  = true;
}

// NOTE: done
void PGraphics::scale(const float x) {
    model_matrix_client = glm::scale(model_matrix_client, glm::vec3(x, x, x));
    model_matrix_dirty  = true;
}

// NOTE: done
void PGraphics::scale(const float x, const float y) {
    model_matrix_client = glm::scale(model_matrix_client, glm::vec3(x, y, 1));
    model_matrix_dirty  = true;
}

// NOTE: done
void PGraphics::scale(const float x, const float y, const float z) {
    model_matrix_client = glm::scale(model_matrix_client, glm::vec3(x, y, z));
    model_matrix_dirty  = true;
}

/* --- color, stroke, and fill --- */

void PGraphics::fill(const float r, const float g, const float b, const float alpha) {
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
    color_inv(c, color_fill.r, color_fill.g, color_fill.b, color_fill.a);
    color_fill.active = true;
}

void PGraphics::noFill() {
    color_fill.active = false;
}

void PGraphics::stroke(const float r, const float g, const float b, const float alpha) {
    color_stroke.r      = r;
    color_stroke.g      = g;
    color_stroke.b      = b;
    color_stroke.a      = alpha;
    color_stroke.active = true;
}

void PGraphics::stroke(const float gray, const float alpha) {
    color_stroke.r      = gray;
    color_stroke.g      = gray;
    color_stroke.b      = gray;
    color_stroke.a      = alpha;
    color_stroke.active = true;
}

void PGraphics::stroke_color(const uint32_t c) {
    color_inv(c, color_stroke.r, color_stroke.g, color_stroke.b, color_stroke.a);
    color_stroke.active = true;
}

void PGraphics::stroke(const float a) {
    stroke(a, a, a);
}

void PGraphics::noStroke() {
    color_stroke.active = false;
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

void PGraphics::pointSize(const float size) { point_size = size < 1 ? 1 : size; }

void PGraphics::resize_ellipse_points_LUT() {
    if (ellipse_detail < ELLIPSE_DETAIL_MIN) {
        return;
    }
    ellipse_points_LUT.clear();
    ellipse_points_LUT.resize(ellipse_detail + 1); // Resize instead of reserve

    constexpr float PI         = 3.14159265358979323846f;
    const float     deltaTheta = (2.0f * PI) / static_cast<float>(ellipse_detail);

    for (int i = 0; i <= ellipse_detail; ++i) {
        const float theta     = deltaTheta * static_cast<float>(i);
        ellipse_points_LUT[i] = {std::cos(theta), std::sin(theta)};
    }
}

/* --- triangulation --- */

// EARCUT

std::vector<Vertex> PGraphics::triangulate_faster(const std::vector<Vertex>& vertices) {
    std::vector<std::vector<std::array<float, 2>>> polygon;
    polygon.emplace_back(); // Outer boundary

    // convert Vertex to 2D format for earcut (ignore z)
    for (const auto& v: vertices) {
        polygon[0].push_back({v.position.x, v.position.y});
    }

    // perform triangulation
    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);
    std::vector<Vertex>   triangleList;

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

const Triangulator PGraphics::triangulator;

std::vector<Vertex> PGraphics::triangulate_good(const std::vector<Vertex>& vertices) {
    const std::vector<Vertex> triangles = triangulator.triangulate(vertices, Triangulator::Winding::WINDING_ODD);
    return triangles;
}

std::vector<Vertex> PGraphics::triangulate_better_quality(const std::vector<Vertex>& vertices) {
    glm::vec4 first_color = vertices[0].color;

    TPPLPartition partitioner;

    // Step 1: Use Clipper2 to resolve self-intersections
    const Clipper2Lib::PathsD inputPaths = {convertToClipperPath(vertices)};
    const Clipper2Lib::PathsD fixedPaths = Clipper2Lib::Union(inputPaths, Clipper2Lib::FillRule::NonZero);

    if (fixedPaths.empty()) {
        std::cerr << "Clipper2 failed to fix the polygon!" << std::endl;
        return {};
    }

    // Step 2: Convert to PolyPartition format
    std::vector<TPPLPoly> convexPolygons;
    for (auto& poly: convertToPolyPartition(fixedPaths)) {
        std::list<TPPLPoly> convexParts;
        if (!partitioner.ConvexPartition_HM(&poly, &convexParts)) {
            std::cerr << "Convex partitioning failed!" << std::endl;
            continue;
        }
        convexPolygons.insert(convexPolygons.end(), convexParts.begin(), convexParts.end());
    }

    // Step 3: Triangulate each convex part
    std::vector<Vertex> triangleList;
    for (auto& part: convexPolygons) {
        std::list<TPPLPoly> triangles;
        if (!partitioner.Triangulate_EC(&part, &triangles)) {
            std::cerr << "Triangulation failed for a convex part!" << std::endl;
            continue;
        }

        // Extract triangle vertices
        for (const auto& tri: triangles) {
            for (int i = 0; i < 3; ++i) {
                triangleList.push_back({static_cast<float>(tri[i].x), static_cast<float>(tri[i].y), 0.0f,
                                        first_color.r, first_color.g, first_color.b, first_color.a,
                                        0, 0});
            }
        }
    }
    return triangleList;
}

// void PGraphicsOpenGLv33::reset_matrices() {
//     model_matrix_shader = glm::mat4(1.0f);
//     model_matrix_client = glm::mat4(1.0f);
//     model_matrix_dirty  = false;
//
//     const float viewport_width  = framebuffer_width;
//     const float viewport_height = framebuffer_height;
//
//     glViewport(0, 0, static_cast<GLint>(viewport_width), static_cast<GLint>(viewport_height));
//
//     // Orthographic projection
//     projection_matrix_2D = glm::ortho(0.0f, viewport_width, viewport_height, 0.0f);
//
//     const float fov            = DEFAULT_FOV;                       // distance from the camera = screen height
//     const float cameraDistance = (height / 2.0f) / tan(fov / 2.0f); // 1 unit = 1 pixel
//
//     // Perspective projection
//     projection_matrix_3D = glm::perspective(fov, width / height, 0.1f, static_cast<float>(depth_range));
//
//     view_matrix = glm::lookAt(
//         glm::vec3(width / 2.0f, height / 2.0f, -cameraDistance), // Flip Z to fix X-axis
//         glm::vec3(width / 2.0f, height / 2.0f, 0.0f),            // Look at the center
//         glm::vec3(0.0f, -1.0f, 0.0f)                             // Keep Y-up as normal
//     );
// }

void PGraphics::to_screen_space(glm::vec3& world_position) const {
    // Transform world position to camera (view) space
    const glm::vec4 viewPos = view_matrix * model_matrix_client * glm::vec4(world_position, 1.0f);

    // Project onto clip space
    glm::vec4 clipPos = projection_matrix_3D * viewPos;

    // Perspective divide (convert to normalized device coordinates)
    if (clipPos.w != 0.0f) {
        clipPos.x /= clipPos.w;
        clipPos.y /= clipPos.w;
    }

    // Now the coordinates are in NDC (-1 to 1 range)
    // Convert NDC to screen space (assuming viewport width and height)
    // TODO what is it? `width` or `framebuffer_width`
    // float screenX = (clipPos.x * 0.5f + 0.5f) * static_cast<float>(framebuffer_width);
    // float screenY = (1.0f - (clipPos.y * 0.5f + 0.5f)) * static_cast<float>(framebuffer_height);
    const float screenX = (clipPos.x * 0.5f + 0.5f) * static_cast<float>(width);
    const float screenY = (1.0f - (clipPos.y * 0.5f + 0.5f)) * static_cast<float>(height);

    world_position.x = screenX;
    world_position.y = screenY;
    world_position.z = 0.0f;
}

void PGraphics::to_world_space(glm::vec3& model_position) const {
    model_position = model_matrix_client * glm::vec4(model_position, 1.0f);
}
