/**
 * @file buttonsbw.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef BUTTONSBW_H_
#define BUTTONSBW_H_

#include <stdint.h>

#include "inputset.h"
#include "hal_i2c.h"

#define BW_UI_DEFAULT_SLAVE_ADDRESS		0x94

class ButtonsBw: public InputSet {
public:
	ButtonsBw();
	~ButtonsBw() override;

	bool Start() override;

	bool IsAvailable() override;
	int GetChar() override;

private:
	void Write(const char *, uint32_t);

private:
	HAL_I2C m_I2C;
	uint32_t m_nWriteMicros{0};
	uint8_t m_nButtons{0};
};

#endif /* BUTTONSBW_H_ */
