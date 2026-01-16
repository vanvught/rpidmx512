/**
 * @file rdm_subdevices.h _subdevices.h
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

#ifndef RDM_SUBDEVICES_H_
#define RDM_SUBDEVICES_H_

#include <cstdint>
#include <cstring>
#include <cassert>

#include "common/utils/utils_enum.h"

namespace rdm::subdevices
{
enum class Types : uint8_t
{
    BW7FETS,
    BWDIMMER,
    BWDIO,
    BWLCD,
    BWRELAY, // BitWizard
    MCP23S08,
    MCP23S17, // GPIO
    MCP4822,
    MCP4902, // DAC
    kUndefined
};

inline static constexpr uint32_t kMaxNameLength = 9; // + '\0'

inline constexpr const char kType[static_cast<uint32_t>(rdm::subdevices::Types::kUndefined)][kMaxNameLength] = {
    "bw7fets",  //
    "bwdimmer", //
    "bwdio",    //
    "bwlcd",    //
    "bwrelay",  //
    "mcp23s08", //
    "mcp23s17", //
    "mcp4822",  //
    "mcp4902"   //
};

[[nodiscard]] inline constexpr const char* GetTypeString(rdm::subdevices::Types type)
{    
    return type < Types::kUndefined ? kType[static_cast<uint32_t>(type)] : "UNDEFINED";
}

inline Types GetTypeString(const char* string)
{
    assert(string != nullptr);
    uint8_t index = 0;

    for (const char(&type)[kMaxNameLength] : kType)
    {
        if (strcasecmp(string, type) == 0)
        {
            return common::FromValue<Types>(index);
        }
        ++index;
    }

    return Types::kUndefined;
}

} // namespace rdm::subdevices

#endif  // RDM_SUBDEVICES_H_
