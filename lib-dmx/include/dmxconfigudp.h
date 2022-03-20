#if !defined(NO_EMAC)
/**
 * @file dmxconfigudp.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

/**
 * Example's udp message: dmx!break#100  dmx!refresh#30 dmx!mab#20 dmx!slots#128
 * min length: dmx!mab#12/n => 11 bytes
 * max length: dmx!mab#1000000/n => 16 bytes
 */

namespace dmxconfigudp {
static constexpr auto MIN_SIZE = 11U;
static constexpr auto MAX_SIZE = 16U;
static constexpr uint16_t UDP_PORT = 5120U;
template<class T>
static constexpr bool validate(const T& n, const T& min, const T& max) {
	return (n >= min) && (n <= max);
}
static uint32_t atoi(const char *pBuffer, uint8_t nSize) {
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
		s_nHandle = Network::Get()->End(dmxconfigudp::UDP_PORT);
	}

	void Run() {
		uint32_t nIPAddressFrom;
		uint16_t nForeignPort;
		auto nBytesReceived = Network::Get()->RecvFrom(s_nHandle, s_Buffer, dmxconfigudp::MAX_SIZE, &nIPAddressFrom, &nForeignPort);

		if (__builtin_expect((nBytesReceived < dmxconfigudp::MIN_SIZE),1)) {
			return;
		}

		if (memcmp("dmx!", s_Buffer, 4) != 0) {
			return;
		}

		if (s_Buffer[nBytesReceived - 1] == '\n') {
			nBytesReceived--;
		}

		static auto *pCmd = &s_Buffer[4];

		if (dmxconfigudp::validate(nBytesReceived, static_cast<uint16_t>(11), static_cast<uint16_t>(14)) && (memcmp("break#", pCmd, 6) == 0)) {
			const auto nBreakTime = dmxconfigudp::atoi(reinterpret_cast<const char*>(&s_Buffer[10]), 4);
			if (nBreakTime >= dmx::transmit::BREAK_TIME_MIN) {
				Dmx::Get()->SetDmxBreakTime(nBreakTime);
			}
			return;
		}

		if (dmxconfigudp::validate(nBytesReceived, static_cast<uint16_t>(9), static_cast<uint16_t>(16)) && (memcmp("mab#", pCmd, 4) == 0)) {
			const auto nMapTime = dmxconfigudp::atoi(reinterpret_cast<const char*>(&s_Buffer[8]), 7);
			if (dmxconfigudp::validate(nMapTime, dmx::transmit::MAB_TIME_MIN, dmx::transmit::MAB_TIME_MAX)) {
				Dmx::Get()->SetDmxMabTime(nMapTime);
			}
			return;
		}

		if (dmxconfigudp::validate(nBytesReceived, static_cast<uint16_t>(13), static_cast<uint16_t>(14)) && (memcmp("refresh#", pCmd, 8) == 0)) {
			const auto nRefreshRate = dmxconfigudp::atoi(reinterpret_cast<const char*>(&s_Buffer[12]), 2);
			uint32_t nPeriodTime = 0;
			if (nRefreshRate != 0) {
				nPeriodTime = 1000000U / nRefreshRate;
			}
			Dmx::Get()->SetDmxPeriodTime(nPeriodTime);
			return;
		}

		if (dmxconfigudp::validate(nBytesReceived, static_cast<uint16_t>(11), static_cast<uint16_t>(13)) && (memcmp("slots#", pCmd, 6) == 0)) {
			const auto nSlots = dmxconfigudp::atoi(reinterpret_cast<const char*>(&s_Buffer[10]), 3);
			if (dmxconfigudp::validate(nSlots, 2U, dmx::max::CHANNELS)) {
				Dmx::Get()->SetDmxSlots(static_cast<uint16_t>(nSlots));
			}
			return;
		}
	}

private:
	static int32_t s_nHandle;
	static uint8_t s_Buffer[dmxconfigudp::MAX_SIZE];
};

#endif /* DMXCONFIGUDP_H_ */
#endif
