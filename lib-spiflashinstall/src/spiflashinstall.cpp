/**
 * @file spiflashinstall.cpp
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "spiflashinstall.h"
#include "spiflashinstallparams.h"

#include "display.h"

#include "spi_flash.h"

#include "hardware.h"

#include "debug.h"

#ifndef ALIGNED
 #define ALIGNED 	__attribute__((aligned(4)))
#endif

#define OFFSET_UBOOT_SPI	0x000000
#define OFFSET_UIMAGE		0x180000

#define COMPARE_BYTES		1024

#define FLASH_SIZE_MINIMUM	0x200000

static const char sFileUbootSpi[] ALIGNED = "uboot.spi";
static const char sFileuImage[] ALIGNED = "uImage";

static const char sWriting[] ALIGNED = "Writing";
static const char sCheckDifference[] ALIGNED = "Check difference";
static const char sNoDifference[] ALIGNED = "No difference";
static const char sDone[] ALIGNED = "Done";

SpiFlashInstall *SpiFlashInstall::s_pThis = 0;

SpiFlashInstall::SpiFlashInstall(void):
	m_bHaveFlashChip(false),
	m_nEraseSize(0),
	m_nFlashSize(0),
	m_pFileBuffer(0),
	m_pFlashBuffer(0),
	m_pFile(0)
{
	DEBUG_ENTRY

	s_pThis = this;

	Display::Get()->Cls();

	if (spi_flash_probe(0, 0, 0) < 0) {
		Display::Get()->TextStatus("No SPI flash", DISPLAY_7SEGMENT_MSG_INFO_SPI_NONE);
		DEBUG_PUTS("No SPI flash chip");
	} else {
		m_nFlashSize = spi_flash_get_size();

		printf("%s, sector size %d, %d bytes\n", spi_flash_get_name(), spi_flash_get_sector_size(), m_nFlashSize);
		Display::Get()->Write(1, spi_flash_get_name());
	}

	if (Hardware::Get()->GetBootDevice() == BOOT_DEVICE_MMC0) {
		DEBUG_PUTS("BOOT_DEVICE_MMC0");

		SpiFlashInstallParams params;

		if (params.Load()) {
			params.Dump();

			if (m_nFlashSize >= FLASH_SIZE_MINIMUM) {

				m_bHaveFlashChip = true;
				m_nEraseSize = spi_flash_get_sector_size();

				m_pFileBuffer = new uint8_t[m_nEraseSize];
				assert(m_pFileBuffer != 0);

				m_pFlashBuffer = new uint8_t[m_nEraseSize];
				assert(m_pFlashBuffer != 0);

				if (params.GetInstalluboot()) {
					Process(sFileUbootSpi, OFFSET_UBOOT_SPI);
				}

				if (params.GetInstalluImage()) {
					Process(sFileuImage, OFFSET_UIMAGE);
				}
			}
		}
	}

	DEBUG_EXIT
}

SpiFlashInstall::~SpiFlashInstall(void) {
	DEBUG_ENTRY

	if (m_pFileBuffer != 0) {
		delete[] m_pFileBuffer;
	}

	if (m_pFlashBuffer != 0) {
		delete[] m_pFlashBuffer;
	}

	DEBUG_EXIT
}

void SpiFlashInstall::Process(const char *pFileName, uint32_t nOffset) {
	if (Open(pFileName)) {
		Display::Get()->TextStatus(sCheckDifference, DISPLAY_7SEGMENT_MSG_INFO_SPI_CHECK);
		puts(sCheckDifference);

		if (Diff(nOffset)) {
			Display::Get()->TextStatus(sWriting, DISPLAY_7SEGMENT_MSG_INFO_SPI_WRITING);
			puts(sWriting);
			Write(nOffset);
		} else {
			Display::Get()->TextStatus(sNoDifference, DISPLAY_7SEGMENT_MSG_INFO_SPI_NODIFF);
			puts(sNoDifference);
		}
		Close();
	}
}

bool SpiFlashInstall::Open(const char* pFileName) {
	DEBUG_ENTRY

	assert(pFileName != 0);
	assert(m_pFile == 0);

	m_pFile = fopen(pFileName, "r");

	if (m_pFile == NULL) {
		printf("Could not open file: %s\n", pFileName);
		DEBUG_EXIT
		return false;
	}

	Display::Get()->ClearLine(2);
	Display::Get()->Write(2, pFileName);
	puts(pFileName);

	DEBUG_EXIT
	return true;
}

void SpiFlashInstall::Close(void) {
	DEBUG_ENTRY

	(void) fclose(m_pFile);
	m_pFile = 0;

	Display::Get()->TextStatus(sDone, DISPLAY_7SEGMENT_MSG_INFO_SPI_DONE);
	puts(sDone);

	DEBUG_EXIT
}

bool SpiFlashInstall::BuffesCompare(uint32_t nSize) {
	DEBUG1_ENTRY

	assert(nSize <= m_nEraseSize);

	const uint32_t *src32 = (uint32_t *) m_pFileBuffer;
	assert(((uint32_t )src32 & 0x3) == 0);

	const uint32_t *dst32 = (uint32_t *) m_pFlashBuffer;
	assert(((uint32_t )dst32 & 0x3) == 0);

	while (nSize >= 4) {
		if (*src32++ != *dst32++) {
			DEBUG1_EXIT
			return false;
		}
		nSize -= 4;
	}

	const uint8_t *src = (uint8_t *) src32;
	const uint8_t *dst = (uint8_t *) dst32;

	while (nSize--) {
		if (*src++ != *dst++) {
			DEBUG1_EXIT
			return false;
		}
	}

	DEBUG1_EXIT
	return true;
}

bool SpiFlashInstall::Diff(uint32_t nOffset) {
	DEBUG_ENTRY

	assert(m_pFile != 0);
	assert(nOffset < m_nFlashSize);
	assert(m_pFileBuffer != 0);
	assert(m_pFlashBuffer != 0);

	if (fseek(m_pFile, 0L, SEEK_SET) != 0) {
		DEBUG_EXIT
		return false;
	}

	if (fread(m_pFileBuffer, sizeof(uint8_t), (size_t) COMPARE_BYTES,  m_pFile) != (size_t) COMPARE_BYTES) {
		DEBUG_EXIT
		return false;
	}

	if (spi_flash_cmd_read_fast(nOffset, COMPARE_BYTES, m_pFlashBuffer) < 0) {
		DEBUG_EXIT
		return false;
	}

	if (!BuffesCompare(COMPARE_BYTES)) {
		DEBUG_EXIT
		return true;
	}

	DEBUG_EXIT
	return false;
}

void SpiFlashInstall::Write(uint32_t nOffset) {
	DEBUG_ENTRY

	assert(m_pFile != 0);
	assert(nOffset < m_nFlashSize);
	assert(m_pFileBuffer != 0);

	bool bSuccess __attribute__((unused)) = false;

	uint32_t n_Address = nOffset;
	size_t nTotalBytes = 0;

	(void) fseek(m_pFile, 0L, SEEK_SET);

	while (n_Address < m_nFlashSize) {
		const size_t nBytes = fread(m_pFileBuffer, sizeof(uint8_t), (size_t) m_nEraseSize, m_pFile);
		nTotalBytes += nBytes;

		if (spi_flash_cmd_erase(n_Address, m_nEraseSize) < 0) {
			printf("error: flash erase\n");
			break;
		}

		if (nBytes < m_nEraseSize) {
			for (uint32_t i = nBytes; i < m_nEraseSize; i++) {
				m_pFileBuffer[i] = 0xFF;
			}
		}

		if (spi_flash_cmd_write_multi(n_Address, m_nEraseSize, m_pFileBuffer) < 0) {
			printf("error: flash write\n");
			break;
		}

		if (spi_flash_cmd_read_fast(n_Address, nBytes, m_pFlashBuffer) < 0) {
			printf("error: flash read\n");
			break;
		}

		if (!BuffesCompare(nBytes)) {
			printf("error: flash verify\n");
			break;
		}

		if (nBytes != m_nEraseSize) {
			if (ferror(m_pFile) == 0 ) {
				bSuccess = true;
			}
			break; // Error or end of file
		}

		n_Address += m_nEraseSize;
	}

	if (bSuccess) {
		Display::Get()->ClearLine(3);
		Display::Get()->Printf(3, "%d", (int) nTotalBytes);
		printf("%d bytes written\n", (int) nTotalBytes);
	}

	DEBUG_EXIT
}

bool SpiFlashInstall::WriteFirmware(const uint8_t* pBuffer, uint32_t nSize) {
	DEBUG_ENTRY

	assert(pBuffer != 0);
	DEBUG_PRINTF("(%d + %d)=%d, m_nFlashSize=%d", OFFSET_UIMAGE, nSize, (OFFSET_UIMAGE + nSize), m_nFlashSize);

	if ((OFFSET_UIMAGE + nSize) > m_nFlashSize) {
		printf("error: flash size %d > %d\n", (OFFSET_UIMAGE + nSize), m_nFlashSize);
		DEBUG_EXIT
		return false;
	}

	printf("Write firmware\n");

	const uint32_t nSectorSize = spi_flash_get_sector_size();
	const uint32_t nEraseSize = (nSize + nSectorSize - 1) & ~(nSectorSize - 1);

	DEBUG_PRINTF("nSize=%x, nSectorSize=%x, nEraseSize=%x", nSize, nSectorSize, nEraseSize);

	Hardware::Get()->WatchdogStop();

	Display::Get()->Status(DISPLAY_7SEGMENT_MSG_INFO_SPI_ERASE);

	if (spi_flash_cmd_erase(OFFSET_UIMAGE, nEraseSize) < 0) {
		printf("error: flash erase\n");
		return false;
	}

	Display::Get()->Status(DISPLAY_7SEGMENT_MSG_INFO_SPI_WRITING);

	if (spi_flash_cmd_write_multi(OFFSET_UIMAGE, nSize, pBuffer) < 0) {
		printf("error: flash write\n");
		return false;
	}

	Hardware::Get()->WatchdogInit();

	Display::Get()->Status(DISPLAY_7SEGMENT_MSG_INFO_SPI_DONE);

	return true;

	DEBUG_EXIT
}

