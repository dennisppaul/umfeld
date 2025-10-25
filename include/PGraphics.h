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

#include <stack>
#include <sstream>
#include <glm/glm.hpp>

#include "UmfeldConstants.h"
#include "UmfeldTypes.h"
#include "UmfeldFunctionsAdditional.h"
#include "PImage.h"
#include "Vertex.h"
#include "UShape.h"
#include "Triangulator.h"
#include "UFont.h"

namespace umfeld {
    class PFont;
    class VertexBuffer;
    class PShader;
    class UShapeRenderer;

    class PGraphics : public virtual PImage {
    public:
        struct FrameBufferObject {
            uint32_t id{};
            uint32_t texture_id{};
            int      width{};
            int      height{};
            bool     msaa{false};
        };

        FrameBufferObject framebuffer{};
        bool              render_to_offscreen{true};
        float             depth_range = 10000.0f;

        PGraphics();
        ~PGraphics() override = default;

        void set_triangle_emitter_callback(void (*callback)(std::vector<Vertex>& triangle_vertices)) {
            this->triangle_emitter_callback = callback;
        }
        void (*get_triangle_emitter_callback() const)(std::vector<Vertex>&) {
            return this->triangle_emitter_callback;
        }

        void set_stroke_emitter_callback(void (*callback)(std::vector<Vertex>& triangle_vertices, bool line_strip_closed)) {
            this->stroke_emitter_callback = callback;
        }
        void (*get_stroke_emitter_callback() const)(std::vector<Vertex>&, bool) {
            return this->stroke_emitter_callback;
        }

        /* --- implementation specific methods --- */

        virtual bool read_framebuffer(std::vector<unsigned char>& pixels) { return false; }
        virtual void render_framebuffer_to_screen(const bool use_blit) { (void) use_blit; }; // TODO this should probably go to PGraphicsOpenGL

        /* --- implementation specific methods ( pure virtual ) --- */

        /**
         * @brief method should emit the fill vertices to the rendering backend. recording, collecting,
         *        and transforming vertices needs to happen here. any drawing should use this method.
         * @param triangle_vertices
         */
        virtual void emit_shape_fill_triangles(std::vector<Vertex>& triangle_vertices); // REMOVE asashrimp
        // virtual void IMPL_emit_shape_fill_triangles(std::vector<Vertex>& triangle_vertices) = 0; // REMOVE asashrimp
        /**
         * @brief method should emit the stroke vertices to the rendering backend. recording, collecting,
         *        and transforming vertices needs to happen here. any drawing should use this method.
         * @param line_strip_vertices
         * @param line_strip_closed
         */
        virtual void emit_shape_stroke_line_strip(std::vector<Vertex>& line_strip_vertices, bool line_strip_closed); // REMOVE asashrimp
        // virtual void IMPL_emit_shape_stroke_line_strip(std::vector<Vertex>& line_strip_vertices, bool line_strip_closed) = 0; // REMOVE asashrimp
        virtual void emit_shape_stroke_points(std::vector<Vertex>& point_vertices, float point_size); // REMOVE asashrimp
        // virtual void IMPL_emit_shape_stroke_points(std::vector<Vertex>& point_vertices, float point_size) = 0;                // REMOVE asashrimp
        virtual void beginDraw();
        virtual void endDraw();
        virtual void reset_mvp_matrices();
        virtual void restore_mvp_matrices();

        /* --- implemented in base class PGraphics --- */

        // ## Shape

        // ### 2d Primitives

        virtual void arc(float x, float y, float w, float h, float start, float stop, int mode = PIE);
        virtual void circle(float x, float y, float diameter);
        virtual void ellipse(float a, float b, float c, float d);
        virtual void line(float x1, float y1, float z1, float x2, float y2, float z2);
        virtual void line(float x1, float y1, float x2, float y2);
        virtual void point(float x, float y, float z = 0.0f);
        virtual void quad(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);
        virtual void rect(float x, float y, float width, float height);
        virtual void rect(float x, float y, float width, float height, bool flip_y_texcoords);
        virtual void square(const float x, const float y, const float extent) { rect(x, y, extent, extent); }
        virtual void triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);

