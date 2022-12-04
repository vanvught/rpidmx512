/**
 * @file remoteconfig.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "remoteconfig.h"

#include "tftp/tftpfileserver.h"
#include "flashcodeinstall.h"

#include "display.h"

#include "debug.h"

void RemoteConfig::PlatformHandleTftpSet() {
	DEBUG_ENTRY

	if (m_bEnableTFTP) {
		Display::Get()->SetSleep(false);
	}

	if (m_bEnableTFTP && (m_pTFTPFileServer == nullptr)) {
		puts("Create TFTP Server");

		m_pTFTPBuffer = new uint8_t[FIRMWARE_MAX_SIZE];
		assert(m_pTFTPBuffer != nullptr);

		m_pTFTPFileServer = new TFTPFileServer(m_pTFTPBuffer, FIRMWARE_MAX_SIZE);
		assert(m_pTFTPFileServer != nullptr);
		Display::Get()->TextStatus("TFTP On", Display7SegmentMessage::INFO_TFTP_ON);
	} else if (!m_bEnableTFTP && (m_pTFTPFileServer != nullptr)) {
		const uint32_t nFileSize = m_pTFTPFileServer->GetFileSize();
		DEBUG_PRINTF("nFileSize=%d, %d", nFileSize, m_pTFTPFileServer->isDone());

		bool bSucces = true;

		if (m_pTFTPFileServer->isDone()) {
			bSucces = FlashCodeInstall::Get()->WriteFirmware(m_pTFTPBuffer, nFileSize);

			if (!bSucces) {
				Display::Get()->TextStatus("Error: TFTP", Display7SegmentMessage::ERROR_TFTP);
			}
		}

		puts("Delete TFTP Server");

		delete m_pTFTPFileServer;
		m_pTFTPFileServer = nullptr;

		delete[] m_pTFTPBuffer;
		m_pTFTPBuffer = nullptr;

		if (bSucces) { // Keep error message
			Display::Get()->TextStatus("TFTP Off", Display7SegmentMessage::INFO_TFTP_OFF);
		}
	}

	DEBUG_EXIT
}

void RemoteConfig::PlatformHandleTftpGet() {
	DEBUG_ENTRY

	DEBUG_EXIT
}
