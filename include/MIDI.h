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

#include <RtMidi.h>
#include <vector>

#include "Umfeld.h"

void midi_message(const std::vector<unsigned char>& message);
void note_off(int channel, int note);
void note_on(int channel, int note, int velocity);
void control_change(int channel, int control, int value);
void program_change(int channel, int program);
void pitch_bend(int channel, int value);
void sys_ex(const std::vector<unsigned char>& message);

namespace umfeld {
    inline void (*midi_callback_midi_message)(const std::vector<unsigned char>&) = midi_message;
    inline void (*midi_callback_note_off)(int, int)                              = note_off;
    inline void (*midi_callback_note_on)(int, int, int)                          = note_on;
    inline void (*midi_callback_control_change)(int, int, int)                   = control_change;
    inline void (*midi_callback_program_change)(int, int)                        = program_change;
    inline void (*midi_callback_pitch_bend)(int, int)                            = pitch_bend;
    inline void (*midi_callback_sys_ex)(const std::vector<unsigned char>&)       = sys_ex;
    // inline void (*midi_callback_aftertouch)(int, int)                       = nullptr;
    // inline void (*midi_callback_poly_aftertouch)(int, int, int)             = nullptr;

    class MIDIListener {
    public:
        virtual ~MIDIListener() = default;
        virtual void midi_message(const std::vector<unsigned char>& message) {}
        virtual void note_off(int channel, int note) {}
        virtual void note_on(int channel, int note, int velocity) {}
        virtual void control_change(int channel, int control, int value) {}
        virtual void program_change(int channel, int program) {}
        virtual void pitch_bend(int channel, int value) {}
        virtual void sys_ex(const std::vector<unsigned char>& message) {}
        //    virtual void aftertouch(int channel, int pressure) {}
        //    virtual void poly_aftertouch(int channel, int note, int pressure) {}
    };

    class MIDI {
    public:
        enum Commands {
            NOTE_OFF         = 0x80,
            NOTE_ON          = 0x90,
            POLY_PRESSURE    = 0xA0,
            CONTROL_CHANGE   = 0xB0,
            PROGRAM_CHANGE   = 0xC0,
            CHANNEL_PRESSURE = 0xD0,
            PITCH_BEND       = 0xE0,
            SYSEX_START      = 0xF0,
            SYSEX_END        = 0xF7
        };

        enum Channel {
            CHANNEL_1 = 0,
            CHANNEL_2 = 1,
            CHANNEL_3 = 2,
            CHANNEL_4 = 3,
            CHANNEL_5 = 4,
            CHANNEL_6 = 5,
            CHANNEL_7 = 6,
            CHANNEL_8 = 7
        };

        MIDI() : listener_instance(nullptr)
        // , callback_message(nullptr)
        {
            midiIn  = new RtMidiIn();
            midiOut = new RtMidiOut();
        }

        ~MIDI() {
            delete midiIn;
            delete midiOut;
        }

        void print_available_ports() const {
            // Listing MIDI Input Ports
            const unsigned int nInPorts = midiIn->getPortCount();
            console("+++ MIDI input ports");
            for (unsigned int i = 0; i < nInPorts; i++) {
                try {
                    std::string portName = midiIn->getPortName(i);
                    console("+++\t", i, ":\t", portName);
                } catch (RtMidiError& error) {
                    error.printMessage();
                }
            }
            console("+++ ");

            // Listing MIDI Output Ports
            const unsigned int nOutPorts = midiOut->getPortCount();
            console("+++ MIDI output ports");
            for (unsigned int i = 0; i < nOutPorts; i++) {
                try {
                    std::string portName = midiOut->getPortName(i);
                    console("+++\t", i, ":\t", portName);
                } catch (RtMidiError& error) {
                    error.printMessage();
                }
            }
        }

        void open_input_port(const unsigned int port = 0) {
            if (port >= midiIn->getPortCount()) {
                error("+++ MIDI error: port ", port, " not available");
                return;
            }
            midiIn->openPort(port);
            midiIn->setCallback(&MIDI::midiInputCallback, this);
            midiIn->ignoreTypes(false, false, false); // Don't ignore sysex, timing, or active sensing messages.
        }

        void open_output_port(const unsigned int port = 0) const {
            if (port >= midiOut->getPortCount()) {
                error("+++ MIDI error: port ", port, " not available");
                return;
            }
            midiOut->openPort(port);
        }

        void open_input_port(const std::string& portName) {
            const unsigned int portNumber = find_port(portName, true);
            open_input_port(portNumber);
        }

        void open_output_port(const std::string& portName) const {
            const unsigned int portNumber = find_port(portName, false);
            open_output_port(portNumber);
        }

        [[nodiscard]] unsigned int find_port(const std::string& portName, const bool isInput) const {
            RtMidi*            midi   = isInput ? static_cast<RtMidi*>(midiIn) : static_cast<RtMidi*>(midiOut);
            const unsigned int nPorts = midi->getPortCount();

            for (unsigned int i = 0; i < nPorts; ++i) {
                try {
                    std::string name = midi->getPortName(i);
                    if (name == portName) {
                        return i;
                    }
                } catch (RtMidiError& error) {
                    error.printMessage();
                }
            }

            error("+++ port '", portName, "' not found. using default port.");
            return 0;
        }

        void note_off(const int channel, const int note) const {
            const std::vector<unsigned char> message = {
                static_cast<unsigned char>(NOTE_OFF + channel),
                static_cast<unsigned char>(note),
                0};
            midiOut->sendMessage(&message);
        }

