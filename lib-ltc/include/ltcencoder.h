/**
 * @file ltcencoder.h
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LTCENCODER_H_
#define LTCENCODER_H_

#include <stdint.h>

#include "ltc.h"

class LtcEncoder {
public:
	LtcEncoder();
	~LtcEncoder();

	void SetTimeCode(const struct TLtcTimeCode* pLtcTimeCode, bool nExternalClock = true);
	void Encode();
	void Send();

	void Dump();
	void DumpBuffer();

	int16_t *GetBufferPointer() {
		return m_pBuffer;
	}

	uint32_t GetBufferSize();

	static LtcEncoder* Get() {
		return s_pThis;
	}

private:
	bool GetParity(uint32_t nValue);
	void SetPolarity(uint32_t nType);
	uint8_t ReverseBits(uint8_t nBits);

private:
	uint8_t *m_pLtcBits{nullptr};
	int16_t *m_pBuffer{nullptr};
	uint32_t m_nBufferSize;
	uint32_t m_nType{0xFF};

	static LtcEncoder *s_pThis;
};

#endif /* LTCENCODER_H_ */
