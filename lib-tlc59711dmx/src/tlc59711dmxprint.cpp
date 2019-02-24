/**
 * @file tlc59711dmxprint.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "tlc59711dmx.h"
#include "tlc59711dmxparams.h"

void TLC59711Dmx::Print(void) {
	printf("PWM parameters\n");
	printf(" Type  : %s [%d]\n", TLC59711DmxParams::GetLedTypeString(m_LEDType), m_LEDType);
	printf(" Count : %d\n", (int) m_nLEDCount);
	printf(" Clock : %d Hz %s {Default: %d Hz, Maximum %d Hz}\n", (int) m_nSpiSpeedHz, (m_nSpiSpeedHz == 0 ? "Default" : ""), TLC59711_SPI_SPEED_DEFAULT, TLC59711_SPI_SPEED_MAX);
}

