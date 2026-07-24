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
#include "dmxnode_debug.h"

namespace dmxnode::scenes {
static bool s_has_flash;
static uint32_t s_offset_base;

static bool CheckHaveFlash() {
    DMXNODE_DEBUG_ENTRY();
    DMXNODE_DEBUG_PRINTF("s_hasFlash=%d", s_has_flash);

    if (!s_has_flash) {
        if (!spi_flash_probe()) {
            DMXNODE_DEBUG_EXIT();
            return false;
        }

        const auto kEraseSize = spi_flash_get_sector_size();
        assert(kEraseSize <= dmxnode::scenes::kBytesNeeded);
        const auto kPages = 1 + (dmxnode::scenes::kBytesNeeded / kEraseSize);

        DMXNODE_DEBUG_PRINTF("Bytes needed=%u, nEraseSize=%u, nPages=%u", dmxnode::scenes::kBytesNeeded, kEraseSize, kPages);

        assert(((kPages + 1) * kEraseSize) <= spi_flash_get_size());

        s_offset_base = spi_flash_get_size() - ((kPages + 1) * kEraseSize);

        DMXNODE_DEBUG_PRINTF("nOffsetBase=%p", s_offset_base);
    }

    DMXNODE_DEBUG_EXIT();
    return true;
}

void WriteStart() {
    DMXNODE_DEBUG_ENTRY();
    DMXNODE_DEBUG_PRINTF("s_hasFlash=%d", s_has_flash);

    if (!CheckHaveFlash()) {
        DMXNODE_DEBUG_EXIT();
        return;
    }

    s_has_flash = spi_flash_cmd_erase(s_offset_base, spi_flash_get_sector_size());

    DMXNODE_DEBUG_PRINTF("s_hasFlash=%d", s_has_flash);
    DMXNODE_DEBUG_EXIT();
}

void Write(uint32_t port_index, const uint8_t* data) {
    DMXNODE_DEBUG_ENTRY();
    assert(port_index < dmxnode::kMaxPorts);
    assert(data != nullptr);

    if (!s_has_flash) {
        DMXNODE_DEBUG_EXIT();
        return;
    }

    const auto kOffset = s_offset_base + (port_index * dmxnode::kUniverseSize);

    DMXNODE_DEBUG_PRINTF("s_offset_base=%p, kOffset=%p", s_offset_base, kOffset);

    spi_flash_cmd_write_multi(kOffset, dmxnode::kUniverseSize, data);

    DMXNODE_DEBUG_EXIT();
}

void WriteEnd() {
    DMXNODE_DEBUG_ENTRY();

    // No code needed here

    DMXNODE_DEBUG_EXIT();
}

void ReadStart() {
    DMXNODE_DEBUG_ENTRY();
    DMXNODE_DEBUG_PRINTF("s_has_flash=%d", s_has_flash);

    if (!CheckHaveFlash()) {
        DMXNODE_DEBUG_EXIT();
        return;
    }

    s_has_flash = true;

    DMXNODE_DEBUG_EXIT();
}

void Read(uint32_t port_index, uint8_t* data) {
    DMXNODE_DEBUG_ENTRY();
    assert(port_index < dmxnode::kMaxPorts);
    assert(data != nullptr);

    if (!s_has_flash) {
        DMXNODE_DEBUG_EXIT();
        return;
    }

    const auto kOffset = s_offset_base + (port_index * dmxnode::kUniverseSize);

    DMXNODE_DEBUG_PRINTF("s_offset_base=%p, kOffset=%u", reinterpret_cast<void*>(s_offset_base), static_cast<unsigned>(kOffset));

    spi_flash_cmd_read_fast(kOffset, dmxnode::kUniverseSize, data);

    DMXNODE_DEBUG_EXIT();
}

void ReadEnd() {
    DMXNODE_DEBUG_ENTRY();

    // No code needed here

    DMXNODE_DEBUG_EXIT();
}
} // namespace dmxnode::scenes
