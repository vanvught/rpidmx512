/**
 * @file devicesparams.cpp
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#if defined (__circle__)
#include <circle/logger.h>
#include <circle/stdarg.h>
#include <circle/util.h>
#include "circle/ws28xxstripe.h"
#include "circle/spisend.h"
#define ALIGNED
#else
#include "util.h"
#include "ws28xxstripe.h"
#include "spisend.h"
#endif

#include "deviceparams.h"

#include "readconfigfile.h"
#include "sscan.h"

#define SET_LED_TYPE_MASK	1<<0
#define SET_LED_COUNT_MASK	1<<1

static const char PARAMS_FILE_NAME[] ALIGNED = "devices.txt";
static const char PARAMS_LED_TYPE[] ALIGNED = "led_type";
static const char PARAMS_LED_COUNT[] ALIGNED = "led_count";

#define LED_TYPES_COUNT 			7
#define LED_TYPES_MAX_NAME_LENGTH 	8
static const char led_types[LED_TYPES_COUNT][LED_TYPES_MAX_NAME_LENGTH] ALIGNED = { "WS2801\0", "WS2811\0", "WS2812\0", "WS2812B", "WS2813\0", "SK6812\0", "SK6812W" };

void DeviceParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((DeviceParams *) p)->callbackFunction(s);
}

void DeviceParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);
	uint16_t value16;
	uint8_t len;
	char buffer[16];

	len = 7;
	if (sscan_char_p(pLine, PARAMS_LED_TYPE, buffer, &len) == SSCAN_OK) {
		for (uint8_t i = 0; i < LED_TYPES_COUNT; i++) {
			if (strcasecmp(buffer, led_types[i]) == 0) {
				tLedType = (TWS28XXType) i;
				m_bSetList |= SET_LED_TYPE_MASK;
				return;
			}
		}
		return;
	}

	if (sscan_uint16_t(pLine, PARAMS_LED_COUNT, &value16) == SSCAN_OK) {
		if (value16 != 0 && value16 <= (4 * 170)) {
			nLedCount = value16;
			m_bSetList |= SET_LED_COUNT_MASK;
		}
	}
}

DeviceParams::DeviceParams(void): m_bSetList(0) {
	tLedType = WS2801;
	nLedCount = 170;
}

DeviceParams::~DeviceParams(void) {
}

bool DeviceParams::Load(void) {
	m_bSetList = 0;

	ReadConfigFile configfile(DeviceParams::staticCallbackFunction, this);
	return configfile.Read(PARAMS_FILE_NAME);
}

void DeviceParams::Set(SPISend *pSpiSend) {
	assert(pSpiSend != 0);

	if (isMaskSet(SET_LED_TYPE_MASK)) {
		pSpiSend->SetLEDType(tLedType);
	}

	if (isMaskSet(SET_LED_COUNT_MASK)) {
		pSpiSend->SetLEDCount(nLedCount);
	}
}

void DeviceParams::Dump(void) {
	if (m_bSetList == 0) {
		return;
	}

	printf("Device parameters \'%s\':\n", PARAMS_FILE_NAME);

	if (isMaskSet(SET_LED_TYPE_MASK)) {
		printf(" Type : %s [%d]\n", led_types[tLedType], (int) tLedType);
	}

	if (isMaskSet(SET_LED_COUNT_MASK)) {
		printf(" Count : %d\n", (int) nLedCount);
	}
}

TWS28XXType DeviceParams::GetLedType(void) const {
	return tLedType;
}

uint16_t DeviceParams::GetLedCount(void) const {
	return nLedCount;
}

const char* DeviceParams::GetLedTypeString(TWS28XXType tType) {
	if (tType > SK6812W) {
		return "Unknown";
	}

	return led_types[tType];
}

bool DeviceParams::isMaskSet(uint16_t mask) const {
	return (m_bSetList & mask) == mask;
}

#if defined (__circle__)
void DeviceParams::printf(const char *fmt, ...) {
	assert(fmt != 0);

	size_t fmtlen = strlen(fmt);
	char fmtbuf[fmtlen + 1];

	strcpy(fmtbuf, fmt);

	if (fmtbuf[fmtlen - 1] == '\n') {
		fmtbuf[fmtlen - 1] = '\0';
	}

	va_list var;
	va_start(var, fmt);

	CLogger::Get()->WriteV("", LogNotice, fmtbuf, var);

	va_end(var);
}
#endif
