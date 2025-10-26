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

#include <SDL3/SDL.h>

#include <cstdint>
#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include "UmfeldDefines.h"
#include "UmfeldConstants.h"

namespace umfeld {
    class PAudio;
    class PImage;
    struct AudioUnitInfo;
    class LibraryListener;
    struct Vertex;
    template<typename T>
    class SamplerT;
    using Sampler = SamplerT<float>;

    // TODO clean up function order
    // Color
    void    rgb_to_hsb_f(float r, float g, float b, float& h, float& s, float& v);
    void    hsb_to_rgb_f(float h, float s, float v, float& r, float& g, float& b);
    color_t color_pack_f(float r, float g, float b, float a);
    void    color_unpack_f(color_t color, float& r, float& g, float& b, float& a);
    float   red_f(color_t color);
    float   green_f(color_t color);
    float   blue_f(color_t color);
    float   alpha_f(color_t color);
    // String
    inline bool              equals(const std::string_view a, const std::string_view b) { return a == b; }
    bool                     begins_with(const std::string& str, const std::string& prefix);
    bool                     ends_with(const std::string& str, const std::string& suffix);
    bool                     file_exists(const std::string& file_path);
    bool                     resource_file_exists(const std::string& resource_file_path);
    bool                     directory_exists(const std::string& dir_path);
    std::string              find_file_in_paths(const std::vector<std::string>& paths, const std::string& filename);
    std::string              find_in_environment_path(const std::string& filename);
    std::string              get_executable_location();
    std::vector<std::string> get_files(const std::string& directory, const std::string& extension = "");
    int                      get_int_from_argument(const std::string& argument);
    std::string              get_string_from_argument(const std::string& argument);
    std::string              timestamp();
    void                     audio(int  input_channels   = DEFAULT_INPUT_CHANNELS,
                                   int  output_channels  = DEFAULT_OUTPUT_CHANNELS,
                                   int  sample_rate      = DEFAULT_SAMPLE_RATE,
                                   int  buffer_size      = DEFAULT_AUDIO_BUFFER_SIZE,
                                   int  input_device_id  = DEFAULT_AUDIO_DEVICE,
                                   int  output_device_id = DEFAULT_AUDIO_DEVICE,
                                   bool threaded         = DEFAULT_AUDIO_RUN_IN_THREAD);
    void                     audio(int                input_channels,
                                   int                output_channels,
                                   int                sample_rate,
                                   int                buffer_size,
                                   const std::string& input_device_name,
                                   const std::string& output_device_name,
                                   bool               threaded = DEFAULT_AUDIO_RUN_IN_THREAD);
    void                     audio(const AudioUnitInfo& info);
    void                     audio_start(PAudio* device = nullptr);
    void                     audio_stop(PAudio* device = nullptr);
    uint32_t                 get_audio_sample_rate();
    int8_t                   get_audio_input_channels();
    int8_t                   get_audio_output_channels();
    uint32_t                 get_audio_buffer_size();
    bool                     is_initialized();
    std::string              getTitle();
    void                     getLocation(const int& x, const int& y);
    void                     setWindowSize(int width, int height); // TODO does not work ATM
    void                     getWindowSize(int& width, int& height);
    void                     set_frame_rate(float fps);
    void                     register_library(LibraryListener* listener);         /* implemented in subsystems */
    void                     unregister_library(const LibraryListener* listener); /* implemented in subsystems */
    std::vector<Vertex>      loadOBJ(const std::string& file, bool material = true);
    Sampler*                 loadSample(const std::string& file, bool resample_to_audio_device = true);
    void                     saveImage(const PImage* image, const std::string& filename);
    PAudio*                  createAudio(const AudioUnitInfo* device_info);
    void                     profile(Profile profile_name);

    /* --- utilities --- */

    SDL_WindowFlags get_SDL_WindowFlags(SDL_WindowFlags& flags);

    template<typename Func, typename... Args>
    double time_function_ms(Func&& func, Args&&... args) {
        const auto start = std::chrono::high_resolution_clock::now();

        // Call the function with forwarded arguments
        std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);

