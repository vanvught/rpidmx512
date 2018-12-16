/**
 * @file ws28xxparams.cpp
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
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#define BOOL2STRING(b)			(b) ? "Yes" : "No"

#include <ws28xxdmxparams.h>

#include "readconfigfile.h"
#include "sscan.h"

#include "ws28xx.h"
#include "ws28xxdmx.h"

#define SET_LED_TYPE_MASK			(1 << 0)
#define SET_LED_COUNT_MASK			(1 << 1)
#define SET_DMX_START_ADDRESS		(1 << 2)
#define SET_LED_GROUPING_MASK		(1 << 3)
#define SET_CLOCK_SPEED_MASK		(1 << 4)
#define SET_GLOBAL_BRIGHTNESS		(1 << 5)

static const char PARAMS_FILE_NAME[] ALIGNED = "devices.txt";
static const char PARAMS_LED_TYPE[] ALIGNED = "led_type";
static const char PARAMS_LED_COUNT[] ALIGNED = "led_count";
static const char PARAMS_LED_GROUPING[] ALIGNED = "led_grouping";
static const char PARAMS_DMX_START_ADDRESS[] ALIGNED = "dmx_start_address";
static const char PARAMS_CLOCK_SPEED_HZ[] ALIGNED = "clock_speed_hz";
static const char PARAMS_GLOBAL_BRIGHTNESS[] ALIGNED = "global_brightness";

#define LED_TYPES_COUNT 			9
#define LED_TYPES_MAX_NAME_LENGTH 	8
static const char led_types[LED_TYPES_COUNT][LED_TYPES_MAX_NAME_LENGTH] ALIGNED = { "WS2801\0", "WS2811\0", "WS2812\0", "WS2812B", "WS2813\0", "WS2815\0", "SK6812\0", "SK6812W", "APA102\0" };

WS28xxDmxParams::WS28xxDmxParams(WS28xxDmxParamsStore *pWS28XXStripeParamsStore): m_pWS28xxParamsStore(pWS28XXStripeParamsStore) {
	m_tWS28xxParams.nSetList = 0;
	m_tWS28xxParams.tLedType = WS2801;
	m_tWS28xxParams.nLedCount = 170;
	m_tWS28xxParams.nDmxStartAddress = 1;
	m_tWS28xxParams.bLedGrouping = 0;
	m_tWS28xxParams.nClockSpeedHz = WS2801_SPI_SPEED_DEFAULT_HZ;
	m_tWS28xxParams.nGlobalBrightness = 0xFF;
}

WS28xxDmxParams::~WS28xxDmxParams(void) {
	m_tWS28xxParams.nSetList = 0;
}

bool WS28xxDmxParams::Load(void) {
	m_tWS28xxParams.nSetList = 0;

	ReadConfigFile configfile(WS28xxDmxParams::staticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
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

void WS28xxDmxParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t value8;
	uint16_t value16;
	uint32_t value32;
	uint8_t len;
	char buffer[16];

	len = 7;
	if (Sscan::Char(pLine, PARAMS_LED_TYPE, buffer, &len) == SSCAN_OK) {
		for (uint32_t i = 0; i < LED_TYPES_COUNT; i++) {
			if (strcasecmp(buffer, led_types[i]) == 0) {
				m_tWS28xxParams.tLedType = (TWS28XXType) i;
				m_tWS28xxParams.nSetList |= SET_LED_TYPE_MASK;
				return;
			}
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_LED_COUNT, &value16) == SSCAN_OK) {
		if (value16 != 0 && value16 <= (4 * 170)) {
			m_tWS28xxParams.nLedCount = value16;
			m_tWS28xxParams.nSetList |= SET_LED_COUNT_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_LED_GROUPING, &value8) == SSCAN_OK) {
		m_tWS28xxParams.bLedGrouping = (value8 != 0);
		m_tWS28xxParams.nSetList |= SET_LED_GROUPING_MASK;
		return;
	}

	if (Sscan::Uint32(pLine, PARAMS_CLOCK_SPEED_HZ, &value32) == SSCAN_OK) {
		m_tWS28xxParams.nClockSpeedHz = value32;
		m_tWS28xxParams.nSetList |= SET_CLOCK_SPEED_MASK;
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_GLOBAL_BRIGHTNESS, &value8) == SSCAN_OK) {
		m_tWS28xxParams.nGlobalBrightness = value8;
		m_tWS28xxParams.nSetList |= SET_GLOBAL_BRIGHTNESS;
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_DMX_START_ADDRESS, &value16) == SSCAN_OK) {
		if (value16 != 0 && value16 <= 512) {
			m_tWS28xxParams.nDmxStartAddress = value16;
			m_tWS28xxParams.nSetList |= SET_DMX_START_ADDRESS;
		}
	}
}

void WS28xxDmxParams::Set(WS28xxDmx *pSpiSend) {
	assert(pSpiSend != 0);

	if (isMaskSet(SET_LED_TYPE_MASK)) {
		pSpiSend->SetLEDType(m_tWS28xxParams.tLedType);
	}

	if (isMaskSet(SET_LED_COUNT_MASK)) {
		pSpiSend->SetLEDCount(m_tWS28xxParams.nLedCount);
	}

	if (isMaskSet(SET_DMX_START_ADDRESS)) {
		pSpiSend->SetDmxStartAddress(m_tWS28xxParams.nDmxStartAddress);
	}

	if (isMaskSet(SET_CLOCK_SPEED_MASK)) {
		pSpiSend->SetClockSpeedHz(m_tWS28xxParams.nClockSpeedHz);
	}
}

void WS28xxDmxParams::Dump(void) {
#ifndef NDEBUG
	if (m_tWS28xxParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(SET_LED_TYPE_MASK)) {
		printf(" %s=%s [%d]\n", PARAMS_LED_TYPE, GetLedTypeString(m_tWS28xxParams.tLedType), (int) m_tWS28xxParams.tLedType);
	}

	if (isMaskSet(SET_LED_COUNT_MASK)) {
		printf(" %s=%d\n", PARAMS_LED_COUNT, (int) m_tWS28xxParams.nLedCount);
	}

	if(isMaskSet(SET_LED_GROUPING_MASK)) {
		printf(" %s=%d [%s]\n", PARAMS_LED_GROUPING, (int) m_tWS28xxParams.bLedGrouping, BOOL2STRING(m_tWS28xxParams.bLedGrouping));
	}

	if (isMaskSet(SET_CLOCK_SPEED_MASK)) {
		printf(" %s=%d\n", PARAMS_CLOCK_SPEED_HZ, (int) m_tWS28xxParams.nClockSpeedHz);
	}

	if (isMaskSet(SET_GLOBAL_BRIGHTNESS)) {
		printf(" %s=%d\n", PARAMS_GLOBAL_BRIGHTNESS, (int) m_tWS28xxParams.nGlobalBrightness);
	}

	if (isMaskSet(SET_DMX_START_ADDRESS)) {
		printf(" %s=%d\n", PARAMS_DMX_START_ADDRESS, (int) m_tWS28xxParams.nDmxStartAddress);
	}
#endif
}

const char* WS28xxDmxParams::GetLedTypeString(TWS28XXType tType) {
	if (tType < LED_TYPES_COUNT) {
		return led_types[tType];
	}

	return "Unknown";
}

void WS28xxDmxParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((WS28xxDmxParams *) p)->callbackFunction(s);
}

bool WS28xxDmxParams::isMaskSet(uint32_t nMask) const {
	return (m_tWS28xxParams.nSetList & nMask) == nMask;
}
