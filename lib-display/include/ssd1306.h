/**
 * @file ssd1306.h
 *
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

#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdint.h>

#include "displayset.h"

#define OLED_I2C_SLAVE_ADDRESS_DEFAULT	0x3C

enum TOledPanel {
	OLED_PANEL_128x64_8ROWS,	///< Default
	OLED_PANEL_128x64_4ROWS,
	OLED_PANEL_128x32_4ROWS
};

class Ssd1306: public DisplaySet {
public:
	Ssd1306 (void);
	Ssd1306 (TOledPanel);
	Ssd1306 (uint8_t, TOledPanel);
	~Ssd1306 (void);

	bool Start(void);

	void Cls(void);
	void ClearLine(uint8_t);

	void PutChar(int);
	void PutString(const char *);

	void TextLine(uint8_t, const char *, uint8_t);
	void Text(const char *, uint8_t);

	void SetCursorPos(uint8_t, uint8_t);
	void SetCursor(TCursorMode);

	static Ssd1306 *Get (void);

private:
	void Setup(void);
	void InitMembers(void);
	void SendCommand(uint8_t);
	void SendData(const uint8_t *, uint32_t);

	void SetCursorOn(void);
	void SetCursorOff(void);
	void SetCursorBlinkOn(void);

#ifndef NDEBUG
	void DumpShadowRam(void);
#endif

private:
	uint8_t m_nSlaveAddress;
	TOledPanel m_OledPanel;
	TCursorMode m_tCursorMode;
	char *m_pShadowRam;
	uint16_t m_nShadowRamIndex;
	uint8_t m_nCursorOnChar;
	uint8_t m_nCursorOnCol;
	uint8_t m_nCursorOnRow;

	static Ssd1306 *s_pThis;
};


#endif /* SSD1306_H_ */
