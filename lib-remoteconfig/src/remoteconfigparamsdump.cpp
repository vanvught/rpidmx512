/**
 * @file remoteconfigparamsdump.cpp
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

#include "remoteconfigparams.h"
#include "remoteconfigconst.h"

#include "debug.h"

void RemoteConfigParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, RemoteConfigConst::PARAMS_FILE_NAME);

	if (isMaskSet(RemoteConfigParamsMask::DISABLE)) {
		printf(" %s=%d [%s]\n", RemoteConfigConst::PARAMS_DISABLE, static_cast<int>(m_tRemoteConfigParams.bDisable), BOOL2STRING::Get(m_tRemoteConfigParams.bDisable));
	}

	if (isMaskSet(RemoteConfigParamsMask::DISABLE_WRITE)) {
		printf(" %s=%d [%s]\n", RemoteConfigConst::PARAMS_DISABLE_WRITE, static_cast<int>(m_tRemoteConfigParams.bDisableWrite), BOOL2STRING::Get(m_tRemoteConfigParams.bDisableWrite));
	}

	if (isMaskSet(RemoteConfigParamsMask::ENABLE_REBOOT)) {
		printf(" %s=%d [%s]\n", RemoteConfigConst::PARAMS_ENABLE_REBOOT, static_cast<int>(m_tRemoteConfigParams.bEnableReboot), BOOL2STRING::Get(m_tRemoteConfigParams.bEnableReboot));
	}

	if (isMaskSet(RemoteConfigParamsMask::ENABLE_UPTIME)) {
		printf(" %s=%d [%s]\n", RemoteConfigConst::PARAMS_ENABLE_UPTIME, static_cast<int>(m_tRemoteConfigParams.bEnableUptime), BOOL2STRING::Get(m_tRemoteConfigParams.bEnableUptime));
	}

	if (isMaskSet(RemoteConfigParamsMask::DISPLAY_NAME)) {
		printf(" %s=%s\n", RemoteConfigConst::PARAMS_DISPLAY_NAME, m_tRemoteConfigParams.aDisplayName);
	}
#endif
}
