/**
 * @file mcp3424.cpp
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

#include <cstdio>
#include <cmath>
#include <unistd.h>
#include <time.h>

#include "bcm2835.h"

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

static constexpr uint32_t R_GND  =  6800;	// 6K8
static constexpr uint32_t R_HIGH = 10000;	// 10K
static constexpr uint32_t R_ADDED[4] = { 12000, 10000, 8200, 4700 }; // 12K, 10K, 8K2, 4K7

static double voltage(const double vout, const uint32_t r = 0) {
	const double vin = vout * ((static_cast<double>(R_GND + R_HIGH + r) / static_cast<double>(R_GND)));
	return vin;
}

static uint32_t resistor(const double vout) {
	const double d = (5 * R_GND) / vout;
	return static_cast<uint32_t>(d) - R_GND;
}

int main(int argc, char **argv) {
	if (getuid() != 0) {
		fprintf(stderr, "Error: Not started with 'root'\n");
		return -1;
	}

	if (bcm2835_init() != 1) {
		fprintf(stderr, "bcm2835_init() failed\n");
		return -2;
	}

	if (bcm2835_i2c_begin() != 1) {
		fprintf(stderr, "bcm2835_i2c_begin() failed\n");
		return -3;
	}

	MCP3424 adc1(0x68);
	MCP3424 adc2(0x69);

//	adc1.SetResolution(adc::mcp3424::Resolution::SAMPLE_14BITS);
//	adc2.SetResolution(adc::mcp3424::Resolution::SAMPLE_14BITS);

	if (adc1.IsConnected() || adc2.IsConnected()) {
		auto nPrevSeconds = 60; // Force initial update

		for (;;) {
			const auto ltime = time(nullptr);
			const auto tm = localtime(&ltime);

			if (tm->tm_sec != nPrevSeconds) {
				nPrevSeconds = tm->tm_sec;
				printf("\033c%s", asctime (tm));

				if (adc1.IsConnected()) {
					for (uint32_t nChannel = 0; nChannel < 4; nChannel++) {
						const auto adcValue = adc1.GetRaw(nChannel);
						const auto vRef = adc1.GetVoltage(nChannel);
						const auto v = voltage(vRef);
						const auto r = resistor(vRef) - R_HIGH;
						const auto t = sensor::thermistor::temperature(r);
						printf("%u:%u 0x%04x %1.3fV %1.1fV %uR -> %3.1fC\n", 1 + nChannel, nChannel, adcValue, vRef, v, r, t);
					}
				}

				if (adc2.IsConnected()) {
					for (uint32_t nChannel = 0; nChannel < 4; nChannel++) {
						const auto adcValue = adc2.GetRaw(nChannel);
						const auto vRef = adc2.GetVoltage(nChannel);
						const auto v = voltage(vRef, R_ADDED[nChannel]);
						const auto r = resistor(vRef);
						const auto t = sensor::thermistor::temperature(r);
						printf("%u:%u 0x%04x %1.3fV %1.1fV %uR -> %3.1fC\n", 5 + nChannel, nChannel, adcValue, vRef, v, r, t);
					}
				}
			}
		}
	} else {
		puts("Not connected.");
	}

	return 0;
}
