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

#include "Umfeld.h"
#include "Subsystems.h"

#ifndef DISABLE_AUDIO
#ifdef ENABLE_PORTAUDIO

#include <iostream>
#include <portaudio.h>
#include <chrono>
#include <thread>

#include "UmfeldFunctionsAdditional.h"
#include "PAudio.h"

namespace umfeld::subsystem {

    struct AudioDevice {
        std::string name;
        int         max_channels;
        float       sample_rate;
        int         logical_device_id;
    };

    class PAudioPortAudio;
    static std::vector<AudioDevice>      _audio_input_devices;
    static std::vector<AudioDevice>      _audio_output_devices;
    static std::vector<PAudioPortAudio*> _audio_devices;

    class PAudioPortAudio {
    public:
        PAudio*   audio{nullptr};
        PaStream* stream{nullptr};

        explicit PAudioPortAudio(PAudio* audio) : audio(audio) {
            if (this->audio == nullptr) {
                error_in_function("PAudioPortAudio: audio is nullptr");
                return;
            }

            if (audio->threaded) {
                console_in_function("threaded audio processing enabled");
            } else {
                console_in_function("PAudioPortAudio: threaded audio processing disabled");
            }

            console_in_function("PAudioPortAudio: update interval: ", update_interval.count(), "ms");

            if (!init()) {
                error_in_function("PAudioPortAudio: could not intialize");
                return;
            }

            update_interval = std::chrono::milliseconds(audio->buffer_size * 1000 / (audio->sample_rate * 4));

            if (audio->input_channels > 0 && audio->buffer_size > 0) {
                audio->input_buffer = new float[audio->buffer_size * audio->input_channels]{0};
            } else {
                if (audio->input_channels > 0 && audio->buffer_size == 0) {
                    warning_in_function("no input buffer created ( this might be intentional ). channel count > 0 but buffer size = ", audio->buffer_size);
                }
                audio->input_buffer = nullptr;
            }
            if (audio->output_channels > 0 && audio->buffer_size > 0) {
                audio->output_buffer = new float[audio->buffer_size * audio->output_channels]{0};
            } else {
                if (audio->output_channels > 0 && audio->buffer_size == 0) {
                    warning_in_function("no output buffer created ( this might be intentional ). channel count > 0 but buffer size = ", audio->buffer_size);
                }
                audio->output_buffer = nullptr;
            }
        }

        ~PAudioPortAudio() {
            shutdown();
        }

        void start() {
            if (stream == nullptr) {
                return;
            }
            isPaused        = false;
            const PaError s = Pa_IsStreamStopped(stream);
            // NOTE           ^^^ "Returns one (1) when the stream is stopped, ..."
            if (s == 1) {
                const PaError e = Pa_StartStream(stream);
                if (e != paNoError) {
                    error("Pa_StartStream failed: ", Pa_GetErrorText(e));
                }
            } else if (s < 0) {
                error("Pa_IsStreamStopped failed: ", Pa_GetErrorText(s));
            }
        }

        void stop() {
            if (stream == nullptr) {
                return;
            }
            isPaused        = true;
            const PaError a = Pa_IsStreamActive(stream);
            if (a == 1) {
                const PaError e = Pa_StopStream(stream);
                if (e != paNoError) {
                    error("Pa_StopStream failed: ", Pa_GetErrorText(e));
                }
            } else if (a < 0) {
                error("Pa_IsStreamActive failed: ", Pa_GetErrorText(a));
            }
        }

