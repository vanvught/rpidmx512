/**
 * @file dmxsend.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cassert>

#include "lightset.h"
#include "lightsetdata.h"

#include "dmx.h"
#include "panel_led.h"
#include "hardware.h"

#include "debug.h"

class DmxSend final: public LightSet  {
public:
	void Start(const uint32_t nPortIndex) override {
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

	void Stop(const uint32_t nPortIndex) override {
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

	void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate) override {
		assert(nPortIndex < CHAR_BIT);
		assert(pData != nullptr);
		assert(nLength != 0);

		if (doUpdate) {
			Dmx::Get()->SetSendDataWithoutSC(nPortIndex, pData, nLength);
			Dmx::Get()->StartOutput(nPortIndex);
			hal::panel_led_on(hal::panelled::PORT_A_TX << nPortIndex);
		}
	}

	void Sync(uint32_t const nPortIndex) override {
		assert(lightset::Data::GetLength(nPortIndex) != 0);
		Dmx::Get()->SetSendDataWithoutSC(nPortIndex, lightset::Data::Backup(nPortIndex), lightset::Data::GetLength(nPortIndex));
	}

	void Sync() override {
		Dmx::Get()->Sync();

		for (uint32_t nPortIndex = 0; nPortIndex < dmx::config::max::PORTS; nPortIndex++) {
			if (lightset::Data::GetLength(nPortIndex) != 0) {
				lightset::Data::ClearLength(nPortIndex);
				hal::panel_led_on(hal::panelled::PORT_A_TX << nPortIndex);
				if (!is_started(m_nStarted, nPortIndex)) {
					Start(nPortIndex);
				}
			}
		}
	}

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle(const uint32_t nPortIndex, const lightset::OutputStyle outputStyle) override {
		Dmx::Get()->SetOutputStyle(nPortIndex, outputStyle == lightset::OutputStyle::CONSTANT ? dmx::OutputStyle::CONTINOUS : dmx::OutputStyle::DELTA);
	}

	lightset::OutputStyle GetOutputStyle(const uint32_t nPortIndex) const override {
		return Dmx::Get()->GetOutputStyle(nPortIndex) == dmx::OutputStyle::CONTINOUS ? lightset::OutputStyle::CONSTANT : lightset::OutputStyle::DELTA;
	}
#endif

	uint32_t GetRefreshRate() override {
		return 1000000U / Dmx::Get()->GetDmxPeriodTime();
	}

	void Blackout([[maybe_unused]] bool bBlackout) override {
		Dmx::Get()->Blackout();
	}

	void FullOn() override {
		Dmx::Get()->FullOn();
	}

	void Print() override {
		puts("DMX Send");
		printf(" Break time   : %u\n", static_cast<unsigned int>(Dmx::Get()->GetDmxBreakTime()));
		printf(" MAB time     : %u\n", static_cast<unsigned int>(Dmx::Get()->GetDmxMabTime()));
		printf(" Refresh rate : %u\n", static_cast<unsigned int>(1000000U / Dmx::Get()->GetDmxPeriodTime()));
		printf(" Slots        : %u\n", Dmx::Get()->GetDmxSlots());
	}

private:
	static constexpr bool is_started(const uint8_t v, const uint32_t p) {
		return (v & (1U << p)) == (1U << p);
	}

private:
	uint8_t m_nStarted { 0 };
};

#endif /* DMXSEND_H_ */
