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

#include <string>
#include <unordered_map>
#include "UmfeldSDLOpenGL.h" // TODO move to cpp implementation
#include <glm/glm.hpp>

namespace umfeld {
    class PShader {
    public:
        PShader();
        ~PShader();

        void set_uniform(const std::string& name, int value);
        void set_uniform(const std::string& name, int value_a, int value_b);
        void set_uniform(const std::string& name, float value);
        void set_uniform(const std::string& name, float value_a, float value_b);
        void set_uniform(const std::string& name, const glm::vec2& value);
        void set_uniform(const std::string& name, const glm::vec3& value);
        void set_uniform(const std::string& name, const glm::vec4& value);
        void set_uniform(const std::string& name, const glm::mat3& value);
        void set_uniform(const std::string& name, const glm::mat4& value);
        void check_uniform_location(const std::string& name) const;
        void check_for_matrix_uniforms();

        // TODO maybe move these to implementation
        bool   load(const std::string& vertex_code, const std::string& fragment_code, const std::string& geometry_code = "");
        void   use();
        void   unuse();
        GLuint get_program_id() const { return programID; }
        bool   is_bound() const { return in_use; }

        bool debug_uniform_location = true;
        bool has_model_matrix       = false;
        bool has_view_matrix        = false;
        bool has_projection_matrix  = false;
        bool has_texture_unit       = false;

    private:
        GLuint                                 programID;
        std::unordered_map<std::string, GLint> uniformLocations;
        bool                                   in_use{false};

        static GLuint compileShader(const std::string& source, GLenum type);
        static void   checkCompileErrors(GLuint shader, GLenum type);
        static void   checkLinkErrors(GLuint program);
        GLint         getUniformLocation(const std::string& name);
    };
} // namespace umfeld
