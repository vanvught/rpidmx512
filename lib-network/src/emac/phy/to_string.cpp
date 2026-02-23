/**
 * @file to_string.cpp
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

#include <cstddef>

#include "emac/phy.h"

namespace net::phy
{
// Ensure order matches enum class Speed
constexpr const char* kSpeedNames[] = {
    "10baseT",   // Speed::SPEED10
    "100baseTX", // Speed::SPEED100
    "1000baseT"  // Speed::SPEED1000
};

static_assert(static_cast<size_t>(phy::Speed::kSpeed10) == 0, "Enum ordering mismatch");
static_assert(static_cast<size_t>(phy::Speed::kSpeed1000) < (sizeof(kSpeedNames) / sizeof(kSpeedNames[0])), "Enum range mismatch");

const char* ToString(phy::Link link)
{
    return link == phy::Link::kStateUp ? "up" : "down";
}

const char* ToString(phy::Duplex duplex)
{
    return duplex == phy::Duplex::kDuplexHalf ? "half" : "full";
}

const char* ToString(phy::Speed speed)
{
    const auto kIndex = static_cast<size_t>(speed);
    if (kIndex < sizeof(kSpeedNames) / sizeof(kSpeedNames[0]))
    {
        return kSpeedNames[kIndex];
    }
    return "unknown";
}

const char* ToStringAutonegotiation(bool autonegotiation)
{
    return autonegotiation ? "on" : "off";
}

} // namespace net::phy
