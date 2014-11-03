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
//
#include <bcm2835.h>
//
#include <fb.h>

uint32_t fb_width, fb_height, fb_pitch, fb_addr, fb_size, fb_depth;

int fb_init()
{
	framebuffer_t volatile frame __attribute__((aligned (16)));

	frame.width_p  = WIDTH;
	frame.height_p = HEIGHT;
	frame.width_v  = WIDTH;
	frame.height_v = HEIGHT;
	frame.pitch    = 0;
	frame.depth    = BPP;
	frame.x        = 0;
	frame.y        = 0;
	frame.address  = 0;
	frame.size     = 0;

	bcm2835_mailbox_write(BCM2835_MAILBOX_FB_CHANNEL,((uint32_t)&frame) + 0x40000000);
	bcm2835_mailbox_read(BCM2835_MAILBOX_FB_CHANNEL);

	if (!frame.address) {
		return FB_ERROR;
	}

	fb_width  = frame.width_p;
	fb_height = frame.height_p;
	fb_pitch  = frame.pitch;
	fb_depth  = frame.depth;
	fb_addr   = frame.address;
	fb_size   = frame.size;

	return 0;
}

