/**
 * @file slushmotor.h
 *
 */
/*
 * Based on https://github.com/Roboteurs/slushengine/tree/master/Slush
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

#ifndef SLUSHMOTOR_H_
#define SLUSHMOTOR_H_

#include <stdint.h>
#include <stdbool.h>

#include "l6470.h"

class SlushMotor: public L6470 {
public:
	SlushMotor(int, bool bUseSPI = true);
	~SlushMotor(void);

	int busyCheck(void);

	/*
	 * Roboteurs Slushengine Phyton compatible methods
	 */
	int isBusy(void);

	void setOverCurrent(unsigned int);
	void setAsHome(void);

	void softFree(void);
	void free(void);

	/*
	 * Additional methods
	 */
	bool GetUseSpiBusy(void) const;
	void SetUseSpiBusy(bool);
	bool IsConnected(void) const;

private:
	uint8_t SPIXfer(uint8_t);

private:
	int m_nSpiChipSelect;
	int m_nBusyPin;
	bool m_bUseSpiBusy;
	bool m_bIsBusy;
	bool m_bIsConnected;
};

#endif /* SLUSHMOTOR_H_ */