        void loop() {
            if (audio == nullptr || audio->threaded) {
                return;
            }
            if (isPaused) {
                return;
            }
            if (stream == nullptr) {
                return;
            }

            const auto now = std::chrono::high_resolution_clock::now();
            if (now - last_audio_update < update_interval) {
                return;
            }

            // Input availability
            const long availIn = Pa_GetStreamReadAvailable(stream);
            if (availIn < 0) {
                error("Pa_GetStreamReadAvailable failed: ", Pa_GetErrorText(availIn));
                last_audio_update = now;
                return;
            }
            if (audio->input_channels > 0 && availIn >= audio->buffer_size) {
                const PaError err = Pa_ReadStream(stream, audio->input_buffer, audio->buffer_size);
                if (err != paNoError) {
                    error("Error reading from stream: ", Pa_GetErrorText(err));
                    last_audio_update = now;
                    return;
                }
            }

            // Output availability
            const long availOut = Pa_GetStreamWriteAvailable(stream);
            if (availOut < 0) {
                error("Pa_GetStreamWriteAvailable failed: ", Pa_GetErrorText(availOut));
                last_audio_update = now;
                return;
            }

            // Run callbacks only when enough frames for both sides (or side unused)
            if ((availIn >= audio->buffer_size || audio->input_channels == 0) &&
                (availOut >= audio->buffer_size || audio->output_channels == 0)) {
                if (audio_device != nullptr) {
                    if (audio == audio_device) {
                        run_audioEvent_callback();
                        if (enable_audio_per_sample_processing) {
                            PAudio::acquire_audio_buffer_per_sample(audio);
                        }
                    }
                }
                run_audioEventPAudio_callback(*audio);
            }

            if (audio->output_channels > 0 && availOut >= audio->buffer_size) {
                const PaError err = Pa_WriteStream(stream, audio->output_buffer, audio->buffer_size);
                if (err != paNoError) {
                    error("Error writing to stream: ", Pa_GetErrorText(err));
                }
            }

            last_audio_update = now;
        }

        void shutdown() {
            if (stream != nullptr) {
                const PaError a = Pa_IsStreamActive(stream);
                // NOTE          ^^^ "Returns one (1) when the stream is active (ie playing or recording audio) ..."
                if (a == 1) {
                    Pa_StopStream(stream);
                }
                Pa_CloseStream(stream);
                stream = nullptr;
            }
            // TODO not entirely sure about the ownership of the 'audio'
            if (audio != nullptr) {
                delete[] audio->input_buffer;
                delete[] audio->output_buffer;
                audio->input_buffer  = nullptr;
                audio->output_buffer = nullptr;
            }
            if (audio != nullptr) {
                delete audio;
                audio = nullptr;
            }
        }

        bool start_stream() {
            const PaError result = Pa_StartStream(stream);
            if (result != paNoError) {
                error("Failed to start stream: ", Pa_GetErrorText(result), "");
                return false;
            }
            last_audio_update = std::chrono::high_resolution_clock::now();
            return true;
        }

    private:
        bool                                                        isPaused = false;
        std::chrono::milliseconds                                   update_interval;
        std::chrono::time_point<std::chrono::high_resolution_clock> last_audio_update;

        static int find_logical_device_id_by_name(const std::vector<AudioDevice>& devices, const std::string& name) {
            for (int i = 0; i < devices.size(); i++) {
                if (begins_with(devices[i].name, name)) {
                    console("found device ", devices[i].name);
                    return devices[i].logical_device_id;
                }
            }
            console("could not find device by name: '", name, "' using default device.");
            return DEFAULT_AUDIO_DEVICE;
        }

        static int find_logical_device_id_by_id(const std::vector<AudioDevice>& devices, const int device_id) {
            if (device_id >= 0 && device_id < devices.size()) {
                console("found device by id: ", device_id, "[", devices[device_id].logical_device_id, "] ", devices[device_id].name);
                return devices[device_id].logical_device_id;
            }
            console("could not find device by id '", device_id, "' using default device.");
            return DEFAULT_AUDIO_DEVICE;
        }

        static int audio_callback(const void*         inputBuffer,
                                  void*               outputBuffer,
                                  const unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo* /*timeInfo*/,
                                  PaStreamCallbackFlags /*statusFlags*/,
                                  void* userData) {
            const auto* audio = static_cast<PAudio*>(userData);

            if (audio->input_channels > 0 && inputBuffer != nullptr) {
                memcpy(audio->input_buffer, inputBuffer, framesPerBuffer * audio->input_channels * sizeof(float));
            }

            if (audio_device != nullptr) {
                if (audio == audio_device) {
                    run_audioEvent_callback();
                }
            }
            run_audioEventPAudio_callback(*audio);

            if (audio->output_channels > 0 && outputBuffer != nullptr) {
                memcpy(outputBuffer, audio->output_buffer, framesPerBuffer * audio->output_channels * sizeof(float));
            }

            return paContinue;
        }


