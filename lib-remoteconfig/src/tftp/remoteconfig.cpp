/**
 * @file remoteconfig.cpp
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined(ENABLE_TFTP_SERVER)

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

static uint8_t s_TFTPBuffer[FIRMWARE_MAX_SIZE];

void RemoteConfig::PlatformHandleTftpSet() {
	DEBUG_ENTRY

	if (m_bEnableTFTP && (m_pTFTPFileServer == nullptr)) {
		m_pTFTPFileServer = new TFTPFileServer(s_TFTPBuffer, FIRMWARE_MAX_SIZE);
		assert(m_pTFTPFileServer != nullptr);
		Display::Get()->TextStatus("TFTP On", CONSOLE_GREEN);
	} else if (!m_bEnableTFTP && (m_pTFTPFileServer != nullptr)) {
		const uint32_t nFileSize = m_pTFTPFileServer->GetFileSize();
		DEBUG_PRINTF("nFileSize=%d, %d", nFileSize, m_pTFTPFileServer->isDone());

		bool bSucces = true;

		if (m_pTFTPFileServer->isDone()) {
			bSucces = FlashCodeInstall::Get()->WriteFirmware(s_TFTPBuffer, nFileSize);

			if (!bSucces) {
				Display::Get()->TextStatus("Error: TFTP", CONSOLE_RED);
			}
		}

		delete m_pTFTPFileServer;
		m_pTFTPFileServer = nullptr;

		if (bSucces) { // Keep error message
			Display::Get()->TextStatus("TFTP Off", CONSOLE_GREEN);
		}
	}

	DEBUG_EXIT
}

void RemoteConfig::PlatformHandleTftpGet() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

#endif
