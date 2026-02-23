/**
 * @file net_private.h
 *
 */
/* Copyright (C) 2023-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_NET_PRIVATE_H_
#define NET_NET_PRIVATE_H_

#include <cstdint>

#include "net_platform.h"
#include "core/protocol/icmp.h"
#include "core/protocol/igmp.h"
#include "core/protocol/udp.h"
#include "core/protocol/tcp.h"

#ifndef ALIGNED
#define ALIGNED __attribute__((aligned(4)))
#endif

namespace console
{
void Error(const char*);
}

uint8_t* emac_eth_send_get_dma_buffer();
void emac_eth_send(const uint32_t);
void emac_eth_send(void*, const uint32_t);
#if defined CONFIG_NET_ENABLE_PTP
void emac_eth_send_timestamp(const uint32_t);
void emac_eth_send_timestamp(void*, const uint32_t);
#endif
uint32_t emac_eth_recv(uint8_t**);
void emac_free_pkt();

namespace network
{
namespace global
{
extern uint32_t broadcast_mask;
extern uint32_t on_network_mask;
} // namespace global

inline uint16_t Chksum(const void* data, uint32_t length)
{
    auto* ptr = reinterpret_cast<const uint16_t*>(data);
    uint32_t sum = 0;

    while (length > 1)
    {
        sum += *ptr;
        ptr++;
        length -= 2;
    }

    // Add left-over byte, if any
    if (length > 0)
    {
        sum += __builtin_bswap16(static_cast<uint16_t>(*(reinterpret_cast<const uint8_t*>(ptr)) << 8));
    }

    // Fold 32-bit sum into 16 bits
    while (sum >> 16)
    {
        sum = (sum >> 16) + (sum & 0xFFFF);
    }

    return static_cast<uint16_t>(~sum);
}

namespace arp
{
enum class EthSend
{
    kIsNormal
#if defined CONFIG_NET_ENABLE_PTP
    ,
    kIsTimestamp
#endif
};
} // namespace arp

namespace ip
{
void Init();
void Shutdown();
void Handle(struct Header*);
} // namespace ip

namespace igmp
{
void Init();
void Input(const struct Header*);
void Shutdown();
} // namespace igmp

namespace icmp
{
void Input(struct Header*);
void Shutdown();
} // namespace icmp

namespace udp
{
void Init();
void Input(const struct Header*);
void Shutdown();
} // namespace udp

namespace tcp
{
void Init();
void Input(struct Header*);
void Run();
} // namespace tcp
} // namespace network

#endif // NET_NET_PRIVATE_H_
