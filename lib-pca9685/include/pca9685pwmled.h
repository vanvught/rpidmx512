/**
 * @file pca9685pwmled.h
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PCA9685PWMLED_H_
#define PCA9685PWMLED_H_

#include <cstdint>

#include "pca9685.h"


namespace pca9685::pwmled {
static constexpr uint32_t DEFAULT_FREQUENCY = 120;
} // namespace pca9685::pwmled


class PCA9685PWMLed: public PCA9685 {
public:
	PCA9685PWMLed(const uint8_t nAddress): PCA9685(nAddress) {
		SetFrequency(pca9685::pwmled::DEFAULT_FREQUENCY);
	}

	void Set(const uint32_t nChannel, const uint16_t nData) {
		if (nData >= 0xFFF) {
			SetFullOn(nChannel, true);
		} else if (nData == 0) {
			SetFullOff(nChannel, true);
		} else {
			Write(nChannel, nData);
		}
	}

	void Set(const uint32_t nChannel, const uint8_t nData) {
		if (nData == 0xFF) {
			SetFullOn(nChannel, true);
		} else if (nData == 0) {
			SetFullOff(nChannel, true);
		} else {
			const auto nValue = static_cast<uint16_t>((nData << 4) | (nData >> 4));
			Write(nChannel, nValue);
		}
	}
};

#endif /* PCA9685PWMLED_H_ */
