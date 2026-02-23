/**
 * @file json_status_pixel.cpp
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

#include <cstdint>
#include <cstdio>

#include "pixelconfiguration.h"
#if defined(OUTPUT_DMX_PIXEL)
#include "pixeloutput.h"
#elif defined(OUTPUT_DMX_PIXEL_MULTI)
#include "pixeloutputmulti.h"
#endif

#if defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI)
namespace json::status
{
uint32_t Pixel(char* out_buffer, uint32_t out_buffer_size)
{
    auto& configuration = PixelConfiguration::Get();
    const auto kUserData = PixelOutputType::Get()->GetUserData();

    return static_cast<uint32_t>(snprintf(out_buffer, out_buffer_size, 
    "{\"refresh_rate\":\"%u\",\"frame_rate\":\"%u\"}", 
    configuration.GetRefreshRate(), 
    kUserData));
}
} // namespace json::status
#endif