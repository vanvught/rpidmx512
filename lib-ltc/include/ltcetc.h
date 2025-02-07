/**
 * @file ltcetc.h
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

#ifndef LTCETC_H_
#define LTCETC_H_

#include <cstdint>
#include <cassert>

#include "midi.h"
#include "network.h"

#include "debug.h"

namespace ltcetc {
enum class UdpTerminator {
	NONE, CR, LF, CRLF, UNDEFINED
};
UdpTerminator get_udp_terminator(const char *pString);
const char* get_udp_terminator(const UdpTerminator updTerminator);
}  // namespace ltcetc

class LtcEtcHandler {
public:
	virtual ~LtcEtcHandler() = default;

	virtual void Handler(const midi::Timecode *pTimeCode)=0;
};

class LtcEtc {
public:
	LtcEtc() {
		assert(s_pThis == nullptr);
		s_pThis = this;
	}

	void SetDestinationIp(const uint32_t nDestinationIp) {
		if ((net::is_private_ip(nDestinationIp) || net::is_multicast_ip(nDestinationIp))) {
			m_Config.nDestinationIp = nDestinationIp;
		} else {
			m_Config.nDestinationIp =  0;
		}
	}

	void SetDestinationPort(const uint16_t nDestinationPort) {
		if (nDestinationPort > 1023) {
			m_Config.nDestinationPort = nDestinationPort;
		} else {
			m_Config.nDestinationPort = 0;
		}
	}

	void SetSourceMulticastIp(const uint32_t nSourceMulticastIp) {
		if (net::is_multicast_ip(nSourceMulticastIp)) {
			m_Config.nSourceMulticastIp = nSourceMulticastIp;
		} else {
			m_Config.nSourceMulticastIp =  0;
		}
	}

	void SetSourcePort(const uint16_t nSourcePort) {
		if (nSourcePort > 1023) {
			m_Config.nSourcePort = nSourcePort;
		} else {
			m_Config.nSourcePort = 0;
		}
	}

	void SetUdpTerminator(const ltcetc::UdpTerminator terminator) {
		if (terminator < ltcetc::UdpTerminator::UNDEFINED) {
			m_Config.terminator = terminator;
		}
	}

	void SetHandler(LtcEtcHandler *pLtcEtcHandler) {
		s_pLtcEtcHandler = pLtcEtcHandler;
	}

	void Start();

	void Send(const midi::Timecode *pTimeCode);

	void Input(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort);

	void Print();

	static LtcEtc *Get() {
		return s_pThis;
	}

private:
	void ParseTimeCode();

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
	const char *m_pUdpBuffer { nullptr };

	struct Config {
		uint32_t nDestinationIp;
		uint32_t nSourceMulticastIp;
		uint16_t nDestinationPort;
		uint16_t nSourcePort;
		ltcetc::UdpTerminator terminator;
	};

	Config m_Config = { 0, 0, 0, 0, ltcetc::UdpTerminator::NONE };

	struct Handle {
		int Destination;
		int Source;
	};

	Handle m_Handle { -1, -1 };

	LtcEtcHandler *s_pLtcEtcHandler { nullptr };

	struct Udp {
		static constexpr char PREFIX[] = { 'M', 'I', 'D', 'I', ' ' };
		static constexpr char HEADER[] = { 'F', '0', ' ', '7', 'F', ' ', '7', 'F', ' ', '0', '1', ' ', '0', '1', ' ' };
		static constexpr char TIMECODE[] = { 'H', 'H', ' ', 'M', 'M', ' ', 'S', 'S', ' ', 'F', 'F', ' ' };
		static constexpr char END[] = { 'F', '7' };
		static constexpr auto MIN_MSG_LENGTH = sizeof(PREFIX) + sizeof(HEADER) + sizeof(TIMECODE) + sizeof(END);
	};

	static inline char s_SendBuffer[Udp::MIN_MSG_LENGTH + 2];
	static inline LtcEtc *s_pThis;
};

#endif /* LTCETC_H_ */
