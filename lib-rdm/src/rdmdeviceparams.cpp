/**
 * @file rdmdeviceparams.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>
#include <cstdio>
#include <cassert>

#include "rdmdeviceparams.h"
#include "rdmdeviceparamsconst.h"
#include "rdmdevice.h"

#include "rdm_e120.h"

#include "network.h"
#include "hardware.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

RDMDeviceParams::RDMDeviceParams() {
	DEBUG_ENTRY
	
	memset(&m_Params, 0, sizeof(struct rdm::deviceparams::Params));
	
	m_Params.nProductCategory = E120_PRODUCT_CATEGORY_OTHER;
	m_Params.nProductDetail = E120_PRODUCT_DETAIL_OTHER;

	DEBUG_EXIT
}

void RDMDeviceParams::Load() {
	DEBUG_ENTRY

#if !defined(DISABLE_FS)
	m_Params.nSetList = 0;

	ReadConfigFile configfile(RDMDeviceParams::staticCallbackFunction, this);

	if (configfile.Read(RDMDeviceParamsConst::FILE_NAME)) {
		RDMDeviceParamsStore::Update(&m_Params);
	} else
#endif
	RDMDeviceParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void RDMDeviceParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(RDMDeviceParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	RDMDeviceParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void RDMDeviceParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint32_t nLength = RDM_DEVICE_LABEL_MAX_LENGTH;
	
	if (Sscan::Char(pLine, RDMDeviceParamsConst::LABEL, m_Params.aDeviceRootLabel, nLength) == Sscan::OK) {
		m_Params.nDeviceRootLabelLength = static_cast<uint8_t>(nLength);
		m_Params.nSetList |= rdm::deviceparams::Mask::LABEL;

		return;
	}

	uint16_t nValue16;

	if (Sscan::HexUint16(pLine, RDMDeviceParamsConst::PRODUCT_CATEGORY, nValue16) == Sscan::OK) {
		m_Params.nProductCategory = nValue16;

		if (nValue16 == E120_PRODUCT_CATEGORY_OTHER) {
			m_Params.nSetList &= ~rdm::deviceparams::Mask::PRODUCT_CATEGORY;
		} else {
			m_Params.nSetList |= rdm::deviceparams::Mask::PRODUCT_CATEGORY;
		}

		return;
	}

	if (Sscan::HexUint16(pLine, RDMDeviceParamsConst::PRODUCT_DETAIL, nValue16) == Sscan::OK) {
		m_Params.nProductDetail = nValue16;

		if (nValue16 == E120_PRODUCT_DETAIL_OTHER) {
			m_Params.nSetList &= ~rdm::deviceparams::Mask::PRODUCT_DETAIL;
		} else {
			m_Params.nSetList |= rdm::deviceparams::Mask::PRODUCT_DETAIL;
		}

		return;
	}
}

void RDMDeviceParams::Set(RDMDevice *pRDMDevice) {
	assert(pRDMDevice != nullptr);

	TRDMDeviceInfoData Info;

	if (isMaskSet(rdm::deviceparams::Mask::LABEL)) {
		Info.data = m_Params.aDeviceRootLabel;
		Info.length = m_Params.nDeviceRootLabelLength;
		pRDMDevice->SetLabel(&Info);
	}

	if (isMaskSet(rdm::deviceparams::Mask::PRODUCT_CATEGORY)) {
		pRDMDevice->SetProductCategory(m_Params.nProductCategory);
	}

	if (isMaskSet(rdm::deviceparams::Mask::PRODUCT_DETAIL)) {
		pRDMDevice->SetProductDetail(m_Params.nProductDetail);
	}
}

void RDMDeviceParams::Builder(const struct rdm::deviceparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct rdm::deviceparams::Params));
	} else {
		RDMDeviceParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(RDMDeviceParamsConst::FILE_NAME, pBuffer, nLength);

	const auto isProductCategory = isMaskSet(rdm::deviceparams::Mask::PRODUCT_CATEGORY);

	if (!isProductCategory) {
		m_Params.nProductCategory = RDMDevice::Get()->GetProductCategory();
	}

	builder.AddHex16(RDMDeviceParamsConst::PRODUCT_CATEGORY, m_Params.nProductCategory, isProductCategory);

	const auto isProductDetail = isMaskSet(rdm::deviceparams::Mask::PRODUCT_DETAIL);

	if (!isProductDetail) {
		m_Params.nProductDetail = RDMDevice::Get()->GetProductDetail();
	}

	builder.AddHex16(RDMDeviceParamsConst::PRODUCT_DETAIL, m_Params.nProductDetail, isProductDetail);

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void RDMDeviceParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<RDMDeviceParams *>(p))->callbackFunction(s);
}

void RDMDeviceParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, RDMDeviceParamsConst::FILE_NAME);
	printf(" %s=%.*s\n", RDMDeviceParamsConst::LABEL, m_Params.nDeviceRootLabelLength, m_Params.aDeviceRootLabel);
	printf(" %s=%.4x\n", RDMDeviceParamsConst::PRODUCT_CATEGORY, m_Params.nProductCategory);
	printf(" %s=%.4x\n", RDMDeviceParamsConst::PRODUCT_DETAIL, m_Params.nProductDetail);
}
