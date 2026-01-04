/**
 * @file dmxserialtftp.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cassert>

#include "dmxserialtftp.h"
#include "dmxserial.h"

 #include "firmware/debug/debug_debug.h"

void DmxSerialTFTP::Exit() {
	DEBUG_ENTRY();

	DmxSerial::Get()->EnableTFTP(false);

	DEBUG_EXIT();
}

bool DmxSerialTFTP::FileOpen(const char *pFileName, [[maybe_unused]] tftp::Mode mode) {
	DEBUG_PRINTF("pFileName=%s, tMode=%d", pFileName, static_cast<int>(mode));

	int32_t nFileNumber;
	if (!DmxSerial::CheckFileName(pFileName, nFileNumber)) {
		DEBUG_EXIT();
		return false;
	}

	file_ = fopen(pFileName, "r");
	return (file_ != nullptr);
}

bool DmxSerialTFTP::FileCreate(const char *pFileName, [[maybe_unused]] tftp::Mode mode) {
	DEBUG_PRINTF("pFileName=%s, tMode=%d", pFileName, static_cast<int>(mode));

	int32_t nFileNumber;
	if (!DmxSerial::CheckFileName(pFileName, nFileNumber)) {
		DEBUG_EXIT();
		return false;
	}

	file_ = fopen(pFileName, "w+");
	return (file_ != nullptr);
}

bool DmxSerialTFTP::FileClose() {
	DEBUG_ENTRY();

	if (file_ != nullptr) {
		fclose(file_);
		file_ = nullptr;
	}

	DEBUG_EXIT();
	return true;
}

size_t DmxSerialTFTP::FileRead(void *pBuffer, size_t nCount, [[maybe_unused]] unsigned nBlockNumber) {
	return fread(pBuffer, 1, nCount, file_);
}

size_t DmxSerialTFTP::FileWrite(const void *pBuffer, size_t nCount, [[maybe_unused]] unsigned nBlockNumber) {
	return fwrite(pBuffer, 1, nCount, file_);
}
