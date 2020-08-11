/**
 * @file autodriver.h
 *
 */
/*
 * Based on https://github.com/sparkfun/L6470-AutoDriver/tree/master/Libraries/Arduino
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef AUTODRIVER_H_
#define AUTODRIVER_H_

#include <stdint.h>

#include "l6470.h"

class AutoDriver: public L6470 {
public:
	AutoDriver(uint8_t, uint8_t, uint8_t, uint8_t);
	AutoDriver(uint8_t, uint8_t, uint8_t);

	~AutoDriver() override;

	int busyCheck() override;

	void Print();

private:
	uint8_t SPIXfer(uint8_t) override;

	/*
	 * Additional methods
	 */
public:
	bool IsConnected();

	unsigned getMotorNumber() {
		return m_nMotorNumber;
	}

	void setMotorNumber(unsigned nMotorNumber) {
		m_nMotorNumber = nMotorNumber;
	}

	static uint16_t getNumBoards();
	static uint8_t getNumBoards(uint8_t cs);

private:
	uint8_t m_nSpiChipSelect;
	uint8_t m_nResetPin;
	uint8_t m_nBusyPin;
	uint8_t m_nPosition;
	bool m_bIsBusy;

	static uint8_t m_nNumBoards[2];
};

#endif /* AUTODRIVER_H_ */
