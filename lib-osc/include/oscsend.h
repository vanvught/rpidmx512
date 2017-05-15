/**
 * @file oscsend.h
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

#ifndef OSCSEND_CPP_
#define OSCSEND_CPP_

#ifdef __circle__
#include <stdint.h>
#include <circle/util.h>
#include <circle/stdarg.h>
#include <circle/net/socket.h>
#include <circle/net/ipaddress.h>
#else
#include <stdarg.h>
#endif

#include "oscmessage.h"

class OSCSend {

public:
#if defined (__circle__)
	OSCSend(CSocket *, CIPAddress *, int, const char *, const char *, ...);
#else
	OSCSend(const int, const int, const char *, const char *, ...);
#endif
	~OSCSend(void);

private:
	void AddVarArgs(va_list);
	void Send(void);

private:
#if defined (__circle__)
	CSocket *m_pSocket;
	CIPAddress *m_pAddress;
#else
	const int m_Address;
#endif
	const int m_Port;
	const char *m_Path;
	const char *m_Types;
	OSCMessage *m_Msg;

	int m_Result;
};

#endif /* OSCSEND_CPP_ */
