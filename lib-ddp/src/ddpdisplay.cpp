/**
 * @file ddpdisplay.cpp
 */
/* Copyright (C) 2021-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "ddpdisplay.h"
#include "ddp.h"
#include "dmxnodedata.h"
#include "dmxnode_data.h"
#include "apps/mdns.h"
#include "network.h"
#include "core/protocol/udp.h"
#include "hal.h"
#include "firmware/debug/debug_dump.h"
#include "firmware/debug/debug_debug.h"

using namespace ddp;

namespace json
{
static constexpr char kStart[] = "{\"status\":{\"update\":\"change\",\"state\":\"up\"}}";
static constexpr char kDiscoverReply[] = "{\"status\":{\"man\":\"%s\",\"mod\":\"Pixel\",\"ver\":\"1.0\",\"mac\":\"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\"}}";
static constexpr char kConfigReply[] =
    "{\"config\":{\"ip\":\"%d.%d.%d.%d\",\"nm\":\"%d.%d.%d.%d\",\"gw\":\"%d.%d.%d.%d\",\"ports\":["
    "{\"port\":\"0\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"1\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"2\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"3\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
#if CONFIG_DMXNODE_PIXEL_MAX_PORTS > 2
    "{\"port\":\"4\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"5\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"6\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"7\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"8\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"9\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"}"
#endif
#if CONFIG_DMXNODE_PIXEL_MAX_PORTS == 16
    ","
    "{\"port\":\"10\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"11\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"12\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"13\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"14\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"15\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"16\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"},"
    "{\"port\":\"17\",\"ts\":\"0\",\"l\":\"%d\",\"ss\":\"0\"}"

#endif
    "]}}";

namespace size
{
static constexpr auto kStart = sizeof(json::kStart) - 1U;
} // namespace size
} // namespace json

DdpDisplay::DdpDisplay()
{
    DEBUG_ENTRY();
    assert(s_this == nullptr);
    s_this = this;

    network::iface::CopyMacAddressTo(mac_address_);

    DEBUG_EXIT();
}

DdpDisplay::~DdpDisplay()
{
    DEBUG_ENTRY();

    Stop();

    DEBUG_EXIT();
}

void DdpDisplay::CalculateOffsets()
{
    uint32_t sum = 0;

    for (uint32_t pixel_port_index = 0; pixel_port_index < ddpdisplay::configuration::pixel::kMaxPorts; pixel_port_index++)
    {
        sum = sum + strip_data_length_;
        s_offset_compare[pixel_port_index] = sum;
    }

    /*
      error: comparison of unsigned expression < 0 is always false [-Werror=type-limits]
      for (uint32_t nDmxPortIndex = 0; nDmxPortIndex < ddpdisplay::configuration::dmx::MAX_PORTS; nDmxPortIndex++) {
     */

#if __GNUC__ < 10
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits" // FIXME ignored "-Wtype-limits"
#endif

    for (uint32_t dmx_port_index = 0; dmx_port_index < ddpdisplay::configuration::dmx::kMaxPorts; dmx_port_index++)
    {
        sum = sum + dmxnode::kUniverseSize;
        const auto kIndexOffset = dmx_port_index + ddpdisplay::configuration::pixel::kMaxPorts;
        s_offset_compare[kIndexOffset] = sum;
    }

#if __GNUC__ < 10
#pragma GCC diagnostic pop
#endif
}

void DdpDisplay::Start()
{
    DEBUG_ENTRY();
    assert(dmxnode_output_type_ != nullptr);

    handle_ = network::udp::Begin(ddp::kUdpPort, StaticCallbackFunction);
    assert(handle_ != -1);

    network::apps::mdns::ServiceRecordAdd(nullptr, network::apps::mdns::Services::kDdp, "type=display");

    ddp::Packet packet;

    memset(&packet.header, 0, HEADER_LEN);
    packet.header.flags1 = flags1::VER1 | flags1::REPLY;
    packet.header.id = id::STATUS;
    packet.header.len[1] = json::size::kStart;
    memcpy(packet.data, json::kStart, json::size::kStart);

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&packet), HEADER_LEN + json::size::kStart, network::GetPrimaryIp() | ~(network::GetNetmask()),
                       ddp::kUdpPort);

    debug::Dump(&packet, HEADER_LEN + json::size::kStart);

    CalculateOffsets();
    DEBUG_EXIT();
}

