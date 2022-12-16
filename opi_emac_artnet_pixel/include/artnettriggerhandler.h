/**
 * @file artnettriggerhandler.h
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

#ifndef ARTNETTRIGGERHANDLER_H_
#define ARTNETTRIGGERHANDLER_H_

#include "artnettrigger.h"

#include "pixelpatterns.h"
#include "pixeltestpattern.h"
#include "displayudf.h"
#include "artnetnode.h"
#include "lightset.h"
#include "lightsetdata.h"

class ArtNetTriggerHandler: ArtNetTrigger {
public:
	ArtNetTriggerHandler(LightSet *pLightSet): m_pLightSet(pLightSet) {
		ArtNetNode::Get()->SetArtNetTrigger(this);
	}

	~ArtNetTriggerHandler() {}

	void Handler(const TArtNetTrigger *ptArtNetTrigger) override {
		if (ptArtNetTrigger->Key == ART_TRIGGER_KEY_SHOW) {
			const auto nShow = static_cast<pixelpatterns::Pattern>(ptArtNetTrigger->SubKey);
			if (nShow == PixelTestPattern::Get()->GetPattern()) {
				return;
			}
			const auto isSet = PixelTestPattern::Get()->SetPattern(nShow);

			if(!isSet) {
				return;
			}

			if (static_cast<pixelpatterns::Pattern>(nShow) != pixelpatterns::Pattern::NONE) {
				ArtNetNode::Get()->SetOutput(nullptr);
				Display::Get()->ClearLine(6);
				Display::Get()->Printf(6, "%s:%u", PixelPatterns::GetName(nShow), static_cast<uint32_t>(nShow));
			} else {
				m_pLightSet->Blackout(true);
				ArtNetNode::Get()->SetOutput(m_pLightSet);
				DisplayUdf::Get()->Show();
			}
		}
	}

private:
	LightSet *m_pLightSet;
};

#endif /* ARTNETTRIGGERHANDLER_H_ */
