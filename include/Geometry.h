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

#include <array>
#include <algorithm>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/geometric.hpp>

#include "UmfeldConstants.h"

// #define GEOMETRY_DRAW_DEBUG

// TODO check if all vertex attributes are copied i.e color and tex_coords
// std::vector<Vertex>        convertQuadStripToQuads(const std::vector<Vertex>& quadStrip) const;
// std::vector<Vertex>        convertPointsToTriangles(const std::vector<Vertex>& points, float size) const;
// static std::vector<Vertex> convertQuadsToTriangles(const std::vector<Vertex>& quads);
// static std::vector<Vertex> convertPolygonToTriangleFan(const std::vector<Vertex>& polygon);
// std::vector<Vertex>        convertTriangleFanToTriangles(const std::vector<Vertex>& fan) const;
// std::vector<Vertex>        convertTriangleStripToTriangles(const std::vector<Vertex>& strip) const;

namespace umfeld {

    inline bool winding_order = CCW;

    struct Segment {
        glm::vec2 position;
        glm::vec2 normal;
        glm::vec2 direction;
        glm::vec2 next_position;
    };

    // Struct to hold a triangle with computed depth
    struct Triangle {
        Vertex v0, v1, v2;
        float  depth; // Depth for sorting
    };

    /**
     * depth sorting function ( back-to-front )
     * @param triangles
     * @param cameraPosition
     */
    inline void depth_sort_triangles(std::vector<Triangle>& triangles, const glm::vec3& cameraPosition) {
        // compute depth for each triangle
        for (auto& tri: triangles) {
            glm::vec3 centroid = (tri.v0.position + tri.v1.position + tri.v2.position) / 3.0f;
            tri.depth          = glm::dot(cameraPosition - centroid, cameraPosition - centroid); // Squared distance for efficiency
        }

        // sort triangles by depth (Back-to-Front)
        std::sort(triangles.begin(), triangles.end(), [](const Triangle& a, const Triangle& b) {
            return a.depth > b.depth; // Sorting from farthest to closest
        });
        // TODO one day replace by Weighted Blended Order-Independent Transparency (WBOIT)
    }

    inline void triangulate_polygon(const std::vector<glm::vec2>& polygon_outline,
                                    std::vector<glm::vec2>&       resulting_triangles) {
        TESStesselator* tess = tessNewTess(nullptr);
        tessAddContour(tess, 2, polygon_outline.data(), sizeof(glm::vec2), polygon_outline.size());
        if (tessTesselate(tess, TESS_WINDING_NONZERO, TESS_POLYGONS, 3, 2, nullptr)) {
            const float*     verts  = tessGetVertices(tess);
            const TESSindex* elems  = tessGetElements(tess);
            const int        nelems = tessGetElementCount(tess);
            resulting_triangles.reserve(nelems * 3);
            for (int i = 0; i < nelems; ++i) {
                const TESSindex* tri = &elems[i * 3];
                resulting_triangles.insert(resulting_triangles.end(), {glm::make_vec2(&verts[tri[0] * 2]),
                                                                       glm::make_vec2(&verts[tri[1] * 2]),
                                                                       glm::make_vec2(&verts[tri[2] * 2])});
            }
        }
        tessDeleteTess(tess);
    }

    inline float angle_between_vectors(const glm::vec2& d1, const glm::vec2& d2) {
        const float determinant = d1.x * d2.y - d1.y * d2.x;
        const float angle       = std::atan2(determinant, glm::dot(d1, d2));
        return angle;
    }

    inline bool are_almost_parallel(const glm::vec3& n1, const glm::vec3& n2, const float epsilon = 0.01f) {
        const float dotProduct = glm::dot(n1, n2);
        return -dotProduct > 1.0f - epsilon; // Closer to 1 or -1 means nearly parallel
        // return fabs(dotProduct) > (1.0f - epsilon); // Closer to 1 or -1 means nearly parallel
    }

    inline bool intersect_lines(const glm::vec2& p1, const glm::vec2& d1,
                                const glm::vec2& p2, const glm::vec2& d2,
                                glm::vec2& intersection) {
        const float det = d1.x * d2.y - d1.y * d2.x;

        if (fabs(det) < 1e-6f) {
            return false; // Parallel or coincident lines
        }

        const float t = ((p2.x - p1.x) * d2.y - (p2.y - p1.y) * d2.x) / det;

        intersection = glm::vec2(p1 + t * d1);
        return true;
    }

