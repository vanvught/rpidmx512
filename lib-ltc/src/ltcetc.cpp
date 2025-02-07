/**
 * @file ltcetc.cpp
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_LTCETC)
# undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "ltcetc.h"

#include "network.h"

static constexpr char UDP_TERMINATOR[static_cast<uint32_t>(ltcetc::UdpTerminator::UNDEFINED)][5] = { "None", "CR", "LF", "CRLF" };

namespace ltcetc {
UdpTerminator get_udp_terminator(const char *pString) {
	assert(pString != nullptr);

	for (uint32_t i = 0; i < sizeof(UDP_TERMINATOR)/ sizeof(UDP_TERMINATOR[0]); i++) {
		if (strcasecmp(pString, UDP_TERMINATOR[i]) == 0) {
			return static_cast<UdpTerminator>(i);
		}
	}

	return UdpTerminator::UNDEFINED;
}

const char *get_udp_terminator(const UdpTerminator updTerminator) {
	if (updTerminator < UdpTerminator::UNDEFINED) {
		return UDP_TERMINATOR[static_cast<uint32_t>(updTerminator)];
	}

	return "Undefined";
}
}  // namespace ltcetc

void LtcEtc::Start() {
	if ((m_Config.nDestinationIp != 0) && (m_Config.nDestinationPort != 0)) {
		m_Handle.Destination = Network::Get()->Begin(m_Config.nDestinationPort);
	}

	if (m_Config.nSourcePort != 0) {
		m_Handle.Source = Network::Get()->Begin(m_Config.nSourcePort, StaticCallbackFunction);

		if ((m_Handle.Source >= 0) && (m_Config.nSourceMulticastIp != 0)) {
			Network::Get()->JoinGroup(m_Handle.Source, m_Config.nSourceMulticastIp);
		}
	}

	auto *p = s_SendBuffer;
	memcpy(p, Udp::PREFIX, sizeof(Udp::PREFIX));
	p += sizeof(Udp::PREFIX);
	memcpy(p, Udp::HEADER, sizeof(Udp::HEADER));
	p += sizeof(Udp::HEADER);
	p[2] = ' ';
	p[5] = ' ';
	p[8] = ' ';
	p[11] = ' ';
	p += sizeof(Udp::TIMECODE);
	memcpy(p, Udp::END, sizeof(Udp::END));

	DEBUG_PRINTF("m_Handle.Destination=%d, m_Handle.Source=%d", m_Handle.Destination, m_Handle.Source);
}

#define TO_HEX(i)	static_cast<char>(((i) < 10) ? '0' + (i) : 'A' + ((i) - 10))

void LtcEtc::Send(const midi::Timecode *pTimeCode) {
	if (m_Handle.Destination < 0) {
		return;
	}

	assert(pTimeCode != nullptr);

	auto *p = &s_SendBuffer[sizeof(Udp::PREFIX) + sizeof(Udp::HEADER)];

	const auto data5  = static_cast<uint8_t>(((pTimeCode->nType) & 0x03) << 5) | (pTimeCode->nHours & 0x1F);

	*p++ = TO_HEX(data5 >> 4);
	*p = TO_HEX(data5 & 0x0F);
	p += 2;
	*p++ = TO_HEX(pTimeCode->nMinutes >> 4);
	*p = TO_HEX(pTimeCode->nMinutes & 0x0F);
	p += 2;
	*p++ = TO_HEX(pTimeCode->nSeconds >> 4);
	*p = TO_HEX(pTimeCode->nSeconds & 0x0F);
	p += 2;
	*p++ = TO_HEX(pTimeCode->nFrames >> 4);
	*p = TO_HEX(pTimeCode->nFrames & 0x0F);

	uint16_t nLength = Udp::MIN_MSG_LENGTH;

	switch (m_Config.terminator) {
		case ltcetc::UdpTerminator::CR:
			s_SendBuffer[Udp::MIN_MSG_LENGTH] = 0x0D;
			nLength++;
			break;
		case ltcetc::UdpTerminator::LF:
			s_SendBuffer[Udp::MIN_MSG_LENGTH] = 0x0A;
			nLength++;
			break;
		case ltcetc::UdpTerminator::CRLF:
			s_SendBuffer[Udp::MIN_MSG_LENGTH] = 0x0D;
			s_SendBuffer[Udp::MIN_MSG_LENGTH + 1] = 0x0A;
			nLength = static_cast<uint16_t>(nLength + 2);
			break;
		default:
			break;
	}

	Network::Get()->SendTo(m_Handle.Destination, s_SendBuffer, nLength, m_Config.nDestinationIp, m_Config.nDestinationPort);

#ifndef NDEBUG
	debug_dump(s_SendBuffer, nLength);
#endif
}

/**
 * @brief Processes an incoming UDP packet.
 *
 * @param pBuffer Pointer to the packet buffer.
 * @param nSize Size of the packet buffer.
 * @param nFromIp IP address of the sender.
 * @param nFromPort Port number of the sender.
 */
