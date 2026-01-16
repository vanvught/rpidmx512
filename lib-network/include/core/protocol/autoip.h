/**
 * @file autoip.h
 *
 */
/* Copyright (C) 2024-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef CORE_PROTOCOL_AUTOIP_H_
#define CORE_PROTOCOL_AUTOIP_H_

#include "ip4/ip4_address.h"

namespace network::autoip
{
// RFC 3927 Section 2.1
inline constexpr auto kNet = network::ConvertToUint(169, 254, 0, 0);
inline constexpr auto kRangeStart = network::ConvertToUint(169, 254, 1, 0);
inline constexpr auto kRangeEnd = network::ConvertToUint(169, 254, 254, 255);

enum class State
{
    kOff,
    kChecking,
    kBound
};
} // namespace network::autoip

#endif // CORE_PROTOCOL_AUTOIP_H_
