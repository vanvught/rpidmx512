/**
 * @file debug_dump.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef COMMON_DEBUG_DEBUG_DUMP_H_
#define COMMON_DEBUG_DEBUG_DUMP_H_

#include <cstdint>
#include <cstdio>
#include <ctype.h>

#if defined(H3)
namespace uart0
{
int Printf(const char* fmt, ...);
}
#define printf uart0::Printf // NOLINT
#endif

namespace debug
{
namespace dump
{
inline constexpr uint32_t kCharsPerLine = 16;
}
#ifdef NDEBUG
inline void Dump([[maybe_unused]] const void* data, [[maybe_unused]] uint32_t size) {}
#else
inline void Dump(const void* data, uint32_t size)
{
    uint32_t chars = 0;
    const auto* p = reinterpret_cast<const uint8_t*>(data);

    printf("%p:%d\n", data, size);

    do
    {
        uint32_t chars_this_line = 0;

        printf("%04x ", chars);

        const auto* q = p;

        while ((chars_this_line < dump::kCharsPerLine) && (chars < size))
        {
            if (chars_this_line % 8 == 0)
            {
                printf(" ");
            }

            printf("%02x ", *p);

            chars_this_line++;
            chars++;
            p++;
        }

        auto chars_dot_line = chars_this_line;

        for (; chars_this_line < dump::kCharsPerLine; chars_this_line++)
        {
            if (chars_this_line % 8 == 0)
            {
                printf(" ");
            }
            printf("   ");
        }

        chars_this_line = 0;

        while (chars_this_line < chars_dot_line)
        {
            if (chars_this_line % 8 == 0)
            {
                printf(" ");
            }

            int ch = *q;
            if (isprint(ch))
            {
                printf("%c", ch);
            }
            else
            {
                printf(".");
            }

            chars_this_line++;
            q++;
        }

        puts("");

    } while (chars < size);
}
#endif
} // namespace debug

#endif /* COMMON_DEBUG_DEBUG_DUMP_H_ */
