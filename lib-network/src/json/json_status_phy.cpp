/**
 * @file json_status_phy.cpp
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

#include <cstdio>

#include "emac/phy.h"

namespace json::status::net
{
uint32_t Phy(char* out_buffer, uint32_t out_buffer_size)
{
    ::net::phy::Status phy_status;
    ::net::phy::CustomizedStatus(phy_status);

    const auto kLength = static_cast<uint32_t>(snprintf(out_buffer, out_buffer_size,
        "{\"link\":\"%s\",\"speed\":\"%s\",\"duplex\":\"%s\",\"autonegotiation\":\"%s\"}",
        ::net::phy::ToString(phy_status.link),
        ::net::phy::ToString(phy_status.speed),
        ::net::phy::ToString(phy_status.duplex),
        ::net::phy::ToStringAutonegotiation(phy_status.autonegotiation))
      );

    return kLength;
}
} // namespace json::status::net
