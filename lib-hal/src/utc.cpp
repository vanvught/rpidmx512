/**
 * @file utc.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "utc.h"

// https://en.wikipedia.org/wiki/List_of_UTC_time_offsets

static const float s_ValidOffets[] = { -9.5, -3.5, 3.5, 4.5, 5.5, 5.75, 6.5, 8.75, 9.5, 10.5, 12.75 };

int32_t Utc::Validate(float fOffset) {
	auto nInt = static_cast<int32_t>(fOffset);

	if ((nInt >= -12) && (nInt <= 14)) {
		if (fOffset == static_cast<float>(nInt)) {
			return (nInt * 3600);
		} else {
			for (uint32_t i = 0; i < sizeof(s_ValidOffets) / sizeof(s_ValidOffets[0]); i++) {
				if (fOffset == s_ValidOffets[i]) {
					return (fOffset * 3600);
				}
			}
		}
	}

	return 0;
}
