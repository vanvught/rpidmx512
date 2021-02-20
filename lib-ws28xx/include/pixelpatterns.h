/**
 * @file pixelpatterns.h
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

#ifndef PIXELPATTERNS_H_
#define PIXELPATTERNS_H_

#include <stdint.h>

#if defined (OUTPUT_PIXEL_MULTI)
# include "ws28xxmulti.h"
#else
# include "ws28xx.h"
#endif

#include "hardware.h"

namespace pixelpatterns {
#if defined (OUTPUT_PIXEL_MULTI)
static constexpr uint32_t MAX_PORTS = 8;
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
	PixelPatterns(uint32_t nActivePorts);
	~PixelPatterns() {}

	void RainbowCycle(uint8_t nPort, uint32_t nInterval, pixelpatterns::Direction dir = pixelpatterns::Direction::FORWARD);
	void TheaterChase(uint8_t nPort, uint32_t nColour1, uint32_t nColour2, uint8_t nInterval, pixelpatterns::Direction dir = pixelpatterns::Direction::FORWARD);
	void ColourWipe(uint8_t nPort, uint32_t nColour, uint32_t nInterval, pixelpatterns::Direction dir = pixelpatterns::Direction::FORWARD);
	void Scanner(uint8_t nPort, uint32_t nColour1, uint32_t nInterval) ;
	void Fade(uint8_t nPort, uint32_t nColour1, uint32_t nColour2, uint32_t nSteps, uint32_t nInterval, pixelpatterns::Direction dir = pixelpatterns::Direction::FORWARD);

	void Run();

	uint32_t Colour(uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
		return static_cast<uint32_t>(nRed << 16) | static_cast<uint32_t>(nGreen << 8) | nBlue;
	}

private:
	void RainbowCycleUpdate(uint8_t nPort);
	void TheaterChaseUpdate(uint8_t nPort);
	void ColourWipeUpdate(uint8_t nPort);
	void ScannerUpdate(uint8_t nPort);
	void FadeUpdate(uint8_t nPort);

	bool PortUpdate(uint32_t nPort, uint32_t nMillis);

	uint32_t Wheel(uint8_t nWheelPos);
	void Increment(uint8_t nPort);
	void Reverse(uint8_t nPort);

	void SetPixelColour(__attribute__((unused)) uint8_t nPort, uint32_t nIndex, uint32_t nColour) {
#if defined (OUTPUT_PIXEL_MULTI)
		m_pOutput->SetLED(nPort, nIndex, Red(nColour), Green(nColour), Blue(nColour));
#else
		m_pOutput->SetLED(nIndex, Red(nColour), Green(nColour), Blue(nColour));
#endif
	}

	void ColourSet(uint8_t nPort, uint32_t nColour) {
		for (uint32_t i = 0; i < m_nLedCount; i++) {
			SetPixelColour(nPort, i, nColour);
		}
	}

    uint32_t DimColour(uint32_t nColour) {
        return Colour(Red(nColour) >> 1, Green(nColour) >> 1, Blue(nColour) >> 1);
    }

    uint8_t Red(uint32_t nColour) {
		return (nColour >> 16) & 0xFF;
	}

	uint8_t Green(uint32_t nColour) {
		return (nColour >> 8) & 0xFF;
	}

	uint8_t Blue(uint32_t nColour) {
		return nColour & 0xFF;
	}

private:
#if defined (OUTPUT_PIXEL_MULTI)
	WS28xxMulti *m_pOutput { nullptr };
#else
	WS28xx *m_pOutput { nullptr };
#endif
	uint32_t m_nActivePorts { pixelpatterns::MAX_PORTS };
	uint32_t m_nLedCount { 0 };

	struct PortConfig {
		uint32_t nLastUpdate;
		uint32_t nInterval;
		uint32_t nColour1;
		uint32_t nColour2;
		uint32_t nTotalSteps;
		uint32_t nIndex;
		pixelpatterns::Direction Direction;
		pixelpatterns::Pattern ActivePattern;
	};

	PortConfig m_PortConfig[pixelpatterns::MAX_PORTS];

	uint32_t *m_pScannerColours { nullptr };
};

#endif /* PIXELPATTERNS_H_ */
