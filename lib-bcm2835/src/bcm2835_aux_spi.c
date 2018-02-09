/**
 * @file bcm2835_aux_spi.c
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stddef.h>

#include "arm/synchronize.h"

#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_aux.h"
#include "bcm2835_aux_spi.h"

static uint32_t speed;

#define DIV_ROUND_UP(n,d)	(((n) + (d) - 1) / (d))		///<
#define MIN(a,b)			(((a) < (b)) ? (a) : (b))	///<

#define BCM2835_AUX_SPI_CNTL0_SPEED		0xFFF00000
	#define BCM2835_AUX_SPI_CNTL0_SPEED_MAX		0xFFF
	#define BCM2835_AUX_SPI_CNTL0_SPEED_SHIFT	20

#define BCM2835_AUX_SPI_CNTL0_CS0_N     0x000C0000 // CS 0 low
#define BCM2835_AUX_SPI_CNTL0_CS1_N     0x000A0000 // CS 1 low
#define BCM2835_AUX_SPI_CNTL0_CS2_N 	0x00060000 // CS 2 low

#define BCM2835_AUX_SPI_CNTL0_POSTINPUT	0x00010000
#define BCM2835_AUX_SPI_CNTL0_VAR_CS	0x00008000
#define BCM2835_AUX_SPI_CNTL0_VAR_WIDTH	0x00004000
#define BCM2835_AUX_SPI_CNTL0_DOUTHOLD	0x00003000
#define BCM2835_AUX_SPI_CNTL0_ENABLE	0x00000800
#define BCM2835_AUX_SPI_CNTL0_CPHA_IN	0x00000400
#define BCM2835_AUX_SPI_CNTL0_CLEARFIFO	0x00000200
#define BCM2835_AUX_SPI_CNTL0_CPHA_OUT	0x00000100
#define BCM2835_AUX_SPI_CNTL0_CPOL		0x00000080
#define BCM2835_AUX_SPI_CNTL0_MSBF_OUT	0x00000040
#define BCM2835_AUX_SPI_CNTL0_SHIFTLEN	0x0000003F

#define BCM2835_AUX_SPI_CNTL1_CSHIGH	0x00000700
#define BCM2835_AUX_SPI_CNTL1_IDLE		0x00000080
#define BCM2835_AUX_SPI_CNTL1_TXEMPTY	0x00000040
#define BCM2835_AUX_SPI_CNTL1_MSBF_IN	0x00000002
#define BCM2835_AUX_SPI_CNTL1_KEEP_IN	0x00000001

#define BCM2835_AUX_SPI_STAT_TX_LVL		0xFF000000
#define BCM2835_AUX_SPI_STAT_RX_LVL		0x00FF0000
#define BCM2835_AUX_SPI_STAT_TX_FULL	0x00000400
#define BCM2835_AUX_SPI_STAT_TX_EMPTY	0x00000200
#define BCM2835_AUX_SPI_STAT_RX_FULL	0x00000100
#define BCM2835_AUX_SPI_STAT_RX_EMPTY	0x00000080
#define BCM2835_AUX_SPI_STAT_BUSY		0x00000040
#define BCM2835_AUX_SPI_STAT_BITCOUNT	0x0000003F

/**
 *
 */
void bcm2835_aux_spi_begin(void) {
	dmb();

	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_36, BCM2835_GPIO_FSEL_ALT4);	// SPI1_CE2_N
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_35, BCM2835_GPIO_FSEL_ALT4);	// SPI1_MISO
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_38, BCM2835_GPIO_FSEL_ALT4);	// SPI1_MOSI
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_40, BCM2835_GPIO_FSEL_ALT4);	// SPI1_SCLK

	bcm2835_aux_spi_setClockDivider(bcm2835_aux_spi_CalcClockDivider(1000000));	// Default 1MHz SPI

	BCM2835_AUX->ENABLE = BCM2835_AUX_ENABLE_SPI0;

	BCM2835_SPI1->CNTL1 = 0;
	BCM2835_SPI1->CNTL0 = BCM2835_AUX_SPI_CNTL0_CLEARFIFO;

	dmb();
}

/**
 *
 * @param speed_hz
 * @return
 */
const uint16_t bcm2835_aux_spi_CalcClockDivider(uint32_t speed_hz) {
	uint16_t divider;

	if (speed_hz < (uint32_t) BCM2835_AUX_SPI_CLOCK_MIN) {
		speed_hz = (uint32_t) BCM2835_AUX_SPI_CLOCK_MIN;
	} else if (speed_hz > (uint32_t) BCM2835_AUX_SPI_CLOCK_MAX) {
		speed_hz = (uint32_t) BCM2835_AUX_SPI_CLOCK_MAX;
	}

	divider = (uint16_t) DIV_ROUND_UP(BCM2835_CORE_CLK_HZ, 2 * speed_hz) - 1;

	if (divider > (uint16_t) BCM2835_AUX_SPI_CNTL0_SPEED_MAX) {
		return (uint16_t) BCM2835_AUX_SPI_CNTL0_SPEED_MAX;
	}

	return divider;
}

/**
 *
 * @param divider
 */
void bcm2835_aux_spi_setClockDivider(uint16_t divider) {
		speed = (uint32_t) divider;
}

/**
 *
 * @param data
 */
