/**
 * @file midibpm.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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
	bool Get(uint32_t nTimeStamp, uint32_t &nBPM) {
		m_nBpmDelta[m_nBpmClockCounter] = nTimeStamp - m_nBpmTimeStampPrevious;

		if (m_nBpmDelta[m_nBpmClockCounter] == 0) {
			return false;
		}

		m_nBpmTimeStampPrevious = nTimeStamp;

		if (++m_nBpmClockCounter == 24) {
			m_nBpmClockCounter = 0;

			uint32_t nDelta = 0;
			uint32_t nCount = 0;

			for (uint32_t i = 1; i < 24; i++) {
				const uint32_t nDiff = m_nBpmDelta[i] - m_nBpmDelta[i - 1];

				if (nDiff <= 1) {
					nDelta += m_nBpmDelta[i];
					nCount++;
				}
			}

			if (nCount == 0) {
				return false;
			}

			const auto fBPM = (25000.0f * static_cast<float>(nCount)) / static_cast<float>(nDelta);	// 25000 = 600000 / 24
			nBPM = static_cast<uint32_t>(fBPM + .5);

			if (nBPM != m_nBpmPrevious) {
				m_nBpmPrevious = nBPM;
				return true;
			}
		}

		return false;
	}

private:
	uint32_t m_nBpmPrevious { 0 };
	uint32_t m_nBpmTimeStampPrevious { 0 };
	uint32_t m_nBpmClockCounter { 0 };
	uint32_t m_nBpmDelta[24];
};

#endif /* MIDIBPM_H_ */
