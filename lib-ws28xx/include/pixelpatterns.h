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
/**
 * Based on https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 */

#ifndef PIXELPATTERNS_H_
#define PIXELPATTERNS_H_

#include <cstdint>
#include <algorithm>
#include <cassert>

#include "pixel.h"
#include "pixelconfiguration.h"

#include "hardware.h"
#include "debug.h"

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
	NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, FADE, LAST
};

static constexpr char PATTERN_NAME[static_cast<uint32_t>(pixelpatterns::Pattern::LAST)][14] = { "None", "Rainbow cycle", "Theater chase", "Colour wipe", "Fade" };

enum class Direction {
	FORWARD, REVERSE
};
}  // namespace pixelpatterns

class PixelPatterns {
public:
	PixelPatterns(const uint32_t nActivePorts) {
		DEBUG_ENTRY
		DEBUG_PRINTF("nActivePorts=%u", nActivePorts);

		s_nActivePorts = std::min(pixelpatterns::MAX_PORTS, nActivePorts);

		DEBUG_EXIT
	}

	~PixelPatterns() = default;

	static const char *GetName(const pixelpatterns::Pattern pattern) {
		if (pattern < pixelpatterns::Pattern::LAST) {
			return pixelpatterns::PATTERN_NAME[static_cast<uint32_t>(pattern)];
		}

		return "Unknown";
	}

	inline uint32_t GetActivePorts() const {
		return s_nActivePorts;
	}

	void RainbowCycle(const uint32_t nPortIndex, const uint32_t nInterval, const pixelpatterns::Direction Direction = pixelpatterns::Direction::FORWARD) {
		Clear(nPortIndex);

		s_PortConfig[nPortIndex].ActivePattern = pixelpatterns::Pattern::RAINBOW_CYCLE;
		s_PortConfig[nPortIndex].nInterval = nInterval;
		s_PortConfig[nPortIndex].nTotalSteps= 255;
		s_PortConfig[nPortIndex].nPixelIndex = 0;
		s_PortConfig[nPortIndex].Direction = Direction;
	}

	void TheaterChase(const uint32_t nPortIndex, const uint32_t nColour1, const uint32_t nColour2, const uint32_t nInterval, pixelpatterns::Direction Direction = pixelpatterns::Direction::FORWARD) {
		Clear(nPortIndex);

		s_PortConfig[nPortIndex].ActivePattern = pixelpatterns::Pattern::THEATER_CHASE;
		s_PortConfig[nPortIndex].nInterval = nInterval;
		s_PortConfig[nPortIndex].nTotalSteps= PixelConfiguration::Get().GetCount();
		s_PortConfig[nPortIndex].nColour1 = nColour1;
		s_PortConfig[nPortIndex].nColour2 = nColour2;
	    s_PortConfig[nPortIndex].nPixelIndex = 0;
	    s_PortConfig[nPortIndex].Direction = Direction;
	}

	void ColourWipe(const uint32_t nPortIndex, const uint32_t nColour, const uint32_t nInterval, pixelpatterns::Direction Direction = pixelpatterns::Direction::FORWARD) {
		Clear(nPortIndex);

		s_PortConfig[nPortIndex].ActivePattern = pixelpatterns::Pattern::COLOR_WIPE;
		s_PortConfig[nPortIndex].nInterval = nInterval;
		s_PortConfig[nPortIndex].nTotalSteps= PixelConfiguration::Get().GetCount();
	    s_PortConfig[nPortIndex].nColour1 = nColour;
	    s_PortConfig[nPortIndex].nPixelIndex = 0;
	    s_PortConfig[nPortIndex].Direction = Direction;
	}

