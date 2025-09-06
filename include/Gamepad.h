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

#include "UmfeldDefines.h"

umfeld::Subsystem*        enable_gamepads();
void                      gamepad_handle_events_in_loop(bool events_in_loop);
void                      gamepad_motion_event_cooldown(int milliseconds);
std::vector<SDL_Gamepad*> gamepad_connected(bool print = true);
void                      gamepad_print_debug(bool print_debug);

void gamepad_button(int id, int button, bool down);
void gamepad_axis(int id, int axis, float value);
void gamepad_event(const SDL_Event& event);

umfeld::Subsystem* umfeld_create_subsystem_gamepad();
