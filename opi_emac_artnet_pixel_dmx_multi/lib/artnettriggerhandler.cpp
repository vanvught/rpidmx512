/**
 * @file artnettriggerhandler.cpp
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

#include <cstdint>
#include <cassert>

#include "artnettrigger.h"
#include "artnetnode.h"

#include "lightset.h"
#include "lightsetwith4.h"

#include "pixeltestpattern.h"

#include "display.h"
#include "displayudf.h"

namespace artnet {
static LightSetWith4<32> *s_pLightSet32with4;
static LightSet *s_pLightSetA;

void triggerhandler_set_lightset(LightSetWith4<32> *pLightSet32with4, LightSet *pLightSetA) {
	s_pLightSet32with4 = pLightSet32with4;
	s_pLightSetA = pLightSetA;
}

void triggerhandler(const ArtNetTrigger *pArtNetTrigger) {
	if (pArtNetTrigger->Key == ArtTriggerKey::ART_TRIGGER_KEY_SHOW) {
		const auto nShow = static_cast<pixelpatterns::Pattern>(pArtNetTrigger->SubKey);
		if (nShow == PixelTestPattern::Get()->GetPattern()) {
			return;
		}
		const auto isSet = PixelTestPattern::Get()->SetPattern(nShow);

		if(!isSet) {
			return;
		}

		if (static_cast<pixelpatterns::Pattern>(nShow) != pixelpatterns::Pattern::NONE) {
			s_pLightSet32with4->SetLightSetA(nullptr);
			Display::Get()->ClearLine(6);
			Display::Get()->Printf(6, "%s:%u", PixelPatterns::GetName(nShow), static_cast<uint32_t>(nShow));
		} else {
			s_pLightSetA->Blackout(true);
			s_pLightSet32with4->SetLightSetA(s_pLightSetA);
			DisplayUdf::Get()->Show();
		}
	}
}
}  // namespace artnet
