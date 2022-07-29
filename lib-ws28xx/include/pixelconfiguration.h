/**
 * @file pixelconfiguration.h
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

#ifndef PIXELCONFIGURATION_H_
#define PIXELCONFIGURATION_H_

#include <cstdint>

#include "pixeltype.h"

class PixelConfiguration {
public:
	void SetType(pixel::Type Type) {
		m_Type = Type;
	}

	pixel::Type GetType() const {
		return m_Type;
	}

	void SetCount(uint32_t nCount) {
		m_nCount = nCount == 0 ? pixel::defaults::COUNT : nCount;
	}

	uint32_t GetCount() const {
		return m_nCount;
	}

	void SetMap(pixel::Map tMap) {
		m_tRGBMapping = tMap;
	}

	pixel::Map GetMap() const {
		return m_tRGBMapping;
	}

	void SetLowCode(uint8_t nLowCode) {
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

	void SetClockSpeedHz(uint32_t nClockSpeedHz)  {
		m_nClockSpeedHz = nClockSpeedHz;
	}

	uint32_t GetClockSpeedHz() const {
		return m_nClockSpeedHz;
	}

	void SetGlobalBrightness(uint8_t nGlobalBrightness) {
		m_nGlobalBrightness = nGlobalBrightness;
	}

	uint8_t GetGlobalBrightness() const {
		return m_nGlobalBrightness;
	}

	bool IsRTZProtocol() const {
		return m_bIsRTZProtocol;
	}

	void Validate(uint32_t& nLedsPerPixel);

	void Dump();

	static pixel::Map GetRgbMapping(pixel::Type tType);
	static void GetTxH(pixel::Type tType, uint8_t &nLowCode, uint8_t &nHighCode);

private:
	pixel::Type m_Type { pixel::defaults::TYPE };
	uint32_t m_nCount { pixel::defaults::COUNT };
	pixel::Map m_tRGBMapping { pixel::Map::UNDEFINED };
	uint32_t m_nClockSpeedHz { 0 };
	uint8_t m_nLowCode { 0 };
	uint8_t m_nHighCode { 0 };
	uint8_t m_nGlobalBrightness { 0xFF };
	// Calculated
	bool m_bIsRTZProtocol { true };
};

#endif /* PIXELCONFIGURATION_H_ */
