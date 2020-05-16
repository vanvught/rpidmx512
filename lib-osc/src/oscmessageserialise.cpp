/**
 * @file oscmessageserialise.cpp
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

unsigned OSCMessage::ArgSize(char nType, void *data) {
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
		return 4;
	case OscType::INT64:
	case OscType::TIMETAG:
	case OscType::DOUBLE:
		return 8;
	case OscType::STRING:
	case OscType::SYMBOL:
		return OSCString::Size(reinterpret_cast<char*>(data));
	case OscType::BLOB:
		return static_cast<unsigned>(OSCBlob::Size(data));
	default:
		// Unknown
		return 0;
	}

	return 0;
}

void OSCMessage::ArgNetworkEndian(char nType, void *data) {
	switch (nType) {
	case OscType::INT32:
	case OscType::FLOAT:
	case OscType::BLOB:
	case OscType::CHAR:
		*reinterpret_cast<int32_t*>(data) = static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(*reinterpret_cast<int32_t*>(data))));
		break;
	case OscType::TIMETAG:
		*reinterpret_cast<uint32_t*>(data) = __builtin_bswap32(*reinterpret_cast<uint32_t*>(data));
		data = (reinterpret_cast<uint32_t*>(data)) + 1;
		*reinterpret_cast<uint32_t*>(data) = __builtin_bswap32(*reinterpret_cast<uint32_t*>(data));
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
		break;
	}
}

void *OSCMessage::Serialise(const char *path, void *to, unsigned * size) {
	unsigned i, argc;
	char *types, *ptr;
	unsigned s = OSCString::Size(path) + OSCString::Size(m_pTypes) + m_nDatalen;

    if (size) {
        *size = s;
    }

    if (!to) {
        to = calloc(1, s);
    }

	memset(reinterpret_cast<char*>(to) + OSCString::Size(path) - 4, 0, 4);

	strcpy(reinterpret_cast<char*>(to), path);

	memset(reinterpret_cast<char*>(to) + OSCString::Size(path) + OSCString::Size(m_pTypes) - 4, 0, 4);

	strcpy(reinterpret_cast<char*>(to) + OSCString::Size(path), m_pTypes);

    types = m_pTypes + 1;
	ptr = reinterpret_cast<char*>(to) + OSCString::Size(path) + OSCString::Size(m_pTypes);

    memcpy(ptr, m_pData, m_nDatalen);

    argc = m_nTypesLength - 1;

    for (i = 0; i < argc; ++i) {
		size_t len = ArgSize(types[i], ptr);
		ArgNetworkEndian(types[i], ptr);
        ptr += len;
    }

    return to;
}
