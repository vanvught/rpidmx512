/**
 * @file ssd1311.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "i2c/ssd1311.h"

// Co – Continuation bit
// D/C# – Data / Command Selection bit
// A control byte mainly consists of Co and D/C# bits following by six “0”’s.

namespace ssd1311 {
static constexpr uint8_t DEFAULT_I2C_ADDRESS = 0x3C;
static constexpr auto MAX_COLUMNS = 20;
static constexpr auto MAX_ROWS = 4;
static constexpr uint8_t MODE_DATA = 0x40;	// Co = 0, D/C# = 1
static constexpr uint8_t MODE_CMD = 0x80;	// Co = 1, D/C# = 0
}  // namespace sdd1311
enum class Rom : uint8_t {
	A, B, C
};
namespace cmd {
static constexpr uint8_t CLEAR_DISPLAY = 0x01;
static constexpr uint8_t CGRAM_ADDRESS = 0x40;
static constexpr uint8_t FUNCTION_SELECTION_B = 0x72;
static constexpr uint8_t DDRAM_ADDRESS = 0x80;
static constexpr uint8_t CONTRAST = 0x81;
}  // namespace cmd

using namespace ssd1311;

static uint8_t _ClearBuffer[1 + MAX_COLUMNS] __attribute__((aligned(4)));
static uint8_t _TextBuffer[1 + MAX_COLUMNS] __attribute__((aligned(4)));

Ssd1311 *Ssd1311::s_pThis = nullptr;

Ssd1311::Ssd1311(): m_I2C(DEFAULT_I2C_ADDRESS) {
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_nRows = MAX_ROWS;
	m_nCols = MAX_COLUMNS;
}

bool Ssd1311::Start() {
	if (!m_I2C.IsConnected()) {
		return false;
	}

	if (!CheckSSD1311()) {
		return false;
	}

	for (uint32_t i = 0; i < sizeof(_ClearBuffer); i++) {
		_ClearBuffer[i] = ' ';
	}

	_ClearBuffer[0] = MODE_DATA;
	_TextBuffer[0] = MODE_DATA;
	
	SendCommand(0x3A);
	SendCommand(0x09);
	SendCommand(0x05);
	SendCommand(0x1C);
	SendCommand(0x3C);
	SendCommand(0x3A);
	SendCommand(0x72);
	SendData(0x00);
	SendCommand(0x3C);
	SendCommand(0x0C);
	SendCommand(0x01);
	
	SelectRamRom(0, static_cast<uint8_t>(Rom::A));

	return true;
}

void Ssd1311::PrintInfo() {
	printf("SSD1311 (%u,%u)\n", static_cast<unsigned int>(m_nRows), static_cast<unsigned int>(m_nCols));
}

void Ssd1311::Cls() {
	SendCommand(cmd::CLEAR_DISPLAY);
}

void Ssd1311::PutChar(int c) {
	SendData(c & 0x7F);
}

void Ssd1311::PutString(const char *pString) {
	assert(pString != nullptr);

	uint32_t n = MAX_COLUMNS;
	auto *pSrc = pString;
	auto *s = reinterpret_cast<char *>(&_TextBuffer[1]);

	while (n > 0 && *pSrc != '\0') {
		*s++ = *pSrc++;
		--n;
	}

	if (m_bClearEndOfLine) {
		m_bClearEndOfLine = false;

		for (auto i = static_cast<uint32_t>(pSrc -  pString); i < MAX_COLUMNS; i++) {
			*s++ = ' ';
		}

		SendData(_TextBuffer, 1U + MAX_COLUMNS);
		return;
	}

	SendData(_TextBuffer, 1U + MAX_COLUMNS - n);
}

/**
 * nLine [1..4]
 */
void Ssd1311::ClearLine(uint32_t nLine) {
	if (__builtin_expect((!((nLine > 0) && (nLine <= MAX_ROWS))), 0)) {
		return;
	}

	Ssd1311::SetCursorPos(0, static_cast<uint8_t>(nLine - 1));
	SendData(_ClearBuffer, sizeof(_ClearBuffer));
	Ssd1311::SetCursorPos(0, static_cast<uint8_t>(nLine - 1));
}

void Ssd1311::TextLine(uint32_t nLine, const char *pData, uint32_t nLength) {
	if (__builtin_expect((!((nLine > 0) && (nLine <= MAX_ROWS))), 0)) {
		return;
	}

	Ssd1311::SetCursorPos(0, static_cast<uint8_t>(nLine - 1));
	Text(pData, nLength);
}

void Ssd1311::Text(const char *pData, uint32_t nLength) {
	if (nLength > MAX_COLUMNS) {
		nLength = MAX_COLUMNS;
	}

	memcpy(&_TextBuffer[1], pData, nLength);

	if (m_bClearEndOfLine) {
		m_bClearEndOfLine = false;

		memset(&_TextBuffer[nLength + 1], ' ', MAX_COLUMNS - nLength);

		SendData(_TextBuffer, 1U + MAX_COLUMNS);
		return;
	}

	SendData(_TextBuffer, 1U + nLength);
}

/**
 * (0,0)
 */
void Ssd1311::SetCursorPos(uint32_t nCol, uint32_t nRow) {
	if  (__builtin_expect((!((nCol < MAX_COLUMNS) && (nRow < MAX_ROWS))), 0)) {
		return;
	}

	// In 4-line display mode (N=1, NW = 1), DDRAM address is from “00H” – “13H” in the 1st line, from
	// “20H” to “33H” in the 2nd line, from “40H” – “53H” in the 3rd line and from “60H” – “73H” in the 4th line.

	SetDDRAM(static_cast<uint8_t>((nCol + nRow * 0x20)));
}

