/**
 * @file mcp3424.cpp
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

#include <cstdio>
#include <cmath>
#include <unistd.h>
#include <time.h>

#include "mcp3424.h"
#include "thermistor.h"

/**
 * The R values are based on:
 * https://www.abelectronics.co.uk/p/69/adc-pi-raspberry-pi-analogue-to-digital-converter
 *
 * Channel 1 -> 12K -> 5V0
 * Channel 2 -> 10K -> 5V0
 * Channel 3 -> 8K2 -> 5V0
 * Channel 4 -> 4K7 -> 5V0
 * Channel 5 -> floating
 * Channel 6 -> 3V3
 * Channel 7 -> floating
 * Channel 8 -> 5V0
 */

static constexpr uint32_t kRGnd = 6800;                            // 6K8
static constexpr uint32_t kRHigh = 10000;                          // 10K
static constexpr uint32_t kRAdded[4] = {12000, 10000, 8200, 4700}; // 12K, 10K, 8K2, 4K7

static double Voltage(double vout, uint32_t r = 0) {
    const double kVin = vout * ((static_cast<double>(kRGnd + kRHigh + r) / static_cast<double>(kRGnd)));
    return kVin;
}

static uint32_t Resistor(double vout) {
    const double kD = (5 * kRGnd) / vout;
    return static_cast<uint32_t>(kD) - kRGnd;
}

int main(int argc, char** argv) { // NOLINT
    MCP3424 adc1(0x68);
    MCP3424 adc2(0x69);

    //	adc1.SetResolution(adc::mcp3424::Resolution::SAMPLE_14BITS);
    //	adc2.SetResolution(adc::mcp3424::Resolution::SAMPLE_14BITS);

    if (adc1.IsConnected() || adc2.IsConnected()) {
        auto prev_seconds = 60; // Force initial update

        for (;;) {
            const auto kTime = time(nullptr);
            const auto kTm = localtime(&kTime);

            if (kTm->tm_sec != prev_seconds) {
                prev_seconds = kTm->tm_sec;
                printf("\033c%s", asctime(kTm));

                if (adc1.IsConnected()) {
                    for (uint32_t channel = 0; channel < 4; channel++) {
                        const auto adcValue = adc1.GetRaw(channel);
                        const auto vRef = adc1.GetVoltage(channel);
                        const auto v = Voltage(vRef);
                        const auto r = Resistor(vRef) - kRHigh;
                        const auto t = sensor::thermistor::Temperature(r);
                        printf("%u:%u 0x%04x %1.3fV %1.1fV %uR -> %3.1fC\n", 1 + channel, channel, adcValue, vRef, v, r, t);
                    }
                }

                if (adc2.IsConnected()) {
                    for (uint32_t channel = 0; channel < 4; channel++) {
                        const auto adcValue = adc2.GetRaw(channel);
                        const auto vRef = adc2.GetVoltage(channel);
                        const auto v = Voltage(vRef, kRAdded[channel]);
                        const auto r = Resistor(vRef);
                        const auto t = sensor::thermistor::Temperature(r);
                        printf("%u:%u 0x%04x %1.3fV %1.1fV %uR -> %3.1fC\n", 5 + channel, channel, adcValue, vRef, v, r, t);
                    }
                }
            }
        }
    } else {
        puts("Not connected.");
    }

    return 0;
}
