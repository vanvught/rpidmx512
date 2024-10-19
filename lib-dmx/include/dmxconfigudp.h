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
static constexpr uint32_t MIN_SIZE = 10;
static constexpr uint32_t MAX_SIZE = 16;
static constexpr uint16_t UDP_PORT = 5120;

static constexpr const char CMD_BREAK[] = "break#";
static constexpr uint32_t CMD_BREAK_LENGTH = sizeof(CMD_BREAK) - 1U; // Exclude null terminator

static constexpr const char CMD_MAB[] = "mab#";
static constexpr uint32_t CMD_MAB_LENGTH = sizeof(CMD_MAB) - 1U;

static constexpr const char CMD_REFRESH[] = "refresh#";
static constexpr uint32_t CMD_REFRESH_LENGTH = sizeof(CMD_REFRESH) - 1U;

static constexpr const char CMD_SLOTS[] = "slots#";
static constexpr uint32_t CMD_SLOTS_LENGTH = sizeof(CMD_SLOTS) - 1U;

template<class T>
static constexpr bool validate(const T& n, const T& min, const T& max) {
	return (n >= min) && (n <= max);
}

inline uint32_t atoi(const char *pBuffer, const uint32_t nSize) {
    uint32_t nResult = 0;
    for (uint32_t i = 0; i < nSize && pBuffer[i] >= '0' && pBuffer[i] <= '9'; ++i) {
        nResult = nResult * 10 + static_cast<uint32_t>(pBuffer[i] - '0');
    }
    return nResult;
}
}  // namespace dmxconfigudp

class DmxConfigUdp {
public:
	DmxConfigUdp() {
		DEBUG_ENTRY

		assert(s_pThis == nullptr);
		s_pThis = this;

		assert(s_nHandle == -1);
		s_nHandle = Network::Get()->Begin(dmxconfigudp::UDP_PORT, DmxConfigUdp::staticCallbackFunction);

		DEBUG_EXIT
	}

	~DmxConfigUdp() {
		assert(s_nHandle != -1);
		Network::Get()->End(dmxconfigudp::UDP_PORT);
		s_nHandle = -1;

		s_pThis = nullptr;

		DEBUG_EXIT
	}

	void Input(const uint8_t *pBuffer, uint32_t nSize, [[maybe_unused]] uint32_t nFromIp, [[maybe_unused]] uint16_t nFromPort) {
		DEBUG_ENTRY

		if (!dmxconfigudp::validate(nSize, dmxconfigudp::MIN_SIZE, dmxconfigudp::MAX_SIZE)) {
			DEBUG_EXIT
			return;
		}

		if (memcmp("dmx!", pBuffer, 4) != 0) {
			DEBUG_EXIT
			return;
		}

		if (pBuffer[nSize - 1] == '\n') {
			nSize--;
		}

		DEBUG_PRINTF("nSize=%u", nSize);

		struct CommandHandler {
		    const char *command;
		    const uint32_t commandLength;
		    void (DmxConfigUdp::*handler)(const uint8_t *, const uint32_t);
		};

        constexpr CommandHandler commandHandlers[] = {
            {dmxconfigudp::CMD_BREAK, dmxconfigudp::CMD_BREAK_LENGTH, &DmxConfigUdp::HandleBreak},
            {dmxconfigudp::CMD_MAB, dmxconfigudp::CMD_MAB_LENGTH, &DmxConfigUdp::HandleMab},
            {dmxconfigudp::CMD_REFRESH, dmxconfigudp::CMD_REFRESH_LENGTH, &DmxConfigUdp::HandleRefresh},
            {dmxconfigudp::CMD_SLOTS, dmxconfigudp::CMD_SLOTS_LENGTH, &DmxConfigUdp::HandleSlots}
        };

        for (const auto& handler : commandHandlers) {
            if (memcmp(handler.command, &pBuffer[4], handler.commandLength) == 0) {
                (this->*handler.handler)(pBuffer, nSize);

                DEBUG_EXIT
                return;
            }
        }

        DEBUG_EXIT
	}

private:
    void HandleBreak(const uint8_t *pBuffer, const uint32_t nSize) {
    	static constexpr auto nOffset = dmxconfigudp::CMD_BREAK_LENGTH + 4;
        const auto nBreakTime = dmxconfigudp::atoi(reinterpret_cast<const char*>(&pBuffer[nOffset]), nSize - nOffset);
        if (nBreakTime >= dmx::transmit::BREAK_TIME_MIN) {
            Dmx::Get()->SetDmxBreakTime(nBreakTime);
            DEBUG_PRINTF("nBreakTime=%u", nBreakTime);
        }
    }

    void HandleMab(const uint8_t *pBuffer, const uint32_t nSize) {
    	static constexpr auto nOffset = dmxconfigudp::CMD_MAB_LENGTH + 4;
        const auto nMabTime = dmxconfigudp::atoi(reinterpret_cast<const char*>(&pBuffer[nOffset]), nSize - nOffset);
        if (dmxconfigudp::validate(nMabTime, dmx::transmit::MAB_TIME_MIN, dmx::transmit::MAB_TIME_MAX)) {
            Dmx::Get()->SetDmxMabTime(nMabTime);
            DEBUG_PRINTF("nMabTime=%u", nMabTime);
        }
    }

    void HandleRefresh(const uint8_t *pBuffer, const uint32_t nSize) {
    	static constexpr auto nOffset = dmxconfigudp::CMD_REFRESH_LENGTH + 4;
        const auto nRefreshRate = dmxconfigudp::atoi(reinterpret_cast<const char*>(&pBuffer[nOffset]), nSize - nOffset);
        uint32_t nPeriodTime = (nRefreshRate != 0) ? 1000000U / nRefreshRate : 0;
        Dmx::Get()->SetDmxPeriodTime(nPeriodTime);
        DEBUG_PRINTF("nPeriodTime=%u", nPeriodTime);
    }

    void HandleSlots(const uint8_t *pBuffer, const uint32_t nSize) {
    	static constexpr auto nOffset = dmxconfigudp::CMD_SLOTS_LENGTH + 4;
        const auto nSlots = dmxconfigudp::atoi(reinterpret_cast<const char*>(&pBuffer[nOffset]), nSize - nOffset);
        if (dmxconfigudp::validate(nSlots, dmx::min::CHANNELS, dmx::max::CHANNELS)) {
            Dmx::Get()->SetDmxSlots(static_cast<uint16_t>(nSlots));
            DEBUG_PRINTF("nSlots=%u", nSlots);
        }
    }

	void static staticCallbackFunction(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		s_pThis->Input(pBuffer, nSize, nFromIp, nFromPort);
	}

private:
	int32_t s_nHandle { -1 };
	static inline DmxConfigUdp *s_pThis;
};

#endif /* DMXCONFIGUDP_H_ */
