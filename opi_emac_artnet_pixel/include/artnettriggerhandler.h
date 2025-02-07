/**
 * @file artnettriggerhandler.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARTNETTRIGGERHANDLER_H_
#define ARTNETTRIGGERHANDLER_H_

#include <cassert>

#include "artnettrigger.h"
#include "artnetnode.h"

#include "lightset.h"

#include "pixeltestpattern.h"

#include "display.h"
#include "displayudf.h"

class ArtNetTriggerHandler {
public:
	ArtNetTriggerHandler(LightSet *pLightSet): m_pLightSet(pLightSet) {
		assert(s_pThis == nullptr);
		s_pThis = this;

		ArtNetNode::Get()->SetArtTriggerCallbackFunctionPtr(StaticCallbackFunction);
	}

	~ArtNetTriggerHandler() = default;

	void static StaticCallbackFunction(const ArtNetTrigger *pArtNetTrigger) {
		assert(s_pThis != nullptr);
		s_pThis->Handler(pArtNetTrigger);
	}

private:
	void Handler(const ArtNetTrigger *pArtNetTrigger) {
		if (pArtNetTrigger->Key == ArtTriggerKey::ART_TRIGGER_KEY_SHOW) {
			ArtNetNode::Get()->SetOutput(m_pLightSet);

			const auto nShow = static_cast<pixelpatterns::Pattern>(pArtNetTrigger->SubKey);

			if (nShow == PixelTestPattern::Get()->GetPattern()) {
				return;
			}

			const auto isSet = PixelTestPattern::Get()->SetPattern(nShow);

			if (!isSet) {
				return;
			}

			if (static_cast<pixelpatterns::Pattern>(nShow) != pixelpatterns::Pattern::NONE) {
				ArtNetNode::Get()->SetOutput(nullptr);
				Display::Get()->ClearLine(6);
				Display::Get()->Printf(6, "%s:%u", PixelPatterns::GetName(nShow), static_cast<uint32_t>(nShow));
			} else {
				m_pLightSet->Blackout(true);
				DisplayUdf::Get()->Show();
			}

			return;
		}

		if (pArtNetTrigger->Key == ArtTriggerKey::ART_TRIGGER_UNDEFINED) {
			if (pArtNetTrigger->SubKey == 0) {
				const auto isSet = PixelTestPattern::Get()->SetPattern(pixelpatterns::Pattern::NONE);

				if (!isSet) {
					return;
				}

				ArtNetNode::Get()->SetOutput(nullptr);

				const auto *pData = &pArtNetTrigger->Data[0];
				const uint32_t nColour = pData[0] | (static_cast<uint32_t>(pData[1]) << 8) | (static_cast<uint32_t>(pData[2]) << 16) | (static_cast<uint32_t>(pData[3]) << 24);

				pixel::set_pixel_colour(0, nColour);
				pixel::update();
			}
		}
	}

private:
	LightSet *m_pLightSet;

	static inline ArtNetTriggerHandler *s_pThis;
};

#endif /* ARTNETTRIGGERHANDLER_H_ */
