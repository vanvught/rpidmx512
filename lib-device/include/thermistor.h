/**
 * @file thermistor.h
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef THERMISTOR_H_
#define THERMISTOR_H_

#include <cmath>

namespace sensor {
namespace thermistor {
// https://www.adafruit.com/product/372
static constexpr auto THERMISTOR_NOMINAL  = 10000U;		// 10K Ohm
static constexpr auto TEMPERATURE_NOMINAL = 25.0f; 		// 25 degrees Celcius
static constexpr auto BCOEFFICIENT = 3950U;				// The beta coefficient of the thermistor (usually 3000-4000)
static constexpr auto RANGE_MIN = -55;
static constexpr auto RANGE_MAX = 125;
static constexpr char DESCRIPTION[] = "Ambient Temperature";

inline static float temperature(const uint32_t resistor) {
	// https://en.wikipedia.org/wiki/Steinhart–Hart_equation
	// https://learn.adafruit.com/thermistor/using-a-thermistor
	float steinhart = static_cast<float>(resistor) / THERMISTOR_NOMINAL;	// (R/Ro)
	steinhart = logf(steinhart);											// ln(R/Ro)
	steinhart /= BCOEFFICIENT;												// 1/B * ln(R/Ro)
	steinhart += (1.0f / (TEMPERATURE_NOMINAL + 273.15f));					// + (1/To)
	steinhart = (1.0f / steinhart);											// Invert
	steinhart -= 273.15f;													// convert absolute temp to C
	return steinhart;
}

}  // namespace thermistor
}  // namespace sensor

#endif /* INCLUDE_THERMISTOR_H_ */
