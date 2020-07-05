/**
 * @file tlc59711dmxparamssave.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "tlc59711dmxparams.h"

#include "propertiesbuilder.h"

#include "devicesparamsconst.h"
#include "lightsetconst.h"

#include "debug.h"

void TLC59711DmxParams::Builder(const struct TTLC59711DmxParams *ptTLC59711Params, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptTLC59711Params != nullptr) {
		memcpy(&m_tTLC59711Params, ptTLC59711Params, sizeof(struct TTLC59711DmxParams));
	} else {
		m_pLC59711ParamsStore->Copy(&m_tTLC59711Params);
	}

	PropertiesBuilder builder(DevicesParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DevicesParamsConst::LED_TYPE, GetLedTypeString(m_tTLC59711Params.LedType), isMaskSet(TLC59711DmxParamsMask::LED_TYPE));
	builder.Add(DevicesParamsConst::LED_COUNT, m_tTLC59711Params.nLedCount, isMaskSet(TLC59711DmxParamsMask::LED_COUNT));
	builder.Add(LightSetConst::PARAMS_DMX_START_ADDRESS, m_tTLC59711Params.nDmxStartAddress, isMaskSet(TLC59711DmxParamsMask::START_ADDRESS));
	builder.Add(DevicesParamsConst::SPI_SPEED_HZ, m_tTLC59711Params.nSpiSpeedHz, isMaskSet(TLC59711DmxParamsMask::SPI_SPEED));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void TLC59711DmxParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pLC59711ParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}
