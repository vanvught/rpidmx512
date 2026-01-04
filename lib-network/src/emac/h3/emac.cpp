/**
 * @file emac.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdbool.h>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "emac.h"
#include "emac/phy.h"
#include "emac/mmi.h"
#include "h3.h"
#include "firmware/debug/debug_printbits.h"
 #include "firmware/debug/debug_debug.h"

#define PHY_ADDRESS 1

#define H3_EPHY_DEFAULT_VALUE 0x00058000
#define H3_EPHY_DEFAULT_MASK 0xFFFF8000
#define H3_EPHY_ADDR_SHIFT 20
#define H3_EPHY_LED_POL (1U << 17)  // 1: active low, 0: active high
#define H3_EPHY_SHUTDOWN (1U << 16) // 1: shutdown, 0: power up
#define H3_EPHY_SELECT (1U << 15)   // 1: internal PHY, 0: external PHY

coherent_region* p_coherent_region = nullptr;

extern void mac_address_get(uint8_t paddr[]);

namespace net::emac
{
void AdjustLink(net::phy::Status phy_status)
{
    DEBUG_ENTRY();

    printf("Link %s, %d, %s\n", phy_status.link == net::phy::Link::kStateUp ? "Up" : "Down", phy_status.speed == net::phy::Speed::kSpeed10 ? 10 : 100,
           phy_status.duplex == net::phy::Duplex::DUPLEX_HALF ? "HALF" : "FULL");

    auto value = H3_EMAC->CTL0;

    if (phy_status.duplex == net::phy::Duplex::DUPLEX_FULL)
    {
        value |= CTL0_DUPLEX_FULL_DUPLEX;
    }
    else
    {
        value &= static_cast<uint32_t>(~CTL0_DUPLEX_FULL_DUPLEX);
    }

    value &= static_cast<uint32_t>(~(CTL0_SPEED_MASK << CTL0_SPEED_SHIFT));

    switch (phy_status.speed)
    {
        case net::phy::Speed::kSpeed10:
            value |= CTL0_SPEED_10M;
            break;
        default:
            value |= CTL0_SPEED_100M;
            break;
    }

    H3_EMAC->CTL0 = value;

    DEBUG_EXIT();
}

static void RxDescsInit()
{
    struct emac_dma_desc* desc_table_p = &p_coherent_region->rx_chain[0];
    char* rxbuffs = &p_coherent_region->rxbuffer[0];
    struct emac_dma_desc* desc_p;
    uint32_t idx;

    for (idx = 0; idx < CONFIG_RX_DESCR_NUM; idx++)
    {
        desc_p = &desc_table_p[idx];
        desc_p->buf_addr = (uintptr_t)&rxbuffs[idx * CONFIG_ETH_BUFSIZE];
        desc_p->next = (uintptr_t)&desc_table_p[idx + 1];
        desc_p->st = CONFIG_ETH_RXSIZE;
        desc_p->status = (1U << 31);
    }

    /* Correcting the last pointer of the chain */
    desc_p->next = (uintptr_t)&desc_table_p[0];

    H3_EMAC->RX_DMA_DESC = (uintptr_t)&desc_table_p[0];
    p_coherent_region->rx_currdescnum = 0;
}

static void TxDescsInit()
{
    struct emac_dma_desc* desc_table_p = &p_coherent_region->tx_chain[0];
    char* txbuffs = &p_coherent_region->txbuffer[0];
    struct emac_dma_desc* desc_p;
    uint32_t idx;

    for (idx = 0; idx < CONFIG_TX_DESCR_NUM; idx++)
    {
        desc_p = &desc_table_p[idx];
        desc_p->buf_addr = (uintptr_t)&txbuffs[idx * CONFIG_ETH_BUFSIZE];
        desc_p->next = (uintptr_t)&desc_table_p[idx + 1];
        desc_p->status = (1U << 31);
        desc_p->st = 0;
    }

    /* Correcting the last pointer of the chain */
    desc_p->next = (uintptr_t)&desc_table_p[0];

    H3_EMAC->TX_DMA_DESC = (uintptr_t)&desc_table_p[0];
    p_coherent_region->tx_currdescnum = 0;
}

void __attribute__((cold)) Config()
{
    DEBUG_ENTRY();

    H3_CCU->BUS_SOFT_RESET2 |= BUS_SOFT_RESET2_EPHY_RST;
    udelay(1000); // 1ms
    H3_CCU->BUS_CLK_GATING4 &= static_cast<uint32_t>(~BUS_CLK_GATING4_EPHY_GATING);
    udelay(1000); // 1ms
    H3_CCU->BUS_CLK_GATING4 |= BUS_CLK_GATING4_EPHY_GATING;

    /* H3 based SoC's that has an Internal 100MBit PHY
     * needs to be configured and powered up before use
     */
    auto value = H3_SYSTEM->EMAC_CLK;

    value &= ~H3_EPHY_DEFAULT_MASK;
    value |= H3_EPHY_DEFAULT_VALUE;
    value |= PHY_ADDRESS << H3_EPHY_ADDR_SHIFT;
    value &= ~H3_EPHY_SHUTDOWN;
    value |= H3_EPHY_SELECT;

    H3_SYSTEM->EMAC_CLK = value;

    net::phy::Config(PHY_ADDRESS);

    DEBUG_EXIT();
}

