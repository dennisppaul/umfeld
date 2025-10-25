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

#include <iomanip>
#include <string>
#include <random>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <curl/curl.h>
#include <algorithm>
#include <regex>
#include <sstream>
#include <iterator>
#include <cctype>
#include <iomanip>

#include <SDL3/SDL.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "tinyfiledialogs.h"

#include "Umfeld.h"
#include "SimplexNoise.h"
#include "UmfeldFunctions.h"
#include "UmfeldFunctionsAdditional.h"

namespace umfeld {

    using namespace std::chrono;

    color_t color(const float brightness) {
        return color(brightness, brightness, brightness, 1);
    }

    color_t color(const float brightness, const float alpha) {
        return color(brightness, brightness, brightness, alpha);
    }

    color_t color(const float r, const float g, const float b) {
        return color(r, g, b, 1);
    }

    color_t color(const float r, const float g, const float b, const float a) {
        return static_cast<uint32_t>(a * 255) << 24 |
               static_cast<uint32_t>(b * 255) << 16 |
               static_cast<uint32_t>(g * 255) << 8 |
               static_cast<uint32_t>(r * 255);
    }

    color_t color_8(const uint8_t gray) {
        return gray << 24 |
               gray << 16 |
               gray << 8 |
               255;
    }

    color_t color_8(const uint8_t gray, const uint8_t alpha) {
        return gray << 24 |
               gray << 16 |
               gray << 8 |
               alpha;
    }

    color_t color_8(const uint8_t r, const uint8_t g, const uint8_t b) {
        return 255 << 24 |
               b << 16 |
               g << 8 |
               r;
    }

    color_t color_8(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
        return a << 24 |
               b << 16 |
               g << 8 |
               r;
    }

    float alpha(const color_t color) {
        return static_cast<float>((color & 0x000000FF) >> 0) / 255.0f;
    }

    float blue(const color_t color) {
        return static_cast<float>((color & 0x0000FF00) >> 8) / 255.0f;
    }

    float green(const color_t color) {
        return static_cast<float>((color & 0x00FF0000) >> 16) / 255.0f;
    }

    float red(const color_t color) {
        return static_cast<float>((color & 0xFF000000) >> 24) / 255.0f;
    }

