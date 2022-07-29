/**
 * @file pixelpatterns.h
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
	PixelPatterns(uint32_t nActivePorts);
	~PixelPatterns() {}

	static const char* GetName(pixelpatterns::Pattern pattern);

	void RainbowCycle(uint32_t nPortIndex, uint32_t nInterval, pixelpatterns::Direction dir = pixelpatterns::Direction::FORWARD);
	void TheaterChase(uint32_t nPortIndex, uint32_t nColour1, uint32_t nColour2, uint32_t nInterval, pixelpatterns::Direction dir = pixelpatterns::Direction::FORWARD);
	void ColourWipe(uint32_t nPortIndex, uint32_t nColour, uint32_t nInterval, pixelpatterns::Direction dir = pixelpatterns::Direction::FORWARD);
	void Scanner(uint32_t nPortIndex, uint32_t nColour1, uint32_t nInterval) ;
	void Fade(uint32_t nPortIndex, uint32_t nColour1, uint32_t nColour2, uint32_t nSteps, uint32_t nInterval, pixelpatterns::Direction dir = pixelpatterns::Direction::FORWARD);
	void None(uint32_t nPortIndex) {
		m_PortConfig[nPortIndex].ActivePattern = pixelpatterns::Pattern::NONE;
		Clear(nPortIndex);
		while (m_pOutput->IsUpdating()) {

		}
		m_pOutput->Update();
	}

	void Run();

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

	void SetPixelColour(__attribute__((unused)) uint32_t nPortIndex, uint32_t nPixelIndex, uint32_t nColour) {
		const auto nRed = Red(nColour);
		const auto nGreen = Green(nColour);
		const auto nBlue = Blue(nColour);
#if defined (PIXELPATTERNS_MULTI)
		if (m_pOutput->GetType() != pixel::Type::SK6812W) {
			m_pOutput->SetPixel(nPortIndex, nPixelIndex, nRed, nGreen, nBlue);
		} else {
			if ((nRed == nGreen) && (nGreen == nBlue)) {
				m_pOutput->SetPixel(nPortIndex, nPixelIndex, 0x00, 0x00, 0x00, nRed);
			} else {
				m_pOutput->SetPixel(nPortIndex, nPixelIndex, nRed, nGreen, nBlue, 0x00);
			}
		}
#else
		if (m_pOutput->GetType() != pixel::Type::SK6812W) {
			m_pOutput->SetPixel(nPixelIndex, nRed, nGreen, nBlue);
		} else {
			if ((nRed == nGreen) && (nGreen == nBlue)) {
				m_pOutput->SetPixel(nPixelIndex, 0x00, 0x00, 0x00, nRed);
			} else {
				m_pOutput->SetPixel(nPixelIndex, nRed, nGreen, nBlue, 0x00);
			}
		}
#endif
	}

	void ColourSet(uint32_t nPortIndex, uint32_t nColour) {
		for (uint32_t i = 0; i < m_nCount; i++) {
			SetPixelColour(nPortIndex, i, nColour);
		}
	}

    uint32_t DimColour(uint32_t nColour) {
        return Colour(static_cast<uint8_t>(Red(nColour) >> 1), static_cast<uint8_t>(Green(nColour) >> 1), static_cast<uint8_t>(Blue(nColour) >> 1));
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

	void Clear(uint32_t nPortIndex) {
		ColourSet(nPortIndex, 0);
	}

private:
#if defined (PIXELPATTERNS_MULTI)
	static WS28xxMulti *m_pOutput;
#else
	static WS28xx *m_pOutput;
#endif
	static uint32_t m_nActivePorts;
	static uint32_t m_nCount;

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

	static PortConfig m_PortConfig[pixelpatterns::MAX_PORTS];

	static uint32_t *m_pScannerColours;
};

#endif /* PIXELPATTERNS_H_ */
