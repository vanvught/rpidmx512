//#define ORANGE_PI_ONE
#if defined ORANGE_PI_ONE
/**
 * @file fb.c
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "device/fb.h"

volatile uint32_t fb_width;
volatile uint32_t fb_height;
volatile uint32_t fb_pitch;
volatile uint32_t fb_addr;

/*
 * u-boot/arch/arm/include/asm/arch-sunxi/display2.h
 */

struct de_ui {
	struct {
		uint32_t attr;
		uint32_t size;
		uint32_t coord;
		uint32_t pitch;
		uint32_t top_laddr;
		uint32_t bot_laddr;
		uint32_t fcolor;
		uint32_t dum;
	} cfg[4];
	uint32_t top_haddr;
	uint32_t bot_haddr;
	uint32_t ovl_size;
};

/*
 * u-boot/arch/arm/include/asm/arch-sunxi/display.h
 */

#define SUNXI_DE2_BASE				0x01000000
#define SUNXI_DE2_MUX0_BASE			(SUNXI_DE2_BASE + 0x100000)

#define SUNXI_DE2_MUX_CHAN_REGS		0x02000
#define SUNXI_DE2_MUX_CHAN_SZ		0x1000

int fb_init(void) {
	const uint32_t de_mux_base = SUNXI_DE2_MUX0_BASE;
	struct de_ui * const de_ui_regs = (struct de_ui *) (de_mux_base + SUNXI_DE2_MUX_CHAN_REGS + SUNXI_DE2_MUX_CHAN_SZ * 1);

	// This is working with u-boot-2018.09
	// fb_addr = 0x5E000000;
	const uint32_t size = de_ui_regs->cfg[0].size;
	fb_width = (size & 0xFFFF) + 1;
	fb_height = (size >> 16) + 1;
	fb_pitch = de_ui_regs->cfg[0].pitch;
	fb_addr = de_ui_regs->cfg[0].top_laddr;

	return FB_OK;
}
#endif