        bool init() {
            if (audio == nullptr) {
                error("PAudioPortAudio: audio is nullptr");
                return false;
            }
            if (audio->input_channels == 0 && audio->output_channels == 0) {
                error("PAudioPortAudio: no input or output channels specified");
                return false;
            }

            int _audio_input_device  = DEFAULT_AUDIO_DEVICE;
            int _audio_output_device = DEFAULT_AUDIO_DEVICE;

            /* try to find audio devices */
            if (audio->input_device_id == AUDIO_DEVICE_FIND_BY_NAME && audio->input_device_name != DEFAULT_AUDIO_DEVICE_NAME) {
                _audio_input_device = find_logical_device_id_by_name(_audio_input_devices, audio->input_device_name);
            } else if (audio->input_device_id > DEFAULT_AUDIO_DEVICE) {
                _audio_input_device = find_logical_device_id_by_id(_audio_input_devices, audio->input_device_id);
            }
            if (audio->output_device_id == AUDIO_DEVICE_FIND_BY_NAME && audio->output_device_name != DEFAULT_AUDIO_DEVICE_NAME) {
                _audio_output_device = find_logical_device_id_by_name(_audio_output_devices, audio->output_device_name);
            } else if (audio->output_device_id > DEFAULT_AUDIO_DEVICE) {
                _audio_output_device = find_logical_device_id_by_id(_audio_output_devices, audio->output_device_id);
            }

            /* use default devices */

            if (_audio_input_device == DEFAULT_AUDIO_DEVICE) {
                _audio_input_device = Pa_GetDefaultInputDevice();
                console(fl("using default input device with ID"), _audio_input_device);
            }
            if (_audio_output_device == DEFAULT_AUDIO_DEVICE) {
                _audio_output_device = Pa_GetDefaultOutputDevice();
                console(fl("using default output device with ID"), _audio_output_device);
            }

            console("Opening audio device (input/output): (", _audio_input_device, "/", _audio_output_device, ")");

            const PaDeviceInfo*  _input_device_info          = Pa_GetDeviceInfo(_audio_input_device);
            const PaHostApiInfo* _input_device_host_api_info = Pa_GetHostApiInfo(_input_device_info->hostApi);
            console("Opening input stream for device with ID : ", _input_device_info->name,
                    "( Host API: ", _input_device_host_api_info->name,
                    ", Channels (input): ", static_cast<int>(audio->input_channels),
                    " ) ... ");

            const PaDeviceInfo*  _output_device_info          = Pa_GetDeviceInfo(_audio_output_device);
            const PaHostApiInfo* _output_device_host_api_info = Pa_GetHostApiInfo(_output_device_info->hostApi);
            console("Opening output stream for device with ID: ", _output_device_info->name,
                    "( Host API: ", _output_device_host_api_info->name,
                    ", Channels (output): ", static_cast<int>(audio->output_channels),
                    " ) ... ");

            /* handle default values: input channels, output channels, sample_rate, audio_buffer_size */
            if (audio->input_channels == DEFAULT_INPUT_CHANNELS) {
                // NOTE default to stereo if possible
                const int default_input_channels = _input_device_info->maxInputChannels < DEFAULT_INPUT_CHANNELS_FALLBACK ? _input_device_info->maxInputChannels : DEFAULT_INPUT_CHANNELS_FALLBACK;
                console(fl("input channels set to default"), default_input_channels);
                audio->input_channels = default_input_channels;
            }
            if (audio->output_channels == DEFAULT_OUTPUT_CHANNELS) {
                // NOTE default to stereo if possible
                const int default_output_channels = _output_device_info->maxOutputChannels < DEFAULT_OUTPUT_CHANNELS_FALLBACK ? _output_device_info->maxOutputChannels : DEFAULT_OUTPUT_CHANNELS_FALLBACK;
                console(fl("output channels set to default"), default_output_channels);
                audio->output_channels = default_output_channels;
            }
            if (audio->sample_rate == DEFAULT_SAMPLE_RATE) {
                console(fl("sample rate set to default"), "in(", _input_device_info->defaultSampleRate, ") out(", _output_device_info->defaultSampleRate, ")");
                if (_input_device_info->defaultSampleRate != _output_device_info->defaultSampleRate) {
                    warning("input and output sample rate differ (", _input_device_info->defaultSampleRate, " != ", _output_device_info->defaultSampleRate, ") using max value");
                }
                audio->sample_rate = std::max(_input_device_info->defaultSampleRate, _output_device_info->defaultSampleRate);
            }
            if (audio->buffer_size == DEFAULT_AUDIO_BUFFER_SIZE) {
                const int input_block_size  = static_cast<int>(_input_device_info->defaultHighInputLatency * audio->sample_rate + 0.5);
                const int output_block_size = static_cast<int>(_output_device_info->defaultHighOutputLatency * audio->sample_rate + 0.5);
                console(fl("buffer size set to default:"), "in(", input_block_size, ") out(", output_block_size, ")");
                if (input_block_size != output_block_size) {
                    warning("input and output block size differ (", input_block_size, " != ", output_block_size, ") using max value");
                }
                audio->buffer_size = std::max(input_block_size, output_block_size);
                // TODO not sure with the computed buffer sizes
                warning("computed buffer size seems off, falling back to ", DEFAULT_AUDIO_BUFFER_SIZE_FALLBACK, " (WIP)");
                audio->buffer_size = DEFAULT_AUDIO_BUFFER_SIZE_FALLBACK;
                warning("default buffer size: ", audio->buffer_size);
            }

            /* input */

            constexpr PaTime         LATENCY_SCALER                 = 2.0;
            constexpr PaSampleFormat SAMPLE_FORMAT                  = paFloat32;                                             // TODO maybe also support other formats? if e.g `paInt16` is supported remove `paDitherOff` flag
            constexpr PaStreamFlags  STREAM_FLAGS_NON_BLOCKING_MODE = paDitherOff | paPrimeOutputBuffersUsingStreamCallback; // NOTE not using `paClipOff`
            constexpr PaStreamFlags  STREAM_FLAGS_BLOCKING_MODE     = paDitherOff;

            PaStreamParameters inputParams;
            if (audio->input_channels > 0) {
                inputParams.device = _audio_input_device;
                if (inputParams.device == paNoDevice) {
                    error("No default input device found.");
                    return false;
                }

                const PaDeviceInfo* input_device_info = Pa_GetDeviceInfo(inputParams.device);
                if (input_device_info == nullptr) {
                    error("No output device info found.");
                    return false;
                }

                const int input_channels = input_device_info->maxInputChannels;
                if (input_channels < audio->input_channels) {
                    warning("Requested input channels: ", audio->input_channels,
                            " but device only supports: ", input_channels, ".",
                            " Setting input channels to: ", input_channels);
                    audio->input_channels = input_channels;
                }

                inputParams.channelCount              = audio->input_channels;
                inputParams.sampleFormat              = SAMPLE_FORMAT;
                inputParams.suggestedLatency          = input_device_info->defaultLowInputLatency * LATENCY_SCALER;
                inputParams.hostApiSpecificStreamInfo = nullptr;

                const char* device_name  = input_device_info->name;
                audio->input_device_name = device_name;
            } else {
                inputParams.device       = paNoDevice; // NOTE a bit of a hack to avoid issues with no input device
                inputParams.channelCount = 0;
                audio->input_device_name = DEFAULT_AUDIO_DEVICE_NOT_USED;
            }

            /* output */

            PaStreamParameters outputParams;
            if (audio->output_channels > 0) {
                outputParams.device = _audio_output_device;
                if (outputParams.device == paNoDevice) {
                    error("No default output device found.");
                    return false;
                }

                const PaDeviceInfo* output_device_info = Pa_GetDeviceInfo(outputParams.device);
                if (output_device_info == nullptr) {
                    error("No output device info found.");
                    return false;
                }
                const double output_device_sample_rate = output_device_info->defaultSampleRate;
                if (output_device_sample_rate != audio->sample_rate) {
                    warning("Requested sample rate: ", audio->sample_rate,
                            " but device only supports: ", output_device_sample_rate, ".",
                            " Setting sample rate to: ", output_device_sample_rate);
                    audio->sample_rate = static_cast<int>(output_device_sample_rate);
                }

                const int output_channels = output_device_info->maxOutputChannels;
                if (output_channels < audio->output_channels) {
                    warning("Requested output channels: ", audio->output_channels,
                            " but device only supports: ", output_channels, ".",
                            " Setting input channels to: ", output_channels);
                    audio->output_channels = output_channels;
                }

                outputParams.channelCount              = audio->output_channels;
                outputParams.sampleFormat              = SAMPLE_FORMAT;
                outputParams.suggestedLatency          = output_device_info->defaultLowOutputLatency * LATENCY_SCALER;
                outputParams.hostApiSpecificStreamInfo = nullptr;

                const char* device_name   = output_device_info->name;
                audio->output_device_name = device_name;
            } else {
                outputParams.device       = paNoDevice; // NOTE a bit of a hack to avoid issues with no input device
                outputParams.channelCount = 0;
                audio->output_device_name = DEFAULT_AUDIO_DEVICE_NOT_USED;
            }

            // --- Open Duplex Stream (input + output) ---
            PaError err;
            if (audio->threaded) {
                console("Opening audio stream in threaded mode");
                err = Pa_OpenStream(
                    &stream,
                    audio->input_channels > 0 ? &inputParams : nullptr,
                    audio->output_channels > 0 ? &outputParams : nullptr,
                    audio->sample_rate,
                    audio->buffer_size,
                    STREAM_FLAGS_NON_BLOCKING_MODE,
                    audio_callback,
                    audio // pointer to audio struct (as user data)
                );
            } else {
                console("Opening audio stream in non-threaded mode");
                err = Pa_OpenStream(
                    &stream,
                    audio->input_channels > 0 ? &inputParams : nullptr,
                    audio->output_channels > 0 ? &outputParams : nullptr,
                    audio->sample_rate,
                    audio->buffer_size,
                    STREAM_FLAGS_BLOCKING_MODE,
                    nullptr, // No callback (blocking mode)
                    nullptr  // No user data
                );
            }

            if (err != paNoError) {
                error("audio->audio_input_channels : ", audio->input_channels);
                error("audio->audio_output_channels: ", audio->output_channels);
                error("Failed to open stream: ", Pa_GetErrorText(err), "");
                return false;
            }

            start_stream();
            stop();

            return true;
        }
    };