void bcm2835_aux_spi_write(uint16_t data) {
	uint32_t cntl0 = (speed << BCM2835_AUX_SPI_CNTL0_SPEED_SHIFT);
	cntl0 |= BCM2835_AUX_SPI_CNTL0_CS2_N;
	cntl0 |= BCM2835_AUX_SPI_CNTL0_ENABLE;
	cntl0 |= BCM2835_AUX_SPI_CNTL0_MSBF_OUT;
	cntl0 |= 16; // Shift length

	BCM2835_SPI1->CNTL0 = cntl0;
	BCM2835_SPI1->CNTL1 = BCM2835_AUX_SPI_CNTL1_MSBF_IN;

	while (BCM2835_SPI1->STAT & BCM2835_AUX_SPI_STAT_TX_FULL)
		;

	BCM2835_SPI1->IO = (uint32_t) data << 16;
}

/**
 *
 * @param tbuf
 * @param len
 */
void bcm2835_aux_spi_writenb(const char *tbuf, uint32_t len) {
	char *tx = (char *) tbuf;
	uint32_t tx_len = len;
	uint32_t count;
	uint32_t data;
	uint32_t i;
	uint8_t byte;

	uint32_t cntl0 = (speed << BCM2835_AUX_SPI_CNTL0_SPEED_SHIFT);
	cntl0 |= BCM2835_AUX_SPI_CNTL0_CS2_N;
	cntl0 |= BCM2835_AUX_SPI_CNTL0_ENABLE;
	cntl0 |= BCM2835_AUX_SPI_CNTL0_MSBF_OUT;
	cntl0 |= BCM2835_AUX_SPI_CNTL0_VAR_WIDTH;

	BCM2835_SPI1->CNTL0 = cntl0;
	BCM2835_SPI1->CNTL1 = BCM2835_AUX_SPI_CNTL1_MSBF_IN;

	while (tx_len > 0) {

		while (BCM2835_SPI1->STAT & BCM2835_AUX_SPI_STAT_TX_FULL)
			;

		count = MIN(tx_len, 3);
		data = 0;

		for (i = 0; i < count; i++) {
			byte = (tx != NULL) ? (uint8_t) *tx++ : (uint8_t) 0;
			data |= byte << (8 * (2 - i));
		}

		data |= (count * 8) << 24;
		tx_len -= count;

		if (tx_len != 0) {
			BCM2835_SPI1->TXHOLD = data;
		} else {
			BCM2835_SPI1->IO = data;
		}

		while (BCM2835_SPI1->STAT & BCM2835_AUX_SPI_STAT_BUSY)
			;
		(void) BCM2835_SPI1->IO;
	}
}

/**
 *
 * @param tbuf
 * @param rbuf
 * @param len
 */
void bcm2835_aux_spi_transfernb(const char *tbuf, char *rbuf, uint32_t len) {
	char *tx = (char *)tbuf;
	char *rx = (char *)rbuf;
	uint32_t tx_len = len;
	uint32_t rx_len = len;
	uint32_t count;
	uint32_t data;
	uint32_t i;
	uint8_t byte;

	uint32_t cntl0 = (speed << BCM2835_AUX_SPI_CNTL0_SPEED_SHIFT);
	cntl0 |= BCM2835_AUX_SPI_CNTL0_CS2_N;
	cntl0 |= BCM2835_AUX_SPI_CNTL0_ENABLE;
	cntl0 |= BCM2835_AUX_SPI_CNTL0_MSBF_OUT;
	cntl0 |= BCM2835_AUX_SPI_CNTL0_VAR_WIDTH;

	BCM2835_SPI1->CNTL0 = cntl0;
	BCM2835_SPI1->CNTL1 = BCM2835_AUX_SPI_CNTL1_MSBF_IN;

	while ((tx_len > 0) || (rx_len > 0)) {

		while (!(BCM2835_SPI1->STAT & BCM2835_AUX_SPI_STAT_TX_FULL) && (tx_len > 0)) {
			count = MIN(tx_len, 3);
			data = 0;

			for (i = 0; i < count; i++) {
				byte = (tx != NULL) ? (uint8_t) *tx++ : (uint8_t) 0;
				data |= byte << (8 * (2 - i));
			}

			data |= (count * 8) << 24;
			tx_len -= count;

			if (tx_len != 0) {
				BCM2835_SPI1->TXHOLD = data;
			} else {
				BCM2835_SPI1->IO = data;
			}

		}

		while (!(BCM2835_SPI1->STAT & BCM2835_AUX_SPI_STAT_RX_EMPTY) && (rx_len > 0)) {
			count = MIN(rx_len, 3);
			data = BCM2835_SPI1->IO;

			if (rbuf != NULL) {
				switch (count) {
				case 3:
					*rx++ = (char)((data >> 16) & 0xFF);
					/*@fallthrough@*/
					/* no break */
				case 2:
					*rx++ = (char)((data >> 8) & 0xFF);
					/*@fallthrough@*/
					/* no break */
				case 1:
					*rx++ = (char)((data >> 0) & 0xFF);
				}
			}

			rx_len -= count;
		}

		while (!(BCM2835_SPI1->STAT & BCM2835_AUX_SPI_STAT_BUSY) && (rx_len > 0)) {
			count = MIN(rx_len, 3);
			data = BCM2835_SPI1->IO;

			if (rbuf != NULL) {
				switch (count) {
				case 3:
					*rx++ = (char)((data >> 16) & 0xFF);
					/*@fallthrough@*/
					/* no break */
				case 2:
					*rx++ = (char)((data >> 8) & 0xFF);
					/*@fallthrough@*/
					/* no break */
				case 1:
					*rx++ = (char)((data >> 0) & 0xFF);
				}
			}

			rx_len -= count;
		}
	}
}

/**
 *
 * @param buf
 * @param len
 */
void bcm2835_aux_spi_transfern(char *buf, uint32_t len) {
	bcm2835_aux_spi_transfernb(buf, buf, len);
}
