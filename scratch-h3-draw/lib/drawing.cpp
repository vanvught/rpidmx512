#if defined (ORANGE_PI_ONE)
/**
 * @file drawing.cpp
 *
 */

/*
 * Drawing functions for the FB console.
 *
*/

/*  Copyright (C) 2019-2020 by hippy mailto:dmxout@gmail.com
 *  Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "drawing.h"

#include "h3/console_fb.h"  // console_putpixel()
#include "device/fb.h"


Drawing *Drawing::s_pThis = 0;

void Drawing::pixel(uint32_t x, uint32_t y, uint32_t c) {
        console_putpixel(x, y, c);
}

void Drawing::pixel(int32_t x, int32_t y, uint32_t c) {
        console_putpixel(static_cast<uint32_t>(x), static_cast<uint32_t>(y), c);
}

uint32_t Drawing::get_pixel(int32_t x, int32_t y) {
        return console_getpixel(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
}


void Drawing::line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t p) { 

    int32_t dx = abs(static_cast<int32_t>(x2 - x1));
    uint32_t sx = x1 < x2 ? 1U : -1U;
    int32_t dy = abs(static_cast<int32_t>(y2 - y1));
    uint32_t sy = y1 < y2 ? 1U : -1U;
    int32_t err = (dx > dy ? dx : -dy) / 2;

    while (pixel(x1, y1, p), x1 != x2 || y1 != y2) {
        int32_t e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 <  dy) { err += dx; y1 += sy; }
    }
}

void Drawing::rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t p) {
    line(x, y, x + w, y, p);
    line(x + w, y, x + w, y + h, p);
    line(x + w, y + h, x, y + h, p);
    line(x, y + h, x, y, p);
}

void Drawing::fillRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t p) {

    uint32_t x2 = x + w;
    uint32_t y2 = y + h;

    if (x >= fbW()) { x = fbW(); }
    if (y >= fbH()) { y = fbH(); }
    if (x2 >= fbW()) { x2 = fbW(); }
    if (y2 >= fbH()) { y2 = fbH(); }
    for (unsigned int i = x; i < x2; i++) {
        for (unsigned int j = y; j < y2; j++) {
            pixel(i, j, p);
        }
    }     
}

// Function for circle-generation using Bresenham's algorithm 
void Drawing::circle(uint32_t xc, uint32_t yc, uint32_t radius, uint32_t p) { 
    
    uint32_t x = 0, y = radius; 
    int32_t d = 3 - 2 * static_cast<int>(radius); 
   
    auto drawcirc = [&](uint32_t xc, uint32_t yc, uint32_t p) {
        pixel(xc+x, yc+y, p); 
        pixel(xc-x, yc+y, p); 
        pixel(xc+x, yc-y, p); 
        pixel(xc-x, yc-y, p); 
        pixel(xc+y, yc+x, p); 
        pixel(xc-y, yc+x, p); 
        pixel(xc+y, yc-x, p); 
        pixel(xc-y, yc-x, p);   
    };

    drawcirc(xc, yc, p);
    while (y >= x) { 
        x++; 
        if (d > 0) { 
            y--;  
            d = d + 4 * static_cast<int32_t>(x - y) + 10; 
        } else {
            d = d + 4 * static_cast<int32_t>(x) + 6; 
        }
        drawcirc(xc, yc, p);
    } 
} 

void Drawing::fillCircle(uint32_t x, uint32_t y, uint32_t radius, uint32_t p) {

    int ox = static_cast<int>(x);
    int oy = static_cast<int>(y);
    int rad = static_cast<int>(radius);

    for(int y=-rad; y<=rad; y++){ 
        for(int x=-rad; x<=rad; x++) {
            if(x*x+y*y <= rad*rad) {
                pixel(ox+x, oy+y, p);
            }
        }
    }
}

void Drawing::triangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, uint32_t p) {

    line(x1, y1, x2, y2, p);
    line(x2, y2, x3, y3, p);
    line(x3, y3, x1, y1, p);
}


/* 

template<typename T> void swap(T& t1, T& t2) {
    T tmp(t1);
    t1=t2;
    t2=tmp;
}

// https://www.avrfreaks.net/sites/default/files/triangles.c
void Drawing::fillTriangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, uint32_t p) {
    
    auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) pixel(i, ny, p); };
    
    int t1x, t2x, y, minx, maxx, t1xp, t2xp;
    bool changed1 = false;
    bool changed2 = false;
    int signx1, signx2, dx1, dy1, dx2, dy2;
    int e1, e2;
    // Sort vertices
    if (y1 > y2) { swap(y1, y2); swap(x1, x2); }
    if (y1 > y3) { swap(y1, y3); swap(x1, x3); }
    if (y2 > y3) { swap(y2, y3); swap(x2, x3); }

    t1x = t2x = x1; y = y1;   // Starting points
    dx1 = (x2 - x1);
    if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
    else signx1 = 1;
    dy1 = (y2 - y1);

    dx2 = (x3 - x1);
    if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
    else signx2 = 1;
    dy2 = (y3 - y1);

    if (dy1 > dx1) { swap(dx1, dy1); changed1 = true; }
    if (dy2 > dx2) { swap(dy2, dx2); changed2 = true; }

    e2 = (dx2 >> 1);
    // Flat top, just process the second half
    if (y1 == y2) goto next;
    e1 = (dx1 >> 1);

    for (int i = 0; i < dx1;) {
        t1xp = 0; t2xp = 0;
        if (t1x < t2x) { minx = t1x; maxx = t2x; }
        else { minx = t2x; maxx = t1x; }
        // process first line until y value is about to change
        while (i < dx1) {
            i++;
            e1 += dy1;
            while (e1 >= dx1) {
                e1 -= dx1;
                if (changed1) t1xp = signx1;//t1x += signx1;
                else          goto next1;
            }
            if (changed1) break;
            else t1x += signx1;
        }
        // Move line
    next1:
        // process second line until y value is about to change
        while (1) {
            e2 += dy2;
            while (e2 >= dx2) {
                e2 -= dx2;
                if (changed2) t2xp = signx2;//t2x += signx2;
                else          goto next2;
            }
            if (changed2)     break;
            else              t2x += signx2;
        }
    next2:
        if (minx > t1x) minx = t1x;
        if (minx > t2x) minx = t2x;
        if (maxx < t1x) maxx = t1x;
        if (maxx < t2x) maxx = t2x;
        drawline(minx, maxx, y);    // Draw line from min to max points found on the y
                                    // Now increase y
        if (!changed1) t1x += signx1;
        t1x += t1xp;
        if (!changed2) t2x += signx2;
        t2x += t2xp;
        y += 1;
        if (y == y2) break;

    }
next:
    // Second half
    dx1 = (x3 - x2); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
    else signx1 = 1;
    dy1 = (y3 - y2);
    t1x = x2;

    if (dy1 > dx1) {   // swap values
        swap(dy1, dx1);
        changed1 = true;
    }
    else changed1 = false;

    e1 = (dx1 >> 1);

    for (int i = 0; i <= dx1; i++) {
        t1xp = 0; t2xp = 0;
        if (t1x < t2x) { minx = t1x; maxx = t2x; }
        else { minx = t2x; maxx = t1x; }
        // process first line until y value is about to change
        while (i < dx1) {
            e1 += dy1;
            while (e1 >= dx1) {
                e1 -= dx1;
                if (changed1) { t1xp = signx1; break; }//t1x += signx1;
                else          goto next3;
            }
            if (changed1) break;
            else   	   	  t1x += signx1;
            if (i < dx1) i++;
        }
    next3:
        // process second line until y value is about to change
        while (t2x != x3) {
            e2 += dy2;
            while (e2 >= dx2) {
                e2 -= dx2;
                if (changed2) t2xp = signx2;
                else          goto next4;
            }
            if (changed2)     break;
            else              t2x += signx2;
        }
    next4:

        if (minx > t1x) minx = t1x;
        if (minx > t2x) minx = t2x;
        if (maxx < t1x) maxx = t1x;
        if (maxx < t2x) maxx = t2x;
        drawline(minx, maxx, y);
        if (!changed1) t1x += signx1;
        t1x += t1xp;
        if (!changed2) t2x += signx2;
        t2x += t2xp;
        y += 1;
        if (y > y3) return;
    }
}
 */


