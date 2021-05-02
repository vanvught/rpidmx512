/**
 * @file rdmsubdevicesstatic.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <string.h>
#include <cassert>

#include "rdmsubdevices.h"
#include "rdmsubdevicesconst.h"

using namespace rdm::subdevices;

const char* RDMSubDevices::GetTypeString(type tType) {
	if (tType < type::UNDEFINED) {
		return RDMSubDevicesConst::TYPE[tType];
	}

	return "Unknown";
}

type RDMSubDevices::GetTypeString(const char *pValue) {
	assert(pValue != nullptr);

	for (uint32_t i = 0; i < (type::UNDEFINED); i++) {
		if (strcasecmp(pValue, RDMSubDevicesConst::TYPE[i]) == 0) {
			return static_cast<type>(i);
		}
	}

	return type::UNDEFINED;
}
