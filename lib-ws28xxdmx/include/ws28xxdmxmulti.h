/**
 * @file ws28xxdmxmulti.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef WS28XXDMXMULTI_H_
#define WS28XXDMXMULTI_H_

#include <cstdint>
#include <cassert>

#include "lightset.h"
#include "lightsetdata.h"

#include "ws28xxmulti.h"

#include "pixeldmxconfiguration.h"
#include "pixelpatterns.h"

#include "logic_analyzer.h"

namespace ws28xxdmxmulti {
#if !defined (CONFIG_PIXELDMX_MAX_PORTS)
# define CONFIG_PIXELDMX_MAX_PORTS	8
#endif
static constexpr auto MAX_PORTS = CONFIG_PIXELDMX_MAX_PORTS;
}  // namespace ws28xxdmxmulti

class WS28xxDmxMulti final: public LightSet {
public:
	WS28xxDmxMulti();
	~WS28xxDmxMulti() override;

	void Start(const uint32_t nPortIndex) override;
	void Stop(const uint32_t nPortIndex) override;

	void SetData(const uint32_t nPortIndex, [[maybe_unused]] const uint8_t *pData, [[maybe_unused]] uint32_t nLength, const bool doUpdate) override {
		logic_analyzer::ch0_set();

		if (!doUpdate) {
			logic_analyzer::ch0_clear();
			return;
		}

		auto &pixelDmxConfiguration = PixelDmxConfiguration::Get();
		auto &portInfo = pixelDmxConfiguration.GetPortInfo();

		if (nPortIndex == portInfo.nProtocolPortIndexLast) {
			logic_analyzer::ch1_set();

			for (uint32_t nIndex = 0 ; nIndex <= portInfo.nProtocolPortIndexLast;nIndex++) {
				logic_analyzer::ch2_set();
				SetData(nIndex, lightset::Data::Backup(nIndex), lightset::Data::GetLength(nIndex));
				logic_analyzer::ch2_clear();
			}

#if defined (H3)
			logic_analyzer::ch3_set();

			while (m_pWS28xxMulti->IsUpdating()) {
				// wait for completion
			}

			logic_analyzer::ch3_clear();
#endif

			m_pWS28xxMulti->Update();

			logic_analyzer::ch1_clear();
		}

		logic_analyzer::ch0_clear();
	}

	void Sync(const uint32_t nPortIndex) override {
		logic_analyzer::ch2_set();

		SetData(nPortIndex, lightset::Data::Backup(nPortIndex), lightset::Data::GetLength(nPortIndex));

		logic_analyzer::ch2_clear();
	}

	void Sync() override {
		logic_analyzer::ch1_set();
		logic_analyzer::ch3_set();

		while (m_pWS28xxMulti->IsUpdating()) {
			// wait for completion
		}

		logic_analyzer::ch3_clear();

		m_pWS28xxMulti->Update();

		logic_analyzer::ch1_clear();
	}

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle([[maybe_unused]] const uint32_t nPortIndex, [[maybe_unused]] const lightset::OutputStyle outputStyle) override {}
	lightset::OutputStyle GetOutputStyle([[maybe_unused]] const uint32_t nPortIndex) const override {
		return lightset::OutputStyle::DELTA;
	}
#endif

	void Blackout(bool bBlackout) override;
	void FullOn() override;

	void Print() override {
		PixelDmxConfiguration::Get().Print();
	}

	// RDMNet LLRP Device Only
	bool SetDmxStartAddress([[maybe_unused]] uint16_t nDmxStartAddress) override {
		return false;
	}

	uint16_t GetDmxStartAddress() override {
		return lightset::dmx::ADDRESS_INVALID;
	}

	uint16_t GetDmxFootprint() override {
		return 0;
	}

private:
	void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength);

private:
	WS28xxMulti *m_pWS28xxMulti { nullptr };

	uint32_t m_bIsStarted { 0 };
	bool m_bBlackout { false };
};

#endif /* WS28XXDMXMULTI_H_ */
