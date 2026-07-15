/**
 * @file debug_stack.h
 *
 */
/* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef FIRMWARE_DEBUG_DEBUG_STACK_H_
#define FIRMWARE_DEBUG_DEBUG_STACK_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "timing.h"

extern unsigned char stack_low;
extern unsigned char _sp; // NOLINT

namespace debug::stack {
inline static constexpr uint32_t kMagicWord = 0xABCDABCD;

inline void Print() {
	static uint32_t s_used_bytes_previous;
    const auto* start = reinterpret_cast<uint32_t*>(&stack_low);
    const auto* end = reinterpret_cast<uint32_t*>(&_sp);
	assert(end > start);
    const auto kSize = static_cast<uint32_t>(end - start);

    const auto* ptr = start;

    while (ptr < end) {
        if (*ptr != kMagicWord) {
            break;
        }
        ptr++;
    }

    const auto kUsedBytes = static_cast<uint32_t>(4 * (end - ptr));
    const auto kFreeBytes = static_cast<uint32_t>(4 * (ptr - start));
    const auto kFreePct = (static_cast<uint32_t>(ptr - start) * 100U) / kSize;

    if (s_used_bytes_previous != kUsedBytes) {
        s_used_bytes_previous = kUsedBytes;

        if (kFreePct == 0) {
            printf("\x1b[31m");
        } else if (kFreePct == 1) {
            printf("\x1b[33m");
        } else {
            printf("\x1b[34m");
        }

#ifndef NDEBUG
        printf("Stack: Size %uKB, [%p:%p:%p], Used: %u, Free: %u [%u]", static_cast<unsigned>(kSize / (1024 / 4)), reinterpret_cast<const void *>(start), reinterpret_cast<const void *>(ptr), reinterpret_cast<const void *>(end), static_cast<unsigned>(kUsedBytes), static_cast<unsigned>(kFreeBytes), static_cast<unsigned>(kFreePct));
#else
        printf("Stack: Size %uKB, Used: %u, Free: %u", static_cast<unsigned>(kSize / (1024 / 4)), static_cast<unsigned>(kUsedBytes), static_cast<unsigned>(kFreeBytes));
#endif
        printf("\x1b[39m\n");
    }
}

inline void Run() {
	static uint32_t s_millis_previous;
    const auto kMillis = timing::Millis();
    if (kMillis - s_millis_previous >= 1000U) {
        s_millis_previous = kMillis;
        Print();
    }
}
} // namespace debug::stack

#endif // FIRMWARE_DEBUG_DEBUG_STACK_H_
