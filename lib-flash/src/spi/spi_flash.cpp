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
#include <cstdio>
#include <algorithm>
#include <time.h>

#include "spi/spi_flash.h"
#include "spi_flash_internal.h"

#include "debug.h"

static struct SpiFlashInfo s_flash = { "", 0, CMD_READ_STATUS };

#define IDCODE_PART_LEN 5

static constexpr struct {
	const uint8_t idcode;
	bool(*probe) (struct SpiFlashInfo *flash, uint8_t *idcode);
} flashes[] = {
	/* Keep it sorted by define name */
#ifdef CONFIG_SPI_FLASH_GIGADEVICE
	{ 0xc8, spi_flash_probe_gigadevice, },
#endif
#ifdef CONFIG_SPI_FLASH_MACRONIX
	{ 0xc2, spi_flash_probe_macronix, },
#endif
#ifdef CONFIG_SPI_FLASH_WINBOND
	{ 0xef, spi_flash_probe_winbond, },
#endif
};

#define IDCODE_LEN IDCODE_PART_LEN

static uint32_t get_timer(const uint32_t nBase) {
	if (0 == nBase) {
		return static_cast<uint32_t>(time(nullptr));
	}

	return static_cast<uint32_t>(time(nullptr)) - nBase;
}

uint32_t spi_flash_get_size() {
	return s_flash.size;
}

const char *spi_flash_get_name() {
	return s_flash.name;
}

static void spi_flash_addr(const uint32_t nAddress, uint8_t *pCommand) {
	/* cmd[0] is actual command */
	pCommand[1] = static_cast<uint8_t>(nAddress >> 16);
	pCommand[2] = static_cast<uint8_t>(nAddress >> 8);
	pCommand[3] = static_cast<uint8_t>(nAddress >> 0);
}

static void spi_flash_read_write(const uint8_t *pCommand, const uint32_t nCommandLength, const uint8_t *pDataOut, uint8_t *pDataIn, const uint32_t nDataLength) {
	uint32_t nFlags = SPI_XFER_BEGIN;

	if (nDataLength == 0) {
		nFlags |= SPI_XFER_END;
	}

	spi_xfer(nCommandLength, pCommand, nullptr, nFlags);

	if (nDataLength != 0) {
		spi_xfer(nDataLength, pDataOut, pDataIn, SPI_XFER_END);
	}
}

static inline void spi_flash_cmd_read(const uint8_t *pCommand, const uint32_t nCommandLength, uint8_t *pData, const uint32_t nDataLength) {
	return spi_flash_read_write(pCommand, nCommandLength, nullptr, pData, nDataLength);
}

static inline void spi_flash_cmd(uint8_t nCommand, uint8_t *pResponse, const uint32_t nLength) {
	return spi_flash_cmd_read(&nCommand, 1, pResponse, nLength);
}

static inline void spi_flash_cmd_write(const uint8_t *pCommand, const uint32_t nCommandLength, const uint8_t *pData, const uint32_t nDataLength) {
	return spi_flash_read_write(pCommand, nCommandLength, pData, nullptr, nDataLength);
}

static inline void spi_flash_cmd_write_enable() {
	return spi_flash_cmd(CMD_WRITE_ENABLE, nullptr, 0);
}

static bool spi_flash_cmd_wait_ready(const uint32_t nTimeout) {
	uint8_t cmd = CMD_READ_STATUS;

	spi_xfer(1, &cmd, nullptr, SPI_XFER_BEGIN);

	const auto nTimebase = get_timer(0);
	uint8_t status;

	do {
		spi_xfer(1, nullptr, &status, 0);

		if ((status & STATUS_WIP) == 0) {
			break;
		}

	} while (get_timer(nTimebase) < nTimeout);

	spi_xfer(0, nullptr, nullptr, SPI_XFER_END);

	if ((status & STATUS_WIP) == 0) {
		DEBUG_PRINTF("get_timer(nTimebase)=%u", get_timer(nTimebase));
		DEBUG_EXIT
		return true;
	}

	DEBUG_PUTS("time out");
	DEBUG_EXIT
	return false;
}

static bool spi_flash_write_common(const uint8_t *pCommand, const uint32_t nCommandLength, const uint8_t *pData, const uint32_t nDataLength, const bool bWaitReady) {
	uint32_t nTimeout;

	if (pData == nullptr) {
		nTimeout = SPI_FLASH_PAGE_ERASE_TIMEOUT;
	} else {
		nTimeout = SPI_FLASH_PROG_TIMEOUT;
	}

	spi_flash_cmd_write_enable();
	spi_flash_cmd_write(pCommand, nCommandLength, pData, nDataLength);

	if (bWaitReady) {
		const auto ret = spi_flash_cmd_wait_ready(nTimeout);

		if (!ret) {
			DEBUG_PRINTF("write %s timed out", nTimeout == SPI_FLASH_PROG_TIMEOUT ? "program" : "page erase");
			return false;
		}
	}

	return true;
}

