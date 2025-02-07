/**
 * @file display.h
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if !defined (CONFIG_DISPLAY_USE_SPI)
# error
#endif

#if defined(__GNUC__) && !defined(__clang__)
# if defined (CONFIG_SPI_LCD_OPTIMIZE_O2) || defined (CONFIG_SPI_LCD_OPTIMIZE_O3)
#  pragma GCC push_options
#  if defined (CONFIG_SPI_LCD_OPTIMIZE_O2)
#   pragma GCC optimize ("O2")
#  else
#   pragma GCC optimize ("O3")
#  endif
#  pragma GCC optimize ("no-tree-loop-distribute-patterns")
#  pragma GCC optimize ("-fprefetch-loop-arrays")
# endif
#endif

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cassert>

#if defined (CONFIG_USE_ILI9341)
# include "spi/ili9341.h"
  typedef ILI9341 LcdDriver;
#elif defined (CONFIG_USE_ST7735S)
# include "spi/st7735s.h"
  typedef ST7735S LcdDriver;
#else
# include "spi/st7789.h"
  typedef ST7789 LcdDriver;
#endif

#include "spi/lcd_font.h"
#include "spi/spilcd.h"

#if defined (DISPLAYTIMEOUT_GPIO)
# include "hal_gpio.h"
#endif

#if defined(SPI_LCD_HAVE_CS_GPIO)
 static constexpr uint32_t CS_GPIO = SPI_LCD_CS_GPIO;
#else
 static constexpr uint32_t CS_GPIO = 0;
#endif

#include "debug.h"

class Display : public LcdDriver {
public:
	Display(uint32_t nCS = CS_GPIO) : LcdDriver(nCS) {
		DEBUG_ENTRY

		s_pThis = this;

		SetBackLight(1);
		SetFlipVertically(false);
		FillColour(COLOR_BACKGROUND);

		m_nCols = (GetWidth() / s_pFONT->Width);
		m_nRows = (GetHeight() / s_pFONT->Height);

#if defined (DISPLAYTIMEOUT_GPIO)
		FUNC_PREFIX(gpio_fsel(DISPLAYTIMEOUT_GPIO, GPIO_FSEL_INPUT));
		FUNC_PREFIX(gpio_set_pud(DISPLAYTIMEOUT_GPIO, GPIO_PULL_UP));
#endif

		PrintInfo();
		DEBUG_EXIT
	}

	~Display() {}

	bool isDetected() const {
		return true;
	}

	void PrintInfo() {
#if defined (CONFIG_USE_ILI9341)
		printf("ILI9341 ");
#elif defined (CONFIG_USE_ST7735S)
		printf("ST7735S ");
#else
		printf("ST7789 ");
#endif
		printf("(%d,%d)\n", m_nRows, m_nCols);
	}

	void Cls() {
		FillColour(COLOR_BACKGROUND);
	}

	void SetCursorPos(const uint32_t nCol, const uint32_t nRow) {
		m_nCursorX = nCol * s_pFONT->Width;
		m_nCursorY = nRow * s_pFONT->Height;
	}

	void PutChar(const int c) {
		DrawChar(m_nCursorX, m_nCursorY, static_cast<char>(c), s_pFONT, COLOR_BACKGROUND, COLOR_FOREGROUND);

		m_nCursorX += s_pFONT->Width;

		if (m_nCursorX >= GetWidth()) {
			m_nCursorX = 0;

			m_nCursorY += s_pFONT->Height;

			if (m_nCursorY >= GetHeight()) {
				m_nCursorY = 0;
			}
		}
	}

	void PutString(const char *p) {
		for (uint32_t i = 0; *p != '\0'; i++) {
			PutChar(static_cast<int>(*p));
			p++;
		}
	}

	void ClearLine(const uint32_t nLine) {
		if (__builtin_expect((!(nLine <= m_nRows)), 0)) {
			return;
		}

		SetCursorPos(0, (nLine - 1U));

		for (uint32_t i = 0; i < m_nCols; i++) {
			PutChar(' ');
		}

		SetCursorPos(0, (nLine - 1U));
	}

	void TextLine(const uint32_t nLine, const char *pText, const uint32_t nLength) {
		if (__builtin_expect((!(nLine <= m_nRows)), 0)) {
			return;
		}

		SetCursorPos(0, (nLine - 1U));
		Text(pText, nLength);
	}

	void ClearEndOfLine() {
		m_bClearEndOfLine = true;
	}

	void Text(const char *pData, uint32_t nLength) {
		if (nLength > m_nCols) {
			nLength = m_nCols;
		}

		for (uint32_t i = 0; i < nLength; i++) {
			PutChar(pData[i]);
		}
	}

	int Write(const uint32_t nLine, const char *pText) {
		const auto *p = pText;
		int nCount = 0;

		const auto nColumns = static_cast<int>(m_nCols);

		while ((*p != 0) && (nCount++ < nColumns)) {
			++p;
		}

		TextLine(nLine, pText, static_cast<uint8_t>(nCount));

		return nCount;
	}

	int Printf(const uint8_t nLine, const char *format, ...) {
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

		for (uint32_t i = 0; i < (m_nCols - 1); i++) {
			PutChar(' ');
		}

		SetCursorPos(0, static_cast<uint8_t>(m_nRows - 1));

		Write(m_nRows, pText);
	}

	void TextStatus(const char *pText, uint32_t nConsoleColor) {
		TextStatus(pText);

		if (nConsoleColor == UINT32_MAX) {
			return;
		}

		console_status(nConsoleColor, pText);
	}

	void Progress() {
		static constexpr char SYMBOLS[] = { '/' , '-', '\\' , '|' };
		static uint32_t nSymbolsIndex;

		SetCursorPos(GetColumns() - 1U, GetRows() - 1U);
		PutChar(SYMBOLS[nSymbolsIndex++]);

		if (nSymbolsIndex >= sizeof(SYMBOLS)) {
			nSymbolsIndex = 0;
		}
	}

	void SetContrast(const uint8_t nContrast) {
		SetBackLight(nContrast);
	}

	void SetSleep(const bool bSleep) {
		m_bIsSleep = bSleep;

		EnableSleep(bSleep);

		if (!bSleep) {
			SetSleepTimer(m_nSleepTimeout != 0);
		}
	}

	bool isSleep() const {
		return m_bIsSleep;
	}

	void SetSleepTimeout(uint32_t nSleepTimeout = display::Defaults::SLEEP_TIMEOUT) {
		m_nSleepTimeout = 1000U * 60U * nSleepTimeout;
		SetSleepTimer(m_nSleepTimeout != 0);
	}

	uint32_t GetSleepTimeout() const {
		return m_nSleepTimeout / 1000U / 60U;
	}

	void SetFlipVertically(bool doFlipVertically) {
		SetRotation(doFlipVertically ? 3 : 1);
	}

	uint32_t GetColumns() const {
		return m_nCols;
	}

	uint32_t GetRows() const {
		return m_nRows;
	}

	uint8_t GetContrast() const {
		return m_nContrast;
	}

	bool GetFlipVertically() const {
		return m_bIsFlippedVertically;
	}

	void Run() {
		if (m_nSleepTimeout == 0) {
			return;
		}

		if (m_bIsSleep) {
#if defined (DISPLAYTIMEOUT_GPIO)
			if (__builtin_expect(((FUNC_PREFIX(gpio_lev(DISPLAYTIMEOUT_GPIO)) == 0)), 0)) {
				SetSleep(false);
			}
#endif
		}
	}

	static Display *Get() {
		return s_pThis;
	}

private:
	void SetSleepTimer(const bool bActive);

private:
	uint32_t m_nCols;
	uint32_t m_nRows;
	uint32_t m_nSleepTimeout { 1000U * 60U * display::Defaults::SLEEP_TIMEOUT };
	uint32_t m_nCursorX { 0 };
	uint32_t m_nCursorY { 0 };

	uint8_t m_nContrast { 0x7F };

	bool m_bIsFlippedVertically { false };
	bool m_bIsSleep { false };
	bool m_bClearEndOfLine { false };

	static inline Display *s_pThis;

#if defined (SPI_LCD_240X320)
	static constexpr sFONT *s_pFONT = &Font16x24;
#elif defined (SPI_LCD_128X128)
	static constexpr sFONT *s_pFONT = &Font8x8;
#elif defined (SPI_LCD_160X80)
	static constexpr sFONT *s_pFONT = &Font8x8;
#else
	static constexpr sFONT *s_pFONT = &Font12x12;
#endif
	static constexpr uint16_t COLOR_BACKGROUND = 0x001F;
	static constexpr uint16_t COLOR_FOREGROUND = 0xFFE0;
};

#if defined(__GNUC__) && !defined(__clang__)
# if defined (CONFIG_SPI_LCD_OPTIMIZE_O2) || defined (CONFIG_SPI_LCD_OPTIMIZE_O3)
#  pragma GCC pop_options
# endif
#endif
#endif /* SPI_DISPLAY_H_ */
