/**
 * @file display.h
 *
 */
/* Copyright (C) 2017-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hal_i2c.h"
#include "hardware.h"

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
	Display(display::Type tDisplayType);

	~Display() {
		s_pThis = nullptr;
		delete m_LcdDisplay;
	}

	void Cls() {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->Cls();
	}

	void ClearLine(uint8_t nLine) {
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

	int Write(uint8_t nLine, const char *pText) {
		if (m_LcdDisplay == nullptr) {
			return 0;
		}

		const auto *p = pText;
		int nCount = 0;

		const auto nColumns = static_cast<int>(m_LcdDisplay->GetColumns());

		while ((*p != 0) && (nCount++ < nColumns)) {
			++p;
		}

		m_LcdDisplay->TextLine(nLine, pText, static_cast<uint8_t>(nCount));

		return nCount;
	}

	int Printf(uint8_t nLine, const char *format, ...) {
		if (m_LcdDisplay == nullptr) {
			return 0;
		}

		char buffer[32];

		va_list arp;

		va_start(arp, format);

		auto i = vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, arp);

		va_end(arp);

		m_LcdDisplay->TextLine(nLine, buffer, static_cast<uint16_t>(i));

		return i;
	}

	void TextLine(uint8_t nLine, const char *pText, uint8_t nLength) {
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

		assert((nColumns - 1) >= 0);
		assert((nRows - 1) >= 0);

		SetCursorPos(0, static_cast<uint8_t>(nRows - 1));

		for (uint32_t i = 0; i < static_cast<uint32_t>(nColumns - 1); i++) {
			PutChar(' ');
		}

		SetCursorPos(0, static_cast<uint8_t>(nRows - 1));

		Write(nRows, pText);
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

	bool isDetected() const {
		return m_LcdDisplay == nullptr ? false : true;
	}

	display::Type GetDetectedType() const {
		return m_tType;
	}

	void SetCursor(uint32_t nMode) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->SetCursor(nMode);
	}

	void SetCursorPos(uint8_t nCol, uint8_t nRow) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->SetCursorPos(nCol, nRow);
	}

	void SetSleepTimeout(uint32_t nSleepTimeout = display::Defaults::SEEP_TIMEOUT) {
		m_nSleepTimeout = 1000 * 60 * nSleepTimeout;
	}

	uint32_t GetSleepTimeout() const {
		return m_nSleepTimeout / 1000U / 60U;
	}

	void SetContrast(uint8_t nContrast) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->SetContrast(nContrast);
	}

	void SetFlipVertically(bool doFlipVertically) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->SetFlipVertically(doFlipVertically);
	}

	uint8_t GetColumns() const {
		if (m_LcdDisplay == nullptr) {
			return 0;
		}

		return m_LcdDisplay->GetColumns();
	}

	uint8_t GetRows() const {
		if (m_LcdDisplay == nullptr) {
			return 0;
		}

		return m_LcdDisplay->GetRows();
	}

	bool GetFlipVertically() const {
		if (m_LcdDisplay == nullptr) {
			return false;
		}

		return m_LcdDisplay->GetFlipVertically();
	}

	uint8_t GetContrast() const {
		if (m_LcdDisplay == nullptr) {
			return 0;
		}

		return m_LcdDisplay->GetContrast();
	}

	void Status(Display7SegmentMessage nData) {
		if (m_bHave7Segment) {
			m_I2C.WriteRegister(display::segment7::MCP23017_GPIOA, static_cast<uint16_t>(~static_cast<uint16_t>(nData)));
		}
	}

	void Status(uint8_t nValue, bool bHex) {
		if (m_bHave7Segment) {
			uint16_t nData;

			if (!bHex) {
				nData = GetData(nValue / 10);
				nData = static_cast<uint16_t>(nData | GetData(nValue % 10) << 8U);
			} else {
				nData = GetData(nValue & 0x0F);
				nData = static_cast<uint16_t>(nData | GetData((nValue >> 4) & 0x0F) << 8U);
			}

			m_I2C.WriteRegister(display::segment7::MCP23017_GPIOA, static_cast<uint16_t>(~nData));
		}
	}

	void PrintInfo() {
		if (m_LcdDisplay == nullptr) {
			puts("No display found");
			return;
		}

		m_LcdDisplay->PrintInfo();
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
			if (__builtin_expect((display::timeout::gpio_renew()), 0)) {
				SetSleep(false);
			}
		}
	}

	static Display* Get() {
		return s_pThis;
	}

private:
	void Detect(display::Type tDisplayType);
	void Detect(uint32_t nRows);
	void Detect7Segment() {
		m_bHave7Segment = m_I2C.IsConnected();

		if (m_bHave7Segment) {
			m_I2C.WriteRegister(display::segment7::MCP23017_IODIRA, static_cast<uint16_t>(0x0000)); // All output
			Status(Display7SegmentMessage::INFO_STARTUP);
		}
	}

	uint16_t GetData(const uint8_t nHexValue) const {
		switch (nHexValue) {
		case 0:
			return display7segment::CH_0;
			break;
		case 1:
			return display7segment::CH_1;
			break;
		case 2:
			return display7segment::CH_2;
			break;
		case 3:
			return display7segment::CH_3;
			break;
		case 4:
			return display7segment::CH_4;
			break;
		case 5:
			return display7segment::CH_5;
			break;
		case 6:
			return display7segment::CH_6;
			break;
		case 7:
			return display7segment::CH_7;
			break;
		case 8:
			return display7segment::CH_8;
			break;
		case 9:
			return display7segment::CH_9;
			break;
		case 0xa:
			return display7segment::CH_A;
			break;
		case 0xb:
			return display7segment::CH_B;
			break;
		case 0xc:
			return display7segment::CH_C;
			break;
		case 0xd:
			return display7segment::CH_D;
			break;
		case 0xe:
			return display7segment::CH_E;
			break;
		case 0xf:
			return display7segment::CH_F;
			break;
		default:
			break;
		}

		return display7segment::CH_BLANK;
	}

private:
	display::Type m_tType { display::Type::UNKNOWN };
	uint32_t m_nMillis { 0 };
	HAL_I2C m_I2C;
	bool m_bIsSleep { false };
	bool m_bHave7Segment { false };
	uint32_t m_nSleepTimeout { 1000 * 60 * display::Defaults::SEEP_TIMEOUT };

	DisplaySet *m_LcdDisplay { nullptr };
	static Display *s_pThis;
};

#endif /* I2C_DISPLAY_H_ */
