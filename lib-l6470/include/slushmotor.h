/**
 * @file slushmotor.h
 *
 */
/*
 * Based on https://github.com/Roboteurs/slushengine/tree/master/Slush
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "l6470.h"

class SlushMotor: public L6470 {
public:
	SlushMotor(unsigned, bool bUseSPI = true);
	~SlushMotor() override;

	int busyCheck() override;

	/*
	 * Roboteurs Slushengine Phyton compatible methods
	 */
	int isBusy();

	void setOverCurrent(unsigned int);
	void setAsHome();

	void softFree();
	void free();

	/*
	 * Additional methods
	 */
	bool GetUseSpiBusy() const;
	void SetUseSpiBusy(bool);
	bool IsConnected() const;

private:
	uint8_t SPIXfer(uint8_t) override;

private:
	unsigned m_nSpiChipSelect;
	unsigned m_nBusyPin;
	bool m_bUseSpiBusy;
	bool m_bIsBusy;
	bool m_bIsConnected;
};

#endif /* SLUSHMOTOR_H_ */
