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

#if defined (__circle__)
 #include <circle/util.h>
#elif defined (__linux__)
 #include <string.h>
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

#define SET_LED_TYPE_MASK			(1 << 0)
#define SET_LED_COUNT_MASK			(1 << 1)
#define SET_DMX_START_ADDRESS_MASK	(1 << 2)
#define SET_SPI_SPEED_MASK			(1 << 3)

static const char PARAMS_FILE_NAME[] ALIGNED = "devices.txt";
static const char PARAMS_LED_TYPE[] ALIGNED = "led_type";
static const char PARAMS_LED_COUNT[] ALIGNED = "led_count";
static const char PARAMS_DMX_START_ADDRESS[] ALIGNED = "dmx_start_address";
static const char PARAMS_SPI_SPEED_HZ[] ALIGNED = "spi_speed_hz";

TLC59711DmxParams::TLC59711DmxParams(void):
	m_bSetList(0),
	m_LEDType(TTLC59711_TYPE_RGB),
	m_nLEDCount(4),
	m_nDmxStartAddress(1),
	m_nSpiSpeedHz(0)
{
}

bool TLC59711DmxParams::Load(void) {
	ReadConfigFile configfile(TLC59711DmxParams::staticCallbackFunction, this);
	return configfile.Read(PARAMS_FILE_NAME);
}

TLC59711DmxParams::~TLC59711DmxParams(void) {
}

void TLC59711DmxParams::Set(TLC59711Dmx* pTLC59711Dmx) {
	if (m_bSetList == 0) {
		return;
	}

	if(isMaskSet(SET_LED_TYPE_MASK)) {
		pTLC59711Dmx->SetLEDType(m_LEDType);
	}

	if(isMaskSet(SET_LED_COUNT_MASK)) {
		pTLC59711Dmx->SetLEDCount(m_nLEDCount);
	}

	if(isMaskSet(SET_DMX_START_ADDRESS_MASK)) {
		pTLC59711Dmx->SetDmxStartAddress(m_nDmxStartAddress);
	}

	if(isMaskSet(SET_SPI_SPEED_MASK)) {
		pTLC59711Dmx->SetSpiSpeedHz(m_nSpiSpeedHz);
	}
}

void TLC59711DmxParams::Dump(void) {
#ifndef NDEBUG
	if (m_bSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if(isMaskSet(SET_LED_TYPE_MASK)) {
		printf(" %s=%d {RGB%s}\n", PARAMS_LED_TYPE, m_LEDType, m_LEDType == TTLC59711_TYPE_RGB ? "" : "W");
	}

	if(isMaskSet(SET_LED_COUNT_MASK)) {
		printf(" %s=%d\n", PARAMS_LED_COUNT, m_nLEDCount);
	}

	if(isMaskSet(SET_DMX_START_ADDRESS_MASK)) {
		printf(" %s=%d\n", PARAMS_DMX_START_ADDRESS, m_nDmxStartAddress);
	}

	if(isMaskSet(SET_SPI_SPEED_MASK)) {
		printf(" %s=%d Hz\n", PARAMS_SPI_SPEED_HZ, m_nSpiSpeedHz);
	}
#endif
}

bool TLC59711DmxParams::isMaskSet(uint32_t nMask) const {
	return (m_bSetList & nMask) == nMask;
}

void TLC59711DmxParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((TLC59711DmxParams *) p)->callbackFunction(s);
}

void TLC59711DmxParams::callbackFunction(const char* pLine) {
	uint8_t value8;
	uint16_t value16;
	uint32_t value32;
	uint8_t len;
	char buffer[12];

	len = 9;
	buffer[8] = '\0';
	buffer[9] = '\0';
	if (Sscan::Char(pLine, PARAMS_LED_TYPE, buffer, &len) == SSCAN_OK) {
		// There is no strncasecmp in Circle
		if ((len == 8) && (strcasecmp(buffer, "tlc59711") == 0)) {
			m_LEDType = TTLC59711_TYPE_RGB;
			m_bSetList |= SET_LED_TYPE_MASK;
		} else if ((len == 9) && (strcasecmp(buffer, "tlc59711w") == 0)) {
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
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_DMX_START_ADDRESS, &value16) == SSCAN_OK) {
		if ((value16 != 0) && (value16 <= 512)) {
			m_nDmxStartAddress = value16;
			m_bSetList |= SET_DMX_START_ADDRESS_MASK;
		}
		return;
	}

	if (Sscan::Uint32(pLine, PARAMS_SPI_SPEED_HZ, &value32) == SSCAN_OK) {
		m_nSpiSpeedHz = value32;
		m_bSetList |= SET_SPI_SPEED_MASK;
	}
}

const char* TLC59711DmxParams::GetLedTypeString(TTLC59711Type tTTLC59711Type) {
	if (tTTLC59711Type == TTLC59711_TYPE_RGB) {
		return "TLC59711 (RGB)";
	} else if (tTTLC59711Type == TTLC59711_TYPE_RGBW) {
		return "TLC59711 (RGBW)";
	}

	return "TLC59711 (Unknown)";
}

bool TLC59711DmxParams::IsSetLedType(void) const {
	return isMaskSet(SET_LED_TYPE_MASK);
}

bool TLC59711DmxParams::IsSetLedCount(void) const {
	return isMaskSet(SET_LED_COUNT_MASK);
}
