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
        int         max_input_channels;
        float       sample_rate;
        int         logical_device_id;
    };

    class PAudioPortAudio;
    static std::vector<AudioDevice>      audio_input_devices;
    static std::vector<AudioDevice>      audio_output_devices;
    static std::vector<PAudioPortAudio*> audio_devices;

    class PAudioPortAudio {
    public:
        PAudio*   audio{nullptr};
        PaStream* stream{nullptr};

        explicit PAudioPortAudio(PAudio* audio) : audio(audio),
                                                  update_interval((audio->buffer_size * 1000) / (audio->sample_rate * 4)) {
            if (audio == nullptr) {
                error("PAudioPortAudio: audio is nullptr");
                return;
            }
            this->audio = audio;
            if (audio->threaded) {
                console("PAudioPortAudio: threaded audio processing enabled");
            } else {
                console("PAudioPortAudio: threaded audio processing disabled");
            }
            console("PAudioPortAudio: update interval: ", update_interval.count(), "ms");
            if (!init()) {
                error("PAudioPortAudio: could not intialize");
                return;
            }
            if (audio->input_channels > 0) {
                audio->input_buffer = new float[audio->buffer_size * audio->input_channels]{0};
            } else {
                audio->input_buffer = nullptr;
            }
            if (audio->output_channels > 0) {
                audio->output_buffer = new float[audio->buffer_size * audio->output_channels]{0};
            } else {
                audio->output_buffer = nullptr;
            }
        }

        void start() {
            if (stream == nullptr) {
                return;
            }

            isPaused = false;
            if (Pa_IsStreamStopped(stream)) {
                Pa_StartStream(stream);
            }
        }

        void stop() {
            if (stream == nullptr) {
                return;
            }
            isPaused = true;
            if (Pa_IsStreamActive(stream)) {
                Pa_StopStream(stream);
            }
        }

        void loop() {
            if (audio == nullptr) {
                return;
            }

            if (!audio->threaded) {

                if (isPaused) {
                    return;
                }

                const auto now = std::chrono::high_resolution_clock::now();
                if (now - last_audio_update >= update_interval) {
                    // check if input is available
                    const long availableInputFrames = Pa_GetStreamReadAvailable(stream);
                    if (availableInputFrames >= audio->buffer_size) {
                        const PaError err = Pa_ReadStream(stream, audio->input_buffer, audio->buffer_size);
                        if (err != paNoError) {
                            error("Error reading from stream: ", Pa_GetErrorText(err));
                            return;
                        }
                    }

                    // check if output buffer has space
                    const long availableOutputFrames = Pa_GetStreamWriteAvailable(stream);

                    // call `audioevent` respecting non-present audio devices and available frames
                    if ((availableInputFrames >= audio->buffer_size || audio->input_channels == 0) &&
                        (availableOutputFrames >= audio->buffer_size || audio->output_channels == 0)) {
                        if (a != nullptr && audio == umfeld::a) {
                            run_audioEvent_callback();
                        }
                        run_audioEventPAudio_callback(*audio);
                    }

                    if (availableOutputFrames >= audio->buffer_size) {
                        const PaError err = Pa_WriteStream(stream, audio->output_buffer, audio->buffer_size);
                        if (err != paNoError) {
                            error("Error writing to stream: ", Pa_GetErrorText(err), "");
                            return;
                        }
                    }

                    last_audio_update = now;
                }
            }
        }

        void shutdown() const {
            Pa_StopStream(stream);
            Pa_CloseStream(stream);
            delete[] audio->input_buffer;
            delete[] audio->output_buffer;
            audio->input_buffer  = nullptr;
            audio->output_buffer = nullptr;
        }

    private:
        bool                                                        isPaused = false;
        std::chrono::milliseconds                                   update_interval;
        std::chrono::time_point<std::chrono::high_resolution_clock> last_audio_update;

        int find_logical_device_id_by_name(const std::vector<AudioDevice>& devices, const std::string& name) const {
            for (int i = 0; i < devices.size(); i++) {
                if (begins_with(devices[i].name, name)) {
                    console("found device ", devices[i].name);
                    return devices[i].logical_device_id;
                }
            }
            console("could not find device by name: '", name, "' using default device.");
            return DEFAULT_AUDIO_DEVICE;
        }

        int find_logical_device_id_by_id(const std::vector<AudioDevice>& devices, const int device_id) const {
            if (device_id >= 0 && device_id < devices.size()) {
                console("found device by id: ", device_id, "[", devices[device_id].logical_device_id, "] ", devices[device_id].name);
                return devices[device_id].logical_device_id;
            }
            console("could not find device by id '", device_id, "' using default device.");
            return DEFAULT_AUDIO_DEVICE;
        }

        static int audio_callback(const void*                     inputBuffer,
                                  void*                           outputBuffer,
                                  unsigned long                   framesPerBuffer,
                                  const PaStreamCallbackTimeInfo* timeInfo,
                                  PaStreamCallbackFlags           statusFlags,
                                  void*                           userData) {
            PAudio* audio = (PAudio*) userData;

            // Copy input data into your buffer (if any input)
            if (audio->input_channels > 0 && inputBuffer != nullptr) {
                memcpy(audio->input_buffer, inputBuffer, framesPerBuffer * audio->input_channels * sizeof(float));
            }

            // Call your event/callback function
            if (audio == umfeld::a) {
                run_audioEvent_callback();
            }
            run_audioEventPAudio_callback(*audio);

            // Copy output data from your buffer (if any output)
            if (audio->output_channels > 0 && outputBuffer != nullptr) {
                memcpy(outputBuffer, audio->output_buffer, framesPerBuffer * audio->output_channels * sizeof(float));
            }

            return paContinue; // Keep going
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
                _audio_input_device = find_logical_device_id_by_name(audio_input_devices, audio->input_device_name);
            } else if (audio->input_device_id > DEFAULT_AUDIO_DEVICE) {
                _audio_input_device = find_logical_device_id_by_id(audio_input_devices, audio->input_device_id);
            }
            if (audio->output_device_id == AUDIO_DEVICE_FIND_BY_NAME && audio->output_device_name != DEFAULT_AUDIO_DEVICE_NAME) {
                _audio_output_device = find_logical_device_id_by_name(audio_output_devices, audio->output_device_name);
            } else if (audio->output_device_id > DEFAULT_AUDIO_DEVICE) {
                _audio_output_device = find_logical_device_id_by_id(audio_output_devices, audio->output_device_id);
            }

            /* use default devices */
            if (_audio_input_device == DEFAULT_AUDIO_DEVICE) {
                _audio_input_device = Pa_GetDefaultInputDevice();
                console("using default input device with ID : ", _audio_input_device);
            }
            if (_audio_output_device == DEFAULT_AUDIO_DEVICE) {
                _audio_output_device = Pa_GetDefaultOutputDevice();
                console("using default output device with ID: ", _audio_output_device);
            }

            console("Opening audio device (input/output): (",
                    _audio_input_device, "/",
                    _audio_output_device, ")");

            const PaDeviceInfo*  _device_info   = Pa_GetDeviceInfo(_audio_output_device);
            const PaHostApiInfo* _host_api_info = Pa_GetHostApiInfo(_device_info->hostApi);

            console("Opening stream for device with ID: ", _device_info->name,
                    "( Host API: ", _host_api_info->name,
                    ", Channels (input/output): (", audio->input_channels,
                    "/", audio->output_channels, ")",
                    " ) ... ");

            /* input */

            PaStreamParameters inputParams;
            if (audio->input_channels > 0) {
                inputParams.device = _audio_input_device;
                if (inputParams.device == paNoDevice) {
                    error("No default input device found.");
                    return false;
                }

                const int input_channels = Pa_GetDeviceInfo(inputParams.device)->maxInputChannels;
                if (input_channels < audio->input_channels) {
                    warning("Requested input channels: ", audio->input_channels,
                            " but device only supports: ", input_channels, ".",
                            " Setting input channels to: ", input_channels);
                    audio->input_channels = input_channels;
                }

                inputParams.channelCount              = audio->input_channels;
                inputParams.sampleFormat              = paFloat32;
                inputParams.suggestedLatency          = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
                inputParams.hostApiSpecificStreamInfo = nullptr;

                const char* device_name  = Pa_GetDeviceInfo(inputParams.device)->name;
                audio->input_device_name = device_name;
            } else {
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

                const int output_channels = Pa_GetDeviceInfo(outputParams.device)->maxOutputChannels;
                if (output_channels < audio->output_channels) {
                    warning("Requested output channels: ", audio->output_channels,
                            " but device only supports: ", output_channels, ".",
                            " Setting input channels to: ", output_channels);
                    audio->output_channels = output_channels;
                }

                outputParams.channelCount              = audio->output_channels;
                outputParams.sampleFormat              = paFloat32;
                outputParams.suggestedLatency          = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
                outputParams.hostApiSpecificStreamInfo = nullptr;

                const char* device_name   = Pa_GetDeviceInfo(outputParams.device)->name;
                audio->output_device_name = device_name;
            } else {
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
                    paClipOff,
                    audio_callback, // Pointer to your callback function
                    audio           // Pointer to your audio struct (as user data)
                );
            } else {
                console("Opening audio stream in non-threaded mode");
                err = Pa_OpenStream(
                    &stream,
                    audio->input_channels > 0 ? &inputParams : nullptr,
                    audio->output_channels > 0 ? &outputParams : nullptr,
                    audio->sample_rate,
                    audio->buffer_size,
                    paClipOff, // No clipping
                    nullptr,   // No callback (blocking mode)
                    nullptr    // No user data
                );
            }

            if (err != paNoError) {
                error("audio->audio_input_channels : ", audio->input_channels);
                error("audio->audio_output_channels: ", audio->output_channels);
                error("Failed to open stream: ", Pa_GetErrorText(err), "");
                return false;
            }

            Pa_StartStream(stream);
            last_audio_update = std::chrono::high_resolution_clock::now();

            return true;
        }
    };

    static void setup_post() {}
    static void draw_pre() {}
    static void draw_post() {}
    static void event(SDL_Event* event) {}

    static void set_flags(uint32_t& subsystem_flags) {
        subsystem_flags |= SDL_INIT_AUDIO;
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
        // TODO this is a bit hacky, but it works for now. look into this …
        Pa_Initialize();
        print_audio_devices();
        Pa_Terminate();

        console("initializing PortAudio audio system");
        const PaError err = Pa_Initialize();
        if (err != paNoError) {
            error("Error message: ", Pa_GetErrorText(err));
            return false;
        }

        return true;
    }

    static void update_loop() {
        for (const auto device: audio_devices) {
            if (device != nullptr) {
                device->loop();
            }
        }
    }

    static void shutdown() {
        for (const auto device: audio_devices) {
            if (device != nullptr) {
                device->shutdown();
                // TODO again … who cleans up the buffers in PAudio i.e `ad->audio`? align with SDL
                delete device;
            }
        }
        Pa_Terminate();
    }

    static PAudioPortAudio* find_device(const PAudio* device) {
        for (const auto d: audio_devices) {
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

    static void setup_pre() {
        for (const auto _device: audio_devices) {
            if (_device != nullptr) {
                _device->start();
            }
        }
    }

    static void update_audio_devices() {
        audio_input_devices.clear();
        audio_output_devices.clear();
        const int numDevices = Pa_GetDeviceCount();
        if (numDevices < 0) {
            error("+++ PortAudio error when counting devices: ", numDevices, ": ", Pa_GetErrorText(numDevices));
        }
        for (int i = 0; i < numDevices; i++) {
            const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
            if (deviceInfo->maxInputChannels > 0) {
                AudioDevice _audio_device_info;
                _audio_device_info.name               = deviceInfo->name;
                _audio_device_info.max_input_channels = deviceInfo->maxInputChannels;
                _audio_device_info.sample_rate        = deviceInfo->defaultSampleRate;
                _audio_device_info.logical_device_id  = i;
                audio_input_devices.push_back(_audio_device_info);
            }
            if (deviceInfo->maxOutputChannels > 0) {
                AudioDevice _audio_device_info;
                _audio_device_info.name               = deviceInfo->name;
                _audio_device_info.max_input_channels = deviceInfo->maxOutputChannels;
                _audio_device_info.sample_rate        = deviceInfo->defaultSampleRate;
                _audio_device_info.logical_device_id  = i;
                audio_output_devices.push_back(_audio_device_info);
            }
        }
    }

    static PAudio* create_audio(const AudioUnitInfo* device_info) {
        update_audio_devices();
        console("update_audio_devices");
        int i = 0;
        console("    INPUT DEVICES");
        for (auto ad: audio_input_devices) {
            console("    [", i, "]::", ad.name, " (", ad.max_input_channels, " channels, ", ad.sample_rate, " Hz)");
            i++;
        }
        i = 0;
        console("    OUTPUT DEVICES");
        for (auto ad: audio_output_devices) {
            console("    [", i, "]::", ad.name, " (", ad.max_input_channels, " channels, ", ad.sample_rate, " Hz)");
            i++;
        }

        const auto _device = new PAudio{device_info};
        _device->unique_id = audio_unique_device_id++;
        // ReSharper disable once CppDFAMemoryLeak
        const auto _audio = new PAudioPortAudio{_device};
        _audio->stop();
        audio_devices.push_back(_audio);
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
