/**
 * @file debug_debug.h
 *
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef FIRMWARE_DEBUG_DEBUG_H_
#define FIRMWARE_DEBUG_DEBUG_H_

#if !defined(NDEBUG)
#include <cstdio>
#include <source_location>

#define DEBUG_ENTRY()                                                                                      \
    do {                                                                                                   \
        const std::source_location loc = std::source_location::current();                                  \
        printf("-> %s(%u):%s\n", loc.file_name(), static_cast<unsigned>(loc.line()), loc.function_name()); \
    } while (0)

#define DEBUG_EXIT()                                                                                       \
    do {                                                                                                   \
        const std::source_location loc = std::source_location::current();                                  \
        printf("<- %s(%u):%s\n", loc.file_name(), static_cast<unsigned>(loc.line()), loc.function_name()); \
    } while (0)

#define DEBUG_PRINTF(fmt, ...)                                                                                                                 \
    do {                                                                                                                                       \
        const std::source_location loc = std::source_location::current();                                                                      \
        printf("   %s(%u):%s: " fmt "\n", loc.file_name(), static_cast<unsigned>(loc.line()), loc.function_name() __VA_OPT__(, ) __VA_ARGS__); \
    } while (0)

#define DEBUG_PUTS(msg)            \
    do {                           \
        DEBUG_PRINTF("%s", (msg)); \
    } while (0)

#else

#define DEBUG_ENTRY() \
    do {              \
    } while (0)

#define DEBUG_EXIT() \
    do {             \
    } while (0)

#define DEBUG_PRINTF(...) \
    do {                  \
    } while (0)

#define DEBUG_PUTS(...) \
    do {                \
    } while (0)

#endif

#endif // FIRMWARE_DEBUG_DEBUG_H_
