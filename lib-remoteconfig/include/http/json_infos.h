/**
 * @file json_infos.h
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

#ifndef HTTP_JSON_INFOS_H_
#define HTTP_JSON_INFOS_H_

#include <cstddef>
#include <cstdint>

#include "http/info_utils.h"

namespace json
{

struct Info
{
    constexpr Info(uint32_t (*get_in)(char*, uint32_t), void (*set_in)(const char*, uint32_t), void (*del_in)(const char*, uint32_t), const char* name_in,
                   uint8_t length_in, uint32_t hash_in, const char* label_in) noexcept
        : get(get_in), set(set_in), del(del_in), name(name_in), length(length_in), hash(hash_in), label(label_in)
    {
    }

    uint32_t (*get)(char*, uint32_t);
    void (*set)(const char*, uint32_t);
    void (*del)(const char*, uint32_t);
    const char* name;
    uint8_t length;
    uint32_t hash;
    const char* label;
};

template <size_t N>
constexpr Info MakeJsonFileInfo(uint32_t (*get)(char*, uint32_t), void (*set)(const char*, uint32_t), void (*del)(const char*, uint32_t), const char (&str)[N],
                                uint32_t hash, const char* label)
{
    return Info{get, set, del, str, static_cast<uint8_t>(N - 1), hash, label};
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

extern const Info kFileInfos[];
extern const size_t kFileInfosSize;

inline void PrintInfos()
{
    PrintInfosGeneric(kFileInfos, kFileInfosSize);
}

inline int32_t GetFileIndex(const char* name)
{
    return GetFileIndexGeneric(kFileInfos, kFileInfosSize, name);
}

inline void CheckHashCollisions()
{
    CheckHashCollisionsGeneric(kFileInfos, kFileInfosSize);
}
} // namespace json

#endif // HTTP_JSON_INFOS_H_
