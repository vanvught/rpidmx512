/**
 * @file compressed.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

/**
 * This is a temporarily class until everyone is using compressed firmware
 */

#if !defined(__clang__)	// Needed for compiling on MacOS
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdio.h>
#include <stdint.h>
#include <cassert>

#include "compressed.h"

#include "ubootheader.h"
#include "spi_flash.h"

#include "debug.h"

constexpr char bootcmd[] = "bootcmd=sf probe; sf read 48000000 180000 22000; bootm 48000000";
#define BOOTCMD_LENGTH	(sizeof bootcmd)

int32_t Compressed::GetFileSize(const char *pFileName) {
	FILE *pFile = fopen(pFileName, "r");

	if (pFile == nullptr) {
		DEBUG_EXIT
		return -1;
	}

	if (fseek(pFile, 0 , SEEK_END) != 0) {
		DEBUG_EXIT
		return -2;
	}

	const long nFileSize = ftell(pFile);

	static_cast<void>(fclose(pFile));

	return static_cast<int32_t>(nFileSize);
}

bool Compressed::IsSupported() {
	const uint32_t nEraseSize = spi_flash_get_sector_size();
	assert(nEraseSize != 0);

	DEBUG_PRINTF("m_nEraseSize=%d", nEraseSize);

	uint8_t *pFlashBuffer = new uint8_t[nEraseSize];
	uint32_t nOffset = 0x55000;

	if (spi_flash_cmd_read_fast(nOffset, nEraseSize, pFlashBuffer) < 0) {
		DEBUG_EXIT
		return false;
	}

	debug_dump(pFlashBuffer, nEraseSize);

	char *pResult = Find(reinterpret_cast<char*>(pFlashBuffer), nEraseSize, bootcmd, BOOTCMD_LENGTH);

	DEBUG_PRINTF("offset %x", (pResult - reinterpret_cast<char*>(pFlashBuffer)));

	if (pResult != nullptr) {
		debug_dump(pResult, 64);
		DEBUG_PUTS(pResult);
	} else {
		DEBUG_PUTS("Not found");
	}

	delete[] pFlashBuffer;

	return (pResult != nullptr);
}

char *Compressed::Find(const char *pBuffer, uint32_t nBufferLength, const char *pFind, uint32_t nFindLength) {
	assert(pBuffer != nullptr);
	assert(pFind != nullptr);

	char *pDst = const_cast<char*>(pBuffer);

	for (; nBufferLength-- > 0; pDst++) {
		if (*pDst != *pFind) {
			continue;
		}

		unsigned nLength = nFindLength;
		char *pTmpFind = const_cast<char *>(pFind);
		char *pRet = pDst;

		while (1) {
			if (nLength-- == 0) {
				return pRet;
			}

			if (*pDst++ != *pTmpFind++) {
				break;
			}

			if (nBufferLength-- == 0) {
				break;
			}
		}
	}

	return nullptr;
}

uint32_t Compressed::Check(const char *pFilename) {
	if (IsSupported()) {
		// Boot SPI supports compressed firmware
		// Sanity check for uImage file size
		if (GetFileSize(pFilename) < 0x22000) {
			DEBUG_EXIT
			return CHECK_CODE_OK;
		} else {
			DEBUG_EXIT
			return CHECK_CODE_UIMAGE_TOO_BIG;
		}
	}

	// SPI flash must be updated

	FILE *pFile = fopen(pFilename, "r");

	if (pFile == nullptr) {
		DEBUG_EXIT
		return CHECK_CODE_CHECK_ERROR;
	}

	uint8_t FileBuffer[1024];

	if (fread(FileBuffer, sizeof(uint8_t), sizeof FileBuffer,  pFile) != sizeof FileBuffer) {
		DEBUG_EXIT
		return CHECK_CODE_CHECK_ERROR;
	}

	static_cast<void>(fclose(pFile));

	UBootHeader header(FileBuffer);
	// Sanity check
	if (header.IsValid()) {
		if (header.IsCompressed()) {
			DEBUG_EXIT
			return CHECK_CODE_SPI_UPDATE_NEEDED| CHECK_CODE_UIMAGE_COMPRESSED;
		} else {
			if (GetFileSize(pFilename) < 0x22000) {
				DEBUG_EXIT
				return CHECK_CODE_SPI_UPDATE_NEEDED;
			} else {
				return CHECK_CODE_SPI_UPDATE_NEEDED | CHECK_CODE_UIMAGE_TOO_BIG;
			}
		}
	}

	DEBUG_EXIT
	return CHECK_CODE_INVALID_HEADER;
}
