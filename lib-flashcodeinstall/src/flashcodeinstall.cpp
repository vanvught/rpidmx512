/**
 * @file flashcodeinstall.cpp
 *
 */
/* Copyright (C) 2018-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "flashcodeinstall.h"

#include "display.h"
#include "hardware.h"

#include "debug.h"

bool FlashCodeInstall::WriteFirmware(const uint8_t *pBuffer, uint32_t nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nSize != 0);

	DEBUG_PRINTF("(%p + %p)=%p, m_nFlashSize=%d", OFFSET_UIMAGE, nSize, (OFFSET_UIMAGE + nSize), m_nFlashSize);

	if ((OFFSET_UIMAGE + nSize) > m_nFlashSize) {
		printf("error: flash size %d > %d\n", (OFFSET_UIMAGE + nSize), m_nFlashSize);
		DEBUG_EXIT
		return false;
	}

	const auto bWatchdog = Hardware::Get()->IsWatchdog();

	if (bWatchdog) {
		Hardware::Get()->WatchdogStop();
	}

	puts("Write firmware");

	const auto nSectorSize = FlashCode::GetSectorSize();
	const auto nEraseSize = (nSize + nSectorSize - 1) & ~(nSectorSize - 1);

	DEBUG_PRINTF("nSize=%x, nSectorSize=%x, nEraseSize=%x", nSize, nSectorSize, nEraseSize);

	Display::Get()->TextStatus("Erase", Display7SegmentMessage::INFO_SPI_ERASE, CONSOLE_GREEN);

	flashcode::result nResult;

	FlashCode::Erase(OFFSET_UIMAGE, nEraseSize, nResult);

	if (flashcode::result::ERROR == nResult) {
		puts("error: flash erase");
		return false;
	}

	Display::Get()->TextStatus("Writing", Display7SegmentMessage::INFO_SPI_WRITING, CONSOLE_GREEN);

	FlashCode::Write(OFFSET_UIMAGE, nSize, pBuffer, nResult);

	if (flashcode::result::ERROR == nResult) {
		puts("error: flash write");
		return false;
	}

	if (bWatchdog) {
		Hardware::Get()->WatchdogInit();
	}

	Display::Get()->TextStatus("Done", Display7SegmentMessage::INFO_SPI_DONE, CONSOLE_GREEN);

	DEBUG_EXIT
	return true;
}