/**
 * 9.2.2 Function Selection B [72h]
 *
 * The character number of the Character Generator RAM and
 * the character ROM can be selected through this command
 *
 * It is recommended to turn off the display (cmd 08h) before setting no. of CGRAM and defining character ROM,
 * while clear display (cmd 01h) is recommended to sent afterwards
 */
void Ssd1311::SelectRamRom(uint32_t nRam, uint32_t nRom) {
	// [IS=X,RE=1,SD=0]
	Ssd1311::SetSleep(true);

	SetRE(FunctionSet::RE_ONE);

	SendCommand(cmd::FUNCTION_SELECTION_B);
	SendCommand(static_cast<uint8_t>(((nRom & 0x03) << 2) | (nRam & 0x03)));

	SetRE(FunctionSet::RE_ZERO);

	Ssd1311::SetSleep(false);
	Ssd1311::Cls();
}

void Ssd1311::SetDDRAM(uint8_t nAddress) {
	// [IS=X,RE=0,SD=0]
	SendCommand(cmd::DDRAM_ADDRESS | (nAddress & 0x7F));
}

void Ssd1311::SetCGRAM(uint8_t nAddress) {
	// [IS=0,RE=0,SD=0]
	SendCommand(cmd::CGRAM_ADDRESS | (nAddress & 0x3F));
}

void Ssd1311::SetRE(FunctionSet re) {
	uint8_t nCmd = 0x20;

	constexpr auto N = 1;
	constexpr auto DH = 0;

	if (re == FunctionSet::RE_ZERO) {
		// 0 0 1 0 N DH RE IS
		// N - Numbers of display line
		// DH - Double height font control
		// RE register set to 0
		// IS register
		constexpr auto IS = 0;
		nCmd |= (N << 3);
		nCmd |= (DH << 2);
		nCmd |= (0 << 1);
		nCmd |= (IS << 0);
	} else {
		// 0 0 1 0 N BE RE REV
		// N - Numbers of display line
		// BE - CGRAM blink enable
		// RE register set to 1
		// REV - reverse display
		constexpr auto BE = 0;
		constexpr auto REV = 0;
		nCmd |= (N << 3);
		nCmd |= (BE << 2);
		nCmd |= (1 << 1);
		nCmd |= (REV << 0);
	}

	SendCommand(nCmd);
}

void Ssd1311::SetSD(CommandSet sd) {
	SetRE(FunctionSet::RE_ONE);
	SendCommand(sd == CommandSet::DISABLED ? 0x78 : 0x79);
}

void Ssd1311::SendCommand(uint8_t nCommand) {
	m_I2C.WriteRegister(MODE_CMD, nCommand);
}

void Ssd1311::SendData(uint8_t nData) {
	m_I2C.WriteRegister(MODE_DATA, nData);
}

void Ssd1311::SendData(const uint8_t *pData, uint32_t nLength) {
	m_I2C.Write(reinterpret_cast<const char*>(pData), nLength);
}

bool Ssd1311::CheckSSD1311() {
	SetCGRAM(0);

	const uint8_t dataSend[] = {MODE_DATA, 0xAA, 0x55, 0xAA, 0x55};

	SendData(dataSend, sizeof(dataSend));

	uint8_t dataReceive[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	static_assert((1 + sizeof(dataSend)) == sizeof(dataReceive), "Mismatch buffers");

	SetCGRAM(0);
	m_I2C.Write(MODE_DATA);
	m_I2C.Read(reinterpret_cast<char *>(dataReceive), sizeof(dataReceive));

	const auto isEqual = (memcmp(&dataSend[1], &dataReceive[1], sizeof(dataSend) - 1) == 0);

#ifndef NDEBUG
	printf("CheckSSD1311 isEqual=%d\n", isEqual);
#endif

	return isEqual;
}

/**
 * 9.1.4 Display ON/OFF Control
 */

constexpr auto DISPLAY_ON_OFF = (1U << 2);
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
constexpr auto CURSOR_ON_OFF = (1U << 1);
constexpr auto CURSOR_BLINK_ON_OFF = (1U << 0);
#endif

void Ssd1311::SetSleep(bool bSleep) {
	if (bSleep) {
		m_nDisplayControl &= static_cast<uint8_t>(~DISPLAY_ON_OFF);
	} else {
		m_nDisplayControl |= DISPLAY_ON_OFF;
	}

	SendCommand(m_nDisplayControl);
}

void Ssd1311::SetContrast(uint8_t nContrast) {
	// [IS=X,RE=1,SD=1]
	SetRE(FunctionSet::RE_ONE);
	SetSD(CommandSet::ENABLED);

	// This is a two bytes command
	SendCommand(cmd::CONTRAST);
	SendCommand(nContrast);

	SetSD(CommandSet::DISABLED);
	SetRE(FunctionSet::RE_ZERO);
}

#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
# define UNUSED
#else
# define UNUSED [[maybe_unused]]
#endif

void Ssd1311::SetCursor(UNUSED uint32_t nMode) {
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
	switch (static_cast<int>(nMode)) {
	case display::cursor::OFF:
		m_nDisplayControl &= static_cast<uint8_t>(~CURSOR_ON_OFF);
		break;
	case display::cursor::ON:
		m_nDisplayControl |= CURSOR_ON_OFF;
		m_nDisplayControl &= static_cast<uint8_t>(~CURSOR_BLINK_ON_OFF);
		break;
	case display::cursor::ON | display::cursor::BLINK_ON:
		m_nDisplayControl |= CURSOR_ON_OFF;
		m_nDisplayControl |= CURSOR_BLINK_ON_OFF;
		break;
	default:
		break;
	}

	SendCommand(m_nDisplayControl);
#endif
}
