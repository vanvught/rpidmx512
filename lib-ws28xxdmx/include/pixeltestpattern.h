/*
 * pixeltestpattern.h
 *
 *  Created on: 16 apr. 2021
 *      Author: arjanvanvught
 */

#ifndef PIXELTESTPATTERN_H_
#define PIXELTESTPATTERN_H_

#include <stdint.h>

#include "pixelpatterns.h"

class PixelTestPattern {
public:
	PixelTestPattern(pixelpatterns::Pattern Pattern, uint32_t OutputPorts = 1) {
		if ((Pattern != pixelpatterns::Pattern::NONE) && (Pattern < pixelpatterns::Pattern::LAST)) {
			m_pPixelPatterns = new PixelPatterns(OutputPorts);

			const auto nColour1 = m_pPixelPatterns->Colour(0, 0, 0);
			const auto nColour2 = m_pPixelPatterns->Colour(100, 100, 100);
			constexpr auto nInterval = 100;
			constexpr auto nSteps = 10;

			for (uint32_t i = 0; i < OutputPorts; i++) {
				switch (Pattern) {
				case pixelpatterns::Pattern::RAINBOW_CYCLE:
					m_pPixelPatterns->RainbowCycle(i, nInterval);
					break;
				case pixelpatterns::Pattern::THEATER_CHASE:
					m_pPixelPatterns->TheaterChase(i, nColour1, nColour2, nInterval);
					break;
				case pixelpatterns::Pattern::COLOR_WIPE:
					m_pPixelPatterns->ColourWipe(i, nColour2, nInterval);
					break;
				case pixelpatterns::Pattern::SCANNER:
					m_pPixelPatterns->Scanner(i, m_pPixelPatterns->Colour(255, 255, 255), nInterval);
					break;
				case pixelpatterns::Pattern::FADE:
					m_pPixelPatterns->Fade(i, nColour1, nColour2, nSteps, nInterval);
					break;
				default:
					break;
				}
			}
		}
	}

	void Run() {
		if (m_pPixelPatterns != nullptr) {
			m_pPixelPatterns->Run();
		}
	}

private:
	PixelPatterns *m_pPixelPatterns { nullptr };
};

#endif /* PIXELTESTPATTERN_H_ */