void DdpDisplay::Stop()
{
    DEBUG_ENTRY();

    network::apps::mdns::ServiceRecordDelete(network::apps::mdns::Services::kDdp);

    handle_ = network::udp::End(ddp::kUdpPort);
    handle_ = -1;

    DEBUG_EXIT();
}

void DdpDisplay::HandleQuery()
{
    DEBUG_ENTRY();

    auto* packet = reinterpret_cast<ddp::Packet*>(receive_buffer_);

    if ((packet->header.id & id::STATUS) == id::STATUS)
    {
        DEBUG_PUTS("id::STATUS");

        const auto kLength =
            snprintf(reinterpret_cast<char*>(packet->data), network::udp::kDataSize - 1, json::kDiscoverReply, hal::kWebsite, MAC2STR(mac_address_));

        packet->header.flags1 = flags1::VER1 | flags1::REPLY | flags1::PUSH;
        packet->header.len[0] = static_cast<uint8_t>(kLength >> 8);
        packet->header.len[1] = static_cast<uint8_t>(kLength);

        network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&packet), (HEADER_LEN + static_cast<uint16_t>(kLength)),
                           network::GetPrimaryIp() | ~(network::GetNetmask()), ddp::kUdpPort);
    }

    if ((packet->header.id & id::STATUS) == id::CONFIG)
    {
        DEBUG_PUTS("id::CONFIG");

        const auto kLength =
            snprintf(reinterpret_cast<char*>(packet->data), network::udp::kDataSize - 1, json::kConfigReply, IP2STR(network::GetPrimaryIp()),
                     IP2STR(network::GetNetmask()), IP2STR(network::GetGatewayIp()), active_ports_ > 0 ? count_ : 0, active_ports_ > 1 ? count_ : 0,
#if CONFIG_DMXNODE_PIXEL_MAX_PORTS > 2
                     active_ports_ > 2 ? count_ : 0, active_ports_ > 3 ? count_ : 0, active_ports_ > 4 ? count_ : 0, active_ports_ > 5 ? count_ : 0,
                     active_ports_ > 6 ? count_ : 0, active_ports_ > 7 ? count_ : 0,
#endif
#if CONFIG_DMXNODE_PIXEL_MAX_PORTS == 16
                     active_ports_ > 8 ? count_ : 0, active_ports_ > 9 ? count_ : 0, active_ports_ > 10 ? count_ : 0, active_ports_ > 11 ? count_ : 0,
                     active_ports_ > 12 ? count_ : 0, active_ports_ > 13 ? count_ : 0, active_ports_ > 14 ? count_ : 0, active_ports_ > 15 ? count_ : 0,
#endif
                     ddpdisplay::configuration::dmx::kMaxPorts == 0 ? 0 : dmxnode::kUniverseSize,
                     ddpdisplay::configuration::dmx::kMaxPorts == 0 ? 0 : dmxnode::kUniverseSize);

        packet->header.flags1 = flags1::VER1 | flags1::REPLY | flags1::PUSH;
        packet->header.len[0] = static_cast<uint8_t>(kLength >> 8);
        packet->header.len[1] = static_cast<uint8_t>(kLength);

        network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&packet), HEADER_LEN + kLength, from_ip_, ddp::kUdpPort);

        debug::Dump(&packet, HEADER_LEN + kLength);
    }

    DEBUG_EXIT();
}

