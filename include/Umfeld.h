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
#include <string>

// ReSharper disable once CppUnusedIncludeDirective
#include "UmfeldDefines.h"
#include "UmfeldConstants.h"
#include "UmfeldCallbacks.h"
#include "UmfeldFunctions.h"
#include "UmfeldRecorder.h"
#include "UmfeldFunctionsGraphics.h"
#include "Subsystems.h"

// #define UMFELD_DISABLE_DEPRECATED_AUDIO

namespace umfeld {

    /* public variables used for *initialization only*  */

    /* --- audio  --- */
    inline SampleAcquisitionMode audio_sample_acquisition_mode = AUDIO_BLOCK;
    inline bool                  enable_audio                  = false;
    inline int                   audio_unique_device_id        = 0x0010;
    // inline int        audio_format       = 0; // TODO currently only supporting F32

    /* --- graphics --- */
    inline bool enable_graphics         = false;
    inline int  antialiasing            = DEFAULT;
    inline int  display                 = DEFAULT;
    inline bool fullscreen              = false;
    inline bool borderless              = false;
    inline bool resizable               = false;
    inline bool retina_support          = true;
    inline bool always_on_top           = false;
    inline bool vsync                   = false;
    inline bool render_to_buffer        = true;
    inline int  save_image_jpeg_quailty = 100;

    /* --- libraries + events --- */
    inline bool enable_libraries     = true;
    inline bool enable_events        = true;
    inline bool run_update_in_thread = DEFAULT_UPDATE_RUN_IN_THREAD;

    /* public variables ( updated by system ) */

    /* --- audio  --- */
    /**
     * @brief global pointer to current audio device. is set by audio subsystem after initialization.
     */
    inline AudioUnitInfo audio_unit_info;
    inline PAudio* audio_device        = nullptr; // TODO consider renaming `a` to `audio` … which would however conflict with the `audio` function
    // [[deprecated("use 'audio_device' instead")]]
    // inline bool    run_audio_in_thread = DEFAULT_AUDIO_RUN_IN_THREAD;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use 'audio_device' instead")]]
    // #endif
    //     inline PAudio*& a                   = audio_device;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use 'void audioEvent(const PAudio& audio) {}' instead")]]
    // #endif
    //     inline float* audio_input_buffer = nullptr;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use 'void audioEvent(const PAudio& audio) {}' instead")]]
    // #endif
    //     inline int8_t audio_input_channels = DEFAULT_INPUT_CHANNELS;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use 'void audioEvent(const PAudio& audio) {}' instead")]]
    // #endif
    //     inline float* audio_output_buffer = nullptr;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use 'void audioEvent(const PAudio& audio) {}' instead")]]
    // #endif
    //     inline int8_t audio_output_channels = DEFAULT_OUTPUT_CHANNELS;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use 'void audioEvent(const PAudio& audio) {}' instead")]]
    // #endif
    //     inline uint32_t audio_sample_rate = DEFAULT_SAMPLE_RATE;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use 'void audioEvent(const PAudio& audio) {}' instead")]]
    // #endif
    //     inline uint32_t audio_buffer_size = DEFAULT_AUDIO_BUFFER_SIZE;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use 'void audioEvent(const PAudio& audio) {}' instead")]]
    // #endif
    //     inline int audio_input_device_id = DEFAULT_AUDIO_DEVICE;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use 'void audioEvent(const PAudio& audio) {}' instead")]]
    // #endif
    //     inline std::string audio_input_device_name = DEFAULT_AUDIO_DEVICE_NAME;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use 'void audioEvent(const PAudio& audio) {}' instead")]]
    // #endif
    //     inline int audio_output_device_id = DEFAULT_AUDIO_DEVICE;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use 'void audioEvent(const PAudio& audio) {}' instead")]]
    // #endif
    //     inline std::string audio_output_device_name = DEFAULT_AUDIO_DEVICE_NAME;
    //     // NOTE duplicate variables for backward compatibility
    //     // ReSharper disable CppDeprecatedEntity
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use 'run_audio_in_thread' instead")]]
    // #endif
    //     inline bool& audio_threaded = run_audio_in_thread;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use audio_input_buffer instead")]]
    // #endif
    //     inline float*& input_buffer = audio_input_buffer;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use audio_input_channels instead")]]
    // #endif
    //     inline int8_t& input_channels = audio_input_channels;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use audio_output_buffer instead")]]
    // #endif
    //     inline float*& output_buffer = audio_output_buffer;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use audio_output_channels instead")]]
    // #endif
    //     inline int8_t& output_channels = audio_output_channels;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use audio_sample_rate instead")]]
    // #endif
    //     inline uint32_t& sample_rate = audio_sample_rate;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use audio_buffer_size instead")]]
    // #endif
    //     inline uint32_t& buffer_size = audio_buffer_size;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use audio_input_device_id instead")]]
    // #endif
    //     inline int& input_device_id = audio_input_device_id;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use audio_input_device_name instead")]]
    // #endif
    //     inline std::string& input_device_name = audio_input_device_name;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use audio_output_device_id instead")]]
    // #endif
    //     inline int& output_device_id = audio_output_device_id;
    // #ifndef UMFELD_DISABLE_DEPRECATED_AUDIO
    //     [[deprecated("use audio_output_device_name instead")]]
    // #endif
    //     inline std::string& output_device_name = audio_output_device_name;
    //     // ReSharper restore CppDeprecatedEntity