void LtcEtc::Input(const uint8_t *pBuffer, uint32_t nSize, [[maybe_unused]] uint32_t nFromIp, [[maybe_unused]] uint16_t nFromPort) {
	m_pUdpBuffer = reinterpret_cast<const char *>(pBuffer);
#ifndef NDEBUG
	debug_dump(m_pUdpBuffer, nSize);
#endif

	if (nSize == Udp::MIN_MSG_LENGTH) {
		if (m_Config.terminator != ltcetc::UdpTerminator::NONE) {
			return;
		}
	}

	if (nSize == 1 + Udp::MIN_MSG_LENGTH) {
		if (m_Config.terminator == ltcetc::UdpTerminator::CRLF) {
			DEBUG_EXIT
			return;
		}

		if ((m_Config.terminator == ltcetc::UdpTerminator::CR) && (m_pUdpBuffer[Udp::MIN_MSG_LENGTH] != 0x0D)) {
			DEBUG_EXIT
			return;
		}

		if ((m_Config.terminator == ltcetc::UdpTerminator::LF) && (m_pUdpBuffer[Udp::MIN_MSG_LENGTH] != 0x0A)) {
			DEBUG_EXIT
			return;
		}
	}

	if (nSize == 2 + Udp::MIN_MSG_LENGTH) {
		if (m_Config.terminator != ltcetc::UdpTerminator::CRLF) {
			DEBUG_EXIT
			return;
		}

		if ((m_pUdpBuffer[Udp::MIN_MSG_LENGTH] != 0x0D) || (m_pUdpBuffer[1 + Udp::MIN_MSG_LENGTH] != 0x0A)) {
			DEBUG_EXIT;
			return;
		}
	}

	if (memcmp(m_pUdpBuffer, s_SendBuffer, sizeof(Udp::PREFIX) + sizeof(Udp::HEADER)) != 0) {
		DEBUG_EXIT;
		return;
	}

	ParseTimeCode();
}

static uint8_t from_hex(const char *pHex) {
	const auto nLow = (pHex[1] > '9' ? (pHex[1] | 0x20) - 'a' + 10 : pHex[1] - '0');
	const auto nHigh = (pHex[0] > '9' ? (pHex[0] | 0x20) - 'a' + 10 : pHex[0] - '0');
	return static_cast<uint8_t>((nHigh << 4) | nLow);
}

void LtcEtc::ParseTimeCode() {
	if (s_pLtcEtcHandler != nullptr) {
		midi::Timecode timeCode;

		auto *p = &m_pUdpBuffer[sizeof(Udp::PREFIX) + sizeof(Udp::HEADER)];
		auto data = from_hex(p);

		timeCode.nHours = data & 0x1F;
		timeCode.nType = static_cast<uint8_t>(data >> 5);

		p += 3;

		timeCode.nMinutes = from_hex(p);

		p += 3;

		timeCode.nSeconds = from_hex(p);

		p += 3;

		timeCode.nFrames = from_hex(p);

		DEBUG_PRINTF("%d:%d:%d:%d.%d", timeCode.nHours, timeCode.nMinutes, timeCode.nSeconds, timeCode.nFrames, timeCode.nType);

		s_pLtcEtcHandler->Handler(&timeCode);
	}
}

void LtcEtc::Print() {
	puts("ETC gateway");

	if ((m_Config.nDestinationIp != 0) && (m_Config.nDestinationPort != 0)) {
		printf(" Destination: " IPSTR ":%d\n", IP2STR(m_Config.nDestinationIp), m_Config.nDestinationPort);
	} else {
		puts(" No output");
	}

	if (m_Config.nSourcePort != 0) {
		printf ("Source port: %d\n", m_Config.nSourcePort);
		if (m_Config.nSourceMulticastIp != 0) {
			printf(" Multicast ip: " IPSTR, IP2STR(m_Config.nSourceMulticastIp));
		}
	} else {
		puts(" No input");
	}

	printf(" UDP Termination: %s\n", get_udp_terminator(m_Config.terminator));
}
