/**
 * @file pixelpatterns.cpp
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
/**
 * Based on https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 */

#include <algorithm>
#include <cassert>

#include "pixelpatterns.h"

#if defined (PIXELPATTERNS_MULTI)
# include "ws28xxmulti.h"
#else
# include "ws28xx.h"
#endif

#include "hardware.h"

#include "debug.h"

using namespace pixelpatterns;

static constexpr char s_patternName[static_cast<uint32_t>(Pattern::LAST)][14] = { "None", "Rainbow cycle", "Theater chase", "Colour wipe", "Scanner", "Fade" };

#if defined (PIXELPATTERNS_MULTI)
WS28xxMulti *PixelPatterns::s_pOutput;
#else
WS28xx *PixelPatterns::s_pOutput;
#endif
uint32_t PixelPatterns::s_nActivePorts;
PixelPatterns::PortConfig PixelPatterns::s_PortConfig[pixelpatterns::MAX_PORTS];
uint32_t *PixelPatterns::s_pScannerColours;

PixelPatterns::PixelPatterns(const uint32_t nActivePorts) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nActivePorts=%u", nActivePorts);

#if defined (PIXELPATTERNS_MULTI)
	s_pOutput = WS28xxMulti::Get();
#else
	s_pOutput = WS28xx::Get();
#endif
	assert(s_pOutput != nullptr);

	s_nActivePorts = std::min(MAX_PORTS, nActivePorts);

	const auto nMillis = Hardware::Get()->Millis();

	for (uint32_t i = 0; i < MAX_PORTS; i++) {
		s_PortConfig[i].ActivePattern = Pattern::NONE;
		s_PortConfig[i].nLastUpdate = nMillis;
		s_PortConfig[i].Direction = Direction::FORWARD;
	}

	DEBUG_EXIT
}

const char* PixelPatterns::GetName(Pattern pattern) {
	if (pattern < Pattern::LAST) {
		return s_patternName[static_cast<uint32_t>(pattern)];
	}

	return "Unknown";
}

bool PixelPatterns::PortUpdate(uint32_t nPortIndex, uint32_t nMillis) {
	if ((nMillis - s_PortConfig[nPortIndex].nLastUpdate) < s_PortConfig[nPortIndex].nInterval) {
		return false;
	}

	s_PortConfig[nPortIndex].nLastUpdate = nMillis;

	switch (s_PortConfig[nPortIndex].ActivePattern) {
	case Pattern::RAINBOW_CYCLE:
		RainbowCycleUpdate(nPortIndex);
		break;
	case Pattern::THEATER_CHASE:
		TheaterChaseUpdate(nPortIndex);
		break;
	case Pattern::COLOR_WIPE:
		ColourWipeUpdate(nPortIndex);
		break;
	case Pattern::SCANNER:
		ScannerUpdate(nPortIndex);
		break;
	case Pattern::FADE:
		FadeUpdate(nPortIndex);
		break;
	default:
		return false;
		break;
	}

	return true;
}

void PixelPatterns::RainbowCycle(uint32_t nPortIndex, uint32_t nInterval, pixelpatterns::Direction Direction) {
	Clear(nPortIndex);

	s_PortConfig[nPortIndex].ActivePattern = Pattern::RAINBOW_CYCLE;
	s_PortConfig[nPortIndex].nInterval = nInterval;
	s_PortConfig[nPortIndex].nTotalSteps= 255;
	s_PortConfig[nPortIndex].nPixelIndex = 0;
	s_PortConfig[nPortIndex].Direction = Direction;
}

void PixelPatterns::RainbowCycleUpdate(uint32_t nPortIndex) {
	const auto nIndex = s_PortConfig[nPortIndex].nPixelIndex;

	for (uint32_t i = 0; i < PixelConfiguration::Get().GetCount(); i++) {
		SetPixelColour(nPortIndex, i, Wheel(((i * 256U / PixelConfiguration::Get().GetCount()) + nIndex) & 0xFF));
	}

	Increment(nPortIndex);
}

