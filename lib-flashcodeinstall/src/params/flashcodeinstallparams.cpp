/**
 * @file flashcodeinstallparams.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "flashcodeinstallparams.h"
#include "flashcodeinstallparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "debug.h"

bool FlashCodeInstallParams::Load() {
	DEBUG_ENTRY

	m_nSetList = 0;

	ReadConfigFile configfile(FlashCodeInstallParams::StaticCallbackFunction, this);
	const auto b = configfile.Read(FlashCodeInstallParamsConst::FILE_NAME);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
	return b;
}

void FlashCodeInstallParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, FlashCodeInstallParamsConst::INSTALL_UBOOT, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_nSetList |= FlashCodeInstallParamsMask::INSTALL_UBOOT;
		} else {
			m_nSetList &= ~FlashCodeInstallParamsMask::INSTALL_UBOOT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, FlashCodeInstallParamsConst::INSTALL_UIMAGE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_nSetList |= FlashCodeInstallParamsMask::INSTALL_UIMAGE;
		} else {
			m_nSetList &= ~FlashCodeInstallParamsMask::INSTALL_UIMAGE;
		}
		return;
	}
}

void FlashCodeInstallParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<FlashCodeInstallParams*>(p))->callbackFunction(s);
}

void FlashCodeInstallParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, FlashCodeInstallParamsConst::FILE_NAME);

	if (isMaskSet(FlashCodeInstallParamsMask::INSTALL_UBOOT)) {
		printf(" %s=1 [Yes]\n", FlashCodeInstallParamsConst::INSTALL_UBOOT);
	}

	if (isMaskSet(FlashCodeInstallParamsMask::INSTALL_UIMAGE)) {
		printf(" %s=1 [Yes]\n", FlashCodeInstallParamsConst::INSTALL_UIMAGE);
	}
}
