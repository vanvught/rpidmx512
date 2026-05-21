/**
 * @file info_utils.h
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

#ifndef HTTP_INFO_UTILS_H_
#define HTTP_INFO_UTILS_H_

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>

#include "common/utils/utils_hash.h"

template <typename InfoT> int32_t GetFileIndexGeneric(const InfoT* infos, size_t size, const char* name) {
    const uint32_t kLen = static_cast<uint32_t>(std::strlen(name));
    const uint32_t kHash = Fnv1a32Runtime(name, kLen);

    for (uint32_t i = 0; i < size; ++i) {
        if (kHash == infos[i].hash) {
            return static_cast<int32_t>(i);
        }
    }

    return -1;
}

template <typename InfoT> void CheckHashCollisionsGeneric(const InfoT* infos, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = i + 1; j < size; ++j) {
            if (infos[i].hash == infos[j].hash) {
                printf("Hash collision between: %s and %s\n", infos[i].name, infos[j].name);
            }
        }
    }
}

#endif // HTTP_INFO_UTILS_H_
