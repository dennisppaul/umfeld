/*
 *  Umfeld
 *
 *  This file is part of the *Umfeld* library (https://github.com/dennisppaul/umfeld).
 *  Copyright (c) 2025 Dennis P Paul.
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "PAudio.h"
#include "Umfeld.h"

namespace umfeld {
    void acquire_audio_buffer(const PAudio* audio_device);
    void acquire_audio_buffer_per_sample_no_input(const PAudio* audio_device);
    void acquire_audio_buffer_per_sample_with_input(const PAudio* audio_device);

    inline void acquire_audio_buffer_per_sample_no_input(const PAudio* audio_device) {
        constexpr int num_output_channels = 2;
        // TOOD check 'num_output_channels' against the actual number of channels
        if (audio_device->output_channels == num_output_channels) {
            for (int i = 0; i < audio_device->buffer_size * num_output_channels; i += num_output_channels) {
                float left  = 0.0f;
                float right = 0.0f;
                run_audioEventFloatRef_2_callback(left, right);
                audio_device->output_buffer[i + 0] = left;
                audio_device->output_buffer[i + 1] = right;
            }
        } else {
            warning_in_function("currently only stereo output (2 channels) is supported");
        }
    }

    inline void acquire_audio_buffer_per_sample_with_input(const PAudio* audio_device) {
        if (audio_device == nullptr) {
            return;
        }
        constexpr int num_output_channels = 2;
        // TODO check 'num_output_channels' against the actual number of channels
        if (audio_device->output_channels == num_output_channels) {
            for (int i = 0; i < audio_device->buffer_size * num_output_channels; i += num_output_channels) {
                float input_left;
                float input_right;
                if (audio_device->input_channels == 2) {
                    input_left  = audio_device->input_buffer[i + 0];
                    input_right = audio_device->input_buffer[i + 1];
                } else if (audio_device->input_channels == 1) {
                    warning_in_function_once("detected mono input. duplicating input for right channel");
                    input_left  = audio_device->input_buffer[i / 2 + 0];
                    input_right = input_left; // NOTE duplicating input for right channel
                } else {
                    warning_in_function_once("currently only mono or stere input is supported");
                    input_left  = 0.0f;
                    input_right = 0.0f;
                }
                float output_left  = 0.0f;
                float output_right = 0.0f;
                run_audioEventFloatRef_4_callback(input_left, input_right, output_left, output_right);
                audio_device->output_buffer[i + 0] = output_left;
                audio_device->output_buffer[i + 1] = output_right;
            }
        } else {
            warning_in_function_once("currently only stereo output (2 channels) is supported");
        }
    }

    inline void acquire_audio_buffer(const PAudio* audio_device) {
        if (audio_device == nullptr) {
            return;
        }
        switch (audio_sample_acquisition_mode) {
            case AUDIO_PER_SAMPLE_NO_INPUT:
                acquire_audio_buffer_per_sample_no_input(audio_device);
                break;
            case AUDIO_PER_SAMPLE_WITH_INPUT:
                acquire_audio_buffer_per_sample_with_input(audio_device);
                break;
            case AUDIO_BLOCK:
            default:
                // NOTE block mode handles sample acquisition in `run_audioEventPAudio_callback`
                break;
        }
    }

    inline void acquire_audio_device_audio_buffer_samples(const PAudio* local_audio) {
        if (local_audio == nullptr) {
            return;
        }
        // NOTE acquire samples from main audio device based on 'audio_sample_acquisition_mode'
        if (audio_device != nullptr) {
            // NOTE acquire sample data for *main* audio device except for block mode this is handled implicitly below
            if (local_audio == audio_device) {
                acquire_audio_buffer(local_audio);
            }
        }

        // NOTE acquire sample data for all registered audio devices ( including *main* audio device )
        run_audioEventPAudio_callback(*local_audio);
    }

} // namespace umfeld