#include <iostream>
#include <GL/glew.h>
#include <vector>

#include "PGraphicsOpenGLv33.h"

#include <Umgebung.h>

using namespace umgebung;

PGraphicsOpenGLv33::PGraphicsOpenGLv33() : PImage(0, 0, 0), currentMatrix(), aspectRatio(0) {}

void PGraphicsOpenGLv33::strokeWeight(const float weight) {
    fStrokeWeight = weight;
}

void PGraphicsOpenGLv33::background(const float a, const float b, const float c, const float d) {
    glClearColor(a, b, c, d);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PGraphicsOpenGLv33::background(const float a) {
    background(a, a, a);
}

void PGraphicsOpenGLv33::rect(const float x, const float y, const float width, const float height) {
    if (current_stroke_color.active) {
        add_stroke_vertex_xyz_rgba(x, y, 0, current_stroke_color.r, current_stroke_color.g, current_stroke_color.b, current_stroke_color.a);
        add_stroke_vertex_xyz_rgba(x + width, y, 0, current_stroke_color.r, current_stroke_color.g, current_stroke_color.b, current_stroke_color.a);

        add_stroke_vertex_xyz_rgba(x + width, y, 0, current_stroke_color.r, current_stroke_color.g, current_stroke_color.b, current_stroke_color.a);
        add_stroke_vertex_xyz_rgba(x + width, y + height, 0, current_stroke_color.r, current_stroke_color.g, current_stroke_color.b, current_stroke_color.a);

        add_stroke_vertex_xyz_rgba(x + width, y + height, 0, current_stroke_color.r, current_stroke_color.g, current_stroke_color.b, current_stroke_color.a);
        add_stroke_vertex_xyz_rgba(x, y + height, 0, current_stroke_color.r, current_stroke_color.g, current_stroke_color.b, current_stroke_color.a);

        add_stroke_vertex_xyz_rgba(x, y + height, 0, current_stroke_color.r, current_stroke_color.g, current_stroke_color.b, current_stroke_color.a);
        add_stroke_vertex_xyz_rgba(x, y, 0, current_stroke_color.r, current_stroke_color.g, current_stroke_color.b, current_stroke_color.a);
    }
    if (current_fill_color.active) {
        add_fill_vertex_xyz_rgba_uv(x, y, 0, current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a);
        add_fill_vertex_xyz_rgba_uv(x + width, y, 0, current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a);
        add_fill_vertex_xyz_rgba_uv(x + width, y + height, 0, current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a);

        add_fill_vertex_xyz_rgba_uv(x + width, y + height, 0, current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a);
        add_fill_vertex_xyz_rgba_uv(x, y + height, 0, current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a);
        add_fill_vertex_xyz_rgba_uv(x, y, 0, current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a);

        constexpr int       RECT_NUM_VERTICES               = 6;
        const unsigned long fill_vertices_count_xyz_rgba_uv = fill_vertices_xyz_rgba_uv.size() / NUM_FILL_VERTEX_ATTRIBUTES_XYZ_RGBA_UV;
        if (renderBatches.empty() || renderBatches.back().texture_id != solid_color_texture_id) {
            renderBatches.emplace_back(fill_vertices_count_xyz_rgba_uv - RECT_NUM_VERTICES, RECT_NUM_VERTICES, solid_color_texture_id);
        } else {
            renderBatches.back().num_vertices += RECT_NUM_VERTICES;
        }
    }
}

void PGraphicsOpenGLv33::draw_filled_ellipse(const float x, const float y,
                                             const float      width,
                                             const float      height,
                                             const int        detail,
                                             const glm::vec4& color) {
    const float            radiusX = width / 2.0f;
    const float            radiusY = height / 2.0f;
    std::vector<glm::vec3> points;
    for (int i = 0; i <= detail; ++i) {
        const float     theta = 2.0f * 3.1415926f * static_cast<float>(i) / static_cast<float>(detail);
        const glm::vec3 p{x + radiusX * cosf(theta),
                          y + radiusY * sinf(theta), 0};
        points.push_back(p);
    }
    const glm::vec3     center{x, y, 0};
    constexpr glm::vec2 tex_coords{0, 0};
    for (int i = 0; i < points.size(); ++i) {
        const glm::vec3 p1 = points[i];
        const glm::vec3 p2 = points[(i + 1) % points.size()];
        add_fill_vertex_xyz_rgba_uv(center, color, tex_coords);
        add_fill_vertex_xyz_rgba_uv(p1, color, tex_coords);
        add_fill_vertex_xyz_rgba_uv(p2, color, tex_coords);
    }
    add_texture_id_to_render_batch(fill_vertices_xyz_rgba_uv, points.size() * 3, solid_color_texture_id);
}


void PGraphicsOpenGLv33::ellipse(const float x, const float y, const float width, const float height) {
    if (current_stroke_color.active) {
        const float            radiusX = width / 2.0f;
        const float            radiusY = height / 2.0f;
        std::vector<glm::vec3> points;
        for (int i = 0; i <= fEllipseDetail; ++i) {
            const float     theta = 2.0f * 3.1415926f * static_cast<float>(i) / static_cast<float>(fEllipseDetail);
            const glm::vec3 p{x + radiusX * cosf(theta),
                              y + radiusY * sinf(theta), 0};
            points.push_back(p);
        }
        const glm::vec4& color{current_stroke_color.r, current_stroke_color.g, current_stroke_color.b, current_stroke_color.a};
        render_line_strip_as_lines(points, color, true, false);
    }
    if (current_fill_color.active) {
        const glm::vec4& color{current_fill_color.r,
                               current_fill_color.g,
                               current_fill_color.b,
                               current_fill_color.a};
        draw_filled_ellipse(x, y, width, height, fEllipseDetail, color);
    }
}

void PGraphicsOpenGLv33::circle(float x, float y, float diameter) {
    ellipse(x, y, diameter, diameter);
}

void PGraphicsOpenGLv33::ellipseDetail(const int detail) {
    fEllipseDetail = detail;
}

void PGraphicsOpenGLv33::add_texture_id_to_render_batch(const std::vector<float>& vertices,
                                                        const int                 num_vertices,
                                                        const GLuint              texture_id) {
    const unsigned long fill_vertices_count_xyz_rgba_uv = vertices.size() / NUM_FILL_VERTEX_ATTRIBUTES_XYZ_RGBA_UV;
    if (renderBatches.empty() || renderBatches.back().texture_id != texture_id) {
        renderBatches.emplace_back(fill_vertices_count_xyz_rgba_uv - num_vertices, num_vertices, texture_id);
    } else {
        renderBatches.back().num_vertices += num_vertices;
    }
}

// TODO decide: either render as lines or as quads

void PGraphicsOpenGLv33::line(const float x1, const float y1, const float x2, const float y2) {
    if (!current_stroke_color.active) {
        return;
    }
    line(x1, y1, 0, x2, y2, 0);
}

static bool intersect_lines(const glm::vec2& p1, const glm::vec2& d1,
                            const glm::vec2& p2, const glm::vec2& d2,
                            glm::vec3& intersection) {
    const float det = d1.x * d2.y - d1.y * d2.x;

    if (fabs(det) < 1e-6f) {
        return false; // Parallel or coincident lines
    }

    const float t = ((p2.x - p1.x) * d2.y - (p2.y - p1.y) * d2.x) / det;

    intersection = glm::vec3(p1 + t * d1, 0);
    return true;
}

static bool are_almost_parallel(const glm::vec3& n1, const glm::vec3& n2, const float epsilon = 0.01f) {
    const float dotProduct = glm::dot(n1, n2);
    return -dotProduct > (1.0f - epsilon); // Closer to 1 or -1 means nearly parallel
    // return fabs(dotProduct) > (1.0f - epsilon); // Closer to 1 or -1 means nearly parallel
}

void PGraphicsOpenGLv33::render_line_strip_as_lines(const std::vector<glm::vec3>& points,
                                                    const glm::vec4&              color,
                                                    const bool                    close_shape,
                                                    const bool                    round_corners) {
    if (points.size() < 2) {
        return;
    }

    if (points.size() == 2) {
        // TODO replace with raw version
        line(points[0].x, points[0].y, points[0].z,
             points[1].x, points[1].y, points[1].z);
        return;
    }

    const uint32_t num_line_segments = points.size() - (close_shape ? 0 : 1);
    for (size_t i = 0; i < num_line_segments; i++) {
        const size_t ii = (i + 1) % points.size();
        line(points[i].x, points[i].y, points[i].z,
             points[ii].x, points[ii].y, points[ii].z);
        if (round_corners) {
            // TODO must be transformed to screenspace!
            draw_filled_ellipse(points[i].x, points[i].y, fStrokeWeight, fStrokeWeight, 8, color);
        }
    }
}

void PGraphicsOpenGLv33::render_line_strip(std::vector<glm::vec3>& points, const glm::vec4& color, const bool close_shape) {
    if (points.size() < 2) {
        return;
    }

    if (points.size() == 2) {
        line(points[0].x, points[0].y, points[0].z,
             points[1].x, points[1].y, points[1].z);
        return;
    }

    // Transform all points to screen space first
    std::vector<glm::vec3> screen_points = points;
    std::vector<glm::vec3> normals(screen_points.size());
    std::vector<glm::vec3> directions(screen_points.size());

    for (auto& p: screen_points) {
        to_screen_space(p);
    }

    for (size_t i = 0; i < normals.size(); i++) {
        const size_t ii   = (i + 1) % screen_points.size();
        glm::vec3&   p1   = screen_points[i];
        glm::vec3&   p2   = screen_points[ii];
        glm::vec3    dir  = p2 - p1;
        glm::vec3    perp = glm::normalize(dir);
        perp              = glm::vec3(-perp.y, perp.x, 0);
        directions[i]     = dir;
        normals[i]        = perp;
    }

    const float half_width = fStrokeWeight * 0.5f;
    uint32_t    vertex_count{0};

    glm::vec3 p1_left{};
    glm::vec3 p1_right{};

    const uint32_t num_line_segments = screen_points.size() + (close_shape ? 1 : 0);

    for (size_t i = 0; i < num_line_segments; i++) {
        const glm::vec3 point     = screen_points[i % screen_points.size()];
        const glm::vec3 direction = directions[i % directions.size()];
        const glm::vec3 normal    = normals[i % normals.size()];

        const glm::vec3 next_point     = screen_points[(i + 1) % screen_points.size()];
        const glm::vec3 next_direction = directions[(i + 1) % directions.size()];
        const glm::vec3 next_normal    = normals[(i + 1) % normals.size()];

        const glm::vec3 p2_left  = point + normal * half_width;
        const glm::vec3 p2_right = point - normal * half_width;
        const glm::vec3 p3_left  = next_point + next_normal * half_width;
        const glm::vec3 p3_right = next_point - next_normal * half_width;

        glm::vec3  intersection_left;
        const bool result_left = intersect_lines(p2_left, direction,
                                                 p3_left, next_direction,
                                                 intersection_left);
        glm::vec3  intersection_right;
        const bool result_right = intersect_lines(p2_right, direction,
                                                  p3_right, next_direction,
                                                  intersection_right);

        if (are_almost_parallel(normal, next_normal, 0.2f)) {
            // TODO maybe handle sharp angles
        }

        if (!result_left) {
            intersection_left = p3_left;
        }
        if (!result_right) {
            intersection_right = p3_right;
        }

        if (close_shape) {
            if (i != 0) {
                add_transformed_fill_vertex_xyz_rgba_uv(p1_left, color);
                add_transformed_fill_vertex_xyz_rgba_uv(intersection_left, color);
                add_transformed_fill_vertex_xyz_rgba_uv(p1_right, color);

                add_transformed_fill_vertex_xyz_rgba_uv(p1_right, color);
                add_transformed_fill_vertex_xyz_rgba_uv(intersection_left, color);
                add_transformed_fill_vertex_xyz_rgba_uv(intersection_right, color);
            }
            vertex_count += 6;
        } else {
            if (i == 0) { // first segment
                add_transformed_fill_vertex_xyz_rgba_uv(p2_left, color);
                add_transformed_fill_vertex_xyz_rgba_uv(intersection_left, color);
                add_transformed_fill_vertex_xyz_rgba_uv(p2_right, color);

                add_transformed_fill_vertex_xyz_rgba_uv(p2_right, color);
                add_transformed_fill_vertex_xyz_rgba_uv(intersection_left, color);
                add_transformed_fill_vertex_xyz_rgba_uv(intersection_right, color);
            } else if (i == num_line_segments - 2) { // last segment
                add_transformed_fill_vertex_xyz_rgba_uv(p1_left, color);
                add_transformed_fill_vertex_xyz_rgba_uv(next_point + normal * half_width, color);
                add_transformed_fill_vertex_xyz_rgba_uv(p1_right, color);

                add_transformed_fill_vertex_xyz_rgba_uv(p1_right, color);
                add_transformed_fill_vertex_xyz_rgba_uv(next_point + normal * half_width, color);
                add_transformed_fill_vertex_xyz_rgba_uv(next_point - normal * half_width, color);
            } else if (i < num_line_segments - 2) {
                add_transformed_fill_vertex_xyz_rgba_uv(p1_left, color);
                add_transformed_fill_vertex_xyz_rgba_uv(intersection_left, color);
                add_transformed_fill_vertex_xyz_rgba_uv(p1_right, color);

                add_transformed_fill_vertex_xyz_rgba_uv(p1_right, color);
                add_transformed_fill_vertex_xyz_rgba_uv(intersection_left, color);
                add_transformed_fill_vertex_xyz_rgba_uv(intersection_right, color);
            }
            vertex_count += 6;
            // TODO this could be used for round caps
        }
        p1_left  = intersection_left;
        p1_right = intersection_right;
    }

    add_texture_id_to_render_batch(fill_vertices_xyz_rgba_uv, vertex_count, solid_color_texture_id);
}

void PGraphicsOpenGLv33::line(const float x1, const float y1, const float z1,
                              const float x2, const float y2, const float z2) {
    if (!current_stroke_color.active) {
        return;
    }
    if (render_lines_as_quads) {
        auto p1 = glm::vec3(x1, y1, z1);
        auto p2 = glm::vec3(x2, y2, z2);

        to_screen_space(p1);
        to_screen_space(p2);

        glm::vec3 perp = p2 - p1;
        perp           = glm::normalize(perp);
        perp           = {-perp.y, perp.x, 0};
        perp *= fStrokeWeight * 0.5f;

        const glm::vec4 color{current_stroke_color.r,
                              current_stroke_color.g,
                              current_stroke_color.b,
                              current_stroke_color.a};

        add_transformed_fill_vertex_xyz_rgba_uv(p1 + perp, color);
        add_transformed_fill_vertex_xyz_rgba_uv(p2 + perp, color);
        add_transformed_fill_vertex_xyz_rgba_uv(p1 - perp, color);
        add_transformed_fill_vertex_xyz_rgba_uv(p1 - perp, color);
        add_transformed_fill_vertex_xyz_rgba_uv(p2 + perp, color);
        add_transformed_fill_vertex_xyz_rgba_uv(p2 - perp, color);
        add_texture_id_to_render_batch(fill_vertices_xyz_rgba_uv, 6, solid_color_texture_id);
    } else {
        add_stroke_vertex_xyz_rgba(x1, y1, z1, current_stroke_color.r, current_stroke_color.g, current_stroke_color.b, current_stroke_color.a);
        add_stroke_vertex_xyz_rgba(x2, y2, z2, current_stroke_color.r, current_stroke_color.g, current_stroke_color.b, current_stroke_color.a);
    }
}

void PGraphicsOpenGLv33::linse(const float x1, const float y1, const float x2, const float y2) {
    line(x1, y1, x2, y2);
}

void PGraphicsOpenGLv33::bezier(const float x1, const float y1,
                                const float x2, const float y2,
                                const float x3, const float y3,
                                const float x4, const float y4) {
    if (!current_stroke_color.active) {
        return;
    }
    if (fBezierDetail < 2) {
        return;
    }

    const int   segments = fBezierDetail;
    const float step     = 1.0f / static_cast<float>(segments);

    float x_prev = 0;
    float y_prev = 0;
    for (int i = 0; i <= segments; ++i) {
        const float t = static_cast<float>(i) * step;
        const float u = 1.0f - t;

        const float b0 = u * u * u;
        const float b1 = 3 * u * u * t;
        const float b2 = 3 * u * t * t;
        const float b3 = t * t * t;

        const float x = b0 * x1 + b1 * x2 + b2 * x3 + b3 * x4;
        const float y = b0 * y1 + b1 * y2 + b2 * y3 + b3 * y4;

        if (i > 0) {
            line(x, y, x_prev, y_prev);
        }
        x_prev = x;
        y_prev = y;
    }
}

void PGraphicsOpenGLv33::bezier(const float x1, const float y1, const float z1,
                                const float x2, const float y2, const float z2,
                                const float x3, const float y3, const float z3,
                                const float x4, const float y4, const float z4) {
    if (!current_stroke_color.active) {
        return;
    }
    if (fBezierDetail < 2) {
        return;
    }

    const int   segments = fBezierDetail;
    const float step     = 1.0f / static_cast<float>(segments);

    float x_prev = 0;
    float y_prev = 0;
    float z_prev = 0;
    // TODO replace with begin-end-shape:
    // beginShape(LINES);
    for (int i = 0; i <= segments; ++i) {
        const float t = static_cast<float>(i) * step;
        const float u = 1.0f - t;

        const float b0 = u * u * u;
        const float b1 = 3 * u * u * t;
        const float b2 = 3 * u * t * t;
        const float b3 = t * t * t;

        const float x = b0 * x1 + b1 * x2 + b2 * x3 + b3 * x4;
        const float y = b0 * y1 + b1 * y2 + b2 * y3 + b3 * y4;
        const float z = b0 * z1 + b1 * z2 + b2 * z3 + b3 * z4;

        if (i > 0) {
            // vertex(x, y,z);
            // vertex(x_prev, y_prev, z_prev);
            line(x, y, z, x_prev, y_prev, z_prev);
        }
        x_prev = x;
        y_prev = y;
        z_prev = z;
    }
    // endShape();
}

void PGraphicsOpenGLv33::bezierDetail(const int detail) {
    fBezierDetail = detail;
}

void PGraphicsOpenGLv33::pointSize(float point_size) {}
void PGraphicsOpenGLv33::point(float x, float y, float z) {}

void PGraphicsOpenGLv33::beginShape(int shape) {
    line_strip_point_cache.clear();
}

void PGraphicsOpenGLv33::endShape(const bool close_shape) {
    const glm::vec4 color = {current_stroke_color.r,
                             current_stroke_color.g,
                             current_stroke_color.b,
                             current_stroke_color.a};
    // render_line_strip(line_strip_point_cache, color, close_shape);
    render_line_strip_as_lines(line_strip_point_cache, color, close_shape, true);

    line_strip_point_cache.clear();
    vertex_cache.clear();
}

void PGraphicsOpenGLv33::vertex(const float x, const float y, const float z) {
    vertex(x, y, z, 0, 0);
}

void PGraphicsOpenGLv33::vertex(const float x, const float y, const float z, const float u, const float v) {
    line_strip_point_cache.push_back(glm::vec3(x, y, z));
    const Vertex vertex_info{
        glm::vec3(x, y, z),
        glm::vec4(current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a),
        glm::vec2(u, v)};
    vertex_cache.push_back(vertex_info);
}

PFont* PGraphicsOpenGLv33::loadFont(const std::string& file, float size) { return nullptr; }
void   PGraphicsOpenGLv33::textFont(PFont* font) {}
void   PGraphicsOpenGLv33::textSize(float size) {}
void   PGraphicsOpenGLv33::text(const char* value, float x, float y, float z) {}
float  PGraphicsOpenGLv33::textWidth(const std::string& text) { return 0.0f; }

PImage* PGraphicsOpenGLv33::loadImage(const std::string& filename) {
    auto* img = new PImage(filename);
    return img;
}

void PGraphicsOpenGLv33::image(PImage* img, const float x, const float y, float w, float h) {
    if (!current_fill_color.active) {
        return;
    }
    if (img == nullptr) {
        return;
    }
    if (img->texture_id == -1) {
        std::cout << "PImage has not been uploaded. Trying to upload image as texture." << std::endl;
        upload_texture(img, true);
    }

    if (w == -1) {
        w = static_cast<float>(img->width);
    }
    if (h == -1) {
        h = static_cast<float>(img->height);
    }
    add_fill_vertex_xyz_rgba_uv(x, y, 0, current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a, 0.0f, 0.0f);
    add_fill_vertex_xyz_rgba_uv(x + w, y, 0, current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a, 1.0f, 0.0f);
    add_fill_vertex_xyz_rgba_uv(x + w, y + h, 0, current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a, 1.0f, 1.0f);

    add_fill_vertex_xyz_rgba_uv(x + w, y + h, 0, current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a, 1.0f, 1.0f);
    add_fill_vertex_xyz_rgba_uv(x, y + h, 0, current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a, 0.0f, 1.0f);
    add_fill_vertex_xyz_rgba_uv(x, y, 0, current_fill_color.r, current_fill_color.g, current_fill_color.b, current_fill_color.a, 0.0f, 0.0f);

    constexpr int       RECT_NUM_VERTICES               = 6;
    const unsigned long fill_vertices_count_xyz_rgba_uv = fill_vertices_xyz_rgba_uv.size() / NUM_FILL_VERTEX_ATTRIBUTES_XYZ_RGBA_UV;
    if (renderBatches.empty() || renderBatches.back().texture_id != img->texture_id) {
        renderBatches.emplace_back(fill_vertices_count_xyz_rgba_uv - RECT_NUM_VERTICES, RECT_NUM_VERTICES, img->texture_id);
    } else {
        renderBatches.back().num_vertices += RECT_NUM_VERTICES;
    }
}

void PGraphicsOpenGLv33::image(PImage* img, const float x, const float y) {
    if (img == nullptr) {
        return;
    }
    image(img, x, y, img->width, img->height);
}

void PGraphicsOpenGLv33::texture(PImage* img) {}

void PGraphicsOpenGLv33::popMatrix() {
    if (!matrixStack.empty()) {
        currentMatrix = matrixStack.back();
        matrixStack.pop_back();
    }
}

void PGraphicsOpenGLv33::pushMatrix() {
    matrixStack.push_back(currentMatrix);
}

void PGraphicsOpenGLv33::translate(const float x, const float y, const float z) {
    currentMatrix = glm::translate(currentMatrix, glm::vec3(x, y, z));
}

void PGraphicsOpenGLv33::rotateX(const float angle) {
    currentMatrix = glm::rotate(currentMatrix, angle, glm::vec3(1.0f, 0.0f, 0.0f));
}

void PGraphicsOpenGLv33::rotateY(const float angle) {
    currentMatrix = glm::rotate(currentMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
}

void PGraphicsOpenGLv33::rotateZ(const float angle) {
    currentMatrix = glm::rotate(currentMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
}

void PGraphicsOpenGLv33::rotate(const float angle) {
    currentMatrix = glm::rotate(currentMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
}

void PGraphicsOpenGLv33::rotate(const float angle, const float x, const float y, const float z) {
    currentMatrix = glm::rotate(currentMatrix, angle, glm::vec3(x, y, z));
}

void PGraphicsOpenGLv33::scale(const float x) {
    currentMatrix = glm::scale(currentMatrix, glm::vec3(x, x, x));
}

void PGraphicsOpenGLv33::scale(const float x, const float y) {
    currentMatrix = glm::scale(currentMatrix, glm::vec3(x, y, 1));
}

void PGraphicsOpenGLv33::scale(const float x, const float y, const float z) {
    currentMatrix = glm::scale(currentMatrix, glm::vec3(x, y, z));
}

void PGraphicsOpenGLv33::pixelDensity(int density) {}
void PGraphicsOpenGLv33::hint(uint16_t property) {}
void PGraphicsOpenGLv33::text_str(const std::string& text, float x, float y, float z) {}

void PGraphicsOpenGLv33::beginDraw() {
#ifdef PGRAPHICS_RENDER_INTO_FRAMEBUFFER
    /* save state */
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fPreviousFBO);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();

    // bind the FBO for offscreen rendering
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
    glViewport(0, 0, framebuffer.width, framebuffer.height);

    /* setup projection and modelview matrices */

    glMatrixMode(GL_PROJECTION);
    // save the current projection matrix
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, framebuffer.width, 0, framebuffer.height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    // save the current modelview matrix
    glPushMatrix();
    glLoadIdentity();
#endif // PGRAPHICS_RENDER_INTO_FRAMEBUFFER
}

void PGraphicsOpenGLv33::endDraw() {
#ifdef PGRAPHICS_RENDER_INTO_FRAMEBUFFER
    // restore projection and modelview matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    /* restore state */
    glBindFramebuffer(GL_FRAMEBUFFER, fPreviousFBO); // Restore the previously bound FBO
    glPopMatrix();
    glPopAttrib();
#endif // PGRAPHICS_RENDER_INTO_FRAMEBUFFER
}

void PGraphicsOpenGLv33::bind() {
    if (texture_id) {
        glBindTexture(GL_TEXTURE_2D, texture_id);
    }
}

void PGraphicsOpenGLv33::reset_matrices() {
    currentMatrix = glm::mat4(1.0f);

    glViewport(0, 0, width, height);

    aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    // Orthographic projection
    projection2D = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f);

    const float fov            = DEFAULT_FOV;                                           // distance from the camera = screen height
    const float cameraDistance = (static_cast<float>(height) / 2.0f) / tan(fov / 2.0f); // 1 unit = 1 pixel

    // Perspective projection
    projection3D = glm::perspective(fov, static_cast<float>(width) / static_cast<float>(height), 0.1f, 10000.0f);

    viewMatrix = glm::lookAt(
        glm::vec3(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f, -cameraDistance), // Flip Z to fix X-axis
        glm::vec3(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f, 0.0f),            // Look at the center
        glm::vec3(0.0f, -1.0f, 0.0f)                                                                     // Keep Y-up as normal
    );
}

void PGraphicsOpenGLv33::init(uint32_t* pixels,
                              const int width,
                              const int height,
                              int       format,
                              bool      generate_mipmap) {
    this->width        = width;
    this->height       = height;
    framebuffer.width  = width;
    framebuffer.height = height;

    stroke_shader_program = build_shader(vertex_shader_source_simple(), fragment_shader_source_simple());
    init_stroke_vertice_buffers();
    fill_shader_program = build_shader(vertex_shader_source_texture(), fragment_shader_source_texture());
    init_fill_vertice_buffers();
    createDummyTexture();

    reset_matrices();

#ifdef PGRAPHICS_RENDER_INTO_FRAMEBUFFER
    glGenFramebuffers(1, &framebuffer.id);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
    glGenTextures(1, &framebuffer.texture);
    glBindTexture(GL_TEXTURE_2D, framebuffer.texture);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 framebuffer.width,
                 framebuffer.height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        // Handle framebuffer incomplete error
        std::cerr << "ERROR Framebuffer is not complete!" << std::endl;
    }
    glViewport(0, 0, framebuffer.width, framebuffer.height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif // PGRAPHICS_RENDER_INTO_FRAMEBUFFER
}

/* additional */

bool PGraphicsOpenGLv33::upload_texture(PImage* image, const bool generate_texture_mipmapped) {
    if (image->pixels == nullptr) {
        std::cerr << "Failed to upload texture. Pixels are null" << std::endl;
        return false;
    }

    GLuint mTextureID;
    glGenTextures(1, &mTextureID);

    if (mTextureID == 0) {
        std::cerr << "Failed to generate texture ID" << std::endl;
        return false;
    }

    image->texture_id = static_cast<int>(mTextureID);
    glBindTexture(GL_TEXTURE_2D, image->texture_id);

    // Set texture parameters
    if (generate_texture_mipmapped) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // Load image data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 image->width,
                 image->height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 image->pixels);
    if (generate_texture_mipmapped) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    return true;
}


void PGraphicsOpenGLv33::flush_stroke() {
    if (stroke_vertices_xyz_rgba.empty()) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ARRAY_BUFFER, stroke_VBO_xyz_rgba);
    const unsigned long size = stroke_vertices_xyz_rgba.size() * sizeof(float);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(size), stroke_vertices_xyz_rgba.data());

    glUseProgram(stroke_shader_program);

    // Upload matrices
    const GLint projLoc = glGetUniformLocation(stroke_shader_program, "uProjection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection3D)); // or projection2D

    const GLint viewLoc = glGetUniformLocation(stroke_shader_program, "uViewMatrix");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    const GLint matrixLoc = glGetUniformLocation(stroke_shader_program, "uModelMatrix");
    glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, glm::value_ptr(currentMatrix));

    glBindVertexArray(stroke_VAO_xyz_rgba);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(stroke_vertices_xyz_rgba.size()) / NUM_STROKE_VERTEX_ATTRIBUTES_XYZ_RGBA);
    glBindVertexArray(0);

    stroke_vertices_xyz_rgba.clear();
}

