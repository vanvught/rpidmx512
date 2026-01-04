/**
 * @file json_showfile_directory.cpp
 *
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
 
#include <cstdint>
#include <cstdio>

#include "showfile.h"

namespace json
{
uint32_t ShowFileDirectory(char* out_buffer, uint32_t out_buffer_size)
{
    const auto kBufferSize = out_buffer_size - 2U;
    auto length = static_cast<uint32_t>(snprintf(out_buffer, kBufferSize, "{\"shows\":["));

    for (uint32_t show_index = 0; show_index < ShowFile::Instance().GetShows(); show_index++)
    {
        const auto kShow = ShowFile::Instance().GetPlayerShowFile(show_index);
        if (kShow >= 0)
        {
            uint32_t file_size;
            if (ShowFile::Instance().GetShowFileSize(kShow, file_size))
            {
                const auto kSize = kBufferSize - length;
                const auto kCharacters = static_cast<uint32_t>(snprintf(&out_buffer[length], kSize, "{\"show\":%d,\"size\":%u},", kShow, file_size));

                if (kCharacters > kSize)
                {
                    break;
                }

                length += kCharacters;

                if (length >= kBufferSize)
                {
                    break;
                }
            }
        }
    }

    if (out_buffer[length - 1] == ',')
    {
        length--;
    }

    out_buffer[length++] = ']';
    out_buffer[length++] = '}';

    assert(length <= out_buffer_size);
    return length;
}
} // namespace json
