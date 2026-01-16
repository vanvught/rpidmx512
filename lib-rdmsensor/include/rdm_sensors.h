/**
 * @file rdm_sensors.h
 *
 */
/* Copyright (C) 2023-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDM_SENSORS_H_
#define RDM_SENSORS_H_

#include <cstdint>
#include <cstring>
#include <cassert>

#include "common/utils/utils_enum.h"
#include "json/rdmsensorsparams.h"

namespace rdm::sensors
{
enum class Types: uint32_t
{
    kBH170,
    kHTU21D,
    kINA219,
    kMCP9808,
    kSI7021,
    kMCP3424,
    kUndefined
};

static_assert(json::RdmSensorsParams::KeysSize() == static_cast<size_t>(Types::kUndefined));

[[nodiscard]] inline constexpr const char* GetType(Types type)
{
    if (type < rdm::sensors::Types::kUndefined)
    {
        const auto& k = json::RdmSensorsParams::Keys();
        return k[static_cast<uint32_t>(type)].GetName();
    }

    return "Unknown";
}

inline Types GetType(const char* string)
{
    assert(string != nullptr);
    const auto& k = json::RdmSensorsParams::Keys();

    for (uint32_t i = 0; i < json::RdmSensorsParams::KeysSize(); i++)

    {
        if (strcasecmp(string, k[i].GetName()) == 0)
        {
            return common::FromValue<Types>(i);
        }
    }

    return Types::kUndefined;
}
} // namespace rdm::sensors

#endif  // RDM_SENSORS_H_
