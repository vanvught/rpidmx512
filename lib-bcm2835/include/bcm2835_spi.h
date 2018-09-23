/**
 * @file bcm2835_spi.h
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#define BCM2835_SPI0_CS_LEN_LONG   0x02000000	///< Enable Long data word in Lossi mode if DMA_LEN is set
#define BCM2835_SPI0_CS_DMA_LEN    0x01000000	///< Enable DMA mode in Lossi mode
#define BCM2835_SPI0_CS_CSPOL2     0x00800000	///< Chip Select 2 Polarity
#define BCM2835_SPI0_CS_CSPOL1     0x00400000	///< Chip Select 1 Polarity
#define BCM2835_SPI0_CS_CSPOL0     0x00200000	///< Chip Select 0 Polarity
#define BCM2835_SPI0_CS_RXF        0x00100000	///< RXF - RX FIFO Full
#define BCM2835_SPI0_CS_RXR        0x00080000	///< RXR RX FIFO needs Reading ( full)
#define BCM2835_SPI0_CS_TXD        0x00040000	///< TXD TX FIFO can accept Data
#define BCM2835_SPI0_CS_RXD        0x00020000	///< RXD RX FIFO contains Data
#define BCM2835_SPI0_CS_DONE       0x00010000	///< Done transfer Done
#define BCM2835_SPI0_CS_TE_EN      0x00008000	///< Unused
#define BCM2835_SPI0_CS_LMONO      0x00004000	///< Unused
#define BCM2835_SPI0_CS_LEN        0x00002000	///< LEN LoSSI enable
#define BCM2835_SPI0_CS_REN        0x00001000	///< REN Read Enable
#define BCM2835_SPI0_CS_ADCS       0x00000800	///< ADCS Automatically Deassert Chip Select
#define BCM2835_SPI0_CS_INTR       0x00000400	///< INTR Interrupt on RXR
#define BCM2835_SPI0_CS_INTD       0x00000200	///< INTD Interrupt on Done
#define BCM2835_SPI0_CS_DMAEN      0x00000100	///< DMAEN DMA Enable
#define BCM2835_SPI0_CS_TA         0x00000080	///< Transfer Active
#define BCM2835_SPI0_CS_CSPOL      0x00000040	///< Chip Select Polarity
#define BCM2835_SPI0_CS_CLEAR      0x00000030	///< Clear FIFO Clear RX and TX
#define BCM2835_SPI0_CS_CLEAR_RX   0x00000020	///< Clear FIFO Clear RX
#define BCM2835_SPI0_CS_CLEAR_TX   0x00000010	///< Clear FIFO Clear TX
#define BCM2835_SPI0_CS_CPOL      	0x00000008	///< Clock Polarity
#define BCM2835_SPI0_CS_CPHA      	0x00000004	///< Clock Phase
#define BCM2835_SPI0_CS_CS        	0x00000003	///< Chip Select

#define BCM2835_SPI0_FIFO			0x0004		///< SPI Master TX and RX FIFOs

#ifdef __ASSEMBLY__
#else
#include <stdint.h>

#include "bcm2835.h"
#include "bcm2835_gpio.h"

/// \ref BCM2835_SPI_BIT_ORDER_MSBFIRST is the only one supported by SPI0
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

// The Raspberry Pi SPI runs at APB clock speed, which is equivalent to core clock speed, 250 MHz
// This can be divided by any even number from 2 to 65536 for the desired speed.
// The datasheet specifies that the divisor must be a power of two, but this is incorrect.
// Odd numbers are rounded down, and 0 (or 1) is equivalent to 65536.
// A divisor smaller than 2 is therefore impossible.

typedef enum {
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

#define BCM2835_SPI_CLOCK_MIN	4000			///< 4kHz
#define BCM2835_SPI_CLOCK_MAX	125000000		///< 125Mhz

#ifdef __cplusplus
extern "C" {
#endif

extern void bcm2835_spi_begin(void);
extern void bcm2835_spi_end(void);
extern void bcm2835_spi_setBitOrder(const uint8_t);

/*@unused@*/inline static void bcm2835_spi_setClockDivider(uint16_t divider) {
	BCM2835_SPI0->CLK = (uint32_t) divider;
}

/*@unused@*/inline static void bcm2835_spi_set_speed_hz(uint32_t speed_hz) {
	uint16_t divider = (uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / speed_hz);
	divider &= 0xFFFE;
	bcm2835_spi_setClockDivider(divider);
}

/*@unused@*/inline static void bcm2835_spi_setDataMode(uint8_t mode) {
	// Mask in the CPO and CPHA bits of CS
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, mode << 2, BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA);
}

/*@unused@*/inline static void bcm2835_spi_chipSelect(uint8_t cs) {
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, cs, BCM2835_SPI0_CS_CS);
}

extern void bcm2835_spi_setChipSelectPolarity(uint8_t, uint8_t);
extern void bcm2835_spi_transfernb(char*, /*@null@*/char*, uint32_t);
extern void bcm2835_spi_transfern(char*, uint32_t);
extern void bcm2835_spi_writenb(const char*, uint32_t);
extern void bcm2835_spi_write(uint16_t data);
extern uint8_t bcm2835_spi_transfer(uint8_t);

#ifdef __cplusplus
}
#endif

#endif

#endif /* BCM2835_SPI_H_ */