    void rgb_to_hsb(const float r, const float g, const float b, float& h, float& s, float& v) {
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

    void hsb_to_rgb(const float h, const float s, const float v, float& r, float& g, float& b) {
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

    float brightness(const uint32_t color) {
        const float r = red(color);
        const float g = green(color);
        const float b = blue(color);
        float       h, s, v;
        rgb_to_hsb(r, g, b, h, s, v);
        return v;
    }

    float hue(const uint32_t color) {
        const float r = red(color);
        const float g = green(color);
        const float b = blue(color);
        float       h, s, v;
        rgb_to_hsb(r, g, b, h, s, v);
        return h;
    }

    float saturation(const uint32_t color) {
        const float r = red(color);
        const float g = green(color);
        const float b = blue(color);
        float       h, s, v;
        rgb_to_hsb(r, g, b, h, s, v);
        return s;
    }

    color_t lerpColor(const color_t c1, const color_t c2, float amt) {
        amt = std::clamp(amt, 0.0f, 1.0f);

        const float r = red(c1) * (1 - amt) + red(c2) * amt;
        const float g = green(c1) * (1 - amt) + green(c2) * amt;
        const float b = blue(c1) * (1 - amt) + blue(c2) * amt;
        const float a = alpha(c1) * (1 - amt) + alpha(c2) * amt;

        const color_t ri = static_cast<color_t>(r * 255.0f);
        const color_t gi = static_cast<color_t>(g * 255.0f);
        const color_t bi = static_cast<color_t>(b * 255.0f);
        const color_t ai = static_cast<color_t>(a * 255.0f);

        return ai << 24 | bi << 16 | gi << 8 | ri;
    }

    float degrees(const float radians) {
        return static_cast<float>(radians * 180.0f / M_PI);
    }

    float radians(const float degrees) {
        return M_PI * degrees / 180.0f;
    }

    std::string join(const std::vector<std::string>& strings, const std::string& separator) {
        std::string result;
        for (size_t i = 0; i < strings.size(); ++i) {
            result += strings[i];
            if (i < strings.size() - 1) {
                result += separator;
            }
        }
        return result;
    }

    // float map(const float value,
    //           const float start0,
    //           const float stop0,
    //           const float start1,
    //           const float stop1) {
    //     const float a = value - start0;
    //     const float b = stop0 - start0;
    //     const float c = stop1 - start1;
    //     const float d = a / b;
    //     const float e = d * c;
    //     return e + start1;
    // }

    template<typename T>
    T mapT(const T value,
           const T start0,
           const T stop0,
           const T start1,
           const T stop1) {
        const T a = value - start0;
        const T b = stop0 - start0;
        const T c = stop1 - start1;
        const T d = a / b;
        const T e = d * c;
        return e + start1;
    }

    float map(const float value, const float start0, const float stop0, const float start1, const float stop1) {
        return mapT<float>(value, start0, stop0, start1, stop1);
    }

    std::vector<std::string> match(const std::string& text, const std::regex& regexp) {
        std::vector<std::string> groups;
        std::smatch              match;
        if (std::regex_search(text, match, regexp)) {
            for (size_t i = 1; i < match.size(); ++i) {
                groups.push_back(match[i]);
            }
        }
        return groups;
    }

    std::vector<std::vector<std::string>> matchAll(const std::string& text, const std::regex& regexp) {
        std::vector<std::vector<std::string>> matches;
        std::sregex_iterator                  begin(text.begin(), text.end(), regexp), end;
        for (auto i = begin; i != end; ++i) {
            std::vector<std::string> matchGroup;
            matchGroup.push_back(i->str()); // Full match

            for (size_t j = 1; j < i->size(); ++j) { // Capturing groups
                matchGroup.push_back((*i)[j]);
            }

            matches.push_back(matchGroup);
        }
        return matches;
    }

    std::string nf(const float num, const int digits) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(digits) << num;
        return out.str();
    }

    std::string nf(const double num, const int digits) {
        return nf(static_cast<float>(num), digits);
    }

    std::string nf(const float num, const int left, const int right) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(right) << num;
        std::string numStr = out.str(); // e.g. " 12.3" or "12.3"

        // find the dot (should always succeed now)
        const auto        dotPos      = numStr.find('.');
        std::string       integerPart = numStr.substr(0, dotPos);
        const std::string fractional  = numStr.substr(dotPos); // ".3"

        // pad the integer part with leading zeros
        if (static_cast<int>(integerPart.length()) < left) {
            integerPart.insert(0, left - integerPart.length(), '0');
        }
        return integerPart + fractional;
    }

    std::string nfc(const int num) {
        std::ostringstream out;
        out.imbue(std::locale("")); // Use system's locale for thousands separator
        out << num;
        return out.str();
    }

    std::string nfc(const float num, const int right) {
        std::ostringstream out;
        out.imbue(std::locale(""));
        out << std::fixed << std::setprecision(right) << num;
        return out.str();
    }

    std::string nfc(const float number) {
        std::stringstream ss;
        ss.imbue(std::locale(""));
        ss << std::fixed << number;
        return ss.str();
    }

    std::string nfp(const float num, const int digits) {
        std::ostringstream out;
        if (num >= 0) {
            out << "+";
        }
        out << std::fixed << std::setprecision(digits) << num;
        return out.str();
    }

