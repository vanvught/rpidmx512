/**
 * @file pwmled.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>

#include "pwmled.h"

#ifndef MIN
#  define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#define MAX_12BIT	(0xFFF)
#define MAX_PCT		(100)
#define MAX_DMX		(0xFF)

PWMLed::PWMLed(uint8_t nAddress): PCA9685(nAddress) {
	SetFrequency(PWMLED_DEFAULT_FREQUENCY);
}

PWMLed::~PWMLed(void) {
}

void PWMLed::Set(uint8_t nChannel, uint16_t nData) {
	nData = MIN(nData, MAX_12BIT);

	if (nData == MAX_12BIT) {
		SetFullOn(nChannel, true);
	} else if (nData == 0) {
		SetFullOff(nChannel, true);
	} else {
		Write(nChannel, nData);
	}
}

void PWMLed::SetPct(uint8_t nChannel, uint8_t nPercentage) {
	nPercentage = MIN(nPercentage, MAX_PCT);

	if (nPercentage == MAX_PCT) {
		SetFullOn(nChannel, true);
	} else if (nPercentage == 0) {
		SetFullOff(nChannel, true);
	} else {
		const uint16_t nData = (uint16_t) (.5 + ((float) MAX_12BIT / MAX_PCT) * nPercentage);
		Write(nChannel, nData);
	}
}

void PWMLed::SetDmx(uint8_t nChannel, uint8_t nDmx) {

	if (nDmx == MAX_DMX) {
		SetFullOn(nChannel, true);
	} else if (nDmx == 0) {
		SetFullOff(nChannel, true);
	} else {
		const uint16_t nData =  (uint16_t) ((MAX_12BIT / MAX_DMX) * nDmx);
		Write(nChannel, nData);
	}
}
