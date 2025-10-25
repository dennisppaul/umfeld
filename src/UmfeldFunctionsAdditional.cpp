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

// ReSharper disable CppDeprecatedEntity
#include <filesystem>

#if defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

#include "tiny_obj_loader.h"

#include "Umfeld.h"
#include "UmfeldFunctions.h"
#include "audio/AudioFileReader.h"
#include "audio/Sampler.h"

namespace umfeld {
    void profile(const Profile profile_name) {
        if (g == nullptr) {
            warning_in_function_once("use 'profile' only in 'setup()' or 'draw()' ( not in 'settings()' ).");
            return;
        }
        switch (profile_name) {
            case PROFILE_2D: {
                g->set_render_mode(RENDER_MODE_SORTED_BY_SUBMISSION_ORDER);
                g->set_stroke_render_mode(STROKE_RENDER_MODE_TRIANGULATE_2D);
                g->set_point_render_mode(POINT_RENDER_MODE_TRIANGULATE);
                hint(ENABLE_SMOOTH_LINES);
            } break;
            case PROFILE_3D: {
                g->set_render_mode(RENDER_MODE_SORTED_BY_Z_ORDER);
                // g->set_stroke_render_mode(STROKE_RENDER_MODE_NATIVE);
                g->set_stroke_render_mode(STROKE_RENDER_MODE_LINE_SHADER);
                g->set_point_render_mode(POINT_RENDER_MODE_NATIVE);
            } break;
        }
    }

    bool begins_with(const std::string& str, const std::string& prefix) {
        if (prefix.size() > str.size()) {
            return false;
        }
        return str.substr(0, prefix.size()) == prefix;
    }

    bool ends_with(const std::string& str, const std::string& suffix) {
        if (suffix.size() > str.size()) {
            return false;
        }
        return str.substr(str.size() - suffix.size()) == suffix;
    }

    /**
     * assumes a normalized color range ( i.e [0...1] )
     * @param color
     * @param r
     * @param g
     * @param b
     * @param a
     */
    void color_unpack_f(const color_t color, float& r, float& g, float& b, float& a) {
        a = static_cast<float>(color >> 24 & 0xFF) / 255.0f;
        b = static_cast<float>(color >> 16 & 0xFF) / 255.0f;
        g = static_cast<float>(color >> 8 & 0xFF) / 255.0f;
        r = static_cast<float>(color & 0xFF) / 255.0f;
    }

    /**
     * assumes a normalized color range ( i.e [0...1] )
     * @param r
     * @param g
     * @param b
     * @param a
     * @return
     */
    color_t color_pack_f(const float r, const float g, const float b, const float a) {
        const uint32_t ir = static_cast<uint32_t>(r * 255.0f) & 0xFF;
        const uint32_t ig = static_cast<uint32_t>(g * 255.0f) & 0xFF;
        const uint32_t ib = static_cast<uint32_t>(b * 255.0f) & 0xFF;
        const uint32_t ia = static_cast<uint32_t>(a * 255.0f) & 0xFF;
        return ia << 24 | ib << 16 | ig << 8 | ir;
    }

    /**
     * assumes a normalized color range ( i.e [0...1] )
     * @param r
     * @param g
     * @param b
     * @param h
     * @param s
     * @param v
     */
    void rgb_to_hsb_f(const float r, const float g, const float b, float& h, float& s, float& v) {
        const float max   = std::max(r, std::max(g, b));
        const float min   = std::min(r, std::min(g, b));
        const float delta = max - min;

        v = max;
        s = max == 0.0f ? 0.0f : delta / max;

        if (delta == 0.0f) {
            h = 0.0f; // undefined hue
        } else {
            if (max == r) {
                h = fmodf((g - b) / delta, 6.0f);
            } else if (max == g) {
                h = (b - r) / delta + 2.0f;
            } else {
                h = (r - g) / delta + 4.0f;
            }
            h *= 60.0f;
            if (h < 0.0f) {
                h += 360.0f;
            }
        }
    }

