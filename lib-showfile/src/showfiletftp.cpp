/**
 * @file showfiletftp.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdio.h>

#include "showfiletftp.h"
#include "showfile.h"

#include "debug.h"

ShowFileTFTP::ShowFileTFTP(void): m_pFile(0) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

ShowFileTFTP::~ShowFileTFTP(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void ShowFileTFTP::Exit(void) {
	DEBUG_ENTRY

	ShowFile::Get()->EnableTFTP(false);

	DEBUG_EXIT
}

bool ShowFileTFTP::FileOpen(const char *pFileName, TTFTPMode tMode) {
	DEBUG_PRINTF("pFileName=%s, tMode=%d", pFileName, tMode);

	uint8_t nShowFileNumber;
	if (!ShowFile::CheckShowFileName(pFileName, nShowFileNumber)) {
		DEBUG_EXIT
		return false;
	}

	m_pFile = fopen(pFileName, "r");
	return (m_pFile != 0);
}

bool ShowFileTFTP::FileCreate(const char *pFileName, TTFTPMode tMode) {
	DEBUG_PRINTF("pFileName=%s, tMode=%d", pFileName, tMode);

	uint8_t nShowFileNumber;
	if (!ShowFile::CheckShowFileName(pFileName, nShowFileNumber)) {
		DEBUG_EXIT
		return false;
	}

	m_pFile = fopen(pFileName, "w+");
	return (m_pFile != 0);
}

bool ShowFileTFTP::FileClose(void) {
	DEBUG_ENTRY

	if (m_pFile != 0) {
		fclose(m_pFile);
		m_pFile = 0;
	}

	DEBUG_EXIT
	return true;
}

int ShowFileTFTP::FileRead(void *pBuffer, unsigned nCount, unsigned nBlockNumber) {
	return fread(pBuffer, 1, nCount, m_pFile);
}

int ShowFileTFTP::FileWrite(const void *pBuffer, unsigned nCount, unsigned nBlockNumber) {
	return fwrite(pBuffer, 1, nCount, m_pFile);
}
