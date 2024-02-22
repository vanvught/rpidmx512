/**
 * @file flashcode.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <stdio.h>
#include <cstring>
#include <cassert>

#include "flashcode.h"

#include "gd32.h"

#include "debug.h"

namespace flashcode {
/* Backwards compatibility with SPI FLASH */
static constexpr auto FLASH_SECTOR_SIZE = 4096U;
/* The flash page size is 4KB for bank1 */
static constexpr auto BANK1_FLASH_PAGE = (4U * 1024U);

enum class State {
	IDLE,
	ERASE_BUSY,
	ERASE_PROGAM,
	WRITE_BUSY,
	WRITE_PROGRAM,
	ERROR
};

static State s_State = State::IDLE;
static uint32_t s_nPage;
static uint32_t s_nLength;
static uint32_t s_nAddress;
static uint32_t *s_pData;
}  // namespace flashcode

using namespace flashcode;

uint32_t FlashCode::GetSize() const {
	const auto FLASH_DENSITY = ((REG32(0x1FF0F7E0) >> 16) & 0xFFFF) * 1024U;
	return FLASH_DENSITY;
}

uint32_t FlashCode::GetSectorSize() const {
	return flashcode::FLASH_SECTOR_SIZE;
}

bool FlashCode::Read(uint32_t nOffset, uint32_t nLength, uint8_t *pBuffer, flashcode::result& nResult) {
	DEBUG_ENTRY
	DEBUG_PRINTF("offset=%p[%d], len=%u[%d], data=%p[%d]", nOffset, (((uint32_t)(nOffset) & 0x3) == 0), nLength, (((uint32_t)(nLength) & 0x3) == 0), pBuffer, (((uint32_t)(pBuffer) & 0x3) == 0));

	const auto *pSrc = reinterpret_cast<uint32_t *>(nOffset + FLASH_BASE);
	auto *pDst = reinterpret_cast<uint32_t *>(pBuffer);

	while (nLength > 0) {
		*pDst++ = *pSrc++;
		nLength -= 4;
	}

	nResult = flashcode::result::OK;

	DEBUG_EXIT
	return true;
}

bool FlashCode::Erase(uint32_t nOffset, uint32_t nLength, flashcode::result& nResult) {
	DEBUG_ENTRY
	DEBUG_PRINTF("State=%d", static_cast<int>(s_State));

	nResult = result::OK;

	switch (s_State) {
	case State::IDLE:
		s_nPage = nOffset + FLASH_BASE;
		s_nLength = nLength;
		fmc_unlock();
		s_State = State::ERASE_BUSY;
		DEBUG_EXIT
		return false;
		break;
	case State::ERASE_BUSY:
		if (SET == fmc_flag_get(FMC_FLAG_BUSY)) {
			DEBUG_EXIT
			return false;
		}

		if (s_nLength == 0) {
			s_State = State::IDLE;
			fmc_lock();
			DEBUG_EXIT
			return true;
		}

		s_State = State::ERASE_PROGAM;
		DEBUG_EXIT
		return false;
		break;
	case State::ERASE_PROGAM:
		if (s_nLength > 0) {
			DEBUG_PRINTF("s_nPage=%p", s_nPage);

			fmc_sector_erase(s_nPage);

			s_nLength -= BANK1_FLASH_PAGE;
			s_nPage += BANK1_FLASH_PAGE;
		}

		s_State = State::ERASE_BUSY;
		DEBUG_EXIT
		return false;
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	assert(0);
	__builtin_unreachable();
	return true;
}

bool FlashCode::Write(uint32_t nOffset, uint32_t nLength, const uint8_t *pBuffer, flashcode::result& nResult) {
	if ((s_State == flashcode::State::WRITE_PROGRAM) || (s_State == flashcode::State::WRITE_BUSY)) {
	} else {
		DEBUG_ENTRY
	}
	nResult = result::OK;

	switch (s_State) {
	case flashcode::State::IDLE:
		DEBUG_PUTS("State::IDLE");
		flashcode::s_nAddress = nOffset + FLASH_BASE;
		s_pData = const_cast<uint32_t *>(reinterpret_cast<const uint32_t *>(pBuffer));
		s_nLength = nLength;
		fmc_unlock();
		s_State = State::WRITE_BUSY;
		DEBUG_EXIT
		return false;
		break;
	case flashcode::State::WRITE_BUSY:
		if (SET == fmc_flag_get(FMC_FLAG_BUSY)) {
			DEBUG_EXIT
			return false;
		}

		if (s_nLength == 0) {
			fmc_lock();
			s_State = State::IDLE;

			if( memcmp(reinterpret_cast<void *>(nOffset + FLASH_BASE), pBuffer, nLength) == 0) {
				DEBUG_PUTS("memcmp OK");
			} else {
				DEBUG_PUTS("memcmp failed");
			}

			DEBUG_EXIT
			return true;
		}

		s_State = flashcode::State::WRITE_PROGRAM;
		return false;
		break;
	case flashcode::State::WRITE_PROGRAM:
		if (s_nLength >= 4) {
			if (FMC_READY == fmc_ready_wait(0xFF)) {
				/* set the PG bit to start program */
				FMC_CTL |= FMC_CTL_PG;
				__ISB();
				__DSB();
				REG32(s_nAddress) = *s_pData;
				__ISB();
				__DSB();
		        /* reset the PG bit */
		        FMC_CTL &= ~FMC_CTL_PG;
				s_pData++;
				s_nAddress += 4;
				s_nLength -= 4;
			}
		} else if (s_nLength > 0) {
			DEBUG_PUTS("Error!");
		}
		s_State = flashcode::State::WRITE_BUSY;
		return false;
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	assert(0);
	__builtin_unreachable();
	return true;
}
