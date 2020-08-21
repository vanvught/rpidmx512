/**
 * @file fb.h
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

#ifndef DEVICE_FB_H_
#define DEVICE_FB_H_

#if !defined ORANGE_PI_ONE
# error Support for Orange Pi One only
#endif

extern volatile uint32_t fb_width;
extern volatile uint32_t fb_height;
extern volatile uint32_t fb_pitch;
extern volatile uint32_t fb_addr;

#define FB_WIDTH			800
#define FB_HEIGHT			480
#define FB_BYTES_PER_PIXEL	4
#define FB_BPP				(FB_BYTES_PER_PIXEL << 3)
#define FB_PITCH			(FB_WIDTH * FB_BYTES_PER_PIXEL)
#define FB_ADDRESS			0x5E000000

#define FB_OK	0

#ifdef __cplusplus
extern "C" {
#endif

extern int fb_init(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_FB_H_ */