void PixelPatterns::TheaterChase(uint32_t nPortIndex, uint32_t nColour1, uint32_t nColour2, uint32_t nInterval, pixelpatterns::Direction Direction){
	Clear(nPortIndex);

	s_PortConfig[nPortIndex].ActivePattern = Pattern::THEATER_CHASE;
	s_PortConfig[nPortIndex].nInterval = nInterval;
	s_PortConfig[nPortIndex].nTotalSteps= PixelConfiguration::Get().GetCount();
	s_PortConfig[nPortIndex].nColour1 = nColour1;
	s_PortConfig[nPortIndex].nColour2 = nColour2;
    s_PortConfig[nPortIndex].nPixelIndex = 0;
    s_PortConfig[nPortIndex].Direction = Direction;
}

void PixelPatterns::TheaterChaseUpdate(uint32_t nPortIndex) {
	const auto Colour1 = s_PortConfig[nPortIndex].nColour1;
	const auto Colour2 = s_PortConfig[nPortIndex].nColour2;
	const auto Index = s_PortConfig[nPortIndex].nPixelIndex;

	for (uint32_t i = 0; i < PixelConfiguration::Get().GetCount(); i++) {
		if ((i + Index) % 3 == 0) {
			SetPixelColour(nPortIndex, i, Colour1);
		} else {
			SetPixelColour(nPortIndex, i, Colour2);
		}
	}

	Increment(nPortIndex);
}

void PixelPatterns::ColourWipe(uint32_t nPortIndex, uint32_t nColour, uint32_t nInterval, pixelpatterns::Direction Direction) {
	Clear(nPortIndex);

	s_PortConfig[nPortIndex].ActivePattern = Pattern::COLOR_WIPE;
	s_PortConfig[nPortIndex].nInterval = nInterval;
	s_PortConfig[nPortIndex].nTotalSteps= PixelConfiguration::Get().GetCount();
    s_PortConfig[nPortIndex].nColour1 = nColour;
    s_PortConfig[nPortIndex].nPixelIndex = 0;
    s_PortConfig[nPortIndex].Direction = Direction;
}

void PixelPatterns::ColourWipeUpdate(uint32_t nPortIndex) {
	const auto nColour1 = s_PortConfig[nPortIndex].nColour1;
	const auto nIndex = s_PortConfig[nPortIndex].nPixelIndex;

	SetPixelColour(nPortIndex, nIndex, nColour1);
	Increment(nPortIndex);
}

void PixelPatterns::Scanner(uint32_t nPortIndex, uint32_t nColour1, uint32_t nInterval) {
	Clear(nPortIndex);

	const auto nCount = PixelConfiguration::Get().GetCount();

	s_PortConfig[nPortIndex].ActivePattern = Pattern::SCANNER;
	s_PortConfig[nPortIndex].nInterval = nInterval;
	s_PortConfig[nPortIndex].nTotalSteps= static_cast<uint16_t>((nCount - 1U) * 2);
    s_PortConfig[nPortIndex].nColour1 = nColour1;
    s_PortConfig[nPortIndex].nPixelIndex = 0;


    if (s_pScannerColours == nullptr) {
    	s_pScannerColours = new uint32_t[nCount];
    	assert(s_pScannerColours != nullptr);
    	for (uint32_t i = 0; i < nCount; i++) {
    		s_pScannerColours[i] = 0;
    	}
    }
}

void PixelPatterns::ScannerUpdate(uint32_t nPortIndex) {
	const auto nColour1 = s_PortConfig[nPortIndex].nColour1;
	const auto nTotalSteps = s_PortConfig[nPortIndex].nTotalSteps;
	const auto nIndex = s_PortConfig[nPortIndex].nPixelIndex;

	for (uint32_t i = 0; i < PixelConfiguration::Get().GetCount(); i++) {
		if (i == nIndex) {
			SetPixelColour(nPortIndex, i, nColour1);
			s_pScannerColours[i] = nColour1;
		} else if (i == nTotalSteps - nIndex) {
			SetPixelColour(nPortIndex, i, nColour1);
			s_pScannerColours[i] = nColour1;
		} else {
			const auto nDimColour = DimColour(s_pScannerColours[i]);
			SetPixelColour(nPortIndex, i, nDimColour);
			s_pScannerColours[i] = nDimColour;
		}
	}

	Increment(nPortIndex);
}

