/**
 * @file spi_lcd.h
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

#ifndef SPI_LCD_H_
#define SPI_LCD_H_

#include "spi/config.h"

#include "hal_spi.h"

namespace spi {
namespace lcd {

inline static void ms_delay(const uint32_t ms) {
	udelay(1000 * ms);
}

inline static void CS_Set() {
#if defined(SPI_LCD_HAVE_CS_GPIO)
	FUNC_PREFIX(gpio_set(SPI_LCD_CS_GPIO));
#endif
}

inline static void CS_Clear() {
#if defined(SPI_LCD_HAVE_CS_GPIO)
	FUNC_PREFIX(gpio_clr(SPI_LCD_CS_GPIO));
#endif
}

inline static void DC_Set() {
	FUNC_PREFIX(gpio_set(SPI_LCD_DC_GPIO));
}

inline static void DC_Clear() {
	FUNC_PREFIX(gpio_clr(SPI_LCD_DC_GPIO));
}

inline static void HW_Reset() {
#if defined (SPI_LCD_RST_GPIO)
	ms_delay(200);
	FUNC_PREFIX(gpio_clr(SPI_LCD_RST_GPIO));
	ms_delay(200);
	FUNC_PREFIX(gpio_set(SPI_LCD_RST_GPIO));
	ms_delay(200);
#endif
}

inline static void Write_Command(uint8_t data) {
	CS_Clear();
	DC_Clear();
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(&data), 1));
	CS_Set();
}

inline static void WriteData_Byte(uint8_t data) {
	CS_Clear();
	DC_Set();
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(&data), 1));
	CS_Set();
}

inline static void WriteData_Word(uint16_t data) {
	CS_Clear();
	DC_Set();
	FUNC_PREFIX(spi_write(data));
	CS_Set();
}

inline static void WriteData(uint8_t *pData, uint32_t nLength) {
	CS_Clear();
	DC_Set();
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(pData), nLength));
	CS_Set();
}

inline static void WriteDataStart(uint8_t *pData, uint32_t nLength) {
	CS_Clear();
	DC_Set();
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(pData), nLength));
}

inline static void WriteDataContinue(uint8_t *pData, uint32_t nLength) {
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(pData), nLength));
}

inline static void WriteDataEnd(uint8_t *pData, uint32_t nLength) {
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(pData), nLength));
	CS_Set();
}

}  // namespace lcd
}  // namespace spi

#endif /* SPI_LCD_H_ */
