/**
 * @file rdmidentify.cpp
 *
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "rdmidentify.h"
#include "rdmresponder.h"
#include "pixeltestpattern.h"

#include "debug.h"

static bool s_isOn = false;
static pixelpatterns::Pattern s_Pattern;

using namespace rdm::identify;

void RDMIdentify::On(rdm::identify::Mode nMode) {
	DEBUG_ENTRY
	DEBUG_PRINTF("Mode=%u, s_isOn=%d", static_cast<uint32_t>(nMode), s_isOn);

	if ((nMode == rdm::identify::Mode::LOUD) && (!s_isOn)) {
		s_isOn = true;
		s_Pattern = PixelTestPattern::Get()->GetPattern();
		PixelTestPattern::Get()->SetPattern(pixelpatterns::Pattern::FADE);
		RDMResponder::Get()->DmxDisableOutput(true);
	}

	DEBUG_EXIT
}

void RDMIdentify::Off([[maybe_unused]] rdm::identify::Mode nMode) {
	DEBUG_ENTRY
	DEBUG_PRINTF("Mode=%u, s_isOn=%d", static_cast<uint32_t>(nMode), s_isOn);

	if (s_isOn) {
		s_isOn = false;
		PixelTestPattern::Get()->SetPattern(s_Pattern);
		RDMResponder::Get()->DmxDisableOutput(pixelpatterns::Pattern::NONE != s_Pattern);
	}

	DEBUG_EXIT
}
