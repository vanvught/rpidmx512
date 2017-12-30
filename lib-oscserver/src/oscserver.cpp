/**
 * @file oscserver.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "oscserver.h"

#include "lightset.h"

#include "oscmessage.h"
#include "oscsend.h"
#include "oscblob.h"

#include "network.h"

#define OSCSERVER_MAX_BUFFER 1024

OscServer::OscServer(void):
	m_nPortIncoming(OSCSERVER_DEFAULT_PORT_INCOMING),
	m_nPortOutgoing(OSCSERVER_DEFAULT_PORT_OUTGOING),
	m_pLightSet(0),
	m_IsBlackout(false)
{
	m_pBuffer = new uint8_t[OSCSERVER_MAX_BUFFER];
	assert(m_pBuffer != 0);

	m_pData  = new uint8_t[512];
	assert(m_pData != 0);

	for (unsigned i = 0; i < 512; i++) {
		m_pData[i] = 0;
	}

	m_pOsc  = new uint8_t[512];
	assert(m_pOsc != 0);
}

OscServer::~OscServer(void) {
	delete[] m_pBuffer;
	m_pBuffer = 0;

	if (m_pLightSet != 0) {
		m_pLightSet->Stop();
		m_pLightSet = 0;
	}
}

void OscServer::Start(void) {
	network_begin(m_nPortIncoming);
}

void OscServer::Stop(void) {
	if (m_pLightSet != 0) {
		m_pLightSet->Stop();
	}
}

uint16_t OscServer::GetPortIncoming(void) const {
	return m_nPortIncoming;
}

void OscServer::SetPortIncoming(uint16_t nPortIncoming) {
	assert(nPortIncoming > 1023);

	m_nPortIncoming = nPortIncoming;
}

uint16_t OscServer::GetPortOutgoing(void) const {
	return m_nPortOutgoing;
}

void OscServer::SetPortOutgoing(uint16_t nPortOutgoing) {
	assert(nPortOutgoing > 1023);

	m_nPortOutgoing = nPortOutgoing;
}

void OscServer::SetOutput(LightSet *pLightSet) {
	assert(pLightSet != 0);

	m_pLightSet = pLightSet;
}

int OscServer::GetChannel(const char* p) {
	assert(p != 0);

	char *s = (char *)p + 6;
	int nChannel = 0;
	int i;

	for (i = 0; (i < 3) && (*s != '\0'); i++) {
		int c = *s;

		if ((c < (int) '0') || (c > (int) '9')) {
			return 0;
		}

		nChannel = nChannel * 10 + c - (int) '0';
		s++;
	}

	if (nChannel > 512) {
		return 0;
	}

	return nChannel;
}

const bool OscServer::IsDmxDataChanged(const uint8_t* pData, uint16_t nStart, uint16_t nLength) {
	assert(nLength <= 512);

	bool isChanged = false;

	uint8_t *src = (uint8_t *)pData;
	uint8_t *dst = (uint8_t *)&m_pData[--nStart];

	uint16_t nEnd = nStart + nLength;

	assert(nEnd <= 512);

	for (unsigned i = nStart; i < nEnd; i++) {
		if (*dst != *src) {
			*dst = *src;
			isChanged = true;
		}
		dst++;
		src++;
	}

	return isChanged;
}

int OscServer::Run(void) {
	uint32_t nRemoteIp;
	uint16_t nRemotePort;

	const int nBytesReceived = network_recvfrom(m_pBuffer, OSCSERVER_MAX_BUFFER, &nRemoteIp, &nRemotePort);

	if (nBytesReceived == 0) {
		return 0;
	}

	if (OSC::isMatch((const char*) m_pBuffer, "/ping")) {
		OSCSend MsgSend(nRemoteIp, m_nPortOutgoing, "/pong", 0);
	} else {
		OSCMessage Msg((char *) m_pBuffer, nBytesReceived);
#ifndef NDEBUG
		printf("path : %s\n", OSC::GetPath((char*) m_pBuffer, nBytesReceived));
#endif
		if (OSC::isMatch((const char*) m_pBuffer, "/dmx1/blackout")) {
			m_IsBlackout = (unsigned)Msg.GetFloat(0) == 1;

			if (m_IsBlackout) {
#ifndef NDEBUG
				puts("Handle blackout");
#endif
			} else {
#ifndef NDEBUG
				puts("Handle undo-blackout");
#endif
				m_pLightSet->SetData(0, m_pData, 512);
			}
		} else if (OSC::isMatch((const char*) m_pBuffer, "/dmx1/*")) {
			const int nArgc = Msg.GetArgc();
			const uint16_t nChannel = GetChannel((const char*) m_pBuffer);
			uint8_t nData;
#ifndef NDEBUG
			printf("(First) Channel:%d ", nChannel);
#endif
			if (nChannel != 0) {
				if (nArgc == 1) { // Backwards compatibility
					if (Msg.GetType(0) == OSC_FLOAT) {
						nData = (uint8_t) Msg.GetFloat(0);
#ifndef NDEBUG
						printf("%d ", (int) nData);
						puts("Backwards compatibility");
#endif
					} else if (Msg.GetType(0) == OSC_INT32) {
						nData = (uint8_t) Msg.GetInt(0);
#ifndef NDEBUG
						printf("%d\n", (int) nData);
#endif
					}
					if (IsDmxDataChanged(&nData, nChannel, 1)) {
						m_pLightSet->SetData(0, m_pData, 512);
					}
				} else {
					if ((nChannel + nArgc) <= 513) {
						for (int i = 0; i < nArgc; i++) {
							const uint8_t nData = (uint8_t) Msg.GetInt(i);
							m_pOsc[i] = nData;
#ifndef NDEBUG
							printf("%d ", (int) nData);
#endif
						}
#ifndef NDEBUG
						puts("");
#endif
						if (IsDmxDataChanged(m_pOsc	, nChannel, nArgc)) {
							m_pLightSet->SetData(0, m_pData, 512);
						}
					} else { // Too many channels
#ifndef NDEBUG
						printf(" -> Too many data items\n");
#endif
						return -1;
					}
				}
			}

		} else if (OSC::isMatch((const char*) m_pBuffer, "/dmx1")) {
			const int nArgc = Msg.GetArgc();

			if (nArgc == 1) { // It must be a BLOB
				if (Msg.GetType(0) == OSC_BLOB) {
#ifndef NDEBUG
					puts("BLOB received");
#endif
					OSCBlob blob = Msg.GetBlob(0);
					const int size = (int) blob.GetSize();
					if (size <= 512) {

					} else { // Too many channels
#ifndef NDEBUG
						printf(" -> Too many data items\n");
#endif
						return -1;
					}
				}
			} else {
				const uint16_t nFirstChannel = (uint16_t) Msg.GetInt(0);
#ifndef NDEBUG
				printf("First channel:%d, ", (int) nFirstChannel);
#endif
				for (int i = 1; i < nArgc; i++) {
					const uint8_t nData = (uint8_t) Msg.GetInt(i);
					m_pOsc[i-1] = nData;
#ifndef NDEBUG
					printf("%d ", (int) nData);
#endif
				}
#ifndef NDEBUG
				puts("");
#endif
				if (IsDmxDataChanged(m_pOsc	, nFirstChannel, nArgc - 1)) {
					m_pLightSet->SetData(0, m_pData, 512);
				}
			}
		}
	}

	return nBytesReceived;
}
