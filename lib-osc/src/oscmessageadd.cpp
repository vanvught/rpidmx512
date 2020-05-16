/**
 * @file oscmessagadd.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <string.h>
#include <stdlib.h>

#include "oscmessage.h"
#include "oscstring.h"
#include "oscblob.h"

typedef union pcast32 {
	int32_t i;
	float f;
	char c;
	uint32_t nl;
	uint8_t b[4];
} osc_pcast32;

static unsigned next_pow2(unsigned x) {
	x -= 1;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);

	return x + 1;
}

int OSCMessage::AddTypeChar(char t) {
	if (m_nTypesLength + 1 >= m_nTypesRealSize) {
		unsigned new_typesize = m_nTypesRealSize * 2;
		char *new_types = 0;

		if (!new_typesize) {
			new_typesize = OSC_DEF_TYPE_SIZE;
		}

		new_types = reinterpret_cast<char*>(realloc(m_pTypes, new_typesize));

		if (!new_types)
			return -1;

		m_pTypes = new_types;
		m_nTypesRealSize = new_typesize;
	}

	m_pTypes[m_nTypesLength] = t;
	m_nTypesLength++;
	m_pTypes[m_nTypesLength] = '\0';

	if (m_pArgv) {
		free(m_pArgv);
		m_pArgv = 0;
	}

	return 0;
}

void *OSCMessage::AddData(unsigned s) {
	uint32_t old_dlen = m_nDatalen;

	unsigned new_datasize = m_nDatasize;
	unsigned new_datalen = m_nDatalen + s;

	void *new_data = 0;

	if (!new_datasize) {
		new_datasize = OSC_DEF_DATA_SIZE;
	}

	if (new_datalen > new_datasize) {
		new_datasize = next_pow2(new_datalen);
	}

	new_data = realloc(m_pData, new_datasize);

	if (!new_data) {
		return 0;
	}

	m_nDatalen = new_datalen;
	m_nDatasize = new_datasize;
	m_pData = new_data;

	if (m_pArgv) {
		free(m_pArgv);
		m_pArgv = 0;
	}

	return (reinterpret_cast<char*>(m_pData) + old_dlen);
}

int OSCMessage::AddFloat(float fAddFloat) {
	int32_t *pValue = reinterpret_cast<int32_t*>(AddData(sizeof(fAddFloat)));

	if (!pValue) {
		return -1;
	}

	osc_pcast32 b;
	b.f = fAddFloat;

	if (AddTypeChar(OscType::FLOAT)) {
		return -1;
	}

	*pValue = b.i;

	return 0;
}

int OSCMessage::AddInt32(int32_t nAddInt32) {
	int32_t *pValue = reinterpret_cast<int32_t*>(AddData(sizeof(nAddInt32)));

	if (!pValue) {
		return -1;
	}

	if (AddTypeChar(OscType::INT32)) {
		return -1;
	}

	*pValue = nAddInt32;

	return 0;
}

int OSCMessage::AddString(const char *pAddString) {
	const unsigned nSize = OSCString::Size(pAddString);
	char *pString = reinterpret_cast<char*>(AddData(nSize));

	if (!pString) {
		return -1;
	}

	if (AddTypeChar(OscType::STRING)) {
		return -1;
	}

	strncpy(pString, pAddString, nSize);

	return 0;
}

int OSCMessage::AddBlob(OSCBlob *pBlob) {
	const int32_t size = pBlob->GetSize();
	const int32_t dsize = pBlob->GetDataSize();

	uint8_t *pData = reinterpret_cast<uint8_t*>(AddData(static_cast<unsigned>(size)));

	if (!pData) {
		return -1;
	}

	if (AddTypeChar(OscType::BLOB)) {
		return -1;
	}

	memset(pData + size - 4, 0, 4);
	memcpy(pData, &dsize, sizeof(int32_t));
	memcpy(pData + sizeof(int32_t), pBlob->GetDataPtr(), static_cast<size_t>(dsize));

	return 0;
}

