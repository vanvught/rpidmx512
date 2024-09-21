/**
 * @file spi_internal.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@g32-dmx.org
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

#ifndef SPI_FLASH_INTERNAL_H_
#define SPI_FLASH_INTERNAL_H_

struct spi_flash {
	const char *name;
	/* Total flash size */
	uint32_t size;
	/* Write (page) size */
	uint32_t page_size;
	/* Erase (sector) size */
	uint32_t sector_size;
	/* Poll cmd - for flash erase/program */
	uint8_t poll_cmd;
};

#define min(X, Y)  ((X) < (Y) ? (X) : (Y))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define SPI_FLASH_PROG_TIMEOUT			(2)
#define SPI_FLASH_PAGE_ERASE_TIMEOUT	(5)
#define SPI_FLASH_SECTOR_ERASE_TIMEOUT	(10)

/* Common commands */
#define CMD_READ_ID			0x9f

#define CMD_READ_ARRAY_SLOW		0x03
#define CMD_READ_ARRAY_FAST		0x0b

#define CMD_WRITE_STATUS		0x01
#define CMD_PAGE_PROGRAM		0x02
#define CMD_WRITE_DISABLE		0x04
#define CMD_READ_STATUS			0x05
#define CMD_FLAG_STATUS			0x70
#define CMD_WRITE_ENABLE		0x06
#define CMD_ERASE_4K			0x20
#define CMD_ERASE_32K			0x52
#define CMD_ERASE_64K			0xd8
#define CMD_ERASE_CHIP			0xc7

/* Common status */
#define STATUS_WIP			0x01
#define STATUS_PEC			0x80

#define SPI_FLASH_16MB_BOUN		0x1000000

#define SPI_XFER_BEGIN	0x01	///< Assert CS before transfer
#define SPI_XFER_END	0x02	///< Deassert CS after transfer

#define SPI_XFER_SPEED_HZ	6000000	///< 6MHz

extern int spi_init();
extern int spi_xfer(uint32_t bitlen, const uint8_t *dout, uint8_t *din, uint32_t flags);

//#define CONFIG_SPI_FLASH_MACRONIX
//int spi_flash_probe_macronix(struct spi_flash *flash, uint8_t *idcode);

#define CONFIG_SPI_FLASH_WINBOND
extern int spi_flash_probe_winbond(struct spi_flash *spi, uint8_t *idcode);

//#define CONFIG_SPI_FLASH_GIGADEVICE
//extern int spi_flash_probe_gigadevice(struct spi_flash *spi, uint8_t *idcode);

#endif /* SPI_FLASH_INTERNAL_H_ */
