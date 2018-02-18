/**
 * @file tlc59711dmxparams.cpp
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#if defined (__linux__)
 #include <string.h>
#elif defined (__circle__)
 #include "circle/util.h"
#else
 #include "util.h"
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__((aligned(4)))
#endif

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#include "readconfigfile.h"
#include "sscan.h"

#define SET_LED_TYPE_MASK	1<<0
#define SET_LED_COUNT_MASK	1<<1
#define SET_SPI_SPEED_MASK	1<<2

static const char PARAMS_FILE_NAME[] ALIGNED = "devices.txt";
static const char PARAMS_LED_TYPE[] ALIGNED = "led_type";
static const char PARAMS_LED_COUNT[] ALIGNED = "led_count";
static const char PARAMS_SPI_SPEED_HZ[] ALIGNED = "spi_speed_hz";

TLC59711DmxParams::TLC59711DmxParams(void):
	m_bSetList(0),
	m_nSpiSpeedHz(0),
	m_LEDType(TTLC59711_TYPE_RGB),
	m_nLEDCount(4)
{
	ReadConfigFile configfile(TLC59711DmxParams::staticCallbackFunction, this);
	configfile.Read(PARAMS_FILE_NAME);
}

TLC59711DmxParams::~TLC59711DmxParams(void) {
}

void TLC59711DmxParams::Set(TLC59711Dmx* pTLC59711Dmx) {
	if (m_bSetList == 0) {
		return;
	}

	if(IsMaskSet(SET_LED_TYPE_MASK)) {
		pTLC59711Dmx->SetLEDType(m_LEDType);
	}

	if(IsMaskSet(SET_LED_COUNT_MASK)) {
		pTLC59711Dmx->SetLEDCount(m_nLEDCount);
	}

	if(IsMaskSet(SET_SPI_SPEED_MASK)) {
		//pPwmDmxTLC59711->
	}
}

void TLC59711DmxParams::Dump(void) {
#ifndef NDEBUG
	if (m_bSetList == 0) {
		return;
	}

	printf("TLC59711DmxParams \'%s\':\n", PARAMS_FILE_NAME);

	if(IsMaskSet(SET_LED_TYPE_MASK)) {
		printf("%s=%d {RGB%s}\n", PARAMS_LED_TYPE, m_LEDType, m_LEDType == TTLC59711_TYPE_RGB ? "" : "W");
	}

	if(IsMaskSet(SET_LED_COUNT_MASK)) {
		printf("%s=%d\n", PARAMS_LED_COUNT, m_nLEDCount);
	}

	if(IsMaskSet(SET_SPI_SPEED_MASK)) {
		printf("%s=%d Hz\n", PARAMS_SPI_SPEED_HZ, m_nSpiSpeedHz);
	}
#endif
}

bool TLC59711DmxParams::IsMaskSet(uint16_t nMask) const {
	return (m_bSetList & nMask) == nMask;
}

void TLC59711DmxParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((TLC59711DmxParams *) p)->callbackFunction(s);
}

void TLC59711DmxParams::callbackFunction(const char* pLine) {
	uint8_t value8;
	uint32_t value32;
	uint8_t len;
	char buffer[4];

	len = 2;
	buffer[2] = '\0';
	if (Sscan::Char(pLine, PARAMS_LED_TYPE, buffer, &len) == SSCAN_OK) {
		if (strcasecmp(buffer, "+w") == 0) {
			m_LEDType = TTLC59711_TYPE_RGBW;
			m_bSetList |= SET_LED_TYPE_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_LED_COUNT, &value8) == SSCAN_OK) {
		if ((value8 != 0) && (value8 <= 170)) {
			m_nLEDCount = value8;
			m_bSetList |= SET_LED_COUNT_MASK;
		}
	}

	if (Sscan::Uint32(pLine, PARAMS_SPI_SPEED_HZ, &value32) == SSCAN_OK) {
		m_nSpiSpeedHz = value32;
		m_bSetList |= SET_SPI_SPEED_MASK;
	}
}
