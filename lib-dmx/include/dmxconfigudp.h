/**
 * @file dmxconfigudp.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXCONFIGUDP_H_
#define DMXCONFIGUDP_H_

#include <cstdint>
#include <cstring>
#include <cassert>

#include "network.h"
#include "dmx.h"

#include "debug.h"

/**
 * Example's udp message: dmx!break#100  dmx!refresh#30 dmx!mab#20 dmx!slots#128
 * min length: dmx!mab#12 => 10 bytes
 * max length: dmx!mab#1000000/n => 16 bytes
 */

namespace dmxconfigudp {
static constexpr uint32_t MIN_SIZE = 10U;
static constexpr uint32_t MAX_SIZE = 16U;
static constexpr uint16_t UDP_PORT = 5120U;
template<class T>
static constexpr bool validate(const T& n, const T& min, const T& max) {
	return (n >= min) && (n <= max);
}
static uint32_t atoi(const char *pBuffer, uint32_t nSize) {
	assert(pBuffer != nullptr);
	assert(nSize <= 7); // 1000000

	const char *p = pBuffer;
	uint32_t res = 0;

	for (; (nSize > 0) && (*p >= '0' && *p <= '9'); nSize--) {
		res = res * 10 + static_cast<uint32_t>(*p - '0');
		p++;
	}

	return res;
}
}  // namespace dmxconfigudp

class DmxConfigUdp {
public:
	DmxConfigUdp() {
		assert(s_nHandle == -1);
		s_nHandle = Network::Get()->Begin(dmxconfigudp::UDP_PORT);
	}

	~DmxConfigUdp() {
		assert(s_nHandle != -1);
		Network::Get()->End(dmxconfigudp::UDP_PORT);
		s_nHandle = -1;
	}

	void Run() {
		uint32_t nIPAddressFrom;
		uint16_t nForeignPort;
		auto nBytesReceived = Network::Get()->RecvFrom(s_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&s_pUdpBuffer)), &nIPAddressFrom, &nForeignPort);

		if (__builtin_expect((!dmxconfigudp::validate(nBytesReceived, dmxconfigudp::MIN_SIZE, dmxconfigudp::MAX_SIZE)), 1)) {
			return;
		}

		if (memcmp("dmx!", s_pUdpBuffer, 4) != 0) {
			return;
		}

		if (s_pUdpBuffer[nBytesReceived - 1] == '\n') {
			nBytesReceived--;
		}

		DEBUG_PRINTF("nBytesReceived=%u", nBytesReceived);

		const auto *pCmd = &s_pUdpBuffer[4];

		if (dmxconfigudp::validate(nBytesReceived, static_cast<uint32_t>(12), static_cast<uint32_t>(13)) && (memcmp("break#", pCmd, 6) == 0)) {
			const auto nBreakTime = dmxconfigudp::atoi(reinterpret_cast<const char*>(&s_pUdpBuffer[10]), nBytesReceived - 10U);
			if (nBreakTime >= dmx::transmit::BREAK_TIME_MIN) {
				Dmx::Get()->SetDmxBreakTime(nBreakTime);
			}
			return;
		}

		if (dmxconfigudp::validate(nBytesReceived, static_cast<uint32_t>(10), static_cast<uint32_t>(16)) && (memcmp("mab#", pCmd, 4) == 0)) {
			const auto nMapTime = dmxconfigudp::atoi(reinterpret_cast<const char*>(&s_pUdpBuffer[8]), nBytesReceived - 8U);
			if (dmxconfigudp::validate(nMapTime, dmx::transmit::MAB_TIME_MIN, dmx::transmit::MAB_TIME_MAX)) {
				Dmx::Get()->SetDmxMabTime(nMapTime);
			}
			return;
		}

		if (dmxconfigudp::validate(nBytesReceived, static_cast<uint32_t>(13), static_cast<uint32_t>(14)) && (memcmp("refresh#", pCmd, 8) == 0)) {
			const auto nRefreshRate = dmxconfigudp::atoi(reinterpret_cast<const char*>(&s_pUdpBuffer[12]), nBytesReceived - 12U);
			uint32_t nPeriodTime = 0;
			if (nRefreshRate != 0) {
				nPeriodTime = 1000000U / nRefreshRate;
			}
			Dmx::Get()->SetDmxPeriodTime(nPeriodTime);
			return;
		}

		if (dmxconfigudp::validate(nBytesReceived, static_cast<uint32_t>(11), static_cast<uint32_t>(13)) && (memcmp("slots#", pCmd, 6) == 0)) {
			const auto nSlots = dmxconfigudp::atoi(reinterpret_cast<const char*>(&s_pUdpBuffer[10]), nBytesReceived - 10U);
				if (dmxconfigudp::validate(nSlots, dmx::min::CHANNELS, dmx::max::CHANNELS)) {
				Dmx::Get()->SetDmxSlots(static_cast<uint16_t>(nSlots));
			}
			return;
		}
	}

private:
	int32_t s_nHandle { -1 };
	char *s_pUdpBuffer { nullptr };
};

#endif /* DMXCONFIGUDP_H_ */
