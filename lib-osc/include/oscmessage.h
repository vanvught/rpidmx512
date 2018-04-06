/**
 * @file oscmessage.h
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

typedef enum osc_message_deserialise {
	OSC_OK = 0,
	OSC_INVALID__INVALID_SIZE,
	OSC_MALLOC_ERROR,
	OSC_INVALID_PATH,
	OSC_NO_TYPE_TAG,
	OSC_INVALID_TYPE,
	OSC_INVALID_TYPE_TAG,
	OSC_INVALID_ARGUMENT,
	OSC_INVALID_SIZE,
	OSC_NONE_ZERO_IN_PADDING,
	OSC_MESSAGE_NULL,
	OSC_INTERNAL_ERROR
} _osc_message_deserialise;

class OSCMessage {

public:
	OSCMessage(void);
	OSCMessage(void *, unsigned);
	~OSCMessage(void);

	int GetResult(void) const;
	char *getTypes(void) const;
	unsigned getDataLength(void) const;
	int GetArgc(void);

	osc_type GetType(unsigned);

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
	unsigned ArgSize(osc_type, void *);
	signed ArgValidate(osc_type, void *, unsigned);

	void ArgvUpdate(void);

	void ArgHostEndian(osc_type, void *);
	void ArgNetworkEndian(osc_type, void *);

	void *AddData(unsigned);
	int AddTypeChar(char);

private:
    char *m_Types;
    unsigned m_Typelen;
    unsigned m_Typesize;
    void *m_Data;
    unsigned m_Datalen;
    unsigned m_Datasize;
    osc_arg **m_Argv;
    /* timestamp from bundle (OSC_TT_IMMEDIATE for unbundled messages) */
    //osc_timetag m_Ts;
	int m_Result;
};

#endif /* OSCMESSAGE_H_ */

