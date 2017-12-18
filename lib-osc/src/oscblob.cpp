/**
 * @file oscblob.cpp
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "oscmessage.h"
#include "oscblob.h"

OSCBlob::OSCBlob(const char *nData, int nSize) : m_pData(nData), m_nSize(nSize) {

}

OSCBlob::~OSCBlob(void) {
	m_pData = 0;
	m_nSize = 0;
}

int OSCBlob::GetDataSize(void) const{
	return m_nSize;
}

const char *OSCBlob::GetDataPtr(void) {
	return m_pData;
}

unsigned OSCBlob::GetSize(void) const {
	const unsigned len = (unsigned) sizeof(uint32_t) + m_nSize;

	return (4 * ((len + 3) / 4));
}

int OSCBlob::GetByte(unsigned i) const {
	if (i < m_nSize) {
		return (unsigned char) m_pData[i];
	}

	return -1;
}

/**
 * @brief A function to calculate the amount of OSC message space required by a \ref osc_blob object.
 *
 * @param p
 * @return Returns the storage size in bytes, which will always be a multiple of four.
 */
unsigned OSCBlob::Size(const void *p) {
	osc_blob *b = (osc_blob *) p;
	const uint32_t len = sizeof(uint32_t) + b->size;

	return (unsigned) (4 * ((len + 3) / 4));
}

unsigned OSCBlob::Validate(void *data, unsigned size) {
	unsigned i, end, len;
	unsigned dsize;

	char *pos = (char *) data;

//	if (size < 0) {
//		return -OSC_INVALID_SIZE;
//	}

	dsize = __builtin_bswap32(*(unsigned *) data);

	if (dsize > OSC_MAX_MSG_SIZE) {
		return -OSC_INVALID_SIZE;
	}

	end = sizeof(unsigned) + dsize;
	len = 4 * ((end + 3) / 4);

	if (len > size) {
		return -OSC_INVALID_SIZE;
	}

	for (i = end; i < len; ++i) {
		if (pos[i] != '\0') {
			return -OSC_NONE_ZERO_IN_PADDING;
		}
	}

	return len;
}
