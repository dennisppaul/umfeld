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

#include "Vertex.h"
#include "PGraphicsOpenGLConstants.h"
#include "UFontPixels.h"

namespace umfeld {

    class UFont {
        // NOTE used for debug text ;)
        static constexpr int _CHAR_WIDTH        = 8;
        static constexpr int _CHAR_HEIGHT       = 12;
        static constexpr int ATLAS_COLS        = 16;
        static constexpr int ATLAS_ROWS        = 8;
        static constexpr int FONT_ATLAS_WIDTH  = _CHAR_WIDTH * ATLAS_COLS;
        static constexpr int FONT_ATLAS_HEIGHT = _CHAR_HEIGHT * ATLAS_ROWS;

        void generateFontAtlas() {
            uint8_t pixelData[FONT_ATLAS_HEIGHT][FONT_ATLAS_WIDTH][4] = {};
            for (uint32_t ascii_char = 32; ascii_char < 128; ++ascii_char) {
                const int charX = ((ascii_char - 32) % ATLAS_COLS) * _CHAR_WIDTH;
                const int charY = ((ascii_char - 32) / ATLAS_COLS) * _CHAR_HEIGHT;
                for (uint32_t i = 0; i < Font_7x10.height; i++) {
                    const uint16_t b = Font_7x10.data[(ascii_char - 32) * Font_7x10.height + i];
                    for (uint32_t j = 0; j < Font_7x10.width; j++) {
                        const bool pixelSet  = (b << j) & 0x8000;
                        const int  px        = charX + j;
                        const int  py        = charY + i;
                        pixelData[py][px][0] = pixelSet ? 255 : 0;
                        pixelData[py][px][1] = pixelSet ? 255 : 0;
                        pixelData[py][px][2] = pixelSet ? 255 : 0;
                        pixelData[py][px][3] = pixelSet ? 255 : 0;
                    }
                }
            }

            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         UMFELD_DEFAULT_INTERNAL_PIXEL_FORMAT,
                         FONT_ATLAS_WIDTH,
                         FONT_ATLAS_HEIGHT, 0,
                         UMFELD_DEFAULT_INTERNAL_PIXEL_FORMAT,
                         UMFELD_DEFAULT_TEXTURE_PIXEL_TYPE,
                         pixelData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

    public:
        UFont() {
            generateFontAtlas();
        }

        GLuint textureID;

        std::vector<Vertex> generate(const std::string& text, const float startX, const float startY, glm::vec4 color) const {
            std::vector<Vertex> vertices;
            float               x = startX, y = startY;
            for (const char c: text) {
                const int       index = static_cast<unsigned char>(c) - 32;
                float           u     = (index % ATLAS_COLS) * (1.0f / ATLAS_COLS);
                float           v     = (index / ATLAS_COLS) * (1.0f / ATLAS_ROWS);
                constexpr float uSize = 1.0f / ATLAS_COLS;
                constexpr float vSize = 1.0f / ATLAS_ROWS;

                vertices.emplace_back(x, y, 0, color.r, color.g, color.b, color.a, u, v);
                vertices.emplace_back(x + _CHAR_WIDTH, y, 0, color.r, color.g, color.b, color.a, u + uSize, v);
                vertices.emplace_back(x + _CHAR_WIDTH, y + _CHAR_HEIGHT, 0, color.r, color.g, color.b, color.a, u + uSize, v + vSize);

                vertices.emplace_back(x, y, 0, color.r, color.g, color.b, color.a, u, v);
                vertices.emplace_back(x + _CHAR_WIDTH, y + _CHAR_HEIGHT, 0, color.r, color.g, color.b, color.a, u + uSize, v + vSize);
                vertices.emplace_back(x, y + _CHAR_HEIGHT, 0, color.r, color.g, color.b, color.a, u, v + vSize);

                x += _CHAR_WIDTH;
            }
            return vertices;
        }
    };
} // namespace umfeld
