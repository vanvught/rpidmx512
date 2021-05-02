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

#if defined (OUTPUT_PIXEL_MULTI)
# include "ws28xxmulti.h"
#else
# include "ws28xx.h"
#endif

#include "hardware.h"

using namespace pixelpatterns;

PixelPatterns::PixelPatterns(uint32_t nActivePorts): m_nActivePorts(std::min(MAX_PORTS, nActivePorts)) {
#if defined (OUTPUT_PIXEL_MULTI)
	m_pOutput = WS28xxMulti::Get();
#else
	m_pOutput = WS28xx::Get();
#endif

	assert(m_pOutput != nullptr);

	m_nCount = m_pOutput->GetCount();
	const auto nMillis = Hardware::Get()->Millis();

	for (uint32_t i = 0; i < MAX_PORTS; i++) {
		m_PortConfig[i].ActivePattern = Pattern::NONE;
		m_PortConfig[i].nLastUpdate = nMillis;
		m_PortConfig[i].Direction = Direction::FORWARD;
	}
}

void PixelPatterns::Run() {
	while (m_pOutput->IsUpdating()) {
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

bool PixelPatterns::PortUpdate(uint32_t nPort, uint32_t nMillis) {
	if ((nMillis - m_PortConfig[nPort].nLastUpdate) < m_PortConfig[nPort].nInterval) {
		return false;
	}

	m_PortConfig[nPort].nLastUpdate = nMillis;

	switch (m_PortConfig[nPort].ActivePattern) {
	case Pattern::RAINBOW_CYCLE:
		RainbowCycleUpdate(nPort);
		break;
	case Pattern::THEATER_CHASE:
		TheaterChaseUpdate(nPort);
		break;
	case Pattern::COLOR_WIPE:
		ColourWipeUpdate(nPort);
		break;
	case Pattern::SCANNER:
		ScannerUpdate(nPort);
		break;
	case Pattern::FADE:
		FadeUpdate(nPort);
		break;
	default:
		return false;
		break;
	}

	return true;
}

void PixelPatterns::RainbowCycle(uint32_t nPort, uint32_t nInterval, pixelpatterns::Direction Direction) {
	m_PortConfig[nPort].ActivePattern = Pattern::RAINBOW_CYCLE;
	m_PortConfig[nPort].nInterval = nInterval;
	m_PortConfig[nPort].nTotalSteps= 255;
	m_PortConfig[nPort].nIndex = 0;
	m_PortConfig[nPort].Direction = Direction;
}

void PixelPatterns::RainbowCycleUpdate(uint32_t nPort) {
	const auto nIndex = m_PortConfig[nPort].nIndex;

	for (uint32_t i = 0; i < m_nCount; i++) {
		SetPixelColour(nPort, i, Wheel(((i * 256 / m_nCount) + nIndex) & 255));
	}

	Increment(nPort);
}

void PixelPatterns::TheaterChase(uint32_t nPort, uint32_t nColour1, uint32_t nColour2, uint8_t nInterval, pixelpatterns::Direction Direction){
	m_PortConfig[nPort].ActivePattern = Pattern::THEATER_CHASE;
	m_PortConfig[nPort].nInterval = nInterval;
	m_PortConfig[nPort].nTotalSteps= m_nCount;
	m_PortConfig[nPort].nColour1 = nColour1;
	m_PortConfig[nPort].nColour2 = nColour2;
    m_PortConfig[nPort].nIndex = 0;
    m_PortConfig[nPort].Direction = Direction;
}

void PixelPatterns::TheaterChaseUpdate(uint32_t nPort) {
	const auto Colour1 = m_PortConfig[nPort].nColour1;
	const auto Colour2 = m_PortConfig[nPort].nColour2;
	const auto Index = m_PortConfig[nPort].nIndex;

	for (uint32_t i = 0; i < m_nCount; i++) {
		if ((i + Index) % 3 == 0) {
			SetPixelColour(nPort, i, Colour1);
		} else {
			SetPixelColour(nPort, i, Colour2);
		}
	}

	Increment(nPort);
}

void PixelPatterns::ColourWipe(uint32_t nPort, uint32_t nColour, uint32_t nInterval, pixelpatterns::Direction Direction) {
	m_PortConfig[nPort].ActivePattern = Pattern::COLOR_WIPE;
	m_PortConfig[nPort].nInterval = nInterval;
	m_PortConfig[nPort].nTotalSteps= m_nCount;
    m_PortConfig[nPort].nColour1 = nColour;
    m_PortConfig[nPort].nIndex = 0;
    m_PortConfig[nPort].Direction = Direction;
}

void PixelPatterns::ColourWipeUpdate(uint32_t nPort) {
	const auto nColour1 = m_PortConfig[nPort].nColour1;
	const auto nIndex = m_PortConfig[nPort].nIndex;

	SetPixelColour(nPort, nIndex, nColour1);
	Increment(nPort);
}

void PixelPatterns::Scanner(uint32_t nPort, uint32_t nColour1, uint32_t nInterval) {
	m_PortConfig[nPort].ActivePattern = Pattern::SCANNER;
	m_PortConfig[nPort].nInterval = nInterval;
	m_PortConfig[nPort].nTotalSteps= (m_nCount - 1U) * 2U;
    m_PortConfig[nPort].nColour1 = nColour1;
    m_PortConfig[nPort].nIndex = 0;


    if (m_pScannerColours == nullptr) {
    	m_pScannerColours = new uint32_t[m_nCount];
    	assert(m_pScannerColours != nullptr);
    	for (uint32_t i = 0; i < m_nCount; i++) {
    		m_pScannerColours[i] = 0;
    	}
    }
}

void PixelPatterns::ScannerUpdate(uint32_t nPort) {
	const auto nColour1 = m_PortConfig[nPort].nColour1;
	const auto nTotalSteps = m_PortConfig[nPort].nTotalSteps;
	const auto nIndex = m_PortConfig[nPort].nIndex;

	for (uint32_t i = 0; i < m_nCount; i++) {
		if (i == nIndex) {
			SetPixelColour(nPort, i, nColour1);
			m_pScannerColours[i] = nColour1;
		} else if (i == nTotalSteps - nIndex) {
			SetPixelColour(nPort, i, nColour1);
			m_pScannerColours[i] = nColour1;
		} else {
			const auto nDimColour = DimColour(m_pScannerColours[i]);
			SetPixelColour(nPort, i, nDimColour);
			m_pScannerColours[i] = nDimColour;
		}
	}

	Increment(nPort);
}

void PixelPatterns::Fade(uint32_t nPort, uint32_t nColour1, uint32_t nColour2, uint32_t nSteps, uint32_t nInterval, pixelpatterns::Direction Direction) {
	m_PortConfig[nPort].ActivePattern = Pattern::FADE;
	m_PortConfig[nPort].nInterval = nInterval;
	m_PortConfig[nPort].nTotalSteps= nSteps;
	m_PortConfig[nPort].nColour1 = nColour1;
	m_PortConfig[nPort].nColour2 = nColour2;
    m_PortConfig[nPort].nIndex = 0;
    m_PortConfig[nPort].Direction = Direction;
}

void PixelPatterns::FadeUpdate(uint32_t nPort) {
	const auto nColour1 = m_PortConfig[nPort].nColour1;
	const auto nColour2 = m_PortConfig[nPort].nColour2;
	const auto nTotalSteps = m_PortConfig[nPort].nTotalSteps;
	const auto nIndex =  m_PortConfig[nPort].nIndex;

	const auto nRed = static_cast<uint8_t>(((Red(nColour1) * (nTotalSteps - nIndex)) + (Red(nColour2) * nIndex)) / nTotalSteps);
	const auto nGreen = static_cast<uint8_t>(((Green(nColour1) * (nTotalSteps - nIndex)) + (Green(nColour2) * nIndex)) / nTotalSteps);
	const auto nBlue = static_cast<uint8_t>(((Blue(nColour1) * (nTotalSteps - nIndex)) + (Blue(nColour2) * nIndex)) / nTotalSteps);

	ColourSet(nPort, Colour(nRed, nGreen, nBlue));
	Increment(nPort);
}

uint32_t PixelPatterns::Wheel(uint8_t nWheelPos) {
	nWheelPos = 255 - nWheelPos;

	if (nWheelPos < 85) {
		return Colour(static_cast<uint8_t>(255 - nWheelPos * 3), 0, nWheelPos * 3);
	} else if (nWheelPos < 170) {
		nWheelPos -= 85;
		return Colour(0, nWheelPos * 3, static_cast<uint8_t>(255 - nWheelPos * 3));
	} else {
		nWheelPos -= 170;
		return Colour(nWheelPos * 3, static_cast<uint8_t>(255 - nWheelPos * 3), 0);
	}
}

void PixelPatterns::Increment(uint32_t nPort) {
	if (m_PortConfig[nPort].Direction == Direction::FORWARD) {
		m_PortConfig[nPort].nIndex++;
		if (m_PortConfig[nPort].nIndex == m_PortConfig[nPort].nTotalSteps) {
			m_PortConfig[nPort].nIndex = 0;
		}
	} else {
		if (m_PortConfig[nPort].nIndex > 0) {
			--m_PortConfig[nPort].nIndex;
		}
		if (m_PortConfig[nPort].nIndex == 0) {
			m_PortConfig[nPort].nIndex = m_PortConfig[nPort].nTotalSteps - 1;
		}
	}
}

void PixelPatterns::Reverse(uint32_t nPort) {
	if (m_PortConfig[nPort].Direction == Direction::FORWARD) {
		m_PortConfig[nPort].Direction = Direction::REVERSE;
		m_PortConfig[nPort].nIndex = m_PortConfig[nPort].nTotalSteps - 1;
	} else {
		m_PortConfig[nPort].Direction = Direction::FORWARD;
		m_PortConfig[nPort].nIndex = 0;
	}
}
