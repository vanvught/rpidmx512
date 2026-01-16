/**
 * @file gps.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

/**
 * https://gpsd.gitlab.io/gpsd/NMEA.html
 */

#ifndef GPS_H_
#define GPS_H_

#include <cstdint>
#include <time.h>
#include <cassert>
#include <cstring>

#include "common/utils/utils_enum.h"

namespace gps
{
namespace nmea
{
inline static constexpr uint32_t kMaxSentenceLength = 82; ///< including the $ and <CR><LF>
inline static constexpr char kStartDelimiter = '$';       ///< The start delimiter is normally '$' (ASCII 36)
} // namespace nmea
namespace module
{
inline static constexpr uint32_t kMaxNameLength = 11; // + '\0'
} // namespace module

enum class Status
{
    kIdle,
    kWarning,
    kValid,
    kUndefined
};

enum class Module: uint8_t
{
    kAtgM336H,
    kUbloxNeo,
    kMtK3339,
    kUndefined
};

inline constexpr const char kModule[static_cast<uint32_t>(Module::kUndefined)][module::kMaxNameLength] = 
{
	"ATGM336H", 
	"ublox-NEO7", 
	"MTK3339"
};

inline constexpr const char kBaud115200[static_cast<uint32_t>(Module::kUndefined)][nmea::kMaxSentenceLength] = {
    "$PCAS01,5*19\r\n", "$PUBX,41,1,0007,0003,115200,0*18\r\n", "$PMTK251,115200*1F\r\n"};

[[nodiscard]] inline constexpr const char* GetModule(Module module)
{
    return module < Module::kUndefined ? kModule[static_cast<uint32_t>(module)] : "UNDEFINED";
}

inline Module GetModule(const char* string)
{
    assert(string != nullptr);
    uint8_t index = 0;

    for (const char(&module)[module::kMaxNameLength] : kModule)
    {
        if (strcasecmp(string, module) == 0)
        {
            return common::FromValue<Module>(index);
        }
        ++index;
    }

    return Module::kUndefined;
}
} // namespace gps

class GPS
{
   public:
    explicit GPS(int32_t utc_offset, gps::Module module = gps::Module::kUndefined);

    void SetUtcOffset(int32_t utc_offset) { utc_offset_ = utc_offset; }
    int32_t GetUtcOffset() const { return utc_offset_; }

    gps::Module GetModule() const { return module_; }

    bool IsDateUpdated() const { return is_date_updated_; }

    bool IsTimeUpdated() const { return is_time_updated_; }

    uint32_t GetTimeTimestampMillis() const { return time_timestamp_millis_; }

    const struct tm* GetDateTime()
    {
        is_time_updated_ = is_date_updated_ = false;
        return &tm_;
    }

    time_t GetLocalSeconds()
    {
        is_time_updated_ = is_date_updated_ = false;
        return mktime(&tm_) + utc_offset_;
    }

    gps::Status GetStatus() const { return status_current_; }

    void Display(gps::Status status);

    void Start();
    void Run();
    void Print();

    static GPS* Get() { return s_this; }

   private:
    void UartInit();
    void UartSetBaud(uint32_t baud = 9600);
    const char* UartGetSentence(); // Return a valid sentence that is starting with $ and a valid checksum
    void UartSend(const char* sentence);
    //
    uint32_t GetTag(const char* p);
    int32_t ParseDecimal(const char* p, uint32_t& length);
    void SetTime(int32_t time);
    void SetDate(int32_t date);

    void DumpSentence(const char* sentence);

   private:
    int32_t utc_offset_;
    gps::Module module_;
    uint32_t baud_{9600};
    uint32_t time_timestamp_millis_;
    uint32_t date_timestamp_millis_;
    struct tm tm_;
    char* sentence_{nullptr};
    bool is_time_updated_{false};
    bool is_date_updated_{false};
    gps::Status status_current_{gps::Status::kUndefined};
    gps::Status status_previous_{gps::Status::kUndefined};

    static GPS* s_this;
};

#endif  // GPS_H_
