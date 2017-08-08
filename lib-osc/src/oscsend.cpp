/**
 * @file oscsend.cpp
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

#include <stdint.h>
#include <assert.h>

#ifdef __circle__
#include <circle/util.h>
#include <circle/stdarg.h>
#include "oscutil.h"
static const char FromOscSend[] = "oscsend";
#elif defined (__linux__) || defined (__CYGWIN__)
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#else
#include <stdarg.h>
#include <stdlib.h>
#endif

#include "oscsend.h"
#include "oscstring.h"
#include "oscmessage.h"

#include "network.h"

/**
 * @brief Send a OSC formatted message to the address specified.
 *
 * @param address The target OSC address
 * @param port The target OSC port
 * @param path The OSC path the message will be delivered to
 * @param types The types of the data items in the message, types are defined in \ref osc_type
 * @param ... The data values to be transmitted. The types of the arguments passed here must agree with the types specified in the type parameter.
 */
OSCSend::OSCSend(const int address, const int port, const char *path, const char *types, ...) :
		m_Address(address), m_Port(port), m_Path(path), m_Types(types), m_Msg(0), m_Result(0) {

	va_list ap;
	va_start(ap, types);
	OSCSend::AddVarArgs(ap);

	if (m_Result == 0) {
		OSCSend::Send();
	}
}

OSCSend::~OSCSend(void) {
	if(m_Msg) {
		delete m_Msg;
	}
}

/**
 *
 * @param ap
 */
void OSCSend::AddVarArgs(va_list ap) {
	m_Msg = new OSCMessage();

	while (m_Types && *m_Types) {
		switch (*m_Types++) {
		case OSC_INT32: {
			int32_t i = va_arg(ap, int32_t);
			m_Msg->AddInt32(i);
			m_Result = m_Msg->GetResult();
			break;
		}
		case OSC_FLOAT: {
			float f = (float) va_arg(ap, double);
			m_Msg->AddFloat(f);
			m_Result = m_Msg->GetResult();
			break;
		}
		case OSC_STRING: {
			char *s = va_arg(ap, char *);
			m_Msg->AddString(s);
			m_Result = m_Msg->GetResult();
			break;
		}
		default: {
			m_Result = -1;
			break;
		}
		}
	}

	va_end(ap);
}

/**
 *
 */
void OSCSend::Send(void) {
	int data_len = OSCString::Size(m_Path) + OSCString::Size(m_Msg->getTypes()) + m_Msg->getDataLength();
	char *data = (char *)m_Msg->Serialise(m_Path, 0, 0);

	network_sendto((const uint8_t *)data, (const uint16_t) data_len, m_Address, (uint16_t)m_Port);

	// Free the memory allocated by m_Msg->Serialise
	if(data) {
		free(data);
	}
}
