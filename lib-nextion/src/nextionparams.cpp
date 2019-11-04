/**
 * @file nextionparams.cpp
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

#include "nextionparams.h"
#include "nextionparamsconst.h"
#include "nextion.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

NextionParams::NextionParams(NextionParamsStore *pNextionParamsStore):  m_pNextionParamsStore(pNextionParamsStore) {
	m_tNextionParams.nBaud = 9600;
	m_tNextionParams.nOnBoardCrystal = 14745600UL;
}

NextionParams::~NextionParams(void) {
}

bool NextionParams::Load(void) {
	m_tNextionParams.nSetList = 0;

	ReadConfigFile configfile(NextionParams::staticCallbackFunction, this);

	if (configfile.Read(NextionParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pNextionParamsStore != 0) {
			m_pNextionParamsStore->Update(&m_tNextionParams);
		}
	} else if (m_pNextionParamsStore != 0) {
		m_pNextionParamsStore->Copy(&m_tNextionParams);
	} else {
		return false;
	}

	return true;
}

void NextionParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pNextionParamsStore != 0);

	m_tNextionParams.nSetList = 0;

	ReadConfigFile config(NextionParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pNextionParamsStore->Update(&m_tNextionParams);
}

void NextionParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	if (Sscan::Uint32(pLine, NextionParamsConst::BAUD, &m_tNextionParams.nBaud) == SSCAN_OK) {
		m_tNextionParams.nSetList |= NEXTION_PARAMS_BAUD;
		return;
	}

	if (Sscan::Uint32(pLine, NextionParamsConst::CRYSTAL, &m_tNextionParams.nOnBoardCrystal) == SSCAN_OK) {
		m_tNextionParams.nSetList |= NEXTION_PARAMS_CRYSTAL;
		return;
	}

}

void NextionParams::Builder(const struct TNextionParams *pNextionParams, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != 0);

	if (pNextionParams != 0) {
		memcpy(&m_tNextionParams, pNextionParams, sizeof(struct TNextionParams));
	} else {
		m_pNextionParamsStore->Copy(&m_tNextionParams);
	}

	PropertiesBuilder builder(NextionParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(NextionParamsConst::BAUD, m_tNextionParams.nBaud, isMaskSet(NEXTION_PARAMS_BAUD));
	builder.Add(NextionParamsConst::CRYSTAL, m_tNextionParams.nOnBoardCrystal, isMaskSet(NEXTION_PARAMS_CRYSTAL));

	nSize = builder.GetSize();

	return;
}

void NextionParams::Save(uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != 0);

	if (m_pNextionParamsStore == 0) {
		nSize = 0;
		return;
	}

	Builder(0, pBuffer, nLength, nSize);

	return;
}

void NextionParams::Set(Nextion *pNextion) {
	if (isMaskSet(NEXTION_PARAMS_BAUD)) {
		pNextion->SetBaud(m_tNextionParams.nBaud);
	}

	if (isMaskSet(NEXTION_PARAMS_CRYSTAL)) {
		pNextion->SetOnBoardCrystal(m_tNextionParams.nOnBoardCrystal);
	}
}

void NextionParams::Dump(void) {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, NextionParamsConst::FILE_NAME);


	if (isMaskSet(NEXTION_PARAMS_BAUD)) {
		printf(" %s=%d\n", NextionParamsConst::BAUD, m_tNextionParams.nBaud);
	}

	if (isMaskSet(NEXTION_PARAMS_CRYSTAL)) {
		printf(" %s=%d\n", NextionParamsConst::CRYSTAL, m_tNextionParams.nOnBoardCrystal);
	}
#endif
}

void NextionParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((NextionParams*) p)->callbackFunction(s);
}

bool NextionParams::isMaskSet(uint32_t nMask) const {
	return (m_tNextionParams.nSetList & nMask) == nMask;
}