    inline void add_cap(const Segment&          cap_seg,
                        const float             half_width,
                        const int               stroke_cap_mode,
                        const float             direction,
                        const float             stroke_cap_round_resolution,
                        std::vector<glm::vec2>& poly_list) {
        /* POINTED */
        if (stroke_cap_mode == POINTED) {
            const glm::vec2 cap_point_project = cap_seg.position - cap_seg.direction * half_width * direction;
            poly_list.push_back(cap_point_project);
#ifdef GEOMETRY_DRAW_DEBUG
            fill(0, 0.5f, 1);
            circle(cap_point_project.x, cap_point_project.y, 10);
#endif // GEOMETRY_DRAW_DEBUG
        }
        /* PROJECT */
        if (stroke_cap_mode == PROJECT) {
            const glm::vec2 cap_point_project = cap_seg.position - cap_seg.direction * half_width * direction;
            const glm::vec2 cap_norm          = cap_seg.normal * half_width;
            poly_list.push_back(cap_point_project + cap_norm);
            poly_list.push_back(cap_point_project - cap_norm);
        }
        /* ROUNG */
        if (stroke_cap_mode == ROUND) {
            const float angle       = atan2(cap_seg.normal.y, cap_seg.normal.x) - 3.0f * HALF_PI * direction;
            const float angle_start = angle - HALF_PI;
            const float angle_end   = angle + HALF_PI;
            const float step        = direction * stroke_cap_round_resolution;
            const bool  forward     = direction == 1.0f;
            for (float r = forward ? angle_start : angle_end; forward ? r <= angle_end : r >= angle_start; r += step) {
                glm::vec2 circle_segment = cap_seg.position + glm::vec2{cos(r), sin(r)} * half_width;
                poly_list.push_back(circle_segment);
#ifdef GEOMETRY_DRAW_DEBUG
                fill(0, 0.5f, 1);
                circle(circle_segment.x, circle_segment.y, 3);
#endif // GEOMETRY_DRAW_DEBUG
            }
        }
        /* SQUARE */
        // NOTE nothing to do here
    }

    inline void create_stroke_join_tessellate(std::vector<glm::vec2>&     triangles,
                                              const bool                  close_shape,
                                              const float                 half_width,
                                              const int                   stroke_join_mode,
                                              const int                   stroke_cap_mode,
                                              const float                 stroke_join_round_resolution,
                                              const float                 stroke_cap_round_resolution,
                                              const float                 stroke_join_miter_max_angle,
                                              const std::vector<Segment>& segments) {
        std::vector<glm::vec2> outline_left;
        std::vector<glm::vec2> outline_right;
        outline_left.reserve(segments.size());
        outline_right.reserve(segments.size());

        /* CAP START */
        if (!close_shape) {
            constexpr int           seg_index = 0;
            constexpr float         direction = 1.0f;
            std::vector<glm::vec2>& poly_list = outline_left;
            const Segment           cap_seg   = segments[seg_index];
            add_cap(cap_seg, half_width, stroke_cap_mode, direction, stroke_cap_round_resolution, poly_list);
        }

        const int num_segments = segments.size() + (close_shape ? +1 : 0);
        for (int i = 0; i < num_segments; ++i) {
            const Segment& s1 = segments[i % segments.size()];
            const Segment& s2 = segments[(i + 1) % segments.size()];
#ifdef GEOMETRY_DRAW_DEBUG
            stroke(0);
            fill(0);
            circle(s1.position.x, s1.position.y, 10);
            noFill();
            circle(s2.position.x, s2.position.y, 15);
#endif // GEOMETRY_DRAW_DEBUG
            const glm::vec2 s1_norm  = s1.normal * half_width;
            const glm::vec2 s1_left  = s1.position - s1_norm;
            const glm::vec2 s1_right = s1.position + s1_norm;
            const glm::vec2 s2_left  = s2.position - s1_norm;
            const glm::vec2 s2_right = s2.position + s1_norm;
            const glm::vec2 s2_norm  = s2.normal * half_width;
#ifdef GEOMETRY_DRAW_DEBUG
            fill(1, 0, 0);
            circle(s1_left.x, s1_left.y, 10 + i * 2);
            fill(0, 1, 0);
            circle(s1_right.x, s1_right.y, 10 + i * 2);

            noFill();
            stroke(1, 0, 0);
            circle(s2_left.x, s2_left.y, 10 + i * 2);
            stroke(0, 1, 0);
            circle(s2_right.x, s2_right.y, 10 + i * 2);
#endif // GEOMETRY_DRAW_DEBUG
            if (close_shape) {
                outline_left.push_back(s1_left);
                outline_left.push_back(s2_left);
                outline_right.push_back(s1_right);
                outline_right.push_back(s2_right);
            } else {
                outline_left.push_back(s1_left);
                outline_right.push_back(s1_right);
                if (i < num_segments - 1) { // works for round & bevel but not vor miter
                    outline_left.push_back(s2_left);
                    outline_right.push_back(s2_right);
                }
            }

            /* ROUND */

            if (stroke_join_mode == ROUND) {
                // skip last one ( creates a cap )
                if (i != num_segments - 1) {
                    float angle_s1 = atan2(s1.normal.y, s1.normal.x);
                    float angle_s2 = atan2(s2.normal.y, s2.normal.x);
                    // compute the signed angular difference
                    float angular_diff = angle_s2 - angle_s1;
                    while (angular_diff <= -PI) {
                        angular_diff += TWO_PI;
                    }
                    while (angular_diff > PI) {
                        angular_diff -= TWO_PI;
                    }
                    float r_offset = angular_diff > 0 ? PI : 0.0f;
                    // flip the start/end if segment goes in the wrong direction
                    if (angular_diff < 0) {
                        std::swap(angle_s1, angle_s2);
                        angular_diff = -angular_diff;
                    }
                    for (float r = angle_s1; r <= angle_s1 + angular_diff; r += stroke_join_round_resolution) {
                        glm::vec2 circle_segment = s2.position + glm::vec2{
                                                                     cos(r + r_offset),
                                                                     sin(r + r_offset)} *
                                                                     half_width;
                        outline_left.push_back(circle_segment);
                        // TODO check if it would be better to add this to right side depending on angle
                    }
                }
            }

            /* BEVEL */

            // NOTE no need to do anything special

            /* MITER */

            if (stroke_join_mode == MITER) {
                if (close_shape || i < num_segments - 1) {
                    const float angle     = glm::degrees(angle_between_vectors(s1.direction, s2.direction));
                    const float abs_angle = fabs(angle);
                    if (abs_angle < stroke_join_miter_max_angle && angle > 0) {
                        const glm::vec2 s2_proj_left = s2.position - s2_norm;
                        glm::vec2       intersection_left;
                        const bool      result_left = intersect_lines(s1_left, s1.direction,
                                                                      s2_proj_left, s2.direction,
                                                                      intersection_left);
                        if (result_left) {
                            outline_left.push_back(intersection_left);
                        }
                    }
                    if (abs_angle < stroke_join_miter_max_angle && angle < 0) {
                        const glm::vec2 p2_proj_right = s2.position + s2_norm;
                        glm::vec2       intersection_right;
                        const bool      result_right = intersect_lines(s1_right, s1.direction,
                                                                       p2_proj_right, s2.direction,
                                                                       intersection_right);
                        if (result_right) {
                            outline_right.push_back(intersection_right);
                        }
                    }
                }
            }
        }

        /* CAP END */
        if (!close_shape) {
            const int               seg_index = segments.size() - 1;
            constexpr float         direction = -1.0f;
            std::vector<glm::vec2>& poly_list = outline_right;
            const Segment           cap_seg   = segments[seg_index];
            add_cap(cap_seg, half_width, stroke_cap_mode, direction, stroke_cap_round_resolution, poly_list);
        }

        std::vector<glm::vec2> polygon_outline;
        polygon_outline.insert(polygon_outline.end(), outline_left.begin(), outline_left.end());
        polygon_outline.insert(polygon_outline.end(), outline_right.rbegin(), outline_right.rend());
        triangulate_polygon(polygon_outline, triangles);
    }

