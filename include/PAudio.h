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
#include "UmfeldConstants.h"

namespace umfeld {

    void merge_interleaved_stereo(const float* left, const float* right, float* interleaved, size_t frames);
    void split_interleaved_stereo(float* left, float* right, const float* interleaved, size_t frames);

    // typedef struct KLST_AudioBlock {
    //     uint32_t sample_rate;
    //     uint8_t  output_channels;
    //     uint8_t  input_channels;
    //     uint16_t block_size;
    //     float**  output;
    //     float**  input;
    //     uint8_t  device_id;
    // } KLST_AudioBlock;

    struct AudioBlock {
        bool     is_interleaved{true};
        uint32_t sample_rate{DEFAULT_SAMPLE_RATE};
        /** buffer for audio input samples. buffer is interleaved by default
         * ( i.e. samples for each channel are contiguous in memory ).
         */
        float* input_buffer{nullptr};
        int8_t input_channels{0};
        /** buffer for audio output samples. buffer is interleaved by default
         * ( i.e. samples for each channel are contiguous in memory ).
         */
        float* output_buffer{nullptr};
        int8_t output_channels{0};
        /** number of *frame* ( i.e samples per channel )
         * e.g for a device with 2 output channels (`output_channels = 2`) and
         * 256 buffer size (`buffer_size = 256`) the length of `output_buffer`
         * is 512 samples (`output_channels * buffer_size` -> 2 channels * 256 = 512 samples).
         */
        uint32_t buffer_size{DEFAULT_AUDIO_BUFFER_SIZE};
        // int format; // TODO currently supporting F32 format only
    };

    struct AudioUnitInfo : AudioBlock {
        /**
         * unique id of audio unit. id is set by audio subsystem. it is not to be confused with `input_device_id`
         * or `output_device_id`. a unit is a combination of input and output device.
         */
        int unique_id{AUDIO_UNIT_NOT_INITIALIZED};
        /** case-sensitive audio device name ( or beginning of the name )
         * if audio device is supposed to be intialized by name
         * make sure to set `input_device_id` to `FIND_AUDIO_DEVICE_BY_NAME`.
         * <code>
         * audio_device_info.input_device_id = FIND_AUDIO_DEVICE_BY_NAME;
         * audio_device_info.name            = "MacBook";
         * </code>
         * name may be reset or completed by audio system.
         */
        int         input_device_id{DEFAULT_AUDIO_DEVICE};
        std::string input_device_name{DEFAULT_AUDIO_DEVICE_NAME};
        int         output_device_id{DEFAULT_AUDIO_DEVICE};
        std::string output_device_name{DEFAULT_AUDIO_DEVICE_NAME};
        bool        threaded{DEFAULT_AUDIO_RUN_IN_THREAD};
    };

    class PAudio : public AudioUnitInfo {
    public:
        explicit PAudio(const AudioUnitInfo* device_info);
        void copy_input_buffer_to_output_buffer() const;
    };
} // namespace umfeld