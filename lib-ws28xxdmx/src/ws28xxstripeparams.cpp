/**
 * @file ws28xxstripeparams.cpp
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#if defined (__circle__)
 #include <circle/util.h>
#elif (__linux__)
 #include <string.h>
#else
 #include "util.h"
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "ws28xxstripeparams.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "ws28xxstripe.h"
#include "ws28xxstripedmx.h"

#define SET_LED_TYPE_MASK		(1 << 0)
#define SET_LED_COUNT_MASK		(1 << 1)
#define SET_DMX_START_ADDRESS	(1 << 2)

static const char PARAMS_FILE_NAME[] ALIGNED = "devices.txt";
static const char PARAMS_LED_TYPE[] ALIGNED = "led_type";
static const char PARAMS_LED_COUNT[] ALIGNED = "led_count";
static const char PARAMS_DMX_START_ADDRESS[] ALIGNED = "dmx_start_address";

#define LED_TYPES_COUNT 			7
#define LED_TYPES_MAX_NAME_LENGTH 	8
static const char led_types[LED_TYPES_COUNT][LED_TYPES_MAX_NAME_LENGTH] ALIGNED = { "WS2801\0", "WS2811\0", "WS2812\0", "WS2812B", "WS2813\0", "SK6812\0", "SK6812W" };

void WS28XXStripeParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((WS28XXStripeParams *) p)->callbackFunction(s);
}

void WS28XXStripeParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint16_t value16;
	uint8_t len;
	char buffer[16];

	len = 7;
	if (Sscan::Char(pLine, PARAMS_LED_TYPE, buffer, &len) == SSCAN_OK) {
		for (uint8_t i = 0; i < LED_TYPES_COUNT; i++) {
			if (strcasecmp(buffer, led_types[i]) == 0) {
				m_tWS28XXStripeParams.tLedType = (TWS28XXType) i;
				m_tWS28XXStripeParams.bSetList |= SET_LED_TYPE_MASK;
				return;
			}
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_LED_COUNT, &value16) == SSCAN_OK) {
		if (value16 != 0 && value16 <= (4 * 170)) {
			m_tWS28XXStripeParams.nLedCount = value16;
			m_tWS28XXStripeParams.bSetList |= SET_LED_COUNT_MASK;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_DMX_START_ADDRESS, &value16) == SSCAN_OK) {
		if (value16 != 0 && value16 <= 512) {
			m_tWS28XXStripeParams.nDmxStartAddress = value16;
			m_tWS28XXStripeParams.bSetList |= SET_DMX_START_ADDRESS;
		}
	}
}
WS28XXStripeParams::WS28XXStripeParams(WS28XXStripeParamsStore *pWS28XXStripeParamsStore): m_pWS28XXStripeParamsStore(pWS28XXStripeParamsStore) {
	m_tWS28XXStripeParams.bSetList = 0;
	m_tWS28XXStripeParams.tLedType = WS2801;
	m_tWS28XXStripeParams.nLedCount = 170;
	m_tWS28XXStripeParams.nDmxStartAddress = 1;

}

WS28XXStripeParams::~WS28XXStripeParams(void) {
	m_tWS28XXStripeParams.bSetList = 0;
}

bool WS28XXStripeParams::Load(void) {
	m_tWS28XXStripeParams.bSetList = 0;

	ReadConfigFile configfile(WS28XXStripeParams::staticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pWS28XXStripeParamsStore != 0) {
			m_pWS28XXStripeParamsStore->Update(&m_tWS28XXStripeParams);
		}
	} else if (m_pWS28XXStripeParamsStore != 0) {
		m_pWS28XXStripeParamsStore->Copy(&m_tWS28XXStripeParams);
	} else {
		return false;
	}

	return true;
}

void WS28XXStripeParams::Set(SPISend *pSpiSend) {
	assert(pSpiSend != 0);

	if (isMaskSet(SET_LED_TYPE_MASK)) {
		pSpiSend->SetLEDType(m_tWS28XXStripeParams.tLedType);
	}

	if (isMaskSet(SET_LED_COUNT_MASK)) {
		pSpiSend->SetLEDCount(m_tWS28XXStripeParams.nLedCount);
	}

	if (isMaskSet(SET_DMX_START_ADDRESS)) {
		pSpiSend->SetDmxStartAddress(m_tWS28XXStripeParams.nDmxStartAddress);
	}
}

void WS28XXStripeParams::Dump(void) {
#ifndef NDEBUG
	if (m_tWS28XXStripeParams.bSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(SET_LED_TYPE_MASK)) {
		printf(" %s=%s [%d]\n", PARAMS_LED_TYPE, GetLedTypeString(m_tWS28XXStripeParams.tLedType), (int) m_tWS28XXStripeParams.tLedType);
	}

	if (isMaskSet(SET_LED_COUNT_MASK)) {
		printf(" %s=%d\n", PARAMS_LED_COUNT, (int) m_tWS28XXStripeParams.nLedCount);
	}

	if (isMaskSet(SET_DMX_START_ADDRESS)) {
		printf(" %s=%d\n", PARAMS_DMX_START_ADDRESS, (int) m_tWS28XXStripeParams.nDmxStartAddress);
	}
#endif
}

const char* WS28XXStripeParams::GetLedTypeString(TWS28XXType tType) {
	if (tType > SK6812W) {
		return "Unknown";
	}

	return led_types[tType];
}

bool WS28XXStripeParams::isMaskSet(uint32_t nMask) const {
	return (m_tWS28XXStripeParams.bSetList & nMask) == nMask;
}
