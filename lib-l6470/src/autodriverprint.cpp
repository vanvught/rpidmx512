/**
 * @file autodriverprint.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdio.h>

#include "autodriver.h"

void AutoDriver::Print() {
	printf("SparkFun AutoDriver [%d]\n", m_nMotorNumber);
	printf(" Position=%d, ChipSelect=%d, ResetPin=%d, BusyPin=%d [%s]\n", m_nPosition, m_nSpiChipSelect, m_nResetPin, m_nBusyPin, m_nBusyPin == 0xFF ? "SPI" : "GPIO");
	printf(" MinSpeed=%3.0f, MaxSpeed=%3.0f, Acc=%4.0f, Dec=%4.0f\n", getMinSpeed(), getMaxSpeed(), getAcc(), getDec());
	printf(" AccKVAL=%d, DecKVAL=%d, RunKVAL=%d, HoldKVAL=%d\n",
			static_cast<int>(getAccKVAL()), static_cast<int>(getDecKVAL()),
			static_cast<int>(getRunKVAL()), static_cast<int>(getHoldKVAL()));
	printf(" MicroSteps=%u, SwitchMode=%d\n",
			static_cast<unsigned>(1) << getStepMode(), getSwitchMode());
}
