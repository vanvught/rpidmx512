/**
 * @file diskio.c
 *
 */
/* Copyright (C) 2015-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "device/sd.h"

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

/**
 *
 * @return
 */
static inline int sdcard_init(void){
	if (sd_card_init() == 0) {
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

/**
 *
 * @param buf
 * @param sector
 * @param count
 * @return
 */
static inline int sdcard_read(uint8_t * buf, int sector, int count) {
	size_t buf_size = count * SECTOR_SIZE;

#ifdef CACHE_ENABLED
	if (count == 1) {
		int index = sector & CACHE_MASK;
		void *cache_p = (void *)(cache_buffer + SECTOR_SIZE * index);

		if (cached_blocks[index] != sector) {

			if (sd_read(cache_p, buf_size, (uint32_t) sector) < (int) buf_size) {
				return RES_ERROR;
			}

			cached_blocks[index] = sector;

		} else {
			// take sector from cache
		}

		memcpy_blk((uint32_t *)buf, (uint32_t *)cache_p, SECTOR_SIZE / 32);

	} else {
#endif
		if (sd_read(buf, buf_size, (uint32_t) sector) < (int) buf_size) {
			return RES_ERROR;
		}
#ifdef CACHE_ENABLED
	}
#endif
	return RES_OK;
}

#ifdef SD_WRITE_SUPPORT
/**
 *
 * @param buf
 * @param sector
 * @param count
 * @return
 */
static inline int sdcard_write(const uint8_t * buf, int sector, int count) {
    size_t buf_size = count * SECTOR_SIZE;

    if (sd_write((uint8_t *) buf, buf_size, sector) < buf_size) {
		return RES_ERROR;
	}
#ifdef CACHE_ENABLED
    int i;
    for (i = 0; i < count; i++) {
    	int index = (sector + i) & CACHE_MASK;
    	memcpy_blk((uint32_t *)(cache_buffer + SECTOR_SIZE * index), (uint32_t *)&buf[SECTOR_SIZE * i], SECTOR_SIZE / 32);
    }
#endif
	return RES_OK;
}
#endif

/* disk_initialize
 *
 * Set up the disk.
 */
DSTATUS disk_initialize(BYTE drv) {
	if (drv == (BYTE) 0 && sdcard_init() == 0) {
		diskio_status &= ~STA_NOINIT;
	}

	return diskio_status;
}

/**
 *
 * @param drv
 * @param buf
 * @param sector
 * @param count
 * @return
 */
DRESULT disk_read(BYTE drv, BYTE *buf, DWORD sector, UINT count) {
	if (drv || !count) {
		return RES_PARERR;
	}

	if (diskio_status & STA_NOINIT) {
		return RES_NOTRDY;
	}

	if (sdcard_read((uint8_t *) buf, (int) sector, (int) count) == 0) {
		return RES_OK;
	}

	return RES_ERROR;
}

/**
 *
 * @param drv
 * @param buf
 * @param sector
 * @param count
 * @return
 */
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

/**
 * Check the status of this drive. All we know how to say is "initialized" vs "uninitialized".
 *
 * @param drv
 * @return
 */
DSTATUS disk_status(BYTE drv) {
	if (drv != (BYTE) 0) {
		return (DSTATUS) STA_NOINIT;
	}
	return diskio_status;
}

/**
 *
 * @param drv
 * @param ctrl
 * @param buf
 * @return
 */
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


/**
 * UTC time
 * @return
 */
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