void DdpDisplay::HandleData()
{
    const auto* const kPacket = reinterpret_cast<ddp::Packet*>(receive_buffer_);
    auto offset = static_cast<uint32_t>((kPacket->header.offset[0] << 24) | (kPacket->header.offset[1] << 16) | (kPacket->header.offset[2] << 8) |
                                        kPacket->header.offset[3]);
    auto length = ((static_cast<uint32_t>(kPacket->header.len[0]) << 8) | kPacket->header.len[1]);
    const auto* const kReceivedData = kPacket->data;

    uint32_t data_source_index = 0;
    uint32_t receiver_buffer_index = 0;

    for (uint32_t port_index = 0; (port_index < active_ports_) && (length != 0); port_index++)
    {
        data_source_index = port_index * 4;

        const auto kOutportIndexEnd = data_source_index + 4;

        while ((offset < s_offset_compare[port_index]) && (data_source_index < kOutportIndexEnd))
        {
            const auto kOutLength = std::min(std::min(length, dmxnode_output_type_data_max_length_), strip_data_length_);

            dmxnode::Data::SetSourceA(data_source_index, &kReceivedData[receiver_buffer_index], kOutLength);
            s_port_length[data_source_index] = kOutLength;

            receiver_buffer_index += kOutLength;
            offset += kOutLength;
            length -= kOutLength;
            data_source_index++;
        }
    }

    /*
     * 2x DMX ports
     */

    data_source_index = ddpdisplay::lightset::kMaxPorts - ddpdisplay::configuration::dmx::kMaxPorts;

    DEBUG_PRINTF("nLightSetPortIndex=%u", data_source_index);

    for (uint32_t port_index = ddpdisplay::configuration::pixel::kMaxPorts; (port_index < ddpdisplay::configuration::kMaxPorts) && (length != 0); port_index++)
    {
        if (offset < s_offset_compare[port_index])
        {
            const auto kLength = std::min(length, dmxnode::kUniverseSize);

            //			DEBUG_PRINTF("==> nPortIndex=%u, nOffset=%u, nLength=%u, nLightSetLength=%u, nLightSetPortIndex=%u", nPortIndex, nOffset, nLength,
            // nLightSetLength, nLightSetPortIndex);

            dmxnode::Data::SetSourceA(port_index, &kReceivedData[receiver_buffer_index], kLength);
            s_port_length[port_index] = kLength;

            receiver_buffer_index += kLength;
            offset += kLength;
            length -= kLength;
            port_index++;

            //			DEBUG_PRINTF("nPortIndex=%u, nOffset=%u, nLength=%u, nLightSetLength=%u, nLightSetPortIndex=%u", nPortIndex, nOffset, nLength,
            // nLightSetLength, nLightSetPortIndex);
        }
    }

    if ((kPacket->header.flags1 & flags1::PUSH) == flags1::PUSH)
    {
        for (uint32_t data_output_port_index = 0; data_output_port_index < ddpdisplay::lightset::kMaxPorts; data_output_port_index++)
        {
            dmxnode::DataOutput(dmxnode_output_type_, data_output_port_index);
            dmxnode::Data::ClearLength(data_output_port_index);
        }
    }
}

void DdpDisplay::Input(const uint8_t* buffer, uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    if (__builtin_expect((size < HEADER_LEN), 0))
    {
        return;
    }

    if (from_ip_ == network::GetPrimaryIp())
    {
        DEBUG_PUTS("Own message");
        return;
    }

    const auto* packet = reinterpret_cast<const ddp::Packet*>(buffer);

    if ((packet->header.flags1 & flags1::VER_MASK) != flags1::VER1)
    {
        DEBUG_PUTS("Invalid version");
        return;
    }

    if (packet->header.id == id::DISPLAY)
    {
        HandleData();
        return;
    }

    if ((packet->header.flags1 & flags1::QUERY) == flags1::QUERY)
    {
        HandleQuery();
        return;
    }
}

void DdpDisplay::Print()
{
    puts("DDP Display");
    printf(" Count             : %u\n", count_);
    printf(" Channels per pixel: %u\n", GetChannelsPerPixel());
    printf(" Active ports      : %u\n", active_ports_);
}
