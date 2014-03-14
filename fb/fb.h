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

#ifndef FB_H
#define FB_H

#define WIDTH				480
#define HEIGHT				320
#define BYTES_PER_PIXEL		2
#define BPP					(BYTES_PER_PIXEL << 3)

#define FB_ERROR	-1

typedef struct framebuffer {
	uint32_t width_p;	/* requested width of physical display */
	uint32_t height_p;	/* requested height of physical display */
	uint32_t width_v;	/* requested width of virtual display */
	uint32_t height_v;	/* requested height of virtual display */
	uint32_t pitch;		/* zero upon request; in response: # bytes between rows */
	uint32_t depth;		/* requested depth in bits per pixel */
	uint32_t x;			/* requested x offset of virtual framebuffer */
	uint32_t y;			/* requested y offset of virtual framebuffer */
	uint32_t address;	/* framebuffer address. Zero upon request; failure if zero upon response. */
	uint32_t size;		/* framebuffer size. Zero upon request. */
} framebuffer_t;

extern uint32_t width, height, pitch, fb_addr, fb_size, depth;

int fb_init();

#endif