    inline void add_quad_as_triangles(std::vector<glm::vec2>& triangles,
                                      const glm::vec2&        p1,
                                      const glm::vec2&        p2,
                                      const glm::vec2&        p3,
                                      const glm::vec2&        p4) {
        triangles.emplace_back(p1);
        triangles.emplace_back(p2);
        triangles.emplace_back(p4);
        triangles.emplace_back(p4);
        triangles.emplace_back(p2);
        triangles.emplace_back(p3);
    }

    inline void create_stroke_join_bevel(std::vector<glm::vec2>& triangles,
                                         const bool              close_shape,
                                         const float             half_width,
                                         std::vector<Segment>&   segments) {
        glm::vec2 p_prev_left{};
        glm::vec2 p_prev_right{};
        const int num_segments = segments.size() - (close_shape ? 0 : 1);
        for (int i = 0; i < num_segments; ++i) {
            Segment&        s1           = segments[i];
            const glm::vec2 p1_line_norm = s1.normal * half_width;
            const glm::vec2 p1_left      = s1.position + p1_line_norm;
            const glm::vec2 p1_right     = s1.position - p1_line_norm;
            const glm::vec2 p2_left      = s1.next_position + p1_line_norm;
            const glm::vec2 p2_right     = s1.next_position - p1_line_norm;
            add_quad_as_triangles(triangles,
                                  p1_left,
                                  p2_left,
                                  p2_right,
                                  p1_right);
            if (i != 0) {
                Segment&    s2    = segments[i - 1];
                const float angle = glm::degrees(angle_between_vectors(s1.direction, s2.direction));
                if (angle > 0) {
                    triangles.emplace_back(s1.position);
                    triangles.emplace_back(p_prev_left);
                    triangles.emplace_back(p1_left);
                } else {
                    triangles.emplace_back(s1.position);
                    triangles.emplace_back(p_prev_right);
                    triangles.emplace_back(p1_right);
                }
            }
            p_prev_left  = p2_left;
            p_prev_right = p2_right;
        }
        if (close_shape) {
            Segment&        s1           = segments[0];
            Segment&        s2           = segments[segments.size() - 1];
            const float     angle        = glm::degrees(angle_between_vectors(s1.direction, s2.direction));
            const glm::vec2 p1_line_norm = s1.normal * half_width;
            const glm::vec2 p1_left      = s1.position + p1_line_norm;
            const glm::vec2 p1_right     = s1.position - p1_line_norm;
            if (angle > 0) {
                triangles.emplace_back(s1.position);
                triangles.emplace_back(p_prev_left);
                triangles.emplace_back(p1_left);
            } else {
                triangles.emplace_back(s1.position);
                triangles.emplace_back(p_prev_right);
                triangles.emplace_back(p1_right);
            }
        }
    }

