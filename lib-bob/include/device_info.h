/**
 * @file device_info.h
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

#ifndef DEVICE_INFO_H_
#define DEVICE_INFO_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum spi_cs {
	SPI_CS0,
	SPI_CS1,
	SPI_CS2
} spi_cs_t;

typedef struct _device_info {
	spi_cs_t chip_select;		///<
	uint8_t slave_address;		///<
	uint32_t speed_hz;			///<
	bool fast_mode;				///< I2C Fast Mode 400KHz
	struct _device_info_internal {
		uint8_t adc_channel;	///<
		uint16_t clk_div;		///<
		uint8_t count;			///<
	} internal;
} device_info_t;

#define DEVICE_INFO_I2C_DEFAULT			SPI_CS0, (uint8_t) 0, 0, true, { (uint8_t) 0, (uint16_t) 0}
#define DEVICE_INFO_SPI_CS0_DEFAULT		SPI_CS0, (uint8_t) 0, 0, false, { (uint8_t) 0, (uint16_t) 0}
#define DEVICE_INFO_SPI_CS1_DEFAULT		SPI_CS1, (uint8_t) 0, 0, false, { (uint8_t) 0, (uint16_t) 0}
#define DEVICE_INFO_SPI_CS2_DEFAULT		SPI_CS2, (uint8_t) 0, 0, false, { (uint8_t) 0, (uint16_t) 0}

#endif /* DEVICE_INFO_H_ */
