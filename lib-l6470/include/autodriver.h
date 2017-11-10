/**
 * @file autodriver.h
 *
 */
/*
 * Based on https://github.com/sparkfun/L6470-AutoDriver/tree/master/Libraries/Arduino
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

#ifndef AUTODRIVER_H_
#define AUTODRIVER_H_

#include <stdint.h>

#include "l6470.h"

class AutoDriver: public L6470 {
public:
	AutoDriver(uint8_t, uint8_t, uint8_t, uint8_t);
	AutoDriver(uint8_t, uint8_t, uint8_t);

	~AutoDriver(void);

	int busyCheck(void);

private:
	uint8_t SPIXfer(uint8_t);

	/*
	 * Additional methods
	 */
public:
	bool IsConnected(void) const;

	inline static uint16_t getNumBoards(void) {
		int n = 0;
		for (int i = 0; i < (int) (sizeof(m_nNumBoards) / sizeof(m_nNumBoards[0])); i++) {
			n += m_nNumBoards[i];
		}
		return n;
	}

	inline static uint8_t getNumBoards(int cs) {
		if (cs < (int) (sizeof(m_nNumBoards) / sizeof(m_nNumBoards[0]))) {
			return m_nNumBoards[cs];
		} else {
			return 0;
		}
	}

	inline int getMotorNumber(void) {
		return m_nMotorNumber;
	}

	void setMotorNumber(int nMotorNumber) {
		m_nMotorNumber = nMotorNumber;
	}

private:
	uint8_t m_nSpiChipSelect;
	uint8_t m_nResetPin;
	uint8_t m_nBusyPin;
	uint8_t m_nPosition;
	bool m_bIsBusy;
	static uint8_t m_nNumBoards[2];
	bool m_bIsConnected;
};

#endif /* AUTODRIVER_H_ */
