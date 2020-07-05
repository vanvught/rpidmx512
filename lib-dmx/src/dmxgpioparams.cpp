/**
 * @file dmxgpioparams.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <cassert>

#include "dmxgpioparams.h"
#include "dmxgpioparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "gpio.h"

#define DATA_DIRECTION_MASK			(1U << 0)
#define DATA_DIRECTION_OUT_A_MASK	(1U << 1)
#define DATA_DIRECTION_OUT_B_MASK	(1U << 2)
#define DATA_DIRECTION_OUT_C_MASK	(1U << 3)
#define DATA_DIRECTION_OUT_D_MASK	(1U << 4)

DmxGpioParams::DmxGpioParams()
		
{
#if defined(H3)
 #if defined(ORANGE_PI_ONE)
	m_nDmxDataDirectionOut[0] = GPIO_DMX_DATA_DIRECTION_OUT_D;
	m_nDmxDataDirectionOut[1] = GPIO_DMX_DATA_DIRECTION_OUT_A;
	m_nDmxDataDirectionOut[2] = GPIO_DMX_DATA_DIRECTION_OUT_B;
	m_nDmxDataDirectionOut[3] = GPIO_DMX_DATA_DIRECTION_OUT_C;
 #else
	m_nDmxDataDirectionOut[1] = GPIO_DMX_DATA_DIRECTION_OUT_C;
	m_nDmxDataDirectionOut[2] = GPIO_DMX_DATA_DIRECTION_OUT_B;
 #endif
#else
	m_nDmxDataDirectionOut[0] = GPIO_DMX_DATA_DIRECTION;
#endif
}

bool DmxGpioParams::Load() {
	m_nSetList = 0;

	ReadConfigFile configfile(DmxGpioParams::staticCallbackFunction, this);
	return configfile.Read(DmxGpioParamsConst::FILE_NAME);
}

void DmxGpioParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint8_t value8;

	if (Sscan::Uint8(pLine, DmxGpioParamsConst::DATA_DIRECTION, value8) == Sscan::OK) {
		if (value8 < 32) {
			m_nDmxDataDirection = value8;
			m_nSetList |= DATA_DIRECTION_MASK;
		}
		return;
	}

	for (unsigned i = 0; i < 4; i++) {
		if (Sscan::Uint8(pLine, DmxGpioParamsConst::DATA_DIRECTION_OUT[i], value8) == Sscan::OK) {
			m_nDmxDataDirectionOut[i] = value8;
			m_nSetList |= (DATA_DIRECTION_OUT_A_MASK << i);
			return;
		}
	}
}

uint8_t DmxGpioParams::GetDataDirection(bool &bIsSet) const {
	bIsSet = isMaskSet(DATA_DIRECTION_MASK);
	return m_nDmxDataDirection;
}

uint8_t DmxGpioParams::GetDataDirection(bool &bIsSet, uint8_t nUart) const {
	assert(nUart < DMX_MAX_OUT);

	switch (nUart) {
	case 0:
		bIsSet = isMaskSet(DATA_DIRECTION_OUT_A_MASK);
		break;
	case 1:
		bIsSet = isMaskSet(DATA_DIRECTION_OUT_B_MASK);
		break;
	case 2:
		bIsSet = isMaskSet(DATA_DIRECTION_OUT_C_MASK);
		break;
	case 3:
		bIsSet = isMaskSet(DATA_DIRECTION_OUT_D_MASK);
		break;
	default:
		break;
	}

	return m_nDmxDataDirectionOut[nUart];
}

void DmxGpioParams::Dump() {
#ifndef NDEBUG
	if (m_nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DmxGpioParamsConst::FILE_NAME);

	if (isMaskSet(DATA_DIRECTION_MASK)) {
		printf(" %s=%d\n", DmxGpioParamsConst::DATA_DIRECTION, m_nDmxDataDirection);
	}

	for (unsigned i = 0; i < 4; i++) {
		if (isMaskSet(DATA_DIRECTION_OUT_A_MASK << i)) {
			printf(" %s=%d\n", DmxGpioParamsConst::DATA_DIRECTION_OUT[i], m_nDmxDataDirectionOut[i]);
		}
	}
#endif
}

void DmxGpioParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<DmxGpioParams*>(p))->callbackFunction(s);
}

