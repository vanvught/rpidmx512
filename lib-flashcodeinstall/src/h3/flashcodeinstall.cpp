/**
 * @file flashcodeinstall.cpp
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdio>
#include <stdlib.h>
#include <cassert>

#include "flashcodeinstall.h"
#include "flashcodeinstallparams.h"
#include "firmware.h"

#include "display.h"

#include "hardware.h"

#include "debug.h"

#define COMPARE_BYTES		1024

#define FLASH_SIZE_MINIMUM	0x200000

constexpr char aFileUbootSpi[] = "uboot.spi";
constexpr char aFileuImage[] = "uImage";

constexpr char aWriting[] = "Writing";
constexpr char aCheckDifference[] = "Check difference";
constexpr char aNoDifference[] = "No difference";
constexpr char aDone[] = "Done";

FlashCodeInstall *FlashCodeInstall::s_pThis;

FlashCodeInstall::FlashCodeInstall() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	Display::Get()->Cls();

	if (!FlashCode::IsDetected()) {
		Display::Get()->TextStatus("No SPI flash");
		DEBUG_PUTS("No SPI flash chip");
	} else {
		m_nFlashSize = FlashCode::GetSize();
		Display::Get()->Write(1, FlashCode::GetName());
	}

	if (Hardware::Get()->GetBootDevice() == hardware::BootDevice::MMC0) {
		DEBUG_PUTS("BOOT_DEVICE_MMC0");

		FlashCodeInstallParams params;

		if (params.Load()) {
			if (m_nFlashSize >= FLASH_SIZE_MINIMUM) {

				m_bHaveFlashChip = true;
				m_nEraseSize = FlashCode::GetSectorSize();

				m_pFileBuffer = new uint8_t[m_nEraseSize];
				assert(m_pFileBuffer != nullptr);

				m_pFlashBuffer = new uint8_t[m_nEraseSize];
				assert(m_pFlashBuffer != nullptr);

				if (params.GetInstalluboot()) {
					Process(aFileUbootSpi, OFFSET_UBOOT_SPI);
				}

				if (params.GetInstalluImage()) {
					Process(aFileuImage, OFFSET_UIMAGE);
				}
			}
		}
	}

	DEBUG_EXIT
}

FlashCodeInstall::~FlashCodeInstall() {
	DEBUG_ENTRY

	if (m_pFileBuffer != nullptr) {
		delete[] m_pFileBuffer;
	}

	if (m_pFlashBuffer != nullptr) {
		delete[] m_pFlashBuffer;
	}

	DEBUG_EXIT
}

void FlashCodeInstall::Process(const char *pFileName, uint32_t nOffset) {
	if (Open(pFileName)) {
		Display::Get()->TextStatus(aCheckDifference);
		puts(aCheckDifference);

		if (Diff(nOffset)) {
			Display::Get()->TextStatus(aWriting);
			puts(aWriting);
			Write(nOffset);
		} else {
			Display::Get()->TextStatus(aNoDifference);
			puts(aNoDifference);
		}
		Close();
	}
}

bool FlashCodeInstall::Open(const char* pFileName) {
	DEBUG_ENTRY

	assert(pFileName != nullptr);
	assert(m_pFile == nullptr);

	m_pFile = fopen(pFileName, "r");

	if (m_pFile == nullptr) {
		printf("Could not open file: %s\n", pFileName);
		DEBUG_EXIT
		return false;
	}

	Display::Get()->ClearEndOfLine();
	Display::Get()->Write(2, pFileName);
	puts(pFileName);

	DEBUG_EXIT
	return true;
}

void FlashCodeInstall::Close() {
	DEBUG_ENTRY

	static_cast<void>(fclose(m_pFile));
	m_pFile = nullptr;

	Display::Get()->TextStatus(aDone);
	puts(aDone);

	DEBUG_EXIT
}

bool FlashCodeInstall::BuffersCompare(uint32_t nSize) {
	DEBUG_ENTRY

	assert(nSize <= m_nEraseSize);

	const uint32_t *pSrc32 = reinterpret_cast<uint32_t*>(m_pFileBuffer);
	assert((reinterpret_cast<uint32_t>(pSrc32) & 0x3) == 0);

	const uint32_t *pDst32 = reinterpret_cast<uint32_t*>(m_pFlashBuffer);
	assert((reinterpret_cast<uint32_t>(pDst32) & 0x3) == 0);

	while (nSize >= 4) {
		if (*pSrc32++ != *pDst32++) {
			DEBUG_EXIT
			return false;
		}
		nSize -= 4;
	}

	const auto *pSrc8 = reinterpret_cast<const uint8_t*>(pSrc32);
	const auto *pDst8 = reinterpret_cast<const uint8_t*>(pDst32);

	while (nSize--) {
		if (*pSrc8++ != *pDst8++) {
			DEBUG_EXIT
			return false;
		}
	}

	DEBUG_EXIT
	return true;
}

bool FlashCodeInstall::Diff(uint32_t nOffset) {
	DEBUG_ENTRY

	assert(m_pFile != nullptr);
	assert(nOffset < m_nFlashSize);
	assert(m_pFileBuffer != nullptr);
	assert(m_pFlashBuffer != nullptr);

	if (fseek(m_pFile, 0L, SEEK_SET) != 0) {
		DEBUG_EXIT
		return false;
	}

	if (fread(m_pFileBuffer, sizeof(uint8_t), COMPARE_BYTES,  m_pFile) != COMPARE_BYTES) {
		DEBUG_EXIT
		return false;
	}

	flashcode::result result;
	FlashCode::Read(nOffset, COMPARE_BYTES, m_pFlashBuffer, result);

	if (flashcode::result::ERROR == result) {
		DEBUG_EXIT
		return false;
	}

	if (!BuffersCompare(COMPARE_BYTES)) {
		DEBUG_EXIT
		return true;
	}

	DEBUG_EXIT
	return false;
}

void FlashCodeInstall::Write(uint32_t nOffset) {
	DEBUG_ENTRY

	assert(m_pFile != nullptr);
	assert(nOffset < m_nFlashSize);
	assert(m_pFileBuffer != nullptr);

	auto bSuccess = false;

	uint32_t n_Address = nOffset;
	size_t nTotalBytes = 0;

	static_cast<void>(fseek(m_pFile, 0L, SEEK_SET));

	while (n_Address < m_nFlashSize) {
		const auto nBytes = fread(m_pFileBuffer, sizeof(uint8_t), m_nEraseSize, m_pFile);
		nTotalBytes += nBytes;

		flashcode::result result;
		FlashCode::Erase(n_Address, m_nEraseSize, result);

		if (flashcode::result::ERROR == result) {
			puts("error: flash erase");
			break;
		}

		if (nBytes < m_nEraseSize) {
			for (uint32_t i = nBytes; i < m_nEraseSize; i++) {
				m_pFileBuffer[i] = 0xFF;
			}
		}

		FlashCode::Write(n_Address, m_nEraseSize, m_pFileBuffer, result);

		if (flashcode::result::ERROR == result) {
			puts("error: flash write");
			break;
		}

		FlashCode::Read(n_Address, nBytes, m_pFlashBuffer, result);

		if (flashcode::result::ERROR == result) {
			puts("error: flash read");
			break;
		}

		if (!BuffersCompare(nBytes)) {
			puts("error: flash verify");
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
		Display::Get()->ClearEndOfLine();
		Display::Get()->Printf(3, "%d", static_cast<int>(nTotalBytes));
		printf("%d bytes written\n", static_cast<int>(nTotalBytes));
	}

	DEBUG_EXIT
}
