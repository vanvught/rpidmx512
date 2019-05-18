/**
 * @file e131paramsset.cpp
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

#include <assert.h>

#include "e131params.h"
#include "e131.h"

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

void E131Params::Set(E131Bridge *pE131Bridge) {
	assert(pE131Bridge != 0);

	if (m_tE131Params.nSetList == 0) {
		return;
	}

	for (uint32_t i = 0; i < E131_PARAMS_MAX_PORTS; i++) {
		if (isMaskSet(SET_MERGE_MODE_A_MASK << i)) {
			pE131Bridge->SetMergeMode(i, (TE131Merge) m_tE131Params.nMergeModePort[i]);
		} else {
			pE131Bridge->SetMergeMode(i, (TE131Merge) m_tE131Params.nMergeMode);
		}
	}

	if(isMaskSet(SET_NETWORK_TIMEOUT)) {
		pE131Bridge->SetDisableNetworkDataLossTimeout(m_tE131Params.nNetworkTimeout < E131_NETWORK_DATA_LOSS_TIMEOUT_SECONDS);
	}

	if(isMaskSet(SET_MERGE_TIMEOUT)) {
		pE131Bridge->SetDisableMergeTimeout(m_tE131Params.bDisableMergeTimeout);
	}
}
