/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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
#include <stdlib.h>
//
#include "ff.h"
#include "diskio.h"

volatile BYTE Stat = STA_NOINIT;

#include <block.h>

// Space for 512 x 512 byte cache areas
#define BLOCK_CACHE_SIZE	0x40000		// 256kB

static struct block_device *sd_dev = NULL;
static struct block_device *c_dev = NULL;

void* sd_aligned_malloc(size_t required_bytes, size_t alignment) {
	void* p1; // original block
	void** p2; // aligned block
	int offset = alignment - 1 + sizeof(void*);
	if ((p1 = (void*) malloc(required_bytes + offset)) == NULL ) {
		return NULL ;
	}
	p2 = (void**) (((size_t) (p1) + offset) & ~(alignment - 1));
	p2[-1] = p1;
	return p2;
}

void sd_aligned_free(void *p) {
	free(((void**) p)[-1]);
}

extern int sd_card_init(struct block_device **dev);
extern int cache_init(struct block_device *parent, struct block_device **dev, uintptr_t cache_start, size_t cache_length);
extern int cache_read(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t starting_block);

static inline int sdcard_init(void){
	if (sd_card_init(&sd_dev) == 0) {
		c_dev = sd_dev;
		void *cache_start = sd_aligned_malloc(BLOCK_CACHE_SIZE, 512);
		if(cache_start != 0)
			cache_init(sd_dev, &c_dev, (uintptr_t)cache_start, BLOCK_CACHE_SIZE);
		return RES_OK;
	}

	return RES_ERROR;
}

static inline int sdcard_read(uint8_t * buf, int sector, int count) {
	size_t buf_size = count * sd_dev->block_size;

	struct block_device *dev = (struct block_device *)c_dev;
	if(cache_read(dev, buf, buf_size, sector) < 0)
        return RES_ERROR;
	return RES_OK;
}

static inline int sdcard_write(const uint8_t * buf, int sector, int count) {
	return RES_OK;
}

/* disk_initialize
 *
 * Set up the disk.
 */
DSTATUS disk_initialize (BYTE drv) {
	if (drv == 0 && sdcard_init() == 0) {
		Stat &= ~STA_NOINIT;
	}

	return Stat;
}

/* disk_read
 *
 * Read some sectors.
 */
DRESULT disk_read (BYTE drv, BYTE *buf, DWORD sector, BYTE count) {
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (sdcard_read(buf, sector, count) == 0)
		return RES_OK;
	else
		return RES_ERROR;
}

/* disk_write
 *
 * Write some sectors.
 */
DRESULT disk_write (BYTE drv, const BYTE *buf,	DWORD sector, BYTE count) {
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (sdcard_write(buf, sector, count) == 0)
		return RES_OK;
	else
		return 	RES_ERROR;
}

/* disk_status
 *
 * Check the status of this drive. All we know how to say is "initialized"
 * vs "uninitialized".
 */
DSTATUS disk_status (BYTE drv) {
	if (drv) return STA_NOINIT;
	return Stat;
}

/* disk_ioctl
 *
 * Everything else.
 */
DRESULT disk_ioctl (BYTE drv, BYTE ctrl, void *buf) {
	return RES_OK;
}

/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
DWORD get_fattime (void)
{
	return	  ((DWORD)(2015 - 1980) << 25)	/* Year = 2015 */
			| ((DWORD)1 << 21)				/* Month = 1 */
			| ((DWORD)1 << 16)				/* Day_m = 1*/
			| ((DWORD)0 << 11)				/* Hour = 0 */
			| ((DWORD)0 << 5)				/* Min = 0 */
			| ((DWORD)0 >> 1);				/* Sec = 0 */
}
