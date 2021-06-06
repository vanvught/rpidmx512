/**
 * @file oscblob.h
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef OSCBLOB_H_
#define OSCBLOB_H_

#include <cstdint>

/*
 * OSC-blob
 * An int32 size count, followed by that many 8-bit bytes of arbitrary binary data,
 * followed by 0-3 additional zero bytes to make the total number of bits a multiple of 32.
 */

class OSCBlob {
public:
	OSCBlob(const uint8_t *pData, uint32_t nSize) :
			m_pData(const_cast<uint8_t*>(pData)), m_nSize(nSize) {
	}
	~OSCBlob() {
	}

	uint32_t GetDataSize() {
		return m_nSize;
	}

	const uint8_t* GetDataPtr() {
		return m_pData;
	}

	uint32_t GetSize() {
		const uint32_t nBlobSize = sizeof(int32_t) + m_nSize;
		return (4 * ((nBlobSize + 3) / 4));
	}

	uint8_t GetByte(uint32_t nIndex) {
		if (nIndex < m_nSize) {
			return m_pData[nIndex];
		}
		return 0;
	}

private:
	uint8_t *m_pData;
	uint32_t m_nSize;
};

#endif /* OSCBLOB_H_ */
