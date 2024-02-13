/**
 * @file oscserver.cpp
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "oscserver.h"
#include "osc.h"
#include "oscsimplemessage.h"
#include "oscsimplesend.h"
#include "oscblob.h"

#include "lightset.h"
#include "network.h"

#include "hardware.h"

#include "debug.h"

#define OSCSERVER_DEFAULT_PATH_PRIMARY		"/dmx1"
#define OSCSERVER_DEFAULT_PATH_SECONDARY	OSCSERVER_DEFAULT_PATH_PRIMARY"/*"
#define OSCSERVER_DEFAULT_PATH_INFO			"/2"
#define OSCSERVER_DEFAULT_PATH_BLACKOUT		OSCSERVER_DEFAULT_PATH_PRIMARY "/blackout"

#define SOFTWARE_VERSION "1.0"

char OscServer::s_aPath[osc::server::Max::PATH_LENGTH];
char OscServer::s_aPathSecond[osc::server::Max::PATH_LENGTH];
char OscServer::s_aPathInfo[osc::server::Max::PATH_LENGTH];
char OscServer::s_aPathBlackOut[osc::server::Max::PATH_LENGTH];

char *OscServer::s_pUdpBuffer;
uint8_t OscServer::s_pData[lightset::dmx::UNIVERSE_SIZE];
uint8_t OscServer::s_pOsc[lightset::dmx::UNIVERSE_SIZE];

OscServer *OscServer::s_pThis;

OscServer::OscServer() {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	memset(s_aPath, 0, sizeof(s_aPath));
	strcpy(s_aPath, OSCSERVER_DEFAULT_PATH_PRIMARY);

	memset(s_aPathSecond, 0, sizeof(s_aPathSecond));
	strcpy(s_aPathSecond, OSCSERVER_DEFAULT_PATH_SECONDARY);

	memset(s_aPathInfo, 0, sizeof(s_aPathInfo));
	strcpy(s_aPathInfo, OSCSERVER_DEFAULT_PATH_INFO);

	memset(s_aPathBlackOut, 0, sizeof(s_aPathBlackOut));
	strcpy(s_aPathBlackOut, OSCSERVER_DEFAULT_PATH_BLACKOUT);

	snprintf(m_Os, sizeof(m_Os) - 1, "[V%s] %s", SOFTWARE_VERSION, __DATE__);

	uint8_t nHwTextLength;
	m_pModel = Hardware::Get()->GetBoardName(nHwTextLength);
	m_pSoC = Hardware::Get()->GetSocName(nHwTextLength);

	if (m_pSoC[0] == '\0') {
		m_pSoC = Hardware::Get()->GetCpuName(nHwTextLength);
	}

	DEBUG_EXIT
}

OscServer::~OscServer() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void OscServer::Start() {
	DEBUG_ENTRY

	m_nHandle = Network::Get()->Begin(m_nPortIncoming);
	assert(m_nHandle != -1);

	OscSimpleSend MsgSend(m_nHandle, Network::Get()->GetIp() | ~(Network::Get()->GetNetmask()), m_nPortIncoming, "/ping", nullptr);

	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);

	DEBUG_EXIT
}

void OscServer::Stop() {
	if (m_pLightSet != nullptr) {
		m_pLightSet->Stop(0);
	}
}

void OscServer::SetPath(const char* pPath) {
	if (*pPath == '/') {
		auto nLength = sizeof(s_aPath) - 3; // We need space for '\0' and "/*"
		strncpy(s_aPath, pPath, nLength);
		s_aPath[sizeof(s_aPath) - 1] = '\0';

		nLength = strlen(s_aPath);

		if (s_aPath[nLength - 1] == '/') {
			s_aPath[nLength - 1] = '\0';
		}

		strncpy(s_aPathSecond, s_aPath, sizeof(s_aPathSecond) - 3);

		nLength = strlen(s_aPathSecond);
		assert(nLength < (sizeof(s_aPathSecond) - 3));

		s_aPathSecond[nLength++] = '/';
		s_aPathSecond[nLength++] = '*';
		s_aPathSecond[nLength] = '\0';
	}

	DEBUG_PUTS(s_aPath);
	DEBUG_PUTS(s_aPathSecond);
}

void OscServer::SetPathInfo(const char* pPathInfo) {
	if (*pPathInfo == '/') {
		strncpy(s_aPathInfo, pPathInfo, sizeof(s_aPathInfo));
		s_aPathInfo[sizeof(s_aPathInfo) - 1] = '\0';

		auto nLength = strlen(s_aPathInfo);

		if (s_aPathInfo[nLength - 1] == '/') {
			s_aPathInfo[nLength - 1] = '\0';
		}
	}

	DEBUG_PUTS(s_aPathInfo);
}

void OscServer::SetPathBlackOut(const char* pPathBlackOut) {
	if (*pPathBlackOut == '/') {
		strncpy(s_aPathBlackOut, pPathBlackOut, sizeof(s_aPathInfo));
		s_aPathBlackOut[sizeof(s_aPathInfo) - 1] = '\0';

		auto nLength = strlen(s_aPathBlackOut);

		if (s_aPathBlackOut[nLength - 1] == '/') {
			s_aPathBlackOut[nLength - 1] = '\0';
		}
	}

	DEBUG_PUTS(s_aPathBlackOut);
}

int OscServer::GetChannel(const char* p) {
	assert(p != nullptr);

	auto *s = const_cast<char *>(p) + strlen(s_aPath) + 1;
	int nChannel = 0;
	int i;

	for (i = 0; (i < 3) && (*s != '\0'); i++) {
		int c = *s;

		if ((c < '0') || (c > '9')) {
			return -1;
		}

		nChannel = nChannel * 10 + c - '0';
		s++;
	}

	if (nChannel > static_cast<int32_t>(lightset::dmx::UNIVERSE_SIZE)) {
		return -1;
	}

	return nChannel;
}

bool OscServer::IsDmxDataChanged(const uint8_t* pData, uint16_t nStartChannel, uint32_t nLength) {
	assert(pData != nullptr);
	assert(nLength <= lightset::dmx::UNIVERSE_SIZE);

	auto isChanged = false;
	const auto *src = pData;
	auto *dst = &s_pData[--nStartChannel];
	const auto nEnd = nStartChannel + nLength;

	assert(nEnd <= lightset::dmx::UNIVERSE_SIZE);

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

void OscServer::Run() {
	uint32_t nRemoteIp;
	uint16_t nRemotePort;

	const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&s_pUdpBuffer)), &nRemoteIp, &nRemotePort);

	if (__builtin_expect((nBytesReceived == 0), 1)) {
		return;
	}

	auto bIsDmxDataChanged = false;

	OscSimpleMessage Msg(s_pUdpBuffer, nBytesReceived);

	debug_dump(s_pUdpBuffer, nBytesReceived);

	DEBUG_PRINTF("[%d] path : %s", nBytesReceived, osc::get_path(s_pUdpBuffer, nBytesReceived));

	if (osc::is_match(s_pUdpBuffer, s_aPath)) {
		const auto nArgc = Msg.GetArgc();

		if ((nArgc == 1) && (Msg.GetType(0) == osc::type::BLOB)) {
			DEBUG_PUTS("Blob received");

			OSCBlob blob = Msg.GetBlob(0);
			const auto size = static_cast<uint16_t>(blob.GetDataSize());

			if (size <= lightset::dmx::UNIVERSE_SIZE) {
				const auto *ptr = blob.GetDataPtr();

				bIsDmxDataChanged = IsDmxDataChanged(ptr, 1, size);

				if (bIsDmxDataChanged || m_bEnableNoChangeUpdate) {
					if ((!m_bPartialTransmission) || (size == lightset::dmx::UNIVERSE_SIZE)) {
						m_pLightSet->SetData(0, s_pData, lightset::dmx::UNIVERSE_SIZE);
					} else {
						m_nLastChannel = static_cast<uint16_t>(size > m_nLastChannel ? size : m_nLastChannel);
						m_pLightSet->SetData(0, s_pData, m_nLastChannel);
					}

					if (!m_bIsRunning) {
						m_bIsRunning = true;
						m_pLightSet->Start(0);
					}
				}
			} else {
				DEBUG_PUTS("Too many channels");
				return;
			}
		} else if ((nArgc == 2) && (Msg.GetType(0) == osc::type::INT32)) {
			auto nChannel = static_cast<uint16_t>(1 + Msg.GetInt(0));

			if ((nChannel < 1) || (nChannel > lightset::dmx::UNIVERSE_SIZE)) {
				DEBUG_PRINTF("Invalid channel [%d]", nChannel);
				return;
			}

			uint8_t nData;

			if (Msg.GetType(1) == osc::type::INT32) {
				DEBUG_PUTS("ii received");
				nData = static_cast<uint8_t>(Msg.GetInt(1));
			} else if (Msg.GetType(1) == osc::type::FLOAT) {
				DEBUG_PUTS("if received");
				nData = static_cast<uint8_t>(Msg.GetFloat(1) * lightset::dmx::MAX_VALUE);
			} else {
				return;
			}

			DEBUG_PRINTF("Channel = %d, Data = %.2x", nChannel, nData);

			bIsDmxDataChanged = IsDmxDataChanged(&nData, nChannel, 1);

			if (bIsDmxDataChanged || m_bEnableNoChangeUpdate) {
				if (!m_bPartialTransmission) {
					m_pLightSet->SetData(0, s_pData, lightset::dmx::UNIVERSE_SIZE);
				} else {
					m_nLastChannel = nChannel > m_nLastChannel ? nChannel : m_nLastChannel;
					m_pLightSet->SetData(0, s_pData, m_nLastChannel);
				}

				if (!m_bIsRunning) {
					m_bIsRunning = true;
					m_pLightSet->Start(0);
				}
			}
		}

		return;
	}

	if ((m_pOscServerHandler != nullptr) && (osc::is_match(s_pUdpBuffer, s_aPathBlackOut))) {
		OscSimpleMessage Msg(s_pUdpBuffer, nBytesReceived);

		if (Msg.GetType(0) != osc::type::FLOAT) {
			DEBUG_PUTS("No float");
			return;
		}

		if (Msg.GetFloat(0) != 0) {
			m_pOscServerHandler->Blackout();
			DEBUG_PUTS("Blackout");
		} else {
			m_pOscServerHandler->Update();
			DEBUG_PUTS("Update");
		}

		return;
	}

	if (osc::is_match(s_pUdpBuffer, s_aPathSecond)) {
		const auto nArgc = Msg.GetArgc();

		if (nArgc == 1) { // /path/N 'i' or 'f'
			const auto nChannel = static_cast<uint16_t>(GetChannel(s_pUdpBuffer));

			if (nChannel >= 1 && nChannel <= lightset::dmx::UNIVERSE_SIZE) {
				uint8_t nData;

				if (Msg.GetType(0) == osc::type::INT32) {
					DEBUG_PUTS("i received");
					nData = static_cast<uint8_t>(Msg.GetInt(0));
				} else if (Msg.GetType(0) == osc::type::FLOAT) {
					DEBUG_PRINTF("f received %f", Msg.GetFloat(0));
					nData = static_cast<uint8_t>(Msg.GetFloat(0) * lightset::dmx::MAX_VALUE);
				} else {
					return;
				}

				DEBUG_PRINTF("Channel = %d, Data = %.2x", nChannel, nData);

				bIsDmxDataChanged = IsDmxDataChanged(&nData, nChannel, 1);

				if (bIsDmxDataChanged || m_bEnableNoChangeUpdate) {
					if (!m_bPartialTransmission) {
						m_pLightSet->SetData(0, s_pData, lightset::dmx::UNIVERSE_SIZE);
					} else {
						m_nLastChannel = nChannel > m_nLastChannel ? nChannel : m_nLastChannel;
						m_pLightSet->SetData(0, s_pData, m_nLastChannel);
					}

					if (!m_bIsRunning) {
						m_bIsRunning = true;
						m_pLightSet->Start(0);
					}
				}
			}
		}

		return;
	}

	if (osc::is_match(s_pUdpBuffer, "/ping")) {
		DEBUG_PUTS("ping received");
		OscSimpleSend MsgSend(m_nHandle, nRemoteIp, m_nPortOutgoing, "/pong", nullptr);

		return;
	}

	if (osc::is_match(s_pUdpBuffer, s_aPathInfo)) {
		OscSimpleSend MsgSendInfo(m_nHandle, nRemoteIp, m_nPortOutgoing, "/info/os", "s", m_Os);
		OscSimpleSend MsgSendModel(m_nHandle, nRemoteIp, m_nPortOutgoing, "/info/model", "s", m_pModel);
		OscSimpleSend MsgSendSoc(m_nHandle, nRemoteIp, m_nPortOutgoing, "/info/soc", "s", m_pSoC);

		if (m_pOscServerHandler != nullptr) {
			m_pOscServerHandler->Info(m_nHandle, nRemoteIp, m_nPortOutgoing);
		}

		return;
	}
}

void OscServer::Print() {
	puts("OSC Server");
	printf(" Incoming Port        : %d\n", m_nPortIncoming);
	printf(" Outgoing Port        : %d\n", m_nPortOutgoing);
	printf(" DMX Path             : [%s][%s]\n", s_aPath, s_aPathSecond);
	printf("  Blackout Path       : [%s]\n", s_aPathBlackOut);
	printf(" Partial Transmission : %s\n", m_bPartialTransmission ? "Yes" : "No");
}
