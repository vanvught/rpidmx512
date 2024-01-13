/**
 * net_phy_string.cpp
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>

#include "emac/phy.h"

#if !defined (ARRAY_SIZE)
# define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

namespace net {
static constexpr char SPEED[3][10] = { "10baseT", "100baseTX", "1000baseT" };

const char *phy_string_get_link(const Link link) {
	return link == Link::STATE_UP ? "up" : "down";
}

const char *phy_string_get_duplex(const Duplex duplex) {
	return duplex == Duplex::DUPLEX_HALF ? "half" : "full";
}

const char *phy_string_get_speed(const Speed speed) {
	const auto nIndex = static_cast<uint32_t>(speed);

	assert(nIndex < ARRAY_SIZE(SPEED));
	return SPEED[nIndex];
}

const char *phy_string_get_autonegotiation(const bool autonegotiation) {
	return autonegotiation ? "on" : "off";
}

}
