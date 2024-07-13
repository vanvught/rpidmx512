/**
 * @file display.h
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef I2C_DISPLAY_H_
#define I2C_DISPLAY_H_

#if defined (CONFIG_DISPLAY_USE_SPI)
# error
#endif

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cassert>

#include "displayset.h"

#include "hardware.h"

#include "hal_i2c.h"
#if defined (DISPLAYTIMEOUT_GPIO)
# include "hal_gpio.h"
#endif

namespace display {
enum class Type {
	PCF8574T_1602, PCF8574T_2004, SSD1306, SSD1311, UNKNOWN
};
namespace segment7 {
static constexpr uint8_t MCP23017_I2C_ADDRESS = 0x20;
static constexpr uint8_t MCP23017_IODIRA = 0x00;	///< I/O DIRECTION (IODIRA) REGISTER, 1 = Input (default), 0 = Output
static constexpr uint8_t MCP23017_GPIOA = 0x12;		///< PORT (GPIOA) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch
static constexpr uint8_t I2C_ADDRESS = (MCP23017_I2C_ADDRESS + 1);	///< It must be different from base address
}  // namespace segment7
}  // namespace display

class Display {
public:
	Display();
	Display(uint32_t nRows);
	Display(display::Type type);

	~Display() {
		s_pThis = nullptr;
		delete m_LcdDisplay;
	}

	bool isDetected() const {
		return m_LcdDisplay == nullptr ? false : true;
	}

	display::Type GetDetectedType() const {
		return m_tType;
	}

	void PrintInfo() {
		if (m_LcdDisplay == nullptr) {
			puts("No display found");
			return;
		}

		m_LcdDisplay->PrintInfo();
	}

	void Cls() {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->Cls();
	}

	void ClearLine(uint32_t nLine) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->ClearLine(nLine);
	}

	void PutChar(int c) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->PutChar(c);
	}

	void PutString(const char *pText) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->PutString(pText);
	}

	int Write(uint32_t nLine, const char *pText) {
		if (m_LcdDisplay == nullptr) {
			return 0;
		}

		const auto *p = pText;
		uint32_t nCount = 0;

		const auto nColumns = m_LcdDisplay->GetColumns();

		while ((*p != 0) && (nCount++ < nColumns)) {
			++p;
		}

		m_LcdDisplay->TextLine(nLine, pText, nCount);

		return static_cast<int>(nCount);
	}

	int Printf(uint32_t nLine, const char *format, ...) {
		if (m_LcdDisplay == nullptr) {
			return 0;
		}

		char buffer[32];

		va_list arp;

		va_start(arp, format);

		auto i = vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, arp);

		va_end(arp);

		m_LcdDisplay->TextLine(nLine, buffer, static_cast<uint32_t>(i));

		return i;
	}

	void TextLine(uint32_t nLine, const char *pText, uint32_t nLength) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->TextLine(nLine, pText, nLength);
	}

	void TextStatus(const char *pText) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		const auto nColumns = m_LcdDisplay->GetColumns();
		const auto nRows = m_LcdDisplay->GetRows();

		assert(nColumns >= 1);
		assert(nRows >= 1);

		SetCursorPos(0, nRows - 1U);

		for (uint32_t i = 0; i < nColumns - 1U; i++) {
			PutChar(' ');
		}

		SetCursorPos(0, nRows - 1U);

		Write(nRows, pText);
	}

	void TextStatus(const char *pText, uint32_t nConsoleColor) {
		TextStatus(pText);

		if (nConsoleColor == UINT32_MAX) {
			return;
		}

		console_status(nConsoleColor, pText);
	}

	void SetCursor(uint32_t nMode) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->SetCursor(nMode);
	}

	void SetCursorPos(uint32_t nCol, uint32_t nRow) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->SetCursorPos(nCol, nRow);
	}

	void SetSleepTimeout(uint32_t nSleepTimeout = display::Defaults::SEEP_TIMEOUT) {
		m_nSleepTimeout = 1000U * 60U * nSleepTimeout;
	}

	uint32_t GetSleepTimeout() const {
		return m_nSleepTimeout / 1000U / 60U;
	}

	void SetContrast(uint8_t nContrast) {
		m_nContrast = nContrast;

		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->SetContrast(nContrast);
	}

	uint8_t GetContrast() const {
		return m_nContrast;
	}

	void SetFlipVertically(bool doFlipVertically) {
		m_bIsFlippedVertically = doFlipVertically;

		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->SetFlipVertically(doFlipVertically);
	}

	void ClearEndOfLine() {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->ClearEndOfLine();
	}

	bool GetFlipVertically() const {
		return m_bIsFlippedVertically;
	}

	uint32_t GetColumns() const {
		if (m_LcdDisplay == nullptr) {
			return 0;
		}

		return m_LcdDisplay->GetColumns();
	}

	uint32_t GetRows() const {
		if (m_LcdDisplay == nullptr) {
			return 0;
		}

		return m_LcdDisplay->GetRows();
	}

	void Progress() {
		static constexpr char SYMBOLS[] = { '/' , '-', '\\' , '|' };
		static uint32_t nSymbolsIndex;

		Display::Get()->SetCursorPos(Display::Get()->GetColumns() - 1U, Display::Get()->GetRows() - 1U);
		Display::Get()->PutChar(SYMBOLS[nSymbolsIndex++]);

		if (nSymbolsIndex >= sizeof(SYMBOLS)) {
			nSymbolsIndex = 0;
		}
	}

	void SetSleep(bool bSleep) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_bIsSleep = bSleep;

		m_LcdDisplay->SetSleep(bSleep);

		if (!bSleep) {
			m_nMillis = Hardware::Get()->Millis();
		}
	}

	bool isSleep() const {
		return m_bIsSleep;
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
#if defined (DISPLAYTIMEOUT_GPIO)
			if (__builtin_expect(((FUNC_PREFIX(gpio_lev(DISPLAYTIMEOUT_GPIO)) == LOW)), 0)) {
				SetSleep(false);
			}
#endif
		}
	}

	static Display* Get() {
		return s_pThis;
	}

private:
	void Detect(display::Type tDisplayType);
	void Detect(uint32_t nRows);

private:
	display::Type m_tType { display::Type::UNKNOWN };
	uint32_t m_nMillis { 0 };
	HAL_I2C m_I2C;
	uint32_t m_nSleepTimeout { 1000 * 60 * display::Defaults::SEEP_TIMEOUT };
	uint8_t m_nContrast { 0x7F };

	bool m_bIsSleep { false };
	bool m_bIsFlippedVertically { false };
#if defined (CONFIG_DISPLAY_HAVE_7SEGMENT)
	bool m_bHave7Segment { false };
#endif

	DisplaySet *m_LcdDisplay { nullptr };
	static Display *s_pThis;
};

#endif /* I2C_DISPLAY_H_ */
