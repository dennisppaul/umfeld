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

#include "ShaderSource.h"

namespace umfeld {
    inline ShaderSource shader_source_color_texture_ES{
        .vertex   = R"(#version 300 es
                    precision mediump float;

                    layout(location = 0) in vec4 aPosition;
                    layout(location = 1) in vec4 aNormal;
                    layout(location = 2) in vec4 aColor;
                    layout(location = 3) in vec2 aTexCoord;

                    out vec4 vColor;
                    out vec2 vTexCoord;

                    uniform mat4 uProjection;
                    uniform mat4 uViewMatrix;
                    uniform mat4 uModelMatrix;

                    void main() {
                        gl_Position = uProjection * uViewMatrix * uModelMatrix * aPosition;
                        vColor = aColor;
                        vTexCoord = aTexCoord;
                    }
        )",
        .fragment = R"(#version 300 es
                    precision mediump float;

                    in vec4 vColor;
                    in vec2 vTexCoord;

                    out vec4 FragColor;

                    uniform sampler2D uTexture;

                    void main() {
                        FragColor = texture(uTexture, vTexCoord) * vColor;
                    }
        )"};
}
