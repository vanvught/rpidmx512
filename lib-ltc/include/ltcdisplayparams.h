/**
 * @file ltcdisplayparams.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTCDISPLAYPARAMS_H_
#define LTCDISPLAYPARAMS_H_

#include <cstdint>

#include "ltcdisplaymax7219.h"
#include "ltcdisplayrgb.h"
#include "configstore.h"

namespace ltcdisplayparams {
struct Params {
	uint32_t nSetList;					//  4
	uint8_t nMax7219Type;				//  5
	uint8_t nMax7219Intensity;			//  6
	uint8_t nWS28xxType;				//  7
	uint8_t nGlobalBrightness;			//  8	// Not used
	uint8_t nWS28xxRgbMapping;			//  9
	uint8_t nDisplayRgbIntensity;		// 10
	uint8_t nDisplayRgbColonBlinkMode;	// 11
	uint32_t aDisplayRgbColour[static_cast<uint32_t>(ltcdisplayrgb::ColourIndex::LAST)]; // 35	6 * 4 = 24
	uint8_t nWS28xxDisplayType;			// 36
	char aInfoMessage[8];				// 44
	uint8_t nOledIntensity;				// 45
	uint8_t nRotaryFullStep;			// 46
} __attribute__((packed));

static_assert(sizeof(struct ltcdisplayparams::Params) <= 64, "struct ltcdisplayparams::Params is too large");

struct Mask {
	static constexpr auto MAX7219_TYPE = (1U << 0);
	static constexpr auto MAX7219_INTENSITY = (1U << 1);
	static constexpr auto WS28XX_TYPE = (1U << 2);
	static constexpr auto GLOBAL_BRIGHTNESS = (1U << 3);		// Not used
	static constexpr auto WS28XX_RGB_MAPPING = (1U << 4);
	static constexpr auto DISPLAYRGB_INTENSITY = (1U << 5);
	static constexpr auto DISPLAYRGB_COLON_BLINK_MODE = (1U << 6);
	static constexpr auto WS28XX_DISPLAY_TYPE = (1U << 7);
	static constexpr auto INFO_MSG = (1U << 8);
	static constexpr auto OLED_INTENSITY = (1U << 9);
	static constexpr auto ROTARY_FULLSTEP = (1U << 10);
	static constexpr auto DISLAYRGB_COLOUR_INDEX = (1U << 11);	// This must be the last one
};
}  // namespace ltcdisplayparams

class LtcDisplayParamsStore {
public:
	static void Update(const struct ltcdisplayparams::Params *ptLtcDisplayParams) {
		ConfigStore::Get()->Update(configstore::Store::LTCDISPLAY, ptLtcDisplayParams, sizeof(struct ltcdisplayparams::Params));
	}

	static void Copy(struct ltcdisplayparams::Params *ptLtcDisplayParams) {
		ConfigStore::Get()->Copy(configstore::Store::LTCDISPLAY, ptLtcDisplayParams, sizeof(struct ltcdisplayparams::Params));
	}
};

class LtcDisplayParams {
public:
	LtcDisplayParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct ltcdisplayparams::Params *ptLtcDisplayParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set(LtcDisplayRgb *pLtcDisplayWS28xx);

	ltc::display::max7219::Types GetMax7219Type() const {
		return static_cast<ltc::display::max7219::Types>(m_Params.nMax7219Type);
	}

	uint8_t GetMax7219Intensity() const {
		return m_Params.nMax7219Intensity;
	}

#if !defined (CONFIG_LTC_DISABLE_WS28XX)
	pixel::Type GetWS28xxLedType() const {
		return static_cast<pixel::Type>(m_Params.nWS28xxType);
	}
#endif

	ltcdisplayrgb::WS28xxType GetWS28xxDisplayType() const {
		return static_cast<ltcdisplayrgb::WS28xxType>(m_Params.nWS28xxDisplayType);
	}

	const char *GetInfoMessage(uint32_t& nLength) const {
		nLength = sizeof(m_Params.aInfoMessage);
		return m_Params.aInfoMessage;
	}

	uint8_t GetOledIntensity() const {
		return m_Params.nOledIntensity;
	}

	bool IsRotaryFullStep() const {
		return m_Params.nRotaryFullStep != 0;
	}

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *pLine);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
	ltcdisplayparams::Params m_Params;
};

#endif /* LTCDISPLAYPARAMS_H_ */
