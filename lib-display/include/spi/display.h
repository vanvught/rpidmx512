/**
 * @file display.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SPI_DISPLAY_H_
#define SPI_DISPLAY_H_

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cassert>

#if defined (CONFIG_USE_ILI9341)
# include "spi/ili9341.h"
#else
# include "spi/st7789.h"
#endif

#include "hardware.h"

class Display {
public:
	Display();
	~Display() {
		s_pThis = nullptr;
	}

	void Cls(void);
	void SetCursorPos(uint32_t nCol, uint32_t nRow);
	void PutChar(int c);

	void PutString(const char *p) {
		for (uint32_t i = 0; *p != '\0'; i++) {
			PutChar(static_cast<int>(*p));
			p++;
		}
	}

	void ClearLine(uint32_t nLine) {
		assert(nLine > 0);

		if (__builtin_expect((!(nLine <= m_nRows)), 0)) {
			return;
		}

		SetCursorPos(0, (nLine - 1U));

		for (uint32_t i = 0; i < m_nCols; i++) {
			PutChar(' ');
		}

		SetCursorPos(0, (nLine - 1U));
	}

	void TextLine(uint32_t nLine, const char *pText, uint32_t nLength) {
		assert(nLine > 0);

		if (__builtin_expect((!(nLine <= m_nRows)), 0)) {
			return;
		}

		SetCursorPos(0, (nLine - 1U));
		Text(pText, nLength);
	}

	void Status(Display7SegmentMessage nValue) {
		printf("Status: %u\n", static_cast<uint32_t>(nValue));
	}

	void Status(uint8_t nValue, bool bHex) {
		printf("Status: %u:%d\n", nValue, bHex);
	}

	void Text(const char *pData, uint32_t nLength) {
		DEBUG_ENTRY

		if (nLength > m_nCols) {
			nLength = m_nCols;
		}

		for (uint32_t i = 0; i < nLength; i++) {
			PutChar(pData[i]);
		}

		DEBUG_EXIT
	}

	int Write(uint32_t nLine, const char *pText) {
		const auto *p = pText;
		int nCount = 0;

		const auto nColumns = static_cast<int>(m_nCols);

		while ((*p != 0) && (nCount++ < nColumns)) {
			++p;
		}

		TextLine(nLine, pText, static_cast<uint8_t>(nCount));

		return nCount;
	}

	int Printf(uint8_t nLine, const char *format, ...) {
		char buffer[32];

		va_list arp;

		va_start(arp, format);

		auto i = vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, arp);

		va_end(arp);

		TextLine(nLine, buffer, static_cast<uint16_t>(i));

		return i;
	}

	void TextStatus(const char *pText) {
		SetCursorPos(0, static_cast<uint8_t>(m_nRows - 1));

		for (uint32_t i = 0; i < static_cast<uint32_t>(m_nCols - 1); i++) {
			PutChar(' ');
		}

		SetCursorPos(0, static_cast<uint8_t>(m_nRows - 1));

		Write(m_nRows, pText);
	}

	void TextStatus(const char *pText, Display7SegmentMessage message, uint32_t nConsoleColor = UINT32_MAX) {
		TextStatus(pText);
		Status(message);

		if (nConsoleColor == UINT32_MAX) {
			return;
		}

		console_status(nConsoleColor, pText);
	}

	void TextStatus(const char *pText, uint8_t nValue7Segment, bool bHex = false) {
		TextStatus(pText);
		Status(nValue7Segment, bHex);
	}

	void SetContrast(uint8_t nContrast) {
		SpiLcd.SetBackLight(nContrast);
	}

	void SetSleep(bool bSleep) {
		m_bIsSleep = bSleep;

		SpiLcd.EnableSleep(bSleep);

		if (!bSleep) {
			m_nMillis = Hardware::Get()->Millis();
		}
	}

	bool isSleep() const {
		return m_bIsSleep;
	}

	void SetSleepTimeout(uint32_t nSleepTimeout = display::Defaults::SEEP_TIMEOUT) {
		m_nSleepTimeout = 1000 * 60 * nSleepTimeout;
	}

	uint32_t GetSleepTimeout() const {
		return m_nSleepTimeout / 1000U / 60U;
	}

	void SetFlipVertically(bool doFlipVertically) {
		SpiLcd.SetRotation(doFlipVertically ? 3 : 1);
	}

	uint8_t GetColumns() const {
		return m_nCols;
	}

	uint8_t GetRows() const {
		return m_nRows;
	}

	uint8_t GetContrast() const {
		return m_nContrast;
	}

	bool GetFlipVertically() const {
		return m_bIsFlippedVertically;
	}

	void PrintInfo() {
#if defined (CONFIG_USE_ILI9341)
		printf("ILI9341 ");
#else
		printf("ST7789 ");
#endif
		printf("(%d,%d)\n", m_nRows, m_nCols);
	}

	void Run() {
		if (m_nSleepTimeout == 0) {
			return;
		}

		if (!m_bIsSleep) {
			if (__builtin_expect(((Hardware::Get()->Millis() - m_nMillis) > m_nSleepTimeout), 0)) {
				SetSleep(true);
			}
		} else {
			if (__builtin_expect((display::timeout::gpio_renew()), 0)) {
				SetSleep(false);
			}
		}
	}

	static Display* Get() {
		return s_pThis;
	}

private:
#if defined (CONFIG_USE_ILI9341)
	ILI9341 SpiLcd;
#else
	ST7789	SpiLcd;
#endif
	uint8_t m_nCols;
	uint8_t m_nRows;
	uint8_t m_nContrast { 0x7F };
	bool m_bIsFlippedVertically { false };
	bool m_bIsSleep { false };
	uint32_t m_nSleepTimeout { 1000 * 60 * display::Defaults::SEEP_TIMEOUT };
	uint32_t m_nMillis { 0 };

	uint16_t m_nCursorX { 0 };
	uint16_t m_nCursorY { 0 };

	static Display *s_pThis;
};

#endif /* SPI_DISPLAY_H_ */
