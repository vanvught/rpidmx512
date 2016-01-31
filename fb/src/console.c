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

#include "bcm2835.h"
#include "bcm2835_mailbox.h"
#include "bcm2835_vc.h"
#include "console.h"

extern unsigned char FONT[];

#define CHAR_W				8							///<
#define CHAR_H				16							///<
#define WIDTH				800							///< Requested width of physical display
#define HEIGHT				480							///< Requested height of physical display
#define BYTES_PER_PIXEL		2							///< bytes per pixel for requested depth (BPP)
#define BPP					(BYTES_PER_PIXEL << 3)		///< Requested depth (bits per pixel)
#define PITCH				(WIDTH * BYTES_PER_PIXEL)	///<

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

const uint32_t console_get_address() {
	return fb_addr;
}

const uint32_t console_get_pitch() {
	return fb_pitch;
}

const uint32_t console_get_width() {
	return fb_width;
}

const uint32_t console_get_height() {
	return fb_height;
}

const uint32_t console_get_size() {
	return fb_size;
}

const uint32_t console_get_depth() {
	return fb_depth;
}

/**
 *
 */
inline static void newline() {
	uint32_t i;
	uint16_t *address;

	cur_y++;
	cur_x = 0;

	if (cur_y == HEIGHT / CHAR_H) {
		/* Pointer to row = 0 */
		uint16_t *to = (uint16_t *) (fb_addr);
		/* Pointer to row = 1 */
		uint16_t *from = to + (CHAR_H * WIDTH);
		/* Copy block from {row = 1, rows} to {row = 0, rows - 1} */
		i = (HEIGHT - CHAR_H) * WIDTH;
		while (i-- != 0 ) {
			*to++ = *from++;
		}
		/* Clear last row */
		address = (uint16_t *)(fb_addr) + ((HEIGHT - CHAR_H) * WIDTH);
		for (i = 0 ; i < (CHAR_H * WIDTH) ; i++) {
			*address++ =  cur_back;
		}

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
	unsigned char *p = FONT + (c * (int) CHAR_H);

	for (i = 0; i < CHAR_H; i++) {
		line = *p++;
		for (j = x; j < (CHAR_W + x); j++) {
			if ((line & 0x1) != 0) {
				draw_pixel(j, y, fore);
			} else {
				draw_pixel(j, y, back);
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
 *
 * @param s
 * @return
 */
void console_puts(const char *s) {
	char c;

	while ((c = *s++) != (char) 0) {
		(void) console_putc((int) c);
	}
}

/**
 *
 * @param s
 * @param n
 */
void console_putsn(const char *s, int n) {
	char c;

	while (((c = *s++) != (char) 0) && (n-- != 0)) {
		(void) console_putc((int) c);
	}
}

#define TO_HEX(i)	((i) < 10) ? (uint8_t)'0' + (i) : (uint8_t)'A' + ((i) - (uint8_t)10)

/**
 *
 * @param data
 */
void console_puthex(const uint8_t data) {
	(void) console_putc(TO_HEX(((data & 0xF0) >> 4)));
	(void) console_putc(TO_HEX(data & 0x0F));
}

void console_puthex_fg_bg(const uint8_t data, const uint16_t fore, const uint16_t back) {
	uint16_t fore_current = cur_fore;
	uint16_t back_current = cur_back;

	cur_fore = fore;
	cur_back = back;

	(void) console_putc(TO_HEX(((data & 0xF0) >> 4)));
	(void) console_putc(TO_HEX(data & 0x0F));

	cur_fore = fore_current;
	cur_back = back_current;
}

/**
 *
 */
void console_newline(){
	newline();
}

/**
 * Clears the entire console.
 */
void console_clear() {
	uint16_t *address = (uint16_t *)(fb_addr);
	uint32_t i;

	for (i = 0; i < (HEIGHT * WIDTH); i++) {
		*address++ = cur_back;
	}

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
void console_set_fg_color(const uint16_t fore) {
	cur_fore = fore;
}

/**
 * Changes the background color of future characters printed on the console.
 *
 * @param back The new color code.
 */
void console_set_bg_color(const uint16_t back) {
	cur_back = back;
}

/**
 *
 * @param line
 */
void console_clear_line(const int line) {
	uint16_t *address;
	uint32_t i;

	if (line > HEIGHT / CHAR_H) {
		cur_y = 0;
	} else {
		cur_y = line;
	}

	cur_x = 0;

	address = (uint16_t *)(fb_addr) + (line * CHAR_H * WIDTH);

	for (i = 0; i < (CHAR_H * WIDTH); i++) {
		*address++ = cur_back;
	}
}

/**
 *
 * @return
 */
int console_init() {
	uint32_t mailbuffer[64] __attribute__((aligned(16)));

	mailbuffer[0] = 8 * 4;
	mailbuffer[1] = 0;
	mailbuffer[2] = BCM2835_VC_TAG_GET_PHYS_WH;
	mailbuffer[3] = 8;
	mailbuffer[4] = 0;
	mailbuffer[5] = 0;
	mailbuffer[6] = 0;
	mailbuffer[7] = 0;

	bcm2835_mailbox_write(BCM2835_MAILBOX_PROP_CHANNEL, GPU_MEM_BASE + (uint32_t)&mailbuffer);
	(void)bcm2835_mailbox_read(BCM2835_MAILBOX_PROP_CHANNEL);

	fb_width  = mailbuffer[5];
	fb_height = mailbuffer[6];

	if ((fb_width == 0) && (fb_height == 0)) {
		fb_width = WIDTH;
		fb_height = HEIGHT;
	}

	if ((fb_width == 0) || (fb_height == 0)) {
		return CONSOLE_FAIL_INVALID_RESOLUTION;
	}

	mailbuffer[0] = 22 * 4;
	mailbuffer[1] = 0;

	mailbuffer[2] = BCM2835_VC_TAG_SET_PHYS_WH;
	mailbuffer[3] = 8;
	mailbuffer[4] = 8;
	mailbuffer[5] = fb_width;
	mailbuffer[6] = fb_height;

	mailbuffer[7] = BCM2835_VC_TAG_SET_VIRT_WH;
	mailbuffer[8] = 8;
	mailbuffer[9] = 8;
	mailbuffer[10] = fb_width;
	mailbuffer[11] = fb_height;

	mailbuffer[12] = BCM2835_VC_TAG_SET_DEPTH;
	mailbuffer[13] = 4;
	mailbuffer[14] = 4;
	mailbuffer[15] = BPP;

	mailbuffer[16] = BCM2835_VC_TAG_ALLOCATE_BUFFER;
	mailbuffer[17] = 8;
	mailbuffer[18] = 4;
	mailbuffer[19] = 16;
	mailbuffer[20] = 0;

	mailbuffer[21] = 0;

	bcm2835_mailbox_write(BCM2835_MAILBOX_PROP_CHANNEL, GPU_MEM_BASE + (uint32_t)&mailbuffer);
	(void)bcm2835_mailbox_read(BCM2835_MAILBOX_PROP_CHANNEL);

	fb_addr = mailbuffer[19] & 0x3FFFFFFF;;
	fb_size = mailbuffer[20];

	if ((fb_addr == 0) || (fb_size == 0)) {
		return CONSOLE_FAIL_INVALID_TAG_DATA;
	}

	fb_depth = mailbuffer[15];

	mailbuffer[0] = 7 * 4;
	mailbuffer[1] = 0;

	mailbuffer[2] = BCM2835_VC_TAG_GET_PITCH;
	mailbuffer[3] = 4;
	mailbuffer[4] = 0;
	mailbuffer[5] = 0;

	mailbuffer[6] = 0;

	bcm2835_mailbox_write(BCM2835_MAILBOX_PROP_CHANNEL, GPU_MEM_BASE + (uint32_t) &mailbuffer);
	(void) bcm2835_mailbox_read(BCM2835_MAILBOX_PROP_CHANNEL);

	fb_pitch = mailbuffer[5];

	if (fb_pitch == 0) {
		return CONSOLE_FAIL_INVALID_PITCH_DATA;
	}

	return CONSOLE_OK;
}
