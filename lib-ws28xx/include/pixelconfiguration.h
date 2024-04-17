/**
 * @file pixelconfiguration.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PIXELCONFIGURATION_H_
#define PIXELCONFIGURATION_H_

#include <cstdint>

#include "pixeltype.h"
#if defined (CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
# include "gamma/gamma_tables.h"
#endif

#include "debug.h"

class PixelConfiguration {
public:
	void SetType(pixel::Type Type) {
		m_type = Type;
	}

	pixel::Type GetType() const {
		return m_type;
	}

	void SetCount(const uint16_t nCount) {
		m_nCount = (nCount == 0 ? pixel::defaults::COUNT : nCount);
	}

	uint16_t GetCount() const {
		return m_nCount;
	}

	void SetMap(const pixel::Map tMap) {
		m_map = tMap;
	}

	pixel::Map GetMap() const {
		return m_map;
	}

	void SetLowCode(const uint8_t nLowCode) {
		m_nLowCode = nLowCode;
	}

	uint8_t GetLowCode() const {
		return m_nLowCode;
	}

	void SetHighCode(uint8_t nHighCode) {
		m_nHighCode = nHighCode;
	}

	uint8_t GetHighCode() const {
		return m_nHighCode;
	}

	void SetClockSpeedHz(const uint32_t nClockSpeedHz)  {
		m_nClockSpeedHz = nClockSpeedHz;
	}

	uint32_t GetClockSpeedHz() const {
		return m_nClockSpeedHz;
	}

	void SetGlobalBrightness(const uint8_t nGlobalBrightness) {
		m_nGlobalBrightness = nGlobalBrightness;
	}

	uint8_t GetGlobalBrightness() const {
		return m_nGlobalBrightness;
	}

	bool IsRTZProtocol() const {
		return m_bIsRTZProtocol;
	}

#if defined (CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
	void SetEnableGammaCorrection(const bool doEnable) {
		m_bEnableGammaCorrection = doEnable;
	}

	bool IsEnableGammaCorrection() const {
		return m_bEnableGammaCorrection;
	}

	void SetGammaTable(const uint32_t nValue) {
		m_nGammaValue = static_cast<uint8_t>(nValue);
	}

	const uint8_t *GetGammaTable() const {
		return m_pGammaTable;
	}
#endif

	void Validate(uint16_t& nLedsPerPixel);

	void Print();

	static void GetTxH(pixel::Type tType, uint8_t &nLowCode, uint8_t &nHighCode);

private:
	pixel::Type m_type { pixel::defaults::TYPE };
	uint16_t m_nCount { pixel::defaults::COUNT };
	pixel::Map m_map { pixel::Map::UNDEFINED };
	uint32_t m_nClockSpeedHz { 0 };
	uint8_t m_nLowCode { 0 };
	uint8_t m_nHighCode { 0 };
	uint8_t m_nGlobalBrightness { 0xFF };
	uint8_t m_nGammaValue { 0 };
	bool m_bIsRTZProtocol { true };
#if defined (CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
	bool m_bEnableGammaCorrection { false };
	const uint8_t *m_pGammaTable { gamma10_0 };
#endif
};

#endif /* PIXELCONFIGURATION_H_ */
