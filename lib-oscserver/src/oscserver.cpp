/**
 * @file oscserver.cpp
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <string.h>
#include <assert.h>

#include "oscserver.h"
#include "osc.h"
#include "oscmessage.h"
#include "oscsend.h"
#include "oscblob.h"

#include "lightset.h"
#include "network.h"

#include "hardware.h"

#include "debug.h"

#define OSCSERVER_MAX_BUFFER 				4096

#define OSCSERVER_DEFAULT_PATH_PRIMARY		"/dmx1"
#define OSCSERVER_DEFAULT_PATH_SECONDARY	OSCSERVER_DEFAULT_PATH_PRIMARY"/*"
#define OSCSERVER_DEFAULT_PATH_INFO			"/2"
#define OSCSERVER_DEFAULT_PATH_BLACKOUT		OSCSERVER_DEFAULT_PATH_PRIMARY "/blackout"

#define SOFTWARE_VERSION "1.0"

enum {
	DMX_UNIVERSE = 512,
	DMX_MAX_VALUE = 255
};

OscServer::OscServer(void):
	m_nPortIncoming(OSCSERVER_DEFAULT_PORT_INCOMING),
	m_nPortOutgoing(OSCSERVER_DEFAULT_PORT_OUTGOING),
	m_nHandle(-1),
	m_bPartialTransmission(false),
	m_nLastChannel(0),
	m_pOscServerHandler(0),
	m_pLightSet(0)
{
	memset(m_aPath, 0, sizeof(m_aPath));
	strcpy(m_aPath, OSCSERVER_DEFAULT_PATH_PRIMARY);

	memset(m_aPathSecond, 0, sizeof(m_aPathSecond));
	strcpy(m_aPathSecond, OSCSERVER_DEFAULT_PATH_SECONDARY);

	memset(m_aPathInfo, 0, sizeof(m_aPathInfo));
	strcpy(m_aPathInfo, OSCSERVER_DEFAULT_PATH_INFO);

	memset(m_aPathBlackOut, 0, sizeof(m_aPathBlackOut));
	strcpy(m_aPathBlackOut, OSCSERVER_DEFAULT_PATH_BLACKOUT);

	m_pBuffer = new uint8_t[OSCSERVER_MAX_BUFFER];
	assert(m_pBuffer != 0);

	m_pData  = new uint8_t[DMX_UNIVERSE];
	assert(m_pData != 0);

	for (unsigned i = 0; i < DMX_UNIVERSE; i++) {
		m_pData[i] = 0;
	}

	m_pOsc  = new uint8_t[DMX_UNIVERSE];
	assert(m_pOsc != 0);

	snprintf(m_Os, sizeof(m_Os), "[V%s] %s", SOFTWARE_VERSION, __DATE__);

	uint8_t nHwTextLength;
	m_pModel = Hardware::Get()->GetBoardName(nHwTextLength);
	m_pSoC = Hardware::Get()->GetSocName(nHwTextLength);
	if (m_pSoC[0] == '\0') {
		m_pSoC = Hardware::Get()->GetCpuName(nHwTextLength);
	}
}

OscServer::~OscServer(void) {
	if (m_pLightSet != 0) {
		m_pLightSet->Stop(0);
		m_pLightSet = 0;
	}

	delete[] m_pBuffer;
	m_pBuffer = 0;

	delete[] m_pData;
	m_pData = 0;

	delete[] m_pOsc;
	m_pOsc = 0;
}

void OscServer::Start(void) {
	m_nHandle = Network::Get()->Begin(m_nPortIncoming);
	assert(m_nHandle != -1);

	OSCSend MsgSend(m_nHandle, Network::Get()->GetIp() | ~(Network::Get()->GetNetmask()), m_nPortIncoming, "/ping", 0);

	if (m_pLightSet != 0) {
		m_pLightSet->Start(0);
	}
}

void OscServer::Stop(void) {
	if (m_pLightSet != 0) {
		m_pLightSet->Stop(0);
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

void  OscServer::SetOscServerHandler(OscServerHandler *pOscServerHandler) {
	assert(pOscServerHandler != 0);
	m_pOscServerHandler = pOscServerHandler;
}

void OscServer::SetPath(const char* pPath) {
	if (*pPath == '/') {
		unsigned length = sizeof(m_aPath) - 3; // We need space for '\0' and "/*"
		strncpy(m_aPath, pPath, length);
		length = strlen(m_aPath);

		if (m_aPath[length - 1] == '/') {
			m_aPath[length - 1] = '\0';
		}

		strcpy(m_aPathSecond, m_aPath);
		length = strlen(m_aPathSecond);
		m_aPathSecond[length++] = '/';
		m_aPathSecond[length++] = '*';
		m_aPathSecond[length] = '\0';
	}

	DEBUG_PUTS(m_aPath);
	DEBUG_PUTS(m_aPathSecond);
}

const char* OscServer::GetPath(void) {
	return m_aPath;
}

void OscServer::SetPathInfo(const char* pPathInfo) {
	if (*pPathInfo == '/') {
		unsigned length = sizeof(m_aPathInfo) - 1; // We need space for '\0'
		strncpy(m_aPathInfo, pPathInfo, length);
		length = strlen(m_aPathInfo);

		if (m_aPathInfo[length - 1] == '/') {
			m_aPathInfo[length - 1] = '\0';
		}
	}

	DEBUG_PUTS(m_aPathInfo);
}

const char* OscServer::GetPathInfo(void) {
	return m_aPathInfo;
}

void OscServer::SetPathBlackOut(const char* pPathBlackOut) {
	if (*pPathBlackOut == '/') {
		unsigned length = sizeof(m_aPathInfo) - 1; // We need space for '\0'
		strncpy(m_aPathBlackOut, pPathBlackOut, length);
		length = strlen(m_aPathBlackOut);

		if (m_aPathBlackOut[length - 1] == '/') {
			m_aPathBlackOut[length - 1] = '\0';
		}
	}

	DEBUG_PUTS(m_aPathBlackOut);
}

const char* OscServer::GetPathBlackOut(void) {
	return m_aPathBlackOut;
}

bool OscServer::IsPartialTransmission(void) const {
	return m_bPartialTransmission;
}

void OscServer::SetPartialTransmission(bool bPartialTransmission) {
	m_bPartialTransmission = bPartialTransmission;
}

int OscServer::GetChannel(const char* p) {
	assert(p != 0);

	char *s = (char *)p + strlen(m_aPath) + 1;
	int nChannel = 0;
	int i;

	for (i = 0; (i < 3) && (*s != '\0'); i++) {
		int c = *s;

		if ((c < (int) '0') || (c > (int) '9')) {
			return -1;
		}

		nChannel = nChannel * 10 + c - (int) '0';
		s++;
	}

	if (nChannel > DMX_UNIVERSE) {
		return -1;
	}

	return nChannel;
}

bool OscServer::IsDmxDataChanged(const uint8_t* pData, uint16_t nStartChannel, uint16_t nLength) {
	assert(pData != 0);
	assert(nLength <= DMX_UNIVERSE);

	bool isChanged = false;

	const uint8_t *src = pData;
	uint8_t *dst = (uint8_t *) &m_pData[--nStartChannel];

	uint16_t nEnd = nStartChannel + nLength;

	assert(nEnd <= DMX_UNIVERSE);

	for (uint32_t i = nStartChannel; i < nEnd; i++) {
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

	const int nBytesReceived = Network::Get()->RecvFrom(m_nHandle, m_pBuffer, OSCSERVER_MAX_BUFFER, &nRemoteIp, &nRemotePort);

	if (nBytesReceived == 0) {
		return 0;
	}

	if (OSC::isMatch((const char*) m_pBuffer, "/ping")) {
		DEBUG_PUTS("ping received");
		OSCSend MsgSend(m_nHandle, nRemoteIp, m_nPortOutgoing, "/pong", 0);
	} else if (OSC::isMatch((const char*) m_pBuffer, m_aPathInfo)) {
		OSCSend MsgSendInfo(m_nHandle, nRemoteIp, m_nPortOutgoing, "/info/os", "s", m_Os);
		OSCSend MsgSendModel(m_nHandle, nRemoteIp, m_nPortOutgoing, "/info/model", "s", m_pModel);
		OSCSend MsgSendSoc(m_nHandle, nRemoteIp, m_nPortOutgoing, "/info/soc", "s", m_pSoC);

		if (m_pOscServerHandler != 0) {
			m_pOscServerHandler->Info(m_nHandle, nRemoteIp, m_nPortOutgoing);
		}
	} else if (OSC::isMatch((const char*) m_pBuffer, m_aPathBlackOut)) {
		OSCMessage Msg(m_pBuffer, nBytesReceived);
		const bool bBlackout = (unsigned) Msg.GetFloat(0) == 1;

		if (bBlackout) {
			if (m_pOscServerHandler != 0) {
				m_pOscServerHandler->Blackout();
			}
			DEBUG_PUTS("Blackout");
		} else {
			if (m_pOscServerHandler != 0) {
				m_pOscServerHandler->Update();
			}
			DEBUG_PUTS("Update");
		}
	} else {
		OSCMessage Msg((char *) m_pBuffer, nBytesReceived);

		debug_dump(m_pBuffer, nBytesReceived);

		DEBUG_PRINTF("[%d] path : %s", nBytesReceived, OSC::GetPath((char*) m_pBuffer, nBytesReceived));

		if (OSC::isMatch((const char*) m_pBuffer, m_aPath)) {
			const int nArgc = Msg.GetArgc();

			if ((nArgc == 1) && (Msg.GetType(0) == OSC_BLOB)) {
				DEBUG_PUTS("Blob received");

				OSCBlob blob = Msg.GetBlob(0);
				const int size = (int) blob.GetDataSize();

				if (size <= DMX_UNIVERSE) {
					const uint8_t *ptr = (const uint8_t *) blob.GetDataPtr();
					if (IsDmxDataChanged(ptr, 1, size)) {
						if ((!m_bPartialTransmission) || (size == DMX_UNIVERSE)) {
							m_pLightSet->SetData(0, m_pData, DMX_UNIVERSE);
						} else {
							m_nLastChannel = size > m_nLastChannel ? size : m_nLastChannel;
							m_pLightSet->SetData(0, m_pData, m_nLastChannel);
						}
					}
				} else {
					DEBUG_PUTS("Too many channels");
					return -1;
				}
			} else if ((nArgc == 2) && (Msg.GetType(0) == OSC_INT32)) {
				uint16_t nChannel = (uint16_t) (1 + Msg.GetInt(0));

				if ((nChannel < 1) || (nChannel > DMX_UNIVERSE)) {
					DEBUG_PRINTF("Invalid channel [%d]", nChannel);
					return -1;
				}

				uint8_t nData;

				if (Msg.GetType(1) == OSC_INT32) {
					DEBUG_PUTS("ii received");
					nData = (uint8_t) Msg.GetInt(1);
				} else if (Msg.GetType(1) == OSC_FLOAT) {
					DEBUG_PUTS("if received");
					nData = (uint8_t) (Msg.GetFloat(1) * DMX_MAX_VALUE);
				} else {
					return -1;
				}

				DEBUG_PRINTF("Channel = %d, Data = %.2x", nChannel, nData);

				if (IsDmxDataChanged(&nData, nChannel, 1)) {
					if (!m_bPartialTransmission) {
						m_pLightSet->SetData(0, m_pData, DMX_UNIVERSE);
					} else {
						m_nLastChannel = nChannel > m_nLastChannel ? nChannel : m_nLastChannel;
						m_pLightSet->SetData(0, m_pData, m_nLastChannel);
					}
				}
			}
		} else if (OSC::isMatch((const char*) m_pBuffer, m_aPathSecond)) {
			const int nArgc = Msg.GetArgc();

			if (nArgc == 1) { // /path/N 'i' or 'f'
				const uint16_t nChannel = GetChannel((const char*) m_pBuffer);

				if (nChannel >= 1 && nChannel <= DMX_UNIVERSE) {
					uint8_t nData;

					if (Msg.GetType(0) == OSC_INT32) {
						DEBUG_PUTS("i received");
						nData = (uint8_t) Msg.GetInt(0);
					} else if (Msg.GetType(0) == OSC_FLOAT) {
						DEBUG_PRINTF("f received %f", Msg.GetFloat(0));
						nData = (uint8_t) (Msg.GetFloat(0) * DMX_MAX_VALUE);
					} else {
						return -1;
					}

					DEBUG_PRINTF("Channel = %d, Data = %.2x", nChannel, nData);

					if (IsDmxDataChanged(&nData, nChannel, 1)) {
						if (!m_bPartialTransmission) {
							m_pLightSet->SetData(0, m_pData, DMX_UNIVERSE);
						} else {
							m_nLastChannel = nChannel > m_nLastChannel ? nChannel : m_nLastChannel;
							m_pLightSet->SetData(0, m_pData, m_nLastChannel);
						}
					}
				} else {
					return -1;
				}
			} else {
				return -1;
			}
		}
	}

	return nBytesReceived;
}
