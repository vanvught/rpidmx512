/**
 * @file json_config_getdirectory.cpp
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "http/json_infos.h"

namespace json::config
{
uint32_t GetDirectory(char* out_buffer, uint32_t out_buffer_size)
{
    uint32_t total = 0;

    total += static_cast<uint32_t>(snprintf(out_buffer + total, out_buffer_size - total, "{\"files\":{"));

    for (size_t i = 0; i < kFileInfosSize; ++i)
    {
        const auto& entry = kFileInfos[i];
        if ((entry.label != nullptr) && (entry.label[0] != '\0'))
        {
            total += static_cast<uint32_t>(snprintf(out_buffer + total, out_buffer_size - total, "\"%s\":\"%s\"%s", entry.name, entry.label, (i + 1 < kFileInfosSize) ? "," : ""));
        }
    }

    if (out_buffer[total - 1] == ',')
    {
        out_buffer[total - 1] = '}';
    }
    else
    {
        out_buffer[total++] = '}';
    }

    out_buffer[total++] = '}';
    return total;
}
} // namespace json::config
