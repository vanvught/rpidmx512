/**
 * @file rdm_sensors.cpp
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
#include <cstring>
#include <cassert>

#include "rdm_sensors.h"

namespace rdm {
namespace sensors {
static constexpr char TYPE[static_cast<uint32_t>(rdm::sensors::Types::UNDEFINED)][8] = {
		"bh1750", "htu21d", "ina219", "mcp9808", "si7021", "mcp3424" };

const char *get_type_string(rdm::sensors::Types type) {
	if (type < rdm::sensors::Types::UNDEFINED) {
		return TYPE[static_cast<uint32_t>(type)];
	}

	return "Unknown";
}

rdm::sensors::Types get_type_string(const char *pValue) {
	assert(pValue != nullptr);

	for (uint32_t i = 0; i < static_cast<uint32_t>(rdm::sensors::Types::UNDEFINED); i++) {
		if (strcasecmp(pValue, TYPE[i]) == 0) {
			return static_cast<rdm::sensors::Types>(i);
		}
	}

	return rdm::sensors::Types::UNDEFINED;
}
}  // namespace sensors
}  // namespace rdm

