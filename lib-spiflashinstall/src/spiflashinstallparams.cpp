/**
 * @file spiflashinstallparams.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "spiflashinstallparams.h"

#include "readconfigfile.h"
#include "sscan.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#define BOOL2STRING(b)	(b) ? "Yes" : "No"

#define INSTALL_UBOOT_MASK			(1 << 0)
#define INSTALL_UIMAGE_MASK			(1 << 1)

static const char PARAMS_FILE_NAME[] ALIGNED = "spiflash.txt";
static const char PARAMS_INSTALL_UBOOT[] ALIGNED = "install_uboot";
static const char PARAMS_INSTALL_UIMAGE[] ALIGNED = "install_uimage";

SpiFlashInstallParams::SpiFlashInstallParams(void):
		m_nSetList(0),
		m_bInstalluboot(false),
		m_bInstalluImage(false) {
}

SpiFlashInstallParams::~SpiFlashInstallParams(void) {
}

bool SpiFlashInstallParams::Load(void) {
	m_nSetList = 0;

	ReadConfigFile configfile(SpiFlashInstallParams::staticCallbackFunction, this);
	return configfile.Read(PARAMS_FILE_NAME);
}

void SpiFlashInstallParams::Dump(void) {
#ifndef NDEBUG
	if (m_nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if(isMaskSet(INSTALL_UBOOT_MASK)) {
		printf(" %s=%d [%s]\n", PARAMS_INSTALL_UBOOT, (int) m_bInstalluboot, BOOL2STRING(m_bInstalluboot));
	}

	if(isMaskSet(INSTALL_UIMAGE_MASK)) {
		printf(" %s=%d [%s]\n", PARAMS_INSTALL_UIMAGE, (int) m_bInstalluImage, BOOL2STRING(m_bInstalluImage));
	}

#endif
}

bool SpiFlashInstallParams::isMaskSet(uint32_t nMask) const {
	return (m_nSetList & nMask) == nMask;
}

void SpiFlashInstallParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((SpiFlashInstallParams *) p)->callbackFunction(s);
}

void SpiFlashInstallParams::callbackFunction(const char* pLine) {
	assert(pLine != 0);

	uint8_t value8;

	if (Sscan::Uint8(pLine, PARAMS_INSTALL_UBOOT, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_bInstalluboot = true;
			m_nSetList |= INSTALL_UBOOT_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_INSTALL_UIMAGE, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_bInstalluImage = true;
			m_nSetList |= INSTALL_UIMAGE_MASK;
		}
		return;
	}
}

