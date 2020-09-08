#pragma GCC push_options
#pragma GCC optimize ("Os")
/**
 * @file h3_de2.c
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
/*
 * https://github.com/u-boot/u-boot/blob/master/drivers/video/sunxi/sunxi_de2.c
 *
 * Optimized for HDMI/DVI fixed resolution and now can be compiled with
 * -Wpedantic -Wsign-conversion
 *
 */
// SPDX-License-Identifier: GPL-2.0+
/*
 * Allwinner DE2 display driver
 *
 * (C) Copyright 2017 Jernej Skrabec <jernej.skrabec@siol.net>
 */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "h3_de2.h"

#include "h3.h"
#include "h3_ccu.h"

#include "display_timing.h"

/*
 * Bits per pixel selector. Each value n is such that the bits-per-pixel is
 * 2 ^ n
 */
enum video_log2_bpp {
	VIDEO_BPP1	= 0,
	VIDEO_BPP2,
	VIDEO_BPP4,
	VIDEO_BPP8,
	VIDEO_BPP16,
	VIDEO_BPP32,
};

#define PLL_DE_ENABLE				(1U << 31)
#define PLL_DE_LOCK					(1U << 28)
#define PLL_DE_MODE_INTEGER			(1U << 24)
#define PLL_FACTOR_N_MASK			0x7F
#define PLL_FACTOR_N_SHIFT			8U
	#define PLL_DE_CTRL_N(n)			((((n) - 1) & PLL_FACTOR_N_MASK) << PLL_FACTOR_N_SHIFT)
#define PLL_FACTOR_M_MASK			0x0F
#define PLL_FACTOR_M_SHIFT			0U
	#define PLL_DE_CTRL_M(n)			((((n) - 1) & PLL_FACTOR_M_MASK) << PLL_FACTOR_M_SHIFT)

#define CCU_DE2_CLK_GATE			(1U << 31)
#define CCU_DE2_CLK_SRC_SEL_MASK	3U
#define CCU_DE2_CLK_SRC_SEL_SHIFT	24U
#define CCU_DE2_CLK_SRC_PLL_DE		1U
	#define PLL_DE_CTRL_CLK_SRC(n)	(n << CCU_DE2_CLK_SRC_SEL_SHIFT)

#define BIT(nr)                		(1UL << (nr))

static void clock_set_pll_de(const uint32_t clk) {
	const uint32_t m = 2; /* 12 MHz steps */

	H3_CCU->PLL_DE_CTRL = PLL_DE_ENABLE | PLL_DE_MODE_INTEGER | PLL_DE_CTRL_N(clk / (H3_F_24M / m))| PLL_DE_CTRL_M(m);

	while (!(H3_CCU->PLL_DE_CTRL & PLL_DE_LOCK))
		;
}

void de2_composer_init(void) {
	clock_set_pll_de(432000000);

	H3_CCU->BUS_CLK_GATING1 |= CCU_BUS_CLK_GATING1_DE;
	H3_CCU->BUS_SOFT_RESET1 |= CCU_BUS_SOFT_RESET1_DE;

	H3_CCU->DE_CLK = (H3_CCU->DE_CLK & ~(CCU_DE2_CLK_SRC_SEL_MASK << CCU_DE2_CLK_SRC_SEL_SHIFT)) | (PLL_DE_CTRL_CLK_SRC(CCU_DE2_CLK_SRC_PLL_DE));
	H3_CCU->DE_CLK |= (CCU_DE2_CLK_GATE);
}

