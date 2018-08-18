/**
 * @file esp8266.c
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "arm/synchronize.h"

#include "bcm2835.h"
#include "bcm2835_gpio.h"

/*
 *      RP					ESP8266		Logic Analyzer
 *
 * 11	GPIO17	--      ->	GPIO5		orange
 * 13	GPIO27	<-      --	GPIO4		red
 *
 * 15	GPIO22	<- DATA ->	GPI012		purple
 * 16	GPIO23	<- DATA ->	GPI013		blue
 * 18	GPIO24	<- DATA ->	GPI014		green
 * 22	GPIO25	<- DATA ->	GPI015		yellow
 */

typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} esp8266_pcast32;

/**
 *
 */
static void data_gpio_fsel_output(void) {
	dmb();

	uint32_t value = BCM2835_GPIO->GPFSEL2;
	value &= ~(7 << 6);
	value |= BCM2835_GPIO_FSEL_OUTP << 6;
	value &= ~(7 << 9);
	value |= BCM2835_GPIO_FSEL_OUTP << 9;
	value &= ~(7 << 12);
	value |= BCM2835_GPIO_FSEL_OUTP << 12;
	value &= ~(7 << 15);
	value |= BCM2835_GPIO_FSEL_OUTP << 15;
	BCM2835_GPIO->GPFSEL2 = value;
}

/**
 *
 */
static void data_gpio_fsel_input(void) {
	dmb();

	uint32_t value = BCM2835_GPIO->GPFSEL2;
	value &= ~(7 << 6);
	value |= BCM2835_GPIO_FSEL_INPT << 6;
	value &= ~(7 << 9);
	value |= BCM2835_GPIO_FSEL_INPT << 9;
	value &= ~(7 << 12);
	value |= BCM2835_GPIO_FSEL_INPT << 12;
	value &= ~(7 << 15);
	value |= BCM2835_GPIO_FSEL_INPT << 15;
	BCM2835_GPIO->GPFSEL2 = value;
}

/**
 *
 */
void esp8266_init(void) {
	dmb();

	uint32_t value = BCM2835_GPIO->GPFSEL1;
	value &= ~(7 << 21);
	value |= BCM2835_GPIO_FSEL_OUTP << 21;
	BCM2835_GPIO->GPFSEL1 = value;
	value = BCM2835_GPIO->GPFSEL2;
	value &= ~(7 << 21);
	value |= BCM2835_GPIO_FSEL_INPT << 21;
	BCM2835_GPIO->GPFSEL2 = value;

	bcm2835_gpio_clr(17);
	udelay(1000);

	while (BCM2835_GPIO->GPLEV0 & (1 << 27));

	dmb();
}

/**
 *
 * @param data
 */
void esp8266_write_4bits(const uint8_t data) {
	data_gpio_fsel_output();

	// 22, 23, 24, 25
	uint32_t out_gpio = (data & 0x0F) << 22;
	BCM2835_GPIO->GPSET0 = out_gpio;
	BCM2835_GPIO->GPCLR0 = out_gpio ^ (0x0F << 22);
	// tell that we have data available for read
	bcm2835_gpio_set(17);
	// wait for ack, wait for 0 -> 1
	while (!(BCM2835_GPIO->GPLEV0 & (1 << 27)));
	// we have 1. now wait for 0
	bcm2835_gpio_clr(17); // we acknowledge, and wait for zero
	while (BCM2835_GPIO->GPLEV0 & (1 << 27));

	dmb();
}

/**
 *
 * @param data
 */
inline static void _write_byte(const uint8_t data) {
	// 22, 23, 24, 25
	uint32_t out_gpio = (data & 0x0F) << 22;
	BCM2835_GPIO->GPSET0 = out_gpio;
	BCM2835_GPIO->GPCLR0 = out_gpio ^ (0x0F << 22);
	// tell that we have data available for read
	bcm2835_gpio_set(17);
	// wait for ack, wait for 0 -> 1
	while (!(BCM2835_GPIO->GPLEV0 & (1 << 27)));
	// we have 1. now wait for 0
	bcm2835_gpio_clr(17); // we acknowledge, and wait for zero
	while (BCM2835_GPIO->GPLEV0 & (1 << 27));

	// 22, 23, 24, 25
	out_gpio = ((data & 0xF0) >> 4) << 22;
	BCM2835_GPIO->GPSET0 = out_gpio;
	BCM2835_GPIO->GPCLR0 = out_gpio ^ (0x0F << 22);
	// tell that we have data available for read
	bcm2835_gpio_set(17);
	// wait for ack, wait for 0 -> 1
	while (!(BCM2835_GPIO->GPLEV0 & (1 << 27)));
	// we have 1. now wait for 0
	bcm2835_gpio_clr(17); // we acknowledge, and wait for zero
	while (BCM2835_GPIO->GPLEV0 & (1 << 27));
}

/**
 *
 * @param data
 */
void esp8266_write_byte(const uint8_t byte) {
	data_gpio_fsel_output();

	_write_byte(byte);

	dmb();
}

/**
 *
 * @param half_word
 */
