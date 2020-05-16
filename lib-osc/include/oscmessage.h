/**
 * @file oscmessage.h
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

#ifndef OSCMESSAGE_H_
#define OSCMESSAGE_H_

#include "osc.h"
#include "oscblob.h"

enum OscMessageDeserialise {
	OK,
	INVALID_INVALID_SIZE,
	MALLOC_ERROR,
	INVALID_PATH,
	NO_TYPE_TAG,
	INVALID_TYPE,
	INVALID_TYPE_TAG,
	INVALID_ARGUMENT,
	INVALID_SIZE,
	NONE_ZERO_IN_PADDING,
	MESSAGE_NULL,
	INTERNAL_ERROR
};

class OSCMessage {
public:
	OSCMessage(void);
	OSCMessage(void *, unsigned);
	~OSCMessage(void);

	OscMessageDeserialise GetResult(void) {
		return m_Result;
	}

	int GetArgc(void) {
		return static_cast<int>(m_nTypesLength) - 1;
	}

	char *GetTypes(void) {
		return m_pTypes;
	}

	unsigned GetDataLength(void) {
		return m_nDatalen;
	}

	char GetType(unsigned);

	float GetFloat(unsigned);
	int GetInt(unsigned);
	char *GetString(unsigned);
	OSCBlob GetBlob(unsigned);

	int AddFloat(float);
	int AddInt32(int32_t);
	int AddString(const char *);
	int AddBlob(OSCBlob *);

	void Dump(void);

	void *Serialise(const char *, void *, unsigned *);

private:
#define OSC_DEF_TYPE_SIZE 8
#define OSC_DEF_DATA_SIZE 8

	unsigned ArgSize(char nType, void *);
	int ArgValidate(char nType, void *, int);

	void ArgvUpdate(void);

	void ArgHostEndian(char nType, void *);
	void ArgNetworkEndian(char nType, void *);

	void *AddData(unsigned);
	int AddTypeChar(char);

private:
    char *m_pTypes;
    unsigned m_nTypesLength;
    unsigned m_nTypesRealSize;
    void *m_pData;
    unsigned m_nDatalen;
    unsigned m_nDatasize;
    osc_arg **m_pArgv;
    OscMessageDeserialise m_Result;
};

#endif /* OSCMESSAGE_H_ */

