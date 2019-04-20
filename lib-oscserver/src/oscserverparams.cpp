/**
 * @file oscserverparams.cpp
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "oscserverparms.h"
#include "oscserverconst.h"
#include "osc.h"

#include "lightset.h"
#include "lightsetconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

OSCServerParams::OSCServerParams(OSCServerParamsStore *pOSCServerParamsStore): m_pOSCServerParamsStore(pOSCServerParamsStore) {
	uint8_t *p = (uint8_t *) &m_tOSCServerParams;

	for (uint32_t i = 0; i < sizeof(struct TOSCServerParams); i++) {
		*p++ = 0;
	}

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

	char value[8];
	uint8_t value8;
	uint16_t value16;
	uint8_t len;

	if (Sscan::Uint16(pLine, OSCServerConst::PARAMS_INCOMING_PORT, &value16) == SSCAN_OK) {
		if (value16 > 1023) {
			m_tOSCServerParams.nIncomingPort = value16;
			m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_INCOMING_PORT;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OSCServerConst::PARAMS_OUTGOING_PORT, &value16) == SSCAN_OK) {
		if (value16 > 1023) {
			m_tOSCServerParams.nOutgoingPort = value16;
			m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_OUTGOING_PORT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, OSCServerConst::PARAMS_TRANSMISSION, &value8) == SSCAN_OK) {
		m_tOSCServerParams.bPartialTransmission = (value8 != 0);
		m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_TRANSMISSION;
		return;
	}

	len = sizeof(m_tOSCServerParams.aPath) - 1;
	if (Sscan::Char(pLine, OSCServerConst::PARAMS_PATH, m_tOSCServerParams.aPath, &len) == SSCAN_OK) {
		m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_PATH;
		return;
	}

	len = sizeof(m_tOSCServerParams.aPathInfo) - 1;
	if (Sscan::Char(pLine, OSCServerConst::PARAMS_PATH_INFO, m_tOSCServerParams.aPathInfo, &len) == SSCAN_OK) {
		m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_PATH_INFO;
		return;
	}

	len = sizeof(m_tOSCServerParams.aPathBlackOut) - 1;
	if (Sscan::Char(pLine, OSCServerConst::PARAMS_PATH_INFO, m_tOSCServerParams.aPathBlackOut, &len) == SSCAN_OK) {
		m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_PATH_BLACKOUT;
		return;
	}

	len = 3;
	if (Sscan::Char(pLine, LightSetConst::PARAMS_OUTPUT, value, &len) == SSCAN_OK) {
		m_tOSCServerParams.tOutputType = LightSet::GetOutputType((const char *) value);
		m_tOSCServerParams.nSetList |= OSCSERVER_PARAMS_MASK_OUTPUT;
		return;
	}
}

void OSCServerParams::Dump(void) {
#ifndef NDEBUG
	if (m_tOSCServerParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, OSCServerConst::PARAMS_FILE_NAME);

	if (isMaskSet(OSCSERVER_PARAMS_MASK_OUTPUT)) {
		printf(" %s=%d [%s]\n", LightSetConst::PARAMS_OUTPUT, (int) m_tOSCServerParams.tOutputType, LightSet::GetOutputType((TLightSetOutputType) m_tOSCServerParams.tOutputType));
	}

	if (isMaskSet(OSCSERVER_PARAMS_MASK_INCOMING_PORT)) {
		printf(" %s=%d\n", OSCServerConst::PARAMS_INCOMING_PORT, (int) m_tOSCServerParams.nIncomingPort);
	}

	if (isMaskSet(OSCSERVER_PARAMS_MASK_OUTGOING_PORT)) {
		printf(" %s=%d\n", OSCServerConst::PARAMS_OUTGOING_PORT, (int) m_tOSCServerParams.nOutgoingPort);
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
#endif
}

void OSCServerParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((OSCServerParams *) p)->callbackFunction(s);
}

bool OSCServerParams::isMaskSet(uint16_t mask) const {
	return (m_tOSCServerParams.nSetList & mask) == mask;
}
