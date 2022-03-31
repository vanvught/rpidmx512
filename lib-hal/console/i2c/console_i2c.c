/**
 * @file console_i2c.c
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdbool.h>

#include "console.h"

#include "hal_i2c.h"

static bool s_isConnected;

#if !defined (CONSOLE_I2C_ADDRESS)
# define CONSOLE_I2C_ADDRESS			0x4D
#endif

#if !defined (CONSOLE_I2C_ONBOARD_CRYSTAL)
# define CONSOLE_I2C_ONBOARD_CRYSTAL	14745600UL
#endif

#if !defined (CONSOLE_I2C_BAUDRATE)
# define CONSOLE_I2C_BAUDRATE			115200
#endif

#define	SC16IS7X0_THR	0x00
#define	SC16IS7X0_FCR 	0x02
#define	SC16IS7X0_LCR	0x03
#define	SC16IS7X0_MCR	0x04
#define	SC16IS7X0_SPR	0x07
#define	SC16IS7X0_TLR	0x07
#define	SC16IS7X0_TXLVL	0x08

#define	SC16IS7X0_DLL	0x00
#define	SC16IS7X0_DLH	0X01
#define	SC16IS7X0_EFR	0X02

/** See section 8.3 of the datasheet for definitions
 * of bits in the FIFO Control Register (FCR)
 */
#define FCR_TX_FIFO_RST		(1U << 2)
#define FCR_ENABLE_FIFO		(1U << 0)

/** See section 8.4 of the datasheet for definitions
 * of bits in the Line Control Register (LCR)
 */

#define LCR_BITS8		0x03
#define LCR_BITS1		0x00
#define LCR_NONE		0x00
#define	LCR_ENABLE_DIV	0x80

/**
 * 8.6 Modem Control Register (MCR)
 */
//MCR[2] only accessible when EFR[4] is set
#define	MCR_ENABLE_TCR_TLR	(1U << 2)
#define	MCR_PRESCALE_4		(1U << 7)

/**
 * 8.11 Enhanced Features Register (EFR)
 */
#define	EFR_ENABLE_ENHANCED_FUNCTIONS	(1U << 4)

static void setup() {
	FUNC_PREFIX(i2c_set_address(CONSOLE_I2C_ADDRESS));
	FUNC_PREFIX(i2c_set_baudrate(400000));
}

static void write_register(uint8_t nRegister, uint8_t nValue) {
	char buffer[2];

	buffer[0] = (char)(nRegister);
	buffer[1] = (char)(nValue);

	setup();
	FUNC_PREFIX(i2c_write(buffer, 2));
}

static uint8_t read_byte() {
	char buf[1] = {0};

	setup();
	FUNC_PREFIX(i2c_read(buf, 1));

	return (uint8_t)(buf[0]);
}

uint8_t read_register(uint8_t nRegister) {
	char buf[2];

	buf[0] = (char)(nRegister);
	buf[1] = 0;

	setup();
	FUNC_PREFIX(i2c_write(&buf[0], 1));

	return read_byte();
}

static bool is_writable() {
	if (!s_isConnected) {
		return false;
	}
	return (read_register(SC16IS7X0_TXLVL) != 0);
}

static void set_baud(uint32_t nBaud) {
	uint32_t nPrescaler;

	if ((read_register(SC16IS7X0_MCR) & MCR_PRESCALE_4) == MCR_PRESCALE_4) {
		nPrescaler = 4;
	} else {
		nPrescaler = 1;
	}

	const uint32_t nDivisor = ((CONSOLE_I2C_ONBOARD_CRYSTAL / nPrescaler) / (nBaud * 16));
	const uint8_t nRegisterLCR = read_register(SC16IS7X0_LCR);

	write_register(SC16IS7X0_LCR, (nRegisterLCR | LCR_ENABLE_DIV));
	write_register(SC16IS7X0_DLL, (nDivisor & 0xFF));
	write_register(SC16IS7X0_DLH, ((nDivisor >> 8) & 0xFF));
	write_register(SC16IS7X0_LCR, nRegisterLCR);
}

