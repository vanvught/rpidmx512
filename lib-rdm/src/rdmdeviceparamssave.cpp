/**
 * @file rdmdeviceparamssave.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#include <cassert>

#include "rdmdeviceparams.h"
#include "rdmdeviceparamsconst.h"

#include "propertiesbuilder.h"

#include "debug.h"

void RDMDeviceParams::Builder(const struct TRDMDeviceParams *ptRDMDeviceParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptRDMDeviceParams != nullptr) {
		DEBUG_PUTS("ptRDMDeviceParams != 0");
		memcpy(&m_tRDMDeviceParams, ptRDMDeviceParams, sizeof(struct TRDMDeviceParams));
	} else {
		DEBUG_PUTS("ptRDMDeviceParams == 0");
		m_pRDMDeviceParamsStore->Copy(&m_tRDMDeviceParams);
	}

	PropertiesBuilder builder(RDMDeviceParamsConst::FILE_NAME, pBuffer, nLength);

	char aRootLabel[RDM_DEVICE_LABEL_MAX_LENGTH + 1];
	memcpy(aRootLabel, m_tRDMDeviceParams.aDeviceRootLabel, RDM_DEVICE_LABEL_MAX_LENGTH);
	aRootLabel[m_tRDMDeviceParams.nDeviceRootLabelLength] = '\0';
	builder.Add(RDMDeviceParamsConst::LABEL, aRootLabel, isMaskSet(RDMDeviceParamsMask::LABEL));

	builder.AddHex16(RDMDeviceParamsConst::PRODUCT_CATEGORY, m_tRDMDeviceParams.nProductCategory, isMaskSet(RDMDeviceParamsMask::PRODUCT_CATEGORY));
	builder.AddHex16(RDMDeviceParamsConst::PRODUCT_DETAIL, m_tRDMDeviceParams.nProductDetail, isMaskSet(RDMDeviceParamsMask::PRODUCT_DETAIL));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void RDMDeviceParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pRDMDeviceParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);

	DEBUG_EXIT
}
