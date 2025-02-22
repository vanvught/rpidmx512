/**
 * @file dmxsend.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXSEND_H_
#define DMXSEND_H_

#include <cstdint>
#include <climits>
#include <cstdio>
#include <cassert>

#include "dmxnode.h"
#include "dmxnodedata.h"

#include "dmx.h"
#if !defined (CONFIG_DMXSEND_DISABLE_CONFIGUDP)
# include "dmxconfigudp.h"
#endif
#include "panel_led.h"
#include "hardware.h"

#include "debug.h"

class DmxSend {
public:
	void Start(const uint32_t nPortIndex)  {
		DEBUG_ENTRY
		DEBUG_PRINTF("nPortIndex=%d", nPortIndex);

		assert(nPortIndex < CHAR_BIT);

		if (is_started(m_nStarted, nPortIndex)) {
			DEBUG_EXIT
			return;
		}

		m_nStarted = static_cast<uint8_t>(m_nStarted | (1U << nPortIndex));

		Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::OUTP, true);

		if (Dmx::Get()->GetOutputStyle(nPortIndex) == dmx::OutputStyle::CONTINOUS) {
			hal::panel_led_on(hal::panelled::PORT_A_TX << nPortIndex);
		}

		DEBUG_EXIT
	}

	void Stop(const uint32_t nPortIndex)  {
		DEBUG_ENTRY
		DEBUG_PRINTF("nPortIndex=%d -> %u", nPortIndex,	is_started(m_nStarted, static_cast<uint8_t>(nPortIndex)));

		assert(nPortIndex < CHAR_BIT);

		if (!is_started(m_nStarted, nPortIndex)) {
			DEBUG_EXIT
			return;
		}

		m_nStarted = static_cast<uint8_t>(m_nStarted & ~(1U << nPortIndex));

		Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::OUTP, false);

		hal::panel_led_off(hal::panelled::PORT_A_TX << nPortIndex);

		DEBUG_EXIT
	}

	void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true)  {
		assert(nPortIndex < CHAR_BIT);
		assert(pData != nullptr);
		assert(nLength != 0);

		if (doUpdate) {
			Dmx::Get()->SetSendDataWithoutSC(nPortIndex, pData, nLength);
			Dmx::Get()->StartOutput(nPortIndex);
			hal::panel_led_on(hal::panelled::PORT_A_TX << nPortIndex);
		}
	}

	void Sync(const uint32_t nPortIndex)  {
		const auto nLightsetOffset = nPortIndex + dmxnode::DMXPORT_OFFSET;
		assert(dmxnode::Data::GetLength(nLightsetOffset) != 0);
		Dmx::Get()->SetSendDataWithoutSC(nPortIndex, dmxnode::Data::Backup(nLightsetOffset), dmxnode::Data::GetLength(nLightsetOffset));
	}

	void Sync()  {
		Dmx::Get()->Sync();

		for (uint32_t nPortIndex = 0; nPortIndex < dmx::config::max::PORTS; nPortIndex++) {
			const auto nLightsetOffset = nPortIndex + dmxnode::DMXPORT_OFFSET;
			if (dmxnode::Data::GetLength(nLightsetOffset) != 0) {
				dmxnode::Data::ClearLength(nLightsetOffset);
				hal::panel_led_on(hal::panelled::PORT_A_TX << nPortIndex);
				if (!is_started(m_nStarted, nPortIndex)) {
					Start(nPortIndex);
				}
			}
		}
	}

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle(const uint32_t nPortIndex, const dmxnode::OutputStyle outputStyle)  {
		Dmx::Get()->SetOutputStyle(nPortIndex, outputStyle == dmxnode::OutputStyle::CONSTANT ? dmx::OutputStyle::CONTINOUS : dmx::OutputStyle::DELTA);
	}

	dmxnode::OutputStyle GetOutputStyle(const uint32_t nPortIndex) const  {
		return Dmx::Get()->GetOutputStyle(nPortIndex) == dmx::OutputStyle::CONTINOUS ? dmxnode::OutputStyle::CONSTANT : dmxnode::OutputStyle::DELTA;
	}
#endif

	void Blackout([[maybe_unused]] bool bBlackout)  {
		Dmx::Get()->Blackout();
	}

	void FullOn()  {
		Dmx::Get()->FullOn();
	}

	void Print()  {
		puts("DMX Send");
		printf(" Break time   : %u\n", static_cast<unsigned int>(Dmx::Get()->GetDmxBreakTime()));
		printf(" MAB time     : %u\n", static_cast<unsigned int>(Dmx::Get()->GetDmxMabTime()));
		printf(" Refresh rate : %u\n", static_cast<unsigned int>(1000000U / Dmx::Get()->GetDmxPeriodTime()));
		printf(" Slots        : %u\n", Dmx::Get()->GetDmxSlots());
	}

	/*
	 * Art-Net ArtPollReply
	 */
	uint32_t GetUserData() { return 0; }
	uint32_t GetRefreshRate()  {
		return 1000000U / Dmx::Get()->GetDmxPeriodTime();
	}

	/*
	 *
	 */

	uint16_t GetDmxStartAddress() {	return dmxnode::START_ADDRESS_DEFAULT; }
	bool SetDmxStartAddress([[maybe_unused]] const uint16_t nDmxStartAddress) { return false; }
	uint16_t GetDmxFootprint() { return dmxnode::UNIVERSE_SIZE; }
	bool GetSlotInfo([[maybe_unused]] const uint16_t nSlotOffset, dmxnode::SlotInfo &slotInfo) {
		slotInfo.nType = 0x00; // ST_PRIMARY
		slotInfo.nCategory = 0x0001; // SD_INTENSITY
		return true;
	}

private:
	static constexpr bool is_started(const uint8_t v, const uint32_t p) {
		return (v & (1U << p)) == (1U << p);
	}

private:
#if !defined (CONFIG_DMXSEND_DISABLE_CONFIGUDP)
	DmxConfigUdp m_DmxConfigUdp;
#endif
	uint8_t m_nStarted { 0 };
};

#endif /* DMXSEND_H_ */
