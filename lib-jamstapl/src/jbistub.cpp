/**
 * @file jbistub.cpp
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdlib>
#include <cstdio>

#include "jbiport.h"
#include "jamstapl.h"

#include "hal_gpio.h"

void uart0_puts(const char *s);
int uart0_printf(const char* fmt, ...);

#define  puts  	uart0_puts
#define  printf	uart0_printf

#define GPIO_TDO		GPIO_EXT_7	// GPIO_EXT_12
#define GPIO_TDI		GPIO_EXT_10	// GPIO_EXT_16
#define GPIO_TCK		GPIO_EXT_8	// GPIO_EXT_18
#define GPIO_TMS		GPIO_EXT_11	// GPIO_EXT_22

void *jbi_malloc(unsigned int size) {
	return malloc(size);
}

void jbi_free(void *ptr) {
	free(ptr);
}

/************************************************************************
*
*	Customized interface functions for Jam STAPL ByteCode Player I/O:
*
*	jbi_jtag_io()
*	jbi_message()
*	jbi_delay()
*/

static JamSTAPL *s_pJamSTAPL = nullptr;
static bool s_bVerbose = false;

void jbi_jtag_io_init(JamSTAPL *pJamSTAPL, bool bVerbose) {
	s_pJamSTAPL = pJamSTAPL;
	s_bVerbose = bVerbose;

	FUNC_PREFIX(gpio_fsel(GPIO_TDO, GPIO_FSEL_INPUT));
	FUNC_PREFIX(gpio_fsel(GPIO_TDI, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_fsel(GPIO_TCK, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_fsel(GPIO_TMS, GPIO_FSEL_OUTPUT));
}

int jbi_jtag_io(int tms, int tdi, int read_tdo) {
	int tdo = 0;

	if (tdi) {
		FUNC_PREFIX(gpio_set(GPIO_TDI));
	} else {
		FUNC_PREFIX(gpio_clr(GPIO_TDI));
	}

	if (tms) {
		FUNC_PREFIX(gpio_set(GPIO_TMS));
	} else {
		FUNC_PREFIX(gpio_clr(GPIO_TMS));
	}

	if (read_tdo) {
		tdo = FUNC_PREFIX(gpio_lev(GPIO_TDO));
	}

	FUNC_PREFIX(gpio_set(GPIO_TCK));
	FUNC_PREFIX(gpio_clr(GPIO_TCK));

	return tdo;
}

void jbi_message(char *message_text) {
	s_pJamSTAPL->SetMessage(message_text);

	if (s_bVerbose) {
		puts(message_text);
		puts("\n");
	}
}

void jbi_delay(long microseconds) {
	udelay(microseconds);
}

/************************************************************************
*
*/

void jbi_export_integer(char *key, long value) {
	s_pJamSTAPL->SetExportInteger(key, value);

	if (s_bVerbose) {
		printf("Export: key = \"%s\", value = %p\n", key, value);
	}
}

#define HEX_LINE_CHARS 72
#define HEX_LINE_BITS (HEX_LINE_CHARS * 4)

static char conv_to_hex(unsigned long value) {
	char c;

	if (value > 9) {
		c = static_cast<char>(value + ('A' - 10));
	} else {
		c = static_cast<char>(value + '0');
	}

	return c;
}

void jbi_export_boolean_array(char *key, unsigned char *data, long count) {
	char string[HEX_LINE_CHARS + 1];
	long i, offset;
	unsigned long size, line, lines, linebits, value, j, k;

	if (s_bVerbose) {
		if (count > HEX_LINE_BITS) {
			printf("Export: key = \"%s\", %ld bits, value = HEX\n", key, count);
			lines = (count + (HEX_LINE_BITS - 1)) / HEX_LINE_BITS;

			for (line = 0; line < lines; ++line) {
				if (line < (lines - 1)) {
					linebits = HEX_LINE_BITS;
					size = HEX_LINE_CHARS;
					offset = count - ((line + 1) * HEX_LINE_BITS);
				} else {
					linebits = count - ((lines - 1) * HEX_LINE_BITS);
					size = (linebits + 3) / 4;
					offset = 0L;
				}

				string[size] = '\0';
				j = size - 1;
				value = 0;

				for (k = 0; k < linebits; ++k) {
					i = k + offset;
					if (data[i >> 3] & (1 << (i & 7)))
						value |= (1 << (i & 3));
					if ((i & 3) == 3) {
						string[j] = conv_to_hex(value);
						value = 0;
						--j;
					}
				}
				if ((k & 3) > 0)
					string[j] = conv_to_hex(value);

				printf("%s\n", string);
			}
		} else {
			size = (count + 3) / 4;
			string[size] = '\0';
			j = size - 1;
			value = 0;

			for (i = 0; i < count; ++i) {
				if (data[i >> 3] & (1 << (i & 7)))
					value |= (1 << (i & 3));
				if ((i & 3) == 3) {
					string[j] = conv_to_hex(value);
					value = 0;
					--j;
				}
			}
			if ((i & 3) > 0)
				string[j] = conv_to_hex(value);

			printf("Export: key = \"%s\", %ld bits, value = HEX %s\n", key, count, string);
		}
	}
}
