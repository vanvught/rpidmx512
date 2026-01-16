/**
 * @file flashcode.cpp
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "flashcode.h"
#include "spi/spi_flash.h"
 #include "firmware/debug/debug_debug.h"

using flashcode::Result;

FlashCode::FlashCode()
{
    DEBUG_ENTRY();
    assert(s_this == nullptr);
    s_this = this;

    if (!spi_flash_probe())
    {
        DEBUG_PUTS("No SPI flash chip");
    }
    else
    {
        printf("Detected %s with sector size %d total %d bytes\n", spi_flash_get_name(), static_cast<unsigned int>(spi_flash_get_sector_size()),
               static_cast<unsigned int>(spi_flash_get_size()));
        detected_ = true;
    }

    DEBUG_EXIT();
}

FlashCode::~FlashCode()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

const char* FlashCode::GetName() const
{
    return spi_flash_get_name();
}

uint32_t FlashCode::GetSize() const
{
    return spi_flash_get_size();
}

uint32_t FlashCode::GetSectorSize() const
{
    return spi_flash_get_sector_size();
}

bool FlashCode::Read(uint32_t offset, uint32_t length, uint8_t* buffer, flashcode::Result& result)
{
    DEBUG_ENTRY();

    result = spi_flash_cmd_read_fast(offset, length, buffer) ? Result::kOk : Result::kError;

    DEBUG_PRINTF("result=%d", static_cast<int>(result));
    DEBUG_EXIT();
    return true;
}

bool FlashCode::Erase(uint32_t offset, uint32_t length, flashcode::Result& result)
{
    DEBUG_ENTRY();

    result = spi_flash_cmd_erase(offset, length) ? Result::kOk : Result::kError;

    DEBUG_PRINTF("result=%d", static_cast<int>(result));
    DEBUG_EXIT();
    return true;
}

bool FlashCode::Write(uint32_t offset, uint32_t length, const uint8_t* buffer, flashcode::Result& result)
{
    DEBUG_ENTRY();

    result = spi_flash_cmd_write_multi(offset, length, buffer) ? Result::kOk : Result::kError;

    DEBUG_PRINTF("result=%d", static_cast<int>(result));
    DEBUG_EXIT();
    return true;
}
