/**
 * @file h3_spi.h
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

#ifndef H3_SPI_H_
#define H3_SPI_H_

#include <stdbool.h>

typedef enum H3_SPI_BIT_ORDER {
	H3_SPI_BIT_ORDER_LSBFIRST = 0,	///< LSB First
	H3_SPI_BIT_ORDER_MSBFIRST = 1	///< MSB First
} h3_spi_bit_order_t;

typedef enum H3_SPI_MODE {
	H3_SPI_MODE0 = 0,	///< CPOL = 0, CPHA = 0
	H3_SPI_MODE1 = 1,	///< CPOL = 0, CPHA = 1
	H3_SPI_MODE2 = 2,	///< CPOL = 1, CPHA = 0
	H3_SPI_MODE3 = 3	///< CPOL = 1, CPHA = 1
} h3_spi_mode_t;

typedef enum H3_SPI_CS {
	H3_SPI_CS0 = 0,		///< Chip Select 0
	H3_SPI_CS1 = 1,		///< Chip Select 1
	H3_SPI_CS2 = 2,		///< Chip Select 2
	H3_SPI_CS3 = 3,		///< Chip Select 3
	H3_SPI_CS_NONE = 4	///< No CS, control it yourself
} h3_spi_chip_select_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void h3_spi_begin(void);
extern void h3_spi_end(void);

extern void h3_spi_setBitOrder(h3_spi_bit_order_t bit_order);
extern void h3_spi_set_speed_hz(uint64_t speed_hz);
extern void h3_spi_setDataMode(h3_spi_mode_t mode);
extern void h3_spi_chipSelect(h3_spi_chip_select_t chip_select);
extern void h3_spi_setChipSelectPolarity(h3_spi_chip_select_t chip_select, uint8_t polarity);

extern uint8_t h3_spi_transfer(uint8_t data);
extern void h3_spi_transfernb(char *tx_buffer, /*@null@*/char *rx_buffer, uint32_t data_length);
extern void h3_spi_transfern(char *tx_buffer, uint32_t data_length);

extern void h3_spi_write(uint16_t data);
extern void h3_spi_writenb(const char *tx_buffer, uint32_t data_length);

extern void h3_spi_set_ws28xx_mode(bool off_on);
extern bool h3_spi_get_ws28xx_mode(void);

extern void h3_spi_setClockDivider(uint16_t divider);	// Backwards compatibility with Raspberry Pi

#ifdef __cplusplus
}
#endif

//FIXME Remove
//TODO Backwards compatibility with the Raspberry Pi API's
#define BCM2835_CORE_CLK_HZ 					250000000

#define bcm2835_aux_spi_begin()						(void)0
#define bcm2835_aux_spi_setClockDivider(__p)		(void)0
#define bcm2835_aux_spi_write(__p)					(void)0
#define bcm2835_aux_spi_writenb(__p1,__p2)			(void)0
#define bcm2835_aux_spi_transfern(__p1,__p2)		(void)0
#define bcm2835_aux_spi_transfernb(__p1,__p2,__p3)	(void)0
static inline uint32_t bcm2835_aux_spi_CalcClockDivider(uint32_t __p) { return 0;}

#define bcm2835_spi_begin						h3_spi_begin
#define bcm2835_spi_setClockDivider				h3_spi_setClockDivider
#define bcm2835_spi_set_speed_hz				h3_spi_set_speed_hz
#define bcm2835_spi_setDataMode(__p)			h3_spi_setDataMode((h3_spi_mode_t)__p)
#define bcm2835_spi_setChipSelectPolarity		h3_spi_setChipSelectPolarity
#define bcm2835_spi_chipSelect(__p)				h3_spi_chipSelect((h3_spi_chip_select_t)__p)
#define bcm2835_spi_write						h3_spi_write
#define bcm2835_spi_writenb						h3_spi_writenb
#define bcm2835_spi_transfer					h3_spi_transfer
#define bcm2835_spi_transfern					h3_spi_transfern
#define bcm2835_spi_transfernb					h3_spi_transfernb

#define BCM2835_SPI_BIT_ORDER_MSBFIRST			H3_SPI_BIT_ORDER_MSBFIRST

#define BCM2835_SPI_MODE0						H3_SPI_MODE0
#define BCM2835_SPI_MODE3						H3_SPI_MODE3

#define BCM2835_SPI_CS0							H3_SPI_CS0
#define BCM2835_SPI_CS_NONE						H3_SPI_CS_NONE

/*
 * The Raspberry Pi SPI runs at APB clock speed, which is equivalent to core clock speed, 250 MHz
 */
typedef enum SPIClockDivider {
	BCM2835_SPI_CLOCK_DIVIDER_65536 = 65536,	///< 65536 = 262.144us = 3.814697260kHz
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
	BCM2835_SPI_CLOCK_DIVIDER_1 = 1,			///< 1 = 262.144us = 3.814697260kHz, same as 0/65536
	BCM2835_SPI_CLOCK_DIVIDER_0 = 0,			///< 0 = 262.144us = 3.814697260kHz, same as 1/65536
} bcm2835SPIClockDivider;

#endif /* H3_SPI_H_ */
