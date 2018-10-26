/**
 * @file oscmessage.cpp
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdlib.h>
#include <assert.h>

#if defined (BARE_METAL)
 #include "util.h"
#elif defined(__circle__)
 #include "circle/util.h"
#else
 #include <string.h>
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "oscmessage.h"
#include "oscstring.h"
#include "oscblob.h"
#include "osc.h"

extern "C" {
int lo_pattern_match(const char *, const char *);
}

#define OSC_DEF_TYPE_SIZE 8
#define OSC_DEF_DATA_SIZE 8

static unsigned next_pow2(unsigned x)
{
	x -= 1;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);

	return x + 1;
}

typedef union pcast32 {
	int32_t i;
	float f;
	char c;
	uint32_t nl;
	uint8_t b[4];
} osc_pcast32;

typedef union pcast64 {
	int64_t i;
	double f;
	uint64_t nl;
} osc_pcast64;

OSCMessage::OSCMessage(void) :
	m_Types(0),
	m_Typelen(1),
	m_Typesize(OSC_DEF_TYPE_SIZE),
	m_Data(0),
	m_Datalen(0),
	m_Datasize(0),
	m_Argv(0),
	m_Result(OSC_MESSAGE_NULL)
{
	m_Types = (char *) calloc(OSC_DEF_TYPE_SIZE, sizeof(char));
	m_Types[0] = ',';
	m_Types[1] = '\0';
}

OSCMessage::OSCMessage(void *nData, unsigned nLen) :
	m_Types(0),
	m_Typelen(0),
	m_Typesize(0),
	m_Data(0),
	m_Datalen(0),
	m_Datasize(0),
	m_Argv(0),
	m_Result(OSC_INTERNAL_ERROR)
{
	char *types = 0, *ptr = 0;
	int i, argc = 0, remain = nLen, len;

	if (nLen <= 0) {
		m_Result = OSC_INVALID__INVALID_SIZE;
		goto fail;
	}

	m_Types = 0;
	m_Typelen = 0;
	m_Typesize = 0;
	m_Data = 0;
	m_Datalen = 0;
	m_Datasize = 0;
	m_Argv = 0;

	len = OSCString::Validate(nData, remain);

    if (len < 0) {
    	m_Result = OSC_INVALID_PATH;
        goto fail;
    }

    remain -= len;

    if (remain <= 0) {
    	m_Result = OSC_NO_TYPE_TAG;
		goto fail;
	}

    types = (char *) nData + len;

    len = OSCString::Validate(types, remain);

    if (len < 0) {
    	m_Result = OSC_INVALID_TYPE;
		goto fail;
	}

	if (types[0] != ',') {
		m_Result = OSC_INVALID_TYPE_TAG;
		goto fail;
	}

	remain -= len;

	m_Typelen = strlen(types);
	m_Typesize = len;
	m_Types = (char *)malloc(m_Typesize);

    if (0 == m_Types) {
		m_Result = OSC_MALLOC_ERROR;
		goto fail;
	}

	memcpy(m_Types, types, m_Typelen);

	m_Data = malloc(remain);

	if (0 == m_Data) {
		m_Result = OSC_MALLOC_ERROR;
		goto fail;
	}

	memcpy(m_Data, types + len, remain);
	m_Datalen = m_Datasize = remain;
	ptr = (char *)m_Data;

	++types;
	argc = m_Typelen - 1;

	if (argc) {
		m_Argv = (osc_arg **)calloc(argc, sizeof(osc_arg *));

		if (0 == m_Argv) {
			m_Result = OSC_MALLOC_ERROR;
			goto fail;
		}
	}

	for (i = 0; remain >= 0 && i < argc; ++i) {
		len = ArgValidate((osc_type) types[i], ptr, remain);

		if (len < 0) {
			m_Result = OSC_INVALID_ARGUMENT;
			goto fail;
		}

		ArgHostEndian((osc_type) types[i], ptr);

		m_Argv[i] = len ? (osc_arg *) ptr : 0;

		remain -= len;
		ptr += len;
	}

	if (0 != remain || i != argc) {
		m_Result = OSC_INVALID_SIZE;
		goto fail;
	}

	m_Result = OSC_OK;
	return;

	fail: if (m_Types) {
		free(m_Types);
		m_Types = 0;
	}

	if (m_Data) {
		free(m_Data);
		m_Data = 0;
	}

	if (m_Argv) {
		free(m_Argv);
		m_Argv = 0;
	}
}

OSCMessage::~OSCMessage(void) {
	if (m_Types) {
		free(m_Types);
		m_Types = 0;
	}

	if (m_Data) {
		free(m_Data);
		m_Data = 0;
	}

	if (m_Argv) {
		free(m_Argv);
		m_Argv = 0;
	}
}

void OSCMessage::Dump(void) {
	printf("types %s\n", m_Types);
	printf("typelen %u\n", m_Typelen);
	printf("typesize %u\n", m_Typesize);
	printf("datalen %u\n", m_Datalen);
	printf("datasize %u\n", m_Datasize);

	printf("data : ");
	if (m_Data != 0) {
		char *p = (char *) m_Data;
		for (int i = 0; i < (int) m_Datasize; i++) {
			printf("%x ", (uint8_t) p[i]);
		}
		printf("\n");
	} else {
		printf("m_Data = 0\n");
	}
}

int OSCMessage::AddFloat(float a) {
	osc_pcast32 b;
	int32_t *nptr = (int32_t *) AddData((unsigned) sizeof(a));

	if (!nptr) {
		return -1;
	}

	b.f = a;

	if (AddTypeChar(OSC_FLOAT)) {
		return -1;
	}

	*nptr = b.nl;

	return 0;
}

int OSCMessage::AddInt32(int32_t a) {
	osc_pcast32 b;
	int32_t *nptr = (int32_t *) AddData((unsigned) sizeof(a));

	if (!nptr) {
		return -1;
	}

	b.i = a;

	if (AddTypeChar(OSC_INT32)) {
		return -1;
	}

	*nptr = b.nl;

	return 0;
}

int OSCMessage::AddString(const char *a) {
	const unsigned size = OSCString::Size(a);
	char *nptr = (char *) AddData(size);

	if (!nptr) {
		return -1;
	}

	if (AddTypeChar(OSC_STRING)) {
		return -1;
	}

	strncpy(nptr, a, size);

	return 0;
}

int OSCMessage::AddBlob(OSCBlob *pBlob) {
	const unsigned size = pBlob->GetSize();
	const unsigned dsize = pBlob->GetDataSize();

	char *nptr = (char *) AddData(size);

	if (!nptr) {
		return -1;
	}

	if (AddTypeChar(OSC_BLOB)) {
		return -1;
	}

	memset(nptr + size - 4, 0, 4);

	memcpy(nptr, &dsize, sizeof(dsize));
	memcpy(nptr + sizeof(uint32_t), pBlob->GetDataPtr(), dsize);

	return 0;
}


int OSCMessage::GetResult(void) const {
	return m_Result;
}

int OSCMessage::GetArgc(void) {
	return m_Typelen - 1;
}

osc_type OSCMessage::GetType(unsigned argc) {
	if (argc > m_Typelen - 1) {
		m_Result = OSC_INVALID_ARGUMENT;
		return OSC_UNKNOWN;
	}

	return (osc_type) m_Types[argc + 1];
}

char *OSCMessage::getTypes(void) const {
	return m_Types;
}

unsigned OSCMessage::getDataLength(void) const {
	return m_Datalen;
}

float OSCMessage::GetFloat(unsigned argc) {
	if (argc > m_Typelen) {
		m_Result = OSC_INVALID_ARGUMENT;
		return 0;
	}

	if (m_Argv == 0) {
		ArgvUpdate();
	}

	osc_pcast32 val32;
	val32.nl = *(int32_t *) m_Argv[argc];

	return val32.f;
}

int OSCMessage::GetInt(unsigned argc) {
	if (argc > m_Typelen) {
		m_Result = OSC_INVALID_ARGUMENT;
		return 0;
	}

	if (m_Argv == 0) {
		ArgvUpdate();
	}

	osc_pcast32 val32;
	val32.nl = *(int32_t *) m_Argv[argc];

	return val32.nl;
}

char * OSCMessage::GetString(unsigned argc) {
	if (argc > m_Typelen) {
		m_Result = OSC_INVALID_ARGUMENT;
		return 0;
	}

	if (m_Argv == 0) {
		ArgvUpdate();
	}

	return (char *)m_Argv[argc];
}

OSCBlob OSCMessage::GetBlob(unsigned argc) {
	void *data;
	unsigned int size = 0;
	const char *p;
	osc_pcast32 val32;

	if (argc > m_Typelen) {
		m_Result = OSC_INVALID_ARGUMENT;
		return OSCBlob(0, 0);
	}

	if (m_Argv == 0) {
		ArgvUpdate();
	}

	data = m_Argv[argc];
	val32.nl = *(int32_t *) data;
	size = val32.i;
	p = (const char *)data + 4;

	return OSCBlob(p, size);
}

void *OSCMessage::Serialise(const char *path, void *to, unsigned * size) {
	int i, argc;
	char *types, *ptr;
	unsigned s = OSCString::Size(path) + OSCString::Size(m_Types) + m_Datalen;

    if (size) {
        *size = s;
    }

    if (!to) {
        to = calloc(1, s);
    }

    memset((char *) to + OSCString::Size(path) - 4, 0, 4);

    strcpy((char *) to, path);

    memset((char *) to + OSCString::Size(path) + OSCString::Size(m_Types) - 4, 0, 4);

    strcpy((char *) to + OSCString::Size(path), m_Types);

    types = m_Types + 1;
    ptr = (char *) to + OSCString::Size(path) + OSCString::Size(m_Types);

    memcpy(ptr, m_Data, m_Datalen);

    argc = m_Typelen - 1;

    for (i = 0; i < argc; ++i) {
        size_t len = ArgSize((osc_type)types[i], (void *)ptr);
        ArgNetworkEndian((osc_type)types[i], ptr);
        ptr += len;
    }

    return to;
}

signed OSCMessage::ArgValidate(osc_type type, void *data, unsigned size) {

	switch (type) {
	case OSC_TRUE:
	case OSC_FALSE:
	case OSC_NIL:
	case OSC_INFINITUM:
		return 0;
	case OSC_INT32:
	case OSC_FLOAT:
	case OSC_MIDI:
	case OSC_CHAR:
		return size >= 4 ? 4 : -OSC_INVALID_SIZE;
	case OSC_INT64:
	case OSC_TIMETAG:
	case OSC_DOUBLE:
		return size >= 8 ? 8 : -OSC_INVALID_SIZE;
	case OSC_STRING:
	case OSC_SYMBOL:
		return OSCString::Validate(data, size);
	case OSC_BLOB:
		return OSCBlob::Validate(data, size);
	default:
		return -OSC_INVALID_TYPE;
	}

	return -OSC_INTERNAL_ERROR;
}

void OSCMessage::ArgNetworkEndian(osc_type type, void *data)
{
	switch (type) {
	case OSC_INT32:
	case OSC_FLOAT:
	case OSC_BLOB:
	case OSC_CHAR:
		*(int32_t *) data = __builtin_bswap32(*(int32_t *) data);
		break;
	case OSC_TIMETAG:
		*(uint32_t *) data = __builtin_bswap32(*(uint32_t *) data);
		data = ((uint32_t *) data) + 1;
		*(uint32_t *) data = __builtin_bswap32(*(uint32_t *) data);
		break;
	case OSC_INT64:
	case OSC_DOUBLE:
		*(int64_t *) data = __builtin_bswap64(*(int64_t *) data);
		break;
	case OSC_STRING:
	case OSC_SYMBOL:
	case OSC_MIDI:
	case OSC_TRUE:
	case OSC_FALSE:
	case OSC_NIL:
	case OSC_INFINITUM:
		/* these are fine */
		break;

	default:
		break;
	}
}