    static void draw_pre() {}
    static void draw_post() {}
    static void event(SDL_Event* event) {}

    static void set_flags(uint32_t& subsystem_flags) {
        // NOTE no need to init audio when using portaudio
        // subsystem_flags |= SDL_INIT_AUDIO;
    }

    int print_audio_devices() {
        const int numDevices = Pa_GetDeviceCount();
        if (numDevices < 0) {
            error("+++ error 'Pa_CountDevices' returned ", numDevices);
            error("+++ error when counting devices: ", Pa_GetErrorText(numDevices));
            return -1;
        }

        console("Found ", numDevices, " audio devices:");
        for (int i = 0; i < numDevices; i++) {
            const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
            console("Device ", i, ": ", deviceInfo->name,
                    "  Max input channels: ", deviceInfo->maxInputChannels,
                    "  Max output channels: ", deviceInfo->maxOutputChannels,
                    "  Default sample rate: ", deviceInfo->defaultSampleRate);

            const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
            if (hostInfo) {
                console("  Host API: ", hostInfo->name);
            }
        }
        console("---");
        return 0;
    }

    static bool init() {
        console("initializing PortAudio audio system");
        const PaError err = Pa_Initialize();
        if (err != paNoError) {
            error("Error message: ", Pa_GetErrorText(err));
            return false;
        }
        print_audio_devices();
        return true;
    }

