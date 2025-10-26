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

#include "UmfeldDefines.h"
#include "UmfeldCallbacks.h"

/* set default callback functions */

namespace umfeld {
    namespace {
        void            callback_settings_default() {}
        FnVoid*         callback_settings_fn = callback_settings_default;
        void            callback_arguments_default(const std::vector<std::string>&) {}
        FnStrings*      callback_arguments_fn = callback_arguments_default;
        void            callback_setup_default() {}
        FnVoid*         callback_setup_fn = callback_setup_default;
        void            callback_draw_default() {}
        FnVoid*         callback_draw_fn = callback_draw_default;
        void            callback_update_default() {}
        FnVoid*         callback_update_fn = callback_update_default;
        void            callback_windowResized_default(int, int) {}
        FnIntInt*       callback_windowResized_fn = callback_windowResized_default;
        void            callback_post_default() {}
        FnVoid*         callback_post_fn = callback_post_default;
        void            callback_shutdown_default() {}
        FnVoid*         callback_shutdown_fn = callback_shutdown_default;
        // void            callback_audioEvent_default() {}
        // FnVoid*         callback_audioEvent_fn = callback_audioEvent_default;
        void            callback_audioEventPAudio_default(const PAudio&) {}
        FnPAudio*       callback_audioEventPAudio_fn = callback_audioEventPAudio_default;
        void            callback_audioEventFloatRef_2_default(float&, float&) {}
        FnFloatRef_2*   callback_audioEventFloatRef_2_fn = callback_audioEventFloatRef_2_default;
        void            callback_audioEventFloatRef_4_default(const float&, const float&, float&, float&) {}
        FnFloatRef_4*   callback_audioEventFloatRef_4_fn = callback_audioEventFloatRef_4_default;
        void            callback_keyPressed_default() {}
        FnVoid*         callback_keyPressed_fn = callback_keyPressed_default;
        void            callback_keyReleased_default() {}
        FnVoid*         callback_keyReleased_fn = callback_keyReleased_default;
        void            callback_mousePressed_default() {}
        FnVoid*         callback_mousePressed_fn = callback_mousePressed_default;
        void            callback_mouseReleased_default() {}
        FnVoid*         callback_mouseReleased_fn = callback_mouseReleased_default;
        void            callback_mouseDragged_default() {}
        FnVoid*         callback_mouseDragged_fn = callback_mouseDragged_default;
        void            callback_mouseMoved_default() {}
        FnVoid*         callback_mouseMoved_fn = callback_mouseMoved_default;
        void            callback_mouseWheel_default(float, float) {}
        FnFloatFloat*   callback_mouseWheel_fn = callback_mouseWheel_default;
        void            callback_dropped_default(const char*) {}
        FnConstCharPtr* callback_dropped_fn = callback_dropped_default;
        bool            callback_sdl_event_default(const SDL_Event&) { return false; }
        FnSDLEvent*     callback_sdl_event_fn = callback_sdl_event_default;
    } // namespace
} // namespace umfeld

