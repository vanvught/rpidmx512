/**
 * @file artnet4paramssave.cpp
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

#include "artnet4params.h"
#include "artnet4paramsconst.h"
#include "artnetparamsconst.h"

#include "propertiesbuilder.h"

#include "debug.h"

bool ArtNet4Params::Builder(const struct TArtNet4Params *pArtNet4Params, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (pArtNet4Params != 0) {
		memcpy(&m_tArtNet4Params, pArtNet4Params, sizeof(struct TArtNet4Params));
	} else {
		m_pArtNet4ParamsStore->Copy(&m_tArtNet4Params);
	}

	PropertiesBuilder builder(ArtNetParamsConst::FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(ArtNet4ParamsConst::MAP_UNIVERSE0, (uint32_t) m_tArtNet4Params.bMapUniverse0, isMaskSet(ARTNET4_PARAMS_MASK_MAP_UNIVERSE0));

	nSize = builder.GetSize();

	DEBUG_PRINTF("isAdded=%d, nSize=%d", isAdded, nSize);

	DEBUG_EXIT
	return isAdded;
}

bool ArtNet4Params::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pArtNet4ParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return false;
	}

	return Builder(0, pBuffer, nLength, nSize);
}
