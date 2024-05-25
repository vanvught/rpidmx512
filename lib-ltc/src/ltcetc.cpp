/**
 * @file ltcetc.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstring>
#include <cstdio>
#include <cassert>

#include "ltcetc.h"

#include "network.h"

LtcEtc *LtcEtc::s_pThis;
LtcEtcHandler *LtcEtc::s_pLtcEtcHandler;
char *LtcEtc::s_pUdpBuffer;

LtcEtc::Config LtcEtc::s_Config;
LtcEtc::Handle LtcEtc::s_Handle;

static constexpr char UDP_TERMINATOR[static_cast<uint32_t>(ltc::etc::UdpTerminator::UNDEFINED)][5] = { "None", "CR", "LF", "CRLF" };

namespace ltc {
namespace etc {
ltc::etc::UdpTerminator get_udp_terminator(const char *pString) {
	assert(pString != nullptr);

	for (uint32_t i = 0; i < sizeof(UDP_TERMINATOR)/ sizeof(UDP_TERMINATOR[0]); i++) {
		if (strcasecmp(pString, UDP_TERMINATOR[i]) == 0) {
			return static_cast<ltc::etc::UdpTerminator>(i);
		}
	}

	return ltc::etc::UdpTerminator::UNDEFINED;
}

const char *get_udp_terminator(const ltc::etc::UdpTerminator updTerminator) {
	if (updTerminator < ltc::etc::UdpTerminator::UNDEFINED) {
		return UDP_TERMINATOR[static_cast<uint32_t>(updTerminator)];
	}

	return "Undefined";
}

namespace udp {
static constexpr char PREFIX[] = { 'M', 'I', 'D', 'I', ' ' };
static constexpr char HEADER[] = { 'F', '0', ' ', '7', 'F', ' ', '7', 'F', ' ', '0', '1', ' ', '0', '1', ' ' };
static constexpr char TIMECODE[] = { 'H', 'H', ' ', 'M', 'M', ' ', 'S', 'S', ' ', 'F', 'F', ' ' };
static constexpr char END[] = { 'F', '7' };
static constexpr auto MIN_MSG_LENGTH = sizeof(PREFIX) + sizeof(HEADER) + sizeof(TIMECODE) + sizeof(END);
}  // namespace udp
}  // namespace etc
}  // namespace ltc

static char s_SendBuffer[ltc::etc::udp::MIN_MSG_LENGTH + 2];

void LtcEtc::Start() {
	if ((s_Config.nDestinationIp != 0) && (s_Config.nDestinationPort != 0)) {
		s_Handle.Destination = Network::Get()->Begin(s_Config.nDestinationPort);
	}

	if (s_Config.nSourcePort != 0) {
		s_Handle.Source = Network::Get()->Begin(s_Config.nSourcePort);

		if ((s_Handle.Source >= 0) && (s_Config.nSourceMulticastIp != 0)) {
			Network::Get()->JoinGroup(s_Handle.Source, s_Config.nSourceMulticastIp);
		}
	}

	auto *p = s_SendBuffer;
	memcpy(p, ltc::etc::udp::PREFIX, sizeof(ltc::etc::udp::PREFIX));
	p += sizeof(ltc::etc::udp::PREFIX);
	memcpy(p, ltc::etc::udp::HEADER, sizeof(ltc::etc::udp::HEADER));
	p += sizeof(ltc::etc::udp::HEADER);
	p[2] = ' ';
	p[5] = ' ';
	p[8] = ' ';
	p[11] = ' ';
	p += sizeof(ltc::etc::udp::TIMECODE);
	memcpy(p, ltc::etc::udp::END, sizeof(ltc::etc::udp::END));

	DEBUG_PRINTF("s_Handle.Destination=%d, s_Handle.Source=%d", s_Handle.Destination, s_Handle.Source);
}

#define TO_HEX(i)	static_cast<char>(((i) < 10) ? '0' + (i) : 'A' + ((i) - 10))

void LtcEtc::Send(const midi::Timecode *pTimeCode) {
	if (s_Handle.Destination < 0) {
		return;
	}

	assert(pTimeCode != nullptr);

	auto *p = &s_SendBuffer[sizeof(ltc::etc::udp::PREFIX) + sizeof(ltc::etc::udp::HEADER)];

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

	uint16_t nLength = ltc::etc::udp::MIN_MSG_LENGTH;

	switch (s_Config.terminator) {
		case ltc::etc::UdpTerminator::CR:
			s_SendBuffer[ltc::etc::udp::MIN_MSG_LENGTH] = 0x0D;
			nLength++;
			break;
		case ltc::etc::UdpTerminator::LF:
			s_SendBuffer[ltc::etc::udp::MIN_MSG_LENGTH] = 0x0A;
			nLength++;
			break;
		case ltc::etc::UdpTerminator::CRLF:
			s_SendBuffer[ltc::etc::udp::MIN_MSG_LENGTH] = 0x0D;
			s_SendBuffer[ltc::etc::udp::MIN_MSG_LENGTH + 1] = 0x0A;
			nLength = static_cast<uint16_t>(nLength + 2);
			break;
		default:
			break;
	}

	Network::Get()->SendTo(s_Handle.Destination, s_SendBuffer, nLength, s_Config.nDestinationIp, s_Config.nDestinationPort);

#ifndef NDEBUG
	debug_dump(s_SendBuffer, nLength);
#endif
}

void LtcEtc::Run() {
	if (s_Handle.Source < 0) {
		return;
	}

	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;

	const auto nBytesReceived = Network::Get()->RecvFrom(s_Handle.Source, const_cast<const void **>(reinterpret_cast<void **>(&s_pUdpBuffer)), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((nBytesReceived < ltc::etc::udp::MIN_MSG_LENGTH), 1)) {
		return;
	}

#ifndef NDEBUG
	debug_dump(s_pUdpBuffer, nBytesReceived);
#endif

	if (nBytesReceived == ltc::etc::udp::MIN_MSG_LENGTH) {
		if (s_Config.terminator != ltc::etc::UdpTerminator::NONE) {
			return;
		}
	}

	if (nBytesReceived == 1 + ltc::etc::udp::MIN_MSG_LENGTH) {
		if (s_Config.terminator == ltc::etc::UdpTerminator::CRLF) {
			DEBUG_EXIT
			return;
		}

		if ((s_Config.terminator == ltc::etc::UdpTerminator::CR) && (s_pUdpBuffer[ltc::etc::udp::MIN_MSG_LENGTH] != 0x0D)) {
			DEBUG_EXIT
			return;
		}

		if ((s_Config.terminator == ltc::etc::UdpTerminator::LF) && (s_pUdpBuffer[ltc::etc::udp::MIN_MSG_LENGTH] != 0x0A)) {
			DEBUG_EXIT
			return;
		}
	}

	if (nBytesReceived == 2 + ltc::etc::udp::MIN_MSG_LENGTH) {
		if (s_Config.terminator != ltc::etc::UdpTerminator::CRLF) {
			DEBUG_EXIT
			return;
		}

		if ((s_pUdpBuffer[ltc::etc::udp::MIN_MSG_LENGTH] != 0x0D) || (s_pUdpBuffer[1 + ltc::etc::udp::MIN_MSG_LENGTH] != 0x0A)) {
			DEBUG_EXIT;
			return;
		}
	}

	if (memcmp(s_pUdpBuffer, s_SendBuffer, sizeof(ltc::etc::udp::PREFIX) + sizeof(ltc::etc::udp::HEADER)) != 0) {
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

		auto *p = &s_pUdpBuffer[sizeof(ltc::etc::udp::PREFIX) + sizeof(ltc::etc::udp::HEADER)];
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
	printf("ETC gateway:\n");

	if ((s_Config.nDestinationIp != 0) && (s_Config.nDestinationPort != 0)) {
		printf(" Destination: " IPSTR ":%d\n", IP2STR(s_Config.nDestinationIp), s_Config.nDestinationPort);
	} else {
		printf(" No output\n");
	}

	if (s_Config.nSourcePort != 0) {
		printf ("Source port: %d\n", s_Config.nSourcePort);
		if (s_Config.nSourceMulticastIp != 0) {
			printf(" Multicast ip: " IPSTR, IP2STR(s_Config.nSourceMulticastIp));
		}
	} else {
		printf(" No input\n");
	}

	printf(" UDP Termination: %s\n", get_udp_terminator(s_Config.terminator));
}
