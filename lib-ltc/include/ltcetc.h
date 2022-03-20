/**
 * @file ltcetc.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cassert>

#include "midi.h"
#include "network.h"

#include "debug.h"

namespace ltc {
namespace etc {

enum class UdpTerminator {
	NONE, CR, LF, CRLF, UNDEFINED
};
UdpTerminator get_udp_terminator(const char *pString);
const char* get_udp_terminator(const UdpTerminator updTerminator);
}  // namespace etc
}  // namespace ltc

class LtcEtcHandler {
public:
	virtual ~LtcEtcHandler() {
	}
	;

	virtual void Handler(const midi::Timecode *pTimeCode)=0;
};

class LtcEtc {
public:
	LtcEtc() {
		assert(s_pThis == nullptr);
		s_pThis = this;

		s_Handle.Destination = -1;
		s_Handle.Source = -1;
	}

	void SetDestinationIp(uint32_t nDestinationIp) {
		if ((network::is_private_ip(nDestinationIp) || network::is_multicast_ip(nDestinationIp))) {
			s_Config.nDestinationIp = nDestinationIp;
		} else {
			s_Config.nDestinationIp =  0;
		}
	}

	void SetDestinationPort(uint16_t nDestinationPort) {
		if (nDestinationPort > 1023) {
			s_Config.nDestinationPort = nDestinationPort;
		} else {
			s_Config.nDestinationPort = 0;
		}
	}

	void SetSourceMulticastIp(uint32_t nSourceMulticastIp) {
		if (network::is_multicast_ip(nSourceMulticastIp)) {
			s_Config.nSourceMulticastIp = nSourceMulticastIp;
		} else {
			s_Config.nSourceMulticastIp =  0;
		}
	}

	void SetSourcePort(uint16_t nSourcePort) {
		if (nSourcePort > 1023) {
			s_Config.nSourcePort = nSourcePort;
		} else {
			s_Config.nSourcePort = 0;
		}
	}

	void SetUdpTerminator(ltc::etc::UdpTerminator terminator) {
		if (terminator < ltc::etc::UdpTerminator::UNDEFINED) {
			s_Config.terminator = terminator;
		}
	}

	void SetHandler(LtcEtcHandler *pLtcEtcHandler) {
		s_pLtcEtcHandler = pLtcEtcHandler;
	}

	void Start();

	void Send(const midi::Timecode *pTimeCode);

	void Run();

	void Print();

	static LtcEtc* Get() {
		return s_pThis;
	}

private:
	void ParseTimeCode();

private:
	struct Config {
		uint32_t nDestinationIp;
		uint32_t nSourceMulticastIp;
		uint16_t nDestinationPort;
		uint16_t nSourcePort;
		ltc::etc::UdpTerminator terminator;
	};
	static Config s_Config;

	struct Handle {
		int Destination;
		int Source;
	};
	static Handle s_Handle;

	static LtcEtcHandler *s_pLtcEtcHandler;
	static char *s_pUdpBuffer;
	static LtcEtc *s_pThis;
};

#endif /* LTCETC_H_ */
