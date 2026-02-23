/*
 * display.h
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "display.h"
#include "pixelpatterns.h"

namespace common::firmware::pixeldmx
{
inline void Show(uint32_t line, pixelpatterns::Pattern pattern = pixelpatterns::Pattern::kNone)
{
    auto& configuration = PixelDmxConfiguration::Get();
    auto* display = Display::Get();
    assert(display != nullptr);

    display->ClearEndOfLine();
    display->Printf(line, "%s:%d G%d %s", pixel::GetType(configuration.GetType()), configuration.GetCount(), configuration.GetGroupingCount(),
                    pixel::GetMap(configuration.GetMap()));
    display->ClearLine(8); // Status line

    if (pattern != pixelpatterns::Pattern::kNone)
    {
        display->ClearLine(6);
        display->Printf(6, "%s:%u", PixelPatterns::GetName(pattern), static_cast<uint32_t>(pattern));
    }
}
} // namespace common::firmware::pixeldmx

#endif // COMMON_FIRMWARE_PIXELDMX_SHOW_H_
