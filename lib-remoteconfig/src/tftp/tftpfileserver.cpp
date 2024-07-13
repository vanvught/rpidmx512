/**
 * @file tftpfileserver.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "tftp/tftpfileserver.h"
#include "remoteconfig.h"
#include "display.h"

#include "debug.h"

using namespace tftpfileserver;

static constexpr auto FILE_NAME_LENGTH = sizeof(FILE_NAME) - 1;

TFTPFileServer::TFTPFileServer(uint8_t *pBuffer, uint32_t nSize): m_pBuffer(pBuffer), m_nSize(nSize) {
	DEBUG_ENTRY

	assert(m_pBuffer != nullptr);
	assert(nSize != 0);

	DEBUG_EXIT
}

void TFTPFileServer::Exit() {
	DEBUG_ENTRY

	RemoteConfig::Get()->TftpExit();

	DEBUG_EXIT
}


bool TFTPFileServer::FileOpen([[maybe_unused]] const char *pFileName, [[maybe_unused]] tftp::Mode tMode) {
	DEBUG_ENTRY

	DEBUG_EXIT
	return false;
}

bool TFTPFileServer::FileCreate(const char* pFileName, tftp::Mode mode) {
	DEBUG_ENTRY

	assert(pFileName != nullptr);

	if (mode != tftp::Mode::BINARY) {
		DEBUG_EXIT
		return false;
	}

	if (strncmp(FILE_NAME, pFileName, FILE_NAME_LENGTH) != 0) {
		DEBUG_EXIT
		return false;
	}

	Display::Get()->TextStatus("TFTP Started", CONSOLE_GREEN);

	m_nFileSize = 0;

	DEBUG_EXIT
	return (true);
}

bool TFTPFileServer::FileClose() {
	DEBUG_ENTRY

	m_bDone = true;

	Display::Get()->TextStatus("TFTP Ended", CONSOLE_GREEN);

	DEBUG_EXIT
	return true;
}

size_t TFTPFileServer::FileRead([[maybe_unused]] void* pBuffer, [[maybe_unused]] size_t nCount, [[maybe_unused]] unsigned nBlockNumber) {
	DEBUG_ENTRY

	DEBUG_EXIT
	return 0;
}

size_t TFTPFileServer::FileWrite(const void *pBuffer, size_t nCount, unsigned nBlockNumber) {
	DEBUG_PRINTF("pBuffer=%p, nCount=%d, nBlockNumber=%d (%d)", pBuffer, nCount, nBlockNumber, m_nSize / 512);

	if (nBlockNumber > (m_nSize / 512)) {
		m_nFileSize = 0;
		return 0;
	}

	assert(nBlockNumber != 0);

	if (nBlockNumber == 1) {
		if (!is_valid(pBuffer)) {
			return 0;
		}
	}

	const auto nOffset = (nBlockNumber - 1) * 512U;

	assert((nOffset + nCount) <= m_nSize);

	memcpy(&m_pBuffer[nOffset], pBuffer, nCount);

	m_nFileSize += nCount; //FIXME BUG When in retry ?

	Display::Get()->Progress();

	return nCount;
}
