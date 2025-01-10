/**
 * @file ws28xxdmxmulti.h
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

#ifndef WS28XXDMXMULTI_H_
#define WS28XXDMXMULTI_H_

#if defined (DEBUG_PIXELDMX)
# if defined (NDEBUG)
#  undef NDEBUG
#  define _NDEBUG
# endif
#endif

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
# pragma GCC optimize ("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <algorithm>
#include <cassert>

#include "lightset.h"
#include "lightsetdata.h"

#include "ws28xxmulti.h"

#include "pixeldmxconfiguration.h"

#if defined (PIXELDMXSTARTSTOP_GPIO)
# include "hal_gpio.h"
#endif

#include "logic_analyzer.h"
#include "debug.h"

namespace ws28xxdmxmulti {
#if !defined (CONFIG_PIXELDMX_MAX_PORTS)
# error
#endif
static constexpr auto MAX_PORTS = CONFIG_PIXELDMX_MAX_PORTS;
}  // namespace ws28xxdmxmulti

class WS28xxDmxMulti final: public LightSet {
public:
	WS28xxDmxMulti();
	~WS28xxDmxMulti() override;

	void Start(const uint32_t nPortIndex) override {
		const auto nIndex = (nPortIndex <= 31) ? 0 : 1;
		DEBUG_PRINTF("%u [%u]", nPortIndex, nIndex);

		if ((m_bIsStarted[0] && m_bIsStarted[1]) == 0) {
#if defined (PIXELDMXSTARTSTOP_GPIO)
			FUNC_PREFIX(gpio_set(PIXELDMXSTARTSTOP_GPIO));
#endif
		}

		if (nIndex == 0) {
			m_bIsStarted[0] |= (1U << nPortIndex);
		} else {
			m_bIsStarted[1] |= (1U << (nPortIndex - 32));
		}
	}

	void Stop(const uint32_t nPortIndex) override {
		const auto nIndex = (nPortIndex <= 31) ? 0 : 1;
		DEBUG_PRINTF("%u [%u]", nPortIndex, nIndex);

		if (nIndex == 0) {
			if (m_bIsStarted[0] & (1U << nPortIndex)) {
				m_bIsStarted[0] &= ~(1U << nPortIndex);
			}
		} else {
			if (m_bIsStarted[1] & (1U << (nPortIndex - 32))) {
				m_bIsStarted[1] &= ~(1U << (nPortIndex - 32));
			}
		}

		if ((m_bIsStarted[0] && m_bIsStarted[1]) == 0) {
#if defined (PIXELDMXSTARTSTOP_GPIO)
			FUNC_PREFIX(gpio_clr(PIXELDMXSTARTSTOP_GPIO));
#endif
		}
	}

	inline void SetData(const uint32_t nPortIndex, const uint8_t *pData, const uint32_t nLength, const bool doUpdate) override {
		logic_analyzer::ch0_set();

		SetData(nPortIndex, pData, nLength);

		auto &pixelDmxConfiguration = PixelDmxConfiguration::Get();
		auto &portInfo = pixelDmxConfiguration.GetPortInfo();

		if ((nPortIndex == portInfo.nProtocolPortIndexLast) && doUpdate) {
			logic_analyzer::ch1_set();

			for (uint32_t nIndex = 0 ; nIndex <= portInfo.nProtocolPortIndexLast;nIndex++) {
				logic_analyzer::ch2_set();
				SetData(nIndex, lightset::Data::Backup(nIndex), lightset::Data::GetLength(nIndex));
				logic_analyzer::ch2_clear();
			}

			m_pWS28xxMulti->Update();

			logic_analyzer::ch1_clear();
		}

		logic_analyzer::ch0_clear();
	}

	inline void Sync([[maybe_unused]] const uint32_t nPortIndex) override {
		logic_analyzer::ch2_set();

		m_bNeedSync = true;

		logic_analyzer::ch2_clear();
	}

	inline void Sync() override {
		if (!m_bNeedSync) {
			return;
		}

		logic_analyzer::ch1_set();

		m_pWS28xxMulti->Update();

		m_bNeedSync = false;

		logic_analyzer::ch1_clear();
	}

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle([[maybe_unused]] const uint32_t nPortIndex, [[maybe_unused]] const lightset::OutputStyle outputStyle) override {}
	lightset::OutputStyle GetOutputStyle([[maybe_unused]] const uint32_t nPortIndex) const override {
		return lightset::OutputStyle::DELTA;
	}
#endif


	void Blackout(const bool bBlackout) override {
		m_bBlackout = bBlackout;

		while (m_pWS28xxMulti->IsUpdating()) {
			// wait for completion
		}

		if (bBlackout) {
			m_pWS28xxMulti->Blackout();
		} else {
			m_pWS28xxMulti->Update();
		}
	}

	void FullOn() override {
		while (m_pWS28xxMulti->IsUpdating()) {
			// wait for completion
		}

		m_pWS28xxMulti->FullOn();
	}

	void Print() override {
		PixelDmxConfiguration::Get().Print();
	}

	// Optional
	inline uint32_t GetUserData() override {
		return m_pWS28xxMulti->GetUserData();
	}

	inline uint32_t GetRefreshRate() override {
		auto& pixelConfiguration = PixelConfiguration::Get();
		return pixelConfiguration.GetRefreshRate();
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
	void SetData(const uint32_t nPortIndex, const uint8_t* pData, const uint32_t nLength) {
		assert(pData != nullptr);
		assert(nLength <= lightset::dmx::UNIVERSE_SIZE);

		auto &pixelDmxConfiguration = PixelDmxConfiguration::Get();

#if defined (NODE_DDP_DISPLAY)
		const auto nOutIndex = (nPortIndex / 4);
		const auto nSwitch = nPortIndex - (nOutIndex * 4);
#else
		const auto nUniverses = pixelDmxConfiguration.GetUniverses();
		const auto nOutIndex = (nPortIndex / nUniverses);
		const auto nSwitch = nPortIndex - (nOutIndex * nUniverses);
#endif
		auto &portInfo = pixelDmxConfiguration.GetPortInfo();

		const auto nGroups = pixelDmxConfiguration.GetGroups();
		const auto beginIndex = portInfo.nBeginIndexPort[nSwitch];
		const auto nChannelsPerPixel = pixelDmxConfiguration.GetLedsPerPixel();
		const auto endIndex = std::min(nGroups, (beginIndex + (nLength / nChannelsPerPixel)));
		const auto nGroupingCount = pixelDmxConfiguration.GetGroupingCount();
		const auto pixelType = pixelDmxConfiguration.GetType();
		const auto isRTZProtocol = pixelDmxConfiguration.IsRTZProtocol();

		uint32_t d = 0;

		if (nChannelsPerPixel == 3) {
			// Define a lambda to handle pixel setting based on color order
			auto setPixelsColourRTZ = [&](const uint32_t nPortIndex, const uint32_t nPixelIndex, const uint32_t r, const uint32_t g, const uint32_t b) {
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
				const auto pGammaTable = m_pWS28xxMulti->m_PixelConfiguration.GetGammaTable();
				r = pGammaTable[r];
				g = pGammaTable[g];
				b = pGammaTable[b];
#endif
				m_pWS28xxMulti->SetColourRTZ(nPortIndex, nPixelIndex, r, g, b);
			};

			// Define a lambda to handle pixel setting based on color order
			auto setPixelsColour3 = [&](const uint32_t nPortIndex, const uint32_t nPixelIndex, const uint32_t r, const uint32_t g, const uint32_t b) {
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
				const auto pGammaTable = m_pWS28xxMulti->m_PixelConfiguration.GetGammaTable();
				r = pGammaTable[r];
				g = pGammaTable[g];
				b = pGammaTable[b];
#endif

				switch (pixelType) {
				case pixel::Type::WS2801:
					m_pWS28xxMulti->SetColourWS2801(nPortIndex, nPixelIndex, r, g, b);
					break;
				case pixel::Type::APA102:
				case pixel::Type::SK9822:
					m_pWS28xxMulti->SetPixel4Bytes(nPortIndex, 1 + nPixelIndex, pixelDmxConfiguration.GetGlobalBrightness(), b, g, r);
					break;
				case pixel::Type::P9813: {
					const auto nFlag = static_cast<uint8_t>(0xC0 | ((~b & 0xC0) >> 2) | ((~r & 0xC0) >> 4) | ((~r & 0xC0) >> 6));
					m_pWS28xxMulti->SetPixel4Bytes(nPortIndex, 1 + nPixelIndex, nFlag, b, g, r);
				}
				break;
				default:
					assert(0);
					__builtin_unreachable();
					break;
				}
			};

			constexpr uint32_t channelMap[6][3] = {
					{0, 1, 2}, // RGB
					{0, 2, 1}, // RBG
					{1, 0, 2}, // GRB
					{2, 0, 1}, // GBR
					{1, 2, 0}, // BRG
					{2, 1, 0}  // BGR
			};

			const auto mapIndex = static_cast<uint32_t>(pixelDmxConfiguration.GetMap());
			// Ensure mapIndex is within valid bounds
			assert(mapIndex < sizeof(channelMap) / sizeof(channelMap[0]));  // Runtime check
			auto const& map = channelMap[mapIndex];

			if (isRTZProtocol) {
				for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
					auto const nPixelIndexStart = j * nGroupingCount;
					for (uint32_t k = 0; k < nGroupingCount; k++) {
						setPixelsColourRTZ(nOutIndex, nPixelIndexStart + k, pData[d + map[0]], pData[d + map[1]], pData[d + map[2]]);
					}
					d += 3; // Increment by 3 since we're processing 3 channels per pixel
				}
			} else {
				for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
					auto const nPixelIndexStart = j * nGroupingCount;
					for (uint32_t k = 0; k < nGroupingCount; k++) {
						setPixelsColour3(nOutIndex, nPixelIndexStart + k, pData[d + map[0]], pData[d + map[1]], pData[d + map[2]]);
					}
					d += 3; // Increment by 3 since we're processing 3 channels per pixel
				}
			}
		} else {
			assert(nChannelsPerPixel == 4);
			assert(isRTZProtocol);
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xxMulti->SetColourRTZ(nOutIndex, nPixelIndexStart + k, pData[d], pData[d + 1], pData[d + 2], pData[d + 3]);
				}
				d = d + 4; // Increment by 4 since we're processing 4 channels per pixel
			}
		}
	}

private:
	WS28xxMulti *m_pWS28xxMulti { nullptr };

	uint32_t m_bIsStarted[2];		///< Support for 16x4 = 64 ports.
	bool m_bBlackout { false };
	bool m_bNeedSync { false };
};

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC pop_options
#endif
#if defined (_NDEBUG)
# undef _NDEBUG
# define NDEBUG
#endif
#endif /* WS28XXDMXMULTI_H_ */