        // ### Vertex

        virtual void beginShape(int shape = POLYGON);
        virtual void endShape(bool closed = false);
        virtual void vertex(float x, float y, float z = 0.0f);
        virtual void vertex(float x, float y, float z, float u, float v);
        virtual void vertex(const Vertex& v);
        void         submit_stroke_shape(bool closed, bool force_transparent = false) const;
        void         submit_fill_shape(bool closed, bool force_transparent = false) const;

        // ## Color

        // ### Creating & Reading

        float   alpha(color_t color) const;
        float   blue(color_t color) const;
        float   brightness(color_t color) const;
        color_t color_f(float brightness, float alpha = 1.0f);
        color_t color_f(float r, float g, float b);
        color_t color_f(float r, float g, float b, float a);
        float   green(color_t color) const;
        float   hue(color_t color) const;
        color_t lerpColor(color_t c1, color_t c2, float amt) const;
        float   red(color_t color) const;
        float   saturation(color_t color) const;

        color_t color(float gray);
        color_t color(float gray, float alpha);
        color_t color(float v1, float v2, float v3);
        color_t color(float v1, float v2, float v3, float alpha);

        // ### Setting

        void background(float gray);
        void background(const int gray) { background(static_cast<float>(gray)); }
        void background(const double gray) { background(static_cast<float>(gray)); }
        void background(float gray, float alpha);
        void background(float v1, float v2, float v3);
        void background(float v1, float v2, float v3, float alpha);
        void fill(float gray);
        void fill(const int gray) { fill(static_cast<float>(gray)); }
        void fill(const double gray) { fill(static_cast<float>(gray)); }
        void fill(float gray, float alpha);
        void fill(float v1, float v2, float v3);
        void fill(float v1, float v2, float v3, float alpha);
        void stroke(float gray);
        void stroke(const int gray) { stroke(static_cast<float>(gray)); }
        void stroke(const double gray) { stroke(static_cast<float>(gray)); }
        void stroke(float gray, float alpha);
        void stroke(float v1, float v2, float v3);
        void stroke(float v1, float v2, float v3, float alpha);

        virtual void background_f(float a, float b, float c, float d = 1.0f);
        virtual void background_f(float a);
        virtual void background_color(color_t color);
        virtual void background(PImage* img);
        // clear()
        virtual void colorMode(ColorMode mode, float max);
        virtual void colorMode(ColorMode mode, float max1, float max2, float max3);
        virtual void colorMode(ColorMode mode, float max1, float max2, float max3, float maxA);
        virtual void fill_f(float r, float g, float b, float alpha = 1.0f);
        virtual void fill_f(float gray, float alpha = 1.0f);
        virtual void fill_color(uint32_t c);
        virtual void noFill();
        virtual void noStroke();
        virtual void stroke_f(float r, float g, float b, float alpha = 1.0f);
        virtual void stroke_f(float gray, float alpha);
        virtual void stroke_f(float a);
        virtual void stroke_color(uint32_t c);

        // ## Transform

        // virtual void applyMatrix()
        virtual void popMatrix();
        virtual void printMatrix(const glm::mat4& matrix);
        virtual void printMatrix();
        virtual void pushMatrix();
        virtual void resetMatrix();
        virtual void rotateX(float angle);
        virtual void rotateY(float angle);
        virtual void rotateZ(float angle);
        virtual void rotate(float angle);
        virtual void rotate(float angle, float x, float y, float z);
        virtual void scale(float x);
        virtual void scale(float x, float y);
        virtual void scale(float x, float y, float z);
        // virtual void shearX()
        // virtual void shearY()
        virtual void translate(float x, float y, float z = 0.0f);

        virtual void strokeWeight(float weight);
        virtual void strokeJoin(int join);
        virtual void strokeCap(int cap);

        // ## Structure

