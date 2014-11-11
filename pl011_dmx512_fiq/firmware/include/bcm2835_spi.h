/**
 * @file bcm2835_spi.h
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

#ifndef BCM2835_SPI_H_
#define BCM2835_SPI_H_

#include <stdint.h>

typedef enum {
	BCM2835_SPI_BIT_ORDER_LSBFIRST = 0,	///< LSB First
	BCM2835_SPI_BIT_ORDER_MSBFIRST = 1	///< MSB First
} bcm2835SPIBitOrder;

typedef enum {
	BCM2835_SPI_MODE0 = 0,	///< CPOL = 0, CPHA = 0
	BCM2835_SPI_MODE1 = 1,	///< CPOL = 0, CPHA = 1
	BCM2835_SPI_MODE2 = 2,	///< CPOL = 1, CPHA = 0
	BCM2835_SPI_MODE3 = 3	///< CPOL = 1, CPHA = 1
} bcm2835SPIMode;

typedef enum {
	BCM2835_SPI_CS0 	= 0,	///< Chip Select 0
	BCM2835_SPI_CS1		= 1,	///< Chip Select 1
	BCM2835_SPI_CS2		= 2,	///< Chip Select 2 (ie pins CS1 and CS2 are asserted)
	BCM2835_SPI_CS_NONE = 3		///< No CS, control it yourself
} bcm2835SPIChipSelect;

typedef enum {
	BCM2835_SPI_CLOCK_DIVIDER_65536 = 0,		///< 65536 = 262.144us = 3.814697260kHz
	BCM2835_SPI_CLOCK_DIVIDER_32768 = 32768,	///< 32768 = 131.072us = 7.629394531kHz
	BCM2835_SPI_CLOCK_DIVIDER_16384 = 16384,	///< 16384 = 65.536us = 15.25878906kHz
	BCM2835_SPI_CLOCK_DIVIDER_8192 = 8192,		///< 8192 = 32.768us = 30/51757813kHz
	BCM2835_SPI_CLOCK_DIVIDER_4096 = 4096,		///< 4096 = 16.384us = 61.03515625kHz
	BCM2835_SPI_CLOCK_DIVIDER_2500 = 2500,		///< 2500 = 10us = 100 kHz
	BCM2835_SPI_CLOCK_DIVIDER_2048 = 2048,		///< 2048 = 8.192us = 122.0703125kHz
	BCM2835_SPI_CLOCK_DIVIDER_1024 = 1024,		///< 1024 = 4.096us = 244.140625kHz
	BCM2835_SPI_CLOCK_DIVIDER_512 = 512,		///< 512 = 2.048us = 488.28125kHz
	BCM2835_SPI_CLOCK_DIVIDER_256 = 256,		///< 256 = 1.024us = 976.5625MHz
	BCM2835_SPI_CLOCK_DIVIDER_128 = 128,		///< 128 = 512ns = = 1.953125MHz
	BCM2835_SPI_CLOCK_DIVIDER_64 = 64,			///< 64 = 256ns = 3.90625MHz
	BCM2835_SPI_CLOCK_DIVIDER_32 = 32,			///< 32 = 128ns = 7.8125MHz
	BCM2835_SPI_CLOCK_DIVIDER_16 = 16,			///< 16 = 64ns = 15.625MHz
	BCM2835_SPI_CLOCK_DIVIDER_8 = 8,			///< 8 = 32ns = 31.25MHz
	BCM2835_SPI_CLOCK_DIVIDER_4 = 4,			///< 4 = 16ns = 62.5MHz
	BCM2835_SPI_CLOCK_DIVIDER_2 = 2,			///< 2 = 8ns = 125MHz, fastest you can get
	BCM2835_SPI_CLOCK_DIVIDER_1 = 1,			///< 0 = 262.144us = 3.814697260kHz, same as 0/65536
} bcm2835SPIClockDivider;

extern void bcm2835_spi_begin(void);
extern void bcm2835_spi_end(void);
extern void bcm2835_spi_setBitOrder(const uint8_t);
extern void bcm2835_spi_setClockDivider(const uint16_t);
extern void bcm2835_spi_setDataMode(const uint8_t);
extern void bcm2835_spi_chipSelect(const uint8_t);
extern void bcm2835_spi_setChipSelectPolarity(const uint8_t, const uint8_t);
extern void bcm2835_spi_transfernb(char*, char*, const uint32_t);
extern void bcm2835_spi_transfern(char* buf, const uint32_t);
extern void bcm2835_spi_writenb(char* tbuf, const uint32_t);
extern void bcm2835_spi_write(const uint16_t data);

#endif /* BCM2835_SPI_H_ */