void umfeld::set_settings_callback(FnVoid* f) { callback_settings_fn = f ? f : callback_settings_default; }
void umfeld::run_settings_callback() { callback_settings_fn(); }
void umfeld::set_arguments_callback(FnStrings* f) { callback_arguments_fn = f ? f : callback_arguments_default; }
void umfeld::run_arguments_callback(const std::vector<std::string>& args) { callback_arguments_fn(args); }
void umfeld::set_setup_callback(FnVoid* f) { callback_setup_fn = f ? f : callback_setup_default; }
void umfeld::run_setup_callback() { callback_setup_fn(); }
void umfeld::set_draw_callback(FnVoid* f) { callback_draw_fn = f ? f : callback_draw_default; }
void umfeld::run_draw_callback() { callback_draw_fn(); }
void umfeld::set_update_callback(FnVoid* f) { callback_update_fn = f ? f : callback_update_default; }
void umfeld::run_update_callback() { callback_update_fn(); }
void umfeld::set_windowResized_callback(FnIntInt* f) { callback_windowResized_fn = f ? f : callback_windowResized_default; }
void umfeld::run_windowResized_callback(const int width, const int height) { callback_windowResized_fn(width, height); }
void umfeld::set_post_callback(FnVoid* f) { callback_post_fn = f ? f : callback_post_default; }
void umfeld::run_post_callback() { callback_post_fn(); }
void umfeld::set_shutdown_callback(FnVoid* f) { callback_shutdown_fn = f ? f : callback_shutdown_default; }
void umfeld::run_shutdown_callback() { callback_shutdown_fn(); }
// void umfeld::set_audioEvent_callback(FnVoid* f) { callback_audioEvent_fn = f ? f : callback_audioEvent_default; }
// void umfeld::run_audioEvent_callback() { callback_audioEvent_fn(); }
void umfeld::set_audioEventPAudio_callback(FnPAudio* f) { callback_audioEventPAudio_fn = f ? f : callback_audioEventPAudio_default; }
void umfeld::run_audioEventPAudio_callback(const PAudio& device) { callback_audioEventPAudio_fn(device); }

void umfeld::set_audioEventFloatRef_2_callback(FnFloatRef_2* f) { callback_audioEventFloatRef_2_fn = f ? f : callback_audioEventFloatRef_2_fn; }
void umfeld::run_audioEventFloatRef_2_callback(float& left, float& right) { callback_audioEventFloatRef_2_fn(left, right); }
void umfeld::set_audioEventFloatRef_4_callback(FnFloatRef_4* f) { callback_audioEventFloatRef_4_fn = f ? f : callback_audioEventFloatRef_4_fn; }
void umfeld::run_audioEventFloatRef_4_callback(const float& input_left, const float& input_right, float& output_left, float& output_right) { callback_audioEventFloatRef_4_fn(input_left, input_right, output_left, output_right); }

void umfeld::set_keyPressed_callback(FnVoid* f) { callback_keyPressed_fn = f ? f : callback_keyPressed_default; }
void umfeld::run_keyPressed_callback() { callback_keyPressed_fn(); }
void umfeld::set_keyReleased_callback(FnVoid* f) { callback_keyReleased_fn = f ? f : callback_keyReleased_default; }
void umfeld::run_keyReleased_callback() { callback_keyReleased_fn(); }
void umfeld::set_mousePressed_callback(FnVoid* f) { callback_mousePressed_fn = f ? f : callback_mousePressed_default; }
void umfeld::run_mousePressed_callback() { callback_mousePressed_fn(); }
void umfeld::set_mouseReleased_callback(FnVoid* f) { callback_mouseReleased_fn = f ? f : callback_mouseReleased_default; }
void umfeld::run_mouseReleased_callback() { callback_mouseReleased_fn(); }
void umfeld::set_mouseDragged_callback(FnVoid* f) { callback_mouseDragged_fn = f ? f : callback_mouseDragged_default; }
void umfeld::run_mouseDragged_callback() { callback_mouseDragged_fn(); }
void umfeld::set_mouseMoved_callback(FnVoid* f) { callback_mouseMoved_fn = f ? f : callback_mouseMoved_default; }
void umfeld::run_mouseMoved_callback() { callback_mouseMoved_fn(); }
void umfeld::set_mouseWheel_callback(FnFloatFloat* f) { callback_mouseWheel_fn = f ? f : callback_mouseWheel_default; }
void umfeld::run_mouseWheel_callback(const float x, const float y) { callback_mouseWheel_fn(x, y); }
void umfeld::set_dropped_callback(FnConstCharPtr* f) { callback_dropped_fn = f ? f : callback_dropped_default; }
void umfeld::run_dropped_callback(const char* dropped_filedir) { callback_dropped_fn(dropped_filedir); }
void umfeld::set_sdl_event_callback(FnSDLEvent* f) { callback_sdl_event_fn = f ? f : callback_sdl_event_default; }
bool umfeld::run_sdl_event_callback(const SDL_Event& event) { return callback_sdl_event_fn(event); }
