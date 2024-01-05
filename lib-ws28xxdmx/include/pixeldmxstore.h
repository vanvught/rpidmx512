/**
 * @file pixeldmxstore.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstddef>
#include <cstdint>

#include "pixeldmxparams.h"
#include "configstore.h"

class PixelDmxStore {
public:
	static void SaveType(uint8_t nType) {
		ConfigStore::Get()->Update(configstore::Store::WS28XXDMX, offsetof(struct pixeldmxparams::Params, nType), &nType, sizeof(uint8_t), pixeldmxparams::Mask::TYPE);
	}

	static void SaveCount(uint16_t nCount) {
		ConfigStore::Get()->Update(configstore::Store::WS28XXDMX, offsetof(struct pixeldmxparams::Params, nCount), &nCount, sizeof(uint16_t), pixeldmxparams::Mask::COUNT);
	}

	static void SaveGroupingCount(uint16_t nGroupingCount) {
		ConfigStore::Get()->Update(configstore::Store::WS28XXDMX, offsetof(struct pixeldmxparams::Params, nGroupingCount), &nGroupingCount, sizeof(uint16_t), pixeldmxparams::Mask::GROUPING_COUNT);
	}

	static void SaveMap(uint8_t nMap) {
		ConfigStore::Get()->Update(configstore::Store::WS28XXDMX, offsetof(struct pixeldmxparams::Params, nMap), &nMap, sizeof(uint8_t), pixeldmxparams::Mask::MAP);
	}

	static void SaveTestPattern(uint8_t nTestPattern) {
		ConfigStore::Get()->Update(configstore::Store::WS28XXDMX, offsetof(struct pixeldmxparams::Params, nTestPattern), &nTestPattern, sizeof(uint8_t), pixeldmxparams::Mask::TEST_PATTERN);
	}

	static void SaveDmxStartAddress(uint16_t nDmxStartAddress) {
		ConfigStore::Get()->Update(configstore::Store::WS28XXDMX, offsetof(struct pixeldmxparams::Params, nDmxStartAddress), &nDmxStartAddress, sizeof(uint16_t), pixeldmxparams::Mask::DMX_START_ADDRESS);
	}
};

#endif /* PIXELDMXSTORE_H_ */
