/**
 * @file dmxsendmulti.cpp
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
#include <assert.h>

#include "h3/dmxsendmulti.h"
#include "h3/dmx_multi.h"

#include "debug.h"

#define MAX_PORTS (sizeof(m_bIsStarted) / sizeof(m_bIsStarted[0]))

DMXSendMulti::DMXSendMulti(void) {
	DEBUG_ENTRY

	for (unsigned i = 0; i < MAX_PORTS ; i++) {
		m_bIsStarted[i] = false;
	}

	DEBUG_EXIT
}

DMXSendMulti::~DMXSendMulti(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void DMXSendMulti::Start(uint8_t nPort) {
	DEBUG_ENTRY

	assert(nPort < MAX_PORTS);

	DEBUG_PRINTF("nPort=%d", nPort);

	if (m_bIsStarted[nPort]) {
		DEBUG_EXIT
		return;
	}

	m_bIsStarted[nPort] = true;

	SetPortDirection(nPort, DMXRDM_PORT_DIRECTION_OUTP, true);

	DEBUG_EXIT
}

void DMXSendMulti::Stop(uint8_t nPort) {
	DEBUG_ENTRY

	assert(nPort < MAX_PORTS);

	DEBUG_PRINTF("nPort=%d", nPort);

	if (!m_bIsStarted[nPort]) {
		DEBUG_EXIT
		return;
	}

	m_bIsStarted[nPort] = false;

	SetPortDirection(nPort, DMXRDM_PORT_DIRECTION_OUTP, false);

	DEBUG_EXIT
}

void DMXSendMulti::SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLength) {
	DEBUG_ENTRY

	assert(nPort < MAX_PORTS);
	assert(pData != 0);
	assert(nLength != 0);

	dmx_multi_set_port_send_data_without_sc(nPort, pData, nLength);

	DEBUG_EXIT
}
