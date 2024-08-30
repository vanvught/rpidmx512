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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith" // FIXME ignored "-Wpointer-arith"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "spi/spi_flash.h"

#include "spi_flash_internal.h"

#include "debug.h"

static struct spi_flash s_flash = { "", 0, 0, 0, CMD_READ_STATUS };

#define IDCODE_CONT_LEN 0
#define IDCODE_PART_LEN 5

static const struct {
	const uint8_t shift;
	const uint8_t idcode;
	int(*probe) (struct spi_flash *flash, uint8_t *idcode);
} flashes[] = {
	/* Keep it sorted by define name */
#ifdef CONFIG_SPI_FLASH_ATMEL
	{ 0, 0x1f, spi_flash_probe_atmel, },
#endif
#ifdef CONFIG_SPI_FLASH_EON
	{ 0, 0x1c, spi_flash_probe_eon, },
#endif
#ifdef CONFIG_SPI_FLASH_GIGADEVICE
	{ 0, 0xc8, spi_flash_probe_gigadevice, },
#endif
#ifdef CONFIG_SPI_FLASH_MACRONIX
	{ 0, 0xc2, spi_flash_probe_macronix, },
#endif
#ifdef CONFIG_SPI_FLASH_SPANSION
	{ 0, 0x01, spi_flash_probe_spansion, },
#endif
#ifdef CONFIG_SPI_FLASH_SST
	{ 0, 0xbf, spi_flash_probe_sst, },
#endif
#ifdef CONFIG_SPI_FLASH_STMICRO
	{ 0, 0x20, spi_flash_probe_stmicro, },
#endif
#ifdef CONFIG_SPI_FLASH_WINBOND
	{ 0, 0xef, spi_flash_probe_winbond, },
#endif
#ifdef CONFIG_SPI_FRAM_RAMTRON
	{ 6, 0xc2, spi_fram_probe_ramtron, },
# undef IDCODE_CONT_LEN
# define IDCODE_CONT_LEN 6
#endif
	/* Keep it sorted by best detection */
#ifdef CONFIG_SPI_FLASH_STMICRO
	{ 0, 0xff, spi_flash_probe_stmicro, },
#endif
#ifdef CONFIG_SPI_FRAM_RAMTRON_NON_JEDEC
	{ 0, 0xff, spi_fram_probe_ramtron, },
#endif
};
#define IDCODE_LEN (IDCODE_CONT_LEN + IDCODE_PART_LEN)

static uint32_t get_timer(uint32_t base) {
	if (0 == base) {
		return static_cast<uint32_t>(time(nullptr));
	}

	return static_cast<uint32_t>(time(nullptr)) - base;
}

uint32_t spi_flash_get_size() {
	return s_flash.size;
}

uint32_t spi_flash_get_sector_size() {
	return s_flash.sector_size;
}

const char *spi_flash_get_name() {
	return s_flash.name;
}

static void spi_flash_addr(uint32_t addr, uint8_t *cmd) {
	/* cmd[0] is actual command */
	cmd[1] = static_cast<uint8_t>(addr >> 16);
	cmd[2] = static_cast<uint8_t>(addr >> 8);
	cmd[3] = static_cast<uint8_t>(addr >> 0);
}

static int spi_flash_read_write(const uint8_t *cmd, size_t cmd_len, const uint8_t *data_out, uint8_t *data_in, size_t data_len) {
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;

	if (data_len == 0) {
		flags |= SPI_XFER_END;
	}

	ret = spi_xfer(cmd_len, cmd, nullptr, flags);

	if (data_len != 0) {
		ret = spi_xfer(data_len, data_out, data_in, SPI_XFER_END);
	}

	return ret;
}

static inline int spi_flash_cmd_read(const uint8_t *cmd, size_t cmd_len, uint8_t *data, size_t data_len) {
	return spi_flash_read_write(cmd, cmd_len, nullptr, data, data_len);
}

static inline int spi_flash_cmd(uint8_t cmd, uint8_t *response, size_t len) {
	return spi_flash_cmd_read(&cmd, 1, response, len);
}

