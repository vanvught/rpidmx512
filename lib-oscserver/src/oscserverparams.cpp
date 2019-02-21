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
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "oscserverparms.h"
#include "osc.h"

#include "readconfigfile.h"
#include "sscan.h"

#define SET_INCOMING_PORT_MASK	(1 << 0)
#define SET_OUTGOING_PORT_MASK	(1 << 1)
#define SET_PATH_MASK			(1 << 2)
#define SET_TRANSMISSION_MASK	(1 << 3)
#define SET_OUTPUT_MASK			(1 << 4)
#define SET_PATH_INFO_MASK		(1 << 5)
#define SET_PATH_BLACKOUT_MASK	(1 << 6)

static const char PARAMS_FILE_NAME[] ALIGNED = "osc.txt";
static const char PARAMS_INCOMING_PORT[] ALIGNED = "incoming_port";
static const char PARAMS_OUTGOING_PORT[] ALIGNED = "outgoing_port";
static const char PARAMS_PATH[] ALIGNED = "path";
static const char PARAMS_TRANSMISSION[] ALIGNED = "partial_transmission";
static const char PARAMS_OUTPUT[] ALIGNED = "output";
static const char PARAMS_PATH_INFO[] ALIGNED = "path_info";
static const char PARAMS_PATH_BLACKOUT[] ALIGNED = "path_blackout";

void OSCServerParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((OSCServerParams *) p)->callbackFunction(s);
}

void OSCServerParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	char value[8];
	uint8_t value8;
	uint16_t value16;
	uint8_t len;

	if (Sscan::Uint16(pLine, PARAMS_INCOMING_PORT, &value16) == SSCAN_OK) {
		if (value16 > 1023) {
			m_tOSCServerParams.nIncomingPort = value16;
			m_tOSCServerParams.nSetList |= SET_INCOMING_PORT_MASK;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_OUTGOING_PORT, &value16) == SSCAN_OK) {
		if (value16 > 1023) {
			m_tOSCServerParams.nOutgoingPort = value16;
			m_tOSCServerParams.nSetList |= SET_OUTGOING_PORT_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_TRANSMISSION, &value8) == SSCAN_OK) {
		m_tOSCServerParams.bPartialTransmission = (value8 != 0);
		m_tOSCServerParams.nSetList |= SET_TRANSMISSION_MASK;
		return;
	}

	len = sizeof(m_tOSCServerParams.aPath) - 1;
	if (Sscan::Char(pLine, PARAMS_PATH, m_tOSCServerParams.aPath, &len) == SSCAN_OK) {
		m_tOSCServerParams.nSetList |= SET_PATH_MASK;
		return;
	}

	len = sizeof(m_tOSCServerParams.aPathInfo) - 1;
	if (Sscan::Char(pLine, PARAMS_PATH_INFO, m_tOSCServerParams.aPathInfo, &len) == SSCAN_OK) {
		m_tOSCServerParams.nSetList |= SET_PATH_INFO_MASK;
		return;
	}

	len = sizeof(m_tOSCServerParams.aPathBlackOut) - 1;
	if (Sscan::Char(pLine, PARAMS_PATH_INFO, m_tOSCServerParams.aPathBlackOut, &len) == SSCAN_OK) {
		m_tOSCServerParams.nSetList |= SET_PATH_BLACKOUT_MASK;
		return;
	}

	len = 3;
	if (Sscan::Char(pLine, PARAMS_OUTPUT, value, &len) == SSCAN_OK) {
		if (memcmp(value, "mon", 3) == 0) {
			m_tOSCServerParams.tOutputType = OSC_OUTPUT_TYPE_MONITOR;
		} else if (memcmp(value, "spi", 3) == 0) {
			m_tOSCServerParams.tOutputType = OSC_OUTPUT_TYPE_SPI;
		} else {
			m_tOSCServerParams.tOutputType = OSC_OUTPUT_TYPE_DMX;
		}
		m_tOSCServerParams.nSetList |= SET_OUTPUT_MASK;
		return;
	}
}

