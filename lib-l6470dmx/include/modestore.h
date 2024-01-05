/**
 * @file modestore.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef MODESTORE_H_
#define MODESTORE_H_

#include <cstdint>
#include <cstddef>
#include <cassert>

#include "l6470dmxstore.h"

#include "configstore.h"

#include "debug.h"

class ModeStore {
public:
	static void SaveDmxStartAddress(uint32_t nMotorIndex, uint16_t nDmxStartAddress) {
		DEBUG_ENTRY

		assert(nMotorIndex < motorstore::MAX_MOTORS);

		const uint32_t nOffsetModeParms = motorstore::OFFSET(nMotorIndex) + offsetof(struct motorstore::MotorStore, ModeParams);

		DEBUG_PRINTF("nOffsetModeParms=%u", nOffsetModeParms);

		ConfigStore::Get()->Update(configstore::Store::MOTORS, nOffsetModeParms + offsetof(struct modeparams::Params, nDmxStartAddress), &nDmxStartAddress, sizeof(uint16_t), modeparams::Mask::DMX_START_ADDRESS, nOffsetModeParms);

		DEBUG_EXIT
	}
};

#endif /* MODESTORE_H_ */
