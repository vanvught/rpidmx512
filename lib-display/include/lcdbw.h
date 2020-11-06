/**
 * @file lcdbw.h
 *
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

#ifndef BWLCD_H_
#define BWLCD_H_

#include <stdint.h>

#include "displayset.h"

#include "hal_i2c.h"

#define BW_LCD_DEFAULT_SLAVE_ADDRESS	0x82
#define BW_UI_DEFAULT_SLAVE_ADDRESS		0x94

class LcdBw final: public DisplaySet {
public:
	LcdBw ();
	LcdBw (uint8_t, uint8_t);
	LcdBw (uint8_t, uint8_t, uint8_t);

	bool Start() override;

	void Cls() override;
	void ClearLine(uint8_t) override;

	void PutChar(int) override;
	void PutString(const char *) override;

	void Text(const char *, uint8_t);
	void TextLine(uint8_t, const char *, uint8_t) override;

	void SetCursorPos(uint8_t, uint8_t) override;
	void SetCursor(uint32_t) override;

private:
	void Write(const char *, uint32_t);

private:
	HAL_I2C m_I2C;
	uint32_t m_nWriteMicros{0};
};

#endif /* BWLCD_H_ */
