/**
 * @file tlc59711.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#if !defined(NDEBUG) || defined(__linux__)
# include <stdio.h>
#endif
#include <string.h>
#include <cassert>

#include "tlc59711.h"

#include "hal_spi.h"

#define TLC59711_RGB_8BIT_VALUE(x)	((uint8_t)(x))
#define TLC59711_RGB_16BIT_VALUE(x)	((uint16_t)(x))

#define TLC59711_COMMAND			0x25U
	#define TLC59711_COMMAND_SHIFT	26U

#define TLC59711_OUTTMG_DEFAULT		0U
	#define TLC59711_OUTTMG_SHIFT	25U

#define TLC59711_EXTGCK_DEFAULT		0U
	#define TLC59711_EXTGCK_SHIFT	24U

#define TLC59711_TMGRST_DEFAULT		1U
	#define TLC59711_TMGRST_SHIFT	23U

#define TLC59711_DSPRPT_DEFAULT		1U
	#define TLC59711_DSPRPT_SHIFT	22U

#define TLC59711_BLANK_DEFAULT		0U
	#define TLC59711_BLANK_SHIFT	21U

#define TLC59711_GS_DEFAULT			0x7FU
	#define TLC59711_GS_MASK		0x7FU
	#define TLC59711_GS_RED_SHIFT	0U
	#define TLC59711_GS_GREEN_SHIFT	7U
	#define TLC59711_GS_BLUE_SHIFT	14U

TLC59711::TLC59711(uint8_t nBoards, uint32_t nSpiSpeedHz):
	m_nBoards(nBoards),
	m_nSpiSpeedHz(nSpiSpeedHz == 0 ? TLC59711SpiSpeed::DEFAULT : nSpiSpeedHz),
	m_nFirst32(0),
	m_pBuffer(0),
	m_pBufferBlackout(0),
	m_nBufSize(0)
{
	FUNC_PREFIX(spi_begin());

	if (m_nSpiSpeedHz > TLC59711SpiSpeed::MAX) {
		m_nSpiSpeedHz = TLC59711SpiSpeed::MAX;
	}

	if (nBoards == 0) {
		nBoards = 1;
	}

	m_nBufSize = nBoards * TLC59711Channels::U16BIT;

	m_pBuffer = new uint16_t[m_nBufSize];
	assert(m_pBuffer != nullptr);

	m_pBufferBlackout = new uint16_t[m_nBufSize];
	assert(m_pBufferBlackout != nullptr);

	for (uint32_t i = 0; i < m_nBufSize; i++) {
		m_pBuffer[i] = 0;
	}

	m_nFirst32 |= (TLC59711_COMMAND << TLC59711_COMMAND_SHIFT);

	SetOnOffTiming(TLC59711_OUTTMG_DEFAULT);
	SetExternalClock(TLC59711_EXTGCK_DEFAULT);
	SetDisplayTimingReset(TLC59711_TMGRST_DEFAULT);
	SetDisplayRepeat(TLC59711_DSPRPT_DEFAULT);
	SetBlank(TLC59711_BLANK_DEFAULT);
	SetGbcRed(TLC59711_GS_DEFAULT);
	SetGbcGreen(TLC59711_GS_DEFAULT);
	SetGbcBlue(TLC59711_GS_DEFAULT);

	memcpy(m_pBufferBlackout, m_pBuffer, m_nBufSize * 2);
}

TLC59711::~TLC59711() {
	delete[] m_pBuffer;
	m_pBuffer = nullptr;
}

bool TLC59711::Get(uint8_t nChannel, uint16_t &nValue) {
	const uint32_t nBoardIndex = nChannel / TLC59711Channels::OUT;

	if (nBoardIndex < m_nBoards) {
		const uint32_t nIndex = 2 + (nBoardIndex * TLC59711Channels::U16BIT) + ((12 * nBoardIndex) + 11 - nChannel);
		nValue =  __builtin_bswap16(m_pBuffer[nIndex]);
		return true;
	}

	return false;
}

void TLC59711::Set(uint8_t nChannel, uint16_t nValue) {
	const uint32_t nBoardIndex = nChannel / TLC59711Channels::OUT;

	if (nBoardIndex < m_nBoards) {
		const uint32_t nIndex = 2 + (nBoardIndex * TLC59711Channels::U16BIT) + ((12 * nBoardIndex) + 11 - nChannel);
		m_pBuffer[nIndex] = __builtin_bswap16(nValue);
	}
#ifndef NDEBUG
	else {
		printf("\t\tm_nBoards=%d, nBoardIndex=%d, nChannel=%d\n", static_cast<int>(m_nBoards), static_cast<int>(nBoardIndex), static_cast<int>(nChannel));
	}
#endif
}

bool TLC59711::GetRgb(uint8_t nOut, uint16_t& nRed, uint16_t& nGreen, uint16_t& nBlue) {
	const uint32_t nBoardIndex = nOut / 4;

	if (nBoardIndex < m_nBoards) {
		uint32_t nIndex = 2 + (nBoardIndex * TLC59711Channels::U16BIT) + (((4 * nBoardIndex) + 3 - nOut) * 3);
		nBlue = __builtin_bswap16(m_pBuffer[nIndex++]);
		nGreen = __builtin_bswap16(m_pBuffer[nIndex++]);
		nRed = __builtin_bswap16(m_pBuffer[nIndex]);
		return true;
	}

	return false;
}

void TLC59711::Set(uint8_t nChannel, uint8_t nValue) {
	const uint32_t nBoardIndex = nChannel / TLC59711Channels::OUT;

	if (nBoardIndex < m_nBoards) {
		const uint32_t nIndex = 2 + (nBoardIndex * TLC59711Channels::U16BIT) + ((12 * nBoardIndex) + 11 - nChannel);
		m_pBuffer[nIndex] = static_cast<uint16_t>(nValue) << 8 | static_cast<uint16_t>(nValue);
	}
#ifndef NDEBUG
	else {
		printf("\t\tm_nBoards=%d, nBoardIndex=%d, nChannel=%d\n",
				static_cast<int>(m_nBoards), static_cast<int>(nBoardIndex),
				static_cast<int>(nChannel));
	}
#endif
}

void TLC59711::SetRgb(uint8_t nOut, uint16_t nRed, uint16_t nGreen, uint16_t nBlue) {
	const uint32_t nBoardIndex = nOut / 4;

	if (nBoardIndex < m_nBoards) {
		uint32_t nIndex = 2 + (nBoardIndex * TLC59711Channels::U16BIT) + (((4 * nBoardIndex) +3 - nOut) * 3);
		m_pBuffer[nIndex++] = __builtin_bswap16(nBlue);
		m_pBuffer[nIndex++] = __builtin_bswap16(nGreen);
		m_pBuffer[nIndex] = __builtin_bswap16(nRed);
	}
#ifndef NDEBUG
	else {
		printf("m_nBoards=%d, nBoardIndex=%d, nOut=%d\n",
				static_cast<int>(m_nBoards), static_cast<int>(nBoardIndex),
				static_cast<int>(nOut));
	}
#endif
}

void TLC59711::SetRgb(uint8_t nOut, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	const uint32_t nBoardIndex = nOut / 4;

	if (nBoardIndex < m_nBoards) {
		uint32_t nIndex = 2 + (nBoardIndex * TLC59711Channels::U16BIT) + (((4 * nBoardIndex) + 3 - nOut) * 3);
		m_pBuffer[nIndex++] = static_cast<uint16_t>(nBlue) << 8 | static_cast<uint16_t>(nBlue);
		m_pBuffer[nIndex++] = static_cast<uint16_t>(nGreen) << 8 | static_cast<uint16_t>(nGreen);
		m_pBuffer[nIndex] = static_cast<uint16_t>(nRed) << 8 | static_cast<uint16_t>(nRed);
	}
#ifndef NDEBUG
	else {
		printf("m_nBoards=%d, nBoardIndex=%d, nOut=%d\n",
				static_cast<int>(m_nBoards), static_cast<int>(nBoardIndex),
				static_cast<int>(nOut));
	}
#endif
}

int TLC59711::GetBlank() const {
	return (m_nFirst32 & (1U << TLC59711_BLANK_SHIFT)) == (1U << TLC59711_BLANK_SHIFT);
}

void TLC59711::SetBlank(bool pBlank) {
	m_nFirst32 &= ~(1U << TLC59711_BLANK_SHIFT);

	if (pBlank) {
		m_nFirst32 |= (1U << TLC59711_BLANK_SHIFT);
	}

	UpdateFirst32();
}

int TLC59711::GetDisplayRepeat() const {
	return (m_nFirst32 & (1U << TLC59711_DSPRPT_SHIFT)) == (1U << TLC59711_DSPRPT_SHIFT);
}

void TLC59711::SetDisplayRepeat(bool pDisplayRepeat) {
	m_nFirst32 &= ~(1U << TLC59711_DSPRPT_SHIFT);

	if (pDisplayRepeat) {
		m_nFirst32 |= (1U << TLC59711_DSPRPT_SHIFT);
	}

	UpdateFirst32();
}

int TLC59711::GetDisplayTimingReset() const {
	return (m_nFirst32 & (1U << TLC59711_TMGRST_SHIFT)) == (1U << TLC59711_TMGRST_SHIFT);
}

void TLC59711::SetDisplayTimingReset(bool pDisplayTimingReset) {
	m_nFirst32 &= ~(1U << TLC59711_TMGRST_SHIFT);

	if (pDisplayTimingReset) {
		m_nFirst32 |= (1U << TLC59711_TMGRST_SHIFT);
	}

	UpdateFirst32();
}

int TLC59711::GetExternalClock() const {
	return (m_nFirst32 & (1U << TLC59711_EXTGCK_SHIFT)) == (1U << TLC59711_EXTGCK_SHIFT);
}

void TLC59711::SetExternalClock(bool pExternalClock) {
	m_nFirst32 &= ~(1U << TLC59711_EXTGCK_SHIFT);

	if (pExternalClock) {
		m_nFirst32 |= (1U << TLC59711_EXTGCK_SHIFT);
	}

	UpdateFirst32();
}

int TLC59711::GetOnOffTiming() const {
	return (m_nFirst32 & (1U << TLC59711_OUTTMG_SHIFT)) == (1U << TLC59711_OUTTMG_SHIFT);
}

void TLC59711::SetOnOffTiming(bool pOnOffTiming) {
	m_nFirst32 &= ~(1U << TLC59711_OUTTMG_SHIFT);

	if (pOnOffTiming) {
		m_nFirst32 |= (1U << TLC59711_OUTTMG_SHIFT);
	}

	UpdateFirst32();
}

uint8_t TLC59711::GetGbcRed() const {
	return (m_nFirst32 >> TLC59711_GS_RED_SHIFT) & TLC59711_GS_MASK;
}

void TLC59711::SetGbcRed(uint8_t nValue) {
	m_nFirst32 &= ~(TLC59711_GS_MASK << TLC59711_GS_RED_SHIFT);
	m_nFirst32 |= ((nValue & TLC59711_GS_MASK) << TLC59711_GS_RED_SHIFT);

	UpdateFirst32();
}

uint8_t TLC59711::GetGbcGreen() const {
	return (m_nFirst32 >> TLC59711_GS_GREEN_SHIFT) & TLC59711_GS_MASK;
}

void TLC59711::SetGbcGreen(uint8_t nValue) {
	m_nFirst32 &= ~(TLC59711_GS_MASK << TLC59711_GS_GREEN_SHIFT);
	m_nFirst32 |= ((nValue & TLC59711_GS_MASK) << TLC59711_GS_GREEN_SHIFT);

	UpdateFirst32();
}

uint8_t TLC59711::GetGbcBlue() const {
	return (m_nFirst32 >> TLC59711_GS_BLUE_SHIFT) & TLC59711_GS_MASK;
}

void TLC59711::SetGbcBlue(uint8_t nValue) {
	m_nFirst32 &= ~(TLC59711_GS_MASK << TLC59711_GS_BLUE_SHIFT);
	m_nFirst32 |= ((nValue & TLC59711_GS_MASK) << TLC59711_GS_BLUE_SHIFT);

	UpdateFirst32();
}

void TLC59711::UpdateFirst32() {
	for (uint32_t i = 0; i < m_nBoards; i++) {
		const auto nIndex = TLC59711Channels::U16BIT * i;
		m_pBuffer[nIndex] = __builtin_bswap16(static_cast<uint16_t>((m_nFirst32 >> 16)));
		m_pBuffer[nIndex + 1] = __builtin_bswap16(static_cast<uint16_t>(m_nFirst32));
	}
}

void TLC59711::Dump() {
#ifndef NDEBUG
	printf("Command:0x%.2X\n", m_nFirst32 >> TLC59711_COMMAND_SHIFT);
	printf("\tOUTTMG:%d (default=%d)\n", GetOnOffTiming(), TLC59711_OUTTMG_DEFAULT);
	printf("\tEXTGCK:%d (default=%d)\n", GetExternalClock(), TLC59711_EXTGCK_DEFAULT);
	printf("\tTMGRST:%d (default=%d)\n", GetDisplayTimingReset(), TLC59711_TMGRST_DEFAULT);
	printf("\tDSPRPT:%d (default=%d)\n", GetDisplayRepeat(), TLC59711_DSPRPT_DEFAULT);
	printf("\tBLANK:%d  (default=%d)\n", GetBlank(),  TLC59711_BLANK_DEFAULT);
	printf("\nGlobal Brightness\n");
	printf("\tRed:0x%.2X (default=0x%.2X)\n", GetGbcRed(), TLC59711_GS_DEFAULT);
	printf("\tGreen:0x%.2X (default=0x%.2X)\n", GetGbcGreen(), TLC59711_GS_DEFAULT);
	printf("\tBlue:0x%.2X (default=0x%.2X)\n", GetGbcBlue(), TLC59711_GS_DEFAULT);
	printf("\nBoards:%d\n", static_cast<int>(m_nBoards));

	uint8_t nOut = 0;

	for (uint32_t i = 0; i < m_nBoards; i++) {
		for (uint32_t j = 0; j < TLC59711_RGB_CHANNELS; j ++) {
			uint16_t nRed = 0, nGreen = 0, nBlue = 0;
			if (GetRgb(nOut, nRed, nGreen, nBlue)) {
				printf("\tOut:%-2d, Red=0x%.4X, Green=0x%.4X, Blue=0x%.4X\n", nOut, nRed, nGreen, nBlue);
			}
			nOut++;
		}
	}

	printf("\n");

	for (uint32_t i = 0; i < m_nBoards * TLC59711Channels::OUT; i++) {
		uint16_t nValue = 0;
		if (Get(i, nValue)) {
			printf("\tChannel:%-3d, Value=0x%.4X\n", static_cast<int>(i), nValue);
		}
	}

	printf("\n");
#endif
}

void TLC59711::Update() {
	assert(m_pBuffer != 0);

	FUNC_PREFIX(spi_chipSelect(SPI_CS_NONE));
	FUNC_PREFIX(spi_set_speed_hz(m_nSpiSpeedHz));
	FUNC_PREFIX(spi_setDataMode(SPI_MODE0));
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(m_pBuffer), m_nBufSize * 2));
}

void TLC59711::Blackout() {
	assert(m_pBufferBlackout != 0);

	FUNC_PREFIX(spi_chipSelect(SPI_CS_NONE));
	FUNC_PREFIX(spi_set_speed_hz(m_nSpiSpeedHz));
	FUNC_PREFIX(spi_setDataMode(SPI_MODE0));
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(m_pBufferBlackout), m_nBufSize * 2));
}
