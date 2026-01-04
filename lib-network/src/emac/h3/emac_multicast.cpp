/**
 * @file emac_multicast.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_EMAC_IGMP)
#undef NDEBUG
#endif

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif

#include <cstdint>

#include "h3.h"
#include "emac.h"
#include "net/ip4_address.h"
#include "firmware/debug/debug_debug.h"

namespace network
{
uint32_t Crc(const uint8_t* data, size_t length);
}

namespace emac::multicast
{
void EnableHashFilter()
{
    DEBUG_ENTRY();

    auto reg_filter = H3_EMAC->RX_FRM_FLT;
    reg_filter &= ~RX_FRM_FLT_RX_ALL_MULTICAST;
    reg_filter |= RX_FRM_FLT_HASH_MULTICAST;
    H3_EMAC->RX_FRM_FLT = reg_filter;

    H3_EMAC->RX_HASH_0 = 0;
    H3_EMAC->RX_HASH_1 = 0;

    DEBUG_EXIT();
}

void DisableHashFilter()
{
    DEBUG_ENTRY();

    auto reg_filter = H3_EMAC->RX_FRM_FLT;
    reg_filter &= ~RX_FRM_FLT_HASH_MULTICAST;
    reg_filter |= RX_FRM_FLT_RX_ALL_MULTICAST;
    H3_EMAC->RX_FRM_FLT = reg_filter;

    H3_EMAC->RX_HASH_0 = 0;
    H3_EMAC->RX_HASH_1 = 0;

    DEBUG_EXIT();
}

void SetHash(const uint8_t* mac_addr)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("RX_FRM_FLT: 0x%08X", H3_EMAC->RX_FRM_FLT);

    const auto kCrc = network::Crc(mac_addr, 6);
    const auto kHash = (kCrc >> 26) & 0x3F;

    if (kHash > 31)
    {
        H3_EMAC->RX_HASH_0 |= (1U << (kHash - 32));
    }
    else
    {
        H3_EMAC->RX_HASH_1 |= (1U << kHash);
    }

    DEBUG_PRINTF("MAC: " MACSTR " -> CRC32: 0x%08X -> Hash Index: %d", MAC2STR(mac_addr), kCrc, kHash);
    DEBUG_PRINTF("RX_HASH_0: 0x%08X, RX_HASH_1: 0x%08X", H3_EMAC->RX_HASH_0, H3_EMAC->RX_HASH_1);
    DEBUG_EXIT();
}

void ResetHash()
{
    DEBUG_ENTRY();

    H3_EMAC->RX_HASH_0 = 0;
    H3_EMAC->RX_HASH_1 = 0;

    DEBUG_EXIT();
}
} // namespace emac::multicast