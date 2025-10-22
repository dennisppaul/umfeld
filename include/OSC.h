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

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>
#include <any>
#include <iostream>

#include "ip/UdpSocket.h"
#include "osc/OscOutboundPacketStream.h"
#include "osc/OscPacketListener.h"
#include "osc/OscReceivedElements.h"

static constexpr int OSC_TRANSMIT_OUTPUT_BUFFER_SIZE = 1024;

class NetAddress {
public:
    NetAddress(const std::string& ip_address, int port) {
        auto mEndpointName = IpEndpointName(ip_address.c_str(), port);
        fTransmitSocket    = new UdpTransmitSocket(mEndpointName);
    }

    ~NetAddress() {
        delete fTransmitSocket;
    }

    UdpTransmitSocket* socket() const {
        return fTransmitSocket;
    }

private:
    UdpTransmitSocket* fTransmitSocket = nullptr;
};

class OscPayloadFragment {
public:
    OscPayloadFragment(std::any value) : value(std::move(value)) {}

    int intValue() const {
        return std::any_cast<int>(value);
    }

    float floatValue() const {
        return std::any_cast<float>(value);
    }

    std::string stringValue() const {
        return std::any_cast<const char*>(value);
    }

    bool boolValue() const {
        return std::any_cast<bool>(value);
    }

    // TODO add more types
    const std::any value;
};

class OscMessage {
    // NOTE in oscP5 `OscMessage` is used for both sending and receiving while in oscpack this is handled by two different classes.
public:
    OscMessage(std::string address_pattern, bool begin_message = true) : fAddrPattern(std::move(address_pattern)) {
        if (begin_message) {
            begin();
        }
        fTypeTag = "";
    }

    std::string addrPattern() const {
        return fAddrPattern;
    }

    std::string typetag() const {
        return fTypeTag;
    }

    void set_type_tag(std::string type_tag) {
        fTypeTag = std::move(type_tag);
    }

    std::size_t size() const {
        return fPacket.Size();
    }

    const char* data() const {
        return fPacket.Data();
    }

    void begin() {
        fIsRecording = true;
        fPacket << osc::BeginBundleImmediate
                << osc::BeginMessage(fAddrPattern.c_str());
    }

    void end() {
        // TODO maybe compile current typetag into `mTypeTag`
        fPacket << osc::EndMessage
                << osc::EndBundle;
        fIsRecording = false;
    }

    void add(float value) {
        if (!fIsRecording) { begin(); }
        fPacket << value;
    }

    void add(int value) {
        if (!fIsRecording) { begin(); }
        fPacket << value;
    }

    void add(const std::string& value) {
        if (!fIsRecording) { begin(); }
        fPacket << value.c_str();
    }

    void add(bool value) {
        if (!fIsRecording) { begin(); }
        fPacket << value;
    }

    OscPayloadFragment get(int index) const {
        return container[index];
    }

    std::vector<OscPayloadFragment> container;

private:
    const std::string         fAddrPattern;
    std::string               fTypeTag;
    char                      fBuffer[OSC_TRANSMIT_OUTPUT_BUFFER_SIZE];
    osc::OutboundPacketStream fPacket{fBuffer, OSC_TRANSMIT_OUTPUT_BUFFER_SIZE};
    bool                      fIsRecording = false;
};

class OSCListener {
public:
    virtual ~OSCListener() = default;
    virtual void receive_native(const osc::ReceivedMessage& msg) {};
    virtual void receive(const OscMessage& msg) {};
};

class OSC {
public:
    OSC(std::string transmit_address, int transmit_port, int receive_port, bool use_UDP_multicast = true) : fTransmitAddress(std::move(transmit_address)),
                                                                                                            fTransmitPort(transmit_port),
                                                                                                            fReceivePort(receive_port),
                                                                                                            fUseUDPMulticast(use_UDP_multicast) {
        mOSCThread = std::thread(&OSC::osc_thread, this);

        auto mEndpointName = IpEndpointName(fTransmitAddress.c_str(), fTransmitPort);
        mTransmitSocket    = new UdpTransmitSocket(mEndpointName);
    }

    OSC(int receive_port, bool use_UDP_multicast = true) : fTransmitAddress(""),
                                                           fTransmitPort(-1),
                                                           fReceivePort(receive_port),
                                                           fUseUDPMulticast(use_UDP_multicast) {
        mOSCThread      = std::thread(&OSC::osc_thread, this);
        mTransmitSocket = nullptr;
    }

