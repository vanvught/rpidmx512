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

#include <stdint.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif

#include "h3_dma.h"
#include "h3.h"

void h3_dma_dump_lli(const struct sunxi_dma_lli *lli) {
#ifndef NDEBUG
	printf("DMA lli %p:\n", (void *)lli);
	printf("\tcfg        0x%08x\n", lli->cfg);
	printf("\tsrc        0x%08x\n", lli->src);
	printf("\tdst        0x%08x\n", lli->dst);
	printf("\tlen        0x%08x\n", lli->len);
	printf("\tpara       0x%08x\n", lli->para);
	printf("\tp_lli_next 0x%08x\n", lli->p_lli_next);
	printf("\n");
#endif
}

void h3_dma_dump_common(void) {
#ifndef NDEBUG
	printf("DMA common registers:\n");
	printf("\t[%p] IRQ_EN0    0x%08x\n", &H3_DMA->IRQ_EN0, H3_DMA->IRQ_EN0);
	printf("\t[%p] IRQ_EN1    0x%08x\n", &H3_DMA->IRQ_EN1, H3_DMA->IRQ_EN1);
	printf("\t[%p] IRQ_PEND0  0x%08x\n", &H3_DMA->IRQ_PEND0, H3_DMA->IRQ_PEND0);
	printf("\t[%p] IRQ_PEND1  0x%08x\n", &H3_DMA->IRQ_PEND1, H3_DMA->IRQ_PEND1);
	printf("\t[%p] SEC        0x%08x\n", &H3_DMA->SEC, H3_DMA->SEC);
	printf("\t[%p] AUTO_GATE  0x%08x\n", &H3_DMA->AUTO_GATE, H3_DMA->AUTO_GATE);
	printf("\t[%p] STA        0x%08x\n", &H3_DMA->STA, H3_DMA->STA);
	printf("\n");
#endif
}

void h3_dma_dump_chl(uint32_t base) {
#ifndef NDEBUG
	const uint32_t chl = (base - H3_DMA_CHL0_BASE) / 0x40;
	H3_DMA_CHL_TypeDef *p = (H3_DMA_CHL_TypeDef *)base;

	printf("DMA chl%d registers:\n", chl);
	printf("\t[%p] EN         0x%08x\n", &p->EN, p->EN);
	printf("\t[%p] PAU        0x%08x\n", &p->PAU, p->PAU);
	printf("\t[%p] DESC_ADDR  0x%08x\n", &p->DESC_ADDR, p->DESC_ADDR);
	printf("\t[%p] CFG        0x%08x\n", &p->CFG, p->CFG);
	printf("\t\tSRC_DRQ=%d\n",  (p->CFG & (0xf <<  0)) >>  0);
	printf("\t\tSRC_MODE=%d\n", (p->CFG & (0x1 <<  5)) >>  5);
	printf("\t\tSRC_BURST=%d\n",(p->CFG & (0x3 <<  6)) >>  6);
	printf("\t\tSRC_WIDTH=%d\n",(p->CFG & (0x3 <<  9)) >>  9);
	printf("\t\tDST_DRQ=%d\n",  (p->CFG & (0xf << 16)) >> 16);
	printf("\t\tDST_MODE=%d\n", (p->CFG & (0x1 << 21)) >> 21);
	printf("\t\tDST_BURST=%d\n",(p->CFG & (0x3 << 22)) >> 22);
	printf("\t\tDST_WIDTH=%d\n",(p->CFG & (0x3 << 25)) >> 25);
	printf("\t[%p] CUR_SRC    0x%08x\n", &p->CUR_SRC, p->CUR_SRC);
	printf("\t[%p] CUR_DST    0x%08x\n", &p->CUR_DST, p->CUR_DST);
	printf("\t[%p] BCNT_LEFT  0x%08x\n", &p->BCNT_LEFT, p->BCNT_LEFT);
	printf("\t[%p] PARA       0x%08x\n", &p->PARA, p->PARA);
	printf("\t[%p] FDESC_ADDR 0x%08x\n", &p->FDESC_ADDR, p->FDESC_ADDR);
	printf("\t[%p] PKG_NUM    0x%08x\n", &p->PKG_NUM, p->PKG_NUM);
	printf("\n");
#endif
}
