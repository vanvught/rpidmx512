/**
 * @file simple.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "pca9685.h"

int main(int argc, char **argv) {
	if (getuid() != 0) {
		fprintf(stderr, "Program is not started as \'root\' (sudo)\n");
		return -1;
	}

	uint16_t OnValue, OffValue;
	PCA9685 pca9685;

	pca9685.Dump();

	pca9685.SetFrequency(100);

	pca9685.SetFullOn(CHANNEL(0), true); 					// Channel 0 Full On

	pca9685.Write(CHANNEL(1), VALUE(PCA9685_VALUE_MAX/2));	// Channel 1, Duty Cycle = 50 %
	pca9685.Write(CHANNEL(2), VALUE(819)); 					// Channel 2, Duty Cycle = 20 %
															// Max value = 4096. 20 % = 819.2 ~ 819 counts
	pca9685.SetFullOff(CHANNEL(3), true); 					// Channel 0 Full Off

	pca9685.Dump();

	printf("\nOutput (%dHz):\n", (int) pca9685.GetFrequency());

	pca9685.Read(CHANNEL(0), &OnValue, &OffValue);
	printf("\tChannel 0: %04xh-%04xh, Full On\n", OnValue, OffValue);

	pca9685.Read(CHANNEL(1), &OnValue, &OffValue);
	printf("\tChannel 1: %04xh-%04xh, Duty Cycle = 50%%\n", OnValue, OffValue);

	pca9685.Read(CHANNEL(2), &OnValue, &OffValue);
	printf("\tChannel 2: %04xh-%04xh, Duty Cycle = 20%%\n", OnValue, OffValue);

	pca9685.Read(CHANNEL(3), &OnValue, &OffValue);
	printf("\tChannel 3: %04xh-%04xh, Full Off\n", OnValue, OffValue);

	for(;;) {

	}

	return 0;
}
