/**
 * @file utc.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef UTC_H_
#define UTC_H_

#include <cstdint>

#include "debug.h"

// https://en.wikipedia.org/wiki/List_of_UTC_time_offsets

namespace hal {
enum class UtcOffset {
	UTC_OFFSET_MIN = -12,
	UTC_OFFSET_MAX = 14
};

inline int32_t utc_validate(const float fOffset) {
	static constexpr float s_ValidOffets[] = { -9.5, -3.5, 3.5, 4.5, 5.5, 5.75, 6.5, 8.75, 9.5, 10.5, 12.75 };
	auto nInt = static_cast<int32_t>(fOffset);

	if ((nInt >= -12) && (nInt <= 14)) {
		if (fOffset == static_cast<float>(nInt)) {
			return (nInt * 3600);
		} else {
			for (uint32_t i = 0; i < sizeof(s_ValidOffets) / sizeof(s_ValidOffets[0]); i++) {
				if (fOffset == s_ValidOffets[i]) {
					return static_cast<int32_t>(fOffset * 3600.0f);
				}
			}
		}
	}

	return 0;
}

inline bool utc_validate(const int8_t nHours, const uint8_t nMinutes, int32_t& nUtcOffset) {
    struct Offset {
        int8_t nHours;
        uint8_t nMinutes;
    };
    // https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
    static constexpr Offset s_ValidOffsets[] = { {-9, 30}, {-3, 30},
    		{3, 30}, {4, 30}, {5, 30}, {5, 45}, {6, 30}, {8, 45}, {9, 30}, {10, 30}, {12, 45} };
    constexpr int8_t UTC_OFFSET_MIN = -12;
    constexpr int8_t UTC_OFFSET_MAX = 14;

    // Check if nHours is within valid range
    if (nHours >= UTC_OFFSET_MIN && nHours <= UTC_OFFSET_MAX) {
        // Check if minutes are 0, meaning a whole hour offset
        if (nMinutes == 0) {
        	nUtcOffset = nHours * 3600;
        	return true;
        } else {
            for (const auto& offset : s_ValidOffsets) {
                if (nHours == offset.nHours && nMinutes == offset.nMinutes) {
                	nUtcOffset = (nHours * 3600);
                	if (nHours > 0) {
                		nUtcOffset = nUtcOffset + (nMinutes * 60);
                	} else {
                		nUtcOffset = nUtcOffset - (nMinutes * 60);
                	}
                	return true;
                }
            }
        }
    }

    // Return false if validation fails
    return false;
}
}  // namespace hal

#endif /* UTC_H_ */
