/**
 * @file oscstring.h
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef OSCSTRING_H_
#define OSCSTRING_H_

#include <cstring>
#include <cstdint>

/*
 * OSC-string
 * A sequence of non-null ASCII characters followed by a null,
 * followed by 0-3 additional null characters to make the total number of bits a multiple of 32.
 */

namespace osc
{
namespace validate
{
inline constexpr int kInvalidSize = 1;
inline constexpr int kNotTerminated = 2;
inline constexpr int kNoneZeroInPadding = 3;
} // namespace validate
inline int StringValidate(const void* data, uint32_t size)
{
    uint32_t length = 0;
    auto* src = reinterpret_cast<const char*>(data);

    uint32_t i = 0;

    for (i = 0; i < size; ++i)
    {
        if (src[i] == '\0')
        {
            length = 4 * (i / 4 + 1);
            break;
        }
    }

    if (0 == length)
    {
        return -osc::validate::kNotTerminated;
    }

    if (length > size)
    {
        return -osc::validate::kInvalidSize;
    }

    for (; i < length; ++i)
    {
        if (src[i] != '\0')
        {
            return -osc::validate::kNoneZeroInPadding;
        }
    }

    return static_cast<int>(length);
}

inline unsigned StringSize(const char* string)
{
    return 4 * (strlen(string) / 4 + 1);
}
} // namespace osc

#endif  // OSCSTRING_H_
