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

#pragma once

// TODO implement color modes and ranges i.e 0–255 or 0–1 and HSB or RGB ...

/*
// TODO implement 2D Primitives
arc() Draws an arc in the display window
circle() Draws a circle to the screen
ellipse() Draws an ellipse (oval) in the display window
line() Draws a line (a direct path between two points) to the screen  // 3D
point() Draws a point, a coordinate in space at the dimension of one pixel // 3D
quad() A quad is a quadrilateral, a four sided polygon
rect() Draws a rectangle to the screen
square() Draws a square to the screen
triangle() A triangle is a plane created by connecting three points
// TODO implement 3D Primitives
box() A box is an extruded rectangle
sphereDetail() Controls the detail used to render a sphere by adjusting the number of vertices of the sphere mesh
sphere() A sphere is a hollow ball made from tessellated triangles
*/

#ifndef DISABLE_GRAPHICS
#endif // DISABLE_GRAPHICS

#include <sstream>
#include <glm/glm.hpp>

#include "UmgebungConstants.h"
#include "PImage.h"
#include "Vertex.h"

namespace umgebung {

    class PFont;
    class PGraphics : public virtual PImage {
    public: // TODO clean up
        struct FBO {
            unsigned int id{};
            unsigned int texture_id{};
            int          width{};
            int          height{};
        };

        FBO framebuffer{};

        virtual void render_framebuffer_to_screen(bool use_blit = false) {}

    public:
        PGraphics();
        ~PGraphics() override = default;

        /* --- implemented in base class PGraphics --- */

        virtual void fill(float r, float g, float b, float alpha = 1.0f);
        virtual void fill(float gray, float alpha = 1.0f);
        virtual void fill_color(uint32_t c);
        virtual void noFill();
        virtual void stroke(float r, float g, float b, float alpha = 1.0f);
        virtual void stroke(float gray, float alpha);
        virtual void stroke(float a);
        virtual void stroke_color(uint32_t c);
        virtual void noStroke();
        virtual void rectMode(int mode);
        virtual void ellipseMode(int mode);
        virtual void ellipseDetail(int detail);
        virtual void pointSize(float size);

        /* --- additional --- */

        virtual void        upload_texture(PImage* img, const uint32_t* pixel_data, int width, int height, int offset_x, int offset_y, bool mipmapped) {}
        virtual void        download_texture(PImage* img) {}
        virtual void        reset_matrices() {}
        virtual void        bind_texture(int texture_id) {}
        virtual void        unbind_texture() {}
        virtual std::string name() { return "PGraphics"; }
        virtual void        lock_init_properties(const bool lock_properties) { init_properties_locked = lock_properties; }

        /* --- interface --- */

        // TODO implement 3D version
        // virtual void    line(float x1, float y1, float z1, float x2, float y2, float z2)    = 0;

