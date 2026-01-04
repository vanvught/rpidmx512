/**
 * @file spi_flash.cpp
 *
 */
/*
 * Original code : https://github.com/martinezjavier/u-boot/blob/master/drivers/mtd/spi/spi_flash.c
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <algorithm>
#include <time.h>

#include "spi/spi_flash.h"
#include "spi_flash_internal.h"
#include "firmware/debug/debug_dump.h"
 #include "firmware/debug/debug_debug.h"

static struct SpiFlashInfo s_flash = { "", 0, CMD_READ_STATUS };

#define IDCODE_PART_LEN 5

static constexpr struct {
	const uint8_t kIdcode;
	bool(*probe) (struct SpiFlashInfo *flash, uint8_t *idcode);
} kFlashes[] = {
	/* Keep it sorted by define name */
#ifdef CONFIG_SPI_FLASH_GIGADEVICE
	{ 0xc8, SpiFlashProbeGigadevice, },
#endif
#ifdef CONFIG_SPI_FLASH_MACRONIX
	{ 0xc2, SpiFlashProbeMacronix, },
#endif
#ifdef CONFIG_SPI_FLASH_WINBOND
	{ 0xef, SpiFlashProbeWinbond, },
#endif
};

#define IDCODE_LEN IDCODE_PART_LEN

static uint32_t GetTimer(uint32_t base) {
	if (0 == base) {
		return static_cast<uint32_t>(time(nullptr));
	}

	return static_cast<uint32_t>(time(nullptr)) - base;
}

uint32_t spi_flash_get_size() {
	return s_flash.size;
}

const char *spi_flash_get_name() {
	return s_flash.name;
}

static void SpiFlashAddr(uint32_t address, uint8_t *pCommand) {
	/* cmd[0] is actual command */
	pCommand[1] = static_cast<uint8_t>(address >> 16);
	pCommand[2] = static_cast<uint8_t>(address >> 8);
	pCommand[3] = static_cast<uint8_t>(address >> 0);
}

static void SpiFlashReadWrite(const uint8_t *pCommand, uint32_t nCommandLength, const uint8_t *pDataOut, uint8_t *pDataIn, uint32_t nDataLength) {
	uint32_t nFlags = SPI_XFER_BEGIN;

	if (nDataLength == 0) {
		nFlags |= SPI_XFER_END;
	}

	SpiXfer(nCommandLength, pCommand, nullptr, nFlags);

	if (nDataLength != 0) {
		SpiXfer(nDataLength, pDataOut, pDataIn, SPI_XFER_END);
	}
}

static inline void SpiFlashCmdRead(const uint8_t *pCommand, const uint32_t nCommandLength, uint8_t *pData, uint32_t nDataLength) {
	return SpiFlashReadWrite(pCommand, nCommandLength, nullptr, pData, nDataLength);
}

static inline void SpiFlashCmd(uint8_t nCommand, uint8_t *pResponse, uint32_t length) {
	return SpiFlashCmdRead(&nCommand, 1, pResponse, length);
}

static inline void SpiFlashCmdWrite(const uint8_t *pCommand, uint32_t nCommandLength, const uint8_t *pData, uint32_t nDataLength) {
	return SpiFlashReadWrite(pCommand, nCommandLength, pData, nullptr, nDataLength);
}

static inline void SpiFlashCmdWriteEnable() {
	return SpiFlashCmd(CMD_WRITE_ENABLE, nullptr, 0);
}

static bool SpiFlashCmdWaitReady(uint32_t nTimeout) {
	uint8_t cmd = CMD_READ_STATUS;

	SpiXfer(1, &cmd, nullptr, SPI_XFER_BEGIN);

	const auto nTimebase = GetTimer(0);
	uint8_t status;

	do {
		SpiXfer(1, nullptr, &status, 0);

		if ((status & STATUS_WIP) == 0) {
			break;
		}

	} while (GetTimer(nTimebase) < nTimeout);

	SpiXfer(0, nullptr, nullptr, SPI_XFER_END);

	if ((status & STATUS_WIP) == 0) {
		DEBUG_PRINTF("get_timer(nTimebase)=%u", GetTimer(nTimebase));
		DEBUG_EXIT();
		return true;
	}

	DEBUG_PUTS("time out");
	DEBUG_EXIT();
	return false;
}

static bool SpiFlashWriteCommon(const uint8_t *pCommand, const uint32_t nCommandLength, const uint8_t *pData, const uint32_t nDataLength, const bool bWaitReady) {
	uint32_t nTimeout;

	if (pData == nullptr) {
		nTimeout = SPI_FLASH_PAGE_ERASE_TIMEOUT;
	} else {
		nTimeout = SPI_FLASH_PROG_TIMEOUT;
	}

	SpiFlashCmdWriteEnable();
	SpiFlashCmdWrite(pCommand, nCommandLength, pData, nDataLength);

	if (bWaitReady) {
		const auto kRet = SpiFlashCmdWaitReady(nTimeout);

		if (!kRet) {
			DEBUG_PRINTF("write %s timed out", nTimeout == SPI_FLASH_PROG_TIMEOUT ? "program" : "page erase");
			return false;
		}
	}

	return true;
}

