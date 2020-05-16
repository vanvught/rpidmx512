/**
 * @file oscmessage.cpp
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#include "oscmessage.h"
#include "oscstring.h"
#include "oscblob.h"
#include "osc.h"

#include "debug.h"

OSCMessage::OSCMessage(void) :
	m_pTypes(0),
	m_nTypesLength(1),
	m_nTypesRealSize(OSC_DEF_TYPE_SIZE),
	m_pData(0),
	m_nDatalen(0),
	m_nDatasize(0),
	m_pArgv(0),
	m_Result(OscMessageDeserialise::MESSAGE_NULL)
{
	m_pTypes = reinterpret_cast<char*>(calloc(OSC_DEF_TYPE_SIZE, sizeof(char)));
	m_pTypes[0] = ',';
	m_pTypes[1] = '\0';
}

OSCMessage::OSCMessage(void *nData, unsigned nMessageLength) :
	m_pTypes(0),
	m_nTypesLength(0),
	m_nTypesRealSize(0),
	m_pData(0),
	m_nDatalen(0),
	m_nDatasize(0),
	m_pArgv(0),
	m_Result(OscMessageDeserialise::INTERNAL_ERROR)
{
	if (nMessageLength == 0) {
		m_Result = OscMessageDeserialise::INVALID_INVALID_SIZE;
		return;
	}

	int nRemain = static_cast<int>(nMessageLength);
	int nValidateLength = OSCString::Validate(nData, nRemain);

    if (nValidateLength < 0) {
    	m_Result = OscMessageDeserialise::INVALID_PATH;
    	return;
    }

    nRemain -= nValidateLength;

    if (nRemain <= 0) {
    	m_Result = OscMessageDeserialise::NO_TYPE_TAG;
    	return;
	}

	char *pTypes = reinterpret_cast<char*>(nData) + nValidateLength;
    nValidateLength = OSCString::Validate(pTypes, nRemain);

    if (nValidateLength < 0) {
    	m_Result = OscMessageDeserialise::INVALID_TYPE;
      	return;
	}

	if (pTypes[0] != ',') {
		m_Result = OscMessageDeserialise::INVALID_TYPE_TAG;
		return;
	}

	nRemain -= nValidateLength;

	m_nTypesLength = strlen(pTypes);
	m_nTypesRealSize = static_cast<unsigned>(nValidateLength);
	m_pTypes = reinterpret_cast<char*>(malloc(m_nTypesRealSize));

    if (0 == m_pTypes) {
		m_Result = OscMessageDeserialise::MALLOC_ERROR;
		return;
	}

	memcpy(m_pTypes, pTypes, m_nTypesLength);

	m_pData = malloc(static_cast<size_t>(nRemain));

	if (0 == m_pData) {
		m_Result = OscMessageDeserialise::MALLOC_ERROR;
		free(m_pTypes);
		m_pTypes = 0;
		return;
	}

	memcpy(m_pData, pTypes + nValidateLength, static_cast<size_t>(nRemain));
	m_nDatalen = m_nDatasize = static_cast<unsigned>(nRemain);
	char *pData = reinterpret_cast<char*>(m_pData);

	++pTypes;											// Skip the ','
	int nArgc = static_cast<int>(m_nTypesLength) - 1;	// Skip the ','

	if (nArgc) {
		m_pArgv = reinterpret_cast<osc_arg**>(calloc(static_cast<size_t>(nArgc), sizeof(osc_arg*)));

		if (0 == m_pArgv) {
			m_Result = OscMessageDeserialise::MALLOC_ERROR;

			free(m_pTypes);
			m_pTypes = 0;

			free(m_pData);
			m_pData = 0;
			return;
		}
	}

	int i = 0;

	for (i = 0; nRemain >= 0 && i < nArgc; ++i) {
		nValidateLength = ArgValidate(pTypes[i], pData, nRemain);

		if (nValidateLength < 0) {
			m_Result = OscMessageDeserialise::INVALID_ARGUMENT;

			free(m_pTypes);
			m_pTypes = 0;

			free(m_pData);
			m_pData = 0;

			free(m_pArgv);
			m_pArgv = 0;
			return;
		}

		ArgHostEndian(pTypes[i], pData);

		m_pArgv[i] = nValidateLength ? reinterpret_cast<osc_arg*>(pData) : 0;

		nRemain -= nValidateLength;
		pData += nValidateLength;
	}

	if (0 != nRemain || i != nArgc) {
		m_Result = OscMessageDeserialise::INVALID_SIZE;

		free(m_pTypes);
		m_pTypes = 0;

		free(m_pData);
		m_pData = 0;

		free(m_pArgv);
		m_pArgv = 0;

		return;
	}

	m_Result = OscMessageDeserialise::OK;

	return;
}

OSCMessage::~OSCMessage(void) {
	if (m_pTypes) {
		free(m_pTypes);
		m_pTypes = 0;
	}

	if (m_pData) {
		free(m_pData);
		m_pData = 0;
	}

	if (m_pArgv) {
		free(m_pArgv);
		m_pArgv = 0;
	}
}

int OSCMessage::ArgValidate(char nType, void *data, int size) {
	switch (nType) {
	case OscType::TRUE:
	case OscType::FALSE:
	case OscType::NIL:
	case OscType::INFINITUM:
		return 0;
	case OscType::INT32:
	case OscType::FLOAT:
	case OscType::MIDI:
	case OscType::CHAR:
		return size >= 4 ? 4 : -OscMessageDeserialise::INVALID_SIZE;
	case OscType::INT64:
	case OscType::TIMETAG:
	case OscType::DOUBLE:
		return size >= 8 ? 8 : -OscMessageDeserialise::INVALID_SIZE;
	case OscType::STRING:
	case OscType::SYMBOL:
		return OSCString::Validate(data, size);
	case OscType::BLOB:
		return OSCBlob::Validate(data, size);
	default:
		return -OscMessageDeserialise::INVALID_TYPE;
	}

	return -OscMessageDeserialise::INTERNAL_ERROR;
}

void OSCMessage::ArgHostEndian(char nType, void *data) {
    switch (nType) {
    case OscType::INT32:
    case OscType::FLOAT:
    case OscType::BLOB:
    case OscType::CHAR:
		*reinterpret_cast<int32_t*>(data) = static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(*reinterpret_cast<int32_t*>(data))));
        break;
    case OscType::TIMETAG:
		*reinterpret_cast<int32_t*>(data) = static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(*reinterpret_cast<int32_t*>(data))));
		data = (reinterpret_cast<int32_t*>(data)) + 1;
		*reinterpret_cast<int32_t*>(data) = static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(*reinterpret_cast<int32_t*>(data))));
        break;
    case OscType::INT64:
    case OscType::DOUBLE:
		*reinterpret_cast<int64_t*>(data) = static_cast<int64_t>(__builtin_bswap64(static_cast<uint64_t>(*reinterpret_cast<int64_t*>(data))));
        break;
    case OscType::STRING:
    case OscType::SYMBOL:
    case OscType::MIDI:
    case OscType::TRUE:
    case OscType::FALSE:
    case OscType::NIL:
    case OscType::INFINITUM:
        /* these are fine */
        break;
    default:
        /* Unknown type */
        break;
    }
}
