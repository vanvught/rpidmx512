/**
 * @file console.c
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
#include <string.h>
#include "fb.h"
#include "console.h"

extern uint32_t fb_pitch, fb_addr, fb_size;

extern unsigned char FONT[];

#define CHAR_W		8
#define CHAR_H		12

#define DEF_FORE	0xFFFF
#define DEF_BACK	0x0000

static int cur_x = 0;
static int cur_y = 0;

static uint16_t cur_fore = DEF_FORE;
static uint16_t cur_back = DEF_BACK;

/**
 *
 */
inline static void newline() {
	cur_y++;
	cur_x = 0;

	if (cur_y == HEIGHT / CHAR_H) {
		/* Pointer to row = 0 */
		uint8_t *to = (uint8_t *)(fb_addr);
		/* Pointer to row = 1 */
		uint8_t *from = to + (CHAR_H * fb_pitch);
		/* Copy block from {row = 1, rows} to {row = 0, rows - 1} */
		memmove(to, from, (HEIGHT - CHAR_H) * fb_pitch);

		uint8_t *block = to + ((HEIGHT - CHAR_H) * fb_pitch);
		/* Clear last row */
		memset(block, cur_back, (CHAR_H * fb_pitch));

		cur_y--;
	}
}

/**
 *
 * @param x
 * @param y
 * @param color
 */
inline static void draw_pixel(const int x, const int y, const uint16_t color) {
	volatile uint16_t *address = (volatile uint16_t *)(fb_addr + (x + y * WIDTH) * BYTES_PER_PIXEL);
	*address = color;
}

/**
 *
 * @param c
 * @param x
 * @param y
 * @param fore
 * @param back
 */
inline static void draw_char(const char c, const int x, int y, const uint16_t fore, const uint16_t back) {
	int i, j;

	for (i = 0; i < CHAR_H; i++) {
		uint8_t line = FONT[c * CHAR_H + i];
		for (j = 0; j < CHAR_W; j++) {
			if (line & 0x1) {
				draw_pixel(x + j, y, fore);
			} else
				draw_pixel(x + j, y, back);
			line >>= 1;
		}
		y++;
	}
}

/**
 *
 * @param ch
 * @param x
 * @param y
 * @param fore
 * @param back
 * @return
 */
int console_draw_char(const char ch, const int x, const int y, const uint16_t fore, const uint16_t back) {
	draw_char(ch, x * CHAR_W, y * CHAR_H, fore, back);
	return ch;
}

/**
 *
 * @param ch
 * @return
 */
int console_putc(const int ch) {
	if (ch == '\n') {
		newline();
	} else if (ch == '\t') {
		cur_x += 4;
	} else {
		draw_char(ch, cur_x * CHAR_W, cur_y * CHAR_H, cur_fore, cur_back);
		cur_x++;
		if (cur_x == WIDTH / CHAR_W) {
			newline();
		}
	}
	return ch;
}

/**
 *
 */
void console_clear()
{
	memset((uint16_t *)fb_addr, cur_back, fb_size);

	cur_x = 0;
	cur_y = 0;
}

/**
 *
 * @param x
 * @param y
 */
void console_set_cursor(const int x, const int y) {
	if (x > WIDTH / CHAR_W)
		cur_x = WIDTH / CHAR_W;
	else
		cur_x = x;

	if (y > HEIGHT / CHAR_H)
		cur_y = HEIGHT / CHAR_H;
	else
		cur_y = y;
}
