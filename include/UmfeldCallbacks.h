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

#ifdef __cplusplus
extern "C" {
#endif
/* NOTE weak implementations in `Umfeld.cpp` */
void settings();
void arguments(const std::vector<std::string>& args);
void setup();
void draw();
void update();
void windowResized(int width, int height);
void post();
// [[deprecated("use 'audioEvent(PAudio& audio)' instead")]]
// void audioEvent();
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
#ifdef __cplusplus
} // extern "C"
#endif

void shutdown();                              // NOTE cannot be `extern "C"` due to conflict with function in `socket.h`
void audioEvent(const umfeld::PAudio& audio); // NOTE cannot be `extern "C"` due to overloading
void audioEvent(float& left, float& right);   // NOTE cannot be `extern "C"` due to overloading
// TODO maybe rename to `audioEventDevice` or remove audioEvent()?

/* declare callbacks */
namespace umfeld {
    using FnVoid         = void();
    using FnIntInt       = void(int, int);
    using FnFloatFloat   = void(float, float);
    using FnStrings      = void(const std::vector<std::string>&);
    using FnPAudio       = void(const PAudio&);
    using FnFloatRef_2   = void(float&, float&);
    using FnFloatRef_4   = void(const float&, const float&, float&, float&);
    using FnConstCharPtr = void(const char*);
    using FnSDLEvent     = bool(const SDL_Event& event);
    void set_settings_callback(FnVoid* f);
    void run_settings_callback();
    void set_arguments_callback(FnStrings* f);
    void run_arguments_callback(const std::vector<std::string>&);
    void set_setup_callback(FnVoid* f);
    void run_setup_callback();
    void set_draw_callback(FnVoid* f);
    void run_draw_callback();
    void set_update_callback(FnVoid* f);
    void run_update_callback();
    void set_windowResized_callback(FnIntInt* f);
    void run_windowResized_callback(int, int);
    void set_post_callback(FnVoid* f);
    void run_post_callback();
    void set_shutdown_callback(FnVoid* f);
    void run_shutdown_callback();
    void set_audioEventPAudio_callback(FnPAudio* f);
    void run_audioEventPAudio_callback(const PAudio&);
    void set_audioEventFloatRef_2_callback(FnFloatRef_2* f);
    void run_audioEventFloatRef_2_callback(float&, float&);
    void set_audioEventFloatRef_4_callback(FnFloatRef_4* f);
    void run_audioEventFloatRef_4_callback(const float&, const float&, float&, float&);
    void set_keyPressed_callback(FnVoid* f);
    void run_keyPressed_callback();
    void set_keyReleased_callback(FnVoid* f);
    void run_keyReleased_callback();
    void set_mousePressed_callback(FnVoid* f);
    void run_mousePressed_callback();
    void set_mouseReleased_callback(FnVoid* f);
    void run_mouseReleased_callback();
    void set_mouseDragged_callback(FnVoid* f);
    void run_mouseDragged_callback();
    void set_mouseMoved_callback(FnVoid* f);
    void run_mouseMoved_callback();
    void set_mouseWheel_callback(FnFloatFloat* f);
    void run_mouseWheel_callback(float x, float y);
    void set_dropped_callback(FnConstCharPtr* f);
    void run_dropped_callback(const char* dropped_filedir);
    void set_sdl_event_callback(FnSDLEvent* f);
    bool run_sdl_event_callback(const SDL_Event& event);
}; // namespace umfeld
