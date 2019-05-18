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
#include <assert.h>

#include "e131params.h"
#include "e131paramsconst.h"

#include "propertiesbuilder.h"

#include "lightset.h"
#include "lightsetconst.h"

#define MERGEMODE2STRING(m)		(m == E131_MERGE_HTP) ? "HTP" : "LTP"

#define SET_UNIVERSE_MASK		(1 << 0)
#define SET_MERGE_MODE_MASK		(1 << 1)
#define SET_OUTPUT_MASK			(1 << 2)
#define SET_CID_MASK			(1 << 3)
#define SET_UNIVERSE_A_MASK		(1 << 4)
#define SET_UNIVERSE_B_MASK		(1 << 5)
#define SET_UNIVERSE_C_MASK		(1 << 6)
#define SET_UNIVERSE_D_MASK		(1 << 7)
#define SET_MERGE_MODE_A_MASK	(1 << 8)
#define SET_MERGE_MODE_B_MASK	(1 << 9)
#define SET_MERGE_MODE_C_MASK	(1 << 10)
#define SET_MERGE_MODE_D_MASK	(1 << 11)
#define SET_NETWORK_TIMEOUT		(1 << 12)
#define SET_MERGE_TIMEOUT		(1 << 13)

bool E131Params::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	m_pE131ParamsStore->Copy(&m_tE131Params);

	PropertiesBuilder builder(E131ParamsConst::PARAMS_FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(LightSetConst::PARAMS_UNIVERSE, (uint32_t) m_tE131Params.nUniverse, isMaskSet(SET_UNIVERSE_MASK));
	isAdded &= builder.Add(LightSetConst::PARAMS_OUTPUT, LightSet::GetOutputType(m_tE131Params.tOutputType), isMaskSet(SET_OUTPUT_MASK));

	isAdded &= builder.Add(E131ParamsConst::PARAMS_MERGE_MODE, MERGEMODE2STRING(m_tE131Params.nMergeMode), isMaskSet(SET_MERGE_MODE_MASK));

	for (unsigned i = 0; i < E131_PARAMS_MAX_PORTS; i++) {
		isAdded &= builder.Add(E131ParamsConst::PARAMS_UNIVERSE_PORT[i], (uint32_t) m_tE131Params.nUniversePort[i], isMaskSet(SET_UNIVERSE_A_MASK << i));
		isAdded &= builder.Add(E131ParamsConst::PARAMS_MERGE_MODE_PORT[i], MERGEMODE2STRING(m_tE131Params.nMergeModePort[i]), isMaskSet(SET_MERGE_MODE_A_MASK << i));
	}

	isAdded &= builder.Add(E131ParamsConst::PARAMS_NETWORK_DATA_LOSS_TIMEOUT, m_tE131Params.nNetworkTimeout, isMaskSet(SET_NETWORK_TIMEOUT));
	isAdded &= builder.Add(E131ParamsConst::PARAMS_DISABLE_MERGE_TIMEOUT, (uint32_t) m_tE131Params.bDisableMergeTimeout, isMaskSet(SET_MERGE_TIMEOUT));

	nSize = builder.GetSize();

	return isAdded;
}