        void note_on(const int channel, const int note, const int velocity) const {
            const std::vector message = {
                static_cast<unsigned char>(NOTE_ON + channel),
                static_cast<unsigned char>(note),
                static_cast<unsigned char>(velocity)};
            midiOut->sendMessage(&message);
        }

        void control_change(const int channel, const int control, const int value) const {
            const std::vector message = {
                static_cast<unsigned char>(CONTROL_CHANGE + channel),
                static_cast<unsigned char>(control),
                static_cast<unsigned char>(value)};
            midiOut->sendMessage(&message);
        }

        void program_change(const int channel, const int program) const {
            const std::vector message = {
                static_cast<unsigned char>(PROGRAM_CHANGE + channel),
                static_cast<unsigned char>(program)};
            midiOut->sendMessage(&message);
        }

        void pitch_bend(const int channel, const int value) const {
            // Pitch bend value is a 14-bit value (0 to 16383).
            // It is split into two 7-bit values: MSB and LSB.
            const int lsb = value & 0x7F;
            const int msb = value >> 7 & 0x7F;

            const std::vector message = {
                static_cast<unsigned char>(PITCH_BEND + channel),
                static_cast<unsigned char>(lsb),
                static_cast<unsigned char>(msb)};
            midiOut->sendMessage(&message);
        }

        void channel_pressure(const int channel, const int pressure) const {
            const std::vector message = {
                static_cast<unsigned char>(CHANNEL_PRESSURE + channel),
                static_cast<unsigned char>(pressure)};
            midiOut->sendMessage(&message);
        }

        void sys_ex(const std::vector<unsigned char>& data) const {
            std::vector<unsigned char> message;
            message.push_back(SYSEX_START); // Start of SysEx
            message.insert(message.end(), data.begin(), data.end());
            message.push_back(SYSEX_END); // End of SysEx
            midiOut->sendMessage(&message);
        }

        void callback(MIDIListener* instance) {
            // callback_message  = &MIDIListener::midi_message;
            listener_instance = instance;
        }

    private:
        RtMidiIn*     midiIn;
        RtMidiOut*    midiOut;
        MIDIListener* listener_instance;

        // void (MIDIListener::*callback_message)(const std::vector<unsigned char>& _message);

        void invoke_callback(const std::vector<unsigned char>& _message) {
            // if (listener_instance == nullptr) {
            //     error("+++ MIDI error: no callback instance");
            //     return;
            // }

            if (_message.size() < 2) {
                error("+++ MIDI error: malformed message ( < 2 )");
                return;
            }

            const int status  = _message[0];
            const int channel = status & 0x0F;
            const int command = status & 0xF0;

            switch (command) {
                case NOTE_OFF: {
                    if (_message.size() >= 2) {
                        if (listener_instance != nullptr) {
                            (listener_instance->*&MIDIListener::note_off)(channel,
                                                                          static_cast<int>(_message[1]));
                        }
                        if (midi_callback_note_off != nullptr) {
                            midi_callback_note_off(channel, _message[1]);
                        }
                    }
                } break;
                case NOTE_ON: {
                    if (_message.size() >= 3) {
                        if (listener_instance != nullptr) {
                            (listener_instance->*&MIDIListener::note_on)(channel,
                                                                         static_cast<int>(_message[1]),
                                                                         static_cast<int>(_message[2]));
                        }
                        if (midi_callback_note_on != nullptr) {
                            midi_callback_note_on(channel, _message[1], _message[2]);
                        }
                    }
                } break;
                case CONTROL_CHANGE: {
                    if (_message.size() >= 3) {
                        if (listener_instance != nullptr) {
                            (listener_instance->*&MIDIListener::control_change)(channel,
                                                                                static_cast<int>(_message[1]),
                                                                                static_cast<int>(_message[2]));
                        }
                        if (midi_callback_control_change != nullptr) {
                            midi_callback_control_change(channel, _message[1], _message[2]);
                        }
                    }
                } break;
                case PROGRAM_CHANGE: {
                    if (_message.size() >= 2) {
                        if (listener_instance != nullptr) {
                            (listener_instance->*&MIDIListener::program_change)(channel,
                                                                                static_cast<int>(_message[1]));
                        }
                        if (midi_callback_program_change != nullptr) {
                            midi_callback_program_change(channel, _message[1]);
                        }
                    }
                } break;
                case PITCH_BEND: {
                    if (_message.size() >= 3) {
                        if (listener_instance != nullptr) {
                            const int value = static_cast<int>(_message[2]) << 7 | static_cast<int>(_message[1]);
                            (listener_instance->*&MIDIListener::pitch_bend)(channel,
                                                                            value);
                        }
                        if (midi_callback_pitch_bend != nullptr) {
                            midi_callback_pitch_bend(channel, static_cast<int>(_message[2]) << 7 | static_cast<int>(_message[1]));
                        }
                    }
                } break;
                default:
                    break;
            }

            if (_message[0] == SYSEX_START) { // SysEx
                if (listener_instance != nullptr) {
                    (listener_instance->*&MIDIListener::sys_ex)(_message);
                }
                if (midi_callback_sys_ex != nullptr) {
                    midi_callback_sys_ex(_message);
                }
                return;
            }

            /* also send native message … for debugging */
            if (listener_instance != nullptr) {
                (listener_instance->*&MIDIListener::midi_message)(_message);
            }
            if (midi_callback_midi_message != nullptr) {
                midi_callback_midi_message(_message);
            }
        }

        // ReSharper disable once CppParameterMayBeConstPtrOrRef
        static void midiInputCallback(double deltatime, std::vector<unsigned char>* message, void* userData) {
            auto* handler = static_cast<MIDI*>(userData);
            handler->invoke_callback(*message);
        }
    };
}; // namespace umfeld