    /**
     * assumes a normalized color range ( i.e [0...1] )
     * @param h
     * @param s
     * @param v
     * @param r
     * @param g
     * @param b
     */
    void hsb_to_rgb_f(const float h, const float s, const float v, float& r, float& g, float& b) {
        const float c = v * s;
        const float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
        const float m = v - c;

        float rp, gp, bp;
        if (h < 60.0f) {
            rp = c;
            gp = x;
            bp = 0.0f;
        } else if (h < 120.0f) {
            rp = x;
            gp = c;
            bp = 0.0f;
        } else if (h < 180.0f) {
            rp = 0.0f;
            gp = c;
            bp = x;
        } else if (h < 240.0f) {
            rp = 0.0f;
            gp = x;
            bp = c;
        } else if (h < 300.0f) {
            rp = x;
            gp = 0.0f;
            bp = c;
        } else {
            rp = c;
            gp = 0.0f;
            bp = x;
        }

        r = rp + m;
        g = gp + m;
        b = bp + m;
    }

    bool file_exists(const std::string& file_path) {
        std::error_code             ec;
        const std::filesystem::path path(file_path);
        return std::filesystem::exists(path, ec) && std::filesystem::is_regular_file(path, ec);
    }

    bool resource_file_exists(const std::string& resource_file_path) {
        return file_exists(resolve_data_path(resource_file_path));
    }

