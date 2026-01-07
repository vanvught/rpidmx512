/**
 * @file oscclient.h
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef OSCCLIENT_H_
#define OSCCLIENT_H_

#include <cstdint>
#include <cassert>

#include "configurationstore.h"
#include "ip4/ip4_address.h"
#include "osc.h"
#include "oscsimplesend.h"
#include "oscclientled.h"
#include "hal_millis.h"
#include "display.h"
 #include "firmware/debug/debug_debug.h"

namespace oscclient
{
namespace defaults
{
inline constexpr uint16_t kPortOutgoing = 8000;
inline constexpr uint16_t kPortIncoming = 9000;
inline constexpr uint32_t kPingDelaySeconds = 10;
} // namespace defaults

namespace buffer::size
{
inline constexpr uint32_t kCmd = common::store::osc::client::kCmdCount * common::store::osc::client::kCmdPathLength;
inline constexpr uint32_t kLed = common::store::osc::client::kLedCount * common::store::osc::client::kLedPathLength;
} // namespace buffer::size

} // namespace oscclient

class OscClient
{
   public:
    OscClient();

    OscClient(const OscClient&) = delete;
    OscClient& operator=(const OscClient&) = delete;

    ~OscClient() = default;

    void Start();
    void Stop();
    void Print();

    void Run()
    {
        if (!ping_disable_)
        {
            current_millis_ = hal::Millis();

            if (ping_sent_ && ((current_millis_ - ping_time_millis_) >= 1000))
            {
				ping_sent_ = false;
				
                if (!pong_received_)
                {
                    Display::Get()->TextStatus("No /Pong");
                    DEBUG_PUTS("No /Pong");
                }
            }

            if ((current_millis_ - previous_millis_) >= ping_delay_millis_)
            {
                OscSimpleSend msg_send(handle_, server_ip_, port_outgoing_, "/ping", nullptr);
                ping_sent_ = true;
                pong_received_ = false;
                previous_millis_ = current_millis_;
                ping_time_millis_ = current_millis_;
                DEBUG_PUTS("Ping sent");
                return;
            }
        }
    }

    void Send(const char* path)
    {
        DEBUG_ENTRY();

        assert(path != nullptr);

        if (*path != 0)
        {
            OscSimpleSend msg_send(handle_, server_ip_, port_outgoing_, path, nullptr);
        }

        DEBUG_EXIT();
    }

    void SendCmd(uint32_t cmd)
    {
        DEBUG_ENTRY();
        DEBUG_PRINTF("cmd=%d", cmd);

        assert(cmd < common::store::osc::client::kCmdCount);

        const char* dst = &s_cmds[cmd * common::store::osc::client::kCmdPathLength];
        Send(dst);

        DEBUG_EXIT();
    }

    void SetServerIP(uint32_t server_ip) { server_ip_ = server_ip; }
    uint32_t GetServerIP() const { return server_ip_; }

    void SetPortOutgoing(uint16_t port_outgoing)
    {
        if (port_outgoing > 1023)
        {
            port_outgoing_ = port_outgoing;
        }
        else
        {
            port_outgoing_ = oscclient::defaults::kPortOutgoing;
        }
    }

    uint16_t GetPortOutgoing() const { return port_outgoing_; }

    void SetPortIncoming(uint16_t port_incoming)
    {
        if (port_incoming > 1023)
        {
            port_incoming_ = port_incoming;
        }
        else
        {
            port_incoming_ = oscclient::defaults::kPortIncoming;
        }
    }

    uint16_t GetPortIncoming() const { return port_incoming_; }

    void SetPingDisable(bool ping_disable = true) { ping_disable_ = ping_disable; }
    bool GetPingDisable() const { return ping_disable_; }

    void SetPingDelaySeconds(uint32_t ping_delay)
    {
        if ((ping_delay >= 2) && (ping_delay <= 60))
        {
            ping_delay_millis_ = ping_delay * 1000;
        }
        else
        {
            ping_delay_millis_ = oscclient::defaults::kPingDelaySeconds * 1000;
        }
    }

    uint32_t GetPingDelaySeconds() const { return ping_delay_millis_ / 1000U; }

    void CopyCmds(const char* cmds)
    {
        assert(cmds != nullptr);

        for (uint32_t i = 0; i < common::store::osc::client::kCmdCount; i++)
        {
            char* dst = &s_cmds[i * common::store::osc::client::kCmdPathLength];
            strncpy(dst, &cmds[i * common::store::osc::client::kCmdPathLength], common::store::osc::client::kCmdPathLength - 1);
            dst[common::store::osc::client::kCmdPathLength - 1] = '\0';
        }
    }

    const char* GetCmd(uint32_t index)
    {
        assert(index < common::store::osc::client::kCmdCount);
        const char* dst = &s_cmds[index * common::store::osc::client::kCmdPathLength];
        return dst;
    }

    const char* GetLed(uint32_t index)
    {
        assert(index < common::store::osc::client::kLedCount);
        const char* dst = &s_leds[index * common::store::osc::client::kLedPathLength];
        return dst;
    }

    void CopyLeds(const char* leds)
    {
        assert(leds != nullptr);

        for (uint32_t i = 0; i < common::store::osc::client::kLedCount; i++)
        {
            char* dst = &s_leds[i * common::store::osc::client::kLedPathLength];
            strncpy(dst, &leds[i * common::store::osc::client::kLedPathLength], common::store::osc::client::kLedPathLength - 1);
            dst[common::store::osc::client::kLedPathLength - 1] = '\0';
        }
    }

    void SetLedHandler(OscClientLed* osc_client_led)
    {
        assert(osc_client_led != nullptr);
        oscclient_led_ = osc_client_led;
    }

    static OscClient& Instance()
    {
        assert(s_this != nullptr); // Ensure that s_this is valid
        return *s_this;
    }

   private:
    void Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
    {
        buffer_ = reinterpret_cast<const char*>(buffer);

        DEBUG_PRINTF(IPSTR " -> %s", IP2STR(from_ip), buffer_);

        if (from_ip != server_ip_)
        {
            DEBUG_PRINTF("Data not received from server " IPSTR, IP2STR(server_ip_));
            return;
        }

        if (oscclient_led_ != nullptr)
        {
            if (HandleLedMessage(size))
            {
                DEBUG_EXIT();
                return;
            }
        }

        if (!ping_disable_)
        {
            if (!osc::is_match(buffer_, "/pong"))
            {
                return;
            }

            Display::Get()->TextStatus("Pong received");
            DEBUG_PUTS("Pong received");

            pong_received_ = true;
            ping_sent_ = false;
        }
    }

    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }

    bool HandleLedMessage(uint16_t bytes_received);

   private:
    uint16_t port_outgoing_;
    uint16_t port_incoming_;
    uint32_t ping_delay_millis_;
    uint32_t server_ip_{0};
    int32_t handle_{-1};
    uint32_t current_millis_{0};
    uint32_t previous_millis_{0};
    uint32_t ping_time_millis_{0};
    const char* buffer_{nullptr};
    bool ping_disable_{false};
    bool ping_sent_{false};
    bool pong_received_{false};

    OscClientLed* oscclient_led_{nullptr};

    static inline char s_cmds[oscclient::buffer::size::kCmd];
    static inline char s_leds[oscclient::buffer::size::kLed];

    static inline OscClient* s_this{nullptr};
};

#endif  // OSCCLIENT_H_
