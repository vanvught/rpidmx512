/**
 * @file oscclient.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "oscclient.h"
#include "oscsimplesend.h"
#include "osc.h"

#include "debug.h"

void OscClient::Send(const char *pPath) {
	DEBUG_ENTRY

	assert(pPath != nullptr);

	if (*pPath != 0) {
		OscSimpleSend MsgSend(m_nHandle, m_nServerIP, m_nPortOutgoing, pPath, nullptr);
	}

	DEBUG_EXIT
}

void OscClient::SendCmd(uint32_t nCmd) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nCmd=%d", nCmd);

	assert(nCmd < oscclient::max::CMD_COUNT);

	Send(&m_pCmds[nCmd * oscclient::max::CMD_PATH_LENGTH]);

	DEBUG_EXIT
}
