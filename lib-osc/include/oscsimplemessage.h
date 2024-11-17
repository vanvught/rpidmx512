/**
 * @file oscsimplemessage.h
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

#ifndef OSCSIMPLEMESSAGE_H_
#define OSCSIMPLEMESSAGE_H_

#include <cstdint>

#include "osc.h"
#include "oscblob.h"

class OscSimpleMessage {
public:
	OscSimpleMessage(const uint8_t *pOscMessage, uint32_t nLength);

	bool IsValid() const {
		return m_bIsValid;
	}

	int GetArgc() const {
		if (m_bIsValid) {
			return static_cast<int>(m_nArgc);
		}

		return -1;
	}

	char GetType(unsigned argc) const {
		if (argc < m_nArgc) {
			return static_cast<char>(m_pArg[argc]);
		}

		return osc::type::UNKNOWN;
	}

	float GetFloat(const unsigned argc);
	int GetInt(const unsigned argc);
	const char *GetString(const unsigned argc);
	OSCBlob GetBlob(const unsigned argc);

private:
	const uint8_t *m_pOscMessage;
	uint32_t m_nLength;
	const uint8_t *m_pArg;
	uint32_t m_nArgc { 0 };
	const uint8_t *m_pOscMessageData { nullptr };
	uint32_t m_nOscMessageDataLength { 0 };
	bool m_bIsValid { false };
};

#endif /* OSCSIMPLEMESSAGE_H_ */
