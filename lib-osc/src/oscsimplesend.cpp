/**
 * @file oscsimplesend.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstring>
#include <cassert>

#include "oscsimplesend.h"
#include "osc.h"
#include "oscstring.h"

#include "network.h"

#include "debug.h"

char OscSimpleSend::s_Message[osc::simple::send::BUFFER_SIZE];

// Support for path only
OscSimpleSend::OscSimpleSend(int32_t nHandle, uint32_t nIpAddress , uint16_t nPort, const char *pPath, const char *pType) {
	if (pType == nullptr) {
		const auto nPathLength = osc::string_size(pPath);
		const auto nMessageLength = nPathLength + 4U;

		assert(nMessageLength < sizeof(s_Message));

		UpdateMessage(pPath, nPathLength, 0);
		Send(nMessageLength, nHandle, nIpAddress, nPort);
	}
}

// Support for 's'
OscSimpleSend::OscSimpleSend(int32_t nHandle, uint32_t nIpAddress , uint16_t nPort, const char *pPath, const char *pType, const char *pString) {
	if ((pType != nullptr) && (*pType == 's')) {
		const auto nPathLength = osc::string_size(pPath);
		const auto nMessageLength = nPathLength + 4U + osc::string_size(pString);

		assert(nMessageLength < sizeof(s_Message));

		UpdateMessage(pPath, nPathLength, 's');

		memset(s_Message + nMessageLength - 4, 0, 4);

		assert((nPathLength + 4) < sizeof(s_Message));
		strncpy(&s_Message[nPathLength + 4], pString, sizeof(s_Message) - (nPathLength + 4));

		Send(nMessageLength, nHandle, nIpAddress, nPort);
	}
}

// Support for type 'i'
OscSimpleSend::OscSimpleSend(int32_t nHandle, uint32_t nIpAddress , uint16_t nPort, const char *pPath, const char *pType, int nValue) {
	if ((pType != nullptr) && (*pType == 'i')) {
		const auto nPathLength = osc::string_size(pPath);
		const auto nMessageLength = nPathLength + 4U + 4U;

		assert(nMessageLength < sizeof(s_Message));

		UpdateMessage(pPath, nPathLength, 'i');

		*reinterpret_cast<int32_t*>(&s_Message[nMessageLength - 4]) = static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(nValue)));

		Send(nMessageLength, nHandle, nIpAddress, nPort);
	}
}

// Support for type 'f'
OscSimpleSend::OscSimpleSend(int32_t nHandle, uint32_t nIpAddress , uint16_t nPort, const char *pPath, const char *pType, float fValue) {
	if ((pType != nullptr) && (*pType == 'f')) {
		const auto nPathLength = osc::string_size(pPath);
		const auto nMessageLength = nPathLength + 4U + 4U;

		assert(nMessageLength < sizeof(s_Message));

		UpdateMessage(pPath, nPathLength, 'f');

		union pcast32 {
			uint32_t u;
			float f;
		} osc_pcast32;

		osc_pcast32.f = fValue;

		*reinterpret_cast<int32_t*>(&s_Message[nMessageLength - 4]) = static_cast<int32_t>(__builtin_bswap32(osc_pcast32.u));

		Send(nMessageLength, nHandle, nIpAddress, nPort);
	}
}

void OscSimpleSend::UpdateMessage(const char *pPath, uint32_t nPathLength, char cType) {
	memset(s_Message + nPathLength - 4, 0, 4);
	strncpy(s_Message, pPath, sizeof(s_Message));

	assert(nPathLength < sizeof(s_Message));

	s_Message[nPathLength++] = ',';
	s_Message[nPathLength++] = cType;
	s_Message[nPathLength++] = '\0';
	s_Message[nPathLength++] = '\0';
}

void OscSimpleSend::Send(uint32_t nMessageLength, int32_t nHandle, uint32_t nIpAddress, uint16_t nPort) {
	debug_dump(s_Message, nMessageLength);

	Network::Get()->SendTo(nHandle, s_Message, nMessageLength, nIpAddress, nPort);
}