    inline void create_stroke_join_none(std::vector<glm::vec2>& triangles,
                                        const bool              close_shape,
                                        const float             half_width,
                                        std::vector<Segment>&   segments) {
        const int num_segments = segments.size() - (close_shape ? 0 : 1);
        for (int i = 0; i < num_segments; ++i) {
            Segment&        s1           = segments[i];
            const glm::vec2 p1_line_norm = s1.normal * half_width;
            const glm::vec2 p1_left      = s1.position + p1_line_norm;
            const glm::vec2 p1_right     = s1.position - p1_line_norm;
            const glm::vec2 p2_left      = s1.next_position + p1_line_norm;
            const glm::vec2 p2_right     = s1.next_position - p1_line_norm;
            add_quad_as_triangles(triangles,
                                  p1_left,
                                  p2_left,
                                  p2_right,
                                  p1_right);
        }
    }

    inline void create_stroke_join_miter(std::vector<glm::vec2>& triangles,
                                         const bool              close_shape,
                                         const float             half_width,
                                         const float             stroke_join_miter_max_angle,
                                         std::vector<Segment>&   segments) {
        glm::vec2 p_prev_left{};
        glm::vec2 p_prev_right{};

        const int num_segments = segments.size() + (close_shape ? 1 : -1);
        for (int i = 0; i < num_segments; ++i) {
            const int      j  = i % segments.size();
            const int      jj = (i + 1) % segments.size();
            const Segment& s1 = segments[j];
            const Segment& s2 = segments[jj];

            const glm::vec2 p1_line_norm = s1.normal * half_width;
            const glm::vec2 p1_left      = s1.position + p1_line_norm;
            const glm::vec2 p1_right     = s1.position - p1_line_norm;

            const glm::vec2 p2_line_norm = s2.normal * half_width;
            const glm::vec2 p2_left      = s2.position + p2_line_norm;
            const glm::vec2 p2_right     = s2.position - p2_line_norm;

            glm::vec2  intersection_left;
            const bool result_left = intersect_lines(p1_left, s1.direction,
                                                     p2_left, s2.direction,
                                                     intersection_left);
            glm::vec2  intersection_right;
            const bool result_right = intersect_lines(p1_right, s1.direction,
                                                      p2_right, s2.direction,
                                                      intersection_right);

            if (!result_left) {
                intersection_left = s2.position + p1_line_norm;
            }

            if (!result_right) {
                intersection_right = s2.position - p1_line_norm;
            }

            bool        flip_intersection = false;
            const float angle             = glm::degrees(angle_between_vectors(s1.direction, s2.direction));
            if (fabs(angle) > stroke_join_miter_max_angle) {
                intersection_left  = s1.next_position + p1_line_norm;
                intersection_right = s1.next_position - p1_line_norm;
                flip_intersection  = true;
            }

            if (i != 0) {
                add_quad_as_triangles(triangles,
                                      p_prev_left,
                                      intersection_left,
                                      intersection_right,
                                      p_prev_right);
            } else {
                if (!close_shape) {
                    add_quad_as_triangles(triangles,
                                          p1_left,
                                          intersection_left,
                                          intersection_right,
                                          p1_right);
                }
            }

            if (flip_intersection) {
                p_prev_left  = intersection_right;
                p_prev_right = intersection_left;
            } else {
                p_prev_left  = intersection_left;
                p_prev_right = intersection_right;
            }
        }
    }

