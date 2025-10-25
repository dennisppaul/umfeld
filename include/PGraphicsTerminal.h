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

#include <map>
#include <tuple>
#include <cmath>
#ifdef SYSTEM_WINDOWS
#include <pdcurses/curses.h>
#else
#include <curses.h>
#endif

#include "PGraphics.h"

extern umfeld::SubsystemGraphics* umfeld_create_subsystem_graphics_terminal();

namespace umfeld {

    class NcursesColorManager {
    public:
        explicit NcursesColorManager(short default_bg_color = COLOR_BLACK)
            : next_color_id(16),
              next_pair_id(1),
              current_bg_color_id(default_bg_color) {
            start_color();
        }

        // Set background color from RGB [0–1] and define/init it
        void set_background(float r, float g, float b) {
            const short r_nc    = std::round(r * 1000);
            const short g_nc    = std::round(g * 1000);
            const short b_nc    = std::round(b * 1000);
            current_bg_color_id = get_or_define_color(r_nc, g_nc, b_nc);
        }

        // Get a color pair (new or cached) for given RGB foreground (float [0–1])
        short get_color_pair(float r, float g, float b) {
            const short r_nc = std::round(r * 1000);
            const short g_nc = std::round(g * 1000);
            const short b_nc = std::round(b * 1000);

            const short fg_color_id = get_or_define_color(r_nc, g_nc, b_nc);
            return get_color_pair_nc(fg_color_id, current_bg_color_id);
        }

        // Define a single color from ncurses 0–1000 RGB and return color ID
        short get_or_define_color(short r, short g, short b) {
            const auto key = std::make_tuple(r, g, b);
            if (color_id_map.count(key)) {
                return color_id_map[key];
            }

            if (!can_change_color() || next_color_id >= COLOR_PAIRS - 1) {
                return COLOR_WHITE; // fallback
            }

            const short id = next_color_id++;
            init_color(id, r, g, b);
            color_id_map[key] = id;
            return id;
        }

        // Access last background color ID
        short get_current_bg_color() const {
            return current_bg_color_id;
        }

    private:
        short next_color_id;
        short next_pair_id;
        short current_bg_color_id;

        std::map<std::tuple<short, short, short>, short> color_id_map;
        std::map<std::tuple<short, short>, short>        color_pair_map;

        // Internal: given fg + bg IDs, return or define a color pair
        short get_color_pair_nc(short fg, short bg) {
            const auto key = std::make_tuple(fg, bg);

            if (color_pair_map.count(key)) {
                return color_pair_map[key];
            }

            if (next_pair_id >= COLOR_PAIRS - 1) {
                return 0; // fallback pair
            }

            const short pair_id = next_pair_id++;
            init_pair(pair_id, fg, bg);
            color_pair_map[key] = pair_id;
            return pair_id;
        }
    };

    class PGraphicsTerminal final : public PGraphics {
    public:
        static void enable_graphics_subsystem() {
            enable_graphics    = true;
            subsystem_graphics = umfeld_create_subsystem_graphics_terminal();
        }

        bool simple_clear = false;

        void set_point(const std::string& character) {
            if (character.empty()) {
                ch = DEFAULT_POINT_CHAR;
            } else {
                ch = character;
            }
        }

        void auto_refresh(const bool auto_refresh) {
            auto_refresh_enabled = auto_refresh;
        }

        void background_f(const float r, const float g, const float b, const float d) override {
            if (simple_clear || !has_colors()) {
                clear();
                current_background_pair = 0;
                return;
            }

            // Tell the manager to update the background color
            color_manager.set_background(r, g, b);

            // Get a color pair with black foreground and the new background
            current_background_pair = color_manager.get_color_pair(0.0f, 0.0f, 0.0f); // black fg

            attron(COLOR_PAIR(current_background_pair));

            const int term_rows = height;
            const int term_cols = width;

            for (int y = 0; y < term_rows; ++y) {
                move(y, 0);
                for (int x = 0; x < term_cols; ++x) {
                    addch(' ');
                }
            }

            move(0, 0);
            refresh();
        }

        void point(const float x, const float y, float z) override {
            if (!color_stroke.active || !has_colors()) {
                return;
            }

            const short pair_id = color_manager.get_color_pair(
                color_stroke.r,
                color_stroke.g,
                color_stroke.b);
            attron(COLOR_PAIR(pair_id));
            if (x >= 0 && y >= 0 && x < COLS && y < LINES) {
                // mvaddch(y, x, ch);
                mvaddstr(y, x, ch.c_str());
            }
            attroff(COLOR_PAIR(pair_id));
        }

        void point(const float x, const float y) {
            point(x, y, 0);
        }

        void line(float x1, float y1, const float z1, const float x2, const float y2, const float z2) override {
            if (!color_stroke.active || !has_colors()) {
                return;
            }

            // Get color pair from stroke color
            const short pair_id = color_manager.get_color_pair(
                color_stroke.r,
                color_stroke.g,
                color_stroke.b);
            attron(COLOR_PAIR(pair_id));

            int xi    = static_cast<int>(x1);
            int yi    = static_cast<int>(y1);
            int x_end = static_cast<int>(x2);
            int y_end = static_cast<int>(y2);

            const int dx  = std::abs(x_end - xi);
            const int dy  = -std::abs(y_end - yi);
            const int sx  = (xi < x_end) ? 1 : -1;
            const int sy  = (yi < y_end) ? 1 : -1;
            int       err = dx + dy;

            while (true) {
                mvaddstr(yi, xi, ch.c_str()); // draw using current point character
                if (xi == x_end && yi == y_end) {
                    break;
                }
                const int e2 = 2 * err;
                if (e2 >= dy) {
                    err += dy;
                    xi += sx;
                }
                if (e2 <= dx) {
                    err += dx;
                    yi += sy;
                }
            }

            attroff(COLOR_PAIR(pair_id));
        }

