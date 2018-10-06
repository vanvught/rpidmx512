#if defined(CONSOLE_ILI9340)
/**
 * @file console_ili9340.c
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

#include <stdint.h>

#if defined(RASPPI)
 #include <stdio.h>
 #include "bcm2835.h"
#elif defined(H3)
 #include "h3_gpio.h"
 #include "h3_spi.h"
 #include "h3_board.h"
#else
 #include "bcm2835.h"
 #include "bcm2835_gpio.h"
 #include "bcm2835_spi.h"
#endif

#include "console.h"

#if !defined(H3)
 #define D_C  	RPI_V2_GPIO_P1_16
 #define RES  	RPI_V2_GPIO_P1_18
#else
 #define D_C	GPIO_EXT_33
 #define RES	GPIO_EXT_35
#endif

#include "./../../lib-bob/src/font_cp437.h" //FIXME font_cp437

#define FONT_CHAR_W			8
#define FONT_CHAR_H			8

#define TFT_WIDTH			320
#define TFT_HEIGHT			240

static uint16_t current_x = 0;
static uint16_t current_y = 0;
static uint16_t saved_x = 0;
static uint16_t saved_y = 0;
static uint16_t cur_fore = CONSOLE_WHITE;
static uint16_t cur_back = CONSOLE_BLACK;
static uint16_t saved_fore = CONSOLE_WHITE;
static uint16_t saved_back = CONSOLE_BLACK;

static uint16_t top_row = 0;

static uint16_t buffer[TFT_HEIGHT * TFT_WIDTH] __attribute__((aligned(4)));

struct char_info {
	int ch;
	uint16_t fore;
	uint16_t back;
} static shadow_ram[(TFT_HEIGHT / FONT_CHAR_H) * (TFT_WIDTH / FONT_CHAR_W)] __attribute__((aligned(4)));

inline static void write_command(uint8_t c) {
	bcm2835_gpio_clr(D_C);
	bcm2835_spi_transfer(c);
	bcm2835_gpio_set(D_C);
}

inline static void write_data_byte(uint8_t c) {
	bcm2835_spi_transfer(c);
}

inline static void write_data_word(uint16_t w) {
	bcm2835_spi_write(w);
}

void _reset(void) {
	bcm2835_gpio_fsel(D_C, BCM2835_GPIO_FSEL_OUTP); // D/C
	bcm2835_gpio_fsel(RES, BCM2835_GPIO_FSEL_OUTP); // Reset
	bcm2835_gpio_set(D_C);

	bcm2835_gpio_clr(RES);   // Reset
	bcm2835_delay(100);
	bcm2835_gpio_set(RES);   // Reset off
	bcm2835_delay(100);
}

static void _setup(void) {
	write_command(0xC0);    //Power control
	write_data_byte(0x23);   //VRH[5:0]

	write_command(0xC1);    //Power control
	write_data_byte(0x10);   //SAP[2:0];BT[3:0]

	write_command(0xC5);    //VCM control
	write_data_byte(0x3e); //
	write_data_byte(0x28);

	write_command(0xC7);    //VCM control2
	write_data_byte(0x86);  //--

	write_command(0x36);    // Memory Access Control
	write_data_byte(0x48); //

	write_command(0x3A);
	write_data_byte(0x55);

	write_command(0xB1);
	write_data_byte(0x00);
	write_data_byte(0x18);

	write_command(0xB6);    // Display Function Control
	write_data_byte(0x08);
//  lcdWriteDataByte(0x82);
	write_data_byte(0xA2);
	write_data_byte(0x27);

	write_command(0xF2);    // 3Gamma Function Disable
	write_data_byte(0x00);

	write_command(0x26);    //Gamma curve selected
	write_data_byte(0x01);

	write_command(0xE0);    //Set Gamma
	write_data_byte(0x0F);
	write_data_byte(0x31);
	write_data_byte(0x2B);
	write_data_byte(0x0C);
	write_data_byte(0x0E);
	write_data_byte(0x08);
	write_data_byte(0x4E);
	write_data_byte(0xF1);
	write_data_byte(0x37);
	write_data_byte(0x07);
	write_data_byte(0x10);
	write_data_byte(0x03);
	write_data_byte(0x0E);
	write_data_byte(0x09);
	write_data_byte(0x00);

	write_command(0XE1);    //Set Gamma
	write_data_byte(0x00);
	write_data_byte(0x0E);
	write_data_byte(0x14);
	write_data_byte(0x03);
	write_data_byte(0x11);
	write_data_byte(0x07);
	write_data_byte(0x31);
	write_data_byte(0xC1);
	write_data_byte(0x48);
	write_data_byte(0x08);
	write_data_byte(0x0F);
	write_data_byte(0x0C);
	write_data_byte(0x31);
	write_data_byte(0x36);
	write_data_byte(0x0F);

	write_command(0x11);    //Exit Sleep
	bcm2835_delay(120);
	write_command(0x29);
}

int console_init(void) {
	int i;
#if defined (__linux__)
	if (bcm2835_init() == -1) {
		printf("bmc2835_init Error\n");
		return -1;
	}
#endif
	bcm2835_spi_begin();
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(8);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

	_reset();
	_setup();

	uint32_t *p = (uint32_t *)buffer;

	for (i = 0; i < sizeof(buffer) / sizeof(buffer[0] / 2); i++) {
		*p++ = cur_back << 16 | cur_back;
	}

	console_clear();
	return 0;
}

void console_off(void) {
  write_command(0x28);
}

void console_on(void) {
  write_command(0x29);
}

uint16_t console_get_line_width(void) {
	return TFT_WIDTH / FONT_CHAR_W;
}

uint16_t console_get_top_row(void) {
	return top_row;
}

void console_set_top_row(uint16_t row) {
	if (row > TFT_HEIGHT / FONT_CHAR_H) {
		top_row = 0;
	} else {
		top_row = row;
	}

	current_x = 0;
	current_y = row;
}

inline static void _draw_char(int c, uint32_t x, uint32_t y, uint16_t fore, uint16_t back) {
	uint32_t i, j;
	uint8_t line;
	unsigned char *p = (unsigned char *) cp437_font + (c * (int) FONT_CHAR_H);

	uint16_t buffer[FONT_CHAR_H * FONT_CHAR_W];
	uint16_t *q = buffer;

	for (i = 0; i < FONT_CHAR_H; i++) {
		line = (uint8_t) *p++;
		for (j = 0; j < FONT_CHAR_W; j++) {
			if ((line & 0x1) != 0) {
				*q = __builtin_bswap16(fore);
			} else {
				*q = __builtin_bswap16(back);
			}
			line >>= 1;
			q++;
		}
	}

	write_command(0x2A);
	write_data_word(x);
	write_data_word(x + FONT_CHAR_H - 1);
	write_command(0x2B);
	write_data_word(y);
	write_data_word(y + FONT_CHAR_W - 1);
	write_command(0x2C);

	bcm2835_spi_writenb((char *) buffer, 2 * FONT_CHAR_H * FONT_CHAR_W);
}

static void draw_char(int ch, uint16_t x, uint16_t y, uint16_t fore, uint16_t back) {
	_draw_char(ch, x * FONT_CHAR_W, y * FONT_CHAR_H, fore, back);

	const uint32_t index = y + x * 40;
	shadow_ram[index].ch = ch;
	shadow_ram[index].fore = fore;
	shadow_ram[index].back = back;
}

static void scroll(void) {
	int i;

	for (i = 0; i < (40 * 29); i++) {
		const int ch = (int) shadow_ram[i + 40].ch;
		const uint16_t fore = shadow_ram[i + 40].fore;
		const uint16_t back = shadow_ram[i + 40].back;
		const uint32_t x = i / 40;
		const uint32_t y = i - 40 * x;

		draw_char(ch, x, y, fore, back);
	}

	for (i = 0; i < 40; i++) {
		draw_char(' ', 29, i, cur_fore, cur_back);
	}
}

inline static void newline(void) {
	current_y++;
	current_x = 0;

	if (current_y == TFT_HEIGHT / FONT_CHAR_H) {
		scroll();
		current_y--;
	}
}

void console_newline(void){
	newline();
}

void console_clear(void) {
	write_command(0x2A);
	write_data_word(0);
	write_data_word(239);
	write_command(0x2B);
	write_data_word(0);
	write_data_word(319);
	write_command(0x2C);

	bcm2835_spi_writenb((char *) buffer, 2 * TFT_HEIGHT * TFT_WIDTH);
}

void console_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
	if (x >= TFT_HEIGHT)
		return;

	if (y >= TFT_WIDTH)
		return;

	write_command(0x2A);
	write_data_word(x);
	write_data_word(x);
	write_command(0x2B);
	write_data_word(y);
	write_data_word(y);
	write_command(0x2C);
	write_data_word(color);
}

int console_putc(int ch) {
	if (ch == (int)'\n') {
		newline();
	} else if (ch == (int)'\r') {
		current_x = 0;
	} else if (ch == (int)'\t') {
		current_x += 4;
	} else {
		draw_char(ch, current_y, current_x, cur_fore, cur_back);
		current_x++;
		if (current_x == TFT_WIDTH / FONT_CHAR_W) {
			newline();
		}
	}

	return ch;
}

int console_puts(const char *s) {
	char c;
	int i = 0;;

	while ((c = *s++) != (char) 0) {
		i++;
		(void) console_putc((int) c);
	}

	return i;
}

void console_write(const char *s, unsigned int n) {
	char c;

	while (((c = *s++) != (char) 0) && (n-- != 0)) {
		(void) console_putc((int) c);
	}
}

void console_clear_line(uint16_t line) {
	if (line > TFT_HEIGHT / FONT_CHAR_H) {
		return;
	} else {
		current_y = line;
	}

	current_x = 0;

	int i;

	for (i = 0; i < 40; i++) {
		draw_char(' ', line, i, cur_fore, cur_back);
	}

}

int console_status(uint16_t color, const char *s) {
	char c;
	int i = 0;

	const uint16_t fore_current = cur_fore;
	const uint16_t back_current = cur_back;

	const uint16_t s_y = current_x;
	const uint16_t s_x = current_y;

	console_clear_line(29);

	cur_fore = color;
	cur_back = CONSOLE_BLACK;

	while ((c = *s++) != (char) 0) {
		i++;
		(void) console_putc((int) c);
	}

	current_x = s_y;
	current_y = s_x;

	cur_fore = fore_current;
	cur_back = back_current;

	return i;
}

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

void console_set_cursor(uint16_t x, uint16_t y) {
	if (x > TFT_WIDTH / FONT_CHAR_W) {
		current_x = 0;
	} else {
		current_x = x;
	}

	if (y > TFT_HEIGHT / FONT_CHAR_H) {
		current_y = 0;
	} else {
		current_y = y;
	}
}

void console_save_cursor(void) {
	saved_y = current_y;
	saved_x = current_x;
	saved_back = cur_back;
	saved_fore = cur_fore;
}

void console_restore_cursor(void) {
	current_y = saved_y;
	current_x = saved_x;
	cur_back = saved_back;
	cur_fore = saved_fore;
}

void console_save_color(void) {
	saved_back = cur_back;
	saved_fore = cur_fore;
}

void console_restore_color(void) {
	cur_back = saved_back;
	cur_fore = saved_fore;
}

void console_set_fg_color(uint16_t fore) {
	cur_fore = fore;
}

void console_set_bg_color(uint16_t back) {
	int i;

	cur_back = back;

	uint32_t *p = (uint32_t *)buffer;

	for (i = 0; i < sizeof(buffer) / sizeof(buffer[0] / 2); i++) {
		*p++ = (back << 16) | back;
	}
}

void console_set_fg_bg_color(uint16_t fore, uint16_t back) {
	cur_fore = fore;
	cur_back = back;
}

#define TO_HEX(i)	((i) < 10) ? (uint8_t)'0' + (i) : (uint8_t)'A' + ((i) - (uint8_t)10)

void console_puthex(uint8_t data) {
	(void) console_putc((int) (TO_HEX(((data & 0xF0) >> 4))));
	(void) console_putc((int) (TO_HEX(data & 0x0F)));
}

void console_puthex_fg_bg(uint8_t data, uint16_t fore, uint16_t back) {
	uint16_t fore_current = cur_fore;
	uint16_t back_current = cur_back;

	cur_fore = fore;
	cur_back = back;

	(void) console_putc((int) (TO_HEX(((data & 0xF0) >> 4))));
	(void) console_putc((int) (TO_HEX(data & 0x0F)));

	cur_fore = fore_current;
	cur_back = back_current;
}
#endif
