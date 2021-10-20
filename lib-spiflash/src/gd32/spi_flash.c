/**
 * @file spi_flash.c
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@gd32-dmx.nl
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
#include <stdio.h>
#include <assert.h>

#include "gd32f20x.h"
#include "gd32f20x_fmc.h"

#include "debug.h"

/* The flash page size is 2KB for bank0 */

#define FLASH_PAGE	(2 * 1024)

const char *spi_flash_get_name(void) {
	return "GD32";
}

uint32_t spi_flash_get_size(void) {
	return *(volatile uint16_t*)(0x1FFFF7E0) * 1024;
}

uint32_t spi_flash_get_sector_size(void) {
	return 4096;
}

int spi_flash_probe(__attribute__((unused)) unsigned int cs, __attribute__((unused)) unsigned int max_hz, __attribute__((unused)) unsigned int spi_mode) {
	return 1;
}

int spi_flash_cmd_read_fast(uint32_t offset, size_t len, void *data) {
	DEBUG_ENTRY
	DEBUG_PRINTF("offset=%p[%d], len=%u[%d], data=%p[%d]", offset, (((uint32_t)(offset) & 0x3) == 0), len, (((uint32_t)(len) & 0x3) == 0), data, (((uint32_t)(data) & 0x3) == 0));

	const uint32_t *src = (uint32_t *)(offset + FLASH_BASE);
	uint32_t *dst = (uint32_t *)data;

	while (len > 0) {
		*dst++ = *src++;
		len -= 4;
	}

	debug_dump((uint8_t *)(offset + FLASH_BASE), 64);
	debug_dump(data, 64);

	DEBUG_EXIT
	return 0;
}

int spi_flash_cmd_write_multi(uint32_t offset, size_t len, const void *buf) {
	DEBUG_ENTRY
	DEBUG_PRINTF("offset=%p[%d], len=%u[%d], data=%p[%d]", offset, (((uint32_t)(offset) & 0x3) == 0), len, (((uint32_t)(len) & 0x3) == 0), buf, (((uint32_t)(buf) & 0x3) == 0));

	fmc_bank0_unlock();

	uint32_t address = offset + FLASH_BASE;
	const uint32_t *data = (uint32_t *)buf;

	while (len >= 4) {
		fmc_state_enum state = fmc_word_program(address, *data);

		if (FMC_READY != state) {
			DEBUG_PRINTF("state=%d [%p]", state, address);
			DEBUG_EXIT
			return -state;
		}

		data++;
		address += 4;
		len -= 4;
	}

	if (len > 0) {
		fmc_state_enum state = fmc_word_program(address, *data);

		if (FMC_READY != state) {
			DEBUG_PRINTF("state=%d [%p]", state, address);
			DEBUG_EXIT
			return -state;
		}
	}

	fmc_bank0_lock();

	debug_dump(buf, 64);
	debug_dump((uint8_t *)(offset + FLASH_BASE), 64);

	DEBUG_EXIT
	return 0;
}

int spi_flash_cmd_erase(uint32_t offset, size_t len) {
	DEBUG_ENTRY
	DEBUG_PRINTF("offset=%p[%d], len=%x[%d]", offset, (((uint32_t)(offset) & 0x3) == 0), len, (((uint32_t)(len) & 0x3) == 0));

	uint32_t page = offset + FLASH_BASE;

	fmc_bank0_unlock();

	while (len > 0) {
		DEBUG_PRINTF("page=%p", page);

		fmc_state_enum state = fmc_page_erase(page);

		if (FMC_READY != state) {
			DEBUG_PRINTF("state=%d", state);
			DEBUG_EXIT
			return -state;
		}

		len -= FLASH_PAGE;
		page += FLASH_PAGE;
	}

	fmc_bank0_lock();

	DEBUG_EXIT
	return 0;
}
