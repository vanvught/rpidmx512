/**
 * @file flashcode.cpp
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>

#include "flashcode.h"

#include "gd32.h"

/**
 * With the latest GD32F firmware, this function is declared as static.
 */
#if defined (GD32F20X)
extern "C" {
fmc_state_enum fmc_bank0_state_get(void);
fmc_state_enum fmc_bank1_state_get(void);
}
#endif

#include "debug.h"

namespace flashcode {
/* Backwards compatibility with SPI FLASH */
static constexpr auto FLASH_SECTOR_SIZE = 4096U;
/* The flash page size is 2KB for bank0 */
static constexpr auto BANK0_FLASH_PAGE = (2U * 1024U);
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
static bool s_isBank0;
}  // namespace flashcode

bool static is_bank0(const uint32_t page_address) {
	/* flash size is greater than 512k */
	if (FMC_BANK0_SIZE < FMC_SIZE) {
		if (FMC_BANK0_END_ADDRESS > page_address) {
			return true;
		} else {
			return false;
		}
	}

	return true;
}

using namespace flashcode;

uint32_t FlashCode::GetSize() const {
	return FMC_SIZE * 1024U;
}

uint32_t FlashCode::GetSectorSize() const {
	return FLASH_SECTOR_SIZE;
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

	nResult = result::OK;

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
		if ((s_isBank0 = is_bank0(s_nPage))) {
			fmc_bank0_unlock();
		} else {
			fmc_bank1_unlock();
		}
		s_State = State::ERASE_BUSY;
		DEBUG_PRINTF("isBank0=%d", static_cast<int>(s_isBank0));
		DEBUG_EXIT
		return false;
		break;
	case State::ERASE_BUSY:
		if (s_isBank0) {
			if (FMC_BUSY == fmc_bank0_state_get()) {
				DEBUG_EXIT
				return false;
			}
		} else {
			if (FMC_BUSY == fmc_bank1_state_get()) {
				DEBUG_EXIT
				return false;
			}
		}

		if (s_isBank0) {
			FMC_CTL0 &= ~FMC_CTL0_PER;
		} else {
			FMC_CTL1 &= ~FMC_CTL1_PER;
		}

		if (s_nLength == 0) {
			if (s_isBank0) {
				fmc_bank0_lock();
			} else {
				fmc_bank1_lock();
			}
			s_State = State::IDLE;
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

			if (s_isBank0) {
				FMC_CTL0 |= FMC_CTL0_PER;
				FMC_ADDR0 = s_nPage;
				FMC_CTL0 |= FMC_CTL0_START;

				s_nLength -= BANK0_FLASH_PAGE;
				s_nPage += BANK0_FLASH_PAGE;
			} else {
				FMC_CTL1 |= FMC_CTL1_PER;
				FMC_ADDR1 = s_nPage;
				if (FMC_OBSTAT & FMC_OBSTAT_SPC) {
					FMC_ADDR0 = s_nPage;
				}
				FMC_CTL1 |= FMC_CTL1_START;

				s_nLength -= BANK1_FLASH_PAGE;
				s_nPage += BANK1_FLASH_PAGE;
			}
		}

		s_State = State::ERASE_BUSY;
		DEBUG_EXIT
		return false;
		break;
	case State::WRITE_BUSY:
		if (s_isBank0) {
			FMC_CTL0 &= ~FMC_CTL0_PG;
		} else {
			FMC_CTL1 &= ~FMC_CTL1_PG;
		}
		/*@fallthrough@*/
		/* no break */
	case State::WRITE_PROGRAM:
		s_State = State::IDLE;
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
	nResult = result::OK;

	switch (s_State) {
	case State::IDLE:
		DEBUG_PUTS("State::IDLE");
		s_nAddress = nOffset + FLASH_BASE;
		s_pData = const_cast<uint32_t *>(reinterpret_cast<const uint32_t *>(pBuffer));
		s_nLength = nLength;
		if ((s_isBank0 = is_bank0(s_nAddress))) {
			fmc_bank0_unlock();
		} else {
			fmc_bank1_unlock();
		}
		s_State = State::WRITE_BUSY;
		DEBUG_PRINTF("isBank0=%d", static_cast<int>(s_isBank0));
		DEBUG_EXIT
		return false;
		break;
	case State::WRITE_BUSY:
		if (s_isBank0) {
			if (FMC_BUSY == fmc_bank0_state_get()) {
				DEBUG_EXIT
				return false;
			}
		} else {
			if (FMC_BUSY == fmc_bank1_state_get()) {
				DEBUG_EXIT
				return false;
			}
		}

		if (s_isBank0) {
			FMC_CTL0 &= ~FMC_CTL0_PG;
		} else {
			FMC_CTL1 &= ~FMC_CTL1_PG;
		}

		if (s_nLength == 0) {
			if (s_isBank0) {
				fmc_bank0_lock();
			} else {
				fmc_bank1_lock();
			}
			s_State = State::IDLE;
			DEBUG_EXIT
			return true;
		}

		s_State = State::WRITE_PROGRAM;
		return false;
		break;
	case State::WRITE_PROGRAM:
		if (s_nLength >= 4) {
			if (s_isBank0) {
				FMC_CTL0 |= FMC_CTL0_PG;
			} else {
				FMC_CTL1 |= FMC_CTL1_PG;
			}
			REG32(s_nAddress) = *s_pData;

			s_pData++;
			s_nAddress += 4;
			s_nLength -= 4;
		} else if (s_nLength > 0) {
			if (s_isBank0) {
				FMC_CTL0 |= FMC_CTL0_PG;
			} else {
				FMC_CTL1 |= FMC_CTL1_PG;
			}
			REG32(s_nAddress) = *s_pData;
		}
		s_State = State::WRITE_BUSY;
		return false;
		break;
	case State::ERASE_BUSY:
		if (s_isBank0) {
			FMC_CTL0 &= ~FMC_CTL0_PER;
		} else {
			FMC_CTL1 &= ~FMC_CTL1_PER;
		}
		/*@fallthrough@*/
		/* no break */
	case State::ERASE_PROGAM:
		s_State = State::IDLE;
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
