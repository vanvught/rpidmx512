/**
 * @file oscmessageget.cpp
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

#include <stdlib.h>

#include "oscmessage.h"

typedef union pcast32 {
	int32_t i;
	float f;
	char c;
	uint32_t nl;
	uint8_t b[4];
} osc_pcast32;

void OSCMessage::ArgvUpdate(void) {
    unsigned i, argc;
    char *types, *ptr;
    osc_arg **argv;

    if (0 != m_pArgv) {
        return;
    }

    argc = m_nTypesLength - 1;
    types = m_pTypes + 1;
	ptr = reinterpret_cast<char*>(m_pData);

	argv = reinterpret_cast<osc_arg**>(calloc(argc, sizeof(osc_arg*)));

    for (i = 0; i < argc; ++i) {
		unsigned len = ArgSize(types[i], ptr);
		argv[i] = len ? reinterpret_cast<osc_arg*>(ptr) : 0;
        ptr += len;
    }

    m_pArgv = argv;
}

char OSCMessage::GetType(unsigned argc) {
	if (argc > m_nTypesLength - 1) {
		m_Result = OscMessageDeserialise::INVALID_ARGUMENT;
		return OscType::UNKNOWN;
	}

	return m_pTypes[argc + 1];
}

float OSCMessage::GetFloat(unsigned argc) {
	if (argc > m_nTypesLength) {
		m_Result = OscMessageDeserialise::INVALID_ARGUMENT;
		return 0;
	}

	if (m_pArgv == 0) {
		ArgvUpdate();
	}

	osc_pcast32 val32;
	val32.i = *reinterpret_cast<int32_t*>(m_pArgv[argc]);

	return val32.f;
}

int OSCMessage::GetInt(unsigned argc) {
	if (argc > m_nTypesLength) {
		m_Result = OscMessageDeserialise::INVALID_ARGUMENT;
		return 0;
	}

	if (m_pArgv == 0) {
		ArgvUpdate();
	}

	return *reinterpret_cast<int32_t*>(m_pArgv[argc]);
}

char *OSCMessage::GetString(unsigned argc) {
	if (argc > m_nTypesLength) {
		m_Result = OscMessageDeserialise::INVALID_ARGUMENT;
		return 0;
	}

	if (m_pArgv == 0) {
		ArgvUpdate();
	}

	return reinterpret_cast<char*>(m_pArgv[argc]);
}

OSCBlob OSCMessage::GetBlob(unsigned argc) {
	if (argc > m_nTypesLength) {
		m_Result = OscMessageDeserialise::INVALID_ARGUMENT;
		return OSCBlob(0, 0);
	}

	if (m_pArgv == 0) {
		ArgvUpdate();
	}

	void *data = m_pArgv[argc];

	int nSize = *reinterpret_cast<int32_t*>(data);
	const uint8_t *pData = reinterpret_cast<const uint8_t*>(data) + 4;

	return OSCBlob(pData, nSize);
}
