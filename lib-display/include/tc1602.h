/**
 * @file tc1602.h
 *
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef TC1602_H_
#define TC1602_H_

#include <cstdint>

#include "displayset.h"

#include "hal_i2c.h"

#define TC1602_I2C_DEFAULT_SLAVE_ADDRESS	0x27

class Tc1602 final: public DisplaySet {
public:
	Tc1602 ();
	Tc1602 (uint8_t nCols, uint8_t nRows);
	Tc1602 (uint8_t nSlaveAddress, uint8_t nCols, uint8_t nRows);

	bool Start() override;

	void Cls() override;
	void ClearLine(uint8_t nLine) override;

	void PutChar(int) override;
	void PutString(const char *) override;

	void Text(const char *pData, uint32_t nLength);
	void TextLine(uint8_t nLine, const char *pData, uint32_t nLength) override;

	void SetCursorPos(uint8_t nCol, uint8_t nRow) override;
	void SetCursor(uint32_t) override;

private:
	void Write4bits(uint8_t);
	void WriteCmd(uint8_t);
	void WriteReg(uint8_t);

private:
	HAL_I2C m_I2C;
};

#endif /* TC1602_H_ */