    std::string nfp(const float num, const int left, const int right) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(right);
        if (num >= 0) {
            out << "+";
        }
        out << std::setw(left + right + 1) << std::setfill('0') << num;
        return out.str();
    }

    std::string nfs(const float num, const int left, const int right) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(right);                  // Ensure right decimal places
        out << std::setw(left + right + 1) << std::setfill('0') << num; // Ensure left digits
        return out.str();
    }

    std::string nfs(const float num, const int digits) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(digits) << num;
        return out.str();
    }

    std::string nfs(const int num, const int digits) {
        std::ostringstream out;
        out << std::setw(digits) << std::setfill('0') << num;
        return out.str();
    }

    std::string formatNumber(const int num, const int digits) {
        std::ostringstream out;
        out << std::setw(digits) << std::setfill('0') << num;
        return out.str();
    }

    static uint32_t seed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    void randomSeed(const unsigned int seed) {
        umfeld::seed = seed;
    }

    uint64_t wyrand_state = 0xa5a5a5a5a5a5a5a5ULL;

    static uint64_t wyrand() {
        wyrand_state += 0xa0761d6478bd642f;
        uint64_t result = wyrand_state;
        result ^= result >> 32;
        result *= 0xe7037ed1a0b428db;
        return result;
    }

    static float wyrand01() {
        return (wyrand() >> 40) / static_cast<float>(1ULL << 24); // top 24 bits
    }

    static float fastRandom01() {
        seed = 1664525 * seed + 1013904223;                          // LCG parameters
        return (seed & 0x00FFFFFF) / static_cast<float>(0x01000000); // keep only lower 24 bits for better float resolution
    }

    static float xorshiftRandom01() {
        uint32_t x = seed;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        seed = x;
        return (x & 0x00FFFFFF) / static_cast<float>(0x01000000);
    }

    float random(const float max) {
        return random(0.0f, max);
    }

    static uint64_t pcg_state = 0x853c49e6748fea9bULL;

    uint32_t pcg32() {
        const uint64_t oldstate   = pcg_state;
        pcg_state                 = oldstate * 6364136223846793005ULL + 1;
        const uint32_t xorshifted = static_cast<uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);
        const uint32_t rot        = oldstate >> 59u;
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    float pcgRandom01() {
        return (pcg32() & 0x00FFFFFF) / static_cast<float>(0x01000000);
    }

    static float normalized_random() {
        // if (get_random_mode() == FAST) {
        //     return fastRandom01();
        // }
        const Random mode = get_random_mode();
        switch (mode) {
            case FAST:
                return fastRandom01();
            case XOR_SHIFT_32:
                return xorshiftRandom01();
            case PCG:
                return pcgRandom01();
            case WYRAND:
                return wyrand01();
            default:
                return fastRandom01();
        }
    }

    float random(const float min, const float max) {
        const float range = max - min;
        return min + range * normalized_random();
    }

    float randomGaussian() { // Box-Muller
        static bool  hasSpare = false;
        static float spare;

        if (hasSpare) {
            hasSpare = false;
            return spare;
        }

        float u, v, s;
        do {
            u = fastRandom01() * 2.0f - 1.0f; // in [-1, 1)
            v = fastRandom01() * 2.0f - 1.0f;
            s = u * u + v * v;
        } while (s >= 1.0f || s == 0.0f);

        s        = std::sqrt(-2.0f * std::log(s) / s);
        spare    = v * s;
        hasSpare = true;
        return u * s;
    }

    void size(const int width, const int height, const Renderer renderer) {
        if (is_initialized()) {
            warning("`size()` must be called before or within `settings()`.");
            return;
        }
        umfeld::enable_graphics = true;
        umfeld::width           = width;
        umfeld::height          = height;
        umfeld::renderer        = renderer;
    }

    std::string nf(const int num, const int digits) {
        std::ostringstream oss;
        oss << std::setw(digits) << std::setfill('0') << num;
        return oss.str();
    }

    static int fNoiseSeed = static_cast<int>(std::time(nullptr));

    void noiseSeed(const int seed) {
        fNoiseSeed = seed;
    }

    float noise(const float x) {
        return SimplexNoise::noise(x);
    }

    float noise(const float x, const float y) {
        return SimplexNoise::noise(x, y);
    }

    float noise(const float x, const float y, const float z) {
        return SimplexNoise::noise(x, y, z);
    }

    static std::string custom_sketch_path;

    void set_sketch_path(const std::string& path) {
        custom_sketch_path = path;
    }

#ifndef UMFELD_USE_NATIVE_SKETCH_PATH
#define USE_SDL_SKETCH_PATH
#endif
    std::string sketchPath() {
        if (custom_sketch_path.empty() == false) {
            return custom_sketch_path;
        }
#ifdef USE_SDL_SKETCH_PATH
        return SDL_GetBasePath();
#else
        return sketchPath_impl();
#endif
    }

