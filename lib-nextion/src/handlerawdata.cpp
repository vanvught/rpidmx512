/**
 * @file handlerawdata.cpp
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
#include <assert.h>

#include "nextion.h"

#include "remoteconfig.h"

#include "display.h"

#include "debug.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

enum TRequestType {
	REQUEST_GET,
	REQUEST_SAVE
};

void Nextion::HandleRawData(void) {
	DEBUG_ENTRY

#ifndef NDEBUG
	m_aCommandReturned[m_nCount]  = '\0';
	DEBUG_PUTS(m_aCommandReturned);
#endif

	uint32_t nOffset = 0;

	DEBUG_PRINTF("nOffset=%d, m_nCount=%d", (int) nOffset, (int) m_nCount);

	while (nOffset < m_nCount) {
		const uint32_t nRemaining = sizeof(m_aCommandReturned) - nOffset;

		if ((nRemaining >= 4) && (memcmp(&m_aCommandReturned[nOffset], "main", 4) == 0)) {
			nOffset += 4;
			HandleMain();
		} else if ((nRemaining >= 9) &&  (memcmp(&m_aCommandReturned[nOffset], "?reboot##", 8) == 0)) {
			RemoteConfig::HandleReboot();
		} else if ((nRemaining >= 10) && (memcmp(&m_aCommandReturned[nOffset], "!display#", 9) == 0)) {
			nOffset += 10	;
			Display::Get()->SetSleep(false);
		} else if ((m_aCommandReturned[nOffset] == '?') || (m_aCommandReturned[nOffset] == '!')) {
			const TRequestType tType = m_aCommandReturned[nOffset] == '?' ? REQUEST_GET : REQUEST_SAVE;

			nOffset++;

			uint32_t nLength = nRemaining;
			const uint32_t nIndex = RemoteConfig::GetIndex(reinterpret_cast<const char *>(&m_aCommandReturned[nOffset]), nLength);

			if (nIndex == TXT_FILE_LAST) {
				nOffset++;
				DEBUG_PUTS("!> Invalid .txt <!");
				continue;
			}

			nOffset += nLength;

			if (tType == REQUEST_GET) {
				HandleGet(nIndex);
			} else {
				HandleSave(nIndex);
			}
		} else {
			nOffset++;
			DEBUG_PUTS("!> Invalid command <!");
			continue;
		}

		DEBUG_PRINTF("nOffset=%d, m_nCount=%d", (int) nOffset, (int) m_nCount);
	}

	DEBUG_EXIT
}

void Nextion::HandleGet(uint32_t nIndex) {
	DEBUG1_ENTRY

	switch (nIndex) {
	case TXT_FILE_NETWORK:
		HandleNetworkGet();
		break;
#if defined (REMOTE_CONFIG)
	case TXT_FILE_RCONFIG:
		HandleRconfigGet();
		break;
#endif
#if defined (DISPLAY_UDF)
	case TXT_FILE_DISPLAY_UDF:
		HandleDisplayGet();
		break;
#endif
#if defined (ARTNET_NODE)
	case TXT_FILE_ARTNET:
		HandleArtNetGet();
		break;
#endif
#if defined (PIXEL)
	case TXT_FILE_DEVICES:
		HandleDevicesGet();
		break;
#endif
	default:
		break;
	}

	DEBUG1_EXIT
}

void Nextion::HandleSave(uint32_t nIndex) {
	DEBUG1_ENTRY

	switch (nIndex) {
	case TXT_FILE_NETWORK:
		HandleNetworkSave();
		break;
#if defined (REMOTE_CONFIG)
	case TXT_FILE_RCONFIG:
		HandleRconfigSave();
		break;
#endif
#if defined (DISPLAY_UDF)
	case TXT_FILE_DISPLAY_UDF:
		HandleDisplaySave();
		break;
#endif
#if defined (ARTNET_NODE)
	case TXT_FILE_ARTNET:
		HandleArtNetSave();
		break;
#endif
#if defined (PIXEL)
	case TXT_FILE_DEVICES:
		HandleDevicesSave();
		break;
#endif
	default:
		break;
	}

	DEBUG1_EXIT
}