void PGraphicsOpenGLv33::flush_fill() {
    if (fill_vertices_xyz_rgba_uv.empty()) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ARRAY_BUFFER, fill_VBO_xyz_rgba_uv);
    const unsigned long size = fill_vertices_xyz_rgba_uv.size() * sizeof(float);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(size), fill_vertices_xyz_rgba_uv.data());

    glUseProgram(fill_shader_program);

    // Upload matrices
    const GLint projLoc = glGetUniformLocation(fill_shader_program, "uProjection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection3D));

    const GLint viewLoc = glGetUniformLocation(fill_shader_program, "uViewMatrix");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    const GLint modelLoc = glGetUniformLocation(fill_shader_program, "uModelMatrix");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(currentMatrix));

    // Bind textures per batch
    glBindVertexArray(fill_VAO_xyz_rgba_uv);
    for (const auto& batch: renderBatches) {
        glBindTexture(GL_TEXTURE_2D, batch.texture_id);
        glDrawArrays(GL_TRIANGLES, batch.start_index, batch.num_vertices);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    fill_vertices_xyz_rgba_uv.clear();
    renderBatches.clear();
}

void PGraphicsOpenGLv33::fill_resize_buffer(const uint32_t newSize) {
    // Create a new buffer
    GLuint newVBO;
    glGenBuffers(1, &newVBO);
    glBindBuffer(GL_ARRAY_BUFFER, newVBO);
    glBufferData(GL_ARRAY_BUFFER, newSize, nullptr, GL_DYNAMIC_DRAW); // Allocate new size

    // Copy old data to new buffer
    glBindBuffer(GL_COPY_READ_BUFFER, fill_VBO_xyz_rgba_uv);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, fill_max_buffer_size);

    // Delete old buffer and update references
    glDeleteBuffers(1, &fill_VBO_xyz_rgba_uv);
    fill_VBO_xyz_rgba_uv = newVBO;
    fill_max_buffer_size = newSize;

    glBindBuffer(GL_ARRAY_BUFFER, fill_VBO_xyz_rgba_uv);
}