    static void update_loop() {
        for (const auto device: _audio_devices) {
            if (device != nullptr) {
                device->loop();
            }
        }
    }

    static void shutdown() {
        for (const auto device: _audio_devices) {
            if (device != nullptr) {
                device->shutdown();
                // TODO again … who cleans up the buffers in PAudio i.e `ad->audio`? align with SDL
                delete device;
            }
        }
        if (!_audio_devices.empty()) {
            _audio_devices.clear();
        }
        const PaError result = Pa_Terminate();
        if (result != paNoError) {
            error("Failed to terminate PortAudio: ", Pa_GetErrorText(result), "");
        }
    }

    static PAudioPortAudio* find_device(const PAudio* device) {
        for (const auto d: _audio_devices) {
            if (d->audio == device) {
                return d;
            }
        }
        return nullptr;
    }

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    static void start(PAudio* device) {
        PAudioPortAudio* _audio = find_device(device);
        if (_audio != nullptr) {
            _audio->start();
        }
    }

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    static void stop(PAudio* device) {
        PAudioPortAudio* _audio = find_device(device);
        if (_audio != nullptr) {
            _audio->stop();
        }
    }

    static void setup_pre() {}

    static void setup_post() {
        for (const auto _device: _audio_devices) {
            if (_device != nullptr) {
                _device->start();
            }
        }
    }