    bool directory_exists(const std::string& dir_path) {
        std::error_code             ec;
        const std::filesystem::path path(dir_path);
        return std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec);
    }

    std::string find_file_in_paths(const std::vector<std::string>& paths, const std::string& filename) {
        for (const auto& path: paths) {
            std::filesystem::path full_path = std::filesystem::path(path) / filename;
            if (std::filesystem::exists(full_path)) {
                return full_path.string();
            }
        }
        return "";
    }

    std::string find_in_environment_path(const std::string& filename) {
        std::string path;
        char*       env = getenv("PATH");
        if (env != nullptr) {
            std::string path_env(env);
            // On Windows, PATH entries are separated by ';', on Linux/macOS by ':'
#if defined(_WIN32)
            char path_separator = ';';
#else
            char path_separator = ':';
#endif

            std::istringstream ss(path_env);
            std::string        token;

            while (std::getline(ss, token, path_separator)) {
                std::filesystem::path candidate = std::filesystem::path(token) / filename;
                if (std::filesystem::exists(candidate)) {
                    path = candidate.string();
                    break;
                }
            }
        }
        return path;
    }

    std::string get_executable_location() {
#if defined(__APPLE__) || defined(__linux__)
        Dl_info info;
        // Get the address of a function within the library (can be any function)
        if (dladdr((void*) &get_executable_location, &info)) {
            const std::filesystem::path lib_path(info.dli_fname);                                // Full path to the library
            return lib_path.parent_path().string() + std::filesystem::path::preferred_separator; // Return the directory without the library name
        } else {
            std::cerr << "Could not retrieve library location (dladdr)" << std::endl;
            return "";
        }
#elif defined(_WIN32)
        HMODULE hModule = nullptr; // Handle to the DLL
        char    path[MAX_PATH];

        if (GetModuleFileNameA(hModule, path, MAX_PATH) != 0) {
            std::filesystem::path lib_path(path);                                                                   // Full path to the DLL
            return lib_path.parent_path().string() + static_cast<char>(std::filesystem::path::preferred_separator); // Add the separator
        } else {
            std::cerr << "Could not retrieve library location (GetModuleFileName)" << std::endl;
            return "";
        }
#endif
    }

    std::vector<std::string> get_files(const std::string& directory, const std::string& extension) {
        std::vector<std::string> files;
        for (const auto& entry: std::filesystem::directory_iterator(directory)) {
            if (entry.path().extension() == extension || extension == "*" || extension == "*.*" || extension.empty()) {
                files.push_back(entry.path().string());
            }
        }
        return files;
    }

    /**
     * get an int value from an argument formated like e.g this "value=23"
     * @param argument argument formated like e.g this "value=23"
     * @return
     */
    int get_int_from_argument(const std::string& argument) {
        const std::size_t pos = argument.find('=');
        if (pos == std::string::npos) {
            throw std::invalid_argument("no '=' character found in argument.");
        }
        const std::string valueStr = argument.substr(pos + 1);
        return std::stoi(valueStr);
    }

    /**
     * get a string value from an argument formated like e.g this "value=twentythree"
     * @param argument argument formated like e.g this "value=twentythree"
     * @return
     */
    std::string get_string_from_argument(const std::string& argument) {
        const std::size_t pos = argument.find('=');
        if (pos == std::string::npos) {
            throw std::invalid_argument("no '=' character found in argument.");
        }
        std::string valueStr = argument.substr(pos + 1);
        return valueStr;
    }

    std::string timestamp() {
        const auto         now   = std::chrono::system_clock::now();
        const auto         ms    = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        const auto         timer = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&timer), "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

    void audio(const int  input_channels,
               const int  output_channels,
               const int  sample_rate,
               const int  buffer_size,
               const int  input_device_id,
               const int  output_device_id,
               const bool threaded) {
        // TODO maybe add option to choose driver e.g SDL, PORTAUDIO, …
        //      similar to renderer parameter in `size()` with RENDERER_OPENGL_3_3_CORE, …
        if (is_initialized()) {
            warning("`audio()` must be called before or within `settings()`.");
            return;
        }
        DISABLE_WARNING_PUSH
        DISABLE_WARNING_DEPRECATED
        // ReSharper disable CppDeprecatedEntity
        umfeld::enable_audio           = true;
        umfeld::audio_input_device_id  = input_device_id;
        umfeld::audio_input_channels   = input_channels;
        umfeld::audio_output_device_id = output_device_id;
        umfeld::audio_output_channels  = output_channels;
        umfeld::audio_buffer_size      = buffer_size;
        umfeld::audio_sample_rate      = sample_rate;
        umfeld::run_audio_in_thread    = threaded;
        // ReSharper restore CppDeprecatedEntity
        DISABLE_WARNING_POP
    }

    void audio(const int          input_channels,
               const int          output_channels,
               const int          sample_rate,
               const int          buffer_size,
               const std::string& input_device_name,
               const std::string& output_device_name,
               const bool         threaded) {
        if (is_initialized()) {
            warning("`audio()` must be called before or within `settings()`.");
            return;
        }
        DISABLE_WARNING_PUSH
        DISABLE_WARNING_DEPRECATED
        // ReSharper disable CppDeprecatedEntity
        umfeld::enable_audio             = true;
        umfeld::audio_input_device_id    = AUDIO_DEVICE_FIND_BY_NAME;
        umfeld::audio_input_device_name  = input_device_name;
        umfeld::audio_input_channels     = input_channels;
        umfeld::audio_output_device_id   = AUDIO_DEVICE_FIND_BY_NAME;
        umfeld::audio_output_device_name = output_device_name;
        umfeld::audio_output_channels    = output_channels;
        umfeld::audio_buffer_size        = buffer_size;
        umfeld::audio_sample_rate        = sample_rate;
        umfeld::run_audio_in_thread      = threaded;
        // ReSharper restore CppDeprecatedEntity
        DISABLE_WARNING_POP
    }

    void audio(const AudioUnitInfo& info) {
        if (is_initialized()) {
            warning("`audio()` must be called before or within `settings()`.");
            return;
        }
        DISABLE_WARNING_PUSH
        DISABLE_WARNING_DEPRECATED
        // ReSharper disable CppDeprecatedEntity
        umfeld::enable_audio             = true;
        umfeld::audio_input_device_id    = info.input_device_id;
        umfeld::audio_input_device_id    = info.input_device_id;
        umfeld::audio_input_device_name  = info.input_device_name;
        umfeld::audio_input_channels     = info.input_channels;
        umfeld::audio_output_device_id   = info.output_device_id;
        umfeld::audio_output_device_name = info.output_device_name;
        umfeld::audio_output_channels    = info.output_channels;
        umfeld::audio_buffer_size        = info.buffer_size;
        umfeld::audio_sample_rate        = info.sample_rate;
        umfeld::run_audio_in_thread      = info.threaded;
        // ReSharper restore CppDeprecatedEntity
        DISABLE_WARNING_POP
    }

    void audio_start(PAudio* device) {
        if (device == nullptr) {
            subsystem_audio->start(audio_device);
        } else {
            subsystem_audio->start(device);
        }
    }

    void audio_stop(PAudio* device) {
        if (device == nullptr) {
            subsystem_audio->stop(audio_device);
        } else {
            subsystem_audio->stop(device);
        }
    }

    uint32_t get_audio_sample_rate() {
        return audio_device != nullptr ? audio_device->sample_rate : 0;
    }

    int8_t get_audio_input_channels() {
        return audio_device != nullptr ? audio_device->input_channels : 0;
    }

    int8_t get_audio_output_channels() {
        return audio_device != nullptr ? audio_device->output_channels : 0;
    }

    uint32_t get_audio_buffer_size() {
        return audio_device != nullptr ? audio_device->buffer_size : 0;
    }

    std::vector<Vertex> loadOBJ_with_material(const std::string& filename) {
        tinyobj::ObjReader       reader;
        tinyobj::ObjReaderConfig config;
        config.triangulate = true;

        if (!reader.ParseFromFile(filename, config)) {
            std::cerr << "Failed to load OBJ: " << reader.Error() << std::endl;
            return {};
        }

        std::vector<Vertex> vertices;

        const auto& attrib    = reader.GetAttrib();
        const auto& shapes    = reader.GetShapes();
        const auto& materials = reader.GetMaterials();

        for (const auto& shape: shapes) {
            size_t face_index = 0; // Face counter (reset per shape)

            for (size_t i = 0; i < shape.mesh.indices.size(); i++) {
                const auto& index = shape.mesh.indices[i];
                Vertex      vertex;

                // Vertex position
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                    1.0f};

                // Texture coordinate (if available)
                if (index.texcoord_index >= 0) {
                    vertex.tex_coord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1] // Flip Y-axis
                        ,
                        0.0f};
                } else {
                    vertex.tex_coord = {0.0f, 0.0f, 0.0f}; // No texture coordinates
                }

                // Normals
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                    0.0};

                // Get correct material index per face
                if (i % 3 == 0) { // Every 3 vertices = new face
                    face_index++;
                }
                int material_id = face_index < shape.mesh.material_ids.size() ? shape.mesh.material_ids[face_index - 1] : -1;

                if (material_id >= 0 && material_id < materials.size()) {
                    const auto& material = materials[material_id];

                    // Assign diffuse color
                    vertex.color = glm::vec4(
                        material.diffuse[0], // R
                        material.diffuse[1], // G
                        material.diffuse[2], // B
                        1.0f                 // A (full opacity)
                    );
                } else {
                    // default color white
                    vertex.color = glm::vec4(1.0f);
                }

                vertices.push_back(vertex);
            }
        }
        return vertices;
    }

    std::vector<Vertex> loadOBJ_no_material(const std::string& filename) {
        tinyobj::ObjReader       reader;
        tinyobj::ObjReaderConfig config;
        config.triangulate = true; // Ensure we only get recorded_triangles

        if (!reader.ParseFromFile(filename, config)) {
            if (!reader.Error().empty()) {
                std::cerr << "OBJ Loader Error: " << reader.Error() << std::endl;
            }
            return {};
        }

        std::vector<Vertex> vertices;

        if (!reader.Warning().empty()) {
            std::cout << "OBJ Loader Warning: " << reader.Warning() << std::endl;
        }

        const tinyobj::attrib_t&             attrib = reader.GetAttrib();
        const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
        // const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

        // Loop over shapes
        for (const auto& shape: shapes) {
            for (const auto& index: shape.mesh.indices) {
                const tinyobj::real_t vx = attrib.vertices[3 * index.vertex_index + 0];
                const tinyobj::real_t vy = attrib.vertices[3 * index.vertex_index + 1];
                const tinyobj::real_t vz = attrib.vertices[3 * index.vertex_index + 2];
                // TODO add normals once Vertex is upgraded
                // const tinyobj::real_t nx = attrib.normals[3 * index.normal_index + 0];
                // const tinyobj::real_t ny = attrib.normals[3 * index.normal_index + 1];
                // const tinyobj::real_t nz = attrib.normals[3 * index.normal_index + 2];
                const tinyobj::real_t tx = attrib.texcoords[2 * index.texcoord_index + 0];
                const tinyobj::real_t ty = attrib.texcoords[2 * index.texcoord_index + 1];
                // Optional: vertex colors
                const tinyobj::real_t red   = attrib.colors[3 * index.vertex_index + 0];
                const tinyobj::real_t green = attrib.colors[3 * index.vertex_index + 1];
                const tinyobj::real_t blue  = attrib.colors[3 * index.vertex_index + 2];

                vertices.emplace_back(glm::vec3(vx, vy, vz),
                                      glm::vec4(red, green, blue, 1.0f),
                                      glm::vec3(tx, ty, 0.0f));
            }
        }
        return vertices;
    }

    std::vector<Vertex> loadOBJ(const std::string& file, const bool material) {
        const std::string absolute_path = resolve_data_path(file);
        if (!file_exists(absolute_path)) {
            error("loadOBJ() failed! file not found: '", file, "'. the 'sketchPath()' is currently set to '", sketchPath(), "'. looking for file at: '", absolute_path, "'");
            return {};
        }
        return material ? loadOBJ_with_material(absolute_path) : loadOBJ_no_material(absolute_path);
    }

    Sampler* loadSample(const std::string& file, const bool resample_to_audio_device) {
        const std::string absolute_path = resolve_data_path(file);
        if (!file_exists(absolute_path)) {
            error("loadSample() failed! file not found: '", file, "'. the 'sketchPath()' is currently set to '", sketchPath(), "'. looking for file at: '", absolute_path, "'");
            return nullptr;
        }

        unsigned int          channels;
        unsigned int          sample_rate;
        drwav_uint64          length;
        float*                sample_buffer        = AudioFileReader::load(absolute_path, channels, sample_rate, length);
        static constexpr bool print_debug_messages = false;
        if (print_debug_messages) {
            console("LOADED SAMPLE INFO");
            console("channels         : ", channels);
            console("audio_sample_rate: ", sample_rate);
            console("length           : ", length);
            console("size             : ", channels * length);
        }
        if (channels > 1) {
            warning("only mono samples are supported for sampler. using first channel only.");
            const auto single_buffer = new float[length];
            for (int i = 0; i < length; i++) {
                single_buffer[i] = sample_buffer[i * channels];
            }
            delete[] sample_buffer;
            sample_buffer = single_buffer;
        }
        uint32_t _sample_rate;
        if (audio_device != nullptr) {
            _sample_rate = audio_device->sample_rate;
        } else {
            warning_in_function("no audio device available. using sample rate of the sample: ", sample_rate, " Hz.");
            _sample_rate = sample_rate;
        }
        const auto sampler = new Sampler(_sample_rate);
        sampler->set_buffer(sample_buffer, static_cast<int32_t>(length), false);
        if (resample_to_audio_device && _sample_rate != sample_rate) {
            console_in_function("resampling sample from ", sample_rate, " Hz to ", _sample_rate, " Hz.");
            sampler->resample(sample_rate, _sample_rate);
        }
        return sampler;
    }
} // namespace umfeld