void PGraphicsOpenGLv33::add_transformed_fill_vertex_xyz_rgba_uv(const glm::vec3& position,
                                                                 const glm::vec4& color,
                                                                 const float u, const float v) {
    fill_vertices_xyz_rgba_uv.push_back(position.x); // Position
    fill_vertices_xyz_rgba_uv.push_back(position.y); // Position
    fill_vertices_xyz_rgba_uv.push_back(position.z); // Position
    fill_vertices_xyz_rgba_uv.push_back(color.r);    // Color
    fill_vertices_xyz_rgba_uv.push_back(color.g);    // Color
    fill_vertices_xyz_rgba_uv.push_back(color.b);    // Color
    fill_vertices_xyz_rgba_uv.push_back(color.a);    // Color
    fill_vertices_xyz_rgba_uv.push_back(u);          // Texture
    fill_vertices_xyz_rgba_uv.push_back(v);          // Texture
}

void PGraphicsOpenGLv33::add_fill_vertex_xyz_rgba_uv(const glm::vec3 position,
                                                     const glm::vec4 color,
                                                     const glm::vec2 tex_coords) {
    add_fill_vertex_xyz_rgba_uv(position.x, position.y, position.z,
                                color.r, color.g, color.b, color.a,
                                tex_coords.x, tex_coords.y);
}

