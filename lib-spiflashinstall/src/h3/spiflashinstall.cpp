/**
 * @file spiflashinstall.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "spiflashinstall.h"
#include "spiflashinstallparams.h"

#include "display.h"

#include "spi_flash.h"

#include "hardware.h"

// Temporarily classes. Only needed for the migration to compressed firmware
#include "compressed.h"
#include "ledblink.h"

#include "debug.h"

#define OFFSET_UBOOT_SPI	0x000000
#define OFFSET_UIMAGE		0x180000

#define COMPARE_BYTES		1024

#define FLASH_SIZE_MINIMUM	0x200000

constexpr char aFileUbootSpi[] = "uboot.spi";
constexpr char aFileuImage[] = "uImage";

constexpr char aWriting[] = "Writing";
constexpr char aCheckDifference[] = "Check difference";
constexpr char aNoDifference[] = "No difference";
constexpr char aDone[] = "Done";

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

	assert(s_pThis == 0);
	s_pThis = this;

	Display::Get()->Cls();

	if (spi_flash_probe(0, 0, 0) < 0) {
		Display::Get()->TextStatus("No SPI flash", Display7SegmentMessage::INFO_SPI_NONE);
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
					Process(aFileUbootSpi, OFFSET_UBOOT_SPI);
				}

				if (params.GetInstalluImage()) {
					// Temporarily code BEGIN
					const uint32_t code = Compressed::Check(aFileuImage);

					DEBUG_PRINTF("code = %x", code);

					Display::Get()->ClearLine(3);
					Display::Get()->ClearLine(4);

					if ((code & CHECK_CODE_SPI_UPDATE_NEEDED) == CHECK_CODE_SPI_UPDATE_NEEDED) {
						puts("Update UBoot SPI");
						Display::Get()->Write(3, "Update UBoot SPI");
					}

					if ((code & CHECK_CODE_UIMAGE_COMPRESSED) == CHECK_CODE_UIMAGE_COMPRESSED) {
						puts("uImage is compressed");
						Display::Get()->Write(4, "uImage is compressed");
					}

					if ((code & CHECK_CODE_UIMAGE_TOO_BIG) == CHECK_CODE_UIMAGE_TOO_BIG) {
						puts("uImage too big");
						Display::Get()->Write(2, "uImage too big");
						Display::Get()->TextStatus("Halted!", Display7SegmentMessage::ERROR_SPI);
						LedBlink::Get()->SetMode(LEDBLINK_MODE_FAST);
						for (;;) {
							LedBlink::Get()->Run();
						}
					}

					if ((code & CHECK_CODE_SPI_UPDATE_NEEDED) == CHECK_CODE_SPI_UPDATE_NEEDED) {
						Display::Get()->TextStatus("Update UBoot SPI", Display7SegmentMessage::INFO_SPI_UPDATE);
						LedBlink::Get()->SetMode(LEDBLINK_MODE_FAST);
						const uint32_t now = Hardware::Get()->Millis();

						while ((Hardware::Get()->Millis() - now) < 3000) {
							LedBlink::Get()->Run();
						}

						if ((code & CHECK_CODE_UIMAGE_COMPRESSED) == CHECK_CODE_UIMAGE_COMPRESSED) {
							Display::Get()->TextStatus("Halted!", Display7SegmentMessage::ERROR_SPI);
							LedBlink::Get()->SetMode(LEDBLINK_MODE_FAST);
							for (;;) {
								LedBlink::Get()->Run();
							}
						}

					}
					LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);
					// Temporarily code END

					Process(aFileuImage, OFFSET_UIMAGE);
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
		Display::Get()->TextStatus(aCheckDifference, Display7SegmentMessage::INFO_SPI_CHECK);
		puts(aCheckDifference);

		if (Diff(nOffset)) {
			Display::Get()->TextStatus(aWriting, Display7SegmentMessage::INFO_SPI_WRITING);
			puts(aWriting);
			Write(nOffset);
		} else {
			Display::Get()->TextStatus(aNoDifference, Display7SegmentMessage::INFO_SPI_NODIFF);
			puts(aNoDifference);
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

	static_cast<void>(fclose(m_pFile));
	m_pFile = 0;

	Display::Get()->TextStatus(aDone, Display7SegmentMessage::INFO_SPI_DONE);
	puts(aDone);

	DEBUG_EXIT
}

bool SpiFlashInstall::BuffersCompare(uint32_t nSize) {
	DEBUG1_ENTRY

	assert(nSize <= m_nEraseSize);

	const uint32_t *pSrc32 = reinterpret_cast<uint32_t*>(m_pFileBuffer);
	assert((reinterpret_cast<uint32_t>(pSrc32) & 0x3) == 0);

	const uint32_t *pDst32 = reinterpret_cast<uint32_t*>(m_pFlashBuffer);
	assert((reinterpret_cast<uint32_t>(pDst32) & 0x3) == 0);

	while (nSize >= 4) {
		if (*pSrc32++ != *pDst32++) {
			DEBUG1_EXIT
			return false;
		}
		nSize -= 4;
	}

	const uint8_t *pSrc8 = reinterpret_cast<const uint8_t*>(pSrc32);
	const uint8_t *pDst8 = reinterpret_cast<const uint8_t*>(pDst32);

	while (nSize--) {
		if (*pSrc8++ != *pDst8++) {
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

	if (fread(m_pFileBuffer, sizeof(uint8_t), COMPARE_BYTES,  m_pFile) != COMPARE_BYTES) {
		DEBUG_EXIT
		return false;
	}

	if (spi_flash_cmd_read_fast(nOffset, COMPARE_BYTES, m_pFlashBuffer) < 0) {
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

void SpiFlashInstall::Write(uint32_t nOffset) {
	DEBUG_ENTRY

	assert(m_pFile != 0);
	assert(nOffset < m_nFlashSize);
	assert(m_pFileBuffer != 0);

	bool bSuccess __attribute__((unused)) = false;

	uint32_t n_Address = nOffset;
	size_t nTotalBytes = 0;

	static_cast<void>(fseek(m_pFile, 0L, SEEK_SET));

	while (n_Address < m_nFlashSize) {
		const size_t nBytes = fread(m_pFileBuffer, sizeof(uint8_t), m_nEraseSize, m_pFile);
		nTotalBytes += nBytes;

		if (spi_flash_cmd_erase(n_Address, m_nEraseSize) < 0) {
			puts("error: flash erase");
			break;
		}

		if (nBytes < m_nEraseSize) {
			for (uint32_t i = nBytes; i < m_nEraseSize; i++) {
				m_pFileBuffer[i] = 0xFF;
			}
		}

		if (spi_flash_cmd_write_multi(n_Address, m_nEraseSize, m_pFileBuffer) < 0) {
			puts("error: flash write");
			break;
		}

		if (spi_flash_cmd_read_fast(n_Address, nBytes, m_pFlashBuffer) < 0) {
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
		Display::Get()->ClearLine(3);
		Display::Get()->Printf(3, "%d", static_cast<int>(nTotalBytes));
		printf("%d bytes written\n", static_cast<int>(nTotalBytes));
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

	const bool bWatchdog = Hardware::Get()->IsWatchdog();

	if (bWatchdog) {
		Hardware::Get()->WatchdogStop();
	}

	puts("Write firmware");

	const uint32_t nSectorSize = spi_flash_get_sector_size();
	const uint32_t nEraseSize = (nSize + nSectorSize - 1) & ~(nSectorSize - 1);

	DEBUG_PRINTF("nSize=%x, nSectorSize=%x, nEraseSize=%x", nSize, nSectorSize, nEraseSize);

	Display::Get()->TextStatus("Erase", Display7SegmentMessage::INFO_SPI_ERASE, CONSOLE_GREEN);

	if (spi_flash_cmd_erase(OFFSET_UIMAGE, nEraseSize) < 0) {
		puts("error: flash erase");
		return false;
	}

	Display::Get()->TextStatus("Writing", Display7SegmentMessage::INFO_SPI_WRITING, CONSOLE_GREEN);

	if (spi_flash_cmd_write_multi(OFFSET_UIMAGE, nSize, pBuffer) < 0) {
		puts("error: flash write");
		return false;
	}

	if (bWatchdog) {
		Hardware::Get()->WatchdogInit();
	}

	Display::Get()->TextStatus("Done", Display7SegmentMessage::INFO_SPI_DONE, CONSOLE_GREEN);

	return true;

	DEBUG_EXIT
}
