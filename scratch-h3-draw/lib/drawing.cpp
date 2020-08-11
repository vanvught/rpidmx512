#if defined ORANGE_PI_ONE
/**
 * @file drawing.cpp
 *
 */

/*
 * Drawing functions for the FB console.
 * 
 * Primative shape drawing functions 
 * Heavily inspired by javidx9's olcPixelGameEngine (https://github.com/OneLoneCoder/olcPixelGameEngine)
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
 * 
 */


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "drawing.h"

#include "device/fb.h"


#define SWAP(x,y) do \
    { unsigned char swap_temp[sizeof(x) == sizeof(y) ? static_cast<signed>(sizeof(x)) : -1]; \
     memcpy(swap_temp,&y,sizeof(x)); \
     memcpy(&y,&x,       sizeof(x)); \
     memcpy(&x,swap_temp,sizeof(x)); \
    } while(0)


void Drawing::pixel(int32_t x, int32_t y, uint32_t c) {
        console_putpixel(static_cast<uint32_t>(x), static_cast<uint32_t>(y), c);
}

void Drawing::line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t p, uint32_t pattern) {
    int32_t x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
    dx = x2 - x1; dy = y2 - y1;

    auto rol = [&](void) { 
        pattern = (pattern << 1) | (pattern >> 31); 
        return pattern & 1; 
    };

    // Line is vertical
    if (dx == 0) {
        if (y2 < y1) SWAP(y1, y2);
        for (y = y1; y <= y2; y++) if (rol()) pixel(x1, y, p);
        return;
    }
    // Line is horizontal
    if (dy == 0) {
        if (x2 < x1) SWAP(x1, x2);
        for (x = x1; x <= x2; x++) if (rol()) pixel(x, y1, p);
        return;
    }

    dx1 = abs(dx); 
    dy1 = abs(dy);
    px = 2 * dy1 - dx1;
    py = 2 * dx1 - dy1;
    if (dy1 <= dx1) {
        if (dx >= 0) {
            x = x1; y = y1; xe = x2;
        } else {
            x = x2; y = y2; xe = x1;
        }			
        if (rol()) pixel(x, y, p);
        for (i = 0; x < xe; i++) {
            x = x + 1;
            if (px < 0)
                px = px + 2 * dy1;
            else {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) y = y + 1; else y = y - 1;
                px = px + 2 * (dy1 - dx1);
            }
            if (rol()) pixel(x, y, p);
        }
    }
    else {
        if (dy >= 0) {
            x = x1; 
            y = y1; 
            ye = y2;
        }
        else {
            x = x2; 
            y = y2; 
            ye = y1;
        }
        if (rol()) pixel(x, y, p);
        for (i = 0; y < ye; i++) {
            y = y + 1;
            if (py <= 0)
                py = py + 2 * dx1;
            else {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) x = x + 1; else x = x - 1;
                py = py + 2 * (dx1 - dy1);
            }
            if (rol()) pixel(x, y, p);
        }
    }
}

void Drawing::rect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t p) {
    line(x, y, x + w, y, p, 0);
    line(x + w, y, x + w, y + h, p, 0);
    line(x + w, y + h, x, y + h, p, 0);
    line(x, y + h, x, y, p, 0);
}

void Drawing::circle(int32_t x, int32_t y, int32_t radius, uint32_t p, uint8_t mask) { 

    if (radius < 0 || x < -radius || y < -radius || x - fbW() > radius || y - fbH() > radius)
        return;

    if (radius > 0) {
        int x0 = 0;
        int y0 = radius;
        int d = 3 - 2 * radius;

        // only formulate 1/8 of circle
        while (y0 >= x0) { 
            // Draw even octants
            if (mask & 0x01) pixel(x + x0, y - y0, p);// Q6 - upper right right
            if (mask & 0x04) pixel(x + y0, y + x0, p);// Q4 - lower lower right
            if (mask & 0x10) pixel(x - x0, y + y0, p);// Q2 - lower left left
            if (mask & 0x40) pixel(x - y0, y - x0, p);// Q0 - upper upper left
            if (x0 != 0 && x0 != y0) {
                if (mask & 0x02) pixel(x + y0, y - x0, p);// Q7 - upper upper right
                if (mask & 0x08) pixel(x + x0, y + y0, p);// Q5 - lower right right
                if (mask & 0x20) pixel(x - y0, y + x0, p);// Q3 - lower lower left
                if (mask & 0x80) pixel(x - x0, y - y0, p);// Q1 - upper left left
            }
            if (d < 0)
                d += 4 * x0++ + 6;
            else
                d += 4 * (x0++ - y0--) + 10;
        }
    }
    else
        pixel(x, y, p);
}

void Drawing::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t p) {

    int32_t x2 = x + w;
    int32_t y2 = y + h;

    if (x < 0) x = 0;
    if (x >= fbW()) x = fbW();
    if (y < 0) y = 0;
    if (y >= fbH()) y = fbH();
    if (x2 < 0) x2 = 0;
    if (x2 >= fbW()) x2 = fbW();
    if (y2 < 0) y2 = 0;
    if (y2 >= fbH()) y2 = fbH();

    for (int i = x; i < x2; i++)
        for (int j = y; j < y2; j++)
            pixel(i, j, p);
}

void Drawing::fillCircle(int32_t x, int32_t y, int32_t radius, uint32_t p) { 

    if (radius < 0 || x < -radius || y < -radius || x - fbW() > radius || y - fbH() > radius)
        return;

    if (radius > 0) {
        int x0 = 0;
        int y0 = radius;
        int d = 3 - 2 * radius;

        auto drawline = [&](int sx, int ex, int y) {
            for (int x = sx; x <= ex; x++)
                pixel(x, y, p);
        };

        while (y0 >= x0) {
            drawline(x - y0, x + y0, y - x0);
            if (x0 > 0)	drawline(x - y0, x + y0, y + x0);
            if (d < 0)
                d += 4 * x0++ + 6;
            else {
                if (x0 != y0) {
                    drawline(x - x0, x + x0, y - y0);
                    drawline(x - x0, x + x0, y + y0);
                }
                d += 4 * (x0++ - y0--) + 10;
            }
        }
    }
    else
        pixel(x, y, p);
}


void Drawing::triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t p) {
    line(x1, y1, x2, y2, p, 0);
    line(x2, y2, x3, y3, p, 0);
    line(x3, y3, x1, y1, p, 0);
}


// https://www.avrfreaks.net/sites/default/files/triangles.c
void Drawing::fillTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t p) {
    
    auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) pixel(i, ny, p); };
    
    int t1x, t2x, y, minx, maxx, t1xp, t2xp;
    bool changed1 = false;
    bool changed2 = false;
    int signx1, signx2, dx1, dy1, dx2, dy2;
    int e1, e2;
    // Sort vertices
    if (y1 > y2) { SWAP(y1, y2); SWAP(x1, x2); }
    if (y1 > y3) { SWAP(y1, y3); SWAP(x1, x3); }
    if (y2 > y3) { SWAP(y2, y3); SWAP(x2, x3); }

    t1x = t2x = x1; y = y1;   // Starting points
    dx1 = (x2 - x1);
    if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
    else signx1 = 1;
    dy1 = (y2 - y1);

    dx2 = (x3 - x1);
    if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
    else signx2 = 1;
    dy2 = (y3 - y1);

    if (dy1 > dx1) { SWAP(dx1, dy1); changed1 = true; }
    if (dy2 > dx2) { SWAP(dy2, dx2); changed2 = true; }

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
        SWAP(dy1, dx1);
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

#endif