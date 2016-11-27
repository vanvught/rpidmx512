/**
 * @file console.c
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "arm/synchronize.h"
#include "arm/arm.h"

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

static int current_x = 0;					///<
static int current_y = 0;					///<
static int saved_x = 0;						///<
static int saved_y = 0;						///<
static uint16_t cur_fore = CONSOLE_WHITE;	///<
static uint16_t cur_back = CONSOLE_BLACK;	///<
static uint16_t saved_fore = CONSOLE_WHITE;	///<
static uint16_t saved_back = CONSOLE_BLACK;	///<
static uint32_t fb_addr;					///< Address of buffer allocated by VC
static uint32_t fb_size;					///< Size of buffer allocated by VC
static uint32_t fb_depth;					///< Depth (bits per pixel)

static uint16_t top_row = (uint16_t) 0;		///<

#if defined (ARM_ALLOW_MULTI_CORE)
static volatile int lock = 0;
#endif

/**
 *
 * @return
 */
const uint32_t console_get_address(void) {
	return fb_addr;
}

/**
 *
 * @return
 */
const uint32_t console_get_width(void) {
	return WIDTH;
}

/**
 *
 * @return
 */
const uint32_t console_get_height(void) {
	return HEIGHT;
}

/**
 *
 * @return
 */
const uint32_t console_get_size(void) {
	return fb_size;
}

/**
 *
 * @return
 */
const uint32_t console_get_depth(void) {
	return fb_depth;
}

/**
 *
 * @return
 */
const int console_get_line_width(void) {
	return WIDTH / CHAR_W;
}

/**
 *
 * @return
 */
uint16_t console_get_top_row(void) {
	return top_row;
}

/**
 *
 * @param row
 */
void console_set_top_row(uint16_t row) {
	if (row > HEIGHT / CHAR_H) {
		top_row = 0;
	} else {
		top_row = row;
	}

	current_x = (int) 0;
	current_y = (int) row;
}

/**
 *
 * @param address
 */
static void clear_row(uint32_t *address) {
	uint16_t i;
	const uint32_t value = ((uint32_t)cur_back << 16) | (uint32_t)cur_back;
	for (i = 0 ; i < (CHAR_H * WIDTH) / 2 ; i++) {
		*address++ =  value;
	}
}

/**
 *
 */
