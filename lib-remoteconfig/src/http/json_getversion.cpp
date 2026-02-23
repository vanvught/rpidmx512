/**
 * @file json_getversion.cpp
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

#include "hal_boardinfo.h"
#include "firmwareversion.h"

namespace json
{
uint32_t GetVersion(char* out_buffer, uint32_t out_buffer_size)
{
    const auto* version = FirmwareVersion::Get()->GetVersion();
    uint8_t hw_text_length;

    const auto kLength = static_cast<uint32_t>(snprintf(out_buffer, out_buffer_size, "{\"version\":\"%.*s\",\"board\":\"%s\",\"build\":{\"date\":\"%.*s\",\"time\":\"%.*s\"}}", firmwareversion::length::kSoftwareVersion,
                                                        version->software_version, hal::BoardName(hw_text_length), firmwareversion::length::kGccDate, version->build_date, firmwareversion::length::kGccTime, version->build_time));
    return kLength;
}
} // namespace json
