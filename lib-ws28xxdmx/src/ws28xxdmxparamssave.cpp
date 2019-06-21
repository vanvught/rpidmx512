/**
 * @file ws28xxparamssave.cpp
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

#include "ws28xxdmxparams.h"

#include "propertiesbuilder.h"

#include "devicesparamsconst.h"

#include "debug.h"

bool WS28xxDmxParams::Builder(const struct TWS28xxDmxParams *ptWS28xxParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (ptWS28xxParams != 0) {
		memcpy(&m_tWS28xxParams, ptWS28xxParams, sizeof(struct TWS28xxDmxParams));
	} else {
		m_pWS28xxParamsStore->Copy(&m_tWS28xxParams);
	}

	PropertiesBuilder builder(DevicesParamsConst::FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(DevicesParamsConst::LED_TYPE, (const char *)GetLedTypeString((TWS28XXType) m_tWS28xxParams.tLedType), isMaskSet(WS28XXDMX_PARAMS_MASK_LED_TYPE));
	isAdded &= builder.Add(DevicesParamsConst::LED_COUNT, (uint32_t) m_tWS28xxParams.nLedCount, isMaskSet(WS28XXDMX_PARAMS_MASK_LED_COUNT));

	isAdded &= builder.Add(DevicesParamsConst::LED_GROUPING, (uint32_t) m_tWS28xxParams.bLedGrouping, isMaskSet(WS28XXDMX_PARAMS_MASK_LED_GROUPING));
	isAdded &= builder.Add(DevicesParamsConst::LED_GROUP_COUNT, (uint32_t) m_tWS28xxParams.nLedGroupCount, isMaskSet(WS28XXDMX_PARAMS_MASK_LED_GROUP_COUNT));

	isAdded &= builder.Add(DevicesParamsConst::DMX_START_ADDRESS, (uint32_t) m_tWS28xxParams.nDmxStartAddress, isMaskSet(WS28XXDMX_PARAMS_MASK_DMX_START_ADDRESS));

	isAdded &= builder.Add(DevicesParamsConst::SPI_SPEED_HZ, m_tWS28xxParams.nSpiSpeedHz, isMaskSet(WS28XXDMX_PARAMS_MASK_SPI_SPEED));
	isAdded &= builder.Add(DevicesParamsConst::GLOBAL_BRIGHTNESS, (uint32_t) m_tWS28xxParams.nGlobalBrightness, isMaskSet(WS28XXDMX_PARAMS_MASK_GLOBAL_BRIGHTNESS));

	isAdded &= builder.Add(DevicesParamsConst::ACTIVE_OUT, (uint32_t) m_tWS28xxParams.nActiveOutputs, isMaskSet(WS28XXDMX_PARAMS_MASK_ACTIVE_OUT));
	isAdded &= builder.Add(DevicesParamsConst::USE_SI5351A, (uint32_t) m_tWS28xxParams.bUseSI5351A, isMaskSet(WS28XXDMX_PARAMS_MASK_USE_SI5351A));

	nSize = builder.GetSize();

	DEBUG_PRINTF("isAdded=%d, nSize=%d", isAdded, nSize);

	DEBUG_EXIT
	return isAdded;
}

bool WS28xxDmxParams::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pWS28xxParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return false;
	}

	return Builder(0, pBuffer, nLength, nSize);
}
