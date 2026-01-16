/**
 * @file pixeltestpattern.h
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

 #include "firmware/debug/debug_debug.h"

class PixelTestPattern final: PixelPatterns {
public:
	PixelTestPattern(const pixelpatterns::Pattern Pattern, const uint32_t OutputPorts) : PixelPatterns(OutputPorts) {
		DEBUG_ENTRY();

		assert(s_this == nullptr);
		s_this = this;

		SetPattern(Pattern);

		DEBUG_EXIT();
	}

	bool SetPattern(const pixelpatterns::Pattern Pattern) {
		if (Pattern >= pixelpatterns::Pattern::kLast)  {
			return false;
		}

		m_Pattern = Pattern;

		const auto colour1 = pixel::GetColour(0, 0, 0);
		const auto colour2 = pixel::GetColour(100, 100, 100);
		constexpr auto nInterval = 100;
		constexpr auto nSteps = 10;

		for (uint32_t i = 0; i < PixelPatterns::GetActivePorts(); i++) {
			DEBUG_PRINTF("i=%u",i);

			switch (Pattern) {
			case pixelpatterns::Pattern::kRainbowCycle:
				PixelPatterns::RainbowCycle(i, nInterval);
				break;
			case pixelpatterns::Pattern::kTheaterChase:
				PixelPatterns::TheaterChase(i, colour1, colour2, nInterval);
				break;
			case pixelpatterns::Pattern::kColorWipe:
				PixelPatterns::ColourWipe(i, colour2, nInterval);
				break;
			case pixelpatterns::Pattern::kFade:
				PixelPatterns::Fade(i, colour1, colour2, nSteps, nInterval);
				break;
			case pixelpatterns::Pattern::kNone:
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
		if (__builtin_expect((m_Pattern != pixelpatterns::Pattern::kNone), 0)) {
			PixelPatterns::Run();
		}
	}

	pixelpatterns::Pattern GetPattern() const {
		return m_Pattern;
	}

	static PixelTestPattern *Get() {
		return s_this;
	}

private:
	pixelpatterns::Pattern m_Pattern;
	static inline PixelTestPattern *s_this;
};

#endif /* PIXELTESTPATTERN_H_ */
