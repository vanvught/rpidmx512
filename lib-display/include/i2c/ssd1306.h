/**
 * @file ssd1306.h
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>

#include "displayset.h"

#include "hal_i2c.h"

#define OLED_I2C_SLAVE_ADDRESS_DEFAULT	0x3C

enum TOledPanel {
	OLED_PANEL_128x64_8ROWS,	///< Default
	OLED_PANEL_128x64_4ROWS,
	OLED_PANEL_128x32_4ROWS
};

class Ssd1306 final: public DisplaySet {
public:
	Ssd1306 ();
	Ssd1306 (TOledPanel);
	Ssd1306 (uint8_t, TOledPanel);
	~Ssd1306() override {
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
		delete[] m_pShadowRam;
		m_pShadowRam = nullptr;
#endif
	}

	bool Start() override;

	void Cls() override;
	void ClearLine(uint32_t nLine) override;

	void PutChar(int) override;
	void PutString(const char *) override;

	void Text(const char *pData, uint32_t nLength);
	void TextLine(uint32_t nLine, const char *pData, uint32_t nLength) override;

	void SetCursorPos(uint32_t nCol, uint32_t nRow) override;
	void SetCursor(uint32_t) override;

	void SetSleep(bool bSleep) override;
	void SetContrast(uint8_t nContrast) override;

	void SetFlipVertically(bool doFlipVertically) override;

	void PrintInfo() override;

	bool IsSH1106() {
		return m_bHaveSH1106;
	}

	static Ssd1306* Get() {
		return s_pThis;
	}

private:
	void CheckSH1106();
	void InitMembers();
	void SendCommand(uint8_t);
	void SendData(const uint8_t *pData, uint32_t nLength);

	void SetCursorOn();
	void SetCursorOff();
	void SetCursorBlinkOn();
	void SetColumnRow(uint8_t nColumn, uint8_t nRow);

	void DumpShadowRam();

private:
	HAL_I2C m_I2C;
	TOledPanel m_OledPanel { OLED_PANEL_128x64_8ROWS };
	bool m_bHaveSH1106 { false };
	uint32_t m_nPages;
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE) || defined(CONFIG_DISPLAY_FIX_FLIP_VERTICALLY)
	char *m_pShadowRam { nullptr };
	uint32_t m_nShadowRamIndex { 0 };
#endif
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
	uint32_t m_nCursorMode { display::cursor::OFF };
	uint8_t m_nCursorOnChar;
	uint8_t m_nCursorOnCol;
	uint8_t m_nCursorOnRow;
#endif
	static Ssd1306 *s_pThis;
};

#endif /* SSD1306_H_ */
