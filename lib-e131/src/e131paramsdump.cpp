/**
 * @file e131paramsdump.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdio.h>

#include "e131params.h"
#include "e131paramsconst.h"

#include "lightsetconst.h"

#include "debug.h"

void E131Params::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, E131ParamsConst::FILE_NAME);

	if (isMaskSet(E131ParamsMask::UNIVERSE)) {
		printf(" %s=%d\n", LightSetConst::PARAMS_UNIVERSE, m_tE131Params.nUniverse);
	}

	for (uint32_t i = 0; i < E131_PARAMS::MAX_PORTS; i++) {
		if (isMaskSet(E131ParamsMask::UNIVERSE_A << i)) {
			printf(" %s=%d\n", LightSetConst::PARAMS_UNIVERSE_PORT[i], m_tE131Params.nUniversePort[i]);
		}
	}

	if (isMaskSet(E131ParamsMask::MERGE_MODE)) {
		printf(" %s=%s\n", LightSetConst::PARAMS_MERGE_MODE, E131::GetMergeMode(m_tE131Params.nMergeMode));
	}

	for (uint32_t i = 0; i < E131_PARAMS::MAX_PORTS; i++) {
		if (isMaskSet(E131ParamsMask::MERGE_MODE_A << i)) {
			printf(" %s=%s\n", LightSetConst::PARAMS_MERGE_MODE_PORT[i], E131::GetMergeMode(m_tE131Params.nMergeModePort[i]));
		}
	}

	if (isMaskSet(E131ParamsMask::NETWORK_TIMEOUT)) {
		printf(" %s=%.1f [%s]\n", E131ParamsConst::NETWORK_DATA_LOSS_TIMEOUT, m_tE131Params.nNetworkTimeout, (m_tE131Params.nNetworkTimeout != 0) ? "" : "Disabled");
	}

	if (isMaskSet(E131ParamsMask::MERGE_TIMEOUT)) {
		printf(" %s=%d [%s]\n", E131ParamsConst::DISABLE_MERGE_TIMEOUT, m_tE131Params.bDisableMergeTimeout, BOOL2STRING::Get(m_tE131Params.bDisableMergeTimeout));
	}

	if(isMaskSet(E131ParamsMask::ENABLE_NO_CHANGE_OUTPUT)) {
		printf(" %s=%d [%s]\n", LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, m_tE131Params.bEnableNoChangeUpdate, BOOL2STRING::Get(m_tE131Params.bEnableNoChangeUpdate));
	}

	if(isMaskSet(E131ParamsMask::DIRECTION)) {
		printf(" %s=%d [%s]\n", E131ParamsConst::DIRECTION,	m_tE131Params.nDirection, m_tE131Params.nDirection == E131_INPUT_PORT ? "Input" : "Output");
	}

	if (isMaskSet(E131ParamsMask::PRIORITY)) {
		printf(" %s=%d\n", E131ParamsConst::PRIORITY, m_tE131Params.nPriority);
	}
#endif
}
