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

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <iomanip>

#include "UmfeldFunctionsAdditional.h"

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884L
#endif

namespace umfeld {

#define FLUSH_PRINT

    // ## Data

    // ### Conversion

    inline std::string binary(const int value) {
        return std::bitset<32>(value).to_string(); // 32-bit representation
    }

    inline std::string binary(const unsigned char value) {
        return std::bitset<8>(value).to_string(); // 8-bit representation
    }

    inline std::string binary(const char value) {
        return std::bitset<8>(static_cast<unsigned char>(value)).to_string();
    }

    inline std::string binary(const uint32_t color) {
        return std::bitset<32>(color).to_string(); // 32-bit RGBA representation
    }

    template<typename T>
    std::string hex(T value, const int width = sizeof(T) * 2) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(width) << static_cast<uint64_t>(value);
        return ss.str();
    }

    inline std::string hex(const char value) {
        return hex(static_cast<unsigned char>(value), 2);
    }

    inline std::string hex(const unsigned char value) {
        return hex(static_cast<int>(value), 2);
    }

    inline std::string hex(const uint32_t color) {
        return hex(color, 8);
    }

    template<typename... Args>
    std::string str(const Args&... args) {
        std::ostringstream oss;
        (oss << ... << args);
        return oss.str();
    }

    inline int unbinary(const std::string& binaryStr) {
        return std::bitset<32>(binaryStr).to_ulong();
    }

    inline int unhex(const std::string& hexStr) {
        int               value;
        std::stringstream ss;
        ss << std::hex << hexStr;
        ss >> value;
        return value;
    }

    // ### String Functions

    std::string                           join(const std::vector<std::string>& strings, const std::string& separator);
    std::vector<std::string>              match(const std::string& text, const std::regex& regexp);
    std::vector<std::vector<std::string>> matchAll(const std::string& text, const std::regex& regexp);
    std::string                           nf(float num, int digits = 2);
    std::string                           nf(double num, int digits = 2);
    std::string                           nf(float num, int left, int right);
    std::string                           nf(int num, int digits = 2);
    std::string                           nfc(int num);
    std::string                           nfc(float num, int right);
    std::string                           nfp(float num, int digits = 2);
    std::string                           nfp(float num, int left, int right);
    std::string                           nfs(float num, int left, int right);
    std::string                           nfs(float num, int digits = 2);
    std::string                           nfs(int num, int digits = 2);
    std::vector<std::string>              split(const std::string& str, const std::string& delimiter);
    std::vector<std::string>              splitTokens(const std::string& str, const std::string& tokens);
    std::string                           trim(const std::string& str);

    // ## Input

    // ### Files

    std::string              loadString(const std::string& file_path);
    std::vector<std::string> loadStrings(const std::string& file_path);
    std::vector<uint8_t>     loadBytesFromURL(const std::string& url);
    std::vector<uint8_t>     loadBytesFromFile(const std::string& file_path);
    std::vector<uint8_t>     loadBytes(const std::string& file_path);
    std::string              selectFolder(const std::string& prompt);
    std::string              selectInput(const std::string& prompt);

    // ### Time & Date

    int       day();
    int       hour();
    long long millis();
    int       minute();
    int       month();
    int       second();
    int       year();

    // ## Typography

    PFont* loadFont(const std::string& file, float size);

    // ## Rendering

    PGraphics* createGraphics(int width, int height, int renderer = DEFAULT);

    // ## Image

    // ### Loading & Displaying

    PImage* loadImage(const std::string& file);

    // ## Math

    float noise(float x);
    float noise(float x, float y);
    float noise(float x, float y, float z);
    void  noiseSeed(int seed);
    float randomGaussian();
    float random(float max = 1.0f);
    float random(float min, float max);

    // ### Random

    // ## Output

    // ### Files

    void saveString(const std::string& file_path, const std::string& content, bool append = false);
    bool saveStrings(const std::string& file_path, const std::vector<std::string>& lines, bool append = false);
    bool saveBytes(const std::string& file_path, const std::vector<uint8_t>& data, bool append = false);

    // ### Text Area

    template<typename... Args>
    void print(const Args&... args) {
        std::ostringstream os;
        ((os << to_printable(args)), ...);
        std::cout << os.str();
#ifdef FLUSH_PRINT
        std::flush(std::cout);
#endif
    }

    template<typename... Args>
    void println(const Args&... args) {
        std::ostringstream os;
        ((os << to_printable(args)), ...);
        std::cout << os.str() << std::endl;
#ifdef FLUSH_PRINT
        std::flush(std::cout);
#endif
    }

    template<typename T>
    void printArray(const std::vector<T>& vec) {
        for (size_t i = 0; i < vec.size(); ++i) {
            std::cout << "[" << i << "] " << vec[i] << std::endl;
        }
#ifdef FLUSH_PRINT
        std::flush(std::cout);
#endif
    }

    // ### Image

    void saveFrame(const std::string& filename);
    void saveFrame();

    // ## Color

    // ### Creating & Reading

    // moved to `UmfeldFunctions.h` // TODO link to header file in comment

    // ## Environment

    void size(int width, int height, Renderer renderer = RENDERER_DEFAULT);
    void cursor();
    void noCursor();

    // ## Math

    // ### Calculation

    template<typename T>
    T constrain(T value, T minVal, T maxVal) {
        return std::clamp(value, minVal, maxVal);
    }

    template<typename T>
    T lerp(T start, T stop, float amt) {
        return start + amt * (stop - start);
    }

    template<typename T, typename... Args>
    T max(T first, Args... args) {
        return std::max({first, args...});
    }

    template<typename T, typename... Args>
    T min(T first, Args... args) {
        return std::min({first, args...});
    }

    template<typename T>
    float norm(T value, T start, T stop) {
        return (stop != start) ? static_cast<float>(value - start) / (stop - start) : 0.0f;
    }

    template<typename T>
    T sq(T value) {
        return value * value;
    }

    template<typename T>
    T     mapT(T value, T start0, T stop0, T start1, T stop1);
    float map(float value, float start0, float stop0, float start1, float stop1);

    template<typename T>
    T dist(T x1, T y1, T x2, T y2) {
        T dx = x2 - x1;
        T dy = y2 - y1;
        return std::sqrt(dx * dx + dy * dy);
    }

    template<typename T>
    T dist(T x1, T y1, T z1, T x2, T y2, T z2) {
        T dx = x2 - x1;
        T dy = y2 - y1;
        T dz = z2 - z1;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    template<typename T>
    T mag(T a, T b) {
        return std::sqrt(a * a + b * b);
    }

    template<typename T>
    T mag(T a, T b, T c) {
        return std::sqrt(a * a + b * b + c * c);
    }

    // ### Trigonometry

    float degrees(float radians);
    float radians(float degrees);

    // ## Structure

    void exit();
    void noLoop();
    void redraw();

    void setLocation(int x, int y);
    void setTitle(const std::string& title);

    // ( part of original processing but not in the Reference )

    std::string sketchPath();
    std::string resolve_data_path(const std::string& path);

} // namespace umfeld
