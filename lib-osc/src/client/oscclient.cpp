/**
 * @file oscclient.cpp
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

#if defined(DEBUG_OSCCLIENT)
#undef NDEBUG
#endif

#include <cstdint>

#include "oscclient.h"
#include "oscsimplemessage.h"
#include "osc.h"
#include "configurationstore.h"
#include "network.h"
#include "apps/mdns.h"
#include "firmware/debug/debug_debug.h"

OscClient::OscClient()
    : port_outgoing_(oscclient::defaults::kPortOutgoing),
      port_incoming_(oscclient::defaults::kPortIncoming),
      ping_delay_millis_(oscclient::defaults::kPingDelaySeconds * 1000)
{
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    DEBUG_EXIT();
}

void OscClient::Start()
{
    DEBUG_ENTRY();

    assert(handle_ == -1);
    handle_ = network::udp::Begin(port_incoming_, StaticCallbackFunction);
    assert(handle_ != -1);

    network::apps::mdns::ServiceRecordAdd(nullptr, network::apps::mdns::Services::kOsc, "type=client", port_incoming_);

    DEBUG_EXIT();
}

void OscClient::Stop()
{
    DEBUG_ENTRY();

    network::apps::mdns::ServiceRecordDelete(network::apps::mdns::Services::kOsc);

    assert(handle_ != -1);
    network::udp::End(port_incoming_);
    handle_ = -1;

    DEBUG_EXIT();
}

bool OscClient::HandleLedMessage(uint16_t bytes_received)
{
    DEBUG_ENTRY();

    uint32_t i;

    for (i = 0; i < common::store::osc::client::kLedCount; i++)
    {
        const char* src = &s_leds[i * common::store::osc::client::kLedPathLength];
        if (osc::is_match(buffer_, src))
        {
            DEBUG_PUTS("");
            break;
        }
    }

    if (i == common::store::osc::client::kLedCount)
    {
        DEBUG_EXIT();
        return false;
    }

    OscSimpleMessage msg(reinterpret_cast<const uint8_t*>(buffer_), bytes_received);

    const int kArgc = msg.GetArgc();

    if (kArgc != 1)
    {
        DEBUG_EXIT();
        return false;
    }

    if (msg.GetType(0) == osc::type::INT32)
    {
        oscclient_led_->SetLed(static_cast<uint8_t>(i), static_cast<uint8_t>(msg.GetInt(0)) != 0);
        DEBUG_PRINTF("%d", msg.GetInt(0));
    }
    else if (msg.GetType(0) == osc::type::FLOAT)
    {
        oscclient_led_->SetLed(static_cast<uint8_t>(i), static_cast<uint8_t>(msg.GetFloat(0)) != 0);
        DEBUG_PRINTF("%f", msg.GetFloat(0));
    }
    else
    {
        return false;
    }

    DEBUG_EXIT();
    return true;
}

void OscClient::Print()
{
    puts("OSC Client");
    printf(" Server        : " IPSTR "\n", IP2STR(server_ip_));
    printf(" Outgoing Port : %d\n", port_outgoing_);
    printf(" Incoming Port : %d\n", port_incoming_);
    printf(" Disable /ping : %s\n", ping_disable_ ? "Yes" : "No");

    if (!ping_disable_)
    {
        printf(" Ping delay        : %ds\n", ping_delay_millis_ / 1000);
    }

    for (uint32_t i = 0; i < common::store::osc::client::kCmdCount; i++)
    {
        const char* p = &s_cmds[i * common::store::osc::client::kCmdPathLength];
        if (*p != '\0')
        {
            printf("  cmd%c             : [%s]\n", i + '0', p);
        }
    }

    for (uint32_t i = 0; i < common::store::osc::client::kLedCount; i++)
    {
        const char* p = &s_leds[i * common::store::osc::client::kLedPathLength];
        if (*p != '\0')
        {
            printf("  led%c             : [%s]\n", i + '0', p);
        }
    }
}
