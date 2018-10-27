/**
 * @file spi_flash.c
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

#ifndef SPI_FLASH_H_
#define SPI_FLASH_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int spi_flash_probe(unsigned int cs, unsigned int max_hz, unsigned int spi_mode);

extern const char *spi_flash_get_name(void);
extern uint32_t spi_flash_get_size(void);
extern uint32_t spi_flash_get_sector_size(void);

extern int spi_flash_cmd_read_fast(uint32_t offset, size_t len, void *data);
extern int spi_flash_cmd_write_multi(uint32_t offset, size_t len, const void *buf);
extern int spi_flash_cmd_erase(uint32_t offset, size_t len);
extern int spi_flash_cmd_write_status(uint8_t sr);

#ifdef __cplusplus
}
#endif

#endif /* SPI_FLASH_H_ */
