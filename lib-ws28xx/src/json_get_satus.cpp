/**
 * @file json_get.cpp
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
#include <cstdio>

#include "pixelconfiguration.h"

#if defined (OUTPUT_DMX_PIXEL_MULTI)
# include "ws28xxmulti.h"
#else
# include "ws28xx.h"
#endif

namespace remoteconfig::pixel {
uint32_t json_get_status(char *pOutBuffer, const uint32_t nOutBufferSize) {
	auto& pixelConfiguration = PixelConfiguration::Get();
#if defined (OUTPUT_DMX_PIXEL_MULTI)
 	const auto nUserData = WS28xxMulti::Get()->GetUserData();
#else
 	const auto nUserData = WS28xx::Get()->GetUserData();
#endif
 	return static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
 			"{\"refresh_rate\":\"%u\",\"frame_rate\":\"%u\"}",
			pixelConfiguration.GetRefreshRate(),
			nUserData));
}
}  // namespace remoteconfig::pixel
