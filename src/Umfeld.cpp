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
#include <SDL3/SDL_main.h>

#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include "UmfeldDefines.h"
#include "Umfeld.h"
#include "PAudio.h"
#include "UmfeldFunctionsAdditional.h"

using namespace std::chrono;

#if UMFELD_SET_DEFAULT_CALLBACK
/* default callback stubs */
// NOTE provide weak callback implementation if default callbacks are used.
// NOTE callbacks are set with `umfeld::set_XXX_callback()` in `SDL_AppInit`
UMFELD_FUNC_WEAK void settings() { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void arguments(const std::vector<std::string>& args) { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void setup() { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void draw() { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void update() { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void windowResized(int width, int height) { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void post() { LOG_CALLBACK_MSG("default post"); }
UMFELD_FUNC_WEAK void shutdown() { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void audioEvent(const umfeld::PAudio& audio) { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void audioEvent(float& left, float& right) { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void audioEvent(const float& input_left, const float& input_right, float& output_left, float& output_right) { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
[[deprecated("use 'audioEvent(PAudio& audio)' instead")]]
UMFELD_FUNC_WEAK void audioEvent() { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void keyPressed() { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void keyReleased() { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void mousePressed() { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void mouseReleased() { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void mouseDragged() { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void mouseMoved() { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void mouseWheel(float x, float y) { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK void dropped(const char* dropped_filedir) { LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__)); }
UMFELD_FUNC_WEAK bool sdl_event(const SDL_Event& event) {
    LOG_CALLBACK_MSG(umfeld::to_string("default: ", __func__));
    return false;
}
#endif

namespace umfeld {
    static high_resolution_clock::time_point lastFrameTime                       = {};
    static bool                              initialized                         = false;
    static double                            target_frame_duration               = 1.0 / frameRate;
    static bool                              handle_subsystem_graphics_cleanup   = false;
    static bool                              handle_subsystem_audio_cleanup      = false;
    static bool                              handle_subsystem_libraries_cleanup  = false;
    static bool                              handle_subsystem_hid_events_cleanup = false;
    static bool                              _app_is_running                     = true;
    static bool                              _app_no_loop                        = false;
    static bool                              _app_force_redraw                   = false;
    static std::vector<SDL_Event>            event_cache;

    void exit() { _app_is_running = false; }
    void noLoop() { _app_no_loop = true; }
    void redraw() { _app_force_redraw = true; }

    bool is_initialized() {
        return initialized;
    }

    void setTitle(const std::string& title) {
        // window_title = title; // NOTE update global variable
        if (subsystem_graphics) {
            if (subsystem_graphics->set_title) {
                subsystem_graphics->set_title(title);
            }
        }
    }

    std::string getTitle() {
        if (subsystem_graphics) {
            if (subsystem_graphics->get_title) {
                const std::string title = subsystem_graphics->get_title();
                // window_title            = title; // NOTE update global variable
                return title;
            }
        }
        return DEFAULT_WINDOW_TITLE;
    }

    void setLocation(const int x, const int y) {
        if (subsystem_graphics) {
            if (subsystem_graphics->set_window_position) {
                subsystem_graphics->set_window_position(x, y);
            }
        }
    }

    void getLocation(const int& x, const int& y) {
        if (subsystem_graphics) {
            if (subsystem_graphics->set_window_position) {
                subsystem_graphics->set_window_position(x, y);
            }
        }
    }

    void setWindowSize(const int width, const int height) {
        if (subsystem_graphics) {
            if (subsystem_graphics->set_window_size) {
                subsystem_graphics->set_window_size(width, height);
            }
        }
    }

    void getWindowSize(int& width, int& height) {
        if (subsystem_graphics) {
            if (subsystem_graphics->get_window_size) {
                subsystem_graphics->get_window_size(width, height);
            }
        }
    }

    void set_frame_rate(const float fps) {
        target_frame_duration = 1.0 / fps;
    }

    void register_library(LibraryListener* listener) {
        if (subsystem_libraries == nullptr) {
            warning("no library subsystem available (yet). make sure to NOT call this function in 'settings()'");
            return;
        }
        subsystem_libraries->register_library(listener);
    }

    void unregister_library(const LibraryListener* listener) {
        if (subsystem_libraries == nullptr) {
            warning("no library subsystem available (yet). make sure to NOT call this function in 'settings()'");
            return;
        }
        subsystem_libraries->unregister_library(listener);
    }

    static bool set_display_size() {
        const SDL_DisplayID display_id = SDL_GetPrimaryDisplay();
        if (display_id == 0) {
            warning(format_label("failed to get primary display"), "SDL ", SDL_GetError(), " ( might be intentional )");
        } else {
            const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display_id);
            if (mode != nullptr) {
                display_width  = mode->w;
                display_height = mode->h;
                console(format_label("display resolution"), display_width, " x ", display_height, " px");
                return true;
            }
            warning(umfeld::format_label("failed to get display mode"), "SDL ", SDL_GetError());
        }
        warning(umfeld::format_label("setting display size to window size"), width, " x ", height, " px");
        display_width  = static_cast<int>(width);
        display_height = static_cast<int>(height);
        return false;
    }

    SDL_WindowFlags get_SDL_WindowFlags(SDL_WindowFlags& flags) {
        /*
         * SDL_WINDOW_FULLSCREEN           SDL_UINT64_C(0x0000000000000001)    //  window is in fullscreen mode
         * SDL_WINDOW_OPENGL               SDL_UINT64_C(0x0000000000000002)    //  window usable with OpenGL context
         * SDL_WINDOW_OCCLUDED             SDL_UINT64_C(0x0000000000000004)    //  window is occluded
         * SDL_WINDOW_HIDDEN               SDL_UINT64_C(0x0000000000000008)    //  window is neither mapped onto the desktop nor shown in the taskbar/dock/window list; SDL_ShowWindow() is required for it to become visible
         * SDL_WINDOW_BORDERLESS           SDL_UINT64_C(0x0000000000000010)    //  no window decoration
         * SDL_WINDOW_RESIZABLE            SDL_UINT64_C(0x0000000000000020)    //  window can be resized
         * SDL_WINDOW_MINIMIZED            SDL_UINT64_C(0x0000000000000040)    //  window is minimized
         * SDL_WINDOW_MAXIMIZED            SDL_UINT64_C(0x0000000000000080)    //  window is maximized
         * SDL_WINDOW_MOUSE_GRABBED        SDL_UINT64_C(0x0000000000000100)    //  window has grabbed mouse input
         * SDL_WINDOW_INPUT_FOCUS          SDL_UINT64_C(0x0000000000000200)    //  window has input focus
         * SDL_WINDOW_MOUSE_FOCUS          SDL_UINT64_C(0x0000000000000400)    //  window has mouse focus
         * SDL_WINDOW_EXTERNAL             SDL_UINT64_C(0x0000000000000800)    //  window not created by SDL
         * SDL_WINDOW_MODAL                SDL_UINT64_C(0x0000000000001000)    //  window is modal
         * SDL_WINDOW_HIGH_PIXEL_DENSITY   SDL_UINT64_C(0x0000000000002000)    //  window uses high pixel density back buffer if possible
         * SDL_WINDOW_MOUSE_CAPTURE        SDL_UINT64_C(0x0000000000004000)    //  window has mouse captured (unrelated to MOUSE_GRABBED)
         * SDL_WINDOW_MOUSE_RELATIVE_MODE  SDL_UINT64_C(0x0000000000008000)    //  window has relative mode enabled
         * SDL_WINDOW_ALWAYS_ON_TOP        SDL_UINT64_C(0x0000000000010000)    //  window should always be above others
         * SDL_WINDOW_UTILITY              SDL_UINT64_C(0x0000000000020000)    //  window should be treated as a utility window, not showing in the task bar and window list
         * SDL_WINDOW_TOOLTIP              SDL_UINT64_C(0x0000000000040000)    //  window should be treated as a tooltip and does not get mouse or keyboard focus, requires a parent window
         * SDL_WINDOW_POPUP_MENU           SDL_UINT64_C(0x0000000000080000)    //  window should be treated as a popup menu, requires a parent window
         * SDL_WINDOW_KEYBOARD_GRABBED     SDL_UINT64_C(0x0000000000100000)    //  window has grabbed keyboard input
         * SDL_WINDOW_VULKAN               SDL_UINT64_C(0x0000000010000000)    //  window usable for Vulkan surface
         * SDL_WINDOW_METAL                SDL_UINT64_C(0x0000000020000000)    //  window usable for Metal view
         * SDL_WINDOW_TRANSPARENT          SDL_UINT64_C(0x0000000040000000)    //  window with transparent buffer
         * SDL_WINDOW_NOT_FOCUSABLE        SDL_UINT64_C(0x0000000080000000)    //  window should not be focusable
         */
        flags |= SDL_WINDOW_HIDDEN; // NOTE always hide window
        flags |= SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
        flags |= fullscreen ? SDL_WINDOW_FULLSCREEN : 0;
        flags |= borderless ? SDL_WINDOW_BORDERLESS : 0;
        flags |= resizable ? SDL_WINDOW_RESIZABLE : 0;
        flags |= retina_support ? SDL_WINDOW_HIGH_PIXEL_DENSITY : 0;
        flags |= always_on_top ? SDL_WINDOW_ALWAYS_ON_TOP : 0;
        return flags;
    }
} // namespace umfeld

/* update in thread section :: START */

namespace {
    std::thread             g_updateThread;
    std::atomic_bool        g_updateRequested{false};
    std::atomic_bool        g_updateRunning{true};
    std::mutex              g_updateMtx;
    std::condition_variable g_updateCv;
} // namespace

// NOTE thread update function
static void update_thread_function() {
    while (g_updateRunning.load(std::memory_order_acquire)) {
        // wait until requested or shutdown
        std::unique_lock<std::mutex> lk(g_updateMtx);
        g_updateCv.wait(lk, [] { return !g_updateRunning.load(std::memory_order_acquire) || g_updateRequested.load(std::memory_order_acquire); });
        if (!g_updateRunning.load(std::memory_order_acquire)) {
            break;
        }
        g_updateRequested.store(false, std::memory_order_release);
        lk.unlock();

        // run update work (no rendering or SDL window calls here)
        umfeld::run_update_callback();
    }
}

// NOTE call once after setup is complete ( i.e end of 'SDL_AppInit' )
static void start_update_thread() {
    g_updateRunning.store(true, std::memory_order_release);
    g_updateThread = std::thread(update_thread_function);
}

// NOTE call during shutdown ( i.e in SDL_AppQuit )
static void stop_update_thread() {
    {
        std::lock_guard<std::mutex> lk(g_updateMtx);
        g_updateRunning.store(false, std::memory_order_release);
    }
    g_updateCv.notify_all();
    if (g_updateThread.joinable()) {
        g_updateThread.join();
    }
}

// NOTE call periodically ( i.e in SDL_AppIterate )
static void request_update_thread() {
    {
        std::lock_guard<std::mutex> lk(g_updateMtx);
        g_updateRequested.store(true, std::memory_order_release);
    }
    g_updateCv.notify_one();
}

/* update in thread section :: END */

static void handle_arguments(const int argc, char* argv[]) {
    std::vector<std::string> args;
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            args.emplace_back(argv[i]);
        }
    }
    umfeld::run_arguments_callback(args);
}

static uint32_t compile_subsystems_flag() {
    /* from `SDL.h`:
     * - `SDL_INIT_TIMER`: timer subsystem
     * - `SDL_INIT_AUDIO`: audio subsystem
     * - `SDL_INIT_VIDEO`: video subsystem; automatically initializes the events
     *   subsystem
     * - `SDL_INIT_JOYSTICK`: joystick subsystem; automatically initializes the
     *   events subsystem
     * - `SDL_INIT_HAPTIC`: haptic (force feedback) subsystem
     * - `SDL_INIT_GAMECONTROLLER`: controller subsystem; automatically
     *   initializes the joystick subsystem
     * - `SDL_INIT_EVENTS`: events subsystem
     * - `SDL_INIT_EVERYTHING`: all of the above subsystems
     * - `SDL_INIT_NOPARACHUTE`: compatibility; this flag is ignored
     */
    constexpr uint32_t subsystem_flags = 0;
    // subsystem_flags |= SDL_INIT_EVENTS;
    return subsystem_flags;
}

static void handle_event_in_update_loop() {
    for (const umfeld::Subsystem* subsystem: umfeld::subsystems) {
        if (subsystem != nullptr && subsystem->event_in_update_loop != nullptr) {
            for (auto& e: umfeld::event_cache) {
                subsystem->event_in_update_loop(&e);
            }
        }
    }
    umfeld::event_cache.clear();
}

static void handle_draw_in_setup();

void handle_setup_draw_and_post() {
    handle_draw_in_setup();

    /* - setup_post */

    for (const umfeld::Subsystem* subsystem: umfeld::subsystems) {
        if (subsystem != nullptr) {
            if (subsystem->setup_post != nullptr) {
                subsystem->setup_post();
            }
        }
    }

    umfeld::lastFrameTime = std::chrono::high_resolution_clock::now();

    /* start update thread */
    if (umfeld::run_update_in_thread) {
        start_update_thread();
    }
}
SDL_AppResult SDL_AppInit(void** appstate, const int argc, char* argv[]) {
    /*
     * 0. setup callbacks
     * 1. prepare umfeld application ( e.g args, settings )
     * 2. initialize SDL
     * 3. initialize graphics
     * 4. initialize audio
     * 5. setup application
     */

    /* 0. setup callbacks */
#if UMFELD_SET_DEFAULT_CALLBACK
    umfeld::set_settings_callback(settings);
    umfeld::set_arguments_callback(arguments);
    umfeld::set_setup_callback(setup);
    umfeld::set_draw_callback(draw);
    umfeld::set_update_callback(update);
    umfeld::set_windowResized_callback(windowResized);
    umfeld::set_post_callback(post);
    umfeld::set_shutdown_callback(shutdown);
    umfeld::set_audioEventPAudio_callback(audioEvent);
    umfeld::set_audioEventFloatRef_2_callback(audioEvent);
    umfeld::set_audioEventFloatRef_4_callback(audioEvent);
    umfeld::set_keyPressed_callback(keyPressed);
    umfeld::set_keyReleased_callback(keyReleased);
    umfeld::set_mousePressed_callback(mousePressed);
    umfeld::set_mouseReleased_callback(mouseReleased);
    umfeld::set_mouseDragged_callback(mouseDragged);
    umfeld::set_mouseMoved_callback(mouseMoved);
    umfeld::set_mouseWheel_callback(mouseWheel);
    umfeld::set_dropped_callback(dropped);
    umfeld::set_sdl_event_callback(sdl_event);
#else
    // NOTE application is required to set callbacks in `umfeld_set_callbacks()`
    umfeld_set_callbacks();
#endif

    /* 1. prepare umfeld application */

    umfeld::console(umfeld::separator());
    umfeld::console(umfeld::format_label("Umfeld version"),
                    "v", umfeld::VERSION_MAJOR,
                    ".", umfeld::VERSION_MINOR,
                    ".", umfeld::VERSION_PATCH);
    umfeld::console(umfeld::separator(false));
    handle_arguments(argc, argv);
    umfeld::run_settings_callback();

    /* create/check graphics subsystem */
    if (umfeld::enable_graphics) {
        if (umfeld::renderer > umfeld::RENDERER_DEFAULT || umfeld::renderer > umfeld::P3D || umfeld::renderer > umfeld::P2D) {
            umfeld::console("setting renderer from paramter `size()` ( or `umfeld::renderer` ).");
            switch (umfeld::renderer) {
                case umfeld::RENDERER_OPENGL_2_0:
#ifndef OPENGL_2_0
                    umfeld::error("RENDERER_OPENGL_2_0 requires `OPENGL_2_0` to be defined e.g `-DOPENGL_2_0` in CLI or `set(UMFELD_OPENGL_VERSION \"OPENGL_2_0\")` in `CMakeLists.txt`");
#endif
                    umfeld::subsystem_graphics = umfeld_create_subsystem_graphics_openglv20();
                    break;
                case umfeld::RENDERER_OPENGL_ES_3_0:
#ifndef OPENGL_ES_3_0
                    umfeld::error("RENDERER_OPENGL_ES_3_0 requires `OPENGL_ES_3_0` to be defined e.g `-DOPENGL_ES_3_0` in CLI or `set(UMFELD_OPENGL_VERSION \"OPENGL_ES_3_0\")` in `CMakeLists.txt`");
#endif
                    umfeld::subsystem_graphics = umfeld_create_subsystem_graphics_openglves30();
                    break;
                default:
                case umfeld::RENDERER_OPENGL_3_3_CORE:
#ifndef OPENGL_3_3_CORE
                    umfeld::error("RENDERER_OPENGL_3_3_CORE requires `OPENGL_3_3_CORE` to be defined e.g `-DOPENGL_3_3_CORE` in CLI or `set(UMFELD_OPENGL_VERSION \"OPENGL_3_3_CORE\")` in `CMakeLists.txt`");
#endif
                    umfeld::subsystem_graphics = umfeld_create_subsystem_graphics_openglv33();
            }
        }
        if (umfeld::subsystem_graphics == nullptr) {
            if (umfeld::create_subsystem_graphics != nullptr) {
                umfeld::console("creating graphics subsystem with callback.");
                umfeld::subsystem_graphics                = umfeld::create_subsystem_graphics();
                umfeld::handle_subsystem_graphics_cleanup = true;
            } else {
                umfeld::console("no graphics subsystem provided, using default.");
#if defined(OPENGL_3_3_CORE)
                umfeld::subsystem_graphics = umfeld_create_subsystem_graphics_openglv33();
#elif defined(OPENGL_2_0)
                umfeld::subsystem_graphics = umfeld_create_subsystem_graphics_openglv20();
#elif defined(OPENGL_ES_3_0)
                umfeld::subsystem_graphics = umfeld_create_subsystem_graphics_openglves30();
#else
                umfeld::subsystem_graphics = umfeld_create_subsystem_graphics_sdl2d();
#endif
                umfeld::handle_subsystem_graphics_cleanup = true;
            }
            if (umfeld::subsystem_graphics == nullptr) {
                umfeld::console("did not create graphics subsystem.");
            }
        } else {
            umfeld::console("client provided graphics subsystem ( including `size()` ).");
        }
        if (umfeld::subsystem_graphics != nullptr) {
            umfeld::subsystems.push_back(umfeld::subsystem_graphics);
            if (umfeld::subsystem_graphics->name != nullptr) {
                umfeld::console(umfeld::format_label("created subsystem graphics"), umfeld::subsystem_graphics->name());
            } else {
                umfeld::console(umfeld::format_label("created subsystem graphics"), "( no name specified )");
            }
            if (umfeld::subsystem_graphics->set_title) {
                umfeld::subsystem_graphics->set_title(umfeld::DEFAULT_WINDOW_TITLE);
            }
        }
    } else {
        umfeld::console("graphics disabled.");
    }

    /* create/check audio subsystem */
    if (umfeld::enable_audio) {
        if (umfeld::subsystem_audio == nullptr) {
            if (umfeld::create_subsystem_audio != nullptr) {
                umfeld::console("creating audio subsystem via callback.");
                umfeld::subsystem_audio                = umfeld::create_subsystem_audio();
                umfeld::handle_subsystem_audio_cleanup = true;
            } else {
                umfeld::console("no audio subsystem provided, using default ( SDL ).");
                umfeld::subsystem_audio = umfeld_create_subsystem_audio_sdl();
                // umfeld::console("no audio subsystem provided, using default ( PortAudio ).");
                // umfeld::subsystem_audio                = umfeld_create_subsystem_audio_portaudio();
                umfeld::handle_subsystem_audio_cleanup = true;
            }
            if (umfeld::subsystem_audio == nullptr) {
                umfeld::console("did not create audio subsystem.");
            }
        } else {
            umfeld::console("client provided audio subsystem.");
        }
        umfeld::console("adding audio subsystem.");
        umfeld::subsystems.push_back(umfeld::subsystem_audio);
        if (umfeld::subsystem_audio->name != nullptr) {
            umfeld::console(umfeld::format_label("created subsystem audio"), umfeld::subsystem_audio->name());
        } else {
            umfeld::console(umfeld::format_label("created subsystem audio"), "( no name specified )");
        }
    } else {
        umfeld::console("audio disabled.");
    }

    /* create/check libraries subsystem */
    if (umfeld::enable_libraries) {
        if (umfeld::subsystem_libraries == nullptr) {
            umfeld::subsystem_libraries                = umfeld_create_subsystem_libraries();
            umfeld::handle_subsystem_libraries_cleanup = true;
        } else {
            umfeld::console("client provided library subsystem.");
        }
        umfeld::console(umfeld::format_label("created subsystem libraries"), umfeld::subsystem_libraries->name());
        umfeld::subsystems.push_back(umfeld::subsystem_libraries);
    } else {
        umfeld::console("libraries disabled.");
    }

    /* create/check events subsystem */
    if (umfeld::enable_events) {
        if (umfeld::subsystem_hid_events == nullptr) {
            umfeld::subsystem_hid_events                = umfeld_create_subsystem_hid();
            umfeld::handle_subsystem_hid_events_cleanup = true;
        } else {
            umfeld::console("client provided HID events subsystem.");
        }
        umfeld::console(umfeld::format_label("created subsystem HID events"), umfeld::subsystem_hid_events->name());
        umfeld::subsystems.push_back(umfeld::subsystem_hid_events);
    } else {
        umfeld::console("HID events disabled.");
    }

    /* 2. initialize SDL */

    uint32_t subsystem_flags = compile_subsystems_flag();

    for (const umfeld::Subsystem* subsystem: umfeld::subsystems) {
        if (subsystem != nullptr) {
            if (subsystem->set_flags != nullptr) {
                subsystem->set_flags(subsystem_flags);
            }
        }
    }

    if (!SDL_Init(subsystem_flags)) {
        umfeld::error(umfeld::format_label("couldn't initialize SDL"), SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (umfeld::subsystem_graphics != nullptr) {
        umfeld::console(umfeld::separator());
        umfeld::console("VIDEO CAPABILITIES");
        umfeld::console(umfeld::separator());
        umfeld::set_display_size();
        const char* video_driver = SDL_GetCurrentVideoDriver();
        if (video_driver == nullptr) {
            umfeld::console(umfeld::format_label("video driver"), "SDL ", SDL_GetError(), " or is not used");
        } else {
            umfeld::console(umfeld::format_label("video driver"), video_driver);
        }
        if (umfeld::width == umfeld::DISPLAY_WIDTH) {
            umfeld::width = static_cast<float>(umfeld::display_width);
        }
        if (umfeld::height == umfeld::DISPLAY_HEIGHT) {
            umfeld::height = static_cast<float>(umfeld::display_height);
        }
    }

    // TODO make this configurable
    SDL_SetAppMetadata("Umfeld Application", "1.0", "de.dennisppaul.umfeld.application");

    for (const umfeld::Subsystem* subsystem: umfeld::subsystems) {
        if (subsystem != nullptr) {
            if (subsystem->init != nullptr) {
                if (!subsystem->init()) {
                    umfeld::warning("Couldn't initialize subsystem.");
                }
            }
        }
    }

    if (umfeld::enable_graphics) {
        if (umfeld::subsystem_graphics != nullptr) {
            if (umfeld::subsystem_graphics->create_native_graphics != nullptr) {
                umfeld::g = umfeld::subsystem_graphics->create_native_graphics(umfeld::render_to_buffer);
            }
            /* NOTE interpret renderer as profiles */
            if (umfeld::renderer == umfeld::P2D) {
                umfeld::profile(umfeld::PROFILE_2D);
            }
            if (umfeld::renderer == umfeld::P3D) {
                umfeld::profile(umfeld::PROFILE_3D);
            }
        }
    }

    if (umfeld::enable_audio) {
        if (umfeld::subsystem_audio != nullptr) {
            if (umfeld::subsystem_audio->create_audio != nullptr) {
                umfeld::audio_device = umfeld::subsystem_audio->create_audio(&umfeld::audio_unit_info);
            }
        }
    }

    umfeld::console(umfeld::format_label("'Vertex' struct size"), sizeof(umfeld::Vertex), " bytes");

    umfeld::initialized = true;

    /* - setup_pre */

    for (const umfeld::Subsystem* subsystem: umfeld::subsystems) {
        if (subsystem != nullptr) {
            if (subsystem->setup_pre != nullptr) {
                subsystem->setup_pre();
            }
        }
    }

    // NOTE feed back subsystem values to the global variables. values are updated before `setup()` is called
    if (umfeld::g != nullptr && umfeld::enable_graphics) {
        // NOTE update global variable
        umfeld::width  = umfeld::g->width;
        umfeld::height = umfeld::g->height;
        if (umfeld::g->displayDensity() > 1) {
            // NOTE create two pixel buffers if display density is greater than 1 ( e.g retina display )
            //      g->pixels -> physical size ( w * d * h * d )
            //      pixels    -> logical size ( w * h )
            const auto count_d = static_cast<size_t>(umfeld::g->width * static_cast<float>(umfeld::g->displayDensity()) * umfeld::g->height * static_cast<float>(umfeld::g->displayDensity()));
            const auto count   = static_cast<size_t>(umfeld::g->width * umfeld::g->height);
            umfeld::g->pixels  = new uint32_t[count_d];
            umfeld::pixels     = new uint32_t[count];
        } else {
            // NOTE create single pixel buffer and share it
            const auto count  = static_cast<size_t>(umfeld::g->width * umfeld::g->height);
            umfeld::g->pixels = new uint32_t[count];
            umfeld::pixels    = umfeld::g->pixels;
        }
        // NOTE umfeld now owns the pixel buffer and takes care of deleting it
    }

    // NOTE call to setup and setup_post is now handled in first call to `SDL_AppIterate`
    // handle_setup_draw_and_post();

    return SDL_APP_CONTINUE;
}

static void handle_event_window_resize() {
    if (umfeld::g != nullptr && umfeld::enable_graphics) {
        int new_width  = -1;
        int new_height = -1;
        umfeld::getWindowSize(new_width, new_height);
        if (new_width > 0 && new_height > 0) {
            umfeld::run_windowResized_callback(new_width, new_height);
        }
    }
}

static void handle_event(const SDL_Event& event, bool& app_is_running) {
    switch (event.type) {
        case SDL_EVENT_WINDOW_RESIZED:
            handle_event_window_resize();
            break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            // TODO implement
            break;
        case SDL_EVENT_WINDOW_SHOWN:
        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_MAXIMIZED:
            // umfeld::warning("TODO window status has changed ( e.g minimized, maximaized, shown, hidden )");
            break;
        case SDL_EVENT_QUIT:
            app_is_running = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (umfeld::use_esc_key_to_quit) {
                umfeld::key = static_cast<int>(event.key.key);
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    app_is_running = false;
                }
            }
            break;
        default: break;
    }
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    for (const umfeld::Subsystem* subsystem: umfeld::subsystems) {
        if (subsystem != nullptr) {
            if (subsystem->event != nullptr) {
                subsystem->event(event);
            }
        }
    }

    umfeld::event_cache.push_back(*event);

    /* only quit events */
    handle_event(*event, umfeld::_app_is_running);
    if (!umfeld::_app_is_running) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

static void handle_draw_in_setup() {
    for (const umfeld::Subsystem* subsystem: umfeld::subsystems) {
        if (subsystem != nullptr) {
            if (subsystem->draw_pre != nullptr) {
                subsystem->draw_pre();
            }
        }
    }

    // NOTE handle events just before drawing so that drawing method have effect in event callback
    handle_event_in_update_loop();
    if (umfeld::g != nullptr) {
        umfeld::g->set_default_graphics_state();
    }

    umfeld::run_setup_callback();

    for (const umfeld::Subsystem* subsystem: umfeld::subsystems) {
        if (subsystem != nullptr) {
            if (subsystem->draw_post != nullptr) {
                subsystem->draw_post();
            }
        }
    }

    umfeld::run_post_callback();
}

static void handle_draw() {
    for (const umfeld::Subsystem* subsystem: umfeld::subsystems) {
        if (subsystem != nullptr) {
            if (subsystem->draw_pre != nullptr) {
                subsystem->draw_pre();
            }
        }
    }

    // NOTE handle events just before drawing so that drawing method have effect in event callback
    handle_event_in_update_loop();
    umfeld::run_draw_callback();

    for (const umfeld::Subsystem* subsystem: umfeld::subsystems) {
        if (subsystem != nullptr) {
            if (subsystem->draw_post != nullptr) {
                subsystem->draw_post();
            }
        }
    }

    umfeld::run_post_callback();
}

static bool first_iterate = true;

SDL_AppResult SDL_AppIterate(void* appstate) {
    if (first_iterate) {
        first_iterate = false;
        handle_setup_draw_and_post();
        return SDL_APP_CONTINUE;
    }

    // NOTE moved events into draw loop to make darwing methods available
    // NOTE always process events even if noLoop() is active
    // handle_event_in_update_loop();

    // NOTE only update and draw if looping or first frame after noLoop()
    if (!umfeld::_app_no_loop || umfeld::frameCount == 0 || umfeld::_app_force_redraw) {

        const high_resolution_clock::time_point currentFrameTime = high_resolution_clock::now();
        const auto                              frameDuration    = duration_cast<duration<double>>(currentFrameTime - umfeld::lastFrameTime);
        const double                            frame_duration   = frameDuration.count();

        for (const umfeld::Subsystem* subsystem: umfeld::subsystems) {
            if (subsystem != nullptr && subsystem->update_loop != nullptr) {
                subsystem->update_loop();
            }
        }

        if (umfeld::run_update_in_thread) {
            request_update_thread();
        } else {
            umfeld::run_update_callback();
        }

        if (frame_duration >= umfeld::target_frame_duration) {
            handle_draw();

            if (frame_duration == 0) {
                umfeld::frameRate = 1;
            } else {
                umfeld::frameRate = static_cast<float>(1.0 / frame_duration);
            }

            umfeld::frameCount++;
            umfeld::lastFrameTime = currentFrameTime;
        }
    }

    if (umfeld::request_shutdown) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    /* stop update thread */
    if (umfeld::run_update_in_thread) {
        stop_update_thread();
    }

    // NOTE 1. call `void umfeld::shutdown()`(?)
    //      2. clean up subsytems e.g audio, graphics, ...
    for (const umfeld::Subsystem* subsystem: umfeld::subsystems) {
        if (subsystem != nullptr) {
            if (subsystem->shutdown != nullptr) {
                subsystem->shutdown();
            }
            // NOTE custom subsystems must be cleaned by client: `delete subsystem;`
        }
    }

    // NOTE clean up graphics + audio subsystem if created internally
    if (umfeld::subsystem_graphics != nullptr) {
        if (umfeld::handle_subsystem_graphics_cleanup) {
            delete umfeld::subsystem_graphics;
        }
        umfeld::subsystem_graphics = nullptr;
    }
    if (umfeld::subsystem_audio != nullptr) {
        if (umfeld::handle_subsystem_audio_cleanup) {
            delete umfeld::subsystem_audio;
        }
        umfeld::subsystem_audio = nullptr;
    }
    if (umfeld::subsystem_libraries != nullptr) {
        if (umfeld::handle_subsystem_libraries_cleanup) {
            delete umfeld::subsystem_libraries;
        }
        umfeld::subsystem_libraries = nullptr;
    }
    if (umfeld::subsystem_hid_events != nullptr) {
        if (umfeld::handle_subsystem_hid_events_cleanup) {
            delete umfeld::subsystem_hid_events;
        }
        umfeld::subsystem_hid_events = nullptr;
    }
    if (umfeld::pixels != nullptr) {
        delete[] umfeld::pixels;
        umfeld::pixels = nullptr;
        if (umfeld::g->displayDensity() > 1) {
            // NOTE delete the pixel buffer if it was created with a display density
            delete[] umfeld::g->pixels;
            umfeld::g->pixels = nullptr;
        } else {
            umfeld::g->pixels = nullptr;
        }
    }

    umfeld::subsystems.clear();

    umfeld::run_shutdown_callback();

    SDL_Quit();
}