void OSCMessage::ArgHostEndian(osc_type type, void *data)
{
    switch (type) {
    case OSC_INT32:
    case OSC_FLOAT:
    case OSC_BLOB:
    case OSC_CHAR:
        *(int32_t *) data = __builtin_bswap32(*(int32_t *) data);
        break;
    case OSC_TIMETAG:
        *(int32_t *) data = __builtin_bswap32(*(int32_t *) data);
        data = ((int32_t *) data) + 1;
        *(int32_t *) data = __builtin_bswap32(*(int32_t *) data);
        break;
    case OSC_INT64:
    case OSC_DOUBLE:
        *(int64_t *) data = __builtin_bswap64(*(int64_t *) data);
        break;
    case OSC_STRING:
    case OSC_SYMBOL:
    case OSC_MIDI:
    case OSC_TRUE:
    case OSC_FALSE:
    case OSC_NIL:
    case OSC_INFINITUM:
        /* these are fine */
        break;
    default:
        /* Unknown type */
        break;
    }
}

void *OSCMessage::AddData(unsigned s) {
    uint32_t old_dlen = m_Datalen;

    int new_datasize = m_Datasize;
    int new_datalen = m_Datalen + s;

    void *new_data = 0;

    if (!new_datasize) {
        new_datasize = OSC_DEF_DATA_SIZE;
    }

	if (new_datalen > new_datasize) {
		new_datasize  = next_pow2(new_datalen);
	}

    new_data = realloc(m_Data, new_datasize);

    if (!new_data) {
        return (void *)0;
    }

    m_Datalen = new_datalen;
    m_Datasize = new_datasize;
    m_Data = new_data;

    if (m_Argv) {
        free(m_Argv);
        m_Argv = 0;
    }

    return (void *) ((char *) m_Data + old_dlen);
}