// #define outside(x, y) (x < __clipx1 || x > __clipx2 || y < __clipy1 || y > __clipy2)
// #define x_outside(x) (x < __clipx1 || x > __clipx2)
// #define y_outside(y) (y < __clipy1 || y > __clipy2)
// #define clipxleft(x) if (x < __clipx1) x = __clipx1;
// #define clipxright(x) if (x > __clipx2) x = __clipx2;
// #define clipytop(y) if (y < __clipy1) y = __clipy1;
// #define clipybottom(y) if (y > __clipy2) y = __clipy2;

/* 
#define ADJUSTBITMAPBOX() \
	nw = w; nh = h; nx = x; ny = y;				\
	if (nx + nw < __clipx1 || nx > __clipx2)		\
		return;						\
	if (ny + nh < __clipy1 || ny > __clipy2)		\
		return;						\
	if (nx < __clipx1) {			\
		nw += nx - __clipx1;				\
		nx = __clipx1;					\
	}							\
	if (ny < __clipy1) {			\
		nh += ny - __clipy1;				\
		ny = __clipy1;					\
	}							\
	if (nx + nw > __clipx2)			\
		nw = __clipx2 - nx + 1;				\
	if (ny + nh > __clipy2)			\
		nh = __clipy2 - ny + 1;				\


 */
// void gl_setclippingwindow(int x1, int y1, int x2, int y2)
// {
//     __clip = 1;
//     __clipx1 = x1;
//     __clipy1 = y1;
//     __clipx2 = x2;
//     __clipy2 = y2;
// }

// void gl_enableclipping()
// {
//     __clip = 1;
//     __clipx1 = 0;
//     __clipy1 = 0;
//     __clipx2 = WIDTH - 1;
//     __clipy2 = HEIGHT - 1;
// }

// void gl_disableclipping()
// {
//     __clip = 0;
// }









#endif

