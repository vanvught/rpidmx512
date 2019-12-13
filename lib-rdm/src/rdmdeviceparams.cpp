/**
 * @file rdmdeviceparams.cpp
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
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "rdmdeviceparams.h"
#include "rdmdeviceparamsconst.h"

#include "rdm_e120.h"

#include "network.h"
#include "hardware.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "debug.h"

RDMDeviceParams::RDMDeviceParams(RDMDeviceParamsStore *pRDMDeviceParamsStore): m_pRDMDeviceParamsStore(pRDMDeviceParamsStore) {
	DEBUG_ENTRY

	m_tRDMDeviceParams.nSetList = 0;

	memset(m_tRDMDeviceParams.aDeviceRootLabel, 0, RDM_DEVICE_LABEL_MAX_LENGTH);
	m_tRDMDeviceParams.nDeviceRootLabelLength = 0;

	m_tRDMDeviceParams.nProductCategory = E120_PRODUCT_CATEGORY_OTHER;
	m_tRDMDeviceParams.nProductDetail = E120_PRODUCT_DETAIL_OTHER;

	DEBUG_EXIT
}

RDMDeviceParams::~RDMDeviceParams(void) {
	DEBUG_ENTRY

	m_tRDMDeviceParams.nSetList = 0;

	DEBUG_EXIT
}

bool RDMDeviceParams::Load(void) {
	DEBUG_ENTRY

	m_tRDMDeviceParams.nSetList = 0;

	ReadConfigFile configfile(RDMDeviceParams::staticCallbackFunction, this);

	if (configfile.Read(RDMDeviceParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pRDMDeviceParamsStore != 0) {
			m_pRDMDeviceParamsStore->Update(&m_tRDMDeviceParams);
		}
	} else if (m_pRDMDeviceParamsStore != 0) {
		m_pRDMDeviceParamsStore->Copy(&m_tRDMDeviceParams);
	} else {
		DEBUG_EXIT
		return false;
	}

	DEBUG_EXIT
	return true;
}

void RDMDeviceParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pRDMDeviceParamsStore != 0);

	if (m_pRDMDeviceParamsStore == 0) {
		DEBUG_EXIT
		return;
	}

	m_tRDMDeviceParams.nSetList = 0;

	ReadConfigFile config(RDMDeviceParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pRDMDeviceParamsStore->Update(&m_tRDMDeviceParams);

	DEBUG_EXIT
}

void RDMDeviceParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t len;
	uint16_t uint16;

	len = RDM_DEVICE_LABEL_MAX_LENGTH;
	if (Sscan::Char(pLine, RDMDeviceParamsConst::LABEL, m_tRDMDeviceParams.aDeviceRootLabel, &len) == SSCAN_OK) {
		m_tRDMDeviceParams.nDeviceRootLabelLength = len;
		m_tRDMDeviceParams.nSetList |= RDMDEVICE_PARAMS_MASK_LABEL;
		return;
	}

	if (Sscan::HexUint16(pLine, RDMDeviceParamsConst::PRODUCT_CATEGORY, &uint16) == SSCAN_OK) {
		m_tRDMDeviceParams.nProductCategory = uint16;
		m_tRDMDeviceParams.nSetList |= RDMDEVICE_PARAMS_MASK_PRODUCT_CATEGORY;
		return;
	}

	if (Sscan::HexUint16(pLine, RDMDeviceParamsConst::PRODUCT_DETAIL, &uint16) == SSCAN_OK) {
		m_tRDMDeviceParams.nProductDetail = uint16;
		m_tRDMDeviceParams.nSetList |= RDMDEVICE_PARAMS_MASK_PRODUCT_DETAIL;
	}
}

void RDMDeviceParams::Set(RDMDevice *pRDMDevice) {
	assert(pRDMDevice != 0);

	struct TRDMDeviceInfoData Info;

	if (isMaskSet(RDMDEVICE_PARAMS_MASK_LABEL)) {
		Info.data = (uint8_t*) m_tRDMDeviceParams.aDeviceRootLabel;
		Info.length = m_tRDMDeviceParams.nDeviceRootLabelLength;
		pRDMDevice->SetLabel(&Info);
	}

	if (isMaskSet(RDMDEVICE_PARAMS_MASK_PRODUCT_CATEGORY)) {
		pRDMDevice->SetProductCategory(m_tRDMDeviceParams.nProductCategory);
	}

	if (isMaskSet(RDMDEVICE_PARAMS_MASK_PRODUCT_DETAIL)) {
		pRDMDevice->SetProductDetail(m_tRDMDeviceParams.nProductDetail);
	}
}

void RDMDeviceParams::Dump(void) {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, RDMDeviceParamsConst::FILE_NAME);

	if (isMaskSet(RDMDEVICE_PARAMS_MASK_LABEL)) {
		printf(" %s=%.*s\n", RDMDeviceParamsConst::LABEL, m_tRDMDeviceParams.nDeviceRootLabelLength, m_tRDMDeviceParams.aDeviceRootLabel);
	}

	if (isMaskSet(RDMDEVICE_PARAMS_MASK_PRODUCT_CATEGORY)) {
		printf(" %s=%.4x\n", RDMDeviceParamsConst::PRODUCT_CATEGORY, m_tRDMDeviceParams.nProductCategory);
	}

	if (isMaskSet(RDMDEVICE_PARAMS_MASK_PRODUCT_DETAIL)) {
		printf(" %s=%.4x\n", RDMDeviceParamsConst::PRODUCT_DETAIL, m_tRDMDeviceParams.nProductDetail);
	}
#endif
}

void RDMDeviceParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((RDMDeviceParams *) p)->callbackFunction(s);
}

bool RDMDeviceParams::isMaskSet(uint32_t nMask) const {
	return (m_tRDMDeviceParams.nSetList & nMask) == nMask;
}