    inline void triangulate_line_strip(const std::vector<glm::vec2>& line_strip,
                                       bool                          close_shape,
                                       const float                   stroke_weight,
                                       const int                     stroke_join_mode,
                                       const int                     stroke_cap_mode,
                                       const float                   stroke_join_round_resolution,
                                       const float                   stroke_cap_round_resolution,
                                       const float                   stroke_join_miter_max_angle,
                                       std::vector<glm::vec2>&       triangle_vertices) {

        // NOTE join mode NONE, MITER_FAST, and BEVEL_FAST ignore stroke caps
        // NOTE if stroke weight is 1 it is best to use NONE for best performance

        if (stroke_weight <= 0.0f) {
            return;
        }

        if (line_strip.size() < 2) {
            return;
        }

        if (line_strip.size() < 3) {
            close_shape = false;
        }

        const float half_width = stroke_weight * 0.5f;

        std::vector<Segment> segments;
        segments.reserve(line_strip.size());
        const int size = static_cast<int>(line_strip.size());

        for (int i = 0; i < size; ++i) {
            Segment s;
            s.position = glm::vec2(line_strip[i].x, line_strip[i].y);
            if (i == size - 1) {   // last point needs special care
                if (close_shape) { // copy first point for closed shapes
                    s.next_position = glm::vec2(line_strip[0].x, line_strip[0].y);
                } else { // project point last for open shapes
                    s.next_position = line_strip[i] + (line_strip[i] - line_strip[i - 1]);
                }
            } else { // for all other segments use next point
                const int ii    = (i + 1) % size;
                s.next_position = glm::vec2(line_strip[ii].x, line_strip[ii].y);
            }
            s.direction = s.next_position - s.position;
            if (s.direction.x * s.direction.x + s.direction.y + s.direction.y > 0.0001f * 0.0001f) {
                s.direction = glm::normalize(s.direction);
                s.normal    = glm::vec2(-s.direction.y, s.direction.x);
            } else {
                s.normal = glm::vec2(1.0f, 0.0f);
            }
            segments.emplace_back(s);
        }

        switch (stroke_join_mode) {
            case NONE:
                create_stroke_join_none(triangle_vertices, close_shape, half_width, segments);
                break;
            case MITER_FAST:
                create_stroke_join_miter(triangle_vertices, close_shape, half_width, stroke_join_miter_max_angle, segments);
                break;
            case BEVEL_FAST:
                create_stroke_join_bevel(triangle_vertices, close_shape, half_width, segments);
                break;
            case BEVEL:
            case MITER:
            case ROUND:
            default:
                create_stroke_join_tessellate(triangle_vertices,
                                              close_shape,
                                              half_width,
                                              stroke_join_mode,
                                              stroke_cap_mode,
                                              stroke_join_round_resolution,
                                              stroke_cap_round_resolution,
                                              stroke_join_miter_max_angle,
                                              segments);
        }
    }


    inline std::vector<Vertex> convertPolygonToTriangleFan(const std::vector<Vertex>& polygon) {
        if (polygon.size() < 3) {
            return {};
        }

        std::vector<Vertex> fan;
        fan.push_back(polygon[0]); // Center vertex

        for (size_t i = 1; i < polygon.size() - 1; i++) {
            fan.push_back(polygon[i]);
            fan.push_back(polygon[i + 1]);
        }
        return fan;
    }

    inline std::vector<Vertex> convertTriangleStripToTriangles(const std::vector<Vertex>& strip) {
        if (strip.size() < 3) {
            return {}; // Not enough vertices for a triangle
        }

        const size_t        numTriangles = strip.size() - 2; // Number of triangles in the strip
        std::vector<Vertex> triangles;
        triangles.reserve(numTriangles * 3); // Pre-allocate memory

        for (size_t i = 0; i < numTriangles; ++i) {
            if (i % 2 == 0) {
                // Regular order: v[i], v[i+1], v[i+2]
                triangles.emplace_back(strip[i + 0]);
                triangles.emplace_back(strip[i + 1]);
                triangles.emplace_back(strip[i + 2]);
            } else {
                // Swapped order: v[i+1], v[i], v[i+2] (to maintain correct winding order)
                triangles.emplace_back(strip[i + 1]);
                triangles.emplace_back(strip[i + 0]);
                triangles.emplace_back(strip[i + 2]);
            }
        }

        return triangles;
    }

    inline std::vector<Vertex> convertTriangleFanToTriangles(const std::vector<Vertex>& fan) {
        if (fan.size() < 3) {
            return {}; // Not enough vertices for a triangle
        }

        const size_t        numTriangles = fan.size() - 2; // Number of triangles in the fan
        std::vector<Vertex> triangles;
        triangles.reserve(numTriangles * 3); // Pre-allocate memory

        const Vertex& anchor = fan[0]; // The first vertex is the anchor

        for (size_t i = 1; i < fan.size() - 1; ++i) {
            // Form a triangle using:
            // - anchor (fan[0])
            // - fan[i]
            // - fan[i + 1]
            triangles.emplace_back(anchor);
            triangles.emplace_back(fan[i]);
            triangles.emplace_back(fan[i + 1]);
        }
        return triangles;
    }

    inline std::vector<Vertex> convertQuadStripToQuads(const std::vector<Vertex>& quadStrip) {
        if (quadStrip.size() < 4) {
            return {}; // Not enough vertices to form at least one quad
        }

        const size_t        numQuads = (quadStrip.size() - 2) / 2; // Each quad requires 2 new vertices
        std::vector<Vertex> quads;
        quads.reserve(numQuads * 4); // Each quad has 4 vertices

        for (size_t i = 0; i < quadStrip.size() - 2; i += 2) {
            // Each quad consists of:
            // v0 = quadStrip[i]
            // v1 = quadStrip[i + 1]
            // v2 = quadStrip[i + 3]
            // v3 = quadStrip[i + 2]

            quads.emplace_back(quadStrip[i]);     // Bottom-left
            quads.emplace_back(quadStrip[i + 1]); // Bottom-right
            quads.emplace_back(quadStrip[i + 3]); // Top-right
            quads.emplace_back(quadStrip[i + 2]); // Top-left
        }

        return quads;
    }

