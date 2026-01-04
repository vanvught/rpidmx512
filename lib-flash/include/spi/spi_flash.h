/**
 * @file spi_flash.h
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

#ifndef SPI_SPI_FLASH_H_
#define SPI_SPI_FLASH_H_

#include <cstdint>

namespace spi::flash
{
inline constexpr uint32_t PAGE_SIZE = 256;
inline constexpr uint32_t SECTOR_SIZE = 4096;
} // namespace spi::flash

bool spi_flash_probe();

const char* spi_flash_get_name();
uint32_t spi_flash_get_size();

inline uint32_t spi_flash_get_sector_size()
{
    return spi::flash::SECTOR_SIZE;
}

bool spi_flash_cmd_read_fast(uint32_t offset, uint32_t length, uint8_t* data);
bool spi_flash_cmd_write_multi(uint32_t offset, uint32_t length, const uint8_t* buffer);
bool spi_flash_cmd_erase(uint32_t offset, uint32_t length);
bool spi_flash_cmd_write_status(uint8_t sr);

#endif  // SPI_SPI_FLASH_H_