#ifndef USE_SDL_SKETCH_PATH
#if defined(SYSTEM_WINDOWS)
#include <windows.h>
    std::string sketchPath_impl() {
        std::vector<char> buffer(1024);
        DWORD             length = GetModuleFileNameA(NULL, buffer.data(), buffer.size());

        while (length == buffer.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            // Increase buffer size if the path was truncated
            buffer.resize(buffer.size() * 2);
            length = GetModuleFileNameA(NULL, buffer.data(), buffer.size());
        }

        if (length == 0) {
            // GetModuleFileName failed
            warning("Error retrieving path, error code: " << GetLastError() << std::endl;
            return std::string();
        }

        std::filesystem::path exePath(buffer.data());
        std::filesystem::path dirPath = exePath.parent_path();
        return dirPath.string() + "/";
    }
#elif defined(SYSTEM_UNIX)
#include <unistd.h>

    std::string sketchPath_impl() {
        std::vector<char> buf(1024);
        ssize_t           len = readlink("/proc/self/exe", buf.data(), buf.size());
        if (len != -1) {
            buf[len] = '\0'; // Null-terminate the string
            std::filesystem::path exePath(buf.data());
            std::filesystem::path dirPath = exePath.parent_path();
            return dirPath.string() + std::string("/");
        } else {
            warning("Error retrieving path" << std::endl;
            return std::string(); // Return an empty string in case of error
        }
    }

#elif defined(SYSTEM_MACOS)
#include <mach-o/dyld.h>

    std::string sketchPath_impl() {
        uint32_t          size = 1024;
        std::vector<char> buf(size);
        if (_NSGetExecutablePath(buf.data(), &size) == 0) {
            const std::filesystem::path exePath(buf.data());
            const std::filesystem::path dirPath = exePath.parent_path();
            return dirPath.string() + std::string("/");
        } else {
            warning("Error retrieving path" << std::endl;
            return {}; // Return an empty string in case of error
        }
    }
#else

    std::string sketchPath_impl() {
        std::filesystem::path currentPath = std::filesystem::current_path();
        return currentPath.string() + std::string("/");
    }
#endif
#endif

    std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> result;
        size_t                   start = 0, end;
        while ((end = str.find(delimiter, start)) != std::string::npos) {
            result.push_back(str.substr(start, end - start));
            start = end + delimiter.length();
        }
        if (start < str.length()) {
            result.push_back(str.substr(start));
        }
        return result;
    }

    std::vector<std::string> splitTokens(const std::string& str, const std::string& tokens) {
        std::vector<std::string> result;
        size_t                   start = 0, end;
        while ((end = str.find_first_of(tokens, start)) != std::string::npos) {
            if (end != start) {
                result.push_back(str.substr(start, end - start));
            }
            start = end + 1;
        }
        if (start < str.length()) {
            result.push_back(str.substr(start));
        }
        return result;
    }

    std::string trim(const std::string& str) {
        const size_t first = str.find_first_not_of(" \t\n\r\f\v");
        if (first == std::string::npos) {
            return "";
        }
        const size_t last = str.find_last_not_of(" \t\n\r\f\v");
        return str.substr(first, last - first + 1);
    }

    PGraphics* createGraphics(const int width, const int height, const int renderer) {
        if (subsystem_graphics == nullptr) {
            return nullptr;
        }

        if (renderer != DEFAULT) {
            warning_in_function_once("other renderers are not supported yet");
        }

        const auto graphics = subsystem_graphics->create_native_graphics(true);
        graphics->init(nullptr, width, height);
        return graphics;

        // NOTE `create_native_graphics` should handle all this:
        //
        // if (renderer == DEFAULT) {
        //     renderer = RENDERER_OPEN_GL; // TODO try to deduce renderer from (sub-)system or main graphics `g`
        // }
        //
        // switch (renderer) {
        //     case RENDERER_OPENGL_2_0: {
        //         const auto graphics = new PGraphicsOpenGL_2_0(true);
        //         graphics->init(nullptr, width, height, 0, false);
        //         return graphics;
        //     }
        //     case RENDERER_OPENGL_3_3_CORE:
        //     default: {
        //         const auto graphics = new PGraphicsOpenGL_3_3_core(true);
        //         graphics->init(nullptr, width, height, 0, false);
        //         return graphics;
        //     }
        // }
    }

    PAudio* createAudio(const AudioUnitInfo* device_info) {
        if (subsystem_audio == nullptr) {
            return nullptr;
        }
        return subsystem_audio->create_audio(device_info);
    }

    std::string loadString(const std::string& file_path) {
        const std::vector<uint8_t> bytes = loadBytes(file_path);
        if (bytes.empty()) {
            warning("Failed to read string from: ", file_path);
            return {};
        }
        return std::string(bytes.begin(), bytes.end());
    }

    std::vector<std::string> loadStrings(const std::string& file_path) {
        const std::vector<uint8_t> bytes = loadBytes(file_path);
        if (bytes.empty()) {
            warning("Failed to read lines from: ", file_path);
            return {};
        }

        std::vector<std::string> lines;
        std::istringstream       stream(std::string(bytes.begin(), bytes.end()));
        std::string              line;

        while (std::getline(stream, line)) {
            lines.push_back(line);
        }

        return lines;
    }

    void saveString(const std::string& file_path, const std::string& content, const bool append) {
        std::ofstream file(file_path, append ? std::ios::app : std::ios::trunc);
        if (file) {
            file << content;
        } else {
            warning("Failed to write to file: ", file_path);
        }
    }


    bool saveStrings(const std::string& file_path, const std::vector<std::string>& lines, const bool append) {
        std::ofstream file(file_path, append ? std::ios::app : std::ios::trunc);
        if (!file) {
            warning("Failed to write to file: ", file_path);
            return false;
        }
        for (const auto& line: lines) {
            file << line << '\n';
        }
        return true;
    }

    static size_t WriteCallback(void* contents, const size_t size, const size_t nmemb, void* userp) {
        const size_t totalSize = size * nmemb;
        auto*        buffer    = static_cast<std::vector<uint8_t>*>(userp);
        buffer->insert(buffer->end(), static_cast<uint8_t*>(contents), static_cast<uint8_t*>(contents) + totalSize);
        return totalSize;
    }

    static std::vector<std::string> supported_URL_protocols;
    static bool                     print_supported_protocols = false;

    static void initialize_supported_protocols() {
        if (supported_URL_protocols.empty()) {
            const curl_version_info_data* info = curl_version_info(CURLVERSION_NOW);
            if (info && info->protocols) {
                for (const char* const* proto = info->protocols; *proto != nullptr; ++proto) {
                    supported_URL_protocols.emplace_back(*proto);
                }
            }
            if (print_supported_protocols) {
                console_n("supported curl URL protocols: ");
                for (const auto& proto: supported_URL_protocols) {
                    console_c(proto, " ");
                }
                console_c("\n");
            }
        }
    }

    static std::vector<uint8_t> decode_base64(const std::string& input) {
        static const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        auto is_base64 = [](const unsigned char c) {
            return std::isalnum(c) || c == '+' || c == '/';
        };

        std::vector<uint8_t> output;
        int                  val = 0, valb = -8;
        for (const unsigned char c: input) {
            if (!is_base64(c)) {
                break;
            }
            val = (val << 6) + base64_chars.find(c);
            valb += 6;
            if (valb >= 0) {
                output.push_back((val >> valb) & 0xFF);
                valb -= 8;
            }
        }
        return output;
    }

    static std::string decode_percent_encoding(const std::string& input) {
        std::ostringstream out;
        for (size_t i = 0; i < input.length(); ++i) {
            if (input[i] == '%' && i + 2 < input.length()) {
                int                val;
                std::istringstream iss(input.substr(i + 1, 2));
                if (iss >> std::hex >> val) {
                    out << static_cast<char>(val);
                    i += 2;
                }
            } else if (input[i] == '+') {
                out << ' ';
            } else {
                out << input[i];
            }
        }
        return out.str();
    }

    std::vector<uint8_t> loadBytesFromURL(const std::string& url) {
        initialize_supported_protocols();

        // check if protocol is supported
        const auto schemeEnd = url.find("://");
        if (schemeEnd != std::string::npos) {
            const std::string scheme = url.substr(0, schemeEnd);
            if (std::find(supported_URL_protocols.begin(), supported_URL_protocols.end(), scheme) == supported_URL_protocols.end()) {
                warning("Protocol not supported by libcurl: ", scheme);
                return {};
            }
        }

        // download via libcurl
        CURL* curl = curl_easy_init();
        if (!curl) {
            return {};
        }

        std::vector<uint8_t> data;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        const CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            warning("CURL error: ", curl_easy_strerror(res));
            return {};
        }

        return data;
    }

    std::vector<uint8_t> loadBytesFromFile(const std::string& file_path) {
        std::ifstream file(file_path, std::ios::binary);
        if (!file) {
            warning("Failed to read file: ", file_path);
            return {};
        }
        return std::vector<uint8_t>(std::istreambuf_iterator(file), {});
    }

    static bool is_protocol_supported(const std::string& file_path) {
        initialize_supported_protocols();
        const auto schemeEnd = file_path.find("://");
        if (schemeEnd != std::string::npos) {
            const std::string scheme = file_path.substr(0, schemeEnd);

            if (scheme == "file") {
                // Handle file:// specially — map to local file
#if defined(_WIN32)
                const std::string prefix = "file:///";
                if (file_path.rfind(prefix, 0) == 0) {
                    // std::string path = file_path.substr(prefix.size());
                    // std::replace(path.begin(), path.end(), '/', '\\');
                    return true;
                }
#else
                // std::string path = file_path.substr(7); // strip file://
                return true;
#endif
            }

            // check if scheme is supported by libcurl
            if (std::find(supported_URL_protocols.begin(), supported_URL_protocols.end(), scheme) != supported_URL_protocols.end()) {
                return true;
            }

            warning("Unsupported protocol: '", scheme, "' assuing local file path.");
            return false;
        }
        return false;
    }

    std::vector<uint8_t> loadBytes(const std::string& file_path) {
        initialize_supported_protocols();

        // URI: check if file path is a data URI (base64 encoded)
        if (file_path.rfind("data:", 0) == 0) {
            const auto commaPos = file_path.find(',');
            if (commaPos == std::string::npos) {
                warning("Malformed data URI.");
                return {};
            }

            const std::string meta = file_path.substr(5, commaPos - 5); // skip "data:"
            const std::string data = file_path.substr(commaPos + 1);

            if (meta.find(";base64") != std::string::npos) {
                return decode_base64(data);
            } else {
                std::string decoded = decode_percent_encoding(data);
                return std::vector<uint8_t>(decoded.begin(), decoded.end());
            }
        }

        // URL: check if file path starts with a protocol (e.g., http://, https://, ftp://, etc.) extract scheme (before ://)
        const auto schemeEnd = file_path.find("://");
        if (schemeEnd != std::string::npos) {
            const std::string scheme = file_path.substr(0, schemeEnd);

            if (scheme == "file") {
                // Handle file:// specially — map to local file
#if defined(_WIN32)
                const std::string prefix = "file:///";
                if (file_path.rfind(prefix, 0) == 0) {
                    std::string path = file_path.substr(prefix.size());
                    std::replace(path.begin(), path.end(), '/', '\\');
                    return loadBytesFromFile(path);
                }
#else
                const std::string path = file_path.substr(7); // strip file://
                return loadBytesFromFile(path);
#endif
            }

            // check if scheme is supported by libcurl
            if (std::find(supported_URL_protocols.begin(), supported_URL_protocols.end(), scheme) != supported_URL_protocols.end()) {
                return loadBytesFromURL(file_path);
            }

            warning("Unsupported protocol: '", scheme, "' assuing local file path.");
            return {};
        }

        // LOCAL: no scheme → assume it's a local path
        const std::string absolute_path = resolve_data_path(file_path);
        return loadBytesFromFile(absolute_path);
    }

    bool saveBytes(const std::string& file_path, const std::vector<uint8_t>& data, const bool append) {
        std::ofstream file(file_path, std::ios::binary | (append ? std::ios::app : std::ios::trunc));
        if (!file) {
            warning("Failed to write to file: ", file_path);
            return false;
        }
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    }

    // Function to get the current system time as a std::tm struct
    static std::tm getCurrentTime() {
        const std::time_t t = std::time(nullptr);
        return *std::localtime(&t);
    }

    // Returns the current day (1-31)
    int day() {
        return getCurrentTime().tm_mday;
    }

    // Returns the current hour (0-23)
    int hour() {
        return getCurrentTime().tm_hour;
    }

    // Returns the number of milliseconds since the program started
    long long millis() {
        static auto start_time = steady_clock::now();
        return duration_cast<milliseconds>(steady_clock::now() - start_time).count();
    }

    // Returns the current minute (0-59)
    int minute() {
        return getCurrentTime().tm_min;
    }

    // Returns the current month (1-12)
    int month() {
        return getCurrentTime().tm_mon + 1; // tm_mon is 0-based
    }

    // Returns the current second (0-59)
    int second() {
        return getCurrentTime().tm_sec;
    }

    // Returns the current year (e.g., 2025)
    int year() {
        return getCurrentTime().tm_year + 1900; // tm_year is years since 1900
    }

    void cursor() { SDL_ShowCursor(); }

    // void cursor(PImage* img, int x, int y) {
    //     SDL_Cursor * SDLCALL SDL_CreateColorCursor(SDL_Surface *surface,
    //                                                       int hot_x,
    //                                                       int hot_y);
    //     SDL_CreateCursor(const Uint8 *data,
    //                                                  const Uint8 *mask,
    //                                                  int w, int h, int hot_x,
    //                                                  int hot_y);
    //     SDL_SetCursor(SDL_Cursor *cursor);
    //     SDL_DestroyCursor();
    // }

    void noCursor() { SDL_HideCursor(); }

    std::string resolve_data_path(const std::string& path) {
        const std::filesystem::path p(path);

        if (p.is_absolute()) {
            return path;
        }

        // treat as relative to "data" directory next to executable
        return sketchPath() + (std::filesystem::path(UMFELD_DATA_PATH) / p).string();
    }

    PFont* loadFont(const std::string& file, const float size) {
        // NOTE if the file starts with "http://", "https://",  etcetera assume it's a URL
        if (is_protocol_supported(file)) {
            const std::vector<uint8_t> data = loadBytes(file);
            return new PFont(data.data(), data.size(), size);
        }

        const std::string absolute_path = resolve_data_path(file);
        if (!file_exists(absolute_path)) {
            error("loadImage() failed! file not found: '", file, "'. the 'sketchPath()' is currently set to '", sketchPath(), "'. looking for file at: '", absolute_path, "'");
            return nullptr;
        }
        return new PFont(absolute_path, size);

        // const std::string absolute_path = resolve_data_path(file);
        // if (!file_exists(absolute_path)) {
        //     error("loadFont() failed! file not found: '", file, "'. the 'sketchPath()' is currently set to '", sketchPath(), "'. looking for file at: '", absolute_path, "'");
        //     return nullptr;
        // }
        // return new PFont(absolute_path, size);
    }

    PImage* loadImage(const std::string& file) {
        // NOTE if the file starts with "http://", "https://",  etcetera assume it's a URL
        if (is_protocol_supported(file)) {
            const std::vector<uint8_t> data = loadBytes(file);
            return new PImage(data.data(), data.size());
        }

        const std::string absolute_path = resolve_data_path(file);
        if (!file_exists(absolute_path)) {
            error("loadImage() failed! file not found: '", file, "'. the 'sketchPath()' is currently set to '", sketchPath(), "'. looking for file at: '", absolute_path, "'");
            return nullptr;
        }
        return new PImage(absolute_path);
    }

    void saveImage(const PImage* image, const std::string& filename) {
        if (!image->pixels || image->width <= 0 || image->height <= 0) {
            warning("invalid PImage. not saving image.");
            return;
        }

        const int _width  = image->width;
        const int _height = image->height;

        if (ends_with(filename, ".png")) {
            stbi_write_png(filename.c_str(), _width, _height, DEFAULT_BYTES_PER_PIXELS, image->pixels, _width * DEFAULT_BYTES_PER_PIXELS);
        } else if (ends_with(filename, ".jpg")) {
            stbi_write_jpg(filename.c_str(), _width, _height, DEFAULT_BYTES_PER_PIXELS, image->pixels, save_image_jpeg_quailty);
        } else if (ends_with(filename, ".bmp")) {
            stbi_write_bmp(filename.c_str(), _width, _height, DEFAULT_BYTES_PER_PIXELS, image->pixels);
        } else if (ends_with(filename, ".tga")) {
            stbi_write_tga(filename.c_str(), _width, _height, DEFAULT_BYTES_PER_PIXELS, image->pixels);
        } else {
            warning("Unsupported file format: ", filename, ". Supported formats are: .png, .jpg, .bmp, .tga");
        }
    }

    void saveFrame(const std::string& filename) {
        if (g == nullptr) {
            return;
        }

        const int _height = g->framebuffer.height;
        const int _width  = g->framebuffer.width;

        // Allocate memory for pixel data (RGBA)
        std::vector<unsigned char> pixels;
        // glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        const bool success = g->read_framebuffer(pixels);
        if (!success) {
            warning("could not read pixel from color buffer. not saving image. try turning of anti-aliasing or offscreen rendering.");
        }

        // Flip the image vertically because OpenGL's origin is bottom-left
        std::vector<unsigned char> flippedPixels(_width * _height * DEFAULT_BYTES_PER_PIXELS);
        for (int y = 0; y < _height; ++y) {
            memcpy(&flippedPixels[(_height - 1 - y) * _width * DEFAULT_BYTES_PER_PIXELS], &pixels[y * _width * DEFAULT_BYTES_PER_PIXELS], _width * DEFAULT_BYTES_PER_PIXELS);
        }

        // save image
        if (ends_with(filename, ".png")) {
            stbi_write_png(filename.c_str(), _width, _height, DEFAULT_BYTES_PER_PIXELS, flippedPixels.data(), _width * DEFAULT_BYTES_PER_PIXELS);
        } else if (ends_with(filename, ".jpg")) {
            stbi_write_jpg(filename.c_str(), _width, _height, DEFAULT_BYTES_PER_PIXELS, flippedPixels.data(), save_image_jpeg_quailty);
        } else if (ends_with(filename, ".bmp")) {
            stbi_write_bmp(filename.c_str(), _width, _height, DEFAULT_BYTES_PER_PIXELS, flippedPixels.data());
        } else if (ends_with(filename, ".tga")) {
            stbi_write_tga(filename.c_str(), _width, _height, DEFAULT_BYTES_PER_PIXELS, flippedPixels.data());
            // } else if (ends_with(filename, ".hdr")) {
            //     stbi_write_hdr((filename).c_str(), _width, _height, DEFAULT_BYTES_PER_PIXELS, reinterpret_cast<float*>(flippedPixels.data()));
        } else {
            warning("Unsupported file format: ", filename, ". Supported formats are: .png, .jpg, .bmp, .tga");
        }
    }

    void saveFrame() {
        saveFrame(sketchPath() + "screenshot-" + nfs(frameCount, 4) + ".png");
    }

    std::string selectFolder(const std::string& prompt) {
        const char* folder_path = tinyfd_selectFolderDialog(
            prompt.c_str(),      // title
            sketchPath().c_str() // default path
        );

        if (folder_path) {
#ifdef SYSTEM_WINDOWS
            // NOTE windows needs an extra slash at the end
            return std::string(folder_path) + "/";
#else
            return folder_path;
#endif
        }
        return "";
    }

    std::string selectInput(const std::string& prompt) {
        const char* save_path = tinyfd_openFileDialog(
            prompt.c_str(),       // title
            sketchPath().c_str(), // default path and file
            0,                    // number of filter patterns
            nullptr,              // filter patterns
            nullptr,              // single filter description
            1);

        if (save_path) {
            return save_path;
        }

        return "";
    }
} // namespace umfeld