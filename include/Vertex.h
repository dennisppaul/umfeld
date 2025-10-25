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

#include <glm/glm.hpp>
#include <glm/gtx/type_aligned.hpp>

#include <ostream>

namespace umfeld {
    struct Vertex {
        static constexpr int  ATTRIBUTE_LOCATION_POSITION     = 0;
        static constexpr int  ATTRIBUTE_LOCATION_NORMAL       = 1;
        static constexpr int  ATTRIBUTE_LOCATION_COLOR        = 2;
        static constexpr int  ATTRIBUTE_LOCATION_TEXCOORD     = 3;
        static constexpr int  ATTRIBUTE_LOCATION_TRANSFORM_ID = 4;
        static constexpr int  ATTRIBUTE_LOCATION_USERDATA     = 5;
        static constexpr int  ATTRIBUTE_SIZE_POSITION         = 4;
        static constexpr int  ATTRIBUTE_SIZE_NORMAL           = 4;
        static constexpr int  ATTRIBUTE_SIZE_COLOR            = 4;
        static constexpr int  ATTRIBUTE_SIZE_TEXCOORD         = 3;
        static constexpr int  ATTRIBUTE_SIZE_TRANSFORM_ID     = 1;
        static constexpr int  ATTRIBUTE_SIZE_USERDATA         = 1;
        static constexpr auto ATTRIBUTE_NAME_POSITION         = "aPosition";
        static constexpr auto ATTRIBUTE_NAME_NORMAL           = "aNormal";
        static constexpr auto ATTRIBUTE_NAME_COLOR            = "aColor";
        static constexpr auto ATTRIBUTE_NAME_TEXCOORD         = "aTexCoord";
        static constexpr auto ATTRIBUTE_NAME_TRANSFORM_ID     = "a_transform_id";
        static constexpr auto ATTRIBUTE_NAME_USERDATA         = "aUserdata";
        static inline auto    DEFAULT_POSITION                = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        static inline auto    DEFAULT_NORMAL                  = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        static inline auto    DEFAULT_COLOR                   = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        static inline auto    DEFAULT_TEX_COORD               = glm::vec3(0.0f, 0.0f, 0.0f);
        static constexpr auto DEFAULT_TRANSFORM_ID            = 0;
        static constexpr auto DEFAULT_USERDATA                = 0;
        glm::aligned_vec4     position;
        glm::aligned_vec4     normal;
        glm::aligned_vec4     color;
        glm::vec3             tex_coord;
        uint16_t              transform_id{DEFAULT_TRANSFORM_ID};
        uint16_t              userdata{DEFAULT_USERDATA};
        // TODO check with profiler if reduced data types are faster. i.e
        //      - glm::aligned_vec3     position;
        //      - glm::aligned_vec3     normal;
        //      - glm::vec2             tex_coord;
        // NOTE aligned_vec4 makes sure all data types are aligned to 16 bytes ( GLM_ENABLE_EXPERIMENTAL )
        //      make sure this does not cause any issues … it s experimental after all

        Vertex(const float x, const float y, const float z = DEFAULT_POSITION.z,
               const float r = DEFAULT_COLOR.r, const float g = DEFAULT_COLOR.g, const float b = DEFAULT_COLOR.b, const float a = DEFAULT_COLOR.a,
               const float u = DEFAULT_TEX_COORD.x, const float v = DEFAULT_TEX_COORD.y)
            : position(x, y, z, DEFAULT_POSITION.w),
              normal(DEFAULT_NORMAL),
              color(r, g, b, a),
              tex_coord(u, v, 0.0f) {}

        explicit Vertex(const glm::vec3& position,
                        const glm::vec4& color     = DEFAULT_COLOR,
                        const glm::vec3& tex_coord = DEFAULT_TEX_COORD,
                        const glm::vec4& normal    = DEFAULT_NORMAL)
            : position(position, DEFAULT_POSITION.w),
              normal(normal),
              color(color),
              tex_coord(tex_coord) {}

        explicit Vertex(const glm::vec2& position)
            : Vertex(glm::vec3(position, DEFAULT_POSITION.z)) {}

        Vertex()
            : Vertex(glm::vec3(DEFAULT_POSITION)) {}

        // copy constructor
        Vertex(const Vertex& other)
            : position(other.position),
              normal(other.normal),
              color(other.color),
              tex_coord(other.tex_coord),
              transform_id(other.transform_id),
              userdata(other.userdata) {}

        // assignment operator
        Vertex& operator=(const Vertex& other) {
            if (this != &other) {
                position     = other.position;
                normal       = other.normal;
                color        = other.color;
                tex_coord    = other.tex_coord;
                transform_id = other.transform_id;
                userdata     = other.userdata;
            }
            return *this;
        }

        friend std::ostream& operator<<(std::ostream& os, const Vertex& v) {
            os << "Vertex { "
               << "\tposition (" << v.position.x << ", " << v.position.y << ", " << v.position.z << ")" << std::endl
               // << "normal: " << v.normal << ", "
               << "\tcolor " << v.color.r << ", " << v.color.g << ", " << v.color.b << ", " << v.color.a << ")" << std::endl
               // << "tex_coord: " << v.tex_coord << ", "
               // << "transform_id: " << v.transform_id << ", "
               // << "userdata: " << v.userdata
               << " }";
            return os;
        }
    };
    static_assert(sizeof(Vertex) == 64, "Vertex size should be exactly 64 bytes");

} // namespace umfeld