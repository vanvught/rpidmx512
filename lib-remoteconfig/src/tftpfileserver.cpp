/**
 * @file tftpfileserver.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "tftpfileserver.h"
#include "ubootheader.h"

#include "display.h"

#include "debug.h"

#if defined(ORANGE_PI)
 static const char sFileName[] __attribute__((aligned(4))) = "orangepi_zero.uImage";
 #define FILE_NAME_LENGTH	(sizeof(sFileName) / sizeof(sFileName[0]) - 1)
#else
 static const char sFileName[] __attribute__((aligned(4))) = "orangepi_one.uImage";
 #define FILE_NAME_LENGTH	(sizeof(sFileName) / sizeof(sFileName[0]) - 1)
#endif

TFTPFileServer::TFTPFileServer(uint8_t *pBuffer, uint32_t nSize):
		m_pBuffer(pBuffer),
		m_nSize(nSize),
		m_nFileSize(0),
		m_bDone(false)
{
	DEBUG_ENTRY

	assert(m_pBuffer != 0);

	DEBUG_EXIT
}

TFTPFileServer::~TFTPFileServer(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

bool TFTPFileServer::FileOpen(const char* pFileName, TTFTPMode tMode) {
	DEBUG_ENTRY

	DEBUG_EXIT
	return (false);
}

bool TFTPFileServer::FileCreate(const char* pFileName, TTFTPMode tMode) {
	DEBUG_ENTRY

	assert(pFileName != 0);

	if (tMode != TFTP_MODE_BINARY) {
		DEBUG_EXIT
		return false;
	}

	if (strncmp(sFileName, pFileName, FILE_NAME_LENGTH) != 0) {
		DEBUG_EXIT
		return false;
	}

	printf("TFTP started ...\n");
	Display::Get()->Status(DISPLAY_7SEGMENT_MSG_INFO_TFTP_STARTED);

	m_nFileSize = 0;

	DEBUG_EXIT
	return (true);
}

bool TFTPFileServer::FileClose(void) {
	DEBUG_ENTRY

	m_bDone = true;
	Display::Get()->Status(DISPLAY_7SEGMENT_MSG_INFO_TFTP_ENDED);

	DEBUG_EXIT
	return true;
}

int TFTPFileServer::FileRead(void* pBuffer, unsigned nCount, unsigned nBlockNumber) {
	DEBUG_ENTRY

	DEBUG_EXIT
	return -1;
}

int TFTPFileServer::FileWrite(const void* pBuffer, unsigned nCount, unsigned nBlockNumber) {
	DEBUG_PRINTF("pBuffer=%p, nCount=%d, nBlockNumber=%d (%d)", pBuffer, nCount, nBlockNumber, m_nSize / 512);

	if (nBlockNumber > (m_nSize / 512)) {
		m_nFileSize = 0;
		return -1;
	}

	assert(nBlockNumber != 0);

	if (nBlockNumber == 1) {
		UBootHeader uImage((uint8_t *)pBuffer);
		if (!uImage.IsValid()) {
			DEBUG_PUTS("uImage is not valid");
			return -1;
		}
	}

	uint32_t nOffset = (nBlockNumber - 1) * 512;

	assert((nOffset + nCount) <= m_nSize);

	memcpy((void *)&m_pBuffer[nOffset], pBuffer, nCount);

	m_nFileSize += nCount; //FIXME BUG When in retry ?

	return nCount;
}
