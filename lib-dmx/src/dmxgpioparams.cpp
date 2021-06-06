/**
 * @file dmxgpioparams.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#ifndef NDEBUG
# include <cstdio>
#endif
#include <cassert>

#include "dmxgpioparams.h"
#include "dmxgpioparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "dmxgpio.h"

#define DATA_DIRECTION_MASK			(1U << 0)

bool DmxGpioParams::Load() {
	m_nSetList = 0;

	ReadConfigFile configfile(DmxGpioParams::staticCallbackFunction, this);
	return configfile.Read(DmxGpioParamsConst::FILE_NAME);
}

void DmxGpioParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint8_t value8;

	if (Sscan::Uint8(pLine, DmxGpioParamsConst::DATA_DIRECTION, value8) == Sscan::OK) {
		if ((value8 < 32) && (value8 != GPIO_DMX_DATA_DIRECTION)) {
			m_nDmxDataDirection = value8;
			m_nSetList |= DATA_DIRECTION_MASK;
		} else {
			m_nDmxDataDirection = GPIO_DMX_DATA_DIRECTION;
			m_nSetList &= ~DATA_DIRECTION_MASK;
		}
		return;
	}
}

void DmxGpioParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DmxGpioParamsConst::FILE_NAME);

	if (isMaskSet(DATA_DIRECTION_MASK)) {
		printf(" %s=%d\n", DmxGpioParamsConst::DATA_DIRECTION, m_nDmxDataDirection);
	}
#endif
}

void DmxGpioParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<DmxGpioParams*>(p))->callbackFunction(s);
}

uint8_t DmxGpioParams::GetDataDirection(bool &bIsSet) const {
	bIsSet = isMaskSet(DATA_DIRECTION_MASK);
	return m_nDmxDataDirection;
}