    static void update_audio_devices() {
        _audio_input_devices.clear();
        _audio_output_devices.clear();
        const int numDevices = Pa_GetDeviceCount();
        if (numDevices < 0) {
            error("+++ PortAudio error when counting devices: ", numDevices, ": ", Pa_GetErrorText(numDevices));
        }
        for (int i = 0; i < numDevices; i++) {
            const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
            if (deviceInfo->maxInputChannels > 0) {
                AudioDevice _audio_device_info;
                _audio_device_info.name              = deviceInfo->name;
                _audio_device_info.max_channels      = deviceInfo->maxInputChannels;
                _audio_device_info.sample_rate       = deviceInfo->defaultSampleRate;
                _audio_device_info.logical_device_id = i;
                _audio_input_devices.push_back(_audio_device_info);
            }
            if (deviceInfo->maxOutputChannels > 0) {
                AudioDevice _audio_device_info;
                _audio_device_info.name              = deviceInfo->name;
                _audio_device_info.max_channels      = deviceInfo->maxOutputChannels;
                _audio_device_info.sample_rate       = deviceInfo->defaultSampleRate;
                _audio_device_info.logical_device_id = i;
                _audio_output_devices.push_back(_audio_device_info);
            }
        }
    }

    static PAudio* create_audio(const AudioUnitInfo* device_info) {
        update_audio_devices();
        console("update_audio_devices");
        int i = 0;
        console("    INPUT DEVICES");
        for (auto ad: _audio_input_devices) {
            console("    [", i, "]::", ad.name, " (", ad.max_channels, " channels, ", ad.sample_rate, " Hz)");
            i++;
        }
        i = 0;
        console("    OUTPUT DEVICES");
        for (auto ad: _audio_output_devices) {
            console("    [", i, "]::", ad.name, " (", ad.max_channels, " channels, ", ad.sample_rate, " Hz)");
            i++;
        }

        const auto _device = new PAudio{device_info};
        _device->unique_id = audio_unique_device_id++;
        // ReSharper disable once CppDFAMemoryLeak
        const auto _audio = new PAudioPortAudio{_device};
        _audio->stop();
        _audio_devices.push_back(_audio);
        // NOTE newing the arrays happens in class … need to check and align with SDL implementation
        // audio_device->input_buffer  = new float[audio_device->audio_input_channels * audio_device->buffer_size]{};
        // audio_device->output_buffer = new float[audio_device->audio_output_channels * audio_device->buffer_size]{};
        return _device;
    }

    static const char* name() {
        return "PortAudio";
    }
} // namespace umfeld::subsystem

umfeld::SubsystemAudio* umfeld_create_subsystem_audio_portaudio() {
    auto* audio         = new umfeld::SubsystemAudio{};
    audio->set_flags    = umfeld::subsystem::set_flags;
    audio->init         = umfeld::subsystem::init;
    audio->setup_pre    = umfeld::subsystem::setup_pre;
    audio->setup_post   = umfeld::subsystem::setup_post;
    audio->update_loop  = umfeld::subsystem::update_loop;
    audio->draw_pre     = umfeld::subsystem::draw_pre;
    audio->draw_post    = umfeld::subsystem::draw_post;
    audio->shutdown     = umfeld::subsystem::shutdown;
    audio->event        = umfeld::subsystem::event;
    audio->name         = umfeld::subsystem::name;
    audio->start        = umfeld::subsystem::start;
    audio->stop         = umfeld::subsystem::stop;
    audio->create_audio = umfeld::subsystem::create_audio;
    return audio;
}

#endif // ENABLE_PORTAUDIO
#else

umfeld::SubsystemAudio* umfeld_create_subsystem_audio_portaudio() {
    return nullptr;
}
#endif // DISABLE_AUDIO
