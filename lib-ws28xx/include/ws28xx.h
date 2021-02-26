/**
 * @file ws28xx.h
 *
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef WS28XX_H_
#define WS28XX_H_

#include <stdint.h>

#include "rgbmapping.h"

namespace ws28xx {
enum class Type {
	WS2801 = 0,
	WS2811,
	WS2812,
	WS2812B,
	WS2813,
	WS2815,
	SK6812,
	SK6812W,
	APA102,
	UCS1903,
	UCS2903,
	P9813,
	CS8812,
	UNDEFINED
};
namespace max {
namespace ledcount {
static constexpr auto RGB = (4 * 170);
static constexpr auto RGBW = (4 * 128);
}  // namespace ledcount
}  // namespace max
namespace single {
static constexpr auto RGB = 24;
static constexpr auto RGBW = 32;
}  // namespace single
namespace spi {
namespace speed {
namespace ws2801 {
static constexpr uint32_t max_hz = 25000000;	///< 25 MHz
static constexpr uint32_t default_hz = 4000000;	///< 4 MHz
}  // namespace ws2801
namespace p9813 {
static constexpr uint32_t max_hz = 15000000;	///< 15 MHz
static constexpr uint32_t default_hz = 4000000;	///< 4 MHz
}  // namespace p9813
}  // namespace speed
}  // namespace spi
namespace defaults {
static constexpr auto TYPE = Type::WS2812B;
static constexpr auto LED_COUNT = 170;
static constexpr auto ACTIVE_OUTPUTS = 1;
}  // namespace defaults
}  // namespace ws28xx

class WS28xx {
public:
	WS28xx(ws28xx::Type Type, uint16_t nLedCount, rgbmapping::Map tRGBMapping = rgbmapping::Map::UNDEFINED, uint8_t nT0H = 0, uint8_t nT1H = 0, uint32_t nClockSpeed = ws28xx::spi::speed::ws2801::default_hz);
	~WS28xx();

	bool Initialize ();

	ws28xx::Type GetLEDType() const {
		return m_tLEDType;
	}

	rgbmapping::Map GetRgbMapping() const {
		return m_tRGBMapping;
	}

	uint8_t GetLowCode() const {
		return m_nLowCode;
	}

	uint8_t GetHighCode() const {
		return m_nHighCode;
	}

	uint16_t GetLEDCount() const {
		return m_nLedCount;
	}

	uint32_t GetClockSpeedHz() const {
		return m_nClockSpeedHz;
	}

	void SetGlobalBrightness(uint8_t nGlobalBrightness);

	uint8_t GetGlobalBrightness() const {
		return m_nGlobalBrightness;
	}

	void SetLED(uint32_t nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void SetLED(uint32_t nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite);

	void Update();
	void Blackout();

	bool IsUpdating() const {
		return false;
	}

	static const char *GetLedTypeString(ws28xx::Type tType);
	static ws28xx::Type GetLedTypeString(const char *pValue);
	static void GetTxH(ws28xx::Type tType, uint8_t &nLowCode, uint8_t &nHighCode);
	static rgbmapping::Map GetRgbMapping(ws28xx::Type tType);
	static float ConvertTxH(uint8_t nCode);
	static uint8_t ConvertTxH(float fTxH);

	static WS28xx *Get() {
		return s_pThis;
	}

private:
	void SetColorWS28xx(uint32_t nOffset, uint8_t nValue);

protected:
	ws28xx::Type m_tLEDType { ws28xx::defaults::TYPE };
	uint16_t m_nLedCount { ws28xx::defaults::LED_COUNT };
	rgbmapping::Map m_tRGBMapping { rgbmapping::Map::UNDEFINED };
	uint32_t m_nClockSpeedHz;
	uint32_t m_nBufSize;
	uint8_t m_nLowCode;
	uint8_t m_nHighCode;
	bool m_bIsRTZProtocol { false };
	uint8_t m_nGlobalBrightness { 0xFF };
	uint8_t *m_pBuffer { nullptr };
	uint8_t *m_pBlackoutBuffer { nullptr };

	static WS28xx *s_pThis;
};

#endif /* WS28XX_H_ */
