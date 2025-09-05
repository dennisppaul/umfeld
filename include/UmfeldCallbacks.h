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
#include <vector>

#include "Subsystems.h"
#include "PAudio.h"

void umfeld_set_callbacks();

// TODO new callback mechanism
// TODO add functions:
//      - `arguments()`
//      - `setup()`
//      - `draw()`
//      - `update()`
//      - `shutdown()`
void settings();
void arguments(const std::vector<std::string>& args);
void setup();
void draw();
void update();
void shutdown();

namespace umfeld {
    // TODO new callback mechanism
    // TODO add functions:
    //      - `arguments()`
    //      - `setup()`
    //      - `draw()`
    //      - `update()`
    //      - `shutdown()`
    using VoidFn = void();
    void callback_settings_set(VoidFn* f);
    void callback_settings_call();
    // TODO decide on naming scheme for callbacks
    void set_callback_setup(VoidFn* f);
    void call_callback_setup();
    void set_setup_callback(VoidFn* f);
    void call_setup_callback();
    // TODO ...

    /* NOTE weak implementations in `Umfeld.cpp` */
    void windowResized(int width, int height);
    void post();

    /* NOTE weak implementations in `SubsystemHIDEvents`*/
    void keyPressed();
    void keyReleased();
    void mousePressed();
    void mouseReleased();
    void mouseDragged();
    void mouseMoved();
    void mouseWheel(float x, float y);
    void dropped(const char* dropped_filedir);
    bool sdl_event(const SDL_Event& event);

    void audioEvent();
    void audioEvent(const umfeld::PAudio& device);
    void callback_audioEvent(const umfeld::PAudio& device);
    void callback_audioEvent();
} // namespace umfeld