    ~OSC() {
        mOSCThread.detach();
        if (mTransmitSocket != nullptr) {
            delete mTransmitSocket;
        }
    }

    void callback(OSCListener* instance) {
        callback_native = &OSCListener::receive_native;
        callback_       = &OSCListener::receive;
        fInstance       = instance;
    }

    void invoke_callback(const osc::ReceivedMessage& msg) const {
        OscMessage msg_(msg.AddressPattern());
        auto       arg = msg.ArgumentsBegin();
        while (arg != msg.ArgumentsEnd()) {
            if (arg->IsFloat()) {
                msg_.container.push_back(OscPayloadFragment(arg->AsFloat()));
            } else if (arg->IsInt32()) {
                msg_.container.push_back(OscPayloadFragment(arg->AsInt32()));
            } else if (arg->IsString()) {
                msg_.container.push_back(OscPayloadFragment(arg->AsString()));
            } else if (arg->IsBool()) {
                msg_.container.push_back(OscPayloadFragment(arg->AsBool()));
            } else {
                std::cerr << "+++ OSC error: unsupported type '" << arg->TypeTag() << "'" << std::endl;
            }
            // TODO add more types
            ++arg;
        }

        if (fInstance == nullptr) {
            std::cerr << "+++ OSC error: no callback instance" << std::endl;
            return;
        }

        msg_.set_type_tag(msg.TypeTags());
        (fInstance->*callback_)(msg_);

        /* also send native message … for debugging */
        (fInstance->*callback_native)(msg);
    }

    /* send */

    void send(OscMessage message, const NetAddress& address) {
        message.end();
        address.socket()->Send(message.data(), message.size());
    }

    void send(OscMessage& message) const {
        message.end();
        if (mTransmitSocket != nullptr) {
            mTransmitSocket->Send(message.data(), message.size());
        } else {
            std::cerr << "+++ OSC error: no transmit socket created" << std::endl;
        }
    }

    template<typename... Args>
    void send(const std::string& addr_pattern, Args... args) {
        char                      buffer[OSC_TRANSMIT_OUTPUT_BUFFER_SIZE];
        osc::OutboundPacketStream p(buffer, OSC_TRANSMIT_OUTPUT_BUFFER_SIZE);
        p << osc::BeginBundleImmediate
          << osc::BeginMessage(addr_pattern.c_str());
        addArgsToPacketStream(p, args...);
        p << osc::EndMessage
          << osc::EndBundle;
        if (mTransmitSocket != nullptr) {
            mTransmitSocket->Send(p.Data(), p.Size());
        } else {
            std::cerr << "+++ OSC error: no transmit socket created" << std::endl;
        }
    }

    //    void process_KLANG_OSC_CMD(const osc::ReceivedMessage &msg) {
    //#ifdef DEBUG_OSC
    //        KLANG_LOG("@sdlApp onReceive *KLANG_OSC_CMD*     : %s", KLANG_OSC_CMD);
    //#endif
    //        uint8_t mData[msg.ArgumentCount()];
    //        uint8_t i = 0;
    //        for (osc::ReceivedMessage::const_iterator arg = msg.ArgumentsBegin();
    //             arg != msg.ArgumentsEnd(); ++arg) {
    //            mData[i] = (uint8_t) arg->AsInt32();
    //            i++;
    //        }
    //        data_receive(ALL_PERIPHERALS, mData, msg.ArgumentCount());
    //    }

    //    void process_KLANG_OSC_DATA(const osc::ReceivedMessage &msg) {
    //#ifdef DEBUG_OSC
    //        KLANG_LOG("@sdlApp onReceive *KLANG_OSC_DATA* : %s", KLANG_OSC_DATA);
    //#endif
    //        if (msg.ArgumentCount() > 1) {
    //            const uint8_t mLength = msg.ArgumentCount() - 1;
    //            osc::ReceivedMessage::const_iterator arg = msg.ArgumentsBegin();
    //            const uint8_t mSender = (uint8_t) arg->AsInt32();
    //            ++arg;
    //            uint8_t mData[mLength];
    //            uint8_t i = 0;
    //            while (arg != msg.ArgumentsEnd()) {
    //                mData[i] = (uint8_t) arg->AsInt32();
    //                ++arg;
    //                ++i;
    //            }
    //            data_receive(mSender, mData, mLength);
    //        }
    //    }

