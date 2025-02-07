/**
 * @file ntpserver.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NTPSERVER_H_
#define NTPSERVER_H_

#include <cstdint>
#include <time.h>

#include "ltc.h"
#include "net/protocol/ntp.h"
#include "network.h"

#include "debug.h"

class NtpServer {
public:
	NtpServer(const uint32_t nYear, const uint32_t nMonth, const uint32_t nDay);
	~NtpServer();

	void Start();
	void Stop();

	void SetTimeCode(const struct ltc::TimeCode *pLtcTimeCode) {
		m_TimeDate = m_Time;
		m_TimeDate += pLtcTimeCode->nSeconds;
		m_TimeDate += pLtcTimeCode->nMinutes * 60U;
		m_TimeDate += pLtcTimeCode->nHours * 60U * 60U;

		const auto type = static_cast<ltc::Type>(pLtcTimeCode->nType);

		if (type == ltc::Type::FILM) {
			m_nFraction = static_cast<uint32_t>((178956970.625 * pLtcTimeCode->nFrames));
		} else if (type == ltc::Type::EBU) {
			m_nFraction = static_cast<uint32_t>((171798691.8 * pLtcTimeCode->nFrames));
		} else if ((type == ltc::Type::DF) || (type == ltc::Type::SMPTE)) {
			m_nFraction = static_cast<uint32_t>((143165576.5 * pLtcTimeCode->nFrames));
		} else {
			assert(0);
		}

		m_Reply.ReferenceTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(m_TimeDate));
		m_Reply.ReferenceTimestamp_f = __builtin_bswap32(m_nFraction);
		m_Reply.ReceiveTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(m_TimeDate));
		m_Reply.ReceiveTimestamp_f = __builtin_bswap32(m_nFraction);
		m_Reply.TransmitTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(m_TimeDate));
		m_Reply.TransmitTimestamp_f = __builtin_bswap32(m_nFraction);
	}

	/**
	 * @brief Processes an incoming UDP packet.
	 *
	 * @param pBuffer Pointer to the packet buffer.
	 * @param nSize Size of the packet buffer.
	 * @param nFromIp IP address of the sender.
	 * @param nFromPort Port number of the sender.
	 */
	void Input(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		if (__builtin_expect((nSize != sizeof(struct ntp::Packet)), 0)) {
			return;
		}

		auto *pRequest = reinterpret_cast<const ntp::Packet *>(pBuffer);

		if (__builtin_expect(((pRequest->LiVnMode & ntp::MODE_CLIENT) != ntp::MODE_CLIENT), 0)) {
			return;
		}

		m_Reply.ReferenceID = Network::Get()->GetIp();
		m_Reply.OriginTimestamp_s = pRequest->TransmitTimestamp_s;
		m_Reply.OriginTimestamp_f = pRequest->TransmitTimestamp_f;

		Network::Get()->SendTo(m_nHandle, &m_Reply, sizeof(struct ntp::Packet), nFromIp, nFromPort);
	}

	void Print();

	static NtpServer *Get() {
		return s_pThis;
	}

private:
	/**
	 * @brief Static callback function for receiving UDP packets.
	 *
	 * @param pBuffer Pointer to the packet buffer.
	 * @param nSize Size of the packet buffer.
	 * @param nFromIp IP address of the sender.
	 * @param nFromPort Port number of the sender.
	 */
	void static StaticCallbackFunction(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		s_pThis->Input(pBuffer, nSize, nFromIp, nFromPort);
	}

private:
	time_t m_Time { 0 };
	time_t m_TimeDate { 0 };
	uint32_t m_nFraction { 0 };
	int32_t m_nHandle { -1 };
	ntp::Packet m_Reply;

	static inline NtpServer *s_pThis;
};

#endif /* NTPSERVER_H_ */