void esp8266_write_halfword(const uint16_t half_word) {
	data_gpio_fsel_output();

	_write_byte((uint8_t)(half_word & (uint16_t)0xFF));
	_write_byte((uint8_t)((half_word >> 8) & (uint16_t)0xFF));

	dmb();
}

/**
 *
 * @param word
 */
void esp8266_write_word(const uint32_t word) {
	esp8266_pcast32 u32 __attribute__((aligned(4)));

	data_gpio_fsel_output();

	u32.u32 = word;

	_write_byte(u32.u8[0]);
	_write_byte(u32.u8[1]);
	_write_byte(u32.u8[2]);
	_write_byte(u32.u8[3]);

	dmb();
}


/**
 *
 * @param data
 * @param len
 */
void esp8266_write_bytes(const uint8_t *data, const uint16_t len) {
	uint8_t *p = (uint8_t *)data;
	uint8_t d;
	uint16_t i;

	data_gpio_fsel_output();

	for (i = 0; i < len; i++) {
		d = *p;
		_write_byte(d);
		p++;
	}

	dmb();
}

/**
 *
 * @param data
 */
void esp8266_write_str(const char *data) {
	uint8_t *p = (uint8_t *)data;
	uint8_t d;

	data_gpio_fsel_output();

	while (*p != (char)0) {
		d = *p;
		_write_byte(d);
		p++;
	}

	_write_byte(0);

	dmb();
}

/**
 *
 * @return
 */
inline static uint8_t _read_byte(void) {
	uint8_t data;

	bcm2835_gpio_set(17);
	while (!(BCM2835_GPIO->GPLEV0 & (1 << 27)));
	uint32_t in_gpio = BCM2835_GPIO->GPLEV0 >> 22;
	data = (uint8_t)(in_gpio & 0x0F);
	bcm2835_gpio_clr(17);
	while (BCM2835_GPIO->GPLEV0 & (1 << 27));

	bcm2835_gpio_set(17);
	while (!(BCM2835_GPIO->GPLEV0 & (1 << 27)));
	in_gpio = BCM2835_GPIO->GPLEV0 >> 22;
	data = data | ((uint8_t)(in_gpio & 0x0F) << 4);
	bcm2835_gpio_clr(17);
	while (BCM2835_GPIO->GPLEV0 & (1 << 27));

	return data;
}

/**
 *
 * @return
 */
uint8_t esp8266_read_byte(void) {
	uint8_t data;

	data_gpio_fsel_input();

	data = _read_byte();

	dmb();

	return data;
}

/**
 *
 * @param data
 * @param len
 */
void esp8266_read_bytes(const uint8_t *data, const uint16_t len){
	uint8_t *p = (uint8_t *)data;
	uint16_t i;

	data_gpio_fsel_input();

	for (i = 0 ; i < len; i++) {
		*p = _read_byte();
		p++;
	}

	dmb();
}

/**
 *
 * @return
 */
uint16_t esp8266_read_halfword(void) {
	uint16_t data;

	data_gpio_fsel_input();

	data = _read_byte();
	data |= (_read_byte() << 8);

	dmb();

	return data;
}

/**
 *
 * @return
 */
uint32_t esp8266_read_word(void) {
	esp8266_pcast32 u32  __attribute__((aligned(4)));

	data_gpio_fsel_input();

	u32.u8[0] = _read_byte();
	u32.u8[1] = _read_byte();
	u32.u8[2] = _read_byte();
	u32.u8[3] = _read_byte();

	dmb();

	return u32.u32;
}

/**
 *
 * @param s
 * @param len
 */
void esp8266_read_str(char *s, uint16_t *len) {
	const char *p = s;
	uint8_t ch;
	uint16_t n = *len;

	data_gpio_fsel_input();

	while ((ch = _read_byte()) != (uint8_t) 0) {
		if (n > (uint16_t) 0) {
			*s++ = (char) ch;
			n--;
		}
	}

	*len = (uint16_t) (s - p);

	while (n > 0) {
		*s++ = '\0';
		--n;
	}

	dmb();
}

/**
 *
 * @return
 */
const bool esp8266_detect(void) {
	esp8266_init();

	data_gpio_fsel_output();

	// send a CMD_NOP
	const uint32_t out_gpio = (0x00 & 0x0F) << 22;
	BCM2835_GPIO->GPSET0 = out_gpio;
	BCM2835_GPIO->GPCLR0 = out_gpio ^ (0x0F << 22);

	bcm2835_gpio_set(17);// Tell that we have data available. Wait for ack, wait for 0 -> 1

	uint32_t micros_now = BCM2835_ST->CLO;

	// wait 0.5 second for the ESP8266 to respond
	while ((!(BCM2835_GPIO->GPLEV0 & (1 << 27))) && (BCM2835_ST->CLO - micros_now < (uint32_t) 500000))
		;

	if (!(BCM2835_GPIO->GPLEV0 & (1 << 27))) {
		return false;
	}

	bcm2835_gpio_clr(17); // we acknowledge, and wait for zero

	while (BCM2835_GPIO->GPLEV0 & (1 << 27))
		;

	dmb();

	return true;
}
