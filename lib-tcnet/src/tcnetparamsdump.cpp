/**
 * @file tcnetparamsdump.cpp
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <stdio.h>

#include "tcnetparams.h"
#include "tcnetparamsconst.h"

void TCNetParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, TCNetParamsConst::FILE_NAME);

	if (isMaskSet(TCNetParamsMask::NODE_NAME)) {
		printf(" %s=%s\n", TCNetParamsConst::NODE_NAME, m_tTTCNetParams.aNodeName);
	}

	if (isMaskSet(TCNetParamsMask::LAYER)) {
		printf(" %s=%d [Layer%c]\n", TCNetParamsConst::LAYER, m_tTTCNetParams.nLayer, TCNet::GetLayerName(static_cast<TCNetLayer>(m_tTTCNetParams.nLayer)));
	}

	if (isMaskSet(TCNetParamsMask::TIMECODE_TYPE)) {
		printf(" %s=%d\n", TCNetParamsConst::TIMECODE_TYPE, m_tTTCNetParams.nTimeCodeType);
	}
#endif
}
