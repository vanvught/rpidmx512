/**
 * @file debug_printbits.h
 *
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef COMMON_DEBUG_DEBUG_PRINTBITS_H_
#define COMMON_DEBUG_DEBUG_PRINTBITS_H_

#include <cstdint>
#include <cstdio>

#include "firmware/debug/debug_config.h"

namespace debug {
inline void PrintBits([[maybe_unused]] uint32_t value) {
    if constexpr (!config::kTraceEnabled) {
        return;
    }

    printf("%.8x ", static_cast<unsigned>(value));

    for (int bit_number = 31; bit_number >= 0; --bit_number) {
        const auto kMask = uint32_t{1} << bit_number;

        if ((value & kMask) != 0U) {
            printf("%d ", bit_number);
        }
    }

    putchar('\n');
}
} // namespace debug

#endif // COMMON_DEBUG_DEBUG_PRINTBITS_H_
