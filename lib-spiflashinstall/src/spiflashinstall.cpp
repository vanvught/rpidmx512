/**
 * @file spiflashinstall.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "spiflashinstall.h"
#include "spiflashinstallparams.h"

#include "spi_flash.h"

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

SpiFlashInstall::SpiFlashInstall(void):
	m_bHaveFlashChip(false),
	m_nEraseSize(0),
	m_nFlashSize(0),
	m_pFileBuffer(0),
	m_pFlashBuffer(0),
	m_pFile(0)
{
	DEBUG_ENTRY

	SpiFlashInstallParams params;

	if (params.Load()) {
		params.Dump();

		if (spi_flash_probe(0, 0, 0) < 0) {
			DEBUG_PUTS("No SPI flash chip");
		} else {
			m_nFlashSize = spi_flash_get_size();

			if (m_nFlashSize >= FLASH_SIZE_MINIMUM) {

				m_bHaveFlashChip = true;
				m_nEraseSize = spi_flash_get_sector_size();

				m_pFileBuffer = new uint8_t[m_nEraseSize];
				assert(m_pFileBuffer != 0);

				m_pFlashBuffer = new uint8_t[m_nEraseSize];
				assert(m_pFlashBuffer != 0);

				if (params.GetInstalluboot()) {
					if (Open(sFileUbootSpi)) {
						if (Diff(OFFSET_UBOOT_SPI)) {
							Write(OFFSET_UBOOT_SPI);
						}
						Close();
					}
				}

				if (params.GetInstalluImage()) {
					if (Open(sFileuImage)) {
						if (Diff(OFFSET_UIMAGE)) {
							Write(OFFSET_UIMAGE);
						}
						Close();
					}
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

bool SpiFlashInstall::Open(const char* pFileName) {
	DEBUG_ENTRY

	assert(pFileName != 0);
	assert(m_pFile == 0);

	m_pFile = fopen(pFileName, "r");

	if (m_pFile == NULL) {
		DEBUG_PRINTF("Could not open file: %s", pFileName);
		DEBUG_EXIT
		return false;
	}

	DEBUG_EXIT
	return true;
}

void SpiFlashInstall::Close(void) {
	DEBUG_ENTRY

	(void) fclose(m_pFile);
	m_pFile = 0;

	DEBUG_EXIT
}

bool SpiFlashInstall::Diff(uint32_t nOffset) {
	DEBUG_ENTRY

	assert(m_pFile != 0);
	assert(nOffset < m_nFlashSize);
	assert(m_pFileBuffer != 0);
	assert(m_pFlashBuffer != 0);

	(void) fseek(m_pFile, 0L, SEEK_SET);

	if (fread(m_pFileBuffer, (size_t) COMPARE_BYTES, sizeof(uint8_t), m_pFile) != (size_t) COMPARE_BYTES) {
		DEBUG_EXIT
		return false;
	}

	if (spi_flash_cmd_read_fast(nOffset, COMPARE_BYTES, m_pFlashBuffer) < 0) {
		DEBUG_EXIT
		return false;
	}

	const uint32_t *src = (uint32_t *) m_pFileBuffer;
	assert(((uint32_t)src & 0x3) == 0);

	const uint32_t *dst = (uint32_t *) m_pFlashBuffer;
	assert(((uint32_t)dst & 0x3) == 0);

	for (uint32_t i = 0; i < COMPARE_BYTES / 4; i++) {
		if (*src != *dst) {
			DEBUG_EXIT
			return true;
		}
		src++;
		dst++;
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

	(void) fseek(m_pFile, 0L, SEEK_SET);

	while (n_Address < m_nFlashSize) {
		const size_t nBytes = fread(m_pFileBuffer, (size_t) m_nEraseSize, sizeof(uint8_t), m_pFile);

		if (spi_flash_cmd_erase(n_Address, m_nEraseSize) < 0) {
			DEBUG_PUTS("error: flash erase");
			break; // Error
		}

		if (spi_flash_cmd_write_multi(n_Address, m_nEraseSize, m_pFileBuffer) < 0) {
			DEBUG_PUTS("error: flash write");
			break; // Error
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
		DEBUG_PRINTF("%d bytes written", (int) (n_Address - nOffset));
	}

	DEBUG_EXIT
}
