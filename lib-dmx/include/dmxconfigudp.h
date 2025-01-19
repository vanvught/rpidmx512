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
 * @class DmxConfigUdp
 * @brief Handles UDP-based configuration of DMX parameters.
 *
 * This class listens for UDP messages to configure DMX settings such as
 * break time, MAB time, refresh rate, and the number of slots. The message
 * format is prefixed with "dmx!" followed by the configuration command.
 *
 * Example's udp message: dmx!break#100  dmx!refresh#30 dmx!mab#20 dmx!slots#128
 * min length: dmx!mab#12 => 10 bytes
 * max length: dmx!mab#1000000/n => 16 bytes
 */
class DmxConfigUdp {
	static constexpr uint32_t MIN_SIZE = 10;	///< Minimum valid size for a UDP packet.
	static constexpr uint32_t MAX_SIZE = 16;	///< Maximum valid size for a UDP packet.
	static constexpr uint16_t UDP_PORT = 5120;	///< UDP port used for receiving DMX configuration messages.

	static constexpr const char CMD_BREAK[] = "break#";
	static constexpr uint32_t CMD_BREAK_LENGTH = sizeof(CMD_BREAK) - 1U; // Exclude null terminator

	static constexpr const char CMD_MAB[] = "mab#";
	static constexpr uint32_t CMD_MAB_LENGTH = sizeof(CMD_MAB) - 1U;

	static constexpr const char CMD_REFRESH[] = "refresh#";
	static constexpr uint32_t CMD_REFRESH_LENGTH = sizeof(CMD_REFRESH) - 1U;

	static constexpr const char CMD_SLOTS[] = "slots#";
	static constexpr uint32_t CMD_SLOTS_LENGTH = sizeof(CMD_SLOTS) - 1U;

	/**
	 * @brief Validates if a value lies within a specified range.
	 *
	 * @tparam T Data type of the value being validated.
	 * @param n The value to check.
	 * @param min Minimum permissible value.
	 * @param max Maximum permissible value.
	 * @return true if `n` is within `[min, max]`, false otherwise.
	 */
	template<class T>
	static constexpr bool validate(const T& n, const T& min, const T& max) {
		return (n >= min) && (n <= max);
	}

	/**
	 * @brief Converts a string to an unsigned integer.
	 *
	 * @param pBuffer Pointer to the string buffer.
	 * @param nSize Length of the string.
	 * @return Converted unsigned integer value.
	 */
	inline uint32_t atoi(const char *pBuffer, const uint32_t nSize) {
		uint32_t nResult = 0;
		for (uint32_t i = 0; i < nSize && pBuffer[i] >= '0' && pBuffer[i] <= '9'; ++i) {
			nResult = nResult * 10 + static_cast<uint32_t>(pBuffer[i] - '0');
		}
		return nResult;
	}

public:
	/**
	 * @brief Constructs the DmxConfigUdp object and initializes UDP listening.
	 */
	DmxConfigUdp() {
		DEBUG_ENTRY

		assert(s_pThis == nullptr);
		s_pThis = this;

		assert(s_nHandle == -1);
		s_nHandle = Network::Get()->Begin(UDP_PORT, DmxConfigUdp::StaticCallbackFunction);

		DEBUG_EXIT
	}

	/**
	 * @brief Destroys the DmxConfigUdp object and stops UDP listening.
	 */
	~DmxConfigUdp() {
		DEBUG_ENTRY

		assert(s_nHandle != -1);
		Network::Get()->End(UDP_PORT);
		s_nHandle = -1;

		s_pThis = nullptr;

		DEBUG_EXIT
	}

