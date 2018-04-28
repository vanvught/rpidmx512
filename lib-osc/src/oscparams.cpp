/**
 * @file oscparams.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#if defined (BARE_METAL)
 #include "util.h"
#elif defined(__circle__)
 #include "circle/util.h"
#else
 #include <string.h>
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "oscparams.h"
#include "osc.h"

#include "readconfigfile.h"
#include "sscan.h"

#define SET_INCOMING_PORT_MASK	1<<0
#define SET_OUTGOING_PORT_MASK	1<<1

static const char PARAMS_FILE_NAME[] ALIGNED = "osc.txt";
static const char PARAMS_INCOMING_PORT[] ALIGNED = "incoming_port";
static const char PARAMS_OUTGOING_PORT[] ALIGNED = "outgoing_port";

void OSCParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((OSCParams *) p)->callbackFunction(s);
}

void OSCParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint16_t value16;

	if (Sscan::Uint16(pLine, PARAMS_INCOMING_PORT, &value16) == SSCAN_OK) {
		if (value16 > 1023) {
			m_nIncomingPort = value16;
			m_bSetList |= SET_INCOMING_PORT_MASK;
		}
	} else if (Sscan::Uint16(pLine, PARAMS_OUTGOING_PORT, &value16) == SSCAN_OK) {
		if (value16 > 1023) {
			m_nOutgoingPort = value16;
			m_bSetList |= SET_OUTGOING_PORT_MASK;
		}
	}
}

OSCParams::OSCParams(void): m_bSetList(0) {
	m_nIncomingPort = OSC_DEFAULT_INCOMING_PORT;
	m_nOutgoingPort = OSC_DEFAULT_OUTGOING_PORT;
}

OSCParams::~OSCParams(void) {
}

bool OSCParams::Load(void) {
	m_bSetList = 0;

	ReadConfigFile configfile(OSCParams::staticCallbackFunction, this);
	return configfile.Read(PARAMS_FILE_NAME);
}

void OSCParams::Dump(void) {
#ifndef NDEBUG
	if (m_bSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(SET_INCOMING_PORT_MASK)) {
		printf(" %s=%d\n", PARAMS_INCOMING_PORT, (int) m_nIncomingPort);
	}

	if (isMaskSet(SET_OUTGOING_PORT_MASK)) {
		printf(" %s=%d\n", PARAMS_OUTGOING_PORT, (int) m_nOutgoingPort);
	}
#endif
}

bool OSCParams::isMaskSet(uint16_t mask) const {
	return (m_bSetList & mask) == mask;
}
