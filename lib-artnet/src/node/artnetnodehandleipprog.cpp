/**
 * @file artnetnodehandleipprog.cpp
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstring>

#include "artnet.h"
#include "artnetnode.h"
#include "network.h"
#include "firmware/debug/debug_printbits.h"
 #include "firmware/debug/debug_debug.h"

static constexpr uint8_t kCommandEnableProgramming = (1U << 7);
static constexpr uint8_t kCommandEnableDhcp = ((1U << 6) | kCommandEnableProgramming);
static constexpr uint8_t kCommandProgramGateway = ((1U << 4) | kCommandEnableProgramming);
static constexpr uint8_t kCommandSetToDefault = ((1U << 3) | kCommandEnableProgramming);
static constexpr uint8_t kCommandProgramIpaddress = ((1U << 2) | kCommandEnableProgramming);
static constexpr uint8_t kCommandProgramSubnetmask = ((1U << 1) | kCommandEnableProgramming);

union uip
{
    uint32_t u32;
    uint8_t u8[4];
} static ip;

void ArtNetNode::HandleIpProg()
{
    DEBUG_ENTRY();

    const auto* const kPArtIpProg = reinterpret_cast<artnet::ArtIpProg*>(receive_buffer_);
    const auto kCommand = kPArtIpProg->command;
    auto* reply = reinterpret_cast<artnet::ArtIpProgReply*>(receive_buffer_);
    const auto kIsDhcp =  network::iface::Dhcp();

    reply->op_code = static_cast<uint16_t>(artnet::OpCodes::kOpIpprogreply);

    if ((kCommand & kCommandEnableDhcp) == kCommandEnableDhcp)
    {
         network::iface::EnableDhcp();
    }

    if ((kCommand & kCommandSetToDefault) == kCommandSetToDefault)
    {
        network::SetPrimaryIp(0);
    }

    if ((kCommand & kCommandProgramIpaddress) == kCommandProgramIpaddress)
    {
        memcpy(ip.u8, &kPArtIpProg->prog_ip_hi, artnet::kIpSize);
        network::SetPrimaryIp(ip.u32);
    }

    if ((kCommand & kCommandProgramSubnetmask) == kCommandProgramSubnetmask)
    {
        memcpy(ip.u8, &kPArtIpProg->prog_sm_hi, artnet::kIpSize);
        network::SetNetmask(ip.u32);
    }

    if ((kCommand & kCommandProgramGateway) == kCommandProgramGateway)
    {
        memcpy(ip.u8, &kPArtIpProg->prog_gw_hi, artnet::kIpSize);
        network::SetGatewayIp(ip.u32);
    }

    if ( network::iface::Dhcp())
    {
        reply->status = (1U << 6);
    }
    else
    {
        reply->status = 0;
    }

    reply->spare2 = 0;

    auto is_changed = (kIsDhcp !=  network::iface::Dhcp());

    ip.u32 = network::GetPrimaryIp();
    is_changed |= (memcmp(&kPArtIpProg->prog_ip_hi, ip.u8, artnet::kIpSize) != 0);
    memcpy(&reply->prog_ip_hi, ip.u8, artnet::kIpSize);

    ip.u32 = network::GetNetmask();
    is_changed |= (memcmp(&kPArtIpProg->prog_sm_hi, ip.u8, artnet::kIpSize) != 0);
    memcpy(&reply->prog_sm_hi, ip.u8, artnet::kIpSize);

    ip.u32 = network::GetGatewayIp();
    is_changed |= (memcmp(&kPArtIpProg->prog_gw_hi, ip.u8, artnet::kIpSize) != 0);
    memcpy(&reply->prog_gw_hi, ip.u8, artnet::kIpSize);

    reply->spare7 = 0;
    reply->spare8 = 0;

    network::udp::Send(handle_, receive_buffer_, sizeof(struct artnet::ArtIpProgReply), ip_address_from_, artnet::kUdpPort);

    if (is_changed)
    {
        art_poll_reply_.Status2 = static_cast<uint8_t>((art_poll_reply_.Status2 & (~(artnet::Status2::kIpDhcp))) |
                                                       ( network::iface::Dhcp() ? artnet::Status2::kIpDhcp : artnet::Status2::kIpManualy));

        memcpy(art_poll_reply_.IPAddress, &reply->prog_ip_hi, artnet::kIpSize);
#if (ARTNET_VERSION >= 4)
        memcpy(art_poll_reply_.BindIp, &reply->prog_ip_hi, artnet::kIpSize);
#endif
        if (state_.send_art_poll_reply_on_change)
        {
            SendPollReply(0, ip_address_from_);
        }

#ifndef NDEBUG
        DEBUG_PUTS("Changed");
        debug::PrintBits(art_poll_reply_.Status2);
#endif
    }
#ifndef NDEBUG
    else
    {
        DEBUG_PUTS("No changes");
    }
#endif

    DEBUG_EXIT();
}