	/**
	 * @brief Processes an incoming UDP packet.
	 *
	 * @param pBuffer Pointer to the packet buffer.
	 * @param nSize Size of the packet buffer.
	 * @param nFromIp IP address of the sender.
	 * @param nFromPort Port number of the sender.
	 */
	void Input(const uint8_t *pBuffer, uint32_t nSize, [[maybe_unused]] uint32_t nFromIp, [[maybe_unused]] uint16_t nFromPort) {
		DEBUG_ENTRY

		if (!validate(nSize, MIN_SIZE, MAX_SIZE)) {
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
				{CMD_BREAK, CMD_BREAK_LENGTH, &DmxConfigUdp::HandleBreak},
				{CMD_MAB, CMD_MAB_LENGTH, &DmxConfigUdp::HandleMab},
				{CMD_REFRESH, CMD_REFRESH_LENGTH, &DmxConfigUdp::HandleRefresh},
				{CMD_SLOTS, CMD_SLOTS_LENGTH, &DmxConfigUdp::HandleSlots}
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
	/**
	 * @brief Handles the "break#" command to configure DMX break time.
	 *
	 * @param pBuffer Pointer to the packet buffer.
	 * @param nSize Size of the packet buffer.
	 */
	void HandleBreak(const uint8_t *pBuffer, const uint32_t nSize) {
		static constexpr auto nOffset = CMD_BREAK_LENGTH + 4;
		const auto nBreakTime = atoi(reinterpret_cast<const char*>(&pBuffer[nOffset]), nSize - nOffset);
		if (nBreakTime >= dmx::transmit::BREAK_TIME_MIN) {
			Dmx::Get()->SetDmxBreakTime(nBreakTime);
			DEBUG_PRINTF("nBreakTime=%u", nBreakTime);
		}
	}

	/**
	 * @brief Handles the "mab#" command to configure DMX MAB time.
	 *
	 * @param pBuffer Pointer to the packet buffer.
	 * @param nSize Size of the packet buffer.
	 */
	void HandleMab(const uint8_t *pBuffer, const uint32_t nSize) {
		static constexpr auto nOffset = CMD_MAB_LENGTH + 4;
		const auto nMabTime = atoi(reinterpret_cast<const char*>(&pBuffer[nOffset]), nSize - nOffset);
		if (validate(nMabTime, dmx::transmit::MAB_TIME_MIN, dmx::transmit::MAB_TIME_MAX)) {
			Dmx::Get()->SetDmxMabTime(nMabTime);
			DEBUG_PRINTF("nMabTime=%u", nMabTime);
		}
	}

	/**
	 * @brief Handles the "refresh#" command to configure DMX refresh rate.
	 *
	 * @param pBuffer Pointer to the packet buffer.
	 * @param nSize Size of the packet buffer.
	 */
	void HandleRefresh(const uint8_t *pBuffer, const uint32_t nSize) {
		static constexpr auto nOffset = CMD_REFRESH_LENGTH + 4;
		const auto nRefreshRate = atoi(reinterpret_cast<const char*>(&pBuffer[nOffset]), nSize - nOffset);
		uint32_t nPeriodTime = (nRefreshRate != 0) ? 1000000U / nRefreshRate : 0;
		Dmx::Get()->SetDmxPeriodTime(nPeriodTime);
		DEBUG_PRINTF("nPeriodTime=%u", nPeriodTime);
	}

	/**
	 * @brief Handles the "slots#" command to configure DMX slot count.
	 *
	 * @param pBuffer Pointer to the packet buffer.
	 * @param nSize Size of the packet buffer.
	 */
	void HandleSlots(const uint8_t *pBuffer, const uint32_t nSize) {
		static constexpr auto nOffset = CMD_SLOTS_LENGTH + 4;
		const auto nSlots = atoi(reinterpret_cast<const char*>(&pBuffer[nOffset]), nSize - nOffset);
		if (validate(nSlots, dmx::min::CHANNELS, dmx::max::CHANNELS)) {
			Dmx::Get()->SetDmxSlots(static_cast<uint16_t>(nSlots));
			DEBUG_PRINTF("nSlots=%u", nSlots);
		}
	}

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
	int32_t s_nHandle { -1 };				///< UDP handle for the network interface.
	static inline DmxConfigUdp *s_pThis;	///< Static instance pointer for the callback function.
};

#endif /* DMXCONFIGUDP_H_ */