    //    void process_KLANG_OSC_MIDI_IN(const osc::ReceivedMessage &msg) {
    //#ifdef DEBUG_OSC
    //        KLANG_LOG("@sdlApp onReceive *KLANG_OSC_MIDI_IN* : %s", KLANG_OSC_MIDI_IN);
    //#endif
    //        EVENT_TYPE mMidiInEvent;
    //        if (msg.ArgumentCount() > 0) {
    //            osc::ReceivedMessage::const_iterator arg = msg.ArgumentsBegin();
    //            mMidiInEvent = (EVENT_TYPE) arg->AsInt32();
    //            if (msg.ArgumentCount() > 1) {
    //                float mData[msg.ArgumentCount() - 1];
    //                uint8_t i = 0;
    //                for (osc::ReceivedMessage::const_iterator args = msg.ArgumentsBegin();
    //                     args != msg.ArgumentsEnd(); ++args) {
    //                    if (i == 0) {
    //                        args++;
    //                    }
    //                    mData[i] = (float) args->AsInt32();
    //                    i++;
    //                }
    //                event_receive(mMidiInEvent, mData);
    //            } else {
    //                event_receive(mMidiInEvent, nullptr);
    //            }
    //        }
    //    }

private:
    OSCListener*       fInstance;
    const std::string  fTransmitAddress;
    const int          fTransmitPort;
    const int          fReceivePort;
    const bool         fUseUDPMulticast;
    std::thread        mOSCThread;
    UdpTransmitSocket* mTransmitSocket = nullptr;

    //    void register_callback(void (OSCListener::*callback)(const osc::ReceivedMessage &), OSCListener *instance) {
    //        callback_ = callback;
    //        instance_ = instance;
    //    }

    void (OSCListener::*callback_native)(const osc::ReceivedMessage&);

    void (OSCListener::*callback_)(const OscMessage& msg);

    template<typename T, typename... Rest>
    [[maybe_unused]] void addArgsToPacketStream(osc::OutboundPacketStream& p, T first, Rest... rest) {
        p << first;
        addArgsToPacketStream(p, rest...);
    }

    void addArgsToPacketStream([[maybe_unused]] osc::OutboundPacketStream& p) {}

    class MOscPacketListener : public osc::OscPacketListener {
    public:
        [[maybe_unused]] MOscPacketListener(OSC* parent) : mParent(parent) {}

        void process(const osc::ReceivedMessage& msg) const {
            mParent->invoke_callback(msg);
        }

    private:
        OSC* mParent;

        [[maybe_unused]] bool addr_pattern_equals(const osc::ReceivedMessage& msg, const char* pAddrPatter) {
            return (strcmp(msg.AddressPattern(), pAddrPatter) == 0);
        }

    protected:
        void ProcessMessage(const osc::ReceivedMessage& msg,
                            const IpEndpointName&       remoteEndpoint) override {
            (void) remoteEndpoint; // suppress unused parameter warning
            try {
                process(msg);
            } catch (osc::Exception& e) {
                std::cerr << "+++ OSC receive error: " << e.what() << std::endl;
            }
        }
    };

    void osc_thread() {
        try {
            if (fUseUDPMulticast) {
                MOscPacketListener mOscListener(this);
                PacketListener*    listener_       = &mOscListener;
                auto               mIpEndpointName = IpEndpointName(fTransmitAddress.c_str(), fReceivePort);
                if (mIpEndpointName.IsMulticastAddress()) {
                    UdpSocket mUdpSocket;
                    mUdpSocket.SetAllowReuse(true);
                    mUdpSocket.Bind(mIpEndpointName);
                    SocketReceiveMultiplexer mux_;
                    mux_.AttachSocketListener(&mUdpSocket, listener_);
                    mux_.Run();
                } else {
                    UdpListeningReceiveSocket s(IpEndpointName(IpEndpointName::ANY_ADDRESS, fReceivePort),
                                                &mOscListener);
                    s.Run();
                }
            } else {
                MOscPacketListener        mOscListener(this);
                UdpListeningReceiveSocket s(IpEndpointName(IpEndpointName::ANY_ADDRESS, fReceivePort), &mOscListener);
                s.Run();
            }
        } catch (std::exception& e) {
            std::cerr << "+++ OSC receive error: " << e.what() << std::endl;
        }
    }
};