    inline std::vector<Vertex> convertQuadsToTriangles(const std::vector<Vertex>& quads) {
        if (quads.size() < 4) {
            return {};
        }

        std::vector<Vertex> triangles;
        const size_t        validQuadCount = quads.size() / 4; // only use full quads

        for (size_t i = 0; i < validQuadCount * 4; i += 4) {
            // First triangle (0-1-2)
            triangles.push_back(quads[i + 0]);
            triangles.push_back(quads[i + 1]);
            triangles.push_back(quads[i + 2]);

            // Second triangle (2-3-1)
            triangles.push_back(quads[i + 2]);
            triangles.push_back(quads[i + 3]);
            triangles.push_back(quads[i + 0]);
        }
        return triangles;
    }

    inline std::vector<Vertex> convertPointsToTriangles(const std::vector<Vertex>& points, float size) {
        if (points.empty()) {
            return {};
        }

        std::vector<Vertex> triangles;
        triangles.reserve(points.size() * 6); // Each point → 2 triangles → 6 vertices

        float halfSize = size * 0.5f;

        for (const Vertex& p: points) {
            // Define the four corners of the quad centered at (p.x, p.y)
            Vertex v0 = {p.position.x - halfSize, p.position.y - halfSize, p.position.z}; // Bottom-left
            Vertex v1 = {p.position.x + halfSize, p.position.y - halfSize, p.position.z}; // Bottom-right
            Vertex v2 = {p.position.x + halfSize, p.position.y + halfSize, p.position.z}; // Top-right
            Vertex v3 = {p.position.x - halfSize, p.position.y + halfSize, p.position.z}; // Top-left

            v0.color = p.color;
            v1.color = p.color;
            v2.color = p.color;
            v3.color = p.color;

            // Triangle 1: v0 → v1 → v2
            triangles.emplace_back(v0);
            triangles.emplace_back(v1);
            triangles.emplace_back(v2);

            // Triangle 2: v2 → v3 → v0
            triangles.emplace_back(v2);
            triangles.emplace_back(v3);
            triangles.emplace_back(v0);
        }

        return triangles;
    }

    inline void generate_box(std::vector<Vertex>& vertices) {
        // 8 corner points of the unit cube
        glm::vec3 p0(-0.5f, -0.5f, -0.5f); // Back face
        glm::vec3 p1(0.5f, -0.5f, -0.5f);
        glm::vec3 p2(0.5f, 0.5f, -0.5f);
        glm::vec3 p3(-0.5f, 0.5f, -0.5f);

        glm::vec3 p4(-0.5f, -0.5f, 0.5f); // Front face
        glm::vec3 p5(0.5f, -0.5f, 0.5f);
        glm::vec3 p6(0.5f, 0.5f, 0.5f);
        glm::vec3 p7(-0.5f, 0.5f, 0.5f);

        // define a helper to create a quad face from 4 corners and a normal
        auto add_face = [&](const glm::vec3& a,
                            const glm::vec3& b,
                            const glm::vec3& c,
                            const glm::vec3& d,
                            const glm::vec3& normal) {
            // texture coordinates (just mapped from 0 to 1)
            glm::vec3 uv0(0.0f, 0.0f, 0.0f);
            glm::vec3 uv1(1.0f, 0.0f, 0.0f);
            glm::vec3 uv2(1.0f, 1.0f, 0.0f);
            glm::vec3 uv3(0.0f, 1.0f, 0.0f);

            if (winding_order == CCW) {
                vertices.emplace_back(a, glm::vec4(1.0f), uv0, glm::vec4(normal, 0.0f));
                vertices.emplace_back(c, glm::vec4(1.0f), uv2, glm::vec4(normal, 0.0f));
                vertices.emplace_back(b, glm::vec4(1.0f), uv1, glm::vec4(normal, 0.0f));

                vertices.emplace_back(a, glm::vec4(1.0f), uv0, glm::vec4(normal, 0.0f));
                vertices.emplace_back(d, glm::vec4(1.0f), uv3, glm::vec4(normal, 0.0f));
                vertices.emplace_back(c, glm::vec4(1.0f), uv2, glm::vec4(normal, 0.0f));
            } else {
                vertices.emplace_back(a, glm::vec4(1.0f), uv0, glm::vec4(normal, 0.0f));
                vertices.emplace_back(b, glm::vec4(1.0f), uv1, glm::vec4(normal, 0.0f));
                vertices.emplace_back(c, glm::vec4(1.0f), uv2, glm::vec4(normal, 0.0f));

                vertices.emplace_back(c, glm::vec4(1.0f), uv2, glm::vec4(normal, 0.0f));
                vertices.emplace_back(d, glm::vec4(1.0f), uv3, glm::vec4(normal, 0.0f));
                vertices.emplace_back(a, glm::vec4(1.0f), uv0, glm::vec4(normal, 0.0f));
            }
        };

        // faces
        add_face(p0, p1, p2, p3, glm::vec3(0.0f, 0.0f, -1.0f)); // Back
        add_face(p5, p4, p7, p6, glm::vec3(0.0f, 0.0f, 1.0f));  // Front
        add_face(p4, p0, p3, p7, glm::vec3(-1.0f, 0.0f, 0.0f)); // Left
        add_face(p1, p5, p6, p2, glm::vec3(1.0f, 0.0f, 0.0f));  // Right
        add_face(p4, p5, p1, p0, glm::vec3(0.0f, -1.0f, 0.0f)); // Bottom
        add_face(p3, p2, p6, p7, glm::vec3(0.0f, 1.0f, 0.0f));  // Top
    }

