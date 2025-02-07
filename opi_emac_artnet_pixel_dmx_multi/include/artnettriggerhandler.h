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
#include "lightsetwith4.h"

#include "pixeltestpattern.h"
#include "pixel.h"
#include "pixeldmxconfiguration.h"

#include "display.h"
#include "displayudf.h"

#include "debug.h"

class ArtNetTriggerHandler {
public:
	ArtNetTriggerHandler(LightSetWith4<32> *pLightSet32with4, LightSet *pLightSetA): m_pLightSet32with4(pLightSet32with4), m_pLightSetA(pLightSetA) {
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
		DEBUG_ENTRY

		if (pArtNetTrigger->Key == ArtTriggerKey::ART_TRIGGER_KEY_SHOW) {
			m_pLightSet32with4->SetLightSetA(m_pLightSetA);

			const auto nShow = static_cast<pixelpatterns::Pattern>(pArtNetTrigger->SubKey);

			DEBUG_PRINTF("nShow=%u", static_cast<uint32_t>(nShow));

			if (nShow == PixelTestPattern::Get()->GetPattern()) {
				DEBUG_EXIT
				return;
			}

			const auto isSet = PixelTestPattern::Get()->SetPattern(nShow);

			DEBUG_PRINTF("isSet=%u", static_cast<uint32_t>(isSet));

			if (!isSet) {
				DEBUG_EXIT
				return;
			}

			if (static_cast<pixelpatterns::Pattern>(nShow) != pixelpatterns::Pattern::NONE) {
				m_pLightSet32with4->SetLightSetA(nullptr);
				Display::Get()->ClearLine(6);
				Display::Get()->Printf(6, "%s:%u", PixelPatterns::GetName(nShow), static_cast<uint32_t>(nShow));
			} else {
				m_pLightSetA->Blackout(true);
				DisplayUdf::Get()->Show();
			}

			DEBUG_EXIT
			return;
		}

		if (pArtNetTrigger->Key == ArtTriggerKey::ART_TRIGGER_UNDEFINED) {
			if (pArtNetTrigger->SubKey == 0) {
				const auto isSet = PixelTestPattern::Get()->SetPattern(pixelpatterns::Pattern::NONE);

				DEBUG_PRINTF("isSet=%u", static_cast<uint32_t>(isSet));

				if (!isSet) {
					DEBUG_EXIT
					return;
				}

				m_pLightSet32with4->SetLightSetA(nullptr);

				auto& pixelDmxConfiguration = PixelDmxConfiguration::Get();
				const auto *pData = &pArtNetTrigger->Data[0];
				const uint32_t nColour = pData[0] | (static_cast<uint32_t>(pData[1]) << 8) | (static_cast<uint32_t>(pData[2]) << 16) | (static_cast<uint32_t>(pData[3]) << 24);

				for (uint32_t nPort = 0; nPort < pixelDmxConfiguration.GetOutputPorts(); nPort++) {
					pixel::set_pixel_colour(nPort, nColour);
				}

				pixel::update();
			}
		}
	}

private:
	LightSetWith4<32> *m_pLightSet32with4;
	LightSet *m_pLightSetA;

	static inline ArtNetTriggerHandler *s_pThis;
};

#endif /* ARTNETTRIGGERHANDLER_H_ */
