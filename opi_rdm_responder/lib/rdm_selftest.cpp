/**
 * @file rdm_selftest.cpp
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

#include <cstdint>
#include <cstring>

#include "rdm_selftest.h"

#include "pixelpatterns.h"
#include "pixeltestpattern.h"
#include "displayudf.h"
#include "ws28xxdmx.h"
#include "rdmresponder.h"


namespace rdm::selftest {

uint8_t Get() {
	return static_cast<uint8_t>(PixelTestPattern::Get()->GetPattern());
}

bool Set(uint8_t nSelfTest) {
	const auto isSet = PixelTestPattern::Get()->SetPattern(static_cast<pixelpatterns::Pattern>(nSelfTest));

	if(!isSet) {
		return false;
	}

	if (static_cast<pixelpatterns::Pattern>(nSelfTest) != pixelpatterns::Pattern::NONE) {
		RDMResponder::Get()->DmxDisableOutput(true);
		Display::Get()->ClearLine(6);
		Display::Get()->Printf(6, "%s:%u", PixelPatterns::GetName(static_cast<pixelpatterns::Pattern>(nSelfTest)), nSelfTest);
	} else {
		RDMResponder::Get()->DmxDisableOutput(false);
		DisplayUdf::Get()->Show();
	}

	return true;
}

const char* GetDescription(uint8_t nSelfTest, uint32_t& nLength) {
	const auto *pText = PixelPatterns::GetName(static_cast<pixelpatterns::Pattern>(nSelfTest));
	if (pText == nullptr) {
		nLength = 0;
		return nullptr;
	}

	nLength = strlen(pText);
	return pText;
}

} // namespace rdm::selftest


