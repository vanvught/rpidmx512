/**
 * @file ws28xxparams.cpp
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#define BOOL2STRING(b)			(b) ? "Yes" : "No"

#include "ws28xxdmxparams.h"

#include "ws28xx.h"
#include "ws28xxconst.h"
#include "ws28xxdmx.h"

#include "rgbmapping.h"

#include "lightset.h"
#include "lightsetconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "devicesparamsconst.h"

WS28xxDmxParams::WS28xxDmxParams(WS28xxDmxParamsStore *pWS28XXStripeParamsStore): m_pWS28xxParamsStore(pWS28XXStripeParamsStore) {
	m_tWS28xxParams.nSetList = 0;
	m_tWS28xxParams.tLedType = WS2812B;
	m_tWS28xxParams.nLedCount = 170;
	m_tWS28xxParams.nDmxStartAddress = 1;
	m_tWS28xxParams.bLedGrouping = false;
	m_tWS28xxParams.nSpiSpeedHz = WS2801_SPI_SPEED_DEFAULT_HZ;
	m_tWS28xxParams.nGlobalBrightness = 0xFF;
	m_tWS28xxParams.nActiveOutputs = 1;
	m_tWS28xxParams.bUseSI5351A = false;
	m_tWS28xxParams.nLedGroupCount = DMX_UNIVERSE_SIZE;
	m_tWS28xxParams.nRgbMapping = RGB_MAPPING_UNDEFINED;
	m_tWS28xxParams.nLowCode = 0;
	m_tWS28xxParams.nHighCode = 0;
}

WS28xxDmxParams::~WS28xxDmxParams(void) {
}

bool WS28xxDmxParams::Load(void) {
	m_tWS28xxParams.nSetList = 0;

	ReadConfigFile configfile(WS28xxDmxParams::staticCallbackFunction, this);

	if (configfile.Read(DevicesParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pWS28xxParamsStore != 0) {
			m_pWS28xxParamsStore->Update(&m_tWS28xxParams);
		}
	} else if (m_pWS28xxParamsStore != 0) {
		m_pWS28xxParamsStore->Copy(&m_tWS28xxParams);
	} else {
		return false;
	}

	return true;
}

void WS28xxDmxParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pWS28xxParamsStore != 0);

	if (m_pWS28xxParamsStore == 0) {
		return;
	}

	m_tWS28xxParams.nSetList = 0;

	ReadConfigFile config(WS28xxDmxParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pWS28xxParamsStore->Update(&m_tWS28xxParams);
}

void WS28xxDmxParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t nValue8;
	uint16_t nValue16;
	uint32_t nValue32;
	uint8_t nLength;
	float fValue;
	char cBuffer[16];

	nLength = 7;
	if (Sscan::Char(pLine, DevicesParamsConst::LED_TYPE, cBuffer, &nLength) == SSCAN_OK) {
		cBuffer[nLength] = '\0';
		uint32_t i;

		for (i = 0; i < WS28XX_UNDEFINED; i++) {
			if (strcasecmp(cBuffer, WS28xxConst::TYPES[i]) == 0) {
				break;
			}
		}

		m_tWS28xxParams.tLedType = (TWS28XXType) i;
		m_tWS28xxParams.nSetList |= WS28XXDMX_PARAMS_MASK_LED_TYPE;

		return;
	}

	if (Sscan::Uint16(pLine, DevicesParamsConst::LED_COUNT, &nValue16) == SSCAN_OK) {
		if (nValue16 != 0 && nValue16 <= (4 * 170)) {
			m_tWS28xxParams.nLedCount = nValue16;
			m_tWS28xxParams.nSetList |= WS28XXDMX_PARAMS_MASK_LED_COUNT;
		}
		return;
	}

	nLength = 3;
	if (Sscan::Char(pLine, DevicesParamsConst::LED_RGB_MAPPING, cBuffer, &nLength) == SSCAN_OK) {
		cBuffer[nLength] = '\0';
		enum TRGBMapping tMapping;

		if ((tMapping = RGBMapping::FromString(cBuffer)) != RGB_MAPPING_UNDEFINED) {
			m_tWS28xxParams.nSetList |= WS28XXDMX_PARAMS_MASK_RGB_MAPPING;
		} else {
			m_tWS28xxParams.nSetList &= ~WS28XXDMX_PARAMS_MASK_RGB_MAPPING;
		}

		m_tWS28xxParams.nRgbMapping = (uint8_t) tMapping;

		return;
	}

	if (Sscan::Float(pLine, DevicesParamsConst::LED_T0H, &fValue) == SSCAN_OK) {
		if ((nValue8 = WS28xx::ConvertTxH(fValue)) != 0) {
			m_tWS28xxParams.nSetList |= WS28XXDMX_PARAMS_MASK_LOW_CODE;
		} else {
			m_tWS28xxParams.nSetList &= ~WS28XXDMX_PARAMS_MASK_LOW_CODE;
		}

		m_tWS28xxParams.nLowCode = nValue8;

		return;
	}

	if (Sscan::Float(pLine, DevicesParamsConst::LED_T1H, &fValue) == SSCAN_OK) {
		if ((nValue8 = WS28xx::ConvertTxH(fValue)) != 0) {
			m_tWS28xxParams.nSetList |= WS28XXDMX_PARAMS_MASK_HIGH_CODE;
		} else {
			m_tWS28xxParams.nSetList &= ~WS28XXDMX_PARAMS_MASK_HIGH_CODE;
		}

		m_tWS28xxParams.nHighCode = nValue8;

		return;
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::ACTIVE_OUT, &nValue8) == SSCAN_OK) {
		m_tWS28xxParams.nActiveOutputs = nValue8;
		m_tWS28xxParams.nSetList |= WS28XXDMX_PARAMS_MASK_ACTIVE_OUT;
		return;
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::USE_SI5351A, &nValue8) == SSCAN_OK) {
		m_tWS28xxParams.bUseSI5351A = (nValue8 != 0);
		m_tWS28xxParams.nSetList |= WS28XXDMX_PARAMS_MASK_USE_SI5351A;
		return;
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::LED_GROUPING, &nValue8) == SSCAN_OK) {
		m_tWS28xxParams.bLedGrouping = (nValue8 != 0);
		m_tWS28xxParams.nSetList |= WS28XXDMX_PARAMS_MASK_LED_GROUPING;
		return;
	}

	if (Sscan::Uint16(pLine, DevicesParamsConst::LED_GROUP_COUNT, &nValue16) == SSCAN_OK) {
		if (nValue16 != 0 && nValue16 <= (4 * 170)) {
			m_tWS28xxParams.nLedGroupCount = nValue16;
			m_tWS28xxParams.nSetList |= WS28XXDMX_PARAMS_MASK_LED_GROUP_COUNT;
		}
		return;
	}

	if (Sscan::Uint32(pLine, DevicesParamsConst::SPI_SPEED_HZ, &nValue32) == SSCAN_OK) {
		m_tWS28xxParams.nSpiSpeedHz = nValue32;
		m_tWS28xxParams.nSetList |= WS28XXDMX_PARAMS_MASK_SPI_SPEED;
		return;
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::GLOBAL_BRIGHTNESS, &nValue8) == SSCAN_OK) {
		m_tWS28xxParams.nGlobalBrightness = nValue8;
		m_tWS28xxParams.nSetList |= WS28XXDMX_PARAMS_MASK_GLOBAL_BRIGHTNESS;
		return;
	}

	if (Sscan::Uint16(pLine, LightSetConst::PARAMS_DMX_START_ADDRESS, &nValue16) == SSCAN_OK) {
		if (nValue16 != 0 && nValue16 <= DMX_UNIVERSE_SIZE) {
			m_tWS28xxParams.nDmxStartAddress = nValue16;
			m_tWS28xxParams.nSetList |= WS28XXDMX_PARAMS_MASK_DMX_START_ADDRESS;
		}
		return;
	}
}

void WS28xxDmxParams::Dump(void) {
#ifndef NDEBUG
	if (m_tWS28xxParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, DevicesParamsConst::FILE_NAME);

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_LED_TYPE)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::LED_TYPE, WS28xx::GetLedTypeString(m_tWS28xxParams.tLedType), (int) m_tWS28xxParams.tLedType);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_RGB_MAPPING)) {
		printf(" %s=%d [%s]\n", DevicesParamsConst::LED_RGB_MAPPING, RGBMapping::ToString((TRGBMapping) m_tWS28xxParams.nRgbMapping), (int) m_tWS28xxParams.nRgbMapping);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_LOW_CODE)) {
		printf(" %s=%.2f [0x%X]\n", WS28xx::ConvertTxH(m_tWS28xxParams.nLowCode), (int) m_tWS28xxParams.nLowCode);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_HIGH_CODE)) {
		printf(" %s=%.2f [0x%X]\n", WS28xx::ConvertTxH(m_tWS28xxParams.nHighCode), (int) m_tWS28xxParams.nHighCode);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_LED_COUNT)) {
		printf(" %s=%d\n", DevicesParamsConst::LED_COUNT, (int) m_tWS28xxParams.nLedCount);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_ACTIVE_OUT)) {
		printf(" %s=%d\n", DevicesParamsConst::ACTIVE_OUT, (int) m_tWS28xxParams.nActiveOutputs);
	}

	if(isMaskSet(WS28XXDMX_PARAMS_MASK_LED_GROUPING)) {
		printf(" %s=%d [%s]\n", DevicesParamsConst::LED_GROUPING, (int) m_tWS28xxParams.bLedGrouping, BOOL2STRING(m_tWS28xxParams.bLedGrouping));
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_LED_GROUP_COUNT)) {
		printf(" %s=%d\n", DevicesParamsConst::LED_GROUP_COUNT, (int) m_tWS28xxParams.nLedGroupCount);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_SPI_SPEED)) {
		printf(" %s=%d\n", DevicesParamsConst::SPI_SPEED_HZ, (int) m_tWS28xxParams.nSpiSpeedHz);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_GLOBAL_BRIGHTNESS)) {
		printf(" %s=%d\n", DevicesParamsConst::GLOBAL_BRIGHTNESS, (int) m_tWS28xxParams.nGlobalBrightness);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_DMX_START_ADDRESS)) {
		printf(" %s=%d\n", LightSetConst::PARAMS_DMX_START_ADDRESS, (int) m_tWS28xxParams.nDmxStartAddress);
	}
#endif
}

void WS28xxDmxParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((WS28xxDmxParams *) p)->callbackFunction(s);
}
