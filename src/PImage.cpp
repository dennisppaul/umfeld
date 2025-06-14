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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Umfeld.h"
#include "PImage.h"
#include "PGraphics.h"

using namespace umfeld;

PImage::PImage() : width(0),
                   height(0),
                   format(0),
                   pixels(nullptr) {
    /* note that PImage is not initialized with any data in this constructor branch */
}

PImage::PImage(const int width, const int height, const int format) : width(static_cast<float>(width)),
                                                                      height(static_cast<float>(height)),
                                                                      format(format),
                                                                      pixels(nullptr) {
    const int length = width * height;
    if (length <= 0) {
        return;
    }
    pixels = new uint32_t[length]{0x00000000};
    PImage::init(pixels, width, height, format, false);
}

PImage::PImage(const std::string& filepath) : width(0),
                                              height(0),
                                              format(0),
                                              pixels(nullptr) {
    if (!file_exists(filepath)) {
        error("file not found: '", filepath, "'");
        return;
    }

    int            _width    = 0;
    int            _height   = 0;
    int            _channels = 0;
    unsigned char* data      = stbi_load(filepath.c_str(), &_width, &_height, &_channels, 0);
    if (data) {
        if (_channels != 4 && _channels != 3) {
            error("unsupported image format, defaulting to RGBA forcing 4 color channels.");
            _channels = 4;
        }
        pixels = new uint32_t[_width * _height];
        for (int i = 0; i < _width * _height; ++i) {
            const int j = i * _channels;
            if (_channels == 4) {
                pixels[i] = RGBA(data[j + 0], data[j + 1], data[j + 2], data[j + 3]);
            } else if (_channels == 3) {
                pixels[i] = RGBA(data[j + 0], data[j + 1], data[j + 2], 0xFF);
            }
        }

        if (_channels == 3) {
            console("Note that RGB is converted to RGBA and number of channels is changed to 4");
            _channels = 4;
        }
        PImage::init(pixels, _width, _height, _channels, true);
    } else {
        error("failed to load image: ", filepath);
    }
    stbi_image_free(data);
}


void PImage::init(uint32_t*  pixels,
                  const int  width,
                  const int  height,
                  const int  format,
                  const bool generate_mipmap) {
    if (pixels == nullptr) {
        warning(umfeld::format_label("PImage::init()"), "pixel buffer is not initialized ( might be intentional )");
    }
    this->pixels = pixels;
    this->width  = static_cast<float>(width);
    this->height = static_cast<float>(height);
    this->format = format;
    if (format != 4) {
        warning("unsupported image format, defaulting to RGBA forcing 4 color channels.");
        this->format = 4;
    }
}

static float clamp(const float x) {
    return (x < 0 ? 0 : x) > 1 ? 1 : x;
}

void PImage::update(PGraphics*   graphics,
                    const float* pixel_data,
                    const int    width,
                    const int    height,
                    const int    offset_x,
                    const int    offset_y) {
    /* NOTE pixel data must be 4 times the length of pixels */
    const int length = width * height;
    uint32_t  mPixels[length];
    for (int i = 0; i < width * height; ++i) {
        const int j = i * 4;
        mPixels[i]  = RGBA(clamp(pixel_data[j + 0]) * 255,
                           clamp(pixel_data[j + 1]) * 255,
                           clamp(pixel_data[j + 2]) * 255,
                           clamp(pixel_data[j + 3]) * 255);
    }
    update(graphics, mPixels, width, height, offset_x, offset_y);
}

void PImage::update_full_internal(PGraphics* graphics) {
    graphics->upload_texture(this,
                             pixels,
                             static_cast<int>(width), static_cast<int>(height),
                             0, 0, true);
}

void PImage::update(PGraphics* graphics, const uint32_t* pixel_data) {
    update(graphics, pixel_data, static_cast<int>(this->width), static_cast<int>(this->height), 0, 0);
}

void PImage::updatePixels(PGraphics* graphics) {
    update(graphics, pixels, static_cast<int>(this->width), static_cast<int>(this->height), 0, 0);
}

void PImage::updatePixels(PGraphics* graphics, const int x, const int y, const int w, const int h) {
    if (!pixels) {
        error("pixel array not initialized");
        return;
    }

    if (x < 0 || y < 0 || x + w > static_cast<int>(this->width) || y + h > static_cast<int>(this->height)) {
        return;
    }

    const int length  = w * h;
    auto*     mPixels = new uint32_t[length];
    for (int i = 0; i < length; ++i) {
        const int src_index = (y + i / w) * static_cast<int>(this->width) + (x + i % w);
        mPixels[i]          = pixels[src_index];
    }
    update(graphics, mPixels, w, h, x, y);
    delete[] mPixels;
}

/**
 * @deprecated this should not use OpenGL directly
 */
void PImage::loadPixels(PGraphics* graphics) {
    if (graphics == nullptr) {
        return;
    }
    graphics->download_texture(this);
}

void PImage::update(PGraphics*      graphics,
                    const uint32_t* pixel_data,
                    const int       width,
                    const int       height,
                    const int       offset_x,
                    const int       offset_y) {
    if (!pixel_data) {
        error("invalid pixel data");
        return;
    }
    if (!pixels) {
        error("pixel array not initialized");
        return;
    }

    /* copy subregion `pixel_data` with offset into `pixels` */

    // Ensure the region is within bounds
    if (offset_x < 0 || offset_y < 0 ||
        offset_x + width > static_cast<int>(this->width) ||
        offset_y + height > static_cast<int>(this->height)) {
        error("subregion is out of bounds");
        return;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int src_index  = y * width + x;
            const int dest_index = (offset_y + y) * static_cast<int>(this->width) + (offset_x + x);
            pixels[dest_index]   = pixel_data[src_index];
        }
    }

    graphics->upload_texture(this, pixel_data, width, height, offset_x, offset_y, true);
}