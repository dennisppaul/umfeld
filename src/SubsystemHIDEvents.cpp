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

#include <SDL3/SDL.h>

#include "UmfeldDefines.h"
#include "Umfeld.h"
#include "UmfeldCallbacks.h"

namespace umfeld::subsystem {
    static bool _handle_events_in_loop = true;
    static bool _mouse_is_pressed      = false;

    void hid_handle_events_in_loop(const bool events_in_loop) {
        _handle_events_in_loop = events_in_loop;
    }

    static void handle_event(const SDL_Event& event) {
        run_sdl_event_callback(event);

        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
                key = static_cast<int>(event.key.key);
                run_keyPressed_callback();
                isKeyPressed = true;
                break;
            case SDL_EVENT_KEY_UP:
                key          = static_cast<int>(event.key.key);
                isKeyPressed = false;
                run_keyReleased_callback();
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                mouseButton       = event.button.button; // TODO not sure how consistent these are across platforms
                _mouse_is_pressed = true;
                run_mousePressed_callback();
                isMousePressed = true;

                // callbackHook();
                // if (callbackHook_func) {
                //     callbackHook_func();
                // }
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                _mouse_is_pressed = false;
                mouseButton       = -1;
                run_mouseReleased_callback();
                isMousePressed = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                pmouseX = mouseX;
                pmouseY = mouseY;
                mouseX  = static_cast<float>(event.motion.x);
                mouseY  = static_cast<float>(event.motion.y);

                if (_mouse_is_pressed) {
                    run_mouseDragged_callback();
                } else {
                    run_mouseMoved_callback();
                }
                break;
            // case SDL_MULTIGESTURE:
            case SDL_EVENT_MOUSE_WHEEL:
                run_mouseWheel_callback(event.wheel.mouse_x, event.wheel.mouse_y);
                break;
            case SDL_EVENT_DROP_FILE: {
                // only allow drag and drop on main window
                // if (event.drop.windowID != 1) { break; }
                const char* dropped_filedir = event.drop.data;
                run_dropped_callback(dropped_filedir);
                break;
            }
            default: break;
        }
    }

    static void shutdown() {}

    static void set_flags(uint32_t& subsystem_flags) {
        subsystem_flags |= SDL_INIT_EVENTS;
    }

    static bool is_hid_event(const SDL_Event* event) {
        return event->type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
               event->type == SDL_EVENT_MOUSE_BUTTON_UP ||
               event->type == SDL_EVENT_MOUSE_MOTION ||
               event->type == SDL_EVENT_MOUSE_WHEEL ||
               event->type == SDL_EVENT_KEY_UP ||
               event->type == SDL_EVENT_KEY_DOWN ||
               event->type == SDL_EVENT_DROP_FILE;
    }

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    static void event(SDL_Event* event) {
        if (is_hid_event(event)) {
            if (!_handle_events_in_loop) {
                handle_event(*event);
            }
        }
    }

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    static void event_in_update_loop(SDL_Event* event) {
        if (is_hid_event(event)) {
            if (_handle_events_in_loop) {
                handle_event(*event);
            }
        }
    }

    static const char* name() {
        return "HID Events ( mouse, keyboard, drag-n-dr... )";
    }
} // namespace umfeld::subsystem

umfeld::Subsystem* umfeld_create_subsystem_hid() {
    auto* libraries                 = new umfeld::Subsystem{};
    libraries->shutdown             = umfeld::subsystem::shutdown;
    libraries->set_flags            = umfeld::subsystem::set_flags;
    libraries->event                = umfeld::subsystem::event;
    libraries->event_in_update_loop = umfeld::subsystem::event_in_update_loop;
    libraries->name                 = umfeld::subsystem::name;
    return libraries;
}