static inline int spi_flash_cmd_write(const uint8_t *cmd, size_t cmd_len, const uint8_t *data, size_t data_len) {
	return spi_flash_read_write(cmd, cmd_len, data, nullptr, data_len);
}

static inline int spi_flash_cmd_write_enable() {
	return spi_flash_cmd(CMD_WRITE_ENABLE, nullptr, 0);
}

static int spi_flash_cmd_wait_ready(unsigned long timeout) {
	unsigned long timebase;
	uint8_t status;
	uint8_t check_status = 0x0;
	uint8_t poll_bit = STATUS_WIP;
	uint8_t cmd = s_flash.poll_cmd;

	if (cmd == CMD_FLAG_STATUS) {
		poll_bit = STATUS_PEC;
		check_status = poll_bit;
	}

	spi_xfer(1, &cmd, nullptr, SPI_XFER_BEGIN);

	timebase = get_timer(0);

	do {
		spi_xfer(1, nullptr, &status, 0);

		if ((status & poll_bit) == check_status) {
			break;
		}

	} while (get_timer(timebase) < timeout);

	spi_xfer(0, nullptr, nullptr, SPI_XFER_END);

	if ((status & poll_bit) == check_status) {
		return 0;
	}

	DEBUG_PUTS("time out");
	return -1;
}

static int spi_flash_write_common(const uint8_t *cmd, size_t cmd_len, const uint8_t *buf, size_t buf_len, bool wait_ready) {
	unsigned long timeout = SPI_FLASH_PROG_TIMEOUT;
	int ret;

	if (buf == nullptr) {
		timeout = SPI_FLASH_PAGE_ERASE_TIMEOUT;
	}

	ret = spi_flash_cmd_write_enable();

	if (ret < 0) {
		DEBUG_PUTS("enabling write failed");
		return ret;
	}

	ret = spi_flash_cmd_write(cmd, cmd_len, buf, buf_len);

	if (ret < 0) {
		DEBUG_PUTS("write cmd failed");
		return ret;
	}

	if (wait_ready) {
		ret = spi_flash_cmd_wait_ready(timeout);

		if (ret < 0) {
			DEBUG_PRINTF("write %s timed out", timeout == SPI_FLASH_PROG_TIMEOUT ? "program" : "page erase");
			return ret;
		}
	}

	return ret;
}

int spi_flash_cmd_write_multi(uint32_t offset, size_t len, const uint8_t *buf) {
	DEBUG_ENTRY

	unsigned long byte_addr, page_size;
	size_t chunk_len, actual;
	uint8_t cmd[4];
	int ret = -1;

	page_size = s_flash.page_size;

	cmd[0] = CMD_PAGE_PROGRAM;

	spi_flash_cmd_wait_ready(SPI_FLASH_SECTOR_ERASE_TIMEOUT);

	for (actual = 0; actual < len; actual += chunk_len) {
		byte_addr = offset % page_size;
		chunk_len = min(len - actual, page_size - byte_addr);

		spi_flash_addr(offset, cmd);

		DEBUG_PRINTF("0x%p => cmd = { 0x%02x 0x%02x%02x%02x } actual=%d, chunk_len=%d", buf + actual, cmd[0], cmd[1], cmd[2], cmd[3], static_cast<int>(actual),static_cast<int>(chunk_len));

		ret = spi_flash_write_common(cmd, sizeof(cmd), buf + actual, chunk_len, ((actual + chunk_len) != len));

		if (ret < 0) {
			DEBUG_PUTS("write failed");
			break;
		}

		offset += chunk_len;
	}

	DEBUG_EXIT
	return ret;
}


int spi_flash_read_common(const uint8_t *cmd, size_t cmd_len, uint8_t *data, size_t data_len) {
	int ret;

	ret = spi_flash_cmd_read(cmd, cmd_len, data, data_len);

	return ret;
}

