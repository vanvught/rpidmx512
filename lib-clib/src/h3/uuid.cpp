/**
 * @file uuid.cpp
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of thnDmxDataDirecte Software, and to permit persons to whom the Software is
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

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-funroll-loops")
#endif

#include <cstdint>
#include <cstring>
#include <uuid/uuid.h>

#include "h3_sid.h"

namespace hal
{
typedef union pcast32
{
    uuid_t uuid;
    uint8_t u8[16];
} _pcast32;

void UuidCopy(uuid_t out)
{
    _pcast32 cast;

    h3_sid_get_rootkey(&cast.u8[0]);

    cast.uuid[6] = static_cast<char>(0x40 | (cast.uuid[6] & 0xf));
    cast.uuid[8] = static_cast<char>(0x80 | (cast.uuid[8] & 0x3f));

    memcpy(out, cast.uuid, sizeof(uuid_t));
}
} // namespace hal