        virtual void  pushStyle();
        virtual void  popStyle();
        virtual void  bezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
        virtual void  bezier(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);
        virtual void  bezierDetail(int detail);
        virtual void  curve(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
        virtual void  curve(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);
        virtual void  curveDetail(int detail);
        virtual void  curveTightness(float tightness);
        virtual void  arcDetail(int detail);
        virtual void  ellipseMode(int mode);
        virtual void  ellipseDetail(int detail);
        virtual void  image(PImage* img, float x, float y, float w, float h);
        virtual void  image(PImage* img, float x, float y);
        virtual void  texture(PImage* img = nullptr);
        virtual void  pointSize(float size);
        virtual void  rectMode(int mode);
        virtual void  textFont(PFont* font);
        virtual void  textSize(float size);
        virtual void  text(const char* value, float x, float y, float z = 0.0f);
        virtual float textWidth(const std::string& text);
        virtual void  textAlign(int alignX, int alignY = BASELINE);
        virtual float textAscent();
        virtual float textDescent();
        virtual void  textLeading(float leading);
        // virtual PFont* loadFont(const std::string& file, float size);
        virtual void box(float width, float height, float depth);
        virtual void box(const float size) { box(size, size, size); }
        virtual void sphere(float width, float height, float depth);
        virtual void sphere(const float size) { sphere(size, size, size); }
        virtual void sphereDetail(int ures, int vres);
        virtual void sphereDetail(const int res) { sphereDetail(res, res); }
        // void             process_collected_fill_vertices();
        // void             process_collected_stroke_vertices(bool close_shape);
        virtual void     shader(PShader* shader);
        virtual PShader* loadShader(const std::string& vertex_code, const std::string& fragment_code, const std::string& geometry_code = "") { return nullptr; };
        virtual void     resetShader();
        virtual void     normal(float x, float y, float z, float w = 0);
        virtual void     blendMode(const BlendMode mode) { current_blend_mode = mode; }
        virtual void     beginCamera();
        virtual void     endCamera();
        virtual void     camera();
        virtual void     camera(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ);
        virtual void     frustum(float left, float right, float bottom, float top, float near, float far);
        virtual void     ortho(float left, float right, float bottom, float top, float near, float far);
        virtual void     perspective(float fovy, float aspect, float near, float far);
        virtual void     printCamera();
        virtual void     printProjection();
        virtual void     lights() {}
        virtual void     noLights() {}
        virtual void     ambientLight(float r, float g, float b, float x, float y, float z) {}
        virtual void     directionalLight(float r, float g, float b, float nx, float ny, float nz) {}
        virtual void     pointLight(float r, float g, float b, float x, float y, float z) {}
        virtual void     spotLight(float r, float g, float b, float x, float y, float z, float nx, float ny, float nz, float angle, float concentration) {}
        virtual void     lightFalloff(float constant, float linear, float quadratic) {}
        virtual void     lightSpecular(float r, float g, float b) {}
        virtual void     ambient(float r, float g, float b) {}
        virtual void     specular(float r, float g, float b) {}
        virtual void     emissive(float r, float g, float b) {}
        virtual void     shininess(float s) {}

        using PImage::loadPixels;
        void         loadPixels() { download_texture(this); }
        void         updatePixels() { update_full_internal(this); }
        virtual void pixelDensity(int density);
        virtual int  displayDensity();

        /* --- additional --- */

        virtual void        flush();
        virtual void        mesh(VertexBuffer* mesh_shape);
        virtual void        lock_init_properties(const bool lock_properties) { init_properties_locked = lock_properties; }
        virtual void        hint(uint16_t property);
        virtual void        text_str(const std::string& text, float x, float y, float z = 0.0f); // TODO maybe make this private?
        virtual void        debug_text(const std::string& text, float x, float y);
        void                to_screen_space(glm::vec3& model_position) const; // NOTE: convert from model space to screen space
        void                to_world_space(glm::vec3& model_position) const;  // NOTE: convert from model space to works space
        void                linse(const float x1, const float y1, const float x2, const float y2) { line(x1, y1, x2, y2); }
        int                 getPixelDensity() const { return pixel_density; }
        void                set_point_render_mode(const int point_render_mode) { this->point_render_mode = point_render_mode; }
        int                 get_point_render_mode() const { return point_render_mode; }
        int                 get_point_size() const { return current_stroke_state.point_weight; }
        void                set_stroke_render_mode(const int stroke_render_mode) { this->stroke_render_mode = stroke_render_mode; }
        int                 get_stroke_render_mode() const { return stroke_render_mode; }
        void                stroke_properties(float stroke_join_round_resolution, float stroke_cap_round_resolution, float stroke_join_miter_max_angle);
        void                triangulate_line_strip_vertex(const glm::mat4&           model_matrix,
                                                          const std::vector<Vertex>& line_strip,
                                                          const StrokeState&         stroke,
                                                          bool                       close_shape,
                                                          std::vector<Vertex>&       line_vertices) const;
        virtual void        set_default_graphics_state() {}
        void                set_render_mode(const RenderMode render_mode) { this->render_mode = render_mode; }
        RenderMode          get_render_mode() const { return render_mode; }
        virtual std::string name() { return "PGraphics"; }
        float               get_stroke_weight() const { return current_stroke_state.stroke_weight; }
        virtual void        texture_filter(TextureFilter filter) {}
        virtual void        texture_wrap(TextureWrap wrap, glm::vec4 color_fill = glm::vec4(0.0f)) {}
        virtual int         texture_update_and_bind(PImage* img) { return 0; }
        virtual void        upload_texture(PImage* img, const uint32_t* pixel_data, int width, int height, int offset_x, int offset_y) {}
        virtual void        download_texture(PImage* img) {}
        virtual void        upload_colorbuffer(uint32_t* pixels) {}
        virtual void        download_colorbuffer(uint32_t* pixels) {}
        virtual void        update_full_internal(PImage* img) {}
        virtual void        set_shader_program(PShader* shader, ShaderProgramType shader_role);
        virtual BlendMode   get_blend_mode() const { return current_blend_mode; }

        template<typename T>
        void text(const T& value, const float x, const float y, const float z = 0.0f) {
            std::ostringstream ss;
            ss << value;
            text_str(ss.str(), x, y, z);
        }

        static std::vector<Vertex> triangulate_faster(const std::vector<Vertex>& vertices);
        static std::vector<Vertex> triangulate_better_quality(const std::vector<Vertex>& vertices);
        static std::vector<Vertex> triangulate_good(const std::vector<Vertex>& vertices);
        void                       convert_fill_shape_to_triangles(UShape& s) const;
        static void                convert_stroke_shape_to_line_strip(UShape& s, std::vector<UShape>& shapes);

    protected:
        // const float                      DEFAULT_FOV            = 2.0f * atan(0.5f); // = 53.1301f; // P5 :: tan(PI*30.0 / 180.0);
        static constexpr uint16_t        ELLIPSE_DETAIL_MIN     = 3;
        static constexpr uint16_t        ELLIPSE_DETAIL_DEFAULT = 36;
        static constexpr uint16_t        ARC_DETAIL_DEFAULT     = 36;
        UShapeRenderer*                  shape_renderer{nullptr}; // TODO @maybe make this `const` and set in constructor?
        PFont*                           current_font{nullptr};
        PImage*                          current_texture{nullptr};
        ShapeState                       current_shape{};
        PShader*                         current_custom_shader{nullptr};
        PImage*                          texture_stack_top{nullptr}; // NOTE not really a stack yet ;)
        bool                             texture_stack_used{false};
        StrokeState                      current_stroke_state;
        BlendMode                        current_blend_mode{BLEND};
        std::stack<StyleState>           style_stack;
        LightingState                    lightingState;
        bool                             lights_enabled{false};
        bool                             init_properties_locked{false};
        ColorState                       color_stroke{};
        ColorState                       color_fill{};
        int                              rect_mode{CORNER};
        int                              ellipse_mode{CENTER};
        int                              ellipse_detail{0};
        int                              arc_detail{ARC_DETAIL_DEFAULT};
        std::vector<glm::vec2>           ellipse_points_LUT{};
        int                              bezier_detail{20};
        int                              curve_detail{20};
        float                            curve_tightness{0.0f};
        uint8_t                          pixel_density{1};
        bool                             shape_force_transparent{false};
        int                              polygon_triangulation_strategy{POLYGON_TRIANGULATION_BETTER};
        int                              stroke_render_mode{STROKE_RENDER_MODE_TRIANGULATE_2D};
        int                              point_render_mode{POINT_RENDER_MODE_TRIANGULATE};
        inline static const Triangulator triangulator{};
        std::vector<ColorState>          color_stroke_stack{};
        std::vector<ColorState>          color_fill_stack{};
        std::vector<Vertex>              box_fill_vertices_LUT{};
        std::vector<Vertex>              box_stroke_vertices_LUT{};
        std::vector<Vertex>              sphere_vertices_LUT{};
        int                              sphere_u_resolution{DEFAULT_SPHERE_RESOLUTION};
        int                              sphere_v_resolution{DEFAULT_SPHERE_RESOLUTION};
        static constexpr uint32_t        VBO_BUFFER_CHUNK_SIZE{1024 * 1024}; // 1MB
        std::vector<Vertex>              shape_stroke_vertex_buffer{VBO_BUFFER_CHUNK_SIZE};
        std::vector<Vertex>              shape_fill_vertex_buffer{VBO_BUFFER_CHUNK_SIZE};
        bool                             model_matrix_dirty{false};
        glm::vec4                        current_normal{Vertex::DEFAULT_NORMAL};
        glm::mat4                        temp_view_matrix{};
        glm::mat4                        temp_projection_matrix{};
        RenderMode                       render_mode{RENDER_MODE_SORTED_BY_SUBMISSION_ORDER};
        bool                             in_camera_block{false};
        const UFont*                     debug_font{nullptr};
        void (*triangle_emitter_callback)(std::vector<Vertex>&){nullptr};
        void (*stroke_emitter_callback)(std::vector<Vertex>&, bool){nullptr};
        bool current_force_transparent{false};

        struct ColorModeState {
            ColorMode mode  = RGB;
            glm::vec4 range = glm::vec4(DEFAULT_COLOR_RANGE);
        };
        ColorModeState color_mode_state;

    public:
        glm::mat4              model_matrix{};
        glm::mat4              view_matrix{};
        glm::mat4              projection_matrix{};
        std::vector<glm::mat4> model_matrix_stack{};
        bool                   hint_force_enable_depth_test{false};
        bool                   auto_flush{true};

        void push_force_transparent() {
            current_force_transparent = shape_force_transparent;
        }

        void pop_force_transparent() {
            shape_force_transparent = current_force_transparent;
        }

        void push_texture_id() {
            if (!texture_stack_used) {
                texture_stack_top  = current_texture;
                texture_stack_used = true;
            } else {
                warning("unbalanced texture id *push*/pop");
            }
        }

        void pop_texture_id() {
            if (texture_stack_used) {
                current_texture    = texture_stack_top;
                texture_stack_top  = nullptr;
                texture_stack_used = false;
            } else {
                warning("unbalanced texture id push/*pop*");
            }
        }

        void set_shape_force_transparent(const bool shape_force_transparent) {
            this->shape_force_transparent = shape_force_transparent;
        }

    protected:
        void auto_flush_on_matrix_change();
        int  get_current_texture_id() const;

        static bool has_transparent_vertices(const std::vector<Vertex>& vertices) {
            for (auto& v: vertices) {
                if (v.color.a < 1.0f) {
                    return true;
                }
            }
            return false;
        }

        void vertex_vec(const glm::vec3& position, const glm::vec2& tex_coords) {
            vertex(position.x, position.y, position.z, tex_coords.x, tex_coords.y);
        }

        static glm::vec4 as_vec4(const ColorState& color) {
            return {color.r, color.g, color.b, color.a};
        }

        static void push_color_state(const ColorState& current, std::vector<ColorState>& stack) {
            stack.push_back(current);
        }

        static void pop_color_state(ColorState& current, std::vector<ColorState>& stack) {
            if (!stack.empty()) {
                current = stack.back();
                stack.pop_back();
            }
        }

        void interpret_color_mode(glm::vec4& color, float v1, float v2, float v3, float alpha) const;

        void resize_ellipse_points_LUT();
        void update_projection_matrix(const glm::mat4& proj);
        void update_view_matrix(const glm::mat4& view);
    };
} // namespace umfeld