    inline void generate_sphere(std::vector<Vertex>& vertices, const int stacks, const int slices) {
        constexpr float radius = 1.0f;
        for (int i = 0; i < stacks; ++i) {
            const float theta1 = glm::pi<float>() * (static_cast<float>(i) / stacks);
            const float theta2 = glm::pi<float>() * (static_cast<float>(i + 1) / stacks);

            for (int j = 0; j < slices; ++j) {
                const float phi1 = 2.0f * glm::pi<float>() * (static_cast<float>(j) / slices);
                const float phi2 = 2.0f * glm::pi<float>() * (static_cast<float>(j + 1) / slices);

                auto compute_vertex = [&](const float theta, const float phi) -> Vertex {
                    const glm::vec3 pos = {
                        radius * sin(theta) * cos(phi),
                        radius * cos(theta),
                        radius * sin(theta) * sin(phi)};
                    const glm::vec3 normal = glm::normalize(pos);
                    const glm::vec3 tex    = {
                        phi / (2.0f * glm::pi<float>()), // u
                        theta / glm::pi<float>(),        // v
                        0.0f                             // padding / unused
                    };
                    return Vertex{
                        glm::vec4(pos, 1.0f),
                        Vertex::DEFAULT_COLOR, // white
                        tex,
                        glm::vec4(normal, 0.0f),
                    };
                };

                Vertex v0 = compute_vertex(theta1, phi1);
                Vertex v1 = compute_vertex(theta2, phi1);
                Vertex v2 = compute_vertex(theta2, phi2);
                Vertex v3 = compute_vertex(theta1, phi2);

                if (winding_order == CCW) {
                    // first triangle (counter-clockwise)
                    vertices.push_back(v0);
                    vertices.push_back(v2);
                    vertices.push_back(v1);

                    // second triangle (counter-clockwise)
                    vertices.push_back(v0);
                    vertices.push_back(v3);
                    vertices.push_back(v2);
                } else {
                    // first triangle
                    vertices.push_back(v0);
                    vertices.push_back(v1);
                    vertices.push_back(v2);

                    // second triangle
                    vertices.push_back(v2);
                    vertices.push_back(v3);
                    vertices.push_back(v0);
                }
            }
        }
    }

    inline std::vector<Vertex> generate_tube_mesh(
        const std::vector<Vertex>& points,
        float                      radius = 0.05f,
        bool                       closed = false,
        const glm::aligned_vec4&   color  = glm::aligned_vec4(1.0f)) {

        std::vector<Vertex> tubeVertices;
        size_t              n = points.size();
        if (n < 2) {
            return tubeVertices;
        }

        constexpr int                          sides = 4;
        std::vector<std::array<Vertex, sides>> ringVertices;

        glm::vec3 referenceUp(0, 1, 0);
        if (glm::length(glm::cross(referenceUp, glm::normalize(glm::vec3(points[1].position) - glm::vec3(points[0].position)))) < 0.1f) {
            referenceUp = glm::vec3(1, 0, 0);
        }

        glm::vec3 prevTangent;
        glm::vec3 normal, binormal;

        for (size_t i = 0; i < n; ++i) {
            glm::vec3 pi = glm::vec3(points[i].position);
            glm::vec3 tangent;

            if (i == 0) {
                tangent = closed
                              ? glm::normalize(glm::vec3(points[1].position) - glm::vec3(points[n - 1].position))
                              : glm::normalize(glm::vec3(points[1].position) - pi);
            } else if (i == n - 1) {
                tangent = closed
                              ? glm::normalize(glm::vec3(points[0].position) - glm::vec3(points[n - 2].position))
                              : glm::normalize(pi - glm::vec3(points[n - 2].position));
            } else {
                tangent = glm::normalize(glm::vec3(points[i + 1].position) - glm::vec3(points[i - 1].position));
            }

            if (i == 0) {
                normal   = glm::normalize(glm::cross(tangent, referenceUp));
                binormal = glm::normalize(glm::cross(tangent, normal));
            } else {
                glm::vec3 rotationAxis = glm::cross(prevTangent, tangent);
                float     angle        = glm::asin(glm::length(rotationAxis));

                if (angle > 1e-4f) {
                    rotationAxis = glm::normalize(rotationAxis);
                    glm::mat4 R  = glm::rotate(glm::mat4(1.0f), angle, rotationAxis);
                    normal       = glm::normalize(glm::vec3(R * glm::vec4(normal, 0.0f)));
                    binormal     = glm::normalize(glm::cross(tangent, normal));
                } else {
                    binormal = glm::normalize(glm::cross(tangent, normal));
                }
            }

            prevTangent = tangent;

            std::array<Vertex, sides> ring;
            for (int j = 0; j < sides; ++j) {
                float     angle = glm::radians(90.0f * j);
                glm::vec3 dir   = glm::cos(angle) * normal + glm::sin(angle) * binormal;
                glm::vec3 pos   = pi + dir * radius;

                ring[j] = Vertex{pos,
                                 color,
                                 glm::vec3(j / static_cast<float>(sides), i / static_cast<float>(n), 0.0f),
                                 glm::aligned_vec4(glm::normalize(dir), 0.0f)};
            }

            ringVertices.push_back(ring);
        }

        if (closed) {
            ringVertices.push_back(ringVertices[0]);
        }

        size_t ringCount = ringVertices.size();
        for (size_t i = 0; i < ringCount - 1; ++i) {
            for (int j = 0; j < sides; ++j) {
                int j0 = j;
                int j1 = (j + 1) % sides;

                const Vertex& a = ringVertices[i][j0];
                const Vertex& b = ringVertices[i][j1];
                const Vertex& c = ringVertices[i + 1][j0];
                const Vertex& d = ringVertices[i + 1][j1];

                // triangle 1
                tubeVertices.push_back(a);
                tubeVertices.push_back(c);
                tubeVertices.push_back(b);

                // triangle 2
                tubeVertices.push_back(b);
                tubeVertices.push_back(c);
                tubeVertices.push_back(d);
            }
        }

        return tubeVertices;
    }

