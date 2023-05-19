/**
 * @file pixelpatterns.cpp
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
WS28xxMulti *PixelPatterns::m_pOutput;
#else
WS28xx *PixelPatterns::m_pOutput;
#endif
uint32_t PixelPatterns::m_nActivePorts;
uint32_t PixelPatterns::m_nCount;
PixelPatterns::PortConfig PixelPatterns::m_PortConfig[pixelpatterns::MAX_PORTS];
uint32_t *PixelPatterns::m_pScannerColours;

PixelPatterns::PixelPatterns(uint32_t nActivePorts) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nActivePorts=%u", nActivePorts);

#if defined (PIXELPATTERNS_MULTI)
	m_pOutput = WS28xxMulti::Get();
#else
	m_pOutput = WS28xx::Get();
#endif
	assert(m_pOutput != nullptr);

	m_nActivePorts = std::min(MAX_PORTS, nActivePorts);
	m_nCount = m_pOutput->GetCount();
	const auto nMillis = Hardware::Get()->Millis();

	for (uint32_t i = 0; i < MAX_PORTS; i++) {
		m_PortConfig[i].ActivePattern = Pattern::NONE;
		m_PortConfig[i].nLastUpdate = nMillis;
		m_PortConfig[i].Direction = Direction::FORWARD;
	}

	DEBUG_EXIT
}

const char* PixelPatterns::GetName(Pattern pattern) {
	if (pattern < Pattern::LAST) {
		return s_patternName[static_cast<uint32_t>(pattern)];
	}

	return "Unknown";
}

void PixelPatterns::Run() {
	if (m_pOutput->IsUpdating()) {
		return;
	}

	auto bIsUpdated = false;
	const auto nMillis = Hardware::Get()->Millis();

	for (uint32_t i = 0; i < m_nActivePorts; i++) {
		bIsUpdated |= PortUpdate(i, nMillis);
	}

	if (bIsUpdated) {
		m_pOutput->Update();
	}
}

bool PixelPatterns::PortUpdate(uint32_t nPortIndex, uint32_t nMillis) {
	if ((nMillis - m_PortConfig[nPortIndex].nLastUpdate) < m_PortConfig[nPortIndex].nInterval) {
		return false;
	}

	m_PortConfig[nPortIndex].nLastUpdate = nMillis;

	switch (m_PortConfig[nPortIndex].ActivePattern) {
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

	m_PortConfig[nPortIndex].ActivePattern = Pattern::RAINBOW_CYCLE;
	m_PortConfig[nPortIndex].nInterval = nInterval;
	m_PortConfig[nPortIndex].nTotalSteps= 255;
	m_PortConfig[nPortIndex].nPixelIndex = 0;
	m_PortConfig[nPortIndex].Direction = Direction;
}

void PixelPatterns::RainbowCycleUpdate(uint32_t nPortIndex) {
	const auto nIndex = m_PortConfig[nPortIndex].nPixelIndex;

	for (uint32_t i = 0; i < m_nCount; i++) {
		SetPixelColour(nPortIndex, i, Wheel(((i * 256U / m_nCount) + nIndex) & 0xFF));
	}

	Increment(nPortIndex);
}

void PixelPatterns::TheaterChase(uint32_t nPortIndex, uint32_t nColour1, uint32_t nColour2, uint32_t nInterval, pixelpatterns::Direction Direction){
	Clear(nPortIndex);

	m_PortConfig[nPortIndex].ActivePattern = Pattern::THEATER_CHASE;
	m_PortConfig[nPortIndex].nInterval = nInterval;
	m_PortConfig[nPortIndex].nTotalSteps= m_nCount;
	m_PortConfig[nPortIndex].nColour1 = nColour1;
	m_PortConfig[nPortIndex].nColour2 = nColour2;
    m_PortConfig[nPortIndex].nPixelIndex = 0;
    m_PortConfig[nPortIndex].Direction = Direction;
}

void PixelPatterns::TheaterChaseUpdate(uint32_t nPortIndex) {
	const auto Colour1 = m_PortConfig[nPortIndex].nColour1;
	const auto Colour2 = m_PortConfig[nPortIndex].nColour2;
	const auto Index = m_PortConfig[nPortIndex].nPixelIndex;

	for (uint32_t i = 0; i < m_nCount; i++) {
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

	m_PortConfig[nPortIndex].ActivePattern = Pattern::COLOR_WIPE;
	m_PortConfig[nPortIndex].nInterval = nInterval;
	m_PortConfig[nPortIndex].nTotalSteps= m_nCount;
    m_PortConfig[nPortIndex].nColour1 = nColour;
    m_PortConfig[nPortIndex].nPixelIndex = 0;
    m_PortConfig[nPortIndex].Direction = Direction;
}

void PixelPatterns::ColourWipeUpdate(uint32_t nPortIndex) {
	const auto nColour1 = m_PortConfig[nPortIndex].nColour1;
	const auto nIndex = m_PortConfig[nPortIndex].nPixelIndex;

	SetPixelColour(nPortIndex, nIndex, nColour1);
	Increment(nPortIndex);
}

void PixelPatterns::Scanner(uint32_t nPortIndex, uint32_t nColour1, uint32_t nInterval) {
	Clear(nPortIndex);

	m_PortConfig[nPortIndex].ActivePattern = Pattern::SCANNER;
	m_PortConfig[nPortIndex].nInterval = nInterval;
	m_PortConfig[nPortIndex].nTotalSteps= static_cast<uint16_t>((m_nCount - 1U) * 2);
    m_PortConfig[nPortIndex].nColour1 = nColour1;
    m_PortConfig[nPortIndex].nPixelIndex = 0;


    if (m_pScannerColours == nullptr) {
    	m_pScannerColours = new uint32_t[m_nCount];
    	assert(m_pScannerColours != nullptr);
    	for (uint32_t i = 0; i < m_nCount; i++) {
    		m_pScannerColours[i] = 0;
    	}
    }
}

void PixelPatterns::ScannerUpdate(uint32_t nPortIndex) {
	const auto nColour1 = m_PortConfig[nPortIndex].nColour1;
	const auto nTotalSteps = m_PortConfig[nPortIndex].nTotalSteps;
	const auto nIndex = m_PortConfig[nPortIndex].nPixelIndex;

	for (uint32_t i = 0; i < m_nCount; i++) {
		if (i == nIndex) {
			SetPixelColour(nPortIndex, i, nColour1);
			m_pScannerColours[i] = nColour1;
		} else if (i == nTotalSteps - nIndex) {
			SetPixelColour(nPortIndex, i, nColour1);
			m_pScannerColours[i] = nColour1;
		} else {
			const auto nDimColour = DimColour(m_pScannerColours[i]);
			SetPixelColour(nPortIndex, i, nDimColour);
			m_pScannerColours[i] = nDimColour;
		}
	}

	Increment(nPortIndex);
}

void PixelPatterns::Fade(uint32_t nPortIndex, uint32_t nColour1, uint32_t nColour2, uint32_t nSteps, uint32_t nInterval, pixelpatterns::Direction Direction) {
	Clear(nPortIndex);

	m_PortConfig[nPortIndex].ActivePattern = Pattern::FADE;
	m_PortConfig[nPortIndex].nInterval = nInterval;
	m_PortConfig[nPortIndex].nTotalSteps= nSteps;
	m_PortConfig[nPortIndex].nColour1 = nColour1;
	m_PortConfig[nPortIndex].nColour2 = nColour2;
    m_PortConfig[nPortIndex].nPixelIndex = 0;
    m_PortConfig[nPortIndex].Direction = Direction;
}

void PixelPatterns::FadeUpdate(uint32_t nPortIndex) {
	const auto nColour1 = m_PortConfig[nPortIndex].nColour1;
	const auto nColour2 = m_PortConfig[nPortIndex].nColour2;
	const auto nTotalSteps = m_PortConfig[nPortIndex].nTotalSteps;
	const auto nIndex =  m_PortConfig[nPortIndex].nPixelIndex;

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
	if (m_PortConfig[nPortIndex].Direction == Direction::FORWARD) {
		m_PortConfig[nPortIndex].nPixelIndex++;
		if (m_PortConfig[nPortIndex].nPixelIndex == m_PortConfig[nPortIndex].nTotalSteps) {
			m_PortConfig[nPortIndex].nPixelIndex = 0;
		}
	} else {
		if (m_PortConfig[nPortIndex].nPixelIndex > 0) {
			--m_PortConfig[nPortIndex].nPixelIndex;
		}
		if (m_PortConfig[nPortIndex].nPixelIndex == 0) {
			m_PortConfig[nPortIndex].nPixelIndex = static_cast<uint16_t>(m_PortConfig[nPortIndex].nTotalSteps - 1);
		}
	}
}

void PixelPatterns::Reverse(uint32_t nPortIndex) {
	if (m_PortConfig[nPortIndex].Direction == Direction::FORWARD) {
		m_PortConfig[nPortIndex].Direction = Direction::REVERSE;
		m_PortConfig[nPortIndex].nPixelIndex = static_cast<uint16_t>(m_PortConfig[nPortIndex].nTotalSteps - 1);
	} else {
		m_PortConfig[nPortIndex].Direction = Direction::FORWARD;
		m_PortConfig[nPortIndex].nPixelIndex = 0;
	}
}