bool spi_flash_cmd_write_multi(uint32_t nOffset, const uint32_t nLength, const uint8_t *pData) {
	DEBUG_ENTRY

	if (!spi_flash_cmd_wait_ready(SPI_FLASH_SECTOR_ERASE_TIMEOUT)) {
		DEBUG_EXIT
		return false;
	}

	uint32_t nChunkLength;
	uint8_t cmd[4];
	cmd[0] = CMD_PAGE_PROGRAM;

	for (uint32_t nActualLength = 0; nActualLength < nLength; nActualLength += nChunkLength) {
		const auto nByteAddress = nOffset % spi_flash::PAGE_SIZE;
		nChunkLength = std::min((nLength - nActualLength), (spi_flash::PAGE_SIZE - nByteAddress));

		spi_flash_addr(nOffset, cmd);

		DEBUG_PRINTF("0x%p => cmd = { 0x%02x 0x%02x%02x%02x } nActualLength=%d, nChunkLength=%d", pData + nActualLength, cmd[0], cmd[1], cmd[2], cmd[3], static_cast<int>(nActualLength),static_cast<int>(nChunkLength));

		const auto ret = spi_flash_write_common(cmd, sizeof(cmd), pData + nActualLength, nChunkLength, ((nActualLength + nChunkLength) != nLength));

		if (!ret) {
			DEBUG_PUTS("write failed");
			DEBUG_EXIT
			return false;
			break;
		}

		nOffset += nChunkLength;
	}

	DEBUG_EXIT
	return true;
}

void spi_flash_read_common(const uint8_t *pCommand, const uint32_t nCommandLength, uint8_t *pData, const uint32_t nDataLength) {
	return spi_flash_cmd_read(pCommand, nCommandLength, pData, nDataLength);
}

bool spi_flash_cmd_read_fast(uint32_t nOffset, uint32_t nLength, uint8_t *pData) {
	DEBUG_ENTRY

	if (!spi_flash_cmd_wait_ready(SPI_FLASH_PROG_TIMEOUT)) {
		DEBUG_EXIT
		return false;
	}

	uint8_t cmd[5];
	cmd[0] = CMD_READ_ARRAY_FAST;
	cmd[4] = 0x00;

	while (nLength) {
		const auto nRemainLength = SPI_FLASH_16MB_BOUN - nOffset;
		uint32_t nReadLength;

		if (nLength < nRemainLength) {
			nReadLength = nLength;
		} else {
			nReadLength = nRemainLength;
		}

		spi_flash_addr(nOffset, cmd);
		spi_flash_read_common(cmd, sizeof(cmd), pData, nReadLength);

		nOffset += nReadLength;
		nLength -= nReadLength;
		pData += nReadLength;
	}

	DEBUG_EXIT
	return true;
}

bool spi_flash_cmd_erase(uint32_t nOffset, uint32_t nLength) {
	DEBUG_ENTRY

	if ((nOffset % spi_flash::SECTOR_SIZE) || (nLength % spi_flash::SECTOR_SIZE)) {
		DEBUG_PUTS("Erase offset/length not multiple of erase size");
		DEBUG_EXIT
		return false;
	}

	if (!spi_flash_cmd_wait_ready(SPI_FLASH_PROG_TIMEOUT)) {
		DEBUG_EXIT
		return false;
	}

	static_assert(spi_flash::SECTOR_SIZE == 4096);
	uint8_t cmd[4];
	cmd[0] = CMD_ERASE_4K;

	while (nLength) {
		spi_flash_addr(nOffset, cmd);

		DEBUG_PRINTF("erase %2x %2x %2x %2x (%x)", cmd[0], cmd[1], cmd[2], cmd[3], nOffset);

		const auto ret = spi_flash_write_common(cmd, sizeof(cmd), nullptr, 0, (nLength != spi_flash::SECTOR_SIZE));

		if (!ret) {
			DEBUG_PUTS("Erase failed");
			DEBUG_EXIT
			return false;
		}

		nOffset += spi_flash::SECTOR_SIZE;
		nLength -= spi_flash::SECTOR_SIZE;
	}

	DEBUG_EXIT
	return true;
}

bool spi_flash_cmd_write_status(uint8_t sr) {
	uint8_t cmd = CMD_WRITE_STATUS;
	const auto ret = spi_flash_write_common(&cmd, 1, &sr, 1, false);

	if (!ret) {
		DEBUG_PUTS("Fail to write status register");
		return false;
	}

	return true;
}

bool spi_flash_probe() {
	spi_init();

	uint8_t idcode[IDCODE_LEN];
	spi_flash_cmd(CMD_READ_ID, idcode, sizeof(idcode));

	debug_dump(idcode, sizeof(idcode));

	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(flashes); ++i) {
		if (flashes[i].idcode == idcode[0]) {
			if (flashes[i].probe(&s_flash, idcode)) {
				break;
			}
		}
	}

	if (i == ARRAY_SIZE(flashes)) {
		DEBUG_PRINTF("Unsupported manufacturer %02x", idcode[0]);
		return false;
	}

	DEBUG_PRINTF("Detected %s total %d bytes", s_flash.name, s_flash.size);

	return true;
}
