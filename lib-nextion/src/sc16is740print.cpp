/**
 * @file sc16is750print.cpp
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

#include "sc16is740.h"

#include "debug.h"

void SC16IS740::Print(void) {
	const uint8_t nRegisterLCR = RegRead(SC16IS7X0_LCR);
	RegWrite(SC16IS7X0_LCR, nRegisterLCR | LCR_ENABLE_DIV);
	const uint32_t nDivisor = RegRead(SC16IS7X0_DLL) | (RegRead(SC16IS7X0_DLH) << 8);
	RegWrite(SC16IS7X0_LCR, nRegisterLCR); // Restore LCR

	uint32_t nPrescaler;

	if ((RegRead(SC16IS7X0_MCR) & MCR_PRESCALE_4) == MCR_PRESCALE_4) {
		nPrescaler = 4;
	} else {
		nPrescaler = 1;
	}

	const uint32_t nActualBaudrate = (m_nOnBoardCrystal / nPrescaler) / (16 * nDivisor);

	printf("SC16IS740\n");
	printf(" %u Hz\n", m_nOnBoardCrystal);
	printf(" %d Baud\n", nActualBaudrate);
}
