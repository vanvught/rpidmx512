/**
 * @file drawing.h
 *
 */

/*
 * Drawing functions for the FB console.
 *
*/

/*  Copyright (C) 2020 by hippy mailto:dmxout@gmail.com
 *  Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
 * 
 */
#ifndef DRAWING_H_
#define DRAWING_H_

#if !defined ORANGE_PI_ONE
# error Support for Orange Pi One only
#endif

#include "stdint.h"

extern "C" {
	void console_putpixel(uint32_t x, uint32_t y, uint32_t color);
	extern volatile uint32_t fb_width;
	extern volatile uint32_t fb_height;
	extern volatile uint32_t fb_pitch;
}

class Drawing {
public:
    Drawing();
    
    void line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t p);
    void rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t p);
    void fillRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t p);    
    void circle(uint32_t xc, uint32_t yc, uint32_t radius, uint32_t p);
    void fillCircle(uint32_t x, uint32_t y, uint32_t radius, uint32_t p);
    void triangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, uint32_t p);
   // void fillTriangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, uint32_t p);

	static Drawing *Get() {
		return s_pThis;
	}

  
private:
    void pixel(uint32_t x, uint32_t y, uint32_t c) {
    	 console_putpixel(x, y, c);
    }

    void pixel(int32_t x, int32_t y, uint32_t c) {
    	 console_putpixel(static_cast<uint32_t>(x), static_cast<uint32_t>(y), c);
    }

	uint32_t fbW() {
		return fb_width;
	}

	uint32_t fbH() {
		return fb_height;
	}

    // static bool clip;           /* is clipping */
    // static uint32_t clipx1;		/* top-left coordinate of clip window */
	// static uint32_t clipy1;
	// static uint32_t clipx2;		/* bottom-right coordinate of clip window */
	// static uint32_t clipy2;

    static Drawing *s_pThis;
};

#endif
