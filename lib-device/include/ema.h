/**
 * @file ema.h
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

#ifndef EMA_H_
#define EMA_H_

#include <cstdint>

/**
 * @brief
 * The EMA class template implements an Exponential Moving Average (EMA) filter in a compact and efficient manner,
 * suitable for real-time applications where resource constraints may be a consideration.
 * The class uses unsigned integer arithmetic to achieve the filtering,
 * which is beneficial for systems that do not have floating-point units or where performance is critical.
 *
 * @tparam K
 * A compile-time constant that determines the weight of the EMA.
 * Higher K values mean a slower response to changes in the input, making the EMA smoother.
 */

template<uint8_t K>
class EMA {
public:
	EMA(uint16_t nIntitial = 0) : nState(static_cast<uint16_t>(nIntitial << K) - nIntitial) {}

	/**
	 * @brief
	 * This method allows resetting the internal state nState of the filter to a specific value nValue.
	 * It sets the state in the same way as the constructor,
	 * preparing the filter to start processing new inputs from the given value.
	 * @param nValue
	 */
	void Reset(const uint16_t nValue = 0) {
		nState = static_cast<uint16_t>(nValue << K) - nValue;
	}

	uint16_t Filter(const uint16_t nInput) {
		nState += nInput;
		const auto nOutput = static_cast<uint16_t>((nState + nHalf) >> K);
		nState -= nOutput;
		return nOutput;
	}

	/*
	 * This static constant is used for rounding during the shift operation.
	 * It ensures that when you shift the value right by K bits,
	 * it rounds to the nearest integer rather than truncating.
	 */
	static constexpr uint16_t nHalf = K > 0 ? 1 << (K - 1) : 0;

private:
	uint16_t nState;
};

#endif /* EMA_H_ */
