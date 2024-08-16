/**
 * @file personalityupdate.cpp
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

#include <cstdint>

#include "rdmresponder.h"

#include "ws28xxdmx.h"
#include "pixeltestpattern.h"
#include "pixelpatterns.h"
#include "displayudf.h"

#include "debug.h"

void RDMResponder::PersonalityUpdate(uint32_t nPersonality)  {
	DEBUG_PRINTF("nPersonality=%u", nPersonality);

	DisplayUdf::Get()->ClearEndOfLine();
	DisplayUdf::Get()->Printf(7, "%s:%d G%d %s",
					pixel::pixel_get_type(PixelConfiguration::Get().GetType()),
					PixelConfiguration::Get().GetCount(),
					PixelDmxConfiguration::Get().GetGroupingCount(),
					pixel::pixel_get_map(PixelConfiguration::Get().GetMap()));
	DisplayUdf::Get()->Show();

	if (nPersonality == 1) {
		const auto nTestPattern = PixelTestPattern::Get()->GetPattern();

		if (nTestPattern == pixelpatterns::Pattern::NONE) {
		} else {
			DisplayUdf::Get()->ClearEndOfLine();
			DisplayUdf::Get()->Printf(6, "%s:%u", PixelPatterns::GetName(nTestPattern), static_cast<uint32_t>(nTestPattern));
		}
	} else if (nPersonality == 2) {
		DisplayUdf::Get()->ClearLine(3);
		DisplayUdf::Get()->ClearEndOfLine();
		DisplayUdf::Get()->Write(4, "Config Mode");
		DisplayUdf::Get()->ClearLine(5);
	}
}
