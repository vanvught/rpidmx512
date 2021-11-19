/**
 * @file propertiesconfig.h
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

#ifndef PROPERTIESCONFIG_H_
#define PROPERTIESCONFIG_H_

#include <cstdint>

#if defined(ENABLE_JSON_ONLY) && defined(DISABLE_JSON)
# error
#endif

struct PropertiesConfig {
public:
	static void EnableJSON(bool enableJSON) {
		if (enableJSON) {
			s_Config |= Mask::ENABLE_JSON;
		} else {
			s_Config &= static_cast<uint8_t>(~Mask::ENABLE_JSON);
		}
	}

	static bool IsJSON() {
#if defined(ENABLE_JSON_ONLY)
		return true;
#elif defined(DISABLE_JSON)
        return false;
#else
		return isMaskSet(Mask::ENABLE_JSON);
#endif
	}

private:
	struct Mask {
		static constexpr uint8_t ENABLE_JSON = (1U << 0);
	};
    static bool isMaskSet(uint8_t nMask) {
    	return (s_Config & nMask) == nMask;
    }

    static uint8_t s_Config;
};

#endif /* PROPERTIESCONFIG_H_ */