int spi_flash_cmd_read_fast(uint32_t offset, size_t len, uint8_t *data) {
	DEBUG_ENTRY

	uint8_t cmd[5], bank_sel = 0;
	uint32_t remain_len, read_len;
	int ret = -1;

	cmd[0] = CMD_READ_ARRAY_FAST;
	cmd[4] = 0x00;

	spi_flash_cmd_wait_ready(SPI_FLASH_PROG_TIMEOUT);

	while (len) {
		remain_len = (SPI_FLASH_16MB_BOUN * static_cast<uint32_t>(bank_sel + 1) - offset);

		if (len < remain_len) {
			read_len = len;
		}
		else {
			read_len = remain_len;
		}

		spi_flash_addr(offset, cmd);

		ret = spi_flash_read_common(cmd, sizeof(cmd), data, read_len);

		if (ret < 0) {
			DEBUG_PUTS("Read failed");
			DEBUG_EXIT
			break;
		}

		offset += read_len;
		len -= read_len;
		data += read_len;
	}

	DEBUG_EXIT
	return ret;
}

int spi_flash_cmd_erase(uint32_t offset, size_t len) {
	DEBUG_ENTRY

	uint32_t erase_size;
	uint8_t cmd[4];
	int ret = -1;

	erase_size = s_flash.sector_size;

	if (offset % erase_size || len % erase_size) {
		DEBUG_PUTS("Erase offset/length not multiple of erase size");
		DEBUG_EXIT
		return -1;
	}

	if (erase_size == 4096) {
		cmd[0] = CMD_ERASE_4K;
	} else {
		cmd[0] = CMD_ERASE_64K;
	}

	spi_flash_cmd_wait_ready(SPI_FLASH_PROG_TIMEOUT);

	while (len) {
		spi_flash_addr(offset, cmd);

		DEBUG_PRINTF("erase %2x %2x %2x %2x (%x)", cmd[0], cmd[1], cmd[2], cmd[3], offset);

		ret = spi_flash_write_common(cmd, sizeof(cmd), nullptr, 0, (len != erase_size));

		if (ret < 0) {
			DEBUG_PUTS("Erase failed");
			DEBUG_EXIT
			break;
		}

		offset += erase_size;
		len -= erase_size;
	}

	DEBUG_EXIT
	return ret;
}

int spi_flash_cmd_write_status(uint8_t sr) {
	uint8_t cmd;
	int ret;

	cmd = CMD_WRITE_STATUS;
	ret = spi_flash_write_common(&cmd, 1, &sr, 1, false);

	if (ret < 0) {
		DEBUG_PUTS("Fail to write status register");
		return ret;
	}

	return 0;
}

int spi_flash_probe([[maybe_unused]] unsigned int cs, [[maybe_unused]] unsigned int max_hz, [[maybe_unused]] unsigned int spi_mode) {
	int shift;
	unsigned i;
	uint8_t idcode[IDCODE_LEN] = {0, };
	uint8_t *idp;

	if (spi_init() < 0) {
		DEBUG_PUTS("spi_init() failed!");
		return -1;
	}

	/* Read the ID codes */
	spi_flash_cmd(CMD_READ_ID, idcode, sizeof(idcode));

	/* count the number of continuation bytes */
	for (shift = 0, idp = idcode; shift < IDCODE_CONT_LEN && *idp == 0x7f; ++shift, ++idp) {
		continue;
	}

	/* search the table for matches in shift and id */
	for (i = 0; i < ARRAY_SIZE(flashes); ++i) {
		if (flashes[i].shift == shift && flashes[i].idcode == *idp) {
			/* we have a match, call probe */
			if (0 == flashes[i].probe(&s_flash, idp)) {
				break;
			}
		}
	}

	if (i == ARRAY_SIZE(flashes)) {
		DEBUG_PRINTF("Unsupported manufacturer %02x", *idp);
		return -1;
	}

	DEBUG_PRINTF("Detected %s with sector size %d total %d bytes", s_flash.name, s_flash.sector_size, s_flash.size);

	return 0;
}
