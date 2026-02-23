/**
 * @file pixeldmxstore.h
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PIXELDMXSTORE_H_
#define PIXELDMXSTORE_H_

#include <cstdint>

#include "configstore.h"
#include "configurationstore.h"

namespace dmxled_store
{
inline void SaveType(uint8_t type)
{
    ConfigStore::Instance().DmxLedUpdate(&common::store::DmxLed::type, type);
}

inline void SaveCount(uint16_t count)
{
    ConfigStore::Instance().DmxLedUpdate(&common::store::DmxLed::count, count);
}

inline void SaveGroupingCount(uint16_t grouping_count)
{
    ConfigStore::Instance().DmxLedUpdate(&common::store::DmxLed::grouping_count, grouping_count);
}

inline void SaveMap(uint8_t map)
{
    ConfigStore::Instance().DmxLedUpdate(&common::store::DmxLed::map, map);
}

inline void SaveTestPattern(uint8_t test_pattern)
{
    ConfigStore::Instance().DmxLedUpdate(&common::store::DmxLed::test_pattern, test_pattern);
}

inline void SaveDmxStartAddress(uint16_t dmx_start_address)
{
    ConfigStore::Instance().DmxLedUpdate(&common::store::DmxLed::dmx_start_address, dmx_start_address);
}
} // namespace dmxled_store

#endif  // PIXELDMXSTORE_H_
