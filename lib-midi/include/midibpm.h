/**
 * @file midibpm.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef MIDIBPM_H_
#define MIDIBPM_H_

#include <cstdint>

class MidiBPM {
public:
	bool Get(uint32_t timestamp, uint32_t &bpm) {
		delta_[clock_counter_] = timestamp - timestamp_previous_;

		if (delta_[clock_counter_] == 0) {
			return false;
		}

		timestamp_previous_ = timestamp;

		if (++clock_counter_ == 24) {
			clock_counter_ = 0;

			uint32_t delta = 0;
			uint32_t count = 0;

			for (uint32_t i = 1; i < 24; i++) {
				const uint32_t kDiff = delta_[i] - delta_[i - 1];

				if (kDiff <= 1) {
					delta += delta_[i];
					count++;
				}
			}

			if (count == 0) {
				return false;
			}

			const auto kBpm = (25000.0f * static_cast<float>(count)) / static_cast<float>(delta);	// 25000 = 600000 / 24
			bpm = static_cast<uint32_t>(kBpm + .5);

			if (bpm != previous_) {
				previous_ = bpm;
				return true;
			}
		}

		return false;
	}

private:
	uint32_t previous_ { 0 };
	uint32_t timestamp_previous_ { 0 };
	uint32_t clock_counter_ { 0 };
	uint32_t delta_[24];
};

#endif  // MIDIBPM_H_
