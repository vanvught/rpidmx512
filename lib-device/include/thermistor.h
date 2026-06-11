/**
 * @file thermistor.h
 *
 */
/* Copyright (C) 2023-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cmath>

namespace sensor::thermistor {
// https://www.adafruit.com/product/372
inline constexpr auto kThermistorNominal = 10000U; // 10K Ohm
inline constexpr auto kTemperatureNominal = 25.0f; // 25 degrees Celcius
inline constexpr auto kBcoefficient = 3950U;       // The beta coefficient of the thermistor (usually 3000-4000)
inline constexpr int16_t kRangeMin = -55;
inline constexpr int16_t kRangeMax = 125;
inline constexpr char kDescription[] = "Ambient Temperature";

inline float Temperature(uint32_t resistor) {
    // https://en.wikipedia.org/wiki/Steinhart–Hart_equation
    // https://learn.adafruit.com/thermistor/using-a-thermistor
    float steinhart = static_cast<float>(resistor) / kThermistorNominal; // (R/Ro)
    steinhart = logf(steinhart);                                         // ln(R/Ro)
    steinhart /= kBcoefficient;                                          // 1/B * ln(R/Ro)
    steinhart += (1.0f / (kTemperatureNominal + 273.15f));               // + (1/To)
    steinhart = (1.0f / steinhart);                                      // Invert
    steinhart -= 273.15f;                                                // convert absolute temp to C
    return steinhart;
}
} // namespace sensor::thermistor

#endif // THERMISTOR_H_
