/**
 * @file hal_serialnumber.cpp
 *
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

#include "network.h"
#include "hal_serialnumber.h"

namespace hal
{
void SerialNumber(uint8_t sn[kSnSize])
{
    const auto kIp = net::GetPrimaryIp();
#if !defined(CONFIG_RDMDEVICE_REVERSE_UID)
    sn[0] = static_cast<uint8_t>(kIp >> 24);
    sn[1] = (kIp >> 16) & 0xFF;
    sn[2] = (kIp >> 8) & 0xFF;
    sn[3] = kIp & 0xFF;
#else
    sn[0] = static_cast<uint8_t>(kIp >> 24);
    sn[1] = (kIp >> 16) & 0xFF;
    sn[2] = (kIp >> 8) & 0xFF;
    sn[3] = kIp & 0xFF;
#endif
}
} // namespace hal