	void Fade(const uint32_t nPortIndex, const uint32_t nColour1, const uint32_t nColour2, const uint32_t nSteps, const uint32_t nInterval, pixelpatterns::Direction Direction = pixelpatterns::Direction::FORWARD) {
		Clear(nPortIndex);

		s_PortConfig[nPortIndex].ActivePattern = pixelpatterns::Pattern::FADE;
		s_PortConfig[nPortIndex].nInterval = nInterval;
		s_PortConfig[nPortIndex].nTotalSteps= nSteps;
		s_PortConfig[nPortIndex].nColour1 = nColour1;
		s_PortConfig[nPortIndex].nColour2 = nColour2;
	    s_PortConfig[nPortIndex].nPixelIndex = 0;
	    s_PortConfig[nPortIndex].Direction = Direction;
	}

	void None(const uint32_t nPortIndex) {
		DEBUG_ENTRY
		DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

		Clear(nPortIndex);

		s_PortConfig[nPortIndex].ActivePattern = pixelpatterns::Pattern::NONE;

		DEBUG_EXIT
	}

	void Run() {
		if (pixel::is_updating()) {
			return;
		}

		auto bIsUpdated = false;
		const auto nMillis = Hardware::Get()->Millis();

		for (uint32_t i = 0; i < s_nActivePorts; i++) {
			bIsUpdated |= PortUpdate(i, nMillis);
		}

		if (bIsUpdated) {
			pixel::update();
		}
	}
private:
	void RainbowCycleUpdate(const uint32_t nPortIndex) {
		const auto nIndex = s_PortConfig[nPortIndex].nPixelIndex;

		for (uint32_t i = 0; i < PixelConfiguration::Get().GetCount(); i++) {
			pixel::set_pixel_colour(nPortIndex, i, Wheel(((i * 256U / PixelConfiguration::Get().GetCount()) + nIndex) & 0xFF));
		}

		Increment(nPortIndex);
	}

	void TheaterChaseUpdate(const uint32_t nPortIndex) {
		const auto nColour1 = s_PortConfig[nPortIndex].nColour1;
		const auto nColour2 = s_PortConfig[nPortIndex].nColour2;
		const auto nPixelIndex = s_PortConfig[nPortIndex].nPixelIndex;

		for (uint32_t i = 0; i < PixelConfiguration::Get().GetCount(); i++) {
			if ((i + nPixelIndex) % 3 == 0) {
				pixel::set_pixel_colour(nPortIndex, i, nColour1);
			} else {
				pixel::set_pixel_colour(nPortIndex, i, nColour2);
			}
		}

		Increment(nPortIndex);
	}

	void ColourWipeUpdate(const uint32_t nPortIndex) {
		const auto nColour1 = s_PortConfig[nPortIndex].nColour1;
		const auto nIndex = s_PortConfig[nPortIndex].nPixelIndex;

		pixel::set_pixel_colour(nPortIndex, nIndex, nColour1);
		Increment(nPortIndex);
	}

	void FadeUpdate(const uint32_t nPortIndex) {
		const auto nColour1 = s_PortConfig[nPortIndex].nColour1;
		const auto nColour2 = s_PortConfig[nPortIndex].nColour2;
		const auto nTotalSteps = s_PortConfig[nPortIndex].nTotalSteps;
		const auto nIndex =  s_PortConfig[nPortIndex].nPixelIndex;

		const auto nRed = static_cast<uint8_t>(((pixel::get_red(nColour1) * (nTotalSteps - nIndex)) + (pixel::get_red(nColour2) * nIndex)) / nTotalSteps);
		const auto nGreen = static_cast<uint8_t>(((pixel::get_green(nColour1) * (nTotalSteps - nIndex)) + (pixel::get_green(nColour2) * nIndex)) / nTotalSteps);
		const auto nBlue = static_cast<uint8_t>(((pixel::get_blue(nColour1) * (nTotalSteps - nIndex)) + (pixel::get_blue(nColour2) * nIndex)) / nTotalSteps);

		pixel::set_pixel_colour(nPortIndex, pixel::get_colour(nRed, nGreen, nBlue));
		Increment(nPortIndex);
	}

