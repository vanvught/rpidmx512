/**
 * @file tlc59711dmxparamssave.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "tlc59711dmxparams.h"

#include "propertiesbuilder.h"

#include "devicesparamsconst.h"

#include "debug.h"

bool TLC59711DmxParams::Builder(const struct TTLC59711DmxParams *ptTLC59711Params, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (ptTLC59711Params != 0) {
		memcpy(&m_tTLC59711Params, ptTLC59711Params, sizeof(struct TTLC59711DmxParams));
	} else {
		m_pLC59711ParamsStore->Copy(&m_tTLC59711Params);
	}

	PropertiesBuilder builder(DevicesParamsConst::FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(DevicesParamsConst::LED_TYPE, (const char *)GetLedTypeString((TTLC59711Type) m_tTLC59711Params.LedType), isMaskSet(TLC59711DMX_PARAMS_MASK_LED_TYPE));
	isAdded &= builder.Add(DevicesParamsConst::LED_COUNT, (uint32_t) m_tTLC59711Params.nLedCount, isMaskSet(TLC59711DMX_PARAMS_MASK_LED_COUNT));
	isAdded &= builder.Add(DevicesParamsConst::DMX_START_ADDRESS, (uint32_t) m_tTLC59711Params.nDmxStartAddress, isMaskSet(TLC59711DMX_PARAMS_MASK_START_ADDRESS));
	isAdded &= builder.Add(DevicesParamsConst::SPI_SPEED_HZ, m_tTLC59711Params.nSpiSpeedHz, isMaskSet(TLC59711DMX_PARAMS_MASK_SPI_SPEED));

	nSize = builder.GetSize();

	DEBUG_PRINTF("isAdded=%d, nSize=%d", isAdded, nSize);

	DEBUG_EXIT
	return isAdded;
}

bool TLC59711DmxParams::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pLC59711ParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return false;
	}

	return Builder(0, pBuffer, nLength, nSize);
}