        virtual void    strokeWeight(float weight)                                                                         = 0;
        virtual void    background(float a, float b, float c, float d = 1.0f)                                              = 0;
        virtual void    background(float a)                                                                                = 0;
        virtual void    rect(float x, float y, float width, float height)                                                  = 0;
        virtual void    ellipse(float x, float y, float width, float height)                                               = 0;
        virtual void    circle(float x, float y, float radius)                                                             = 0;
        virtual void    line(float x1, float y1, float x2, float y2)                                                       = 0;
        virtual void    line(float x1, float y1, float z1, float x2, float y2, float z2)                                   = 0;
        virtual void    triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) = 0;
        virtual void    bezier(float x1, float y1, float x2, float y2,
                               float x3, float y3, float x4, float y4)                                                     = 0;
        virtual void    bezier(float x1, float y1, float z1, float x2, float y2, float z2,
                               float x3, float y3, float z3, float x4, float y4, float z4)                                 = 0;
        virtual void    bezierDetail(int detail)                                                                           = 0;
        virtual void    point(float x, float y, float z = 0.0f)                                                            = 0;
        virtual void    beginShape(int shape = POLYGON)                                                                    = 0;
        virtual void    endShape(bool close_shape = false)                                                                 = 0;
        virtual void    vertex(float x, float y, float z = 0.0f)                                                           = 0;
        virtual void    vertex(float x, float y, float z, float u, float v)                                                = 0;
        virtual PFont*  loadFont(const std::string& file, float size)                                                      = 0;
        virtual void    textFont(PFont* font)                                                                              = 0;
        virtual void    textSize(float size)                                                                               = 0;
        virtual void    text(const char* value, float x, float y, float z = 0.0f)                                          = 0;
        virtual float   textWidth(const std::string& text)                                                                 = 0;
        virtual PImage* loadImage(const std::string& filename)                                                             = 0;
        virtual void    image(PImage* img, float x, float y, float w, float h)                                             = 0;
        virtual void    image(PImage* img, float x, float y)                                                               = 0;
        virtual void    texture(PImage* img)                                                                               = 0;
        virtual void    popMatrix();
        virtual void    pushMatrix();
        virtual void    resetMatrix();
        virtual void    printMatrix(const glm::mat4& matrix);
        virtual void    printMatrix();
        virtual void    translate(float x, float y, float z = 0.0f);
        virtual void    rotateX(float angle);
        virtual void    rotateY(float angle);
        virtual void    rotateZ(float angle);
        virtual void    rotate(float angle);
        virtual void    rotate(float angle, float x, float y, float z);
        virtual void    scale(float x);
        virtual void    scale(float x, float y);
        virtual void    scale(float x, float y, float z);
        virtual void    pixelDensity(int density)                                           = 0;
        virtual void    hint(uint16_t property)                                             = 0;
        virtual void    beginDraw()                                                         = 0;
        virtual void    endDraw()                                                           = 0;
        virtual void    text_str(const std::string& text, float x, float y, float z = 0.0f) = 0; // TODO maybe make this private?

        template<typename T>
        void text(const T& value, const float x, const float y, const float z = 0.0f) {
            std::ostringstream ss;
            ss << value;
            text_str(ss.str(), x, y, z);
        }

        bool render_to_offscreen{true};

    protected:
        struct ColorState : glm::vec4 {
            bool active = false;
        };

        const float               DEFAULT_FOV            = 2.0f * atan(0.5f); // = 53.1301f;
        static constexpr uint16_t ELLIPSE_DETAIL_MIN     = 3;
        static constexpr uint16_t ELLIPSE_DETAIL_DEFAULT = 18;
        bool                      init_properties_locked{false};
        PFont*                    current_font{nullptr};
        ColorState                color_stroke{};
        ColorState                color_fill{};
        uint8_t                   rect_mode{CORNER};
        uint8_t                   ellipse_mode{CENTER};
        int                       ellipse_detail{0};
        std::vector<glm::vec2>    ellipse_points_LUT;
        float                     point_size{1};
        float                     stroke_weight{1};
        int                       bezier_detail{20};
        uint8_t                   pixel_density{1};
        int                       texture_id_current{};
        bool                      shape_has_begun{false};
        int                       polygon_triangulation_strategy = POLYGON_TRIANGULATION_BETTER;

        /* --- transform matrices --- */

        glm::mat4              model_matrix_client{};
        glm::mat4              model_matrix_shader{};
        std::vector<glm::mat4> model_matrix_stack{};
        bool                   model_matrix_dirty{false};
        glm::mat4              projection_matrix_2D{};
        glm::mat4              projection_matrix_3D{};
        glm::mat4              view_matrix{};

        static glm::vec4 as_vec4(const ColorState& color) {
            return {color.r, color.g, color.b, color.a};
        }
        uint8_t get_pixel_density() const { return pixel_density; }

        static std::vector<Vertex> triangulate_faster(const std::vector<Vertex>& vertices);
        static std::vector<Vertex> triangulate_better_quality(const std::vector<Vertex>& vertices);

        void resize_ellipse_points_LUT();
        void to_screen_space(glm::vec3& world_position) const; // convert from model space to screen space
        void to_world_space(glm::vec3& model_position) const;  // convert from model space to works space
    };

} // namespace umgebung