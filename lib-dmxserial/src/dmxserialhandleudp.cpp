/**
 * @file dmxserialhandleudp.cpp
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
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "dmxserial.h"
#include "dmxserial_internal.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

static const char sRequestFiles[] ALIGNED = "?files#";
#define REQUEST_FILES_LENGTH (sizeof(sRequestFiles)/sizeof(sRequestFiles[0]) - 1)

static const char sGetTFTP[] ALIGNED = "?tftp#";
#define GET_TFTP_LENGTH (sizeof(sGetTFTP)/sizeof(sGetTFTP[0]) - 1)

static const char sSetTFTP[] ALIGNED = "!tftp#";
#define SET_TFTP_LENGTH (sizeof(sSetTFTP)/sizeof(sSetTFTP[0]) - 1)

static const char sReload[] ALIGNED = "?reload##";
#define REQUEST_RELOAD_LENGTH (sizeof(sReload)/sizeof(sReload[0]) - 1)

static const char sDelete[] ALIGNED = "!delete#";
#define REQUEST_DELETE_LENGTH (sizeof(sDelete)/sizeof(sDelete[0]) - 1)

enum {
	UDP_BUFFER_SIZE = 32
};

static char s_UdpBuffer[UDP_BUFFER_SIZE];

void DmxSerial::HandleUdp(void) {
	uint16_t nForeignPort;
	uint32_t nIPAddressFrom;

	uint16_t nBytesReceived = Network::Get()->RecvFrom(m_nHandle, s_UdpBuffer, UDP_BUFFER_SIZE, &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((nBytesReceived < 6), 1)) {
		return;
	}

#ifndef NDEBUG
	debug_dump(s_UdpBuffer, nBytesReceived);
#endif

	if (s_UdpBuffer[nBytesReceived - 1] == '\n') {
		nBytesReceived--;
	}

	if (memcmp(s_UdpBuffer, sRequestFiles, REQUEST_FILES_LENGTH) == 0) {
		for (uint32_t i = 0; i < m_nFilesCount; i++) {
			uint32_t nLength = snprintf(s_UdpBuffer, sizeof(s_UdpBuffer) - 1, DMXSERIAL_FILE_PREFIX "%.3d" DMXSERIAL_FILE_SUFFIX "\n", m_aFileIndex[i]);
			Network::Get()->SendTo(m_nHandle, s_UdpBuffer, nLength, nIPAddressFrom, UDP_PORT);
		}
		return;
	}

	if ((nBytesReceived >= GET_TFTP_LENGTH) && (memcmp(s_UdpBuffer, sGetTFTP, GET_TFTP_LENGTH) == 0)) {
		if (nBytesReceived == GET_TFTP_LENGTH) {
			const uint32_t nLength = snprintf(s_UdpBuffer, UDP_BUFFER_SIZE - 1, "tftp:%s\n", m_bEnableTFTP ? "On" : "Off");
			Network::Get()->SendTo(m_nHandle, s_UdpBuffer, nLength, nIPAddressFrom, UDP_PORT);
		} else if (nBytesReceived == GET_TFTP_LENGTH + 3) {
			if (memcmp(&s_UdpBuffer[GET_TFTP_LENGTH], "bin", 3) == 0) {
				Network::Get()->SendTo(m_nHandle, &m_bEnableTFTP, sizeof(bool) , nIPAddressFrom, UDP_PORT);
			}
		}
		return;
	}

	if ((nBytesReceived == SET_TFTP_LENGTH + 1) && (memcmp(s_UdpBuffer, sSetTFTP, SET_TFTP_LENGTH) == 0)) {
		EnableTFTP(s_UdpBuffer[GET_TFTP_LENGTH] != '0');
		return;
	}

	if (memcmp(s_UdpBuffer, sReload, REQUEST_RELOAD_LENGTH) == 0) {
		Hardware::Get()->SoftReset();
		return;
	}

	if ((nBytesReceived == REQUEST_DELETE_LENGTH + 3) && (memcmp(s_UdpBuffer, sDelete, REQUEST_DELETE_LENGTH) == 0)) {
		s_UdpBuffer[REQUEST_DELETE_LENGTH + 3] = '\0';

		if (DmxSerial::Get()->DeleteFile(&s_UdpBuffer[REQUEST_DELETE_LENGTH])) {
			Network::Get()->SendTo(m_nHandle, "Success\n",  8, nIPAddressFrom, UDP_PORT);
		} else {
			const char *pError = strerror(errno);
			const uint32_t nLength = snprintf(s_UdpBuffer, UDP_BUFFER_SIZE - 1, "%s\n", pError);
			Network::Get()->SendTo(m_nHandle, s_UdpBuffer,  nLength, nIPAddressFrom, UDP_PORT);
		}
		return;
	}
}
