#if defined ORANGE_PI_ONE
/**
 * @file fb.c
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "h3_de2.h"

#if !defined(USE_UBOOT_HDMI)
# include <string.h>
# include "display_timing.h"
void h3_de2_init(struct display_timing *timing, uint32_t fbbase);
#endif

extern int uart0_printf(const char* fmt, ...);
#define printf uart0_printf

volatile uint32_t fb_width;
volatile uint32_t fb_height;
volatile uint32_t fb_pitch;
volatile uint32_t fb_addr;

int __attribute__((cold)) fb_init(void) {
#if !defined(USE_UBOOT_HDMI)
	struct display_timing default_timing;
	memset(&default_timing, 0, sizeof(struct display_timing));

	default_timing.hdmi_monitor = false;
	default_timing.pixelclock.typ = 32000000;
	default_timing.hactive.typ = 800;
	default_timing.hback_porch.typ = 40;
	default_timing.hfront_porch.typ = 40;
	default_timing.hsync_len.typ = 48;
	default_timing.vactive.typ = 480;
	default_timing.vback_porch.typ = 29;
	default_timing.vfront_porch.typ = 13;
	default_timing.vsync_len.typ = 3;
	default_timing.flags = (DISPLAY_FLAGS_HSYNC_LOW | DISPLAY_FLAGS_VSYNC_LOW);

	h3_de2_init(&default_timing, FB_ADDRESS);
#endif
	const uint32_t size = H3_DE2_MUX0_UI->CFG[0].SIZE;
	fb_width = (size & 0xFFFF) + 1;
	fb_height = (size >> 16) + 1;
	fb_pitch = H3_DE2_MUX0_UI->CFG[0].PITCH;
	fb_addr = H3_DE2_MUX0_UI->CFG[0].TOP_LADDR;

	if (fb_addr == 0) {
		printf("fb_addr == 0\n");
		fb_addr = FB_ADDRESS;
	}

	return FB_OK;
}
#else
 typedef int ISO_C_forbids_an_empty_translation_unit;
#endif
