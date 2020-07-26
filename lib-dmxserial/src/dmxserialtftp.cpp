/**
 * @file dmxserialtftp.cpp
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
#include <cassert>

#include "dmxserialtftp.h"
#include "dmxserial.h"

#include "debug.h"

DmxSerialTFTP::DmxSerialTFTP() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

DmxSerialTFTP::~DmxSerialTFTP() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void DmxSerialTFTP::Exit() {
	DEBUG_ENTRY

	DmxSerial::Get()->EnableTFTP(false);

	DEBUG_EXIT
}

bool DmxSerialTFTP::FileOpen(const char *pFileName, __attribute__((unused)) TFTPMode tMode) {
	DEBUG_PRINTF("pFileName=%s, tMode=%d", pFileName, static_cast<int>(tMode));

	int16_t nFileNumber;
	if (!DmxSerial::CheckFileName(pFileName, nFileNumber)) {
		DEBUG_EXIT
		return false;
	}

	m_pFile = fopen(pFileName, "r");
	return (m_pFile != nullptr);
}

bool DmxSerialTFTP::FileCreate(const char *pFileName, __attribute__((unused)) TFTPMode tMode) {
	DEBUG_PRINTF("pFileName=%s, tMode=%d", pFileName, static_cast<int>(tMode));

	int16_t nFileNumber;
	if (!DmxSerial::CheckFileName(pFileName, nFileNumber)) {
		DEBUG_EXIT
		return false;
	}

	m_pFile = fopen(pFileName, "w+");
	return (m_pFile != nullptr);
}

bool DmxSerialTFTP::FileClose() {
	DEBUG_ENTRY

	if (m_pFile != nullptr) {
		fclose(m_pFile);
		m_pFile = nullptr;
	}

	DEBUG_EXIT
	return true;
}

size_t DmxSerialTFTP::FileRead(void *pBuffer, size_t nCount, __attribute__((unused)) unsigned nBlockNumber) {
	return fread(pBuffer, 1, nCount, m_pFile);
}

size_t DmxSerialTFTP::FileWrite(const void *pBuffer, size_t nCount, __attribute__((unused))unsigned nBlockNumber) {
	return fwrite(pBuffer, 1, nCount, m_pFile);
}
