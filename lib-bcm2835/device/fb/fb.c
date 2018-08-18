/**
 * @file fb.c
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

#include "device/fb.h"

#include "arm/synchronize.h"
#include "arm/arm.h"

#include "bcm2835.h"
#include "bcm2835_mailbox.h"
#include "bcm2835_vc.h"

volatile uint32_t fb_addr;	///< Address of buffer allocated by VC
volatile uint32_t fb_size;	///< Size of buffer allocated by VC
volatile uint32_t fb_depth;	///< Depth (bits per pixel)

int fb_init(void) {
	uint32_t *mailbuffer = (uint32_t *)MEM_COHERENT_REGION;

	mailbuffer[0] = 22 * 4;
	mailbuffer[1] = 0;

	mailbuffer[2] = BCM2835_VC_TAG_SET_PHYS_WH;
	mailbuffer[3] = 8;
	mailbuffer[4] = 8;
	mailbuffer[5] = FB_WIDTH;
	mailbuffer[6] = FB_HEIGHT;

	mailbuffer[7] = BCM2835_VC_TAG_SET_VIRT_WH;
	mailbuffer[8] = 8;
	mailbuffer[9] = 8;
	mailbuffer[10] = FB_WIDTH;
	mailbuffer[11] = FB_HEIGHT;

	mailbuffer[12] = BCM2835_VC_TAG_SET_DEPTH;
	mailbuffer[13] = 4;
	mailbuffer[14] = 4;
	mailbuffer[15] = (uint32_t) FB_BPP;

	mailbuffer[16] = BCM2835_VC_TAG_ALLOCATE_BUFFER;
	mailbuffer[17] = 8;
	mailbuffer[18] = 4;
	mailbuffer[19] = 16;
	mailbuffer[20] = 0;

	mailbuffer[21] = 0;

	(void) bcm2835_mailbox_write_read(BCM2835_MAILBOX_PROP_CHANNEL, GPU_MEM_BASE + (uint32_t) mailbuffer);

	if (mailbuffer[1] != BCM2835_MAILBOX_SUCCESS) {
		return FB_FAIL_SETUP_FB;
	}

	fb_addr = mailbuffer[19] & 0x3FFFFFFF;
	fb_size = mailbuffer[20];

	if ((fb_addr == 0) || (fb_size == 0)) {
		return FB_FAIL_INVALID_TAG_DATA;
	}

	fb_depth = mailbuffer[15];

	return FB_OK;
}
