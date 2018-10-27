/**
 * @file diskio.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

/*
 * Based on code from NXP app note AN10916.
 */

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#include "diskio.h"

#include "mmc_internal.h"

#define SD_WRITE_SUPPORT
#define CACHE_ENABLED

#if (_MAX_SS != _MIN_SS) && (_MAX_SS != 512)
#error Wrong sector size configuration
#endif
#define SECTOR_SIZE	512

#ifdef CACHE_ENABLED
#define CACHE_ENTRIES	(1 << 4)			///< 16 entries
#define CACHE_MASK		(CACHE_ENTRIES - 1)	///< mask 0x0F

static uint32_t cached_blocks[CACHE_ENTRIES] __attribute__((aligned(4)));
static uint8_t cache_buffer[SECTOR_SIZE * CACHE_ENTRIES] __attribute__((aligned(SECTOR_SIZE)));

#include "arm/arm.h"
#endif

static volatile BYTE diskio_status = (BYTE) STA_NOINIT;

extern int sunxi_mmc_init(void);
extern int mmc_read_blocks(struct mmc *mmc, void *dst, unsigned long start, unsigned blkcnt);
#ifdef SD_WRITE_SUPPORT
extern unsigned mmc_write_blocks(struct mmc *mmc, unsigned long start, unsigned blkcnt, const void *src);
#endif

static inline int sdcard_init(void){
	if (sunxi_mmc_init() > 0) {
#ifdef CACHE_ENABLED
		int i;
		for (i = 0; i < CACHE_ENTRIES; i++) {
			cached_blocks[i] = 0xFFFFFFFF;
		}
#endif
		return RES_OK;
	}

	return RES_ERROR;
}

static inline int sdcard_read(uint8_t* buf, int sector, int count) {
	struct mmc *mmc = find_mmc_device(0);

#ifdef CACHE_ENABLED
	if (count == 1) {
		const int index = sector & CACHE_MASK;
		const void *cache_p = (void *)(cache_buffer + SECTOR_SIZE * index);

		if (cached_blocks[index] != sector) {
			// Store sector in cache
			if (mmc_read_blocks(mmc, (void *)cache_p, sector, count) != count) {
				return RES_ERROR;
			}

			cached_blocks[index] = sector;
		}

		memcpy_blk((uint32_t *)buf, (uint32_t *)cache_p, SECTOR_SIZE / 32);

	} else {
#endif
		if (mmc_read_blocks(mmc, (void *)buf, sector, count) != count) {
			return RES_ERROR;
		}
#ifdef CACHE_ENABLED
	}
#endif
	return RES_OK;
}

#ifdef SD_WRITE_SUPPORT
static inline int sdcard_write(const uint8_t* buf, int sector, int count) {
    struct mmc *mmc = find_mmc_device(0);

    if (mmc_write_blocks(mmc, (unsigned long) sector, count, (const void *)buf) != count) {
		return RES_ERROR;
	}

#ifdef CACHE_ENABLED
    int i;
    for (i = 0; i < count; i++) {
    	const int index = (sector + i) & CACHE_MASK;
    	cached_blocks[index] = sector + i;
    	memcpy_blk((uint32_t *)(cache_buffer + SECTOR_SIZE * index), (uint32_t *)&buf[SECTOR_SIZE * i], SECTOR_SIZE / 32);
    }
#endif
	return RES_OK;
}
#endif

DSTATUS disk_initialize(BYTE drv) {
	if (drv == (BYTE) 0 && sdcard_init() == 0) {
		diskio_status &= ~STA_NOINIT;
	}

	return diskio_status;
}

DRESULT disk_read(BYTE drv, BYTE *buf, DWORD sector, UINT count) {
	if (drv || !count) {
		return RES_PARERR;
	}

	if (diskio_status & STA_NOINIT) {
		return RES_NOTRDY;
	}

	return sdcard_read((uint8_t *) buf, (int) sector, (int) count);
}

DRESULT disk_write(BYTE drv, const BYTE *buf, DWORD sector, UINT count) {
#ifdef SD_WRITE_SUPPORT
	if (drv || !count) {
		return RES_PARERR;
	}

	if (diskio_status & STA_NOINIT) {
		return RES_NOTRDY;
	}

	if (sdcard_write(buf, sector, count) == 0) {
		return RES_OK;
	}

	return RES_ERROR;
#else
	return RES_OK;
#endif
}

DSTATUS disk_status(BYTE drv) {
	if (drv != (BYTE) 0) {
		return (DSTATUS) STA_NOINIT;
	}
	return diskio_status;
}

DRESULT disk_ioctl(/*@unused@*/BYTE drv, BYTE ctrl, void *buf) {
	switch (ctrl) {
	case CTRL_SYNC:
		return RES_OK;
		break;
	case GET_SECTOR_SIZE:
		*(DWORD *) buf = (DWORD) SECTOR_SIZE;
		return RES_OK;
		break;
	case GET_BLOCK_SIZE:
		*(DWORD *) buf = (DWORD) SECTOR_SIZE;
		return RES_OK;
		break;
	default:
		return RES_PARERR;
		break;
	}
	return RES_OK;
}

DWORD get_fattime(void) {
	time_t ltime;
	struct tm *local_time;
	DWORD packed_time;

	ltime = time(NULL);
	local_time = localtime(&ltime);

	packed_time = ((DWORD) (local_time->tm_year + 20) << 25)
			| ((DWORD) (local_time->tm_mon + 1) << 21)
			| ((DWORD) local_time->tm_mday << 16)
			| ((DWORD) local_time->tm_hour << 11)
			| ((DWORD) local_time->tm_min << 5)
			| ((DWORD) local_time->tm_sec >> 1);

	return packed_time;
}
