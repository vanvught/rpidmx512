/**
 * @file oscparams.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#define ALIGNED
#elif defined (__linux__) || defined (__CYGWIN__)
#include <string>
#define ALIGNED
#else
#include "util.h"
#endif

#include "oscparams.h"
#include "osc.h"

#include "readconfigfile.h"
#include "sscan.h"

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
		m_nIncomingPort = value16;
	} else if (Sscan::Uint16(pLine, PARAMS_OUTGOING_PORT, &value16) == SSCAN_OK) {
		m_nOutgoingPort = value16;
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
	if(m_bSetList == 0) {
		return;
	}
}

uint16_t OSCParams::GetIncomingPort(void) const {
	return m_nIncomingPort;
}

uint16_t OSCParams::GetOutgoingPort(void) const {
	return m_nOutgoingPort;
}

bool OSCParams::isMaskSet(uint16_t mask) const {
	return (m_bSetList & mask) == mask;
}

#if defined (__circle__)
void OSCParams::printf(const char *fmt, ...) {
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
