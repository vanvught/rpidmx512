/**
 * @file spi_flash.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstddef>

int spi_flash_probe(unsigned int cs, unsigned int max_hz, unsigned int spi_mode);

const char *spi_flash_get_name();
uint32_t spi_flash_get_size();
uint32_t spi_flash_get_sector_size();

int spi_flash_cmd_read_fast(uint32_t offset, size_t len, uint8_t *data);
int spi_flash_cmd_write_multi(uint32_t offset, size_t len, const uint8_t *buf);
int spi_flash_cmd_erase(uint32_t offset, size_t len);
int spi_flash_cmd_write_status(uint8_t sr);

#endif /* SPI_FLASH_H_ */
