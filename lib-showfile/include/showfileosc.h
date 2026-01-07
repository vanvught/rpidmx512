/**
 * @file showfileosc.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SHOWFILEOSC_H_
#define SHOWFILEOSC_H_

#if !defined(CONFIG_SHOWFILE_ENABLE_OSC)
#error This file should not be included
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "showfiledisplay.h"
#include "osc.h"
#include "network.h"
#include "firmware/debug/debug_debug.h"

namespace showfileosc
{
inline constexpr char kCmdPath[] = "/showfile/";
inline constexpr uint32_t kPathLength = sizeof(kCmdPath) - 1;
inline constexpr uint32_t kMaxCmdLength = 128;
inline constexpr uint32_t kMaxFilesEntries = 10;
} // namespace showfileosc

class ShowFileOSC
{
   public:
    explicit ShowFileOSC(uint16_t port_incoming = osc::port::DEFAULT_INCOMING, uint16_t port_outgoing = osc::port::DEFAULT_OUTGOING)
        : port_outgoing_(port_outgoing)
    {
        DEBUG_ENTRY();

        assert(s_this == nullptr);
        s_this = this;

        SetPortIncoming(port_incoming);

        DEBUG_EXIT();
    }

    ~ShowFileOSC()
    {
        DEBUG_ENTRY();

        network::udp::End(port_incoming_);

        DEBUG_EXIT();
    }

    void Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        assert(buffer != nullptr);

        buffer_ = buffer;
        remote_ip_ = from_ip;
        bytes_received_ = size;
        remote_port_ = from_port;

        if (memcmp(buffer_, showfileosc::kCmdPath, showfileosc::kPathLength) == 0)
        {
            Process();
        }
    }

    void Print()
    {
        puts("OSC Server");
        printf(" Path : [%s]\n", showfileosc::kCmdPath);
        printf(" Incoming port : %u\n", port_incoming_);
        printf(" Outgoing port : %u\n", port_outgoing_);
    }

    void SetPortIncoming(uint16_t port_incoming)
    {
        if (port_incoming == port_incoming_)
        {
            return;
        }

        if (handle_ != 1)
        {
            network::udp::End(port_incoming_);
        }

        if (port_incoming > 1023)
        {
            port_incoming_ = port_incoming;
        }
        else
        {
            port_incoming_ = osc::port::DEFAULT_INCOMING;
        }

        handle_ = network::udp::Begin(port_incoming_, StaticCallbackFunction);
        assert(handle_ != -1);
    }

    uint16_t GetPortIncoming() const { return port_incoming_; }

    void SetPortOutgoing(uint16_t port_outgoing)
    {
        if (port_outgoing > 1023)
        {
            port_outgoing_ = port_outgoing;
        }
        else
        {
            port_outgoing_ = osc::port::DEFAULT_OUTGOING;
        }
    }

    uint16_t GetPortOutgoing() const { return port_outgoing_; }

   private:
    void Process();
    void SendStatus();
    void ShowFiles();

    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }

   private:
    uint16_t port_incoming_{osc::port::DEFAULT_INCOMING};
    uint16_t port_outgoing_{osc::port::DEFAULT_OUTGOING};
    int32_t handle_{-1};
    const uint8_t* buffer_{nullptr};
    uint32_t remote_ip_{0};
    uint32_t bytes_received_{0};
    uint16_t remote_port_{0};

    static inline ShowFileOSC* s_this;
};

#endif // SHOWFILEOSC_H_