        const auto                                      end     = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double, std::milli> elapsed = end - start;
        return elapsed.count(); // in milliseconds
    }

    /* --- templated functions --- */

    template<typename T>
    static auto to_printable(const T& value) -> typename std::conditional<std::is_same<T, uint8_t>::value, int, const T&>::type {
        if constexpr (std::is_same_v<T, uint8_t>) {
            return static_cast<int>(value);
        } else {
            return value;
        }
    }

    template<typename... Args>
    std::string to_string(const Args&... args) {
        std::ostringstream oss;
        (oss << ... << args);
        return oss.str();
    }

    /* --- logging --- */

    template<typename... Args>
    void error(const Args&... args) {
#if (UMFELD_PRINT_ERRORS)
        std::ostringstream os;
        ((os << to_printable(args)), ...);
        std::cerr
            << timestamp() << " "
            << "UMFELD.ERROR   : "
            << os.str()
            << std::endl;
        std::flush(std::cerr);
#endif
    }

    template<typename... Args>
    void warning(const Args&... args) {
#if (UMFELD_PRINT_WARNINGS)
        std::ostringstream os;
        ((os << to_printable(args)), ...);
        std::cerr
            << timestamp() << " "
            << "UMFELD.WARNING : "
            << os.str()
            << std::endl;
        std::flush(std::cerr);
#endif
    }

    template<typename... Args>
    void console(const Args&... args) {
#if (UMFELD_PRINT_CONSOLE)
        std::ostringstream os;
        ((os << to_printable(args)), ...);
        std::cout
            << timestamp() << " "
            << "UMFELD.CONSOLE : "
            << os.str()
            << std::endl;
        std::flush(std::cout);
#endif
    }

    template<typename... Args>
    void console_n(const Args&... args) {
#if (UMFELD_PRINT_CONSOLE)
        std::ostringstream os;
        ((os << to_printable(args)), ...);
        std::cout
            << timestamp() << " "
            << "UMFELD.CONSOLE : "
            << os.str();
        std::flush(std::cout);
#endif
    }

    template<typename... Args>
    void console_c(const Args&... args) {
#if (UMFELD_PRINT_CONSOLE)
        std::ostringstream os;
        ((os << to_printable(args)), ...);
        std::cout
            << os.str();
        std::flush(std::cout);
#endif
    }

#define error_in_function(...)   umfeld::error("'", __func__, "' :: ", __VA_ARGS__)
#define warning_in_function(...) umfeld::warning("'", __func__, "' :: ", __VA_ARGS__)
#define console_in_function(...) umfeld::console("'", __func__, "' :: ", __VA_ARGS__)
#define _console_once(counter, ...)               \
    do {                                          \
        static bool _once_flag_##counter = false; \
        if (!_once_flag_##counter) {              \
            umfeld::console(__VA_ARGS__);         \
            _once_flag_##counter = true;          \
        }                                         \
    } while (0)
#define console_once(...) \
    _console_once(__COUNTER__, __VA_ARGS__)
#define _warning_in_function_once(counter, ...)                   \
    do {                                                          \
        static bool _once_flag_##counter = false;                 \
        if (!_once_flag_##counter) {                              \
            umfeld::warning("'", __func__, "' :: ", __VA_ARGS__); \
            _once_flag_##counter = true;                          \
        }                                                         \
    } while (0)
#define warning_in_function_once(...) \
    _warning_in_function_once(__COUNTER__, __VA_ARGS__)

    inline std::string format_label(const std::string& label, const size_t width = DEFAULT_CONSOLE_LABEL_WIDTH) {
        if (label.length() >= width) {
            return label + " : "; // Ensure spacing even if label is too long
        }
        return label + std::string(width - label.length(), ' ') + " : ";
    }

    inline std::string fl(const std::string& text) {
        constexpr size_t column_width = DEFAULT_CONSOLE_LABEL_WIDTH;
        return format_label(text, column_width);
    }

    inline std::string separator(const bool equal_sign = true, const std::size_t length = DEFAULT_CONSOLE_WIDTH) {
        return std::string(length, equal_sign ? '=' : '-');
    }

#define run_once(code)             \
    do {                           \
        static bool _done = false; \
        if (!_done) {              \
            _done = true;          \
            code                   \
        }                          \
    } while (false)

    /* --- random --- */

    inline Random& _random_mode() {
        static Random mode = FAST;
        return mode;
    }

    inline void   set_random_mode(const Random mode) { _random_mode() = mode; }
    inline Random get_random_mode() { return _random_mode(); }
    void          set_sketch_path(const std::string& path);
} // namespace umfeld