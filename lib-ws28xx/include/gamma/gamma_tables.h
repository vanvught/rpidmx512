/**
 * @file gamma_tables.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef GAMMA_TABLES_H_
#define GAMMA_TABLES_H_

#include <cstdint>

#include "gamma10offset0.h"
#include "gamma20offset0.h"
#include "gamma21offset0.h"
#include "gamma22offset0.h"
#include "gamma23offset0.h"
#include "gamma24offset0.h"
#include "gamma25offset0.h"
#include "gamma25offset5.h"

#include "pixeltype.h"

namespace gamma {
static constexpr auto MIN = 20U; ///< 2.0
static constexpr auto MAX = 25U; ///< 2.5

inline static const uint8_t *get_table_default(const pixel::Type type) {
	if (type == pixel::Type::WS2801) {
		return gamma25_0;
	}

	if ((type == pixel::Type::APA102) || (type == pixel::Type::SK9822)) {
		return gamma25_5;
	}

	if (type == pixel::Type::P9813) {
		return gamma10_0;
	}

	return gamma22_0;
}

inline static const uint8_t *get_table(const uint32_t nValue) {
	if ((nValue < MIN) || (nValue > MAX)) {
		return gamma10_0;
	}

	switch (nValue) {
	case 20:
		return gamma20_0;
		break;
	case 21:
		return gamma21_0;
		break;
	case 22:
		return gamma22_0;
		break;
	case 23:
		return gamma23_0;
		break;
	case 24:
		return gamma24_0;
		break;
	case 25:
		return gamma25_0;
		break;
	default:
		return gamma10_0;
		break;
	}
}

}  // namespace gamma

#endif /* GAMMA_TABLES_H_ */
