/**
 * @file e131paramssave.cpp
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

#include "e131params.h"
#include "e131paramsconst.h"

#include "propertiesbuilder.h"

#include "lightset.h"
#include "lightsetconst.h"

#include "debug.h"

#define MERGEMODE2STRING(m)		(m == E131_MERGE_HTP) ? "htp" : "ltp"

bool E131Params::Builder(const struct TE131Params *ptE131Params, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (ptE131Params != 0) {
		memcpy(&m_tE131Params, ptE131Params, sizeof(struct TE131Params));
	} else {
		m_pE131ParamsStore->Copy(&m_tE131Params);
	}

	PropertiesBuilder builder(E131ParamsConst::PARAMS_FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(LightSetConst::PARAMS_UNIVERSE, (uint32_t) m_tE131Params.nUniverse, isMaskSet(E131_PARAMS_MASK_UNIVERSE));
	isAdded &= builder.Add(LightSetConst::PARAMS_OUTPUT, LightSet::GetOutputType(m_tE131Params.tOutputType), isMaskSet(E131_PARAMS_MASK_OUTPUT));

	isAdded &= builder.Add(E131ParamsConst::PARAMS_MERGE_MODE, MERGEMODE2STRING(m_tE131Params.nMergeMode), isMaskSet(E131_PARAMS_MASK_MERGE_MODE));

	for (unsigned i = 0; i < E131_PARAMS_MAX_PORTS; i++) {
		isAdded &= builder.Add(E131ParamsConst::PARAMS_UNIVERSE_PORT[i], (uint32_t) m_tE131Params.nUniversePort[i], isMaskSet(E131_PARAMS_MASK_UNIVERSE_A << i));
		isAdded &= builder.Add(E131ParamsConst::PARAMS_MERGE_MODE_PORT[i], MERGEMODE2STRING(m_tE131Params.nMergeModePort[i]), isMaskSet(E131_PARAMS_MASK_MERGE_MODE_A << i));
	}

	isAdded &= builder.Add(E131ParamsConst::PARAMS_NETWORK_DATA_LOSS_TIMEOUT, m_tE131Params.nNetworkTimeout, isMaskSet(E131_PARAMS_MASK_NETWORK_TIMEOUT));
	isAdded &= builder.Add(E131ParamsConst::PARAMS_DISABLE_MERGE_TIMEOUT, (uint32_t) m_tE131Params.bDisableMergeTimeout, isMaskSet(E131_PARAMS_MASK_MERGE_TIMEOUT));

	nSize = builder.GetSize();

	DEBUG_EXIT
	return isAdded;
}

bool E131Params::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pE131ParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return false;
	}

	return Builder(0, pBuffer, nLength, nSize);
}
