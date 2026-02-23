/**
 * @file h3_spi.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

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

void H3SpiBegin();
void H3SpiEnd();

void H3SpiSetBitOrder(h3_spi_bit_order_t bit_order);
void H3SpiSetSpeedHz(uint32_t speed_hz);
void H3SpiSetDataMode(uint8_t mode);
void H3SpiChipSelect(uint8_t chip_select);
void H3SpiSetChipSelectPolarity(uint8_t chip_select, uint8_t polarity);

extern uint8_t H3SpiTransfer(uint8_t data);
void H3SpiTransfernb(char *tx_buffer, char *rx_buffer, uint32_t data_length);
void H3SpiTransfern(char *tx_buffer, uint32_t data_length);

void H3SpiWrite(uint16_t data);
void H3SpiWritenb(const char *tx_buffer, uint32_t data_length);

/*
 * PixelOutput support
 */

void H3SpiSetPixelMode(bool off_on);
bool H3SpiGetPixelMode();

/*
 * DMA support
 */

const uint8_t *H3SpiDmaTxPrepare(uint32_t *data_length);
void H3SpiDmaTxStart(const uint8_t *tx_buffer, uint32_t length);
bool H3SpiDmaTxIsActive();

/*
 * bitbang support
 */

void H3BitbangSpiBegin();
void H3BitbangSpiChipSelect(uint8_t chip_select);
void H3BitbangSpiSetSpeedHz(uint32_t speed_hz);
void H3BitbangSpiSetDataMode(uint8_t mode);
void H3BitbangSpiWritenb(const char *tx_buffer, uint32_t data_length);
void H3BitbangSpiWrite(uint16_t data);
#endif /* H3_SPI_H_ */
