/**
 * @file e131paramsdump.cpp
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstdio>
#include <algorithm>

#include "e131params.h"
#include "e131paramsconst.h"

#include "lightset.h"
#include "lightsetparamsconst.h"

#include "debug.h"

using namespace e131;
using namespace e131params;

void E131Params::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, E131ParamsConst::FILE_NAME);

#if defined (OUTPUT_DMX_ARTNET)
	if (isMaskSet(Mask::UNIVERSE)) {
		printf(" %s=%d\n", LightSetParamsConst::UNIVERSE, m_Params.nUniverse);
	}

	if (isMaskSet(Mask::MERGE_MODE)) {
		printf(" %s=%s\n", LightSetParamsConst::MERGE_MODE, lightset::get_merge_mode(m_Params.nMergeMode));
	}
#endif

	const auto nPorts = static_cast<uint32_t>(std::min(e131params::MAX_PORTS, E131::PORTS));

	for (uint32_t i = 0; i < nPorts; i++) {
		if (isMaskSet(Mask::UNIVERSE_A << i)) {
			printf(" %s=%d\n", LightSetParamsConst::UNIVERSE_PORT[i], m_Params.nUniversePort[i]);
		}
	}

	for (uint32_t i = 0; i < nPorts; i++) {
		if (isMaskSet(Mask::MERGE_MODE_A << i)) {
			printf(" %s=%s\n", LightSetParamsConst::MERGE_MODE_PORT[i], lightset::get_merge_mode(m_Params.nMergeModePort[i]));
		}
	}

	if (isMaskSet(Mask::DISABLE_NETWORK_DATA_LOSS_TIMEOUT)) {
		printf(" %s=1 [Yes]\n", E131ParamsConst::DISABLE_NETWORK_DATA_LOSS_TIMEOUT);
	}

	if (isMaskSet(Mask::DISABLE_MERGE_TIMEOUT)) {
		printf(" %s=1 [Yes]\n", E131ParamsConst::DISABLE_MERGE_TIMEOUT);
	}

	for (uint32_t i = 0; i < nPorts; i++) {
		printf(" %s=%d [%s]\n", LightSetParamsConst::DIRECTION[i], (m_Params.nDirection >> i) & 0x1, lightset::get_direction(i, m_Params.nDirection));
	}

	for (uint32_t i = 0; i < nPorts; i++) {
		if (isMaskSet(Mask::PRIORITY_A << i)) {
			printf(" %s=%d\n", E131ParamsConst::PRIORITY[i], m_Params.nPriority[i]);
		}
	}
#endif
}
