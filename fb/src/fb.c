/**
 * @file fb.c
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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
#include "bcm2835_mailbox.h"
#include "fb.h"

uint32_t fb_width;	///< Width of physical display
uint32_t fb_height;	///< Height of physical display
uint32_t fb_pitch;	///< Number of bytes between each row of the frame buffer
uint32_t fb_addr;	///< Address of buffer allocated by VC
uint32_t fb_size;	///< Size of buffer allocated by VC
uint32_t fb_depth;	///< Depth (bits per pixel)

typedef struct framebuffer {
	uint32_t width_p;	///< Requested width of physical display
	uint32_t height_p;	///< Requested height of physical display
	uint32_t width_v;	///< Requested width of virtual display
	uint32_t height_v;	///< Requested height of virtual display
	uint32_t pitch;		///< Request: Set to zero, Response: Number of bytes between each row of the frame buffer
	uint32_t depth;		///< Requested depth (bits per pixel)
	uint32_t x;			///< Requested X offset of the virtual framebuffer
	uint32_t y;			///< Requested Y offset of the virtual framebuffer
	uint32_t address;	///< Framebuffer address. Request: Set to zero, Response: Address of buffer allocated by VC, or zero if request fails
	uint32_t size;		///< Framebuffer size. Request: Set to zero, Response: Size of buffer allocated by VC
} framebuffer_t;

/**
 * @ingroup FB
 *
 * @return
 */
int fb_init()
{
	uint32_t mb_addr = 0x40007000;		// 0x7000 in L2 cache coherent mode
	volatile framebuffer_t *frame  = (framebuffer_t *)mb_addr;

	frame->width_p  = WIDTH;
	frame->height_p = HEIGHT;
	frame->width_v  = WIDTH;
	frame->height_v = HEIGHT;
	frame->pitch    = 0;
	frame->depth    = BPP;
	frame->x        = 0;
	frame->y        = 0;
	frame->address  = 0;
	frame->size     = 0;

	bcm2835_mailbox_write(BCM2835_MAILBOX_FB_CHANNEL, mb_addr);
	bcm2835_mailbox_read(BCM2835_MAILBOX_FB_CHANNEL);

	if (!frame->address) {
		return FB_ERROR;
	}

	fb_width  = frame->width_p;
	fb_height = frame->height_p;
	fb_pitch  = frame->pitch;
	fb_depth  = frame->depth;
	fb_addr   = frame->address;
	fb_size   = frame->size;

	return FB_OK;
}

