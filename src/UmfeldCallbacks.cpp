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

UMFELD_FUNC_WEAK void umfeld::audioEvent(const umfeld::PAudio& device) {}
UMFELD_FUNC_WEAK void umfeld::audioEvent() { /* NOTE same as above but for default audio device */ }

void umfeld::callback_audioEvent(const umfeld::PAudio& device) {
    audioEvent(device);
}

void umfeld::callback_audioEvent() {
    audioEvent();
}

// TODO new callback mechanism
// TODO add functions:
//      - `arguments()`
//      - `setup()`
//      - `draw()`
//      - `update()`
//      - `shutdown()`

namespace umfeld {
    namespace {
        void    callback_settings_default() {}
        VoidFn* callback_settings_fn = callback_settings_default;
    } // namespace
} // namespace umfeld

void umfeld::callback_settings_set(VoidFn* f) { callback_settings_fn = f ? f : callback_settings_default; }
void umfeld::callback_settings_call() { callback_settings_fn(); }
