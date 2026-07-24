/**
 * @file scenes.cpp
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

#include <cstdint>
#include <cassert>

#include "dmxnode_scenes.h"
#include "flashcode.h"
#include "dmxnode_debug.h"

namespace dmxnode::scenes {

static bool s_is_detected;
static uint32_t s_offset_base;

static bool IsDetected() {
    DMXNODE_DEBUG_ENTRY();
    DMXNODE_DEBUG_PRINTF("isDetected=%d", s_is_detected);

    if (!s_is_detected) {
        if (!FlashCode::Get()->IsDetected()) {
            DMXNODE_DEBUG_EXIT();
            return false;
        }

        const auto kEraseSize = FlashCode::Get()->GetSectorSize();
        assert(kEraseSize <= dmxnode::scenes::kBytesNeeded);
        const auto kPages = 1 + dmxnode::scenes::kBytesNeeded / kEraseSize;

        DMXNODE_DEBUG_PRINTF("Bytes needed=%u, kEraseSize=%u, kPages=%u", dmxnode::scenes::kBytesNeeded, kEraseSize, kPages);

        assert(((kPages + 1) * kEraseSize) <= FlashCode::Get()->GetSize());

        s_offset_base = FlashCode::Get()->GetSize() - ((kPages + 1) * kEraseSize);

        DMXNODE_DEBUG_PRINTF("nOffsetBase=%p", s_offset_base);
    }

    DMXNODE_DEBUG_EXIT();
    return true;
}

void WriteStart() {
    DMXNODE_DEBUG_ENTRY();
    DMXNODE_DEBUG_PRINTF("isDetected=%d", s_is_detected);

    if (!IsDetected()) {
        DMXNODE_DEBUG_EXIT();
        return;
    }

    flashcode::Result result;
    uint32_t timeout = 0;

    while (!FlashCode::Get()->Erase(s_offset_base, FlashCode::Get()->GetSectorSize(), result)) {
        timeout++;
    }

    s_is_detected = (result == flashcode::Result::kOk);

    DMXNODE_DEBUG_PRINTF("result=%d, s_is_detected=%d, timeout=%u", result, s_is_detected, timeout);
    DMXNODE_DEBUG_EXIT();
}

void Write(uint32_t port_index, const uint8_t* data) {
    DMXNODE_DEBUG_ENTRY();
    assert(port_index < dmxnode::kMaxPorts);
    assert(data != nullptr);

    if (!s_is_detected) {
        DMXNODE_DEBUG_EXIT();
        return;
    }

    const auto kOffset = s_offset_base + (port_index * dmxnode::kUniverseSize);

    DMXNODE_DEBUG_PRINTF("s_offset_base=%p, kOffset=%p", s_offset_base, kOffset);

    flashcode::Result result;
    uint32_t timeout = 0;

    while (!FlashCode::Get()->Write(kOffset, dmxnode::kUniverseSize, data, result)) {
        timeout++;
    }

    DMXNODE_DEBUG_PRINTF("nResult=%d, nTimeout=%u", result, timeout);

    assert(result == flashcode::Result::kOk);

    DMXNODE_DEBUG_EXIT();
}

void WriteEnd() {
    DMXNODE_DEBUG_ENTRY();

    // No code needed here

    DMXNODE_DEBUG_EXIT();
}

void ReadStart() {
    DMXNODE_DEBUG_ENTRY();
    DMXNODE_DEBUG_PRINTF("s_is_detected=%d", s_is_detected);

    if (!IsDetected()) {
        DMXNODE_DEBUG_EXIT();
        return;
    }

    s_is_detected = true;

    DMXNODE_DEBUG_EXIT();
}

void Read(uint32_t port_index, uint8_t* data) {
    DMXNODE_DEBUG_ENTRY();
    assert(port_index < dmxnode::kMaxPorts);
    assert(data != nullptr);

    if (!s_is_detected) {
        DMXNODE_DEBUG_EXIT();
        return;
    }

    const auto kOffset = s_offset_base + (port_index * dmxnode::kUniverseSize);

    DMXNODE_DEBUG_PRINTF("nOffsetBase=%p, nOffset=%p", s_offset_base, kOffset);

    flashcode::Result result;
    uint32_t timeout = 0;

    while (!FlashCode::Get()->Read(kOffset, dmxnode::kUniverseSize, data, result)) {
        timeout++;
    }

    DMXNODE_DEBUG_PRINTF("result=%d, timeout=%u", result, timeout);

    assert(result == flashcode::Result::kOk);

    DMXNODE_DEBUG_EXIT();
}

void ReadEnd() {
    DMXNODE_DEBUG_ENTRY();

    // No code needed here

    DMXNODE_DEBUG_EXIT();
}
} // namespace dmxnode::scenes