int OSCMessage::AddTypeChar(char t) {
	if (m_Typelen + 1 >= m_Typesize) {
		int new_typesize = m_Typesize * 2;
		char *new_types = 0;

		if (!new_typesize) {
			new_typesize = OSC_DEF_TYPE_SIZE;
		}

		new_types = (char *)realloc(m_Types, new_typesize);

		if (!new_types)
			return -1;

		m_Types = new_types;
		m_Typesize = new_typesize;
	}

	m_Types[m_Typelen] = t;
	m_Typelen++;
	m_Types[m_Typelen] = '\0';

	if (m_Argv) {
		free(m_Argv);
		m_Argv = 0;
	}

	return 0;
}

unsigned OSCMessage::ArgSize(osc_type type, void *data)
{
    switch (type) {
    case OSC_TRUE:
    case OSC_FALSE:
    case OSC_NIL:
    case OSC_INFINITUM:
        return 0;

    case OSC_INT32:
    case OSC_FLOAT:
    case OSC_MIDI:
    case OSC_CHAR:
        return 4;

    case OSC_INT64:
    case OSC_TIMETAG:
    case OSC_DOUBLE:
        return 8;

    case OSC_STRING:
    case OSC_SYMBOL:
        return OSCString::Size((char *) data);

    case OSC_BLOB:
        return OSCBlob::Size(data);

    default:
    	// Unknown
        return 0;
    }

    return 0;
}

void OSCMessage::ArgvUpdate(void) {
    int i, argc;
    char *types, *ptr;
    osc_arg **argv;

    if (0 != m_Argv) {
        return;
    }

    argc = m_Typelen - 1;
    types = m_Types + 1;
    ptr = (char *)m_Data;

    argv = (osc_arg **)calloc(argc, sizeof(osc_arg *));

    for (i = 0; i < argc; ++i) {
        unsigned len = ArgSize((osc_type)types[i], ptr);
        argv[i] = len ? (osc_arg *) ptr : 0;
        ptr += len;
    }

    m_Argv = argv;
}