void __attribute__((cold)) Start(uint8_t mac_address[], net::phy::Link& link)
{
    DEBUG_ENTRY();

    mac_address_get(mac_address);

    net::phy::Status phy_status;

    phy_status.duplex = net::phy::Duplex::DUPLEX_HALF;
    phy_status.speed = net::phy::Speed::kSpeed10;

    net::phy::Start(PHY_ADDRESS, phy_status);

    link = phy_status.link;

    AdjustLink(phy_status);

#ifndef NDEBUG
    printf("sizeof(struct coherent_region)=%u\n", sizeof(struct coherent_region));
    printf("H3_SYSTEM->EMAC_CLK=%p ", H3_SYSTEM->EMAC_CLK);
    debug::PrintBits(H3_SYSTEM->EMAC_CLK);
    printf("H3_EMAC->CTL0=%p ", H3_EMAC->CTL0);
    debug::PrintBits(H3_EMAC->CTL0);
#endif

    assert(p_coherent_region == nullptr);
    assert(sizeof(struct coherent_region) < MEGABYTE / 2);

    p_coherent_region = (struct coherent_region*)H3_MEM_COHERENT_REGION;

    auto value = H3_EMAC->RX_CTL0;
    value &= ~RX_CTL0_RX_EN;
    H3_EMAC->RX_CTL0 = value;

    value = H3_EMAC->TX_CTL0;
    value &= ~TX_CTL0_TX_EN;
    H3_EMAC->TX_CTL0 = value;

    value = H3_EMAC->TX_CTL1;
    value &= (uint32_t)~TX_CTL1_TX_DMA_EN;
    H3_EMAC->TX_CTL1 = value;

    value = H3_EMAC->RX_CTL1;
    value &= (uint32_t)~RX_CTL1_RX_DMA_EN;
    H3_EMAC->RX_CTL1 = value;

    RxDescsInit();
    TxDescsInit();

    H3_EMAC->RX_FRM_FLT = RX_FRM_FLT_RX_ALL_MULTICAST;

    value = H3_EMAC->RX_CTL1;
    value |= RX_CTL1_RX_DMA_EN;
    H3_EMAC->RX_CTL1 = value;

    value = H3_EMAC->TX_CTL1;
    value |= TX_CTL1_TX_DMA_EN;
    H3_EMAC->TX_CTL1 = value;

    value = H3_EMAC->RX_CTL0;
    value |= RX_CTL0_RX_EN;
    H3_EMAC->RX_CTL0 = value;

    value = H3_EMAC->TX_CTL0;
    value |= TX_CTL0_TX_EN;
    H3_EMAC->TX_CTL0 = value;

    //	H3_EMAC->INT_EN = (uint32_t)(~0);

#ifndef NDEBUG
    printf("================\n");
    printf("H3_EMAC->RX_CTL0=%p ", H3_EMAC->RX_CTL0);
    debug::PrintBits(H3_EMAC->RX_CTL0);
    printf("H3_EMAC->RX_CTL1=%p ", H3_EMAC->RX_CTL1);
    debug::PrintBits(H3_EMAC->RX_CTL1);
    printf("H3_EMAC->RX_FRM_FLT=%p ", H3_EMAC->RX_FRM_FLT);
    debug::PrintBits(H3_EMAC->RX_FRM_FLT);
    puts("");
    printf("H3_EMAC->TX_CTL0=%p ", H3_EMAC->TX_CTL0);
    debug::PrintBits(H3_EMAC->TX_CTL0);
    printf("H3_EMAC->TX_CTL1=%p ", H3_EMAC->TX_CTL1);
    debug::PrintBits(H3_EMAC->TX_CTL1);
    puts("");
    printf("H3_EMAC->ADDR[0].LOW=%08x, H3_EMAC->ADDR[0].HIGH=%08x\n", H3_EMAC->ADDR[0].LOW, H3_EMAC->ADDR[0].HIGH);
    puts("");
    printf("H3_EMAC->TX_DMA_STA=%p\n", H3_EMAC->TX_DMA_STA);
    printf("H3_EMAC->TX_CUR_DESC=%p\n", H3_EMAC->TX_CUR_DESC);
    printf("H3_EMAC->TX_CUR_BUF=%p\n", H3_EMAC->TX_CUR_BUF);
    puts("");
    printf("H3_EMAC->RX_DMA_STA=%p\n", H3_EMAC->RX_DMA_STA);
    printf("H3_EMAC->RX_CUR_DESC=%p\n", H3_EMAC->RX_CUR_DESC);
    printf("H3_EMAC->RX_CUR_BUF=%p\n", H3_EMAC->RX_CUR_BUF);
    printf("================\n");
#endif
}
} // namespace net::emac