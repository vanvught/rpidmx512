/**
 * @file json_format_helpers.h
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

#ifndef JSON_JSON_FORMAT_HELPERS_H_
#define JSON_JSON_FORMAT_HELPERS_H_

#include <cstdio>
#include <cstdint>

namespace format
{
constexpr size_t kFloatBufferSize = 8;   // For "%.2f", "%.1f"
constexpr size_t kOffsetBufferSize = 12; // For timezone offsets e.g. "+01:00"

[[nodiscard]] inline const char* FormatFloat(float value, char (&buf)[kFloatBufferSize], const char* fmt = "%.2f")
{
    snprintf(buf, sizeof(buf) - 1, fmt, value);
    return buf;
}

[[nodiscard]] inline const char* FormatUtcOffset(int32_t hours, uint32_t minutes, char (&buf)[kOffsetBufferSize])
{
    if (hours == 0)
    {
        snprintf(buf, sizeof(buf) - 1, "%.2d:%.2u", hours, minutes);
    }
    else
    {
        snprintf(buf, sizeof(buf) - 1, "%c%.2d:%.2u", hours < 0 ? '-' : '+', hours, minutes);
    }
    return buf;
}

} // namespace format

#endif  // JSON_JSON_FORMAT_HELPERS_H_
