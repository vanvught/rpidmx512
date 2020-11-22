/**
 * @file rgbpanelconst.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RGBPANELCONST_H_
#define RGBPANELCONST_H_

#include <stdint.h>

namespace rgbpanel {
enum class Types {
	HUB75,
	FM6126A,
	FM6127,
	UNDEFINED
};
namespace defaults {
static constexpr auto COLS = 32;
static constexpr auto ROWS = 32;
static constexpr auto CHAIN = 1;
static constexpr auto TYPE = Types::HUB75;
}  // namespace defaults
namespace config {
static constexpr auto COLS = 2;
static constexpr auto ROWS = 4;
}  // namespace max
namespace type {
static constexpr auto MAX_NAME_LENGTH = 7 + 1;  	// + '\0'
}  // namespace type
}  // namespace rgbpanel

struct RgbPanelConst {
	static const char TYPE[static_cast<unsigned>(rgbpanel::Types::UNDEFINED)][rgbpanel::type::MAX_NAME_LENGTH];

	static const uint32_t COLS[rgbpanel::config::COLS];
	static const uint32_t ROWS[rgbpanel::config::ROWS];
};

#endif /* RGBPANELCONST_H_ */
