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

#include <vector>

#include "UmfeldSDLOpenGL.h" // TODO move to cpp implementation
#include "Umfeld.h"
#include "Vertex.h"

namespace umfeld {
    class VertexBuffer {
    public:
        // NOTE make sure to align to locations in shader.
        static constexpr int ATTRIBUTE_LOCATION_POSITION = 0;
        static constexpr int ATTRIBUTE_LOCATION_NORMAL   = 1;
        static constexpr int ATTRIBUTE_LOCATION_COLOR    = 2;
        static constexpr int ATTRIBUTE_LOCATION_TEXCOORD = 3;
        static constexpr int ATTRIBUTE_LOCATION_USERDATA = 4;
        static constexpr int ATTRIBUTE_SIZE_POSITION     = 4;
        static constexpr int ATTRIBUTE_SIZE_NORMAL       = 4;
        static constexpr int ATTRIBUTE_SIZE_COLOR        = 4;
        static constexpr int ATTRIBUTE_SIZE_TEXCOORD     = 3;
        static constexpr int ATTRIBUTE_SIZE_USERDATA     = 1;

        ~VertexBuffer();

        void                 add_vertex(const Vertex& vertex);
        void                 add_vertices(const std::vector<Vertex>& new_vertices);
        void                 draw();
        void                 clear();
        void                 update();
        std::vector<Vertex>& vertices_data() { return _vertices; }
        void                 init();
        void                 set_shape(int shape, bool map_to_opengl_draw_mode = true);
        int                  get_shape() const { return shape; }

    private:
        const int           VBO_BUFFER_CHUNK_SIZE_BYTES = 1024 * 16 * sizeof(Vertex);
        std::vector<Vertex> _vertices;
        GLuint              vbo                = 0;
        GLuint              vao                = 0;
        bool                vao_supported      = false;
        bool                initial_upload     = false;
        bool                buffer_initialized = false;
        int                 server_buffer_size = 0;
        int                 shape              = TRIANGLES;
        bool                dirty              = false;

        void        resize_buffer();
        static void enable_vertex_attributes();
        static void disable_vertex_attributes();
        void        upload();
        void        checkVAOSupport();
        bool        needs_buffer_resize(size_t current_size) const;
        bool        needs_buffer_shrink(size_t current_size) const;
        void        upload_with_resize(size_t current_size, size_t required_bytes);
    };
} // namespace umfeld