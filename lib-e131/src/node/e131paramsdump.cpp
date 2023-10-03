/**
 * @file e131paramsdump.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "e131params.h"
#include "e131paramsconst.h"

#include "lightset.h"
#include "lightsetparamsconst.h"

void E131Params::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, E131ParamsConst::FILE_NAME);

	if (isMaskSet(e131params::Mask::FAILSAFE)) {
		printf(" %s=%d [%s]\n", LightSetParamsConst::FAILSAFE, m_Params.nFailSafe, lightset::get_failsafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe)));
	}

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		if (isMaskSet(e131params::Mask::UNIVERSE_A << i)) {
			printf(" %s=%d\n", LightSetParamsConst::UNIVERSE_PORT[i], m_Params.nUniverse[i]);
		}
	}

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		printf(" %s=%s\n", LightSetParamsConst::MERGE_MODE_PORT[i], lightset::get_merge_mode(mergemode_get(i)));
	}

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		const auto portDir = static_cast<lightset::PortDir>(e131params::portdir_shif_right(m_Params.nDirection, i));
		printf(" %s=%d [%s]\n", LightSetParamsConst::DIRECTION[i], e131params::portdir_shif_right(m_Params.nDirection, i), lightset::get_direction(portDir));
	}

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		if (isMaskSet(e131params::Mask::PRIORITY_A << i)) {
			printf(" %s=%d\n", E131ParamsConst::PRIORITY[i], m_Params.nPriority[i]);
		}
	}

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		const auto nOutputStyle = static_cast<uint32_t>(isOutputStyleSet(1U << i));
		printf(" %s=%u [%s]\n", LightSetParamsConst::OUTPUT_STYLE[i], nOutputStyle, lightset::get_output_style(static_cast<lightset::OutputStyle>(nOutputStyle)));
	}

	if (isMaskSet(e131params::Mask::DISABLE_MERGE_TIMEOUT)) {
		printf(" %s=1 [Yes]\n", LightSetParamsConst::DISABLE_MERGE_TIMEOUT);
	}
#endif
}