inline static void newline() {
	uint32_t i;
	uint32_t *address;
	uint32_t *to;
	uint32_t *from;

	current_y++;
	current_x = 0;

	if (current_y == HEIGHT / CHAR_H) {
		if (top_row == 0) {
			/* Pointer to row = 0 */
			to = (uint32_t *) (fb_addr);
			/* Pointer to row = 1 */
			from = to + (CHAR_H * WIDTH) / 2;
			/* Copy block from {row = 1, rows} to {row = 0, rows - 1} */
			i = ((HEIGHT - CHAR_H) * WIDTH) / 2;
		} else {
			to = (uint32_t *) (fb_addr) + ((CHAR_H * WIDTH) * top_row) / 2;
			from = to + (CHAR_H * WIDTH) / 2;
			i = ((HEIGHT - CHAR_H) * WIDTH - ((CHAR_H * WIDTH) * top_row)) / 2;
		}

		memcpy_blk(to, from, i/8);

		/* Clear last row */
		address = (uint32_t *)(fb_addr) + ((HEIGHT - CHAR_H) * WIDTH) / 2;
		clear_row(address);

		current_y--;
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
		line = (uint8_t) *p++;
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
#if defined (ARM_ALLOW_MULTI_CORE)
	while (__sync_lock_test_and_set(&lock, 1) == 1);
#endif

	if (ch == (int)'\n') {
		newline();
	} else if (ch == (int)'\r') {
		current_x = 0;
	} else if (ch == (int)'\t') {
		current_x += 4;
	} else {
		draw_char(ch, current_x * CHAR_W, current_y * CHAR_H, cur_fore, cur_back);
		current_x++;
		if (current_x == WIDTH / CHAR_W) {
			newline();
		}
	}

#if defined (ARM_ALLOW_MULTI_CORE)
	__sync_lock_release(&lock);
#endif

	return ch;
}

/**
 *
 * @param s
 * @return
 */
int console_puts(const char *s) {
	char c;
	int i = 0;;

	while ((c = *s++) != (char) 0) {
		i++;
		(void) console_putc((int) c);
	}

	return i;
}

/**
 *
 * @param s
 * @param n
 */
void console_write(const char *s, int n) {
	char c;

	while (((c = *s++) != (char) 0) && (n-- != 0)) {
		(void) console_putc((int) c);
	}
}

/**
 *
 * @param s
 * @return
 */
int console_error(const char *s) {
	char c;
	int i = 0;;

	uint16_t fore_current = cur_fore;
	uint16_t back_current = cur_back;

	cur_fore = CONSOLE_RED;
	cur_back = CONSOLE_BLACK;

	(void) console_puts("Error <");

	while ((c = *s++) != (char) 0) {
		i++;
		(void) console_putc((int) c);
	}

	(void) console_puts(">\n");

	cur_fore = fore_current;
	cur_back = back_current;

	return i;
}

/**
 *
 */
int console_status(uint16_t const color, const char *s) {
	char c;
	int i = 0;;

	uint16_t const fore_current = cur_fore;
	uint16_t const back_current = cur_back;
	int const s_y = current_y;
	int const s_x = current_x;

#if defined (ARM_ALLOW_MULTI_CORE)
	while (__sync_lock_test_and_set(&lock, 1) == 1);
#endif

	console_clear_line(29);

	cur_fore = color;
	cur_back = CONSOLE_BLACK;

	while ((c = *s++) != (char) 0) {
		i++;
		(void) console_putc((int) c);
	}

	current_y = s_y;
	current_x = s_x;

	cur_fore = fore_current;
	cur_back = back_current;

#if defined (ARM_ALLOW_MULTI_CORE)
	__sync_lock_release(&lock);
#endif

	return i;
}

#define TO_HEX(i)	((i) < 10) ? (uint8_t)'0' + (i) : (uint8_t)'A' + ((i) - (uint8_t)10)

/**
 *
 * @param data
 */
void console_puthex(const uint8_t data) {
	(void) console_putc((int) (TO_HEX(((data & 0xF0) >> 4))));
	(void) console_putc((int) (TO_HEX(data & 0x0F)));
}

/**
 *
 */
void console_puthex_fg_bg(const uint8_t data, const uint16_t fore, const uint16_t back) {
	uint16_t fore_current = cur_fore;
	uint16_t back_current = cur_back;

	cur_fore = fore;
	cur_back = back;

	(void) console_putc((int) (TO_HEX(((data & 0xF0) >> 4))));
	(void) console_putc((int) (TO_HEX(data & 0x0F)));

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

	current_x = 0;
	current_y = 0;
}

/**
 * Sets the position of the cursor to the position (col, row).
 *
 * @param x The new col for the cursor.
 * @param y The new row for the cursor.
 */
void console_set_cursor(const int x, const int y) {
#if defined (ARM_ALLOW_MULTI_CORE)
	while (__sync_lock_test_and_set(&lock, 1) == 1);
#endif

	if (x > WIDTH / CHAR_W)
		current_x = 0;
	else
		current_x = x;

	if (y > HEIGHT / CHAR_H)
		current_y = 0;
	else
		current_y = y;

#if defined (ARM_ALLOW_MULTI_CORE)
	__sync_lock_release(&lock);
#endif
}

/**
 *
 */
void console_save_cursor(void) {
	saved_y = current_y;
	saved_x = current_x;
	saved_back = cur_back;
	saved_fore = cur_fore;
}

/**
 *
 */
void console_restore_cursor(void) {
	current_y = saved_y;
	current_x = saved_x;
	cur_back = saved_back;
	cur_fore = saved_fore;
}

/**
 *
 */
void console_save_color(void) {
	saved_back = cur_back;
	saved_fore = cur_fore;
}

/**
 *
 */
void console_restore_color(void) {
	cur_back = saved_back;
	cur_fore = saved_fore;
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
 * @param fore
 * @param back
 */
void console_set_fg_bg_color(const uint16_t fore, const uint16_t back) {
	cur_fore = fore;
	cur_back = back;
}

/**
 *
 * @param line
 */
void console_clear_line(const int line) {
	uint32_t *address;

	if (line > HEIGHT / CHAR_H) {
		return;
	} else {
		current_y = line;
	}

	current_x = 0;

	address = (uint32_t *)(fb_addr) + (line * CHAR_H * WIDTH) / 2;
	clear_row(address);
}

/**
 *
 * @return
 */
int console_init() {
	uint32_t *mailbuffer = (uint32_t *)MEM_COHERENT_REGION;

	mailbuffer[0] = 22 * 4;
	mailbuffer[1] = 0;

	mailbuffer[2] = BCM2835_VC_TAG_SET_PHYS_WH;
	mailbuffer[3] = 8;
	mailbuffer[4] = 8;
	mailbuffer[5] = WIDTH;
	mailbuffer[6] = HEIGHT;

	mailbuffer[7] = BCM2835_VC_TAG_SET_VIRT_WH;
	mailbuffer[8] = 8;
	mailbuffer[9] = 8;
	mailbuffer[10] = WIDTH;
	mailbuffer[11] = HEIGHT;

	mailbuffer[12] = BCM2835_VC_TAG_SET_DEPTH;
	mailbuffer[13] = 4;
	mailbuffer[14] = 4;
	mailbuffer[15] = (uint32_t) BPP;

	mailbuffer[16] = BCM2835_VC_TAG_ALLOCATE_BUFFER;
	mailbuffer[17] = 8;
	mailbuffer[18] = 4;
	mailbuffer[19] = 16;
	mailbuffer[20] = 0;

	mailbuffer[21] = 0;

#if defined ( RPI2 ) || defined ( RPI3 )
	clean_data_cache();
#endif

	dsb();

#if defined ( RPI2 ) || defined ( RPI3 )
	dmb();
#endif

	bcm2835_mailbox_flush();
	bcm2835_mailbox_write(BCM2835_MAILBOX_PROP_CHANNEL, GPU_MEM_BASE + (uint32_t)mailbuffer);
	(void)bcm2835_mailbox_read(BCM2835_MAILBOX_PROP_CHANNEL);

#if defined ( RPI2 ) || defined ( RPI3 )
	dmb();
	invalidate_data_cache();
#endif

	dmb();

	if (mailbuffer[1] != BCM2835_MAILBOX_SUCCESS) {
		return CONSOLE_FAIL_SETUP_FB;
	}

	fb_addr = mailbuffer[19] & 0x3FFFFFFF;
	fb_size = mailbuffer[20];

	if ((fb_addr == 0) || (fb_size == 0)) {
		return CONSOLE_FAIL_INVALID_TAG_DATA;
	}

	fb_depth = mailbuffer[15];

#if defined (ARM_ALLOW_MULTI_CORE)
	lock = 0;
#endif
	return CONSOLE_OK;
}
