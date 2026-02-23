/**
 * @file hal_spi.h
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef HAL_SPI_H_
#define HAL_SPI_H_

#include "hal_api.h"

#if defined(__linux__) || defined(__APPLE__)
#include "linux/hal_api.h"
#include "linux/hal_spi.h"
#elif defined(H3)
#include "h3/hal_api.h"
#include "h3/hal_spi.h"
#elif defined(GD32)
#include "gd32/hal_api.h"
#include "gd32/hal_spi.h"
#else
#include "rpi/hal_api.h"
#include "rpi/hal_spi.h"
#endif

#ifdef __cplusplus
#include <cstdint>

class HAL_SPI
{
    void Setup()
    {
        FUNC_PREFIX(SpiChipSelect(chip_select_));
        FUNC_PREFIX(SpiSetDataMode(mode_));
        FUNC_PREFIX(SpiSetSpeedHz(speed_hz_));
    }

   public:
    HAL_SPI(uint8_t chip_select, uint32_t speed_hz, uint8_t mode = 0) : speed_hz_(speed_hz), chip_select_(chip_select), mode_(mode & 0x3) { FUNC_PREFIX(SpiBegin()); }

    void Write(const char* data, uint32_t length, bool do_setup = true)
    {
        if (do_setup)
        {
            Setup();
        }
        FUNC_PREFIX(SpiWritenb(data, length));
    }

    void Write(uint16_t data, bool do_setup = true)
    {
        if (do_setup)
        {
            Setup();
        }
        FUNC_PREFIX(SpiWrite(data));
    }

    void WriteRead(char* data, uint32_t length, bool do_setup = true)
    {
        if (do_setup)
        {
            Setup();
        }
        FUNC_PREFIX(SpiTransfern(data, length));
    }

   private:
    uint32_t speed_hz_;
    uint8_t chip_select_;
    uint8_t mode_;
};
#endif

#endif  // HAL_SPI_H_