OSCServerParams::OSCServerParams(OSCServerParamsStore *pOSCServerParamsStore): m_pOSCServerParamsStore(pOSCServerParamsStore) {
	uint8_t *p = (uint8_t *) &m_tOSCServerParams;

	for (uint32_t i = 0; i < sizeof(struct TOSCServerParams); i++) {
		*p++ = 0;
	}

	m_tOSCServerParams.nIncomingPort = OSC_DEFAULT_INCOMING_PORT;
	m_tOSCServerParams.nOutgoingPort = OSC_DEFAULT_OUTGOING_PORT;
}

OSCServerParams::~OSCServerParams(void) {
}

bool OSCServerParams::Load(void) {
	m_tOSCServerParams.nSetList = 0;

	ReadConfigFile configfile(OSCServerParams::staticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
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

void OSCServerParams::Set(OscServer *pOscServer) {
	assert(pOscServer != 0);

	if (isMaskSet(SET_INCOMING_PORT_MASK)) {
		pOscServer->SetPortIncoming(m_tOSCServerParams.nIncomingPort);
	}

	if (isMaskSet(SET_OUTGOING_PORT_MASK)) {
		pOscServer->SetPortOutgoing(m_tOSCServerParams.nOutgoingPort);
	}

	if (isMaskSet(SET_PATH_MASK)) {
		pOscServer->SetPath(m_tOSCServerParams.aPath);
	}

	if (isMaskSet(SET_PATH_INFO_MASK)) {
		pOscServer->SetPathInfo(m_tOSCServerParams.aPathInfo);
	}

	if (isMaskSet(SET_PATH_BLACKOUT_MASK)) {
		pOscServer->SetPathBlackOut(m_tOSCServerParams.aPathBlackOut);
	}

	if (isMaskSet(SET_TRANSMISSION_MASK)) {
		pOscServer->SetPartialTransmission(m_tOSCServerParams.bPartialTransmission);
	}
}

void OSCServerParams::Dump(void) {
#ifndef NDEBUG
	if (m_tOSCServerParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(SET_INCOMING_PORT_MASK)) {
		printf(" %s=%d\n", PARAMS_INCOMING_PORT, (int) m_tOSCServerParams.nIncomingPort);
	}

	if (isMaskSet(SET_OUTGOING_PORT_MASK)) {
		printf(" %s=%d\n", PARAMS_OUTGOING_PORT, (int) m_tOSCServerParams.nOutgoingPort);
	}

	if (isMaskSet(SET_PATH_MASK)) {
		printf(" %s=%s\n", PARAMS_PATH, m_tOSCServerParams.aPath);
	}

	if (isMaskSet(SET_PATH_INFO_MASK)) {
		printf(" %s=%s\n", PARAMS_PATH_INFO, m_tOSCServerParams.aPathInfo);
	}

	if (isMaskSet(SET_PATH_BLACKOUT_MASK)) {
		printf(" %s=%s\n", PARAMS_PATH_BLACKOUT, m_tOSCServerParams.aPathBlackOut);
	}

	if (isMaskSet(SET_TRANSMISSION_MASK)) {
		printf(" %s=%d\n", PARAMS_TRANSMISSION, m_tOSCServerParams.bPartialTransmission);
	}

	if (isMaskSet(SET_OUTPUT_MASK)) {
		printf(" %s=%d [%s]\n", PARAMS_OUTPUT, (int) m_tOSCServerParams.tOutputType, m_tOSCServerParams.tOutputType == OSC_OUTPUT_TYPE_MONITOR ? "mon" : (m_tOSCServerParams.tOutputType == OSC_OUTPUT_TYPE_SPI ? "spi" : "dmx"));
	}
#endif
}

bool OSCServerParams::isMaskSet(uint16_t mask) const {
	return (m_tOSCServerParams.nSetList & mask) == mask;
}

