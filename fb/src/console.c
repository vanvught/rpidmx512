/**
 * @file console.c
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include "bcm2835_mailbox.h"
#include "console.h"

extern unsigned char FONT[];

#define CHAR_W				8						///<
#define CHAR_H				12						///<
#define WIDTH				480						///< Requested width of physical display
#define HEIGHT				320						///< Requested height of physical display
#define BYTES_PER_PIXEL		2						///< bytes per pixel for requested depth (BPP)
#define BPP					(BYTES_PER_PIXEL << 3)	///< Requested depth (bits per pixel)

static int cur_x = 0;						///<
static int cur_y = 0;						///<
static uint16_t cur_fore = CONSOLE_WHITE;	///<
static uint16_t cur_back = CONSOLE_BLACK;	///<
static uint32_t fb_width;					///< Width of physical display
static uint32_t fb_height;					///< Height of physical display
static uint32_t fb_pitch;					///< Number of bytes between each row of the frame buffer
static uint32_t fb_addr;					///< Address of buffer allocated by VC
static uint32_t fb_size;					///< Size of buffer allocated by VC
static uint32_t fb_depth;					///< Depth (bits per pixel)

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
int console_init() {
	uint32_t mb_addr = 0x40007000;		// 0x7000 in L2 cache coherent mode
	volatile framebuffer_t *frame = (framebuffer_t *) mb_addr;

	frame->width_p = (uint32_t) WIDTH;
	frame->height_p = (uint32_t) HEIGHT;
	frame->width_v = (uint32_t) WIDTH;
	frame->height_v = (uint32_t) HEIGHT;
	frame->pitch = (uint32_t) 0;
	frame->depth = (uint32_t) BPP;
	frame->x = (uint32_t) 0;
	frame->y = (uint32_t) 0;
	frame->address = (uint32_t) 0;
	frame->size = (uint32_t) 0;

	bcm2835_mailbox_write(BCM2835_MAILBOX_FB_CHANNEL, mb_addr);
	(void) bcm2835_mailbox_read(BCM2835_MAILBOX_FB_CHANNEL);

	if (frame->address == 0) {
		return CONSOLE_ERROR;
	}

	fb_width  = frame->width_p;
	fb_height = frame->height_p;
	fb_pitch  = frame->pitch;
	fb_depth  = frame->depth;
	fb_addr   = frame->address;
	fb_size   = frame->size;

	return CONSOLE_OK;
}

/**
 *
 */
inline static void newline() {
	uint8_t *block = NULL;

	cur_y++;
	cur_x = 0;

	if (cur_y == HEIGHT / CHAR_H) {
		/* Pointer to row = 0 */
		uint8_t *to = (uint8_t *) (fb_addr);
		/* Pointer to row = 1 */
		uint8_t *from = to + (CHAR_H * fb_pitch);
		/* Copy block from {row = 1, rows} to {row = 0, rows - 1} */
		memmove(to, from, (HEIGHT - CHAR_H) * fb_pitch);

		block = to + ((HEIGHT - CHAR_H) * fb_pitch);
		/* Clear last row */
		memset(block, (int) cur_back, (size_t) (CHAR_H * fb_pitch));

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
	volatile uint16_t *address = (volatile uint16_t *)(fb_addr + (x * BYTES_PER_PIXEL) + (y * WIDTH * BYTES_PER_PIXEL));
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
inline static void draw_char(const int c, const int x, int y, const uint16_t fore, const uint16_t back) {
	int i, j;
	uint8_t line;
	int index = c * (int) CHAR_H;

	for (i = 0; i < CHAR_H; i++) {
		line = (uint8_t) FONT[index++];
		for (j = 0; j < CHAR_W; j++) {
			if ((line & 0x1) != 0) {
				draw_pixel(x + j, y, fore);
			} else {
				draw_pixel(x + j, y, back);
			}
			line >>= 1;
		}
		y++;
	}
}

/**
 * Prints character ch with the specified color at position (col, row).
 *
 * @param ch The character to display.
 * @param x The column in which to display the character.
 * @param y The row in which to display the character.
 * @param fore The foreground color to use to display the character.
 * @param back he background color to use to display the character.
 * @return
 */
int console_draw_char(const int ch, const int x, const int y, const uint16_t fore, const uint16_t back) {
	draw_char(ch, x * CHAR_W, y * CHAR_H, fore, back);
	return (int)ch;
}

/**
 * Prints character ch at the current location of the cursor.
 *
 *  If the character is a newline ('\n'), the cursor is
 *  moved to the next line (scrolling if necessary). If
 *  the character is a carriage return ('\r'), the cursor
 *  is immediately reset to the beginning of the current
 *  line, causing any future output to overwrite any existing
 *  output on the line.
 *
 * @param ch The character to print.
 * @return
 */
int console_putc(const int ch) {
	if (ch == (int)'\n') {
		newline();
	} else if (ch == (int)'\r') {
		cur_x = 0;
	} else if (ch == (int)'\t') {
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
 * Clears the entire console.
 */
void console_clear() {
	memset((uint16_t *) fb_addr, (int)cur_back, (size_t)fb_size);

	cur_x = 0;
	cur_y = 0;
}

/**
 * Sets the position of the cursor to the position (col, row).
 *
 * @param x The new col for the cursor.
 * @param y The new row for the cursor.
 */
void console_set_cursor(const int x, const int y) {
	if (x > WIDTH / CHAR_W)
		cur_x = 0;
	else
		cur_x = x;

	if (y > HEIGHT / CHAR_H)
		cur_y = 0;
	else
		cur_y = y;
}

/**
 * Changes the foreground color of future characters printed on the console.
 *
 * @param fore The new color code.
 */
void console_set_fg_color(const uint16_t fore)
{
	cur_fore = fore;
}

/**
 * Changes the background color of future characters printed on the console.
 *
 * @param back The new color code.
 */
void console_set_bg_color(const uint16_t back)
{
	cur_back = back;
}


void console_clear_line(const int y) {
	volatile uint16_t *address = NULL;

	if (y > HEIGHT / CHAR_H)
		cur_y = 0;
	else
		cur_y = y;

	cur_x = 0;

	address = (volatile uint16_t *)(fb_addr + (y * CHAR_H * WIDTH) * BYTES_PER_PIXEL);
	memset((uint16_t *)address, (int)cur_back, (size_t)(CHAR_H * WIDTH * BYTES_PER_PIXEL));
}
