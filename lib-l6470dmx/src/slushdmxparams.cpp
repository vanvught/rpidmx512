#if !defined(ORANGE_PI)
/**
 * @file slushdmxparams.h
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <cassert>

#include "slushdmxparams.h"
#include "slushdmxparamsconst.h"

#include "l6470dmxconst.h"

#include "slushdmx.h"

#include "lightset.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

SlushDmxParams::SlushDmxParams(SlushDmxParamsStore *pSlushDmxParamsStore): m_pSlushDmxParamsStore(pSlushDmxParamsStore) {

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	strncpy(m_aFileName, L6470DmxConst::FILE_NAME_MOTOR, sizeof(m_aFileName));
}

bool SlushDmxParams::Load(void) {
	m_tSlushDmxParams.nSetList = 0;

	ReadConfigFile configfile(SlushDmxParams::staticCallbackFunction, this);

	if (configfile.Read(SlushDmxParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pSlushDmxParamsStore != 0) {
			m_pSlushDmxParamsStore->Update(&m_tSlushDmxParams);
		}
	} else if (m_pSlushDmxParamsStore != 0) {
		m_pSlushDmxParamsStore->Copy(&m_tSlushDmxParams);
	} else {
		return false;
	}

	return true;
}

void SlushDmxParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pSlushDmxParamsStore != 0);

	if (m_pSlushDmxParamsStore == 0) {
		return;
	}

	m_tSlushDmxParams.nSetList = 0;

	ReadConfigFile config(SlushDmxParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pSlushDmxParamsStore->Update(&m_tSlushDmxParams);
}

void SlushDmxParams::callbackFunction(const char *pLine) {
	uint8_t value;
	uint16_t value16;

	if (Sscan::Uint8(pLine, SlushDmxParamsConst::USE_SPI, value) == Sscan::OK) {
		if (value != 0) {
			m_tSlushDmxParams.nUseSpiBusy = 1;
			m_tSlushDmxParams.nSetList |= SlushDmxParamsMask::USE_SPI_BUSY;
			return;
		}
	}

	if (Sscan::Uint16(pLine, SlushDmxParamsConst::DMX_START_ADDRESS_PORT_A, value16) == Sscan::OK) {
		if (value16 <= DMX_UNIVERSE_SIZE) {
			m_tSlushDmxParams.nDmxStartAddressPortA = value16;
			m_tSlushDmxParams.nSetList |= SlushDmxParamsMask::START_ADDRESS_PORT_A;
		}
		return;
	}

	if (Sscan::Uint16(pLine, SlushDmxParamsConst::DMX_START_ADDRESS_PORT_B, value16) == Sscan::OK) {
		if (value16 <= DMX_UNIVERSE_SIZE) {
			m_tSlushDmxParams.nDmxStartAddressPortB = value16;
			m_tSlushDmxParams.nSetList |= SlushDmxParamsMask::START_ADDRESS_PORT_B;
		}
		return;
	}

	if (Sscan::Uint16(pLine, SlushDmxParamsConst::DMX_FOOTPRINT_PORT_A, value16) == Sscan::OK) {
		if ((value16 > 0) && (value16 <= IO_PINS_IOPORT)) {
			m_tSlushDmxParams.nDmxFootprintPortA = value16;
			m_tSlushDmxParams.nSetList |= SlushDmxParamsMask::FOOTPRINT_PORT_A;
		}
		return;
	}

	if (Sscan::Uint16(pLine, SlushDmxParamsConst::DMX_FOOTPRINT_PORT_B, value16) == Sscan::OK) {
		if ((value16 > 0) && (value16 <= IO_PINS_IOPORT)) {
			m_tSlushDmxParams.nDmxFootprintPortB = value16;
			m_tSlushDmxParams.nSetList |= SlushDmxParamsMask::FOOTPRINT_PORT_B;
		}
		return;
	}
}

void SlushDmxParams::Set(SlushDmx *pSlushDmx) {
	assert(pSlushDmx != 0);

	if (isMaskSet(SlushDmxParamsMask::USE_SPI_BUSY)) {
		pSlushDmx->SetUseSpiBusy(m_tSlushDmxParams.nUseSpiBusy == 1);
	}

	if (isMaskSet(SlushDmxParamsMask::START_ADDRESS_PORT_A)) {
		pSlushDmx->SetDmxStartAddressPortA(m_tSlushDmxParams.nDmxStartAddressPortA);
	}

	if (isMaskSet(SlushDmxParamsMask::FOOTPRINT_PORT_A)) {
		pSlushDmx->SetDmxFootprintPortA(m_tSlushDmxParams.nDmxFootprintPortA);
	}

	if (isMaskSet(SlushDmxParamsMask::START_ADDRESS_PORT_B)) {
		pSlushDmx->SetDmxStartAddressPortB(m_tSlushDmxParams.nDmxStartAddressPortB);
	}

	if (isMaskSet(SlushDmxParamsMask::FOOTPRINT_PORT_B)) {
		pSlushDmx->SetDmxFootprintPortB(m_tSlushDmxParams.nDmxFootprintPortB);
	}
}

void SlushDmxParams::Builder(const struct TSlushDmxParams *ptSlushDmxParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != 0);

	if (ptSlushDmxParams != 0) {
		memcpy(&m_tSlushDmxParams, ptSlushDmxParams, sizeof(struct TSlushDmxParams));
	} else {
		m_pSlushDmxParamsStore->Copy(&m_tSlushDmxParams);
	}

	PropertiesBuilder builder(SlushDmxParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(SlushDmxParamsConst::USE_SPI, m_tSlushDmxParams.nUseSpiBusy, isMaskSet(SlushDmxParamsMask::USE_SPI_BUSY));

	builder.Add(SlushDmxParamsConst::DMX_START_ADDRESS_PORT_A, m_tSlushDmxParams.nDmxStartAddressPortA, isMaskSet(SlushDmxParamsMask::START_ADDRESS_PORT_A));
	builder.Add(SlushDmxParamsConst::DMX_FOOTPRINT_PORT_A, m_tSlushDmxParams.nDmxFootprintPortA, isMaskSet(SlushDmxParamsMask::FOOTPRINT_PORT_A));

	builder.Add(SlushDmxParamsConst::DMX_START_ADDRESS_PORT_B, m_tSlushDmxParams.nDmxStartAddressPortB, isMaskSet(SlushDmxParamsMask::START_ADDRESS_PORT_B));
	builder.Add(SlushDmxParamsConst::DMX_FOOTPRINT_PORT_B, m_tSlushDmxParams.nDmxFootprintPortB, isMaskSet(SlushDmxParamsMask::FOOTPRINT_PORT_B));

	nSize = builder.GetSize();
}

void SlushDmxParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	if (m_pSlushDmxParamsStore == 0) {
		nSize = 0;
		return;
	}

	Builder(0, pBuffer, nLength, nSize);
}

void SlushDmxParams::Dump(void) {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, SlushDmxParamsConst::FILE_NAME);

	if (isMaskSet(SlushDmxParamsMask::USE_SPI_BUSY)) {
		printf(" %s=%d [%s]\n", SlushDmxParamsConst::USE_SPI, m_tSlushDmxParams.nUseSpiBusy, m_tSlushDmxParams.nUseSpiBusy == 0 ? "No" : "Yes");
	}

	if (isMaskSet(SlushDmxParamsMask::START_ADDRESS_PORT_A)) {
		printf(" %s=%d\n", SlushDmxParamsConst::DMX_START_ADDRESS_PORT_A, m_tSlushDmxParams.nDmxStartAddressPortA);
	}

	if (isMaskSet(SlushDmxParamsMask::FOOTPRINT_PORT_A)) {
		printf(" %s=%d\n", SlushDmxParamsConst::DMX_FOOTPRINT_PORT_A, m_tSlushDmxParams.nDmxFootprintPortA);
	}

	if (isMaskSet(SlushDmxParamsMask::START_ADDRESS_PORT_B)) {
		printf(" %s=%d\n", SlushDmxParamsConst::DMX_START_ADDRESS_PORT_B, m_tSlushDmxParams.nDmxStartAddressPortB);
	}

	if (isMaskSet(SlushDmxParamsMask::FOOTPRINT_PORT_B)) {
		printf(" %s=%d\n", SlushDmxParamsConst::DMX_FOOTPRINT_PORT_B, m_tSlushDmxParams.nDmxFootprintPortB);
	}
#endif
}

void SlushDmxParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	(static_cast<SlushDmxParams*>(p))->callbackFunction(s);
}

#endif /* #if !defined(ORANGE_PI) */
