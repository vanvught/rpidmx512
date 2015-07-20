/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include "bcm2835_emmc.h"
#include "sys_time.h"
#include "diskio.h"

volatile BYTE Stat = STA_NOINIT;

static struct emmc_block_dev *emmc_dev __attribute__((aligned(4)));

extern int sd_card_init(struct block_device **dev);
extern int sd_read(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t block_no);
extern int sd_write(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t block_no);

static inline int sdcard_init(void){
	if (sd_card_init((struct block_device **)&emmc_dev) == 0) {
		return RES_OK;
	}

	return RES_ERROR;
}

static inline int sdcard_read(uint8_t * buf, int sector, int count) {
	size_t buf_size = count * emmc_dev->bd.block_size;

	if (sd_read((struct block_device *)emmc_dev, buf, buf_size, sector) < buf_size) {
		return RES_ERROR;
	}

	return RES_OK;
}

static inline int sdcard_write(const uint8_t * buf, int sector, int count) {
    size_t buf_size = count * emmc_dev->bd.block_size;

    if (sd_write((struct block_device *)emmc_dev, (uint8_t *) buf, buf_size, sector) < buf_size) {
		return RES_ERROR;
	}

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
	switch (ctrl) {
		case CTRL_SYNC:
			return RES_OK;
			break;
		case GET_SECTOR_COUNT:
			*(DWORD *)buf = emmc_dev->bd.num_blocks;
			return RES_OK;
			break;
		case GET_SECTOR_SIZE:
			*(DWORD *)buf = emmc_dev->bd.block_size;
			return RES_OK;
			break;
		case GET_BLOCK_SIZE:
			*(DWORD *)buf = emmc_dev->bd.block_size;
			return RES_OK;
			break;
		default:
			return RES_PARERR;
			break;
	}
	return RES_OK;
}

/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
DWORD get_fattime (void)
{
	time_t ltime = 0;
	struct tm *local_time = NULL;

	ltime = sys_time(NULL);
    local_time = localtime(&ltime);

    return   ((DWORD)(local_time->tm_year - 80) << 25)
           | ((DWORD)(local_time->tm_mon + 1) << 21)
           | ((DWORD)local_time->tm_mday << 16)
           | ((DWORD)local_time->tm_hour << 11)
           | ((DWORD)local_time->tm_min << 5)
           | ((DWORD)local_time->tm_sec >> 1);

}
