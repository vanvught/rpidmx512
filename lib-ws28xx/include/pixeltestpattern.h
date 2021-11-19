/**
 * @file pixeltestpattern.h
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

#ifndef PIXELTESTPATTERN_H_
#define PIXELTESTPATTERN_H_

#include <cstdint>
#include <cassert>

#include "pixelpatterns.h"

class PixelTestPattern: PixelPatterns {
public:
	PixelTestPattern(pixelpatterns::Pattern Pattern, uint32_t OutputPorts = 1):
		PixelPatterns(OutputPorts)
	{
		s_Pattern = Pattern;
		SetPattern(Pattern);
	}

	bool SetPattern(pixelpatterns::Pattern Pattern) {
		if (Pattern >= pixelpatterns::Pattern::LAST)  {
			return false;
		}

		s_Pattern = Pattern;

		const auto nColour1 = PixelPatterns::Colour(0, 0, 0);
		const auto nColour2 = PixelPatterns::Colour(100, 100, 100);
		constexpr auto nInterval = 100;
		constexpr auto nSteps = 10;

		for (uint32_t i = 0; i < pixelpatterns::MAX_PORTS; i++) {
			switch (Pattern) {
			case pixelpatterns::Pattern::RAINBOW_CYCLE:
				PixelPatterns::RainbowCycle(i, nInterval);
				break;
			case pixelpatterns::Pattern::THEATER_CHASE:
				PixelPatterns::TheaterChase(i, nColour1, nColour2, nInterval);
				break;
			case pixelpatterns::Pattern::COLOR_WIPE:
				PixelPatterns::ColourWipe(i, nColour2, nInterval);
				break;
			case pixelpatterns::Pattern::SCANNER:
				PixelPatterns::Scanner(i, PixelPatterns::Colour(255, 255, 255), nInterval);
				break;
			case pixelpatterns::Pattern::FADE:
				PixelPatterns::Fade(i, nColour1, nColour2, nSteps, nInterval);
				break;
			case pixelpatterns::Pattern::NONE:
				PixelPatterns::None(i);
				break;
			default:
				assert(0);
				break;
			}
		}

		return true;
	}

	void Run() {
		if (s_Pattern != pixelpatterns::Pattern::NONE) {
			PixelPatterns::Run();
		}
	}

	static pixelpatterns::Pattern GetPattern() {
		return s_Pattern;
	}

	static PixelTestPattern* Get() {
		return s_pThis;
	}

private:
	static pixelpatterns::Pattern s_Pattern;
	static PixelTestPattern *s_pThis;
};

#endif /* PIXELTESTPATTERN_H_ */
