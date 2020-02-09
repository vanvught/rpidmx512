/**
 * @file e131paramssave.cpp
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
#include <assert.h>

#include "e131params.h"
#include "e131paramsconst.h"

#include "propertiesbuilder.h"

#include "lightsetconst.h"

#include "debug.h"

#define MERGEMODE2STRING(m)		(m == E131_MERGE_HTP) ? "htp" : "ltp"

void E131Params::Builder(const struct TE131Params *ptE131Params, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (ptE131Params != 0) {
		memcpy(&m_tE131Params, ptE131Params, sizeof(struct TE131Params));
	} else {
		m_pE131ParamsStore->Copy(&m_tE131Params);
	}

	PropertiesBuilder builder(E131ParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(LightSetConst::PARAMS_UNIVERSE, (uint32_t) m_tE131Params.nUniverse, isMaskSet(E131_PARAMS_MASK_UNIVERSE));

	builder.Add(E131ParamsConst::MERGE_MODE, MERGEMODE2STRING(m_tE131Params.nMergeMode), isMaskSet(E131_PARAMS_MASK_MERGE_MODE));

	for (unsigned i = 0; i < E131_PARAMS_MAX_PORTS; i++) {
		builder.Add(E131ParamsConst::UNIVERSE_PORT[i], (uint32_t) m_tE131Params.nUniversePort[i], isMaskSet(E131_PARAMS_MASK_UNIVERSE_A << i));
		builder.Add(E131ParamsConst::MERGE_MODE_PORT[i], MERGEMODE2STRING(m_tE131Params.nMergeModePort[i]), isMaskSet(E131_PARAMS_MASK_MERGE_MODE_A << i));
	}

	builder.Add(E131ParamsConst::NETWORK_DATA_LOSS_TIMEOUT, m_tE131Params.nNetworkTimeout, isMaskSet(E131_PARAMS_MASK_NETWORK_TIMEOUT));
	builder.Add(E131ParamsConst::DISABLE_MERGE_TIMEOUT, (uint32_t) m_tE131Params.bDisableMergeTimeout, isMaskSet(E131_PARAMS_MASK_MERGE_TIMEOUT));

	builder.Add(LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, (uint32_t) m_tE131Params.bEnableNoChangeUpdate, isMaskSet(E131_PARAMS_MASK_ENABLE_NO_CHANGE_OUTPUT));

	// DMX Input
	builder.Add(E131ParamsConst::DIRECTION, m_tE131Params.nDirection == (uint8_t) E131_INPUT_PORT ? "input" : "output" , isMaskSet(E131_PARAMS_MASK_DIRECTION));
	builder.Add(E131ParamsConst::PRIORITY, m_tE131Params.nPriority, isMaskSet(E131_PARAMS_MASK_PRIORITY));

	nSize = builder.GetSize();

	DEBUG_EXIT
	return;
}

void E131Params::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pE131ParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(0, pBuffer, nLength, nSize);

	return;
}
