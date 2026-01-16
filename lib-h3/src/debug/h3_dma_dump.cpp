/**
 * @file h3_dma_dump.cpp
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

#include <stdint.h>

#include "h3_dma.h"
#include "h3.h"

namespace uart0
{
void Puts(const char *);
int Printf(const char* fmt, ...);
}

void __attribute__((cold)) h3_dma_dump_lli(const struct sunxi_dma_lli *lli) {
	uart0::Printf("DMA lli %p:\n", (void *)lli);
	uart0::Printf("\tcfg        0x%08x\n", lli->cfg);
	uart0::Printf("\tsrc        0x%08x\n", lli->src);
	uart0::Printf("\tdst        0x%08x\n", lli->dst);
	uart0::Printf("\tlen        0x%08x\n", lli->len);
	uart0::Printf("\tpara       0x%08x\n", lli->para);
	uart0::Printf("\tp_lli_next 0x%08x\n", lli->p_lli_next);
    uart0::Puts("");
}

void __attribute__((cold)) h3_dma_dump_common(void) {
	uart0::Printf("DMA common registers:\n");
	uart0::Printf("\t[%p] IRQ_EN0    0x%08x\n", &H3_DMA->IRQ_EN0, H3_DMA->IRQ_EN0);
	uart0::Printf("\t[%p] IRQ_EN1    0x%08x\n", &H3_DMA->IRQ_EN1, H3_DMA->IRQ_EN1);
	uart0::Printf("\t[%p] IRQ_PEND0  0x%08x\n", &H3_DMA->IRQ_PEND0, H3_DMA->IRQ_PEND0);
	uart0::Printf("\t[%p] IRQ_PEND1  0x%08x\n", &H3_DMA->IRQ_PEND1, H3_DMA->IRQ_PEND1);
	uart0::Printf("\t[%p] SEC        0x%08x\n", &H3_DMA->SEC, H3_DMA->SEC);
	uart0::Printf("\t[%p] AUTO_GATE  0x%08x\n", &H3_DMA->AUTO_GATE, H3_DMA->AUTO_GATE);
	uart0::Printf("\t[%p] STA        0x%08x\n", &H3_DMA->STA, H3_DMA->STA);
	uart0::Puts("");
}

void __attribute__((cold)) h3_dma_dump_chl(uint32_t base) {
	const uint32_t kChl = (base - H3_DMA_CHL0_BASE) / 0x40;
	H3_DMA_CHL_TypeDef *p = (H3_DMA_CHL_TypeDef *)base;

	uart0::Printf("DMA chl%d registers:\n", kChl);
	uart0::Printf("\t[%p] EN         0x%08x\n", &p->EN, p->EN);
	uart0::Printf("\t[%p] PAU        0x%08x\n", &p->PAU, p->PAU);
	uart0::Printf("\t[%p] DESC_ADDR  0x%08x\n", &p->DESC_ADDR, p->DESC_ADDR);
	uart0::Printf("\t[%p] CFG        0x%08x\n", &p->CFG, p->CFG);
	uart0::Printf("\t\tSRC_DRQ=%d\n",  (p->CFG & (0xf <<  0)) >>  0);
	uart0::Printf("\t\tSRC_MODE=%d\n", (p->CFG & (0x1 <<  5)) >>  5);
	uart0::Printf("\t\tSRC_BURST=%d\n",(p->CFG & (0x3 <<  6)) >>  6);
	uart0::Printf("\t\tSRC_WIDTH=%d\n",(p->CFG & (0x3 <<  9)) >>  9);
	uart0::Printf("\t\tDST_DRQ=%d\n",  (p->CFG & (0xf << 16)) >> 16);
	uart0::Printf("\t\tDST_MODE=%d\n", (p->CFG & (0x1 << 21)) >> 21);
	uart0::Printf("\t\tDST_BURST=%d\n",(p->CFG & (0x3 << 22)) >> 22);
	uart0::Printf("\t\tDST_WIDTH=%d\n",(p->CFG & (0x3 << 25)) >> 25);
	uart0::Printf("\t[%p] CUR_SRC    0x%08x\n", &p->CUR_SRC, p->CUR_SRC);
	uart0::Printf("\t[%p] CUR_DST    0x%08x\n", &p->CUR_DST, p->CUR_DST);
	uart0::Printf("\t[%p] BCNT_LEFT  0x%08x\n", &p->BCNT_LEFT, p->BCNT_LEFT);
	uart0::Printf("\t[%p] PARA       0x%08x\n", &p->PARA, p->PARA);
	uart0::Printf("\t[%p] FDESC_ADDR 0x%08x\n", &p->FDESC_ADDR, p->FDESC_ADDR);
	uart0::Printf("\t[%p] PKG_NUM    0x%08x\n", &p->PKG_NUM, p->PKG_NUM);
	uart0::Puts("");
}
