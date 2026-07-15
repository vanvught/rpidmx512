/**
 * @file global.h
 * @brief Defines the Global class for managing UTC offset configuration and validation.
 *
 * This file contains declarations and implementation details of the Global singleton class,
 * which is used to get and set the current UTC offset. It also contains validation logic
 * for standard and non-standard UTC offsets.
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <cstdint>

#include "utc.h"

namespace global {

/**
 * @brief Gets the current UTC offset in seconds.
 * @return UTC offset in seconds.
 */
inline int32_t GetUtcOffset() {
    return global::g_utc_offset;
}

/**
 * @brief Gets the current UTC offset as (hours, minutes).
 * @param[out] hours Signed hour component.
 * @param[out] minutes Unsigned minute component.
 */
inline void GetUtcOffset(int32_t& hours, uint32_t& minutes) {
    utc::SplitOffset(global::g_utc_offset, hours, minutes);
}

/**
 * @brief Sets the global UTC offset if the value is valid.
 * @param utc_offset_seconds Offset in seconds
 * @return true if successfully set; false otherwise
 */
inline bool SetUtcOffsetIfValid(int32_t utc_offset_seconds) {
    if (utc::IsValidOffset(utc_offset_seconds)) {
        ::global::g_utc_offset = utc_offset_seconds;
        return true;
    }
    return false;
}

/**
 * @brief Sets the global UTC offset from (hours, minutes) if valid.
 * @param hours Signed hour component
 * @param minutes Unsigned minute component
 * @return true if valid and set; false otherwise
 */
inline bool SetUtcOffsetIfValid(int32_t hours, uint32_t minutes) {
    int32_t offset_seconds;
    if (utc::ValidateOffset(hours, minutes, offset_seconds)) {
        return SetUtcOffsetIfValid(offset_seconds);
    }
    return false;
}
} // namespace global

#endif // GLOBAL_H_