    inline std::vector<glm::vec3> generate_tube_mesh(const std::vector<glm::vec3>& points,
                                                     float                         radius = 0.05f,
                                                     bool                          closed = false) {
        std::vector<glm::vec3> tubeVertices;
        size_t                 n = points.size();
        if (n < 2) {
            return tubeVertices;
        }

        constexpr int                             sides = 4;
        std::vector<std::array<glm::vec3, sides>> ringVertices;

        // choose a stable initial reference up vector
        glm::vec3 referenceUp(0, 1, 0);
        if (glm::length(glm::cross(referenceUp, glm::normalize(points[1] - points[0]))) < 0.1f) {
            referenceUp = glm::vec3(1, 0, 0); // fallback if colinear
        }

        glm::vec3 prevTangent;
        glm::vec3 normal, binormal;

        for (size_t i = 0; i < n; ++i) {
            glm::vec3 tangent;
            if (i == 0) {
                tangent = closed
                              ? glm::normalize(points[1] - points[n - 1])
                              : glm::normalize(points[1] - points[0]);
            } else if (i == n - 1) {
                tangent = closed
                              ? glm::normalize(points[0] - points[n - 2])
                              : glm::normalize(points[n - 1] - points[n - 2]);
            } else {
                tangent = glm::normalize(points[i + 1] - points[i - 1]);
            }

            if (i == 0) {
                normal   = glm::normalize(glm::cross(tangent, referenceUp));
                binormal = glm::normalize(glm::cross(tangent, normal));
            } else {
                glm::vec3 rotationAxis = glm::cross(prevTangent, tangent);
                float     angle        = glm::asin(glm::length(rotationAxis));

                if (angle > 1e-4f) {
                    rotationAxis = glm::normalize(rotationAxis);
                    glm::mat4 R  = glm::rotate(glm::mat4(1.0f), angle, rotationAxis);
                    normal       = glm::normalize(glm::vec3(R * glm::vec4(normal, 0.0)));
                    binormal     = glm::normalize(glm::cross(tangent, normal));
                } else {
                    // parallel: just keep previous frame
                    binormal = glm::normalize(glm::cross(tangent, normal));
                }
            }

            prevTangent = tangent;

            // build cross-section ring
            std::array<glm::vec3, sides> ring;
            for (int j = 0; j < sides; ++j) {
                float     angle = glm::radians(90.0f * j);
                glm::vec3 dir   = glm::cos(angle) * normal + glm::sin(angle) * binormal;
                ring[j]         = points[i] + dir * radius;
            }

            ringVertices.push_back(ring);
        }

        // ensure seamless ring if closed
        if (closed) {
            ringVertices.push_back(ringVertices[0]);
        }

        size_t ringCount = ringVertices.size();
        for (size_t i = 0; i < ringCount - 1; ++i) {
            for (int j = 0; j < sides; ++j) {
                int j0 = j;
                int j1 = (j + 1) % sides;

                glm::vec3 a = ringVertices[i][j0];
                glm::vec3 b = ringVertices[i][j1];
                glm::vec3 c = ringVertices[i + 1][j0];
                glm::vec3 d = ringVertices[i + 1][j1];

                // triangle 1
                tubeVertices.push_back(a);
                tubeVertices.push_back(c);
                tubeVertices.push_back(b);

                // triangle 2
                tubeVertices.push_back(b);
                tubeVertices.push_back(c);
                tubeVertices.push_back(d);
            }
        }
        return tubeVertices;
    }
} // namespace umfeld