	bool PortUpdate(const uint32_t nPortIndex, const uint32_t nMillis) {
		if ((nMillis - s_PortConfig[nPortIndex].nLastUpdate) < s_PortConfig[nPortIndex].nInterval) {
			return false;
		}

		s_PortConfig[nPortIndex].nLastUpdate = nMillis;

		switch (s_PortConfig[nPortIndex].ActivePattern) {
		case pixelpatterns::Pattern::RAINBOW_CYCLE:
			RainbowCycleUpdate(nPortIndex);
			break;
		case pixelpatterns::Pattern::THEATER_CHASE:
			TheaterChaseUpdate(nPortIndex);
			break;
		case pixelpatterns::Pattern::COLOR_WIPE:
			ColourWipeUpdate(nPortIndex);
			break;
		case pixelpatterns::Pattern::FADE:
			FadeUpdate(nPortIndex);
			break;
		default:
			return false;
			break;
		}

		return true;
	}

	uint32_t Wheel(uint8_t nWheelPos) {
		nWheelPos = static_cast<uint8_t>(255U - nWheelPos);

		if (nWheelPos < 85) {
			return pixel::get_colour(static_cast<uint8_t>(255U - nWheelPos * 3), 0, static_cast<uint8_t>(nWheelPos * 3));
		} else if (nWheelPos < 170U) {
			nWheelPos = static_cast<uint8_t>(nWheelPos - 85U);
			return pixel::get_colour(0, static_cast<uint8_t>(nWheelPos * 3), static_cast<uint8_t>(255U - nWheelPos * 3));
		} else {
			nWheelPos = static_cast<uint8_t>(nWheelPos - 170U);
			return pixel::get_colour(static_cast<uint8_t>(nWheelPos * 3), static_cast<uint8_t>(255U - nWheelPos * 3), 0);
		}
	}

	void Increment(const uint32_t nPortIndex) {
		if (s_PortConfig[nPortIndex].Direction == pixelpatterns::Direction::FORWARD) {
			s_PortConfig[nPortIndex].nPixelIndex++;
			if (s_PortConfig[nPortIndex].nPixelIndex == s_PortConfig[nPortIndex].nTotalSteps) {
				s_PortConfig[nPortIndex].nPixelIndex = 0;
			}
		} else {
			if (s_PortConfig[nPortIndex].nPixelIndex > 0) {
				s_PortConfig[nPortIndex].nPixelIndex--;
			}
			if (s_PortConfig[nPortIndex].nPixelIndex == 0) {
				s_PortConfig[nPortIndex].nPixelIndex = s_PortConfig[nPortIndex].nTotalSteps - 1;
			}
		}
	}

	void Reverse(const uint32_t nPortIndex) {
		if (s_PortConfig[nPortIndex].Direction == pixelpatterns::Direction::FORWARD) {
			s_PortConfig[nPortIndex].Direction = pixelpatterns::Direction::REVERSE;
			s_PortConfig[nPortIndex].nPixelIndex = s_PortConfig[nPortIndex].nTotalSteps - 1;
		} else {
			s_PortConfig[nPortIndex].Direction = pixelpatterns::Direction::FORWARD;
			s_PortConfig[nPortIndex].nPixelIndex = 0;
		}
	}

    uint32_t DimColour(const uint32_t nColour) {
        return pixel::get_colour(static_cast<uint8_t>(pixel::get_red(nColour) >> 1), static_cast<uint8_t>(pixel::get_green(nColour) >> 1), static_cast<uint8_t>(pixel::get_blue(nColour) >> 1));
    }

	void Clear(const uint32_t nPortIndex) {
		pixel::set_pixel_colour(nPortIndex, 0);
	}

private:
	static inline uint32_t s_nActivePorts;

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

	static inline PortConfig s_PortConfig[pixelpatterns::MAX_PORTS];
};

#endif /* PIXELPATTERNS_H_ */
