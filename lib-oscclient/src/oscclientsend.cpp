/**
 * @file oscclient.cpp
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
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "oscclient.h"
#include "oscsend.h"
#include "osc.h"

#include "debug.h"

void OscClient::Send(const char *pPath) {
	DEBUG_ENTRY

	assert(pPath != 0);

	if (*pPath != 0) {
		OSCSend MsgSend(m_nHandle, m_nServerIP, m_nPortOutgoing, pPath, 0);
	}

	DEBUG_EXIT
}

void OscClient::SendCmd(uint8_t nCmd) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nCmd=%d", nCmd);

	assert(nCmd < OSCCLIENT_CMD_MAX_COUNT);

	Send((const char *)&m_pCmds[nCmd * OSCCLIENT_CMD_MAX_PATH_LENGTH]);

	DEBUG_EXIT
}