void PGraphicsOpenGLv33::add_fill_vertex_xyz_rgba_uv_raw(const glm::vec3 position,
                                                         const glm::vec4 color,
                                                         const glm::vec2 tex_coords) {
    fill_vertices_xyz_rgba_uv.push_back(position.x);   // Position
    fill_vertices_xyz_rgba_uv.push_back(position.y);   // Position
    fill_vertices_xyz_rgba_uv.push_back(position.z);   // Position
    fill_vertices_xyz_rgba_uv.push_back(color.r);      // Color
    fill_vertices_xyz_rgba_uv.push_back(color.g);      // Color
    fill_vertices_xyz_rgba_uv.push_back(color.b);      // Color
    fill_vertices_xyz_rgba_uv.push_back(color.a);      // Color
    fill_vertices_xyz_rgba_uv.push_back(tex_coords.x); // Texture
    fill_vertices_xyz_rgba_uv.push_back(tex_coords.y); // Texture
}

void PGraphicsOpenGLv33::to_screen_space(glm::vec3& world_position) const {
    // Transform world position to camera (view) space
    const glm::vec4 viewPos = viewMatrix * currentMatrix * glm::vec4(world_position, 1.0f);

    // Project onto clip space
    glm::vec4 clipPos = projection3D * viewPos;

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

// // TODO optimize … this is inefficient AF
// void PGraphicsOpenGLv33::add_quad_line(const glm::vec3& p1,
//                                        const glm::vec3& p2,
//                                        const glm::vec4& color,
//                                        float            width,
//                                        const glm::mat4& mvp) {
//
//     glm::vec3 p1_transformed = currentMatrix * glm::vec4(p1.x, p1.y, p1.z, 1.0f);
//     glm::vec3 p2_transformed = currentMatrix * glm::vec4(p2.x, p2.y, p2.z, 1.0f);
//     // Transform points to clip space (multiply by MVP)
//     glm::vec4 clipP1 = mvp * glm::vec4(p1_transformed, 1.0f);
//     glm::vec4 clipP2 = mvp * glm::vec4(p2_transformed, 1.0f);
//
//     // Perform perspective division to get NDC coordinates
//     glm::vec2 ndcP1 = glm::vec2(clipP1) / clipP1.w;
//     glm::vec2 ndcP2 = glm::vec2(clipP2) / clipP2.w;
//
//     // Compute the 2D line direction
//     glm::vec2 dir = glm::normalize(ndcP2 - ndcP1);
//
//     // Compute the perpendicular vector for thickness
//     glm::vec2 perp = glm::vec2(-dir.y, dir.x) * (width / 2.0f);
//
//     // Generate the quad vertices in screen space
//     glm::vec2 v1 = ndcP1 + perp;
//     glm::vec2 v2 = ndcP1 - perp;
//     glm::vec2 v3 = ndcP2 + perp;
//     glm::vec2 v4 = ndcP2 - perp;
//
//     // // Convert back to clip space (set z and w = 1)
//     // std::vector<glm::vec3> quad = {
//     //     glm::vec3(v1, clipP1.z),
//     //     glm::vec3(v2, clipP1.z),
//     //     glm::vec3(v4, clipP2.z), // First triangle
//     //     glm::vec3(v1, clipP1.z),
//     //     glm::vec3(v4, clipP2.z),
//     //     glm::vec3(v3, clipP2.z) // Second triangle
//     // };
//     glm::vec2 tex_coords(1, 1);
//
//     add_fill_vertex_xyz_rgba_uv_raw(glm::vec3(v1, clipP1.z), color, tex_coords);
//     add_fill_vertex_xyz_rgba_uv_raw(glm::vec3(v2, clipP1.z), color, tex_coords);
//     add_fill_vertex_xyz_rgba_uv_raw(glm::vec3(v4, clipP1.z), color, tex_coords);
//
//     add_fill_vertex_xyz_rgba_uv_raw(glm::vec3(v1, clipP1.z), color, tex_coords);
//     add_fill_vertex_xyz_rgba_uv_raw(glm::vec3(v4, clipP1.z), color, tex_coords);
//     add_fill_vertex_xyz_rgba_uv_raw(glm::vec3(v3, clipP1.z), color, tex_coords);
// }

void PGraphicsOpenGLv33::add_fill_vertex_xyz_rgba_uv(const float x, const float y, const float z,
                                                     const float r, const float g, const float b, const float a,
                                                     const float u, const float v) {
    // Each vertex consists of 9 floats (x, y, z, r, g, b, u, v)
    // uint32_t vertexSize = NUM_VERTEX_ATTRIBUTES * sizeof(float);
    if ((fill_vertices_xyz_rgba_uv.size() + NUM_FILL_VERTEX_ATTRIBUTES_XYZ_RGBA_UV) * sizeof(float) > fill_max_buffer_size) {
        fill_resize_buffer(fill_max_buffer_size + VBO_BUFFER_CHUNK_SIZE);
    }

    const glm::vec4 transformed = currentMatrix * glm::vec4(x, y, z, 1.0f); // Apply transformation
    fill_vertices_xyz_rgba_uv.push_back(transformed.x);                     // Position
    fill_vertices_xyz_rgba_uv.push_back(transformed.y);                     // Position
    fill_vertices_xyz_rgba_uv.push_back(transformed.z);                     // Position
    fill_vertices_xyz_rgba_uv.push_back(r);                                 // Color
    fill_vertices_xyz_rgba_uv.push_back(g);                                 // Color
    fill_vertices_xyz_rgba_uv.push_back(b);                                 // Color
    fill_vertices_xyz_rgba_uv.push_back(a);                                 // Color
    fill_vertices_xyz_rgba_uv.push_back(u);                                 // Texture
    fill_vertices_xyz_rgba_uv.push_back(v);                                 // Texture
}

void PGraphicsOpenGLv33::add_stroke_vertex_xyz_rgba(const float x, const float y, const float z,
                                                    const float r, const float g, const float b, const float a) {
    // Each vertex consists of 7 floats (x, y, z, r, g, b, u, v)
    // uint32_t vertexSize = NUM_STROKE_VERTEX_ATTRIBUTES_XYZ_RGBA * sizeof(float);
    if ((stroke_vertices_xyz_rgba.size() + NUM_STROKE_VERTEX_ATTRIBUTES_XYZ_RGBA) * sizeof(float) > stroke_max_buffer_size) {
        std::cerr << "Stroke buffer is full!" << std::endl;
        // TODO create version for stroke buffer
        // stroke_resize_buffer(stroke_max_buffer_size + VBO_BUFFER_CHUNK_SIZE);
    }

    const glm::vec4 transformed = currentMatrix * glm::vec4(x, y, z, 1.0f); // Apply transformation
    stroke_vertices_xyz_rgba.push_back(transformed.x);                      // Position
    stroke_vertices_xyz_rgba.push_back(transformed.y);                      // Position
    stroke_vertices_xyz_rgba.push_back(transformed.z);                      // Position
    stroke_vertices_xyz_rgba.push_back(r);                                  // Color
    stroke_vertices_xyz_rgba.push_back(g);                                  // Color
    stroke_vertices_xyz_rgba.push_back(b);                                  // Color
    stroke_vertices_xyz_rgba.push_back(a);                                  // Color
}

void PGraphicsOpenGLv33::init_stroke_vertice_buffers() {
    glGenVertexArrays(1, &stroke_VAO_xyz_rgba);
    glGenBuffers(1, &stroke_VBO_xyz_rgba);

    glBindVertexArray(stroke_VAO_xyz_rgba);

    glBindBuffer(GL_ARRAY_BUFFER, stroke_VBO_xyz_rgba);
    glBufferData(GL_ARRAY_BUFFER, stroke_max_buffer_size, nullptr, GL_DYNAMIC_DRAW);

    const auto STRIDE = static_cast<GLsizei>(NUM_STROKE_VERTEX_ATTRIBUTES_XYZ_RGBA * sizeof(float));

    // Position Attribute (Location 0) -> 3 floats (x, y, z)
    constexpr int NUM_POSITION_ATTRIBUTES = 3;
    glVertexAttribPointer(0, NUM_POSITION_ATTRIBUTES, GL_FLOAT, GL_FALSE, STRIDE, static_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // Color Attribute (Location 1) -> 4 floats (r, g, b, a)
    constexpr int NUM_COLOR_ATTRIBUTES = 4;
    glVertexAttribPointer(1, NUM_COLOR_ATTRIBUTES, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<void*>(NUM_POSITION_ATTRIBUTES * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void PGraphicsOpenGLv33::init_fill_vertice_buffers() {
    glGenVertexArrays(1, &fill_VAO_xyz_rgba_uv);
    glGenBuffers(1, &fill_VBO_xyz_rgba_uv);

    glBindVertexArray(fill_VAO_xyz_rgba_uv);
    glBindBuffer(GL_ARRAY_BUFFER, fill_VBO_xyz_rgba_uv);

    // Allocate buffer memory (maxBufferSize = 1024 KB) but don't fill it yet
    glBufferData(GL_ARRAY_BUFFER, fill_max_buffer_size, nullptr, GL_DYNAMIC_DRAW);

    const auto STRIDE = static_cast<GLsizei>(NUM_FILL_VERTEX_ATTRIBUTES_XYZ_RGBA_UV * sizeof(float));

    // Position Attribute (Location 0) -> 3 floats (x, y, z)
    constexpr int NUM_POSITION_ATTRIBUTES = 3;
    glVertexAttribPointer(0, NUM_POSITION_ATTRIBUTES, GL_FLOAT, GL_FALSE, STRIDE, static_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // Color Attribute (Location 1) -> 4 floats (r, g, b, a)
    constexpr int NUM_COLOR_ATTRIBUTES = 4;
    glVertexAttribPointer(1, NUM_COLOR_ATTRIBUTES, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<void*>(NUM_POSITION_ATTRIBUTES * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture Coordinate Attribute (Location 2) -> 2 floats (u, v)
    constexpr int NUM_TEXTURE_ATTRIBUTES = 2;
    glVertexAttribPointer(2, NUM_TEXTURE_ATTRIBUTES, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<void*>((NUM_POSITION_ATTRIBUTES + NUM_COLOR_ATTRIBUTES) * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void PGraphicsOpenGLv33::createDummyTexture() {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    constexpr unsigned char whitePixel[4] = {255, 255, 255, 255}; // RGBA: White
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    solid_color_texture_id = textureID;
}

GLuint PGraphicsOpenGLv33::build_shader(const char* vertexShaderSource, const char* fragmentShaderSource) {
    // Build shaders
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    checkShaderCompileStatus(vertexShader);

    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    checkShaderCompileStatus(fragmentShader);

    const GLuint mShaderProgram = glCreateProgram();
    glAttachShader(mShaderProgram, vertexShader);
    glAttachShader(mShaderProgram, fragmentShader);
    glLinkProgram(mShaderProgram);
    checkProgramLinkStatus(mShaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return mShaderProgram;
}

void PGraphicsOpenGLv33::checkShaderCompileStatus(const GLuint shader) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
}

void PGraphicsOpenGLv33::checkProgramLinkStatus(const GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
}

void PGraphicsOpenGLv33::printMatrix(const glm::mat4& matrix) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << matrix[j][i] << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


const char* PGraphicsOpenGLv33::vertex_shader_source_texture() {
    const auto vertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;

out vec4 vColor;
out vec2 vTexCoord;

uniform mat4 uProjection;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

void main() {
    gl_Position = uProjection * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
    vColor = aColor;
    vTexCoord = aTexCoord;
}
)";
    return vertexShaderSource;
}

const char* PGraphicsOpenGLv33::fragment_shader_source_texture() {
    const auto fragmentShaderSource = R"(
#version 330 core

in vec4 vColor;
in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    FragColor = texture(uTexture, vTexCoord) * vColor;
}
)";
    return fragmentShaderSource;
}

const char* PGraphicsOpenGLv33::vertex_shader_source_simple() {
    // Vertex Shader source ( without texture )
    const auto vertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;

out vec4 vColor;

uniform mat4 uProjection;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

void main() {
    gl_Position = uProjection * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
    vColor = aColor;
}
)";
    return vertexShaderSource;
}

const char* PGraphicsOpenGLv33::fragment_shader_source_simple() {
    // Fragment Shader source ( without texture )
    const auto fragmentShaderSource = R"(
#version 330 core

in vec4 vColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(vColor);
}
)";
    return fragmentShaderSource;
}