        void rect(const float x, const float y, const float width, const float height) override {
            if (!color_stroke.active || !has_colors()) {
                return;
            }

            const short pair_id = color_manager.get_color_pair(
                color_stroke.r,
                color_stroke.g,
                color_stroke.b);

            attron(COLOR_PAIR(pair_id));

            const int x0 = static_cast<int>(x);
            const int y0 = static_cast<int>(y);
            const int x1 = static_cast<int>(x + width - 1);
            const int y1 = static_cast<int>(y + height - 1);

            // top and bottom edges
            for (int i = x0; i <= x1; ++i) {
                mvaddstr(y0, i, ch.c_str());
                mvaddstr(y1, i, ch.c_str());
            }

            // left and right edges
            for (int i = y0 + 1; i < y1; ++i) {
                mvaddstr(i, x0, ch.c_str());
                mvaddstr(i, x1, ch.c_str());
            }

            attroff(COLOR_PAIR(pair_id));
        }

        void image(PImage* img, float x, float y, float w, float h) override {
            if (!color_fill.active) {
                return;
            }

            if (img == nullptr) {
                error("img is null");
                return;
            }

            if (w < 0) {
                w = img->width;
            }
            if (h < 0) {
                h = img->height;
            }

            const int img_width  = static_cast<int>(img->width);
            const int img_height = static_cast<int>(img->height);

            const float scale_x = static_cast<float>(img_width) / w;
            const float scale_y = static_cast<float>(img_height) / h;

            const short pair_id = color_manager.get_color_pair(color_fill.r, color_fill.g, color_fill.b);
            attron(COLOR_PAIR(pair_id));

            for (int ty = 0; ty < static_cast<int>(h); ++ty) {
                for (int tx = 0; tx < static_cast<int>(w); ++tx) {
                    const int sx    = static_cast<int>(tx * scale_x);
                    const int sy    = static_cast<int>(ty * scale_y);
                    const int index = sy * img_width + sx;

                    const uint32_t pixel = img->pixels[index];
                    const uint8_t  r     = (pixel >> 16) & 0xFF;
                    const uint8_t  g     = (pixel >> 8) & 0xFF;
                    const uint8_t  b     = pixel & 0xFF;

                    // Convert to grayscale brightness (0–255)
                    const uint8_t gray = static_cast<uint8_t>((r * 299 + g * 587 + b * 114) / 1000);

                    // Map to ASCII character
                    const char c      = ASCII_RAMP[(gray * NUM_ASCII_LEVELS) / 256];
                    const char str[2] = {c, '\0'};
                    // // Get a color pair with this pixel's color as foreground and the current background
                    // const short pair_id = color_manager.get_color_pair(
                    //     static_cast<float>(r) / 255.0f,
                    //     static_cast<float>(g) / 255.0f,
                    //     static_cast<float>(b) / 255.0f
                    // );
                    //
                    // attron(COLOR_PAIR(pair_id));
                    // mvaddstr(static_cast<int>(y) + ty, static_cast<int>(x) + tx, str);
                    // attroff(COLOR_PAIR(pair_id));
                    mvaddstr(static_cast<int>(y) + ty, static_cast<int>(x) + tx, str);
                }
            }
            attroff(COLOR_PAIR(pair_id));
            refresh();
        }

        void text_str(const std::string& text, const float x, const float y, const float z) override {
            if (!color_fill.active || !has_colors()) {
                mvaddstr((int) y, (int) x, text.c_str());
                return;
            }

            // Use color_fill as foreground, and manager's last background
            const short pair_id = color_manager.get_color_pair(color_fill.r, color_fill.g, color_fill.b);
            attron(COLOR_PAIR(pair_id));
            mvaddstr((int) y, (int) x, text.c_str());
            attroff(COLOR_PAIR(pair_id));
        }

        void endDraw() override {
            if (auto_refresh_enabled) {
                refresh();
            }
            PGraphics::endDraw();
        }

    private:
        inline static const std::string DEFAULT_POINT_CHAR = "*";

        std::string ch                   = DEFAULT_POINT_CHAR;
        bool        auto_refresh_enabled = true;

        const char*         ASCII_RAMP       = " .:-=+*#%@";
        const int           NUM_ASCII_LEVELS = 10;
        NcursesColorManager color_manager;
        short               current_background_pair = 0;

        std::string utf8_braille(uint8_t dots) {
            const uint32_t codepoint = 0x2800 + dots;

            char utf8[4] = {0};
            if (codepoint <= 0x7F) {
                utf8[0] = codepoint;
            } else if (codepoint <= 0x7FF) {
                utf8[0] = 0xC0 | ((codepoint >> 6) & 0x1F);
                utf8[1] = 0x80 | (codepoint & 0x3F);
            } else {
                utf8[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                utf8[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                utf8[2] = 0x80 | (codepoint & 0x3F);
            }

            return std::string(utf8);
        }
    };
} // namespace umfeld