    /* --- graphics --- */
    inline PGraphics* g                    = nullptr;
    inline uint32_t*  pixels               = nullptr;
    inline float      width                = DEFAULT_WINDOW_WIDTH;
    inline float      height               = DEFAULT_WINDOW_HEIGHT;
    inline int        frameCount           = 0;
    inline float      frameRate            = DEFAULT_FRAME_RATE;
    inline int        key                  = 0; // events
    inline int        keyCode              = 0; // events
    inline bool       isKeyPressed         = false;
    inline int        mouseButton          = DEFAULT;
    inline bool       isMousePressed       = false;
    inline float      mouseX               = 0;
    inline float      mouseY               = 0;
    inline float      pmouseX              = 0;
    inline float      pmouseY              = 0;
    inline Renderer   renderer             = RENDERER_DEFAULT;
    inline int        display_width        = DEFAULT;
    inline int        display_height       = DEFAULT;
    inline int        depth_buffer_depth   = 24;
    inline int        stencil_buffer_depth = 8;
    inline bool       double_buffered      = true;


    /* public variables *mainly for internal use* */

    inline bool                    use_esc_key_to_quit = true;
    inline bool                    request_shutdown    = false;
    inline std::vector<Subsystem*> subsystems;
    inline SubsystemGraphics*      subsystem_graphics   = nullptr;
    inline SubsystemAudio*         subsystem_audio      = nullptr;
    inline SubsystemLibraries*     subsystem_libraries  = nullptr;
    inline Subsystem*              subsystem_hid_events = nullptr;

    // TODO move these functions to `UmfeldFunctions`

    inline SDL_Window* get_window() {
        if (subsystem_graphics != nullptr && subsystem_graphics->get_sdl_window != nullptr) {
            return subsystem_graphics->get_sdl_window();
        }
        return nullptr;
    }

    inline void* get_native_renderer() {
        if (subsystem_graphics != nullptr && subsystem_graphics->get_renderer != nullptr) {
            return subsystem_graphics->get_renderer();
        }
        return nullptr;
    }

    inline int get_native_renderer_type() {
        if (subsystem_graphics != nullptr && subsystem_graphics->get_renderer_type != nullptr) {
            return subsystem_graphics->get_renderer_type();
        }
        return DEFAULT;
    }

    inline void add_subsystem(Subsystem* subsystem) {
        if (subsystem != nullptr) {
            subsystems.push_back(subsystem);
        }
    }

    inline bool remove_subsystem(const Subsystem* subsystem) {
        for (auto it = subsystems.begin(); it != subsystems.end(); ++it) {
            if (*it == subsystem) {
                subsystems.erase(it);
                return true;
            }
        }
        return false;
    }
} // namespace umfeld
