/**
 * @file scenes.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>

#include "spi/spi_flash.h"
#include "dmxnode.h"
 #include "firmware/debug/debug_debug.h"

namespace dmxnode::scenes
{
static bool s_has_flash;
static uint32_t s_offset_base;

static bool CheckHaveFlash()
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("s_hasFlash=%d", s_has_flash);

    if (!s_has_flash)
    {
        if (!spi_flash_probe())
        {
            DEBUG_EXIT();
            return false;
        }

        const auto kEraseSize = spi_flash_get_sector_size();
        assert(kEraseSize <= dmxnode::scenes::kBytesNeeded);
        const auto kPages = 1 + dmxnode::scenes::kBytesNeeded / kEraseSize;

        DEBUG_PRINTF("Bytes needed=%u, nEraseSize=%u, nPages=%u", dmxnode::scenes::kBytesNeeded, kEraseSize, kPages);

        assert(((kPages + 1) * kEraseSize) <= spi_flash_get_size());

        s_offset_base = spi_flash_get_size() - ((kPages + 1) * kEraseSize);

        DEBUG_PRINTF("nOffsetBase=%p", s_offset_base);
    }

    DEBUG_EXIT();
    return true;
}

void WriteStart()
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("s_hasFlash=%d", s_has_flash);

    if (!CheckHaveFlash())
    {
        DEBUG_EXIT();
        return;
    }

    s_has_flash = spi_flash_cmd_erase(s_offset_base, spi_flash_get_sector_size());

    DEBUG_PRINTF("s_hasFlash=%d", s_has_flash);
    DEBUG_EXIT();
}

void Write(uint32_t port_index, const uint8_t* data)
{
    DEBUG_ENTRY();
    assert(port_index < dmxnode::kMaxPorts);
    assert(data != nullptr);

    if (!s_has_flash)
    {
        DEBUG_EXIT();
        return;
    }

    const auto kOffset = s_offset_base + (port_index * dmxnode::kUniverseSize);

    DEBUG_PRINTF("s_offset_base=%p, kOffset=%p", s_offset_base, kOffset);

    spi_flash_cmd_write_multi(kOffset, dmxnode::kUniverseSize, data);

    DEBUG_EXIT();
}

void WriteEnd()
{
    DEBUG_ENTRY();

    // No code needed here

    DEBUG_EXIT();
}

void ReadStart()
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("s_has_flash=%d", s_has_flash);

    if (!CheckHaveFlash())
    {
        DEBUG_EXIT();
        return;
    }

    s_has_flash = true;

    DEBUG_EXIT();
}

void Read(uint32_t port_index, uint8_t* data)
{
    DEBUG_ENTRY();
    assert(port_index < dmxnode::kMaxPorts);
    assert(data != nullptr);

    if (!s_has_flash)
    {
        DEBUG_EXIT();
        return;
    }

    const auto kOffset = s_offset_base + (port_index * dmxnode::kUniverseSize);

    DEBUG_PRINTF("s_offset_base=%p, kOffset=%p", s_offset_base, kOffset);

    spi_flash_cmd_read_fast(kOffset, dmxnode::kUniverseSize, data);

    DEBUG_EXIT();
}

void ReadEnd()
{
    DEBUG_ENTRY();

    // No code needed here

    DEBUG_EXIT();
}
} // namespace dmxnode::scenes
