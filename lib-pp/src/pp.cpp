/**
 * @file pp.cpp
 *
 */
/*
 *  Universal Discovery Protocol
 *  A UDP protocol for finding Etherdream/Heroic Robotics lighting devices
 *
 *  (c) 2012 Jas Strong and Jacob Potter
 *  <jasmine@electronpusher.org> <jacobdp@gmail.com>
 *
 *	PixelPusherBase/PixelPusherExt split created by Henner Zeller 2016
 *
 *	pusher command stuff added by Christopher Schardt 2017
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "pp.h"
#include "network.h"
#include "hal_millis.h"
#include "dmxnodedata.h"
#include "dmxnode_data.h"
 #include "firmware/debug/debug_debug.h"

#if !defined(CONFIG_PP_16BITSTUFF)
static constexpr uint8_t kCommandMagic[16] = {0x40, 0x09, 0x2d, 0xa6, 0x15, 0xa5, 0xdd, 0xe5, 0x6a, 0x9d, 0x4d, 0x5a, 0xcf, 0x09, 0xaf, 0x50};
#endif

typedef union pcast32
{
    uint32_t u32;
    uint8_t u8[4];
} _pcast32;

PixelPusher::PixelPusher() : millis_(hal::Millis())
{
    DEBUG_ENTRY();
    assert(s_this == nullptr);
    s_this = this;

    memset(&discovery_packet_, 0, sizeof(struct pp::DiscoveryPacket));

     network::iface::CopyMacAddressTo(discovery_packet_.header.mac_address);
    discovery_packet_.header.device_type = static_cast<uint8_t>(pp::DeviceType::PIXELPUSHER);
    discovery_packet_.header.protocol_version = 1;
    discovery_packet_.header.vendor_id = 3;
    // discovery_packet_.header.hw_revision = ;
    discovery_packet_.header.sw_revision = pp::version::MIN;
    discovery_packet_.header.link_speed = 10000000;
    //
    discovery_packet_.pixelpusher.base.update_period = 1000;
    discovery_packet_.pixelpusher.base.power_total = 1;
    discovery_packet_.pixelpusher.base.max_strips_per_packet = 1; // TODO (a) This can be active_ports_ ?
    discovery_packet_.pixelpusher.base.my_port = pp::UDP_PORT_DATA;
    //
    discovery_packet_.pixelpusher.ext.segments = 1;

    DEBUG_EXIT();
}

void PixelPusher::Start()
{
    DEBUG_ENTRY();
    assert(dmxnode_output_type_ != nullptr);

    handle_discovery_ = network::udp::Begin(pp::UDP_PORT_DISCOVERY, nullptr);
    assert(handle_discovery_ != -1);

    handle_data_ = network::udp::Begin(pp::UDP_PORT_DATA, StaticCallbackFunction);
    assert(handle_data_ != -1);

#if !defined(CONFIG_PP_16BITSTUFF)
    discovery_packet_.pixelpusher.base.strips_attached = static_cast<uint8_t>(active_ports_);
#else
    discovery_packet_.pixelpusher.base.strips_attached = 1;
#endif
    discovery_packet_.pixelpusher.base.pixels_per_strip = static_cast<uint16_t>(count_);
    discovery_packet_.pixelpusher.ext.strip_count_16 = static_cast<uint16_t>(active_ports_);
#if !defined(CONFIG_PP_16BITSTUFF)
    discovery_packet_.pixelpusher.ext.pusher_flags = 0;
#else
    static const uint32_t nPusherFlags = (m_hasGlobalBrightness ? static_cast<uint32_t>(pp::PusherFlags::GLOBAL_BRIGHTNESS) : 0) |
                                         static_cast<uint32_t>(pp::PusherFlags::DYNAMICS) | static_cast<uint32_t>(pp::PusherFlags::_16BITSTUFF);
    discovery_packet_.pixelpusher.ext.pusher_flags = nPusherFlags;
#endif

    strip_data_length_ = 1U + count_ * pp::configuration::CHANNELS_PER_PIXEL;

    DEBUG_EXIT();
}

void PixelPusher::Stop()
{
    DEBUG_ENTRY();

    handle_data_ = network::udp::End(pp::UDP_PORT_DATA);
    handle_data_ = -1;

    network::udp::End(pp::UDP_PORT_DISCOVERY);
    handle_discovery_ = -1;

    DEBUG_EXIT();
}

void PixelPusher::Input(const uint8_t* buffer, uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    if (__builtin_expect((size < 4), 0)) return;

    auto* data = buffer;

    uint32_t sequence_number;
    memcpy(&sequence_number, data, 4);
    size -= 4;
    data += 4;

#if !defined(CONFIG_PP_16BITSTUFF)
    if (size >= sizeof(kCommandMagic) && memcmp(data, kCommandMagic, sizeof(kCommandMagic)) == 0)
    {
        HandlePusherCommand(data + sizeof(kCommandMagic), size - sizeof(kCommandMagic));
        return;
    }

    if (size % strip_data_length_ != 0)
    {
        DEBUG_PRINTF("Expecting multiple of {1 + (RGB)*%u} = %u but got %u bytes (leftover: %u)", count_, strip_data_length_, size, size % strip_data_length_);
        return;
    }

    const auto kReceivedStrips = size / strip_data_length_;

    for (uint32_t i = 0; i < kReceivedStrips; i++)
    {
        const auto kPortIndexStart = data[0] * universes_;
        uint32_t port_index;
        for (port_index = kPortIndexStart; port_index < (kPortIndexStart + universes_) && (size > 0); port_index++)
        {
            const auto kLength = std::min(std::min(size, pp::configuration::UNIVERSE_MAX_LENGTH), strip_data_length_ - 1);

            //          DEBUG_PRINTF("i=%u, port_index=%u, size=%u, kLength=%u", i, port_index, size, kLength);

            dmxnode::Data::SetSourceA(port_index, &data[1], kLength);

            size -= kLength;
            data += kLength;
        }

        data += strip_data_length_;

        //		DEBUG_PRINTF("nPortIndex=%u, port_index_last_=%u", nPortIndex, port_index_last_);

        if (port_index == port_index_last_)
        {
            for (uint32_t type_port_index = 0; type_port_index < port_index_last_; type_port_index++)
            {
                dmxnode::DataOutput(dmxnode_output_type_, type_port_index);
                dmxnode::Data::ClearLength(type_port_index);
            }
        }
    }
#else
#endif
}

void PixelPusher::Run()
{
    const auto kMillis = hal::Millis();
    if (__builtin_expect((kMillis - millis_ < 1000), 1))
    {
        return;
    }
    millis_ = kMillis;
    _pcast32 src;
    src.u32 = network::GetPrimaryIp();
    memcpy(discovery_packet_.header.ip_address, src.u8, 4);
    network::udp::Send(handle_discovery_, reinterpret_cast<const uint8_t*>(&discovery_packet_), sizeof(struct pp::DiscoveryPacket), static_cast<uint32_t>(~0),
                   pp::UDP_PORT_DISCOVERY);
}

void PixelPusher::HandlePusherCommand([[maybe_unused]] const uint8_t* buffer, [[maybe_unused]] uint32_t size)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("pBuffer=%p, nSize=%u", reinterpret_cast<const void*>(buffer), size);
#if !defined(CONFIG_PP_16BITSTUFF)
#else
#endif
    DEBUG_EXIT();
}

void PixelPusher::Print()
{
    puts("PixelPusher");
    printf(" Count             : %u\n", count_);
    printf(" Channels per pixel: %u\n", pp::configuration::CHANNELS_PER_PIXEL);
    printf(" Active ports      : %u\n", active_ports_);
    DEBUG_PRINTF("universes_=%u", universes_);
    DEBUG_PRINTF("port_index_last_=%u", port_index_last_);
}
