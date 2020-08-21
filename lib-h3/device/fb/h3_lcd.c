#pragma GCC push_options
#pragma GCC optimize ("Os")
/**
 * @file h3_lcdc.c
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
 * https://github.com/u-boot/u-boot/blob/master/drivers/video/sunxi/lcdc.c
 *
 * Optimized for HDMI/DVI fixed resolution and now can be compiled with
 * -Wpedantic -Wsign-conversion
 *
 */
// SPDX-License-Identifier: GPL-2.0+
/*
 * Timing controller driver for Allwinner SoCs.
 *
 * (C) Copyright 2013-2014 Luc Verhaegen <libv@skynet.be>
 * (C) Copyright 2014-2015 Hans de Goede <hdegoede@redhat.com>
 * (C) Copyright 2017 Jernej Skrabec <jernej.skrabec@siol.net>
 */

#include <stdint.h>
#include <stdbool.h>

#include "h3.h"
#include "h3_ccu.h"

#include "display_timing.h"

#define LCDC_CTRL_TCON_ENABLE				(1U << 31)
#define LCDC_X(x)							(((x) - 1) << 16)
#define LCDC_Y(y)							(((y) - 1) << 0)
#define LCDC_TCON_VSYNC_MASK				(1U << 24)
#define LCDC_TCON_HSYNC_MASK				(1U << 25)
#define LCDC_TCON0_DCLK_ENABLE				(0xf << 28)
#define LCDC_TCON1_CTRL_CLK_DELAY(n)		(((n) & 0x1f) << 4)
#define LCDC_TCON1_CTRL_INTERLACE_ENABLE	(1U << 20)
#define LCDC_TCON1_CTRL_ENABLE				(1U << 31)
#define LCDC_TCON1_TIMING_H_BP(n)			(((n) - 1) << 0)
#define LCDC_TCON1_TIMING_H_TOTAL(n)		(((n) - 1) << 16)
#define LCDC_TCON1_TIMING_V_BP(n)			(((n) - 1) << 0)
#define LCDC_TCON1_TIMING_V_TOTAL(n)		((n) << 16)

/*
 * The LCD0 module is used for HDMI
 */

void h3_lcdc_enable(__attribute__((unused)) int depth) {
	H3_LCD0->GCTL |= LCDC_CTRL_TCON_ENABLE;
}

static uint32_t lcdc_get_clk_delay(const struct display_timing *mode, const uint32_t tcon) {
	uint32_t delay = mode->vfront_porch.typ + mode->vsync_len.typ + mode->vback_porch.typ;

	if (mode->flags & DISPLAY_FLAGS_INTERLACED) {
		delay /= 2;
	}

	if (tcon == 1) {
		delay -= 2;
	}

	return (delay > 30) ? 30 : delay;
}

void h3_lcdc_tcon1_mode_set(const struct display_timing *mode) {
	uint32_t bp, clk_delay, total, yres;

	clk_delay = lcdc_get_clk_delay(mode, 1);

	H3_LCD0->TCON1_CTL = LCDC_TCON1_CTRL_ENABLE | ((mode->flags & DISPLAY_FLAGS_INTERLACED) ? LCDC_TCON1_CTRL_INTERLACE_ENABLE : 0) | LCDC_TCON1_CTRL_CLK_DELAY(clk_delay);

	yres = mode->vactive.typ;

	if (mode->flags & DISPLAY_FLAGS_INTERLACED) {
		yres /= 2;
	}

	H3_LCD0->TCON1_BASIC0 = LCDC_X(mode->hactive.typ) | LCDC_Y(yres);
	H3_LCD0->TCON1_BASIC1 = LCDC_X(mode->hactive.typ) | LCDC_Y(yres);
	H3_LCD0->TCON1_BASIC2 = LCDC_X(mode->hactive.typ) | LCDC_Y(yres);

	bp = mode->hsync_len.typ + mode->hback_porch.typ;
	total = mode->hactive.typ + mode->hfront_porch.typ + bp;

	H3_LCD0->TCON1_BASIC3 = LCDC_TCON1_TIMING_H_TOTAL(total) | LCDC_TCON1_TIMING_H_BP(bp);

	bp = mode->vsync_len.typ + mode->vback_porch.typ;
	total = mode->vactive.typ + mode->vfront_porch.typ + bp;

	if (!(mode->flags & DISPLAY_FLAGS_INTERLACED)) {
		total *= 2;
	}

	H3_LCD0->TCON1_BASIC4 = LCDC_TCON1_TIMING_V_TOTAL(total) | LCDC_TCON1_TIMING_V_BP(bp);
	H3_LCD0->TCON1_BASIC5 = LCDC_X(mode->hsync_len.typ) | LCDC_Y(mode->vsync_len.typ);
}

void __attribute__((cold)) h3_lcdc_init(void) {
	H3_LCD0->GCTL = 0;
	H3_LCD0->GINT0 = 0;
	H3_LCD0->TCON0_DCLK &= (~LCDC_TCON0_DCLK_ENABLE);
	/* Set all io lines to tristate */
	H3_LCD0->TCON0_IO_TRI = 0xffffffff;
	H3_LCD0->TCON1_IO_TRI = 0xffffffff;
}
