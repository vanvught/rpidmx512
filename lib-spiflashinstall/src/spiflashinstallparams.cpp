/**
 * @file spiflashinstallparams.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <cassert>

#include "spiflashinstallparams.h"
#include "spiflashinstallparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"

bool SpiFlashInstallParams::Load() {
	m_nSetList = 0;

	ReadConfigFile configfile(SpiFlashInstallParams::staticCallbackFunction, this);
	return configfile.Read(SpiFlashInstallParamsConst::FILE_NAME);
}

void SpiFlashInstallParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, SpiFlashInstallParamsConst::INSTALL_UBOOT, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_bInstalluboot = true;
			m_nSetList |= SpiFlashInstallParamsMask::INSTALL_UBOOT;
		} else {
			m_bInstalluboot = false;
			m_nSetList &= ~SpiFlashInstallParamsMask::INSTALL_UBOOT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, SpiFlashInstallParamsConst::INSTALL_UIMAGE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_bInstalluImage = true;
			m_nSetList |= SpiFlashInstallParamsMask::INSTALL_UIMAGE;
		} else {
			m_bInstalluImage = false;
			m_nSetList &= ~SpiFlashInstallParamsMask::INSTALL_UIMAGE;
		}
		return;
	}
}

void SpiFlashInstallParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<SpiFlashInstallParams*>(p))->callbackFunction(s);
}
