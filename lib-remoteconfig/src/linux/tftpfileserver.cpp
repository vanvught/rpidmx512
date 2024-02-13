/**
 * @file tftpfileserver.cpp
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

#if defined(__clang__)
# pragma GCC diagnostic ignored "-Wunused-private-field"
#endif

#include "tftp/tftpfileserver.h"

#include "debug.h"

TFTPFileServer::TFTPFileServer(uint8_t *pBuffer, uint32_t nSize): m_pBuffer(pBuffer), m_nSize(nSize) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void TFTPFileServer::Exit() {
	DEBUG_ENTRY
	DEBUG_EXIT
}

bool TFTPFileServer::FileOpen([[maybe_unused]] const char* pFileName, [[maybe_unused]] tftp::Mode mode) {
	DEBUG_ENTRY
	DEBUG_EXIT
	return false;
}

bool TFTPFileServer::FileCreate([[maybe_unused]] const char* pFileName, [[maybe_unused]] tftp::Mode mode) {
	DEBUG_ENTRY
	DEBUG_EXIT
	return false;
}

bool TFTPFileServer::FileClose() {
	DEBUG_ENTRY
	DEBUG_EXIT
	return false;
}

size_t TFTPFileServer::FileRead([[maybe_unused]] void* pBuffer, [[maybe_unused]] size_t nCount, [[maybe_unused]] unsigned nBlockNumber) {
	DEBUG_ENTRY
	DEBUG_EXIT
	return 0;
}

size_t TFTPFileServer::FileWrite([[maybe_unused]] const void *pBuffer, [[maybe_unused]] size_t nCount, [[maybe_unused]] unsigned nBlockNumber) {
	DEBUG_ENTRY
	DEBUG_EXIT
	return 0;
}