void PixelPatterns::Fade(uint32_t nPortIndex, uint32_t nColour1, uint32_t nColour2, uint32_t nSteps, uint32_t nInterval, pixelpatterns::Direction Direction) {
	Clear(nPortIndex);

	s_PortConfig[nPortIndex].ActivePattern = Pattern::FADE;
	s_PortConfig[nPortIndex].nInterval = nInterval;
	s_PortConfig[nPortIndex].nTotalSteps= nSteps;
	s_PortConfig[nPortIndex].nColour1 = nColour1;
	s_PortConfig[nPortIndex].nColour2 = nColour2;
    s_PortConfig[nPortIndex].nPixelIndex = 0;
    s_PortConfig[nPortIndex].Direction = Direction;
}

void PixelPatterns::FadeUpdate(uint32_t nPortIndex) {
	const auto nColour1 = s_PortConfig[nPortIndex].nColour1;
	const auto nColour2 = s_PortConfig[nPortIndex].nColour2;
	const auto nTotalSteps = s_PortConfig[nPortIndex].nTotalSteps;
	const auto nIndex =  s_PortConfig[nPortIndex].nPixelIndex;

	const auto nRed = static_cast<uint8_t>(((Red(nColour1) * (nTotalSteps - nIndex)) + (Red(nColour2) * nIndex)) / nTotalSteps);
	const auto nGreen = static_cast<uint8_t>(((Green(nColour1) * (nTotalSteps - nIndex)) + (Green(nColour2) * nIndex)) / nTotalSteps);
	const auto nBlue = static_cast<uint8_t>(((Blue(nColour1) * (nTotalSteps - nIndex)) + (Blue(nColour2) * nIndex)) / nTotalSteps);

	ColourSet(nPortIndex, Colour(nRed, nGreen, nBlue));
	Increment(nPortIndex);
}

uint32_t PixelPatterns::Wheel(uint8_t nWheelPos) {
	nWheelPos = static_cast<uint8_t>(255U - nWheelPos);

	if (nWheelPos < 85) {
		return Colour(static_cast<uint8_t>(255U - nWheelPos * 3), 0, static_cast<uint8_t>(nWheelPos * 3));
	} else if (nWheelPos < 170U) {
		nWheelPos = static_cast<uint8_t>(nWheelPos - 85U);
		return Colour(0, static_cast<uint8_t>(nWheelPos * 3), static_cast<uint8_t>(255U - nWheelPos * 3));
	} else {
		nWheelPos = static_cast<uint8_t>(nWheelPos - 170U);
		return Colour(static_cast<uint8_t>(nWheelPos * 3), static_cast<uint8_t>(255U - nWheelPos * 3), 0);
	}
}

void PixelPatterns::Increment(uint32_t nPortIndex) {
	if (s_PortConfig[nPortIndex].Direction == Direction::FORWARD) {
		s_PortConfig[nPortIndex].nPixelIndex++;
		if (s_PortConfig[nPortIndex].nPixelIndex == s_PortConfig[nPortIndex].nTotalSteps) {
			s_PortConfig[nPortIndex].nPixelIndex = 0;
		}
	} else {
		if (s_PortConfig[nPortIndex].nPixelIndex > 0) {
			--s_PortConfig[nPortIndex].nPixelIndex;
		}
		if (s_PortConfig[nPortIndex].nPixelIndex == 0) {
			s_PortConfig[nPortIndex].nPixelIndex = static_cast<uint16_t>(s_PortConfig[nPortIndex].nTotalSteps - 1);
		}
	}
}

void PixelPatterns::Reverse(uint32_t nPortIndex) {
	if (s_PortConfig[nPortIndex].Direction == Direction::FORWARD) {
		s_PortConfig[nPortIndex].Direction = Direction::REVERSE;
		s_PortConfig[nPortIndex].nPixelIndex = static_cast<uint16_t>(s_PortConfig[nPortIndex].nTotalSteps - 1);
	} else {
		s_PortConfig[nPortIndex].Direction = Direction::FORWARD;
		s_PortConfig[nPortIndex].nPixelIndex = 0;
	}
}
