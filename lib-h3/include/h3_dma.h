/**
 * @file h3_dma.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef H3_DMA_H_
#define H3_DMA_H_

#include <stdint.h>

enum H3_DMA_DRQSRC {
	DRQSRC_SDRAM = 1
};

enum H3_DMA_DRQDST {
	DRQDST_UART0TX = 6,
	DRQDST_UART1TX = 7,
	DRQDST_UART2TX = 8,
	DRQDST_UART3TX = 9
};

	#define DMA_IRQ_EN_MASK			(0x07)
#define DMA_IRQ_EN0_DMA0_HALF_IRQ_EN	(1 << 0)
#define DMA_IRQ_EN0_DMA0_PKG_IRQ_EN		(1 << 1)
#define DMA_IRQ_EN0_DMA0_QUEUE_IRQ_EN	(1 << 2)
	#define DMA_IRQ_EN0_DMA0_SHIFT	(0)
#define DMA_IRQ_EN0_DMA1_HALF_IRQ_EN	(1 << 4)
#define DMA_IRQ_EN0_DMA1_PKG_IRQ_EN		(1 << 5)
#define DMA_IRQ_EN0_DMA1_QUEUE_IRQ_EN	(1 << 6)
	#define DMA_IRQ_EN0_DMA1_SHIFT	(4)
#define DMA_IRQ_EN0_DMA2_HALF_IRQ_EN	(1 << 8)
#define DMA_IRQ_EN0_DMA2_PKG_IRQ_EN		(1 << 9)
#define DMA_IRQ_EN0_DMA2_QUEUE_IRQ_EN	(1 << 10)
	#define DMA_IRQ_EN0_DMA2_SHIFT	(8)
#define DMA_IRQ_EN0_DMA3_HALF_IRQ_EN	(1 << 12)
#define DMA_IRQ_EN0_DMA3_PKG_IRQ_EN		(1 << 13)
#define DMA_IRQ_EN0_DMA3_QUEUE_IRQ_EN	(1 << 14)
	#define DMA_IRQ_EN0_DMA3_SHIFT	(12)

	#define DMA_IRQ_PEND_MASK			(0x07)
#define DMA_IRQ_PEND0_DMA0_HALF_IRQ_EN	(1 << 0)
#define DMA_IRQ_PEND0_DMA0_PKG_IRQ_EN	(1 << 1)
#define DMA_IRQ_PEND0_DMA0_QUEUE_IRQ_EN	(1 << 2)
	#define DMA_IRQ_PEND0_DMA0_SHIFT	(0)
#define DMA_IRQ_PEND0_DMA1_HALF_IRQ_EN	(1 << 4)
#define DMA_IRQ_PEND0_DMA1_PKG_IRQ_EN	(1 << 5)
#define DMA_IRQ_PEND0_DMA1_QUEUE_IRQ_EN	(1 << 6)
	#define DMA_IRQ_PEND0_DMA1_SHIFT	(4)
#define DMA_IRQ_PEND0_DMA2_HALF_IRQ_EN	(1 << 8)
#define DMA_IRQ_PEND0_DMA2_PKG_IRQ_EN	(1 << 9)
#define DMA_IRQ_PEND0_DMA2_QUEUE_IRQ_EN	(1 << 10)
	#define DMA_IRQ_PEND0_DMA2_SHIFT	(8)
#define DMA_IRQ_PEND0_DMA3_HALF_IRQ_EN	(1 << 12)
#define DMA_IRQ_PEND0_DMA3_PKG_IRQ_EN	(1 << 13)
#define DMA_IRQ_PEND0_DMA3_QUEUE_IRQ_EN	(1 << 14)
	#define DMA_IRQ_PEND0_DMA3_SHIFT	(12)

#define DMA_LLI_LAST_ITEM				0xfffff800
#define DMA_NORMAL_WAIT					8

#define DMA_CHAN_ENABLE_START			(1 << 0)
#define DMA_CHAN_ENABLE_STOP			0

#define DMA_CHAN_MAX_DRQ				0x1f
#define DMA_CHAN_CFG_SRC_DRQ(x)			((x) & DMA_CHAN_MAX_DRQ)
#define DMA_CHAN_CFG_SRC_IO_MODE		(1 << 5)
#define DMA_CHAN_CFG_SRC_LINEAR_MODE	(0 << 5)
#define DMA_CHAN_CFG_SRC_BURST(x)		(((x) & 0x3) << 6)
#define DMA_CHAN_CFG_SRC_WIDTH(x)		(((x) & 0x3) << 9)

#define DMA_CHAN_CFG_DST_DRQ(x)			(DMA_CHAN_CFG_SRC_DRQ(x) << 16)
#define DMA_CHAN_CFG_DST_IO_MODE		(DMA_CHAN_CFG_SRC_IO_MODE << 16)
#define DMA_CHAN_CFG_DST_LINEAR_MODE	(DMA_CHAN_CFG_SRC_LINEAR_MODE << 16)
#define DMA_CHAN_CFG_DST_BURST(x)		(DMA_CHAN_CFG_SRC_BURST(x) << 16)
#define DMA_CHAN_CFG_DST_WIDTH(x)		(DMA_CHAN_CFG_SRC_WIDTH(x) << 16)

/*
 * Hardware representation of the LLI
 *
 * The hardware will be fed the physical address of this structure,
 * and read its content in order to start the transfer.
 */
struct sunxi_dma_lli {
	uint32_t cfg;
	uint32_t src;
	uint32_t dst;
	uint32_t len;
	uint32_t para;
	uint32_t p_lli_next;
	/*
	 * This field is not used by the DMA controller, but will be
	 * used by the CPU to go through the list (mostly for dumping
	 * or freeing it).
	 */
	struct sunxi_dma_lli *v_lli_next;
}__attribute__((packed));

#ifdef __cplusplus
extern "C" {
#endif

extern void h3_dma_dump_common(void);
extern void h3_dma_dump_chl(uint32_t base);

extern void h3_dma_dump_lli(const struct sunxi_dma_lli *lli);

#ifdef __cplusplus
}
#endif

#endif /* H3_DMA_H_ */
