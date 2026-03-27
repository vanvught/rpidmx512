/**
 * @file html_infos.h
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

#ifndef HTTP_HTML_INFOS_H_
#define HTTP_HTML_INFOS_H_

#include <cstddef>
#include <cstdint>

#include "http/info_utils.h"

namespace html
{

struct Info
{
    constexpr Info(const char* name_in, uint8_t length_in, uint32_t hash_in, const char* label_in) noexcept
        : name(name_in), length(length_in), hash(hash_in), label(label_in)
    {
    }

    const char* name;
    uint8_t length;
    uint32_t hash;
    const char* label;
};

template <size_t N> constexpr Info MakeHtmlInfo(const char (&str)[N], uint32_t hash, const char* label)
{
    return Info{str, static_cast<uint8_t>(N - 1), hash, label};
}

constexpr bool HasUniqueHashes(const Info* entries, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        for (size_t j = i + 1; j < count; ++j)
        {
            if (entries[i].hash == entries[j].hash)
            {
                return false;
            }
        }
    }
    return true;
}

extern const Info kHtmlInfos[];
extern const size_t kHtmlInfosSize;

inline void PrintInfos()
{
    PrintInfosGeneric(kHtmlInfos, kHtmlInfosSize);
}

inline int32_t GetFileIndex(const char* name)
{
    return GetFileIndexGeneric(kHtmlInfos, kHtmlInfosSize, name);
}

inline void CheckHashCollisions()
{
    CheckHashCollisionsGeneric(kHtmlInfos, kHtmlInfosSize);
}
} // namespace html

#endif // HTTP_HTML_INFOS_H_
