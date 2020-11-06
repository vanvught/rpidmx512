/**
 * @file ssd1311.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SDD1311_H_
#define SDD1311_H_

#include <stdint.h>

#include "displayset.h"
#include "hal_i2c.h"

class Ssd1311 final: public DisplaySet {
public:
	Ssd1311 ();
	~Ssd1311 () override;

	bool Start() override;

	void Cls() override;
	void ClearLine(uint8_t) override;

	void PutChar(int) override;
	void PutString(const char *) override;

	void Text(const char *, uint8_t);
	void TextLine(uint8_t, const char *, uint8_t) override;

	void SetSleep(bool bSleep) override;

	void SetCursorPos(uint8_t, uint8_t) override;
	void SetCursor(uint32_t) override;

	void PrintInfo() override;

	static Ssd1311* Get() {
		return s_pThis;
	}

private:
	bool CheckSSD1311();
	void SelectRamRom(uint32_t nRam, uint32_t nRom);
	void SetDDRAM(uint8_t nAddress);
	void SetCGRAM(uint8_t nAddress);
	void SendCommand(uint8_t nCommand);
	void SendData(uint8_t nData);
	void SendData(const uint8_t *, uint32_t);

private:
	HAL_I2C m_I2C;
	uint8_t m_nDisplayControl{1U << 3}; // Section 9.1.4 Display ON/OFF Control

	static Ssd1311 *s_pThis;
};

#endif /* SDD1311_H_ */
