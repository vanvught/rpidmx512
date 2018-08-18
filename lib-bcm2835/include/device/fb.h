/**
 * @file fb.h
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

#ifndef DEVICE_FB_H_
#define DEVICE_FB_H_

extern unsigned char FONT[] __attribute__((aligned(4)));

#define FB_CHAR_W			8
#define FB_CHAR_H			16
#define FB_WIDTH			800							///< Requested width of physical display
#define FB_HEIGHT			480							///< Requested height of physical display
#define FB_BYTES_PER_PIXEL	2							///< bytes per pixel for requested depth (FB_BPP)
#define FB_BPP				(FB_BYTES_PER_PIXEL << 3)	///< Requested depth (bits per pixel)
#define FB_PITCH			(FB_WIDTH * FB_BYTES_PER_PIXEL)

#define FB_OK	0

#define FB_FAIL_GET_RESOLUTION			-1	///<
#define FB_FAIL_INVALID_RESOLUTION		-2	///<
#define FB_FAIL_SETUP_FB				-3	///<
#define FB_FAIL_INVALID_TAGS			-4	///<
#define FB_FAIL_INVALID_TAG_RESPONSE	-5	///<
#define FB_FAIL_INVALID_TAG_DATA		-6	///<
#define FB_FAIL_INVALID_PITCH_RESPONSE	-7	///<
#define FB_FAIL_INVALID_PITCH_DATA		-8	///<

#ifdef __cplusplus
extern "C" {
#endif

extern int fb_init(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_FB_H_ */
