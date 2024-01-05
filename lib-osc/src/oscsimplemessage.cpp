/**
 * @file oscsimplemessage.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>

#include "oscsimplemessage.h"
#include "osc.h"

OscSimpleMessage::OscSimpleMessage(void *pData, unsigned nLength) : m_pOscMessage(reinterpret_cast<uint8_t *>(pData)), m_nLength(nLength) {
	auto nResult = osc::string_validate(m_pOscMessage, m_nLength);

	if (nResult < 0) {
		return;
	}

	m_pArg = &m_pOscMessage[nResult];
	auto nDataOffset = static_cast<uint32_t>(nResult);

	nResult = osc::string_validate(m_pArg, m_nLength - static_cast<unsigned>(nResult));

	if ((nResult < 0) || (m_pArg[0] != ',')) {
		return;
	}

	// Support for 1 osc-string or blob only
	if (((m_pArg[1] == 's') || (m_pArg[1] == 'b')) && (m_pArg[2] != '\0')) {
		return;
	}

	m_pArg++; // Skip ','
	m_nArgc = strlen(reinterpret_cast<const char *>(m_pArg));

	nDataOffset += static_cast<uint32_t>(nResult);

	m_pOscMessageData = &m_pOscMessage[nDataOffset];
	m_nOscMessageDataLength = m_nLength - nDataOffset;

	m_bIsValid = true;
}

float OscSimpleMessage::GetFloat(unsigned argc) {
	union pcast32 {
		int32_t i;
		float f;
	} osc_pcast32;

	if ((m_nOscMessageDataLength >= 4 * (1 + argc)) && (m_pArg[argc] == osc::type::FLOAT)) {
		osc_pcast32.i = static_cast<int32_t>(__builtin_bswap32(*reinterpret_cast<uint32_t*>((4 * argc) + m_pOscMessageData)));
		return osc_pcast32.f;
	}

	return 0;
}

int OscSimpleMessage::GetInt(unsigned argc) {
	if ((m_nOscMessageDataLength >= 4 * (1 + argc)) && (m_pArg[argc] == osc::type::INT32)) {
		return static_cast<int>(__builtin_bswap32(*reinterpret_cast<uint32_t*>((4 * argc) + m_pOscMessageData)));
	}

	return 0;
}

char* OscSimpleMessage::GetString(__attribute__ ((unused)) unsigned argc) {
	if ((m_pArg[0] == osc::type::STRING) && (m_nOscMessageDataLength == osc::string_size(reinterpret_cast<char*>(m_pOscMessageData)))) {
		return reinterpret_cast<char *>(m_pOscMessageData);
	}

	return nullptr;
}

OSCBlob OscSimpleMessage::GetBlob(__attribute__ ((unused)) unsigned argc) {
	if (m_pArg[0] == osc::type::BLOB) {

		const uint32_t nSize = __builtin_bswap32(*reinterpret_cast<uint32_t*>(m_pOscMessageData));
		const uint8_t *pData = reinterpret_cast<const uint8_t*>(m_pOscMessageData) + 4;

		if ((nSize + 4) <= m_nOscMessageDataLength) {
			return OSCBlob(pData, nSize);
		}
	}

	return OSCBlob(nullptr, 0);
}