void de2_mode_set(const struct display_timing *mode, const uint32_t bpp, uint32_t address) {
	const uint32_t size = H3_DE2_WH(mode->hactive.typ, mode->vactive.typ);
	uint32_t channel;
	uint32_t format;

	H3_DE2->RST |= 1;
	H3_DE2->GATE |= BIT(0);
	H3_DE2->BUS |= BIT(0);
	H3_DE2->SEL &= ~(1U);

	H3_DE2_MUX0_GLB->CTL = H3_DE2_MUX_GLB_CTL_EN;
	H3_DE2_MUX0_GLB->STATUS = 0;
	H3_DE2_MUX0_GLB->SIZE = size;

	for (channel = 0; channel < 4; channel++) {
		void *ch = (void *)(H3_DE2_MUX0_CHAN_BASE + H3_DE2_MUX_CHAN_SZ * channel);
		memset(ch, 0, (channel == 0) ? sizeof(struct T_H3_DE2_VI) : sizeof(struct T_H3_DE2_UI));
	}

	memset(H3_DE2_MUX0_BLD, 0, sizeof(struct T_H3_DE2_BLD));

	H3_DE2_MUX0_BLD->FCOLOR_CTL = 0x00000101;
	H3_DE2_MUX0_BLD->ROUTE = 1;
	H3_DE2_MUX0_BLD->PREMULTIPLY = 0;
	H3_DE2_MUX0_BLD->BKCOLOR = 0xff000000;
	H3_DE2_MUX0_BLD->BLD_MODE[0] = 0x03010301;
	H3_DE2_MUX0_BLD->OUTPUT_SIZE = size;
	H3_DE2_MUX0_BLD->OUT_CTL = mode->flags & DISPLAY_FLAGS_INTERLACED ? 2 : 0;
	H3_DE2_MUX0_BLD->CK_CTL = 0;
	H3_DE2_MUX0_BLD->ATTR[0].FCOLOR = 0xff000000;
	H3_DE2_MUX0_BLD->ATTR[0].INSIZE = size;

	/* Disable all other units */
	*((volatile uint32_t *) H3_DE2_MUX0_VSU_BASE) = 0;
	*((volatile uint32_t *) H3_DE2_MUX0_GSU1_BASE) = 0;
	*((volatile uint32_t *) H3_DE2_MUX0_GSU2_BASE) = 0;
	*((volatile uint32_t *) H3_DE2_MUX0_GSU3_BASE) = 0;
	*((volatile uint32_t *) H3_DE2_MUX0_FCE_BASE) = 0;
	*((volatile uint32_t *) H3_DE2_MUX0_BWS_BASE) = 0;
	*((volatile uint32_t *) H3_DE2_MUX0_LTI_BASE) = 0;
	*((volatile uint32_t *) H3_DE2_MUX0_PEAK_BASE) = 0;
	*((volatile uint32_t *) H3_DE2_MUX0_ASE_BASE) = 0;
	*((volatile uint32_t *) H3_DE2_MUX0_FCC_BASE) = 0;

	H3_DE2_MUX0_CSC->CTL = 0;

	switch (bpp) {
	case 16:
		format = H3_DE2_UI_CFG_ATTR_FMT(H3_DE2_UI_FORMAT_RGB_565);
		break;
	case 32:
	default:
		format = H3_DE2_UI_CFG_ATTR_FMT(H3_DE2_UI_FORMAT_XRGB_8888);
		break;
	}

	H3_DE2_MUX0_UI->CFG[0].ATTR = (H3_DE2_UI_CFG_ATTR_EN | format);
	H3_DE2_MUX0_UI->CFG[0].SIZE = size;
	H3_DE2_MUX0_UI->CFG[0].COORD = 0;
	H3_DE2_MUX0_UI->CFG[0].PITCH = (bpp / 8) * mode->hactive.typ;
	H3_DE2_MUX0_UI->CFG[0].TOP_LADDR = address;
	H3_DE2_MUX0_UI->OVL_SIZE = size;

	H3_DE2_MUX0_GLB->DBUFFER = 1;
}

extern int h3_hdmi_probe(void);
extern int h3_hdmi_enable(uint32_t panel_bpp, const struct display_timing *edid);

void __attribute__((cold)) h3_de2_init(struct display_timing *timing, uint32_t fbbase) {
	h3_hdmi_probe();

	de2_composer_init();
	de2_mode_set(timing, 1 << VIDEO_BPP32, fbbase);

	h3_hdmi_enable(1 << VIDEO_BPP32, timing);
}
