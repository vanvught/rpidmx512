/**
 * @file firmwareversion.h
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef FIRMWAREVERSION_H_
#define FIRMWAREVERSION_H_

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "hal_boardinfo.h"

#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

namespace firmwareversion
{
namespace length
{
static constexpr auto kSoftwareVersion = 3;
static constexpr auto kGccDate = 11;
static constexpr auto kGccTime = 8;
} // namespace length
struct Info
{
    char software_version[length::kSoftwareVersion];
    char build_date[length::kGccDate];
    char build_time[length::kGccTime];
};
} // namespace firmwareversion

class FirmwareVersion
{
   public:
    explicit FirmwareVersion(const char* software_version, const char* date, const char* time, uint32_t software_version_id = 0)
        : kSoftwareVersionId(software_version_id)
    {
        assert(software_version != nullptr);
        assert(date != nullptr);
        assert(time != nullptr);

        assert(s_this == nullptr);
        s_this = this;

        memcpy(s_firmware_version.software_version, software_version, firmwareversion::length::kSoftwareVersion);
        memcpy(s_firmware_version.build_date, date, firmwareversion::length::kGccDate);
        memcpy(s_firmware_version.build_time, time, firmwareversion::length::kGccTime);

        uint8_t hw_text_length;

        snprintf(s_print, sizeof(s_print) - 1, "[V%.*s] %s Compiled on %.*s at %.*s", firmwareversion::length::kSoftwareVersion,
                 s_firmware_version.software_version, hal::BoardName(hw_text_length), firmwareversion::length::kGccDate, s_firmware_version.build_date,
                 firmwareversion::length::kGccTime, s_firmware_version.build_time);
    }

    void Print(const char* title = nullptr)
    {
        puts(s_print);

        if (title != nullptr)
        {
            printf("\x1b[32m%s\x1b[0m\n", title);
        }
    }

    const struct firmwareversion::Info* GetVersion() { return &s_firmware_version; }
    const char* GetPrint() { return s_print; }
    const char* GetSoftwareVersion() { return s_firmware_version.software_version; }
    uint32_t GetVersionId() const { return kSoftwareVersionId; }

    static FirmwareVersion* Get() { return s_this; }

   private:
    const uint32_t kSoftwareVersionId;
    static inline firmwareversion::Info s_firmware_version;
    static inline char s_print[64];
    static inline FirmwareVersion* s_this;
};

#endif  // FIRMWAREVERSION_H_
