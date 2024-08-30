/**
 * @file pixelpatterns.h
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

#ifndef PIXELPATTERNS_H_
#define PIXELPATTERNS_H_

#include <cstdint>

#if defined (PIXELPATTERNS_MULTI)
# include "ws28xxmulti.h"
#else
# include "ws28xx.h"
#endif

#include "hardware.h"

namespace pixelpatterns {
#if defined (PIXELPATTERNS_MULTI)
# if !defined (CONFIG_PIXELDMX_MAX_PORTS)
#  define CONFIG_PIXELDMX_MAX_PORTS	8U
# endif
 static constexpr uint32_t MAX_PORTS = CONFIG_PIXELDMX_MAX_PORTS;
#else
 static constexpr uint32_t MAX_PORTS = 1;
#endif

enum class Pattern {
	NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE, LAST
};
enum class Direction {
	FORWARD, REVERSE
};
}  // namespace pixelpatterns

class PixelPatterns {
public:
	PixelPatterns(const uint32_t nActivePorts);
	~PixelPatterns() = default;

	static const char* GetName(pixelpatterns::Pattern pattern);

	void RainbowCycle(uint32_t nPortIndex, uint32_t nInterval, pixelpatterns::Direction dir = pixelpatterns::Direction::FORWARD);
	void TheaterChase(uint32_t nPortIndex, uint32_t nColour1, uint32_t nColour2, uint32_t nInterval, pixelpatterns::Direction dir = pixelpatterns::Direction::FORWARD);
	void ColourWipe(uint32_t nPortIndex, uint32_t nColour, uint32_t nInterval, pixelpatterns::Direction dir = pixelpatterns::Direction::FORWARD);
	void Scanner(uint32_t nPortIndex, uint32_t nColour1, uint32_t nInterval) ;
	void Fade(uint32_t nPortIndex, uint32_t nColour1, uint32_t nColour2, uint32_t nSteps, uint32_t nInterval, pixelpatterns::Direction dir = pixelpatterns::Direction::FORWARD);
	void None(const uint32_t nPortIndex) {
		s_PortConfig[nPortIndex].ActivePattern = pixelpatterns::Pattern::NONE;
		Clear(nPortIndex);
		while (s_pOutput->IsUpdating()) {

		}
		s_pOutput->Update();
	}

	void Run() {
		if (s_pOutput->IsUpdating()) {
			return;
		}

		auto bIsUpdated = false;
		const auto nMillis = Hardware::Get()->Millis();

		for (uint32_t i = 0; i < s_nActivePorts; i++) {
			bIsUpdated |= PortUpdate(i, nMillis);
		}

		if (bIsUpdated) {
			s_pOutput->Update();
		}
	}

	uint32_t Colour(uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
		return static_cast<uint32_t>(nRed << 16) | static_cast<uint32_t>(nGreen << 8) | nBlue;
	}

private:
	void RainbowCycleUpdate(uint32_t nPortIndex);
	void TheaterChaseUpdate(uint32_t nPortIndex);
	void ColourWipeUpdate(uint32_t nPortIndex);
	void ScannerUpdate(uint32_t nPortIndex);
	void FadeUpdate(uint32_t nPortIndex);

	bool PortUpdate(uint32_t nPortIndex, uint32_t nMillis);

	uint32_t Wheel(uint8_t nWheelPos);
	void Increment(uint32_t nPortIndex);
	void Reverse(uint32_t nPortIndex);

	void SetPixelColour([[maybe_unused]] uint32_t nPortIndex, const uint32_t nPixelIndex, const uint32_t nColour) {
		const auto nRed = Red(nColour);
		const auto nGreen = Green(nColour);
		const auto nBlue = Blue(nColour);
#if defined (PIXELPATTERNS_MULTI)
		switch (PixelConfiguration::Get().GetType()) {
		case pixel::Type::WS2801:
			s_pOutput->SetColourWS2801(nPortIndex, nPixelIndex, nRed, nGreen, nBlue);
			break;
		case pixel::Type::APA102:
		case pixel::Type::SK9822:
			s_pOutput->SetPixel4Bytes(nPortIndex, nPixelIndex, 0xFF, nRed, nGreen, nBlue);
			break;
		case pixel::Type::P9813: {
			const auto nFlag = static_cast<uint8_t>(0xC0 | ((~nBlue & 0xC0) >> 2) | ((~nRed & 0xC0) >> 4) | ((~nRed & 0xC0) >> 6));
			s_pOutput->SetPixel4Bytes(nPortIndex, nPixelIndex, nFlag, nBlue, nGreen, nRed);
		}
			break;
		case pixel::Type::SK6812W:
			if ((nRed == nGreen) && (nGreen == nBlue)) {
				s_pOutput->SetColourRTZ(nPortIndex, nPixelIndex, 0x00, 0x00, 0x00, nRed);
			} else {
				s_pOutput->SetColourRTZ(nPortIndex, nPixelIndex, nRed, nGreen, nBlue, 0x00);
			}
			break;
		default:
			s_pOutput->SetColourRTZ(nPortIndex, nPixelIndex, nRed, nGreen, nBlue);
			break;
		}
#else
		auto& pixelConfiguration = PixelConfiguration::Get();
		const auto type = pixelConfiguration.GetType();

		if (type != pixel::Type::SK6812W) {
			s_pOutput->SetPixel(nPixelIndex, nRed, nGreen, nBlue);
		} else {
			if ((nRed == nGreen) && (nGreen == nBlue)) {
				s_pOutput->SetPixel(nPixelIndex, 0x00, 0x00, 0x00, nRed);
			} else {
				s_pOutput->SetPixel(nPixelIndex, nRed, nGreen, nBlue, 0x00);
			}
		}
#endif
	}

	void ColourSet(const uint32_t nPortIndex, const uint32_t nColour) {
		for (uint32_t i = 0; i < PixelConfiguration::Get().GetCount(); i++) {
			SetPixelColour(nPortIndex, i, nColour);
		}
	}

    uint32_t DimColour(const uint32_t nColour) {
        return Colour(static_cast<uint8_t>(Red(nColour) >> 1), static_cast<uint8_t>(Green(nColour) >> 1), static_cast<uint8_t>(Blue(nColour) >> 1));
    }

    uint8_t Red(const uint32_t nColour) {
		return (nColour >> 16) & 0xFF;
	}

	uint8_t Green(const uint32_t nColour) {
		return (nColour >> 8) & 0xFF;
	}

	uint8_t Blue(const uint32_t nColour) {
		return nColour & 0xFF;
	}

	void Clear(const uint32_t nPortIndex) {
		ColourSet(nPortIndex, 0);
	}

private:
#if defined (PIXELPATTERNS_MULTI)
	static WS28xxMulti *s_pOutput;
#else
	static WS28xx *s_pOutput;
#endif
	static uint32_t s_nActivePorts;

	struct PortConfig {
		uint32_t nLastUpdate;
		uint32_t nInterval;
		uint32_t nColour1;
		uint32_t nColour2;
		uint32_t nTotalSteps;
		uint32_t nPixelIndex;
		pixelpatterns::Direction Direction;
		pixelpatterns::Pattern ActivePattern;
	};

	static PortConfig s_PortConfig[pixelpatterns::MAX_PORTS];

	static uint32_t *s_pScannerColours;
};

#endif /* PIXELPATTERNS_H_ */
