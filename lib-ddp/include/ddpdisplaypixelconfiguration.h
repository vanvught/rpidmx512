/**
 * @file ddpdisplaypixelconfiguration.h
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

#ifndef DDPDISPLAYPIXELCONFIGURATION_H_
#define DDPDISPLAYPIXELCONFIGURATION_H_

#include <cstdint>

#include "pixelconfiguration.h"

namespace ddpdisplay {
namespace configuration {
namespace pixel {
static constexpr auto MAX_PORTS = 8;
}  // namespace pixel
}  // namespace configuration
}  // namespace ddp

class DdpDisplayPixelConfiguration: public PixelConfiguration {
public:
	void SetCountPort(uint32_t nPortIndex, uint32_t nCount) {
		if (nPortIndex < ddpdisplay::configuration::pixel::MAX_PORTS) {
			m_nCount[nPortIndex] = nCount;
		}
	}

	uint32_t GetCountPort(uint32_t nPortIndex) const {
		if (nPortIndex < ddpdisplay::configuration::pixel::MAX_PORTS) {
			return m_nCount[nPortIndex];
		}
		return 0;
	}

	void Validate(uint32_t& nLedsPerPixel);

	void Dump();

private:
	uint32_t m_nCount[ddpdisplay::configuration::pixel::MAX_PORTS];
};

#endif /* DDPDISPLAYPIXELCONFIGURATION_H_ */
