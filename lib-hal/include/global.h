/**
 * @file global.h
 * @brief Defines the Global class for managing UTC offset configuration and validation.
 *
 * This file contains declarations and implementation details of the Global singleton class,
 * which is used to get and set the current UTC offset. It also contains validation logic
 * for standard and non-standard UTC offsets.
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <cstdint>

#include "utc.h"

/**
 * @namespace global
 * @brief Contains global variables related to time configuration.
 */
namespace global
{
extern int32_t g_nUtcOffset;
}

/**
 * @class Global
 * @brief Singleton class for managing and validating UTC offsets.
 *
 * The Global class provides methods to set and get the UTC offset in seconds.
 * It also includes validation logic for standard UTC time zones.
 */
class Global
{
   public:
    /**
     * @brief Get the singleton instance of the Global class.
     * @return Reference to the singleton Global object.
     */
    static Global& Instance()
    {
        static Global instance;
        return instance;
    }

    /**
     * @brief Gets the current UTC offset in seconds.
     * @return UTC offset in seconds.
     */
    int32_t GetUtcOffset() const { return global::g_nUtcOffset; }

    /**
     * @brief Gets the current UTC offset as (hours, minutes).
     * @param[out] hours Signed hour component.
     * @param[out] minutes Unsigned minute component.
     */
    inline void GetUtcOffset(int32_t& hours, uint32_t& minutes) { hal::utc::SplitOffset(global::g_nUtcOffset, hours, minutes); }

    /**
     * @brief Sets the global UTC offset if the value is valid.
     * @param utc_offset_seconds Offset in seconds
     * @return true if successfully set; false otherwise
     */
    inline bool SetUtcOffsetIfValid(int32_t utc_offset_seconds)
    {
        if (hal::utc::IsValidOffset(utc_offset_seconds))
        {
            ::global::g_nUtcOffset = utc_offset_seconds;
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
    inline bool SetUtcOffsetIfValid(int32_t hours, uint32_t minutes)
    {
        int32_t offset_seconds;
        if (hal::utc::ValidateOffset(hours, minutes, offset_seconds))
        {
            return SetUtcOffsetIfValid(offset_seconds);
        }
        return false;
    }

   private:
    Global() = default;
    // Delete copy/move constructors and assignment operators
    Global(const Global&) = delete;
    Global& operator=(const Global&) = delete;
    Global(Global&&) = delete;
    Global& operator=(Global&&) = delete;
};

#endif  // GLOBAL_H_
