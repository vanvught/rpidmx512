/**
 * @file oscserverparams.cpp
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "oscserverparms.h"
#include "oscserverconst.h"
#include "osc.h"

#include "lightsetconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

OSCServerParams::OSCServerParams(OSCServerParamsStore *pOSCServerParamsStore): m_pOSCServerParamsStore(pOSCServerParamsStore) {
	memset(&m_tOSCServerParams, 0, sizeof(struct TOSCServerParams));
	m_tOSCServerParams.nIncomingPort = OSC_DEFAULT_INCOMING_PORT;
	m_tOSCServerParams.nOutgoingPort = OSC_DEFAULT_OUTGOING_PORT;
}

OSCServerParams::~OSCServerParams(void) {
	m_tOSCServerParams.nSetList = 0;
}

bool OSCServerParams::Load(void) {
	m_tOSCServerParams.nSetList = 0;

	ReadConfigFile configfile(OSCServerParams::staticCallbackFunction, this);

	if (configfile.Read(OSCServerConst::PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pOSCServerParamsStore != 0) {
			m_pOSCServerParamsStore->Update(&m_tOSCServerParams);
		}
	} else if (m_pOSCServerParamsStore != 0) {
		m_pOSCServerParamsStore->Copy(&m_tOSCServerParams);
	} else {
		return false;
	}

	return true;
}

void OSCServerParams::Load(const char* pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pOSCServerParamsStore != 0);

	if (m_pOSCServerParamsStore == 0) {
		return;
	}

	m_tOSCServerParams.nSetList = 0;

	ReadConfigFile config(OSCServerParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pOSCServerParamsStore->Update(&m_tOSCServerParams);
}

void OSCServerParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t nValue8;
	uint16_t nValue16;
	uint8_t nLength;

	if (Sscan::Uint16(pLine, OSCServerConst::PARAMS_INCOMING_PORT, &nValue16) == SSCAN_OK) {
		if (nValue16 > 1023) {
			m_tOSCServerParams.nIncomingPort = nValue16;
			m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_INCOMING_PORT;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OSCServerConst::PARAMS_OUTGOING_PORT, &nValue16) == SSCAN_OK) {
		if (nValue16 > 1023) {
			m_tOSCServerParams.nOutgoingPort = nValue16;
			m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_OUTGOING_PORT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, OSCServerConst::PARAMS_TRANSMISSION, &nValue8) == SSCAN_OK) {
		m_tOSCServerParams.bPartialTransmission = (nValue8 != 0);
		m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_TRANSMISSION;
		return;
	}

	nLength = sizeof(m_tOSCServerParams.aPath) - 1;
	if (Sscan::Char(pLine, OSCServerConst::PARAMS_PATH, m_tOSCServerParams.aPath, &nLength) == SSCAN_OK) {
		m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_PATH;
		return;
	}

	nLength = sizeof(m_tOSCServerParams.aPathInfo) - 1;
	if (Sscan::Char(pLine, OSCServerConst::PARAMS_PATH_INFO, m_tOSCServerParams.aPathInfo, &nLength) == SSCAN_OK) {
		m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_PATH_INFO;
		return;
	}

	nLength = sizeof(m_tOSCServerParams.aPathBlackOut) - 1;
	if (Sscan::Char(pLine, OSCServerConst::PARAMS_PATH_INFO, m_tOSCServerParams.aPathBlackOut, &nLength) == SSCAN_OK) {
		m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_PATH_BLACKOUT;
		return;
	}

	if (Sscan::Uint8(pLine, LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, &nValue8) == SSCAN_OK) {
		m_tOSCServerParams.bEnableNoChangeUpdate = (nValue8 != 0);
		m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_ENABLE_NO_CHANGE_OUTPUT;
		return;
	}
}

void OSCServerParams::Dump(void) {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, OSCServerConst::PARAMS_FILE_NAME);

	if (isMaskSet(OSCSERVER_PARAMS_MASK_INCOMING_PORT)) {
		printf(" %s=%d\n", OSCServerConst::PARAMS_INCOMING_PORT,
				static_cast<int>(m_tOSCServerParams.nIncomingPort));
	}

	if (isMaskSet(OSCSERVER_PARAMS_MASK_OUTGOING_PORT)) {
		printf(" %s=%d\n", OSCServerConst::PARAMS_OUTGOING_PORT,
				static_cast<int>(m_tOSCServerParams.nOutgoingPort));
	}

	if (isMaskSet(OSCSERVER_PARAMS_MASK_PATH)) {
		printf(" %s=%s\n", OSCServerConst::PARAMS_PATH, m_tOSCServerParams.aPath);
	}

	if (isMaskSet(OSCSERVER_PARAMS_MASK_PATH_INFO)) {
		printf(" %s=%s\n", OSCServerConst::PARAMS_PATH_INFO, m_tOSCServerParams.aPathInfo);
	}

	if (isMaskSet(OSCSERVER_PARAMS_MASK_PATH_BLACKOUT)) {
		printf(" %s=%s\n", OSCServerConst::PARAMS_PATH_BLACKOUT, m_tOSCServerParams.aPathBlackOut);
	}

	if (isMaskSet(OSCSERVER_PARAMS_MASK_TRANSMISSION)) {
		printf(" %s=%d\n", OSCServerConst::PARAMS_TRANSMISSION, m_tOSCServerParams.bPartialTransmission);
	}

	if(isMaskSet(OSCSERVER_PARAMS_MASK_ENABLE_NO_CHANGE_OUTPUT)) {
		printf(" %s=%d\n", LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, m_tOSCServerParams.bEnableNoChangeUpdate);
	}
#endif
}

void OSCServerParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	(static_cast<OSCServerParams*>(p))->callbackFunction(s);
}

