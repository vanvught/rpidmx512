/**
 * @file flashrom.cpp
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cassert>
#include <unistd.h>

#include "flashrom.h"

#include "debug.h"

namespace flashrom {
static constexpr auto FLASH_SECTOR_SIZE = 4096U;
static constexpr auto FLASH_SIZE = (512 * FLASH_SECTOR_SIZE);
static constexpr char FLASH_FILE_NAME[] = "spiflash.bin";
static constexpr auto BLOCK_WRITE_LENGTH = 1024;

static FILE *pFile;

enum class State {
	IDLE, RUNNING, ERROR
};

static State s_State = State::IDLE;
static uint32_t s_nIndex = 0;
}  // namespace flashrom

using namespace flashrom;

FlashRom *FlashRom::s_pThis;

FlashRom::FlashRom() {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	if ((pFile = fopen(FLASH_FILE_NAME, "r+")) == nullptr) {
		perror("fopen r+");

		pFile = fopen(FLASH_FILE_NAME, "w+");

		for (uint32_t i = 0; i < FLASH_SIZE; i++) {
			if (fputc(0xFF, pFile) == EOF) {
				perror("fputc(0xFF, file)");
				DEBUG_EXIT
				return;
			}
		}

		if (fflush(pFile) != 0) {
			perror("fflush");
		}
	}

	m_IsDetected = true;

	DEBUG_EXIT
}

FlashRom::~FlashRom() {
	DEBUG_ENTRY

	if (pFile != nullptr) {
		fclose(pFile);
		pFile = nullptr;
	}

	DEBUG_EXIT
}

const char *FlashRom::GetName() const  {
	return "SPI Flash None Driver";
}

uint32_t FlashRom::GetSize() const {
	return FLASH_SIZE;
}

uint32_t FlashRom::GetSectorSize() const {
	return FLASH_SECTOR_SIZE;
}

bool FlashRom::Read(uint32_t nOffset, uint32_t nLength, uint8_t *pBuffer, result& nResult) {
	assert(pFile != nullptr);
	DEBUG_ENTRY
	DEBUG_PRINTF("nOffset=%u, nLength=%u", nOffset, nLength);

	if (fseek(pFile, static_cast<long int>(nOffset), SEEK_SET) != 0) {
		nResult = result::ERROR;
		perror("fseek");
		DEBUG_EXIT
		return true;
	}

	if (fread(pBuffer, 1, nLength, pFile) != nLength) {
		nResult = result::ERROR;
		perror("fread");
		DEBUG_EXIT
		return true;
	}

	DEBUG_EXIT
	nResult = result::OK;
	return true;
}

bool FlashRom::Erase(uint32_t nOffset, uint32_t nLength, flashrom::result& nResult) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nOffset=%u, nLength=%u, s_State=%d", nOffset, nLength, static_cast<int>(s_State));
	assert(s_State != State::ERROR);
	assert(pFile != nullptr);

	if (s_State == State::IDLE) {
		if (nOffset % FLASH_SECTOR_SIZE || nLength % FLASH_SECTOR_SIZE) {
			s_State = State::ERROR;
			nResult = result::ERROR;
			DEBUG_PUTS("Erase offset/length not multiple of erase size");
			DEBUG_EXIT
			return true;
		}

		if (fseek(pFile, static_cast<long int>(nOffset), SEEK_SET) != 0) {
			s_State = State::ERROR;
			nResult = result::ERROR;
			perror("fseek");
			DEBUG_EXIT
			return true;
		}

		s_nIndex = 0;
		s_State = State::RUNNING;
		nResult = result::OK;
		DEBUG_EXIT
		return false;
	} else if (s_State == State::RUNNING) {
		for (uint32_t i = 0; i < nLength; i++) {
			if (fputc(0xFF, pFile) == EOF) {
				s_State = State::ERROR;
				nResult = result::ERROR;
				perror("fputc(0xFF, file)");
				DEBUG_EXIT
				return true;
			}
		}

		s_State = State::IDLE;
		nResult = result::OK;

		if (fflush(pFile) != 0) {
			perror("fflush");
		}

		sync();
		DEBUG_EXIT
		return true;
	}

	DEBUG_EXIT
	return true;
}

bool FlashRom::Write(uint32_t nOffset, uint32_t nLength, const uint8_t *pBuffer, flashrom::result& nResult) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nOffset=%u, nLength=%u", nOffset, nLength);
	assert(s_State != State::ERROR);
	assert(pFile != nullptr);

	if (s_State == State::IDLE) {
		if (fseek(pFile, static_cast<long int>(nOffset), SEEK_SET) != 0) {
			s_State = State::ERROR;
			nResult = result::ERROR;
			perror("fseek");
			DEBUG_EXIT
			return true;
		}

		s_nIndex = 0;
		s_State = State::RUNNING;
		nResult = result::OK;
		DEBUG_EXIT
		return false;
	} else if (s_State == State::RUNNING) {
		uint32_t nBlockWriteLength = nLength - s_nIndex;

		if (nBlockWriteLength > BLOCK_WRITE_LENGTH) {
			nBlockWriteLength = BLOCK_WRITE_LENGTH;
		}

		if (fwrite(&pBuffer[s_nIndex], 1, nBlockWriteLength, pFile) != nBlockWriteLength) {
			s_State = State::ERROR;
			nResult = result::ERROR;
			perror("fwrite");
			DEBUG_EXIT
			return true;
		}

		s_nIndex += nBlockWriteLength;

		if (s_nIndex == nLength) {
			s_State = State::IDLE;

			if (fflush(pFile) != 0) {
				perror("fflush");
			}

			sync();
			DEBUG_EXIT
			return true;
		}

		nResult = result::OK;
		DEBUG_EXIT
		return false;
	}

	DEBUG_EXIT
	return true;
}
