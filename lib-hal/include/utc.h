/**
 * @file utc.h
 * @brief UTC offset handling utilities (validation, conversion, parsing).
 *
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

#ifndef UTC_H_
#define UTC_H_

#include <cstdint>
#include <cstring>

/**
 * @namespace global
 * @brief Holds global UTC offset variable used system-wide.
 */
namespace global
{
/**
 * @brief The current UTC offset in seconds.
 */
extern int32_t g_nUtcOffset;
} // namespace global

namespace hal::utc
{

constexpr int32_t kUtcOffsetMin = -12; ///< Minimum valid UTC offset (hours)
constexpr int32_t kUtcOffsetMax = 14;  ///< Maximum valid UTC offset (hours)

/**
 * @struct Offset
 * @brief Represents a fractional UTC offset.
 */
struct Offset
{
    int32_t hours;    ///< Signed hour component
    uint32_t minutes; ///< Unsigned minute component
};

/**
 * @brief List of valid fractional UTC offsets.
 */
constexpr Offset kValidOffsets[] = {{-9, 30}, {-3, 30}, {3, 30}, {4, 30}, {5, 30}, {5, 45}, {6, 30}, {8, 45}, {9, 30}, {10, 30}, {12, 45}};

/**
 * @brief Validates (hours, minutes) and converts to UTC offset in seconds.
 * @param hours Signed hours offset
 * @param minutes Unsigned minutes offset
 * @param[out] utc_offset_seconds Resulting offset in seconds
 * @return true if valid; false otherwise
 */
inline bool ValidateOffset(int32_t hours, uint32_t minutes, int32_t& utc_offset_seconds)
{
    if (hours >= kUtcOffsetMin && hours <= kUtcOffsetMax)
    {
        if (minutes == 0)
        {
            utc_offset_seconds = hours * 3600;
            return true;
        }
        for (const auto& offset : kValidOffsets)
        {
            if (offset.hours == hours && offset.minutes == minutes)
            {
                utc_offset_seconds = (hours >= 0) ? (hours * 3600 + static_cast<int32_t>(minutes) * 60) : (hours * 3600 - static_cast<int32_t>(minutes) * 60);
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Checks if the given UTC offset in seconds is valid.
 * @param utc_offset_seconds UTC offset in seconds
 * @return true if offset is valid; false otherwise
 */
inline bool IsValidOffset(int32_t utc_offset_seconds)
{
    if (utc_offset_seconds == 0) return true;
    int32_t hours = utc_offset_seconds / 3600;
    uint32_t minutes = (utc_offset_seconds >= 0) ? static_cast<uint32_t>(utc_offset_seconds - hours * 3600) / 60
                                                 : static_cast<uint32_t>((hours * 3600 - utc_offset_seconds)) / 60;

    if (minutes == 0 && hours >= kUtcOffsetMin && hours <= kUtcOffsetMax)
    {
        return true;
    }

    for (const auto& offset : kValidOffsets)
    {
        int32_t offset_seconds = (offset.hours >= 0) ? offset.hours * 3600 + static_cast<int32_t>(offset.minutes * 60)
                                                     : offset.hours * 3600 - static_cast<int32_t>(offset.minutes * 60);
        if (utc_offset_seconds == offset_seconds) return true;
    }
    return false;
}

/**
 * @brief Converts UTC offset in seconds to (hours, minutes).
 * @param utc_offset_seconds Offset in seconds
 * @param[out] hours Signed hour component
 * @param[out] minutes Unsigned minute component
 */
inline void SplitOffset(int32_t utc_offset_seconds, int32_t& hours, uint32_t& minutes)
{
    hours = utc_offset_seconds / 3600;
    if (utc_offset_seconds >= 0)
    {
        minutes = static_cast<uint32_t>((utc_offset_seconds - hours * 3600) / 60);
    }
    else
    {
        minutes = static_cast<uint32_t>(((hours * 3600) - utc_offset_seconds) / 60);
    }
}

/**
 * @brief Parses a UTC string in "+HH:MM" or "-HH:MM" format.
 * @param buffer Pointer to the 6-character buffer
 * @param buffer_length Must be 6
 * @param[out] hours Signed hour component
 * @param[out] minutes Unsigned minute component
 * @return true if parse was successful and valid; false otherwise
 */
inline bool ParseOffset(const char* buffer, uint32_t buffer_length, int32_t& hours, uint32_t& minutes)
{
    if (buffer == nullptr) return false;

    if (buffer_length == 5)
    {
        static constexpr const char kZeroOffset[5] = {'0', '0', ':', '0', '0'};
        if (memcmp(kZeroOffset, buffer, sizeof(kZeroOffset)) == 0)
        {
            hours = 0;
            minutes = 0;
            return true;
        }
    }

    if (buffer_length != 6) return false;
    if (buffer[0] != '+' && buffer[0] != '-') return false;

    bool negative = (buffer[0] == '-');

    if (buffer[1] < '0' || buffer[1] > '1') return false;
    if (buffer[2] < '0' || buffer[2] > '9') return false;
    if (buffer[3] != ':') return false;
    if (buffer[4] < '0' || buffer[4] > '5') return false;
    if (buffer[5] < '0' || buffer[5] > '9') return false;

    int32_t h = (buffer[1] - '0') * 10 + (buffer[2] - '0');
    if (h > 14) return false;
    uint32_t m = static_cast<uint32_t>((buffer[4] - '0') * 10 + (buffer[5] - '0'));
    if (m >= 60) return false;

    hours = negative ? -h : h;
    minutes = m;

    int32_t dummy;
    return ValidateOffset(hours, minutes, dummy);
}
} // namespace hal::utc

#endif // UTC_H_
