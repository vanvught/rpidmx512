/**
 * @file nextion.cpp
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
#include <stdio.h>
#include <string.h>
#ifndef NDEBUG	// TODO remove
 #include <ctype.h>
#endif
#include <assert.h>

#include "nextion.h"

#include "debug.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

static const uint8_t s_aTermination[3] ALIGNED = { 0xFF, 0xFF, 0xFF};
static const uint32_t s_aValidBaud[] = {2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 256000, 512000, 921600};
#define ARRAY_SIZE	((sizeof s_aValidBaud) / (sizeof s_aValidBaud[0]))

#define TIME_OUT	(40)

enum TReturnCodes {
	RETURN_CODE_INVALID_VARIABLE_NAME = 0x1A,
	RETURN_CODE_TOUCH_EVENT = 0x65,
	RETURN_CODE_READY = 0x88
};

Nextion::Nextion(void): m_nBaud(9600) {
}

Nextion::~Nextion(void) {
}

void Nextion::SetBaud(uint32_t nBaud) {
	DEBUG_ENTRY

	for (uint32_t i = 0; i < ARRAY_SIZE; i++) {
		if (nBaud == s_aValidBaud[i]) {
			m_nBaud = nBaud;

			DEBUG_EXIT
			return;
		}
	}

	m_nBaud = 9600;

	DEBUG_EXIT
	return;
}

bool Nextion::Start(void) {
	DEBUG_ENTRY

	if (!SC16IS740::Init()) {
		DEBUG_EXIT
		return false;
	}

	SC16IS740::SetBaud(m_nBaud);

	SendCommand("bkcmd=1");
	const bool r1 = ReceiveCommandResponse();

	SendCommand("page 0");
	const bool r2 = ReceiveCommandResponse();

	SendCommand("bkcmd=2");

	InitInterrupt();

	DEBUG_PRINTF("%d %d", (int ) r1, (int ) r2);
	DEBUG_EXIT
	return r1 & r2;
}

void Nextion::Run(void) {
	if (IsInterrupt()) {
		DEBUG_PUTS(">> IsInterrupt() <<");
		if(SC16IS740::IsInterrupt()) {
			DEBUG_PUTS(">> SC16IS740::IsInterrupt() <<");
			Listen();
		}
	}
}

bool Nextion::Listen(void) {
#define TO_HEX(i)	((i) < 10) ? '0' + (i) : 'A' + ((i) - 10)

	int c;
	uint32_t nCount0xFF = 0;
	m_nCount = 0;

	while ((c = GetChar(TIME_OUT)) != -1) {
		DEBUG_PRINTF("%.2x [%c]", (int) c, isprint(c) ? c : '.');

		if ((m_nCount == 0) && (c == 0)) {
			return false;
		}

		if (c == 0xFF) {
			nCount0xFF++;
		} else {
			m_aCommandReturned[m_nCount++] = c;
			if (m_nCount == sizeof(m_aCommandReturned)) {
				break;
			}
		}

		if (nCount0xFF == 3) {
			break;
		}
	}

	if (nCount0xFF == 3) {
#ifndef NDEBUG
		switch (m_aCommandReturned[0]) {
		case RETURN_CODE_INVALID_VARIABLE_NAME:
			DEBUG_PUTS("!> Invalid Variable name or attribute <!");
			break;
		case RETURN_CODE_TOUCH_EVENT:
			HandleTouchEvent();
			break;
		case RETURN_CODE_READY:
			HandleReady();
			break;
		default:
			break;
		}
#endif
		return true;
	}

	if (m_nCount > 0) {
		HandleRawData();
		return true;
	}

	return false;
}

void Nextion::SendCommand(const char *pCommand) {
	assert(pCommand != 0);

	DEBUG_PUTS(pCommand);

	FlushRead(TIME_OUT);

	WriteBytes(reinterpret_cast<const uint8_t *>(pCommand), strlen(pCommand));
	WriteBytes(reinterpret_cast<const uint8_t *>(s_aTermination), sizeof(s_aTermination));
}

bool Nextion::ReceiveCommandResponse(void) {
	DEBUG_ENTRY

	uint32_t nSize = 4;

	ReadBytes(m_aCommandReturned, nSize, TIME_OUT);

	if (nSize != 4) {
		DEBUG_EXIT
		return false;
	}

	debug_dump(m_aCommandReturned, 4);
	DEBUG_EXIT

	return ((m_aCommandReturned[0] == 0x01) && (m_aCommandReturned[1] = 0xFF)
			&& (m_aCommandReturned[2] = 0xFF) && (m_aCommandReturned[3] = 0xFF));
}

void Nextion::SetText(const char *pObjectName, const char *pValue) {
	char componentText[80];

	int i __attribute__((unused));

	i = snprintf(componentText, sizeof componentText - 1, "%s.txt=\"%s\"", pObjectName, pValue);

	DEBUG_PRINTF("i=%d", i);

	SendCommand(componentText);
}

bool Nextion::GetText(const char *pObjectName, char *pValue, uint32_t &nLength) {
	char componentText[32];

	snprintf(componentText, sizeof componentText - 1, "get %s.txt", pObjectName);
	SendCommand(componentText);

	return ReceiveReturnedText(pValue, nLength);
}

bool Nextion::ReceiveReturnedText(char *pValue, uint32_t &nLength) {
	DEBUG2_ENTRY

	int c;
	uint32_t nCount0xFF = 0;
	uint32_t nCount = 0;
	int nReturnedCode = -1;

	if ((nReturnedCode = GetChar(TIME_OUT)) == -1) {
		DEBUG2_EXIT
		return false;
	}

	while ((c = GetChar(TIME_OUT)) != -1) {
		DEBUG_PRINTF("%.2x [%c]", (int ) c, isprint(c) ? c : '.');

		if (c == 0xFF) {
			nCount0xFF++;
		} else if (nCount < nLength) {
			pValue[nCount++] = c;
		}

		if (nCount0xFF == 3) {
			break;
		}
	}

	if (nReturnedCode != 0x70) {
		DEBUG_PRINTF("nCount=%d, nReturnedCode=%x", nCount, nReturnedCode);
		DEBUG2_EXIT
		return false;
	}

	nLength = nCount;

	DEBUG_PRINTF("nLength=%d", nLength);

	DEBUG2_EXIT
	return true;
}

void Nextion::SetValue(const char *pObjectName, uint32_t nValue) {
	char componentText[32];

	snprintf(componentText, sizeof componentText - 1, "%s.val=%d", pObjectName, nValue);
	SendCommand(componentText);
}

bool Nextion::GetValue(const char *pObjectName, uint32_t &nValue) {
	char componentText[32];

	snprintf(componentText, sizeof componentText - 1, "get %s.val", pObjectName);
	SendCommand(componentText);

	return ReceiveReturnedValue(nValue);
}

bool Nextion::ReceiveReturnedValue(uint32_t &nValue) {
	DEBUG2_ENTRY

	uint32_t nLength = 8;
	ReadBytes(m_aCommandReturned, nLength, TIME_OUT);

	debug_dump(m_aCommandReturned, 8);

	if (nLength != 8) {
		DEBUG2_EXIT
		return false;
	}

	if (m_aCommandReturned[0] != 0x71) {
		DEBUG2_EXIT
		return false;
	}

	nValue =  ((uint32_t) m_aCommandReturned[4] << 24)
			| ((uint32_t) m_aCommandReturned[3] << 16)
			| ((uint32_t) m_aCommandReturned[2] << 8)
			| ((uint32_t) m_aCommandReturned[1]);

	DEBUG_PRINTF("nValue=%d [%x]", nValue, nValue);

	DEBUG2_EXIT
	return true;
}
