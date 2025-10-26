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

#include "Umfeld.h"
#include "PAudio.h"

using namespace umfeld;

PAudio::PAudio(const AudioUnitInfo* device_info) : AudioUnitInfo(*device_info) {}

void umfeld::merge_interleaved_stereo(const float* left, const float* right, float* interleaved, const size_t frames) {
    if (left == nullptr || right == nullptr || interleaved == nullptr) {
        return;
    }
    for (size_t i = 0; i < frames; ++i) {
        interleaved[i * 2]     = left[i];  // Left channel
        interleaved[i * 2 + 1] = right[i]; // Right channel
    }
}

void umfeld::split_interleaved_stereo(float* left, float* right, const float* interleaved, const size_t frames) {
    if (left == nullptr || right == nullptr || interleaved == nullptr) {
        return;
    }
    for (size_t i = 0; i < frames; ++i) {
        left[i]  = interleaved[i * 2];     // Extract left channel
        right[i] = interleaved[i * 2 + 1]; // Extract right channel
    }
}

void PAudio::copy_input_buffer_to_output_buffer() const {
    if (output_channels == input_channels) {
        std::memcpy(output_buffer,                                 // destination
                    input_buffer,                                  // source
                    input_channels * buffer_size * sizeof(float)); // size
    } else {
        for (int i = 0; i < buffer_size; ++i) {
            for (int ch = 0; ch < output_channels; ++ch) {
                if (ch < input_channels) {
                    output_buffer[i * output_channels + ch] = input_buffer[i * input_channels + ch];
                } else {
                    output_buffer[i * output_channels + ch] = 0.0f; // fill remaining channels with silence
                }
            }
        }
    }
}
