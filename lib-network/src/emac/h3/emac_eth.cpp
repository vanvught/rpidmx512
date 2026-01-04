/**
 * @file emac_eth.cpp
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_EMAC)
#undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>

#include "h3.h"
#include "emac.h"
#include "firmware/debug/debug_dump.h"
 #include "firmware/debug/debug_debug.h"

extern struct coherent_region* p_coherent_region;

__attribute__((hot)) uint32_t emac_eth_recv(uint8_t** packetp)
{
    uint32_t status, desc_num = p_coherent_region->rx_currdescnum;
    struct emac_dma_desc* desc_p = &p_coherent_region->rx_chain[desc_num];

    status = desc_p->status;

    /* Check for DMA own bit */
    if (!(status & (1U << 31)))
    {
        uint32_t length = (desc_p->status >> 16) & 0x3FFF;

        if (length < 0x40)
        {
            DEBUG_PUTS("Bad Packet (length < 0x40)");
            return 0;
        }
        else
        {
            if (length > CONFIG_ETH_RXSIZE)
            {
                DEBUG_PRINTF("Received packet is too big (length=%d)\n", length);
                return 0;
            }

            *packetp = reinterpret_cast<uint8_t*>(desc_p->buf_addr);
#ifdef NDEBUG
            debug::Dump(reinterpret_cast<void*>(*packetp), static_cast<uint16_t>(length));
#endif
            return length;
        }
    }

    return 0;
}

void emac_free_pkt()
{
    auto desc_num = p_coherent_region->rx_currdescnum;
    auto* desc_p = &p_coherent_region->rx_chain[desc_num];

    /* Make the current descriptor valid again */
    desc_p->status |= (1U << 31);

    /* Move to next desc and wrap-around condition. */
    if (++desc_num >= CONFIG_RX_DESCR_NUM)
    {
        desc_num = 0;
    }

    p_coherent_region->rx_currdescnum = desc_num;
}

uint8_t* emac_eth_send_get_dma_buffer()
{
    const auto kDescNum = p_coherent_region->tx_currdescnum;
    auto* desc_p = &p_coherent_region->tx_chain[kDescNum];

    /* Mandatory undocumented bit */
    desc_p->st |= (1U << 24);

    return reinterpret_cast<uint8_t*>(desc_p->buf_addr);
}

void emac_eth_send(uint32_t length)
{
    auto desc_num = p_coherent_region->tx_currdescnum;
    auto* desc_p = &p_coherent_region->tx_chain[desc_num];

    desc_p->st = length;

    /* frame end */
    desc_p->st |= (1U << 30);
    desc_p->st |= (1U << 31);

    /*frame begin */
    desc_p->st |= (1U << 29);
    desc_p->status = (1U << 31);

    /* Move to next Descriptor and wrap around */
    if (++desc_num >= CONFIG_TX_DESCR_NUM)
    {
        desc_num = 0;
    }

    p_coherent_region->tx_currdescnum = desc_num;

    /* Start the DMA */
    uint32_t value = H3_EMAC->TX_CTL1;
    value |= (1U << 31); /* mandatory */
    value |= (1U << 30); /* mandatory */
    H3_EMAC->TX_CTL1 = value;
}

void emac_eth_send(void* buffer, uint32_t length)
{
    auto* pDst = emac_eth_send_get_dma_buffer();

    h3_memcpy(pDst, buffer, length);
#ifndef NDEBUG
    debug::Dump(pDst, length);
#endif

    emac_eth_send(length);
}
