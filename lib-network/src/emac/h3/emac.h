/**
 * @file emac.h
 *
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

#ifndef EMAC_H_
#define EMAC_H_

#define BUS_SOFT_RESET2_EPHY_RST 	(1 << 2)
#define BUS_CLK_GATING4_EPHY_GATING	(1 << 0)

#define CTL0_DUPLEX_FULL_DUPLEX		(1 << 0)
#define CTL0_SPEED_10M				(2 << 2)
#define CTL0_SPEED_100M				(3 << 2)
	#define CTL0_SPEED_SHIFT	2
	#define CTL0_SPEED_MASK		3

#define TX_CTL0_TX_EN				(1U << 31)
#define TX_CTL1_TX_DMA_EN			(1U << 30)

#define RX_CTL0_RX_EN				(1U << 31)
#define RX_CTL1_RX_DMA_EN			(1U << 30)

#define RX_FRM_FLT_HASH_MULTICAST	(1U << 9)
#define RX_FRM_FLT_RX_ALL_MULTICAST	(1U << 16)

#define PHY_ADDR		1

#define	ARM_DMA_ALIGN	64

#define CONFIG_TX_DESCR_NUM	48
#define CONFIG_RX_DESCR_NUM 48
#define CONFIG_ETH_BUFSIZE	2048 /* Note must be DMA aligned */
/*
 * The datasheet says that each descriptor can transfers up to 4096 bytes
 * But later, the register documentation reduces that value to 2048,
 * using 2048 cause strange behaviors and even BSP driver use 2047
 */
#define CONFIG_ETH_RXSIZE	2044 /* Note must fit in ETH_BUFSIZE */

#define TX_TOTAL_BUFSIZE	(CONFIG_ETH_BUFSIZE * CONFIG_TX_DESCR_NUM)
#define RX_TOTAL_BUFSIZE	(CONFIG_ETH_BUFSIZE * CONFIG_RX_DESCR_NUM)

#define __aligned(x)            __attribute__((aligned(x)))

struct emac_dma_desc {
	uint32_t status;
	uint32_t st;
	uint32_t buf_addr;
	uint32_t next;
} __aligned(ARM_DMA_ALIGN);

struct coherent_region {
	struct emac_dma_desc rx_chain[CONFIG_TX_DESCR_NUM];
	struct emac_dma_desc tx_chain[CONFIG_RX_DESCR_NUM];
	char rxbuffer[RX_TOTAL_BUFSIZE] __aligned(ARM_DMA_ALIGN);
	char txbuffer[TX_TOTAL_BUFSIZE] __aligned(ARM_DMA_ALIGN);
	uint32_t rx_currdescnum;
	uint32_t tx_currdescnum;
};

#endif /* EMAC_H_ */