int __attribute__((cold)) console_init(void) {
	uint8_t LCR = LCR_BITS8;
	LCR |= LCR_NONE;
	LCR |= LCR_BITS1;
	write_register(SC16IS7X0_LCR, LCR);

	set_baud(CONSOLE_I2C_BAUDRATE);

	uint8_t test_character = 'A';
	write_register(SC16IS7X0_SPR, test_character);

	if ((read_register(SC16IS7X0_SPR) != test_character)) {
		s_isConnected = false;
		return CONSOLE_OK;
	}

	s_isConnected = true;

	uint8_t MCR = read_register(SC16IS7X0_MCR);
	MCR |= MCR_ENABLE_TCR_TLR;
	write_register(SC16IS7X0_MCR, MCR);

	uint8_t EFR = read_register(SC16IS7X0_EFR);
	write_register(SC16IS7X0_EFR, (uint8_t)(EFR | EFR_ENABLE_ENHANCED_FUNCTIONS));

	write_register(SC16IS7X0_TLR, (uint8_t)(0x10));

	write_register(SC16IS7X0_EFR, EFR);

	write_register(SC16IS7X0_FCR, (uint8_t)(FCR_TX_FIFO_RST));
	write_register(SC16IS7X0_FCR, FCR_ENABLE_FIFO);

	return CONSOLE_OK;
}

void console_putc(int c) {
	while (!is_writable()) {
	}

	write_register(SC16IS7X0_THR, (uint8_t)(c));
}

void console_puts(const char *s) {
	if (!s_isConnected) {
		return;
	}

	uint8_t *p = (uint8_t *)(s);

	while (*p != '\0') {
		uint32_t nAvailable = read_register(SC16IS7X0_TXLVL);

		while ((*p != '\0') && (nAvailable > 0)) {
			write_register(SC16IS7X0_THR, *p);
			nAvailable--;
			p++;
		}
	}
}

void console_write(const char *s, unsigned int n) {
	if (!s_isConnected) {
		return;
	}

	uint8_t *p = (uint8_t *)(s);

	while (n > 0) {
		uint32_t nAvailable = read_register(SC16IS7X0_TXLVL);

		while ((n > 0) && (nAvailable > 0)) {
			write_register(SC16IS7X0_THR, *p);
			n--;
			nAvailable--;
			p++;
		}
	}
}

void console_error(const char *s) {
	if (!s_isConnected) {
		return;
	}
	console_puts("\x1b[31m");
	console_puts(s);
	console_puts("\x1b[37m");
}

void console_set_fg_color(uint16_t fg) {
	switch (fg) {
	case CONSOLE_BLACK:
		console_puts("\x1b[30m");
		break;
	case CONSOLE_RED:
		console_puts("\x1b[31m");
		break;
	case CONSOLE_GREEN:
		console_puts("\x1b[32m");
		break;
	case CONSOLE_YELLOW:
		console_puts("\x1b[33m");
		break;
	case CONSOLE_WHITE:
		console_puts("\x1b[37m");
		break;
	default:
		console_puts("\x1b[39m");
		break;
	}
}

void console_set_bg_color(uint16_t bg) {
	switch (bg) {
	case CONSOLE_BLACK:
		console_puts("\x1b[40m");
		break;
	case CONSOLE_RED:
		console_puts("\x1b[41m");
		break;
	case CONSOLE_WHITE:
		console_puts("\x1b[47m");
		break;
	default:
		console_puts("\x1b[49m");
		break;
	}
}

void console_status(uint32_t color, const char *s) {
	if (!s_isConnected) {
		return;
	}
	console_set_fg_color((uint16_t) color);
	console_set_bg_color(CONSOLE_BLACK);
	console_puts(s);
	console_putc('\n');
	console_set_fg_color(CONSOLE_WHITE);
}
