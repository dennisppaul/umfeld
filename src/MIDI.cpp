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
#include "MIDI.h"

UMFELD_FUNC_WEAK void midi_message(const std::vector<unsigned char>& message) { LOG_CALLBACK_MSG("default midi_message"); }
UMFELD_FUNC_WEAK void note_off(int channel, int note) { LOG_CALLBACK_MSG("default note_off"); }
UMFELD_FUNC_WEAK void note_on(int channel, int note, int velocity) { LOG_CALLBACK_MSG("default note_on"); }
UMFELD_FUNC_WEAK void control_change(int channel, int control, int value) { LOG_CALLBACK_MSG("default control_change"); }
UMFELD_FUNC_WEAK void program_change(int channel, int program) { LOG_CALLBACK_MSG("default program_change"); }
UMFELD_FUNC_WEAK void pitch_bend(int channel, int value) { LOG_CALLBACK_MSG("default pitch_bend"); }
UMFELD_FUNC_WEAK void sys_ex(const std::vector<unsigned char>& message) { LOG_CALLBACK_MSG("default sys_ex"); }