bool spi_flash_cmd_write_multi(uint32_t nOffset, uint32_t length, const uint8_t *pData) {
	DEBUG_ENTRY();

	if (!SpiFlashCmdWaitReady(SPI_FLASH_SECTOR_ERASE_TIMEOUT)) {
		DEBUG_EXIT();
		return false;
	}

	uint32_t nChunkLength;
	uint8_t cmd[4];
	cmd[0] = CMD_PAGE_PROGRAM;

	for (uint32_t nActualLength = 0; nActualLength < length; nActualLength += nChunkLength) {
		const auto nByteAddress = nOffset % spi::flash::PAGE_SIZE;
		nChunkLength = std::min((length - nActualLength), (spi::flash::PAGE_SIZE - nByteAddress));

		SpiFlashAddr(nOffset, cmd);

		DEBUG_PRINTF("0x%p => cmd = { 0x%02x 0x%02x%02x%02x } nActualLength=%d, nChunkLength=%d", pData + nActualLength, cmd[0], cmd[1], cmd[2], cmd[3], static_cast<int>(nActualLength),static_cast<int>(nChunkLength));

		const auto kRet = SpiFlashWriteCommon(cmd, sizeof(cmd), pData + nActualLength, nChunkLength, ((nActualLength + nChunkLength) != length));

		if (!kRet) {
			DEBUG_PUTS("write failed");
			DEBUG_EXIT();
			return false;
			break;
		}

		nOffset += nChunkLength;
	}

	DEBUG_EXIT();
	return true;
}

void spi_flash_read_common(const uint8_t *pCommand, uint32_t nCommandLength, uint8_t *pData, const uint32_t nDataLength) {
	return SpiFlashCmdRead(pCommand, nCommandLength, pData, nDataLength);
}

bool spi_flash_cmd_read_fast(uint32_t nOffset, uint32_t length, uint8_t *pData) {
	DEBUG_ENTRY();

	if (!SpiFlashCmdWaitReady(SPI_FLASH_PROG_TIMEOUT)) {
		DEBUG_EXIT();
		return false;
	}

	uint8_t cmd[5];
	cmd[0] = CMD_READ_ARRAY_FAST;
	cmd[4] = 0x00;

	while (length) {
		const auto nRemainLength = SPI_FLASH_16MB_BOUN - nOffset;
		uint32_t nReadLength;

		if (length < nRemainLength) {
			nReadLength = length;
		} else {
			nReadLength = nRemainLength;
		}

		SpiFlashAddr(nOffset, cmd);
		spi_flash_read_common(cmd, sizeof(cmd), pData, nReadLength);

		nOffset += nReadLength;
		length -= nReadLength;
		pData += nReadLength;
	}

	DEBUG_EXIT();
	return true;
}

bool spi_flash_cmd_erase(uint32_t nOffset, uint32_t length) {
	DEBUG_ENTRY();

	if ((nOffset % spi::flash::SECTOR_SIZE) || (length % spi::flash::SECTOR_SIZE)) {
		DEBUG_PUTS("Erase offset/length not multiple of erase size");
		DEBUG_EXIT();
		return false;
	}

	if (!SpiFlashCmdWaitReady(SPI_FLASH_PROG_TIMEOUT)) {
		DEBUG_EXIT();
		return false;
	}

	static_assert(spi::flash::SECTOR_SIZE == 4096);
	uint8_t cmd[4];
	cmd[0] = CMD_ERASE_4K;

	while (length) {
		SpiFlashAddr(nOffset, cmd);

		DEBUG_PRINTF("erase %2x %2x %2x %2x (%x)", cmd[0], cmd[1], cmd[2], cmd[3], nOffset);

		const auto kRet = SpiFlashWriteCommon(cmd, sizeof(cmd), nullptr, 0, (length != spi::flash::SECTOR_SIZE));

		if (!kRet) {
			DEBUG_PUTS("Erase failed");
			DEBUG_EXIT();
			return false;
		}

		nOffset += spi::flash::SECTOR_SIZE;
		length -= spi::flash::SECTOR_SIZE;
	}

	DEBUG_EXIT();
	return true;
}

bool spi_flash_cmd_write_status(uint8_t sr) {
	uint8_t cmd = CMD_WRITE_STATUS;
	const auto kRet = SpiFlashWriteCommon(&cmd, 1, &sr, 1, false);

	if (!kRet) {
		DEBUG_PUTS("Fail to write status register");
		return false;
	}

	return true;
}

bool spi_flash_probe() {
	SpiInit();

	uint8_t idcode[IDCODE_LEN];
	SpiFlashCmd(CMD_READ_ID, idcode, sizeof(idcode));

	debug::Dump(idcode, sizeof(idcode));

	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(kFlashes); ++i) {
		if (kFlashes[i].kIdcode == idcode[0]) {
			if (kFlashes[i].probe(&s_flash, idcode)) {
				break;
			}
		}
	}

	if (i == ARRAY_SIZE(kFlashes)) {
		DEBUG_PRINTF("Unsupported manufacturer %02x", idcode[0]);
		return false;
	}

	DEBUG_PRINTF("Detected %s total %d bytes", s_flash.name, s_flash.size);

	return true;
}
