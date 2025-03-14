/*
* Umgebung
 *
 * This file is part of the *Umgebung* library (https://github.com/dennisppaul/umgebung).
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

#include "Umgebung.h"
#include "UmgebungSubsystems.h"
#include "PAudio.h"

namespace umgebung {
    struct PAudioSDL {
        PAudio*          audio_device{nullptr};
        int              logical_input_device_id{0};
        SDL_AudioStream* sdl_input_stream{nullptr};
        int              logical_output_device_id{0};
        SDL_AudioStream* sdl_output_stream{nullptr};
    };

    struct AudioUnitInfoSDL : AudioUnitInfo {
        int logical_device_id{0};
    };

    static std::vector<PAudioSDL*> _audio_devices;
    // TODO callback mode is not working well.
    //      also it appears to be slightly deprecated by SDL devs.
    //      maybe drop it at some point ...
    // static bool _audio_device_callback_mode = false;

    static void setup_pre() {}
    static void setup_post() {}
    static void draw_pre() {}
    static void draw_post() {}
    static void event(SDL_Event* event) {}

    static void set_flags(uint32_t& subsystem_flags) {
        subsystem_flags |= SDL_INIT_AUDIO;
    }

    // static bool request_audio_samples(PAudioSDL* const _device) {
    //     if (SDL_AudioDevicePaused(_device->logical_device_id)) {
    //         return true;
    //     }
    //     if (SDL_AudioStreamDevicePaused(_device->sdl_stream)) {
    //         return true;
    //     }
    //
    //     const int request_num_sample_frames = _device->audio_device->buffer_size;
    //     if (SDL_GetAudioStreamQueued(_device->sdl_stream) < request_num_sample_frames) {
    //         // NOTE for default audio device
    //         if (_device->audio_device == a) {
    //             audioEvent();
    //         }
    //
    //         // NOTE for all registered audio devices ( including default audio device )
    //         audioEvent(*_device->audio_device);
    //
    //         const float* buffer      = _device->audio_device->output_buffer;
    //         const int    buffer_size = _device->audio_device->buffer_size * _device->audio_device->output_channels;
    //         SDL_PutAudioStreamData(_device->sdl_stream, buffer, buffer_size * sizeof(float));
    //     }
    //     return false;
    // }

    // static SDL_AudioDeviceID _TMP_input_device_id = -1;
    // static SDL_AudioStream*  _TMP_input_stream = nullptr;

    // static void SDLCALL received_audio_samples_callback(void* userdata, SDL_AudioStream* astream, int additional_amount, int total_amount) {
    //     int input_bytes_available = SDL_GetAudioStreamAvailable(_TMP_input_stream);
    //     console("input_bytes_available [", _TMP_input_device_id, "]: ", input_bytes_available);
    //     console("additional_amount: ", additional_amount, " :: total_amount: ", total_amount);
    //     float buffer[input_bytes_available / sizeof(float)];
    //     SDL_GetAudioStreamData(astream, buffer, input_bytes_available);
    //     // extern SDL_DECLSPEC int SDLCALL SDL_GetAudioStreamData(SDL_AudioStream *stream, void *buf, int len);
    //     // SDL_GetAudioStreamData(_TMP_input_stream,,);
    // }

    // static void SDLCALL request_audio_samples_callback(void* userdata, SDL_AudioStream* astream, int additional_amount, int total_amount) {
    //
    //     /* total_amount is how much data the audio stream is eating right now, additional_amount is how much more it needs
    //     than what it currently has queued (which might be zero!). You can supply any amount of data here; it will take what
    //     it needs and use the extra later. If you don't give it enough, it will take everything and then feed silence to the
    //     hardware for the rest. Ideally, though, we always give it what it needs and no extra, so we aren't buffering more
    //     than necessary. */
    //
    //     for (const auto _device: _audio_devices) {
    //         if (_device != nullptr &&
    //             _device->audio_device != nullptr &&
    //             _device->sdl_stream != nullptr &&
    //             _device->sdl_stream == astream) {
    //             if (request_audio_samples(_device)) {
    //                 continue;
    //             }
    //         }
    //     }
    // }

    static void find_audio_devices(std::vector<AudioUnitInfoSDL>& devices,
                                   const int                      _num_playback_devices,
                                   const SDL_AudioDeviceID*       _audio_device_ids,
                                   const bool                     is_input_device) {
        if (_audio_device_ids != nullptr) {
            for (int i = 0; i < _num_playback_devices; i++) {
                const int   id    = _audio_device_ids[i];
                const char* _name = SDL_GetAudioDeviceName(id);
                if (_name == nullptr) {
                    console(i, "\tID: ", _audio_device_ids[i], "\tname: ", _name);
                    warning("failed to acquire audio device name for device:", _name);
                }
                SDL_AudioSpec spec;
                if (!SDL_GetAudioDeviceFormat(id, &spec, nullptr)) {
                    warning("failed to acquire audio device info for device:", _name);
                    continue;
                }
                AudioUnitInfoSDL _device;
                _device.logical_device_id = _audio_device_ids[i];
                _device.input_buffer      = nullptr;
                _device.input_channels    = 0;
                _device.output_buffer     = nullptr;
                _device.output_channels   = spec.channels;
                _device.buffer_size       = BUFFER_SIZE_UNDEFINED;
                _device.sample_rate       = spec.freq;
                if (is_input_device) {
                    _device.input_device_name = _name;
                } else {
                    _device.output_device_name = _name;
                }
                devices.push_back(_device);
            }
        }
    }

    static void find_audio_input_devices(std::vector<AudioUnitInfoSDL>& devices) {
        int                      _num_recording_devices;
        const SDL_AudioDeviceID* _audio_device_ids = SDL_GetAudioRecordingDevices(&_num_recording_devices);
        find_audio_devices(devices, _num_recording_devices, _audio_device_ids, true);
        // if (_audio_device_ids != nullptr) {
        //             for (int i = 0; i < _num_recording_devices; i++) {
        //                 const int   id    = _audio_device_ids[i];
        //                 const char* _name = SDL_GetAudioDeviceName(id);
        //                 if (_name == nullptr) {
        //                     console(i, "\tID: ", _audio_device_ids[i], "\tname: ", _name);
        //                     warning("failed to acquire audio device name for device:", _name);
        //                 }
        //                 SDL_AudioSpec spec;
        //                 if (!SDL_GetAudioDeviceFormat(id, &spec, nullptr)) {
        //                     warning("failed to acquire audio device info for device:", _name);
        //                     continue;
        //                 }
        //                 AudioUnitInfoSDL _device;
        //                 _device.logical_device_id = _audio_device_ids[i];
        //                 _device.input_buffer      = nullptr;
        //                 _device.input_channels    = spec.channels;
        //                 _device.output_buffer     = nullptr;
        //                 _device.output_channels   = 0;
        //                 _device.buffer_size       = BUFFER_SIZE_UNDEFINED;
        //                 _device.sample_rate       = spec.freq;
        //                 _device.name              = _name;
        //                 devices.push_back(_device);
        //             }
        //         }
    }

    static void find_audio_output_devices(std::vector<AudioUnitInfoSDL>& devices) {
        int                      _num_playback_devices;
        const SDL_AudioDeviceID* _audio_device_ids = SDL_GetAudioPlaybackDevices(&_num_playback_devices);
        find_audio_devices(devices, _num_playback_devices, _audio_device_ids, false);
    }

    static constexpr int column_width = 80;
    static constexpr int format_width = 40;

    static void separator_headline() {
        console(separator(true, column_width));
    }

    static void separator_subheadline() {
        console(separator(false, column_width));
    }

    static void print_device_info(const AudioUnitInfoSDL& device) {
        // TODO there is an incosistency here: AudioUnitInfoSDL only stores one logical device id although a unit is made up of two logical devices
        console(format_label("- [" + to_string(device.logical_device_id) + "]" + (device.logical_device_id > 9 ? " " : "") + device.input_device_name + "/" + device.output_device_name, format_width),
                "in: ", device.input_channels, ", ",
                "out: ", device.output_channels, ", ",
                device.sample_rate, " Hz");
    }

    std::vector<AudioUnitInfoSDL> get_audio_info() {
        separator_headline();
        console("AUDIO INFO");

        separator_subheadline();
        console("AUDIO DEVICE DRIVERS");
        separator_subheadline();
        const int _num_audio_drivers = SDL_GetNumAudioDrivers();
        for (int i = 0; i < _num_audio_drivers; ++i) {
            console("- [", i, "]\t", SDL_GetAudioDriver(i));
        }
        if (SDL_GetCurrentAudioDriver() != nullptr) {
            console("( current audio driver: '", SDL_GetCurrentAudioDriver(), "' )");
        }

        separator_subheadline();
        console("AUDIO INPUT DEVICES");
        separator_subheadline();
        std::vector<AudioUnitInfoSDL> _devices_found;
        find_audio_input_devices(_devices_found);
        for (auto d: _devices_found) {
            print_device_info(d);
        }

        separator_subheadline();
        console("AUDIO OUTPUT DEVICES");
        _devices_found.clear();
        find_audio_output_devices(_devices_found);
        for (auto d: _devices_found) {
            print_device_info(d);
        }
        separator_headline();

        return _devices_found;
    }

    static bool init() {
        // NOTE not sure if we should use the AudioStream concept. currently the mental model
        //     is slightly different: a subsystem correlates to an audio driver ( e.g SDL or PortAudio )
        //     and the actual audio devices are represented by `PAudio`. so each `create_audio` call
        //     would need to open a device and create a single stream for that device. this is different
        //     from `PGraphics` and graphics susbsytem where we have a single window and multiple contexts.

        // TODO see https://github.com/libsdl-org/SDL/blob/main/examples/audio

        separator_headline();
        console("initializing SDL audio system");
        separator_headline();
        get_audio_info();

        return true;
    }

    // static SDL_AudioStream* get_stream_from_paudio(const PAudio* device) {
    // for (int i = 0; i < _audio_devices.size(); i++) {
    //     if (_audio_devices[i]->audio_device == device) {
    //         return _audio_devices[i]->sdl_stream;
    //     }
    // }
    // return nullptr;
    // }

    static PAudioSDL* get_paudio_sdl_from_paudio(const PAudio* device) {
        for (int i = 0; i < _audio_devices.size(); i++) {
            if (_audio_devices[i]->audio_device == device) {
                return _audio_devices[i];
            }
        }
        return nullptr;
    }

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    void start(PAudio* device) {
        if (device == nullptr) {
            return;
        }

        const PAudioSDL* _paudio_sdl = get_paudio_sdl_from_paudio(device);

        if (_paudio_sdl == nullptr) {
            error("could not start audio device: ", device->input_device_name, "+", device->output_device_name);
            return;
        }

        /* bind audio input stream to device */

        if (_paudio_sdl->sdl_input_stream != nullptr) {
            if (!SDL_ResumeAudioStreamDevice(_paudio_sdl->sdl_input_stream)) {
                error("could not start audio device input stream: ", device->input_device_name);
            }
            // TODO maybe do this on device level:
            // SDL_ResumeAudioDevice(_paudio_sdl->logical_input_device_id);
        }

        /* bind audio output stream to device */

        if (_paudio_sdl->sdl_output_stream != nullptr) {
            if (!SDL_ResumeAudioStreamDevice(_paudio_sdl->sdl_output_stream)) {
                error("could not start audio device output stream: ", device->output_device_name);
            }
            // TODO maybe do this on device level:
            // if (SDL_ResumeAudioDevice(_paudio_sdl->logical_output_device_id)) {}
        }
    }

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    void stop(PAudio* device) {
        if (device == nullptr) {
            return;
        }

        const PAudioSDL* _paudio_sdl = get_paudio_sdl_from_paudio(device);

        if (_paudio_sdl == nullptr) {
            error("could not stop audio device: could not find ", device->input_device_name, "/", device->output_device_name);
            return;
        }

        if (_paudio_sdl->sdl_input_stream != nullptr) {
            if (!SDL_PauseAudioStreamDevice(_paudio_sdl->sdl_input_stream)) {
                error("could not stop audio device input stream: ", device->input_device_name);
            }
            // TODO maybe do this on device level:
            // SDL_ResumeAudioDevice(_paudio_sdl->logical_input_device_id);
        }

        if (_paudio_sdl->sdl_output_stream != nullptr) {
            if (!SDL_PauseAudioStreamDevice(_paudio_sdl->sdl_output_stream)) {
                error("could not stop audio device output stream: ", device->output_device_name);
            }
            // TODO maybe do this on device level:
            // SDL_ResumeAudioDevice(_paudio_sdl->logical_output_device_id);
        }
    }

    static void loop() {
        // NOTE consult https://wiki.libsdl.org/SDL3/Tutorials/AudioStream
        for (const auto _device: _audio_devices) {
            if (_device != nullptr &&
                _device->audio_device != nullptr) {

                const int _num_sample_frames = _device->audio_device->buffer_size;

                /* prepare samples from input stream */

                if (!SDL_AudioDevicePaused(_device->logical_input_device_id)) {
                    if (_device->sdl_input_stream != nullptr) {
                        SDL_AudioStream* _stream = _device->sdl_input_stream;
                        if (!SDL_AudioStreamDevicePaused(_stream)) {
                            const int input_bytes_available = SDL_GetAudioStreamAvailable(_stream);
                            const int num_required_bytes    = _num_sample_frames * _device->audio_device->input_channels * sizeof(float);
                            if (input_bytes_available >= num_required_bytes) {
                                float* buffer = _device->audio_device->input_buffer;
                                if (buffer != nullptr) {
                                    if (SDL_GetAudioStreamData(_stream, buffer, num_required_bytes) < 0) {
                                        console("could not get data from ", _device->audio_device->input_device_name, " input stream: ", SDL_GetError());
                                    }
                                }
                            }
                        }
                    }
                }

                /* request samples for output stream */

                if (!SDL_AudioDevicePaused(_device->logical_output_device_id)) {
                    if (_device->sdl_output_stream != nullptr) {
                        SDL_AudioStream* _stream = _device->sdl_output_stream;
                        if (!SDL_AudioStreamDevicePaused(_stream)) {
                            const int request_num_sample_frames = _device->audio_device->buffer_size;
                            if (SDL_GetAudioStreamQueued(_stream) < request_num_sample_frames) {
                                // NOTE for main audio device
                                if (a != nullptr) {
                                    if (_device->audio_device == a) {
                                        audioEvent();
                                    }
                                }

                                // NOTE for all registered audio devices ( including main audio device )
                                audioEvent(*_device->audio_device);

                                const int    num_processed_bytes = _num_sample_frames * _device->audio_device->output_channels * sizeof(float);
                                const float* buffer              = _device->audio_device->output_buffer;
                                if (buffer != nullptr) {
                                    if (!SDL_PutAudioStreamData(_stream, buffer, num_processed_bytes)) {
                                        console("could not send data to ", _device->audio_device->output_device_name, " output stream: ", SDL_GetError());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    static void shutdown() {
        for (const auto audio_device: _audio_devices) {
            if (audio_device != nullptr) {
                SDL_CloseAudioDevice(audio_device->logical_input_device_id);
                SDL_CloseAudioDevice(audio_device->logical_output_device_id);
                SDL_DestroyAudioStream(audio_device->sdl_input_stream);
                SDL_DestroyAudioStream(audio_device->sdl_output_stream);
                // NOTE below needs to be handled by caller of `create_audio`
                // delete(audio_device->audio_device);
                // delete[] audio_device->audio_device->input_buffer;
                // delete[] audio_device->audio_device->output_buffer;
                delete audio_device;
            }
        }
        _audio_devices.clear();
    }

    // static int find_audio_devices_by_name(const std::vector<AudioUnitInfo>& vector, const std::string& name) {
    //     for (const auto& device: vector) {
    //         if (begins_with(device.name, name)) {
    //             console("found audio device by name: ", name, " [", device., "]");
    //             return device.id;
    //         }
    //     }
    //     return AUDIO_DEVICE_NOT_FOUND;
    // }

    static void register_audio_devices(PAudio* device) {
        if (device == nullptr) {
            // ReSharper disable once CppDFAUnreachableCode
            return; // NOTE this should never happen …
        }

        // if (device->input_channels > 0 && device->output_channels > 0) {
        //     warning("NOT IMPLEMENTED: trying to create device with input and output. ",
        //             "currently only either out or input works. ",
        //             "defaulting to just output device.");
        //     device->input_channels = 0;
        // }

        // if (device->id == AUDIO_DEVICE_DEFAULT) {
        //     console("trying to create default audio device");
        //     if (device->input_channels > 0) {
        //         device->id = SDL_AUDIO_DEVICE_DEFAULT_RECORDING;
        //     } else if (device->output_channels > 0) {
        //         device->id = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
        //     } else if (device->output_channels <= 0 && device->input_channels <= 0) {
        //         error("no audio channels specified. not creating audio device: ", device->id);
        //     } else {
        //         error("this should not happen");
        //     }
        // }

        // if (device->id == AUDIO_DEVICE_FIND_BY_NAME) {
        //     console("trying to find audio device by name: ", device->name);
        //     std::vector<AudioUnitInfo> _devices_found;
        //     find_audio_input_devices(_devices_found);
        //     find_audio_output_devices(_devices_found);
        //     const int _id = find_audio_devices_by_name(_devices_found, device->name);
        //     if (_id == AUDIO_DEVICE_NOT_FOUND) {
        //         console("did not find audio device '", device->name, "' trying default device");
        //         device->id = AUDIO_DEVICE_DEFAULT;
        //     }
        // }

        if (device->input_channels < 1 && device->output_channels < 1) {
            error("either input channels or output channels must be greater than 0. ",
                  "not creating audio device: ", device->input_device_name, "/", device->output_device_name);
            return;
        }


        /* find and open audio devices and streams */

        const auto _device = new PAudioSDL();

        warning("currently only default devices are supported");
        _device->logical_input_device_id  = SDL_AUDIO_DEVICE_DEFAULT_RECORDING; // TODO get this info from PAudio device
        _device->logical_output_device_id = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;  // TODO get this info from PAudio device

        if (device->input_channels > 0) {
            // TODO do we want the callback way still? maybe just for input?
            // if (_audio_device_callback_mode) {}
            // std::vector<AudioUnitInfoSDL> _devices_found;
            // find_audio_input_devices(_devices_found);
            // TODO find logical id from name … or name
            console("currently only default devices are supported");
            SDL_AudioSpec stream_specs; // NOTE these will be used as stream specs for input and output streams
            SDL_zero(stream_specs);
            stream_specs.format              = SDL_AUDIO_F32; // NOTE currently only F32 is supported
            stream_specs.freq                = device->sample_rate;
            stream_specs.channels            = device->input_channels;
            _device->logical_input_device_id = SDL_OpenAudioDevice(_device->logical_input_device_id, nullptr);
            _device->sdl_input_stream        = SDL_CreateAudioStream(nullptr, &stream_specs);
            // _device->sdl_input_stream        = SDL_CreateAudioStream(&stream_specs, nullptr);
            if (_device->sdl_input_stream != nullptr && _device->logical_input_device_id > 0) {
                console("created audio input: ", _device->logical_input_device_id);
                /* --- */
                if (SDL_BindAudioStream(_device->logical_input_device_id, _device->sdl_input_stream)) {
                    SDL_AudioSpec src_spec;
                    SDL_AudioSpec dst_spec;
                    if (SDL_GetAudioStreamFormat(_device->sdl_input_stream, &src_spec, &dst_spec)) {
                        console("audio input stream info:");
                        console("    driver side format ( physical or 'dst' )   : ", src_spec.channels, ", ", src_spec.freq, ", ", SDL_GetAudioFormatName(src_spec.format));
                        console("    client side format ( application or 'src' ): ", dst_spec.channels, ", ", dst_spec.freq, ", ", SDL_GetAudioFormatName(dst_spec.format));
                    } else {
                        error("could not read audio stream format: ", SDL_GetError());
                    }
                    console("binding audio input stream to device: [", _device->logical_input_device_id, "]");
                } else {
                    error("could not bind input stream to device: ", SDL_GetError());
                }
                /* --- */
                SDL_ResumeAudioDevice(_device->logical_input_device_id);
            } else {
                error("couldn't create audio input stream: ", SDL_GetError(), "[", _device->logical_input_device_id, "]");
            }
        }

        if (device->output_channels > 0) {
            // std::vector<AudioUnitInfoSDL> _devices_found;
            // find_audio_output_devices(_devices_found);
            // TODO find logical id from name … or name
            SDL_AudioSpec stream_specs; // NOTE these will be used as stream specs for input and output streams
            SDL_zero(stream_specs);
            stream_specs.format               = SDL_AUDIO_F32; // NOTE currently only F32 is supported
            stream_specs.freq                 = device->sample_rate;
            stream_specs.channels             = device->output_channels;
            _device->logical_output_device_id = SDL_OpenAudioDevice(_device->logical_output_device_id, nullptr);
            _device->sdl_output_stream        = SDL_CreateAudioStream(&stream_specs, nullptr);
            if (_device->sdl_output_stream != nullptr && _device->logical_output_device_id > 0) {
                console("created audio output: ", _device->logical_output_device_id);
                /* --- */
                if (SDL_BindAudioStream(_device->logical_output_device_id, _device->sdl_output_stream)) {
                    SDL_AudioSpec src_spec;
                    SDL_AudioSpec dst_spec;
                    if (SDL_GetAudioStreamFormat(_device->sdl_output_stream, &src_spec, &dst_spec)) {
                        console("audio output stream info:");
                        console("    client side format ( application or 'src' ): ", src_spec.channels, ", ", src_spec.freq, ", ", SDL_GetAudioFormatName(src_spec.format));
                        console("    driver side format ( physical or 'dst' )   : ", dst_spec.channels, ", ", dst_spec.freq, ", ", SDL_GetAudioFormatName(dst_spec.format));
                    }
                    console("binding audio input stream to device: [", _device->logical_output_device_id, "]");
                }
                /* --- */
            } else {
                error("couldn't create audio output stream: ", SDL_GetError(), "[", _device->logical_output_device_id, "]");
            }
        }

        /* handle device names */

        const char* _input_device_name = SDL_GetAudioDeviceName(_device->logical_input_device_id);
        if (_input_device_name == nullptr) {
            device->input_device_name = "(nA)"; // TODO can we need to do better than that?
        } else {
            if (device->input_device_name != _input_device_name) {
                console("updating input device name from '", device->input_device_name, "' to '", _input_device_name, "'");
            }
            device->input_device_name = _input_device_name;
        }

        const char* _output_device_name = SDL_GetAudioDeviceName(_device->logical_output_device_id);
        if (_output_device_name == nullptr) {
            device->output_device_name = "(nA)"; // TODO can we need to do better than that?
        } else {
            if (device->output_device_name != _output_device_name) {
                console("updating output device name from '", device->output_device_name, "' to '", _output_device_name, "'");
            }
            device->output_device_name = _output_device_name;
        }

        _device->audio_device = device;
        device->input_buffer  = new float[device->input_channels * device->buffer_size]{};
        device->output_buffer = new float[device->output_channels * device->buffer_size]{};
        device->unique_id     = audio_unique_device_id++;
        _audio_devices.push_back(_device);
    }

    static PAudio* create_audio(const AudioUnitInfo* device_info) {
        const auto audio_device = new PAudio{device_info};
        register_audio_devices(audio_device);
        return audio_device;
    }
} // namespace umgebung

umgebung::SubsystemAudio* umgebung_subsystem_audio_create_sdl() {
    auto* audio         = new umgebung::SubsystemAudio{};
    audio->set_flags    = umgebung::set_flags;
    audio->init         = umgebung::init;
    audio->setup_pre    = umgebung::setup_pre;
    audio->setup_post   = umgebung::setup_post;
    audio->loop         = umgebung::loop;
    audio->draw_pre     = umgebung::draw_pre;
    audio->draw_post    = umgebung::draw_post;
    audio->shutdown     = umgebung::shutdown;
    audio->event        = umgebung::event;
    audio->start        = umgebung::start;
    audio->stop         = umgebung::stop;
    audio->create_audio = umgebung::create_audio;
    return audio;
}
