/*
 * display.h
 *
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef COMMON_FIRMWARE_PIXELDMX_SHOW_H_
#define COMMON_FIRMWARE_PIXELDMX_SHOW_H_

#include <cstdint>

#include "pixeldmxconfiguration.h"
#include "display.h" // IWYU pragma: keep
#include "pixelpatterns.h"
#include "firmware/debug/debug_debug.h"

namespace common::firmware::pixeldmx {
inline void Show(uint32_t line, pixelpatterns::Pattern pattern) {
	DEBUG_PRINTF("line=%u, pattern=%u", static_cast<unsigned>(line), static_cast<unsigned>(pattern));
	
    auto& configuration = PixelDmxConfiguration::Get();
    auto* display = Display::Get();
    assert(display != nullptr);

    display->ClearEndOfLine();
    display->Printf(line, "%s:%d G%d %s", 
		pixel::GetTypeName(configuration.GetType()), 
		static_cast<unsigned>(configuration.GetCount()), 
		static_cast<unsigned>(configuration.GetGroupingCount()), 
		pixel::GetMapName(configuration.GetMap())
	);

    display->ClearLine(8); // Status line

    display->ClearLine(6);
    if (pattern != pixelpatterns::Pattern::kNone) {
        display->Printf(6, "%s:%u", 
			PixelPatterns::GetName(pattern), 
			static_cast<unsigned>(pattern)
		);
    }
}
} // namespace common::firmware::pixeldmx

#endif // COMMON_FIRMWARE_PIXELDMX_SHOW_H_
