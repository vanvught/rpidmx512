/**
 * @file ptpclient.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>

#include "ptpclient.h"
#include "ptp.h"

#include "network.h"

#include "debug.h"

using namespace ptp;

typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} pcast32;

PtpClient::PtpClient() {
	DEBUG_ENTRY

	struct in_addr group_ip;
	static_cast<void>(inet_aton(udp::MULTICAST_ADDRESS, &group_ip));
	m_nMulticastIp = group_ip.s_addr;

	memset(&m_DelayReq, 0, sizeof(m_DelayReq));
	m_DelayReq.Header.TransportType = MessageType::DELAY_REQ;
	m_DelayReq.Header.Version = 2;
	m_DelayReq.Header.MessageLength = 0x2c00;

	Network::Get()->MacAddressCopyTo(m_DelayReq.Header.ClockIdentity);
	memmove(&m_DelayReq.Header.ClockIdentity[5],&m_DelayReq.Header.ClockIdentity[3], 2);
	m_DelayReq.Header.ClockIdentity[3] = 0xFF;
	m_DelayReq.Header.ClockIdentity[4] = 0xFE;
	m_DelayReq.Header.SourcePortIdentity[1] = 1;

	m_DelayReq.Header.LogMessageInterval = 0x7f; // ?
	m_DelayReq.Header.ControlField = 1;

	DEBUG_EXIT
}

void PtpClient::Start() {
	DEBUG_ENTRY

	m_nHandleEvent = Network::Get()->Begin(udp::port::EVENT);
	assert(m_nHandleEvent != -1);

	m_nHandleGeneral = Network::Get()->Begin(udp::port::GENERAL);
	assert(m_nHandleGeneral != -1);

	Network::Get()->JoinGroup(m_nHandleEvent, m_nMulticastIp);

	DEBUG_EXIT
}

void PtpClient::Stop() {
	DEBUG_ENTRY

	Network::Get()->LeaveGroup(m_nHandleEvent, m_nMulticastIp);

	Network::Get()->End(udp::port::GENERAL);
	m_nHandleGeneral = -1;

	Network::Get()->End(udp::port::EVENT);
	m_nHandleEvent = -1;

	DEBUG_EXIT
}

void PtpClient::HandleEventMessage() {
	if (m_Message.Header.TransportType == MessageType::SYNC) {
		if ((m_Message.Header.Flags & __builtin_bswap16(Flags::TWO_STEP)) ==  __builtin_bswap16(Flags::TWO_STEP)) {
			return;
		}

		pcast32 cast;
		memcpy(cast.u8, m_Message.Sync.Seconds, 4);
		T1.Seconds = __builtin_bswap32(cast.u32);
		memcpy(cast.u8, m_Message.Sync.NanoSeconds, 4);
		T1.NanoSeconds = __builtin_bswap32(cast.u32);

		T2.Seconds = m_nSeconds;
		T2.NanoSeconds = m_nMicroSeconds / 1000;

		DEBUG_PRINTF("SYNC (%d %d) (%d %d)", T1.Seconds, T1.NanoSeconds, T2.Seconds, T2.NanoSeconds);
		return;
	}
}

void PtpClient::HandleGeneralMessage() {
	if (m_Message.Header.TransportType == ptp::MessageType::FOLLOW_UP) {
		pcast32 cast;	// T1
		memcpy(cast.u8, m_Message.FollowUp.Seconds, 4);
		T1.Seconds = __builtin_bswap32(cast.u32);
		memcpy(cast.u8, m_Message.FollowUp.NanoSeconds, 4);
		T1.NanoSeconds = __builtin_bswap32(cast.u32);

		T2.Seconds = m_nSeconds;
		T2.NanoSeconds = m_nMicroSeconds / 1000;

		m_DelayReq.Header.SequenceId[0] = m_DelayReqSequenceId >> 8;
		m_DelayReq.Header.SequenceId[1] = m_DelayReqSequenceId & 0xFF;

		GetTime();
		cast.u32 = __builtin_bswap32(m_nSeconds);
		memcpy(m_DelayReq.Seconds, cast.u8, 4);
		cast.u32 = __builtin_bswap32(1000 * m_nMicroSeconds);
		memcpy(m_DelayReq.NanoSeconds, cast.u8, 4);

		Network::Get()->SendTo(m_nHandleEvent, &m_DelayReq, sizeof(m_DelayReq), m_nMasterIpAddress, udp::port::EVENT);

		T3.Seconds = m_nSeconds;
		T3.NanoSeconds = m_nMicroSeconds / 1000;
		m_DelayReqSequenceId++;

		DEBUG_PRINTF("DELAY_REQ (%d %d)", T3.Seconds, T3.NanoSeconds);
		return;
	}

	if (m_Message.Header.TransportType == ptp::MessageType::DELAY_RESP) {
		pcast32 cast;

		memcpy(cast.u8, m_Message.DelayResp.Seconds, 4);
		T4.Seconds = __builtin_bswap32(cast.u32);
		memcpy(cast.u8, m_Message.DelayResp.NanoSeconds, 4);
		T4.NanoSeconds = __builtin_bswap32(cast.u32);

		DEBUG_PRINTF("DELAY_RESP (%d %d)", T4.Seconds, T4.NanoSeconds);

		const int32_t nOffsetSeconds = (static_cast<int32_t>(T2.Seconds - T1.Seconds) + static_cast<int32_t>(T4.Seconds - T3.Seconds)) / 2;

		DEBUG_PRINTF("((%d) + (%d)) / 2 = %d", (T2.Seconds - T1.Seconds), (T4.Seconds - T3.Seconds), nOffsetSeconds);

		return;
	}

	if (m_Message.Header.TransportType == ptp::MessageType::ANNOUNCE) {
		DEBUG_PUTS("ANNOUNCE");
		return;
	}
}

void PtpClient::Run() {
	uint16_t nForeignPort;

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandleEvent, &m_Message, sizeof(struct PTPMessage), &m_nMasterIpAddress, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived > sizeof(struct PTPHeader)), 0)) {
		GetTime();
		HandleEventMessage();
		return;
	}

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandleGeneral, &m_Message, sizeof(struct PTPMessage), &m_nMasterIpAddress, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived > sizeof(struct PTPHeader)), 0)) {
		GetTime();
		HandleGeneralMessage();
		return;
	}
}

void PtpClient::Print() {
	DEBUG_ENTRY

	DEBUG_EXIT
}
