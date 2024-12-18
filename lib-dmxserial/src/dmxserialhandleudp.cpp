/**
 * @file dmxserialhandleudp.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <errno.h>
#include <cassert>

#include "dmxserial.h"
#include "dmxserial_internal.h"

#include "hardware.h"
#include "network.h"
#include "net/protocol/udp.h"

#include "debug.h"

namespace cmd {
	static constexpr char REQUEST_FILES[] = "?files#";
	static constexpr char GET_TFTP[] = "?tftp#";
	static constexpr char SET_TFTP[] = "!tftp#";
	static constexpr char REQUEST_RELOAD[] = "?reload##";
	static constexpr char REQUEST_DELETE[] = "!delete#";
}

namespace length {
	static constexpr auto REQUEST_FILES = sizeof(cmd::REQUEST_FILES) - 1;
	static constexpr auto GET_TFTP = sizeof(cmd::GET_TFTP) - 1;
	static constexpr auto SET_TFTP = sizeof(cmd::SET_TFTP) - 1;
	static constexpr auto REQUEST_RELOAD = sizeof(cmd::REQUEST_RELOAD) - 1;
	static constexpr auto REQUEST_DELETE = sizeof(cmd::REQUEST_DELETE) - 1;
}

void DmxSerial::HandleUdp() {
	uint16_t nForeignPort;
	uint32_t nIPAddressFrom;

	auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&m_pReceiveBuffer)), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((nBytesReceived < 6), 1)) {
		return;
	}

	if (m_pReceiveBuffer[nBytesReceived - 1] == '\n') {
		nBytesReceived--;
	}

#ifndef NDEBUG
	debug_dump(m_pReceiveBuffer, nBytesReceived);
#endif

	if (memcmp(m_pReceiveBuffer, cmd::REQUEST_FILES, length::REQUEST_FILES) == 0) {
		for (uint32_t i = 0; i < m_nFilesCount; i++) {
			const auto nLength = snprintf(reinterpret_cast<char *>(m_pReceiveBuffer), UDP_DATA_SIZE - 1, DMXSERIAL_FILE_PREFIX "%.3d" DMXSERIAL_FILE_SUFFIX "\n", m_aFileIndex[i]);
			Network::Get()->SendTo(m_nHandle, m_pReceiveBuffer, nLength, nIPAddressFrom, UDP::PORT);
		}
		return;
	}

	if ((nBytesReceived >= length::GET_TFTP) && (memcmp(m_pReceiveBuffer, cmd::GET_TFTP, length::GET_TFTP) == 0)) {
		if (nBytesReceived == length::GET_TFTP) {
			const auto nLength = snprintf(reinterpret_cast<char *>(m_pReceiveBuffer), UDP_DATA_SIZE - 1, "tftp:%s\n", m_bEnableTFTP ? "On" : "Off");
			Network::Get()->SendTo(m_nHandle, m_pReceiveBuffer,nLength, nIPAddressFrom, UDP::PORT);
			return;
		}

		if (nBytesReceived == length::GET_TFTP + 3) {
			if (memcmp(&m_pReceiveBuffer[length::GET_TFTP], "bin", 3) == 0) {
				Network::Get()->SendTo(m_nHandle, &m_bEnableTFTP, sizeof(bool) , nIPAddressFrom, UDP::PORT);
				return;
			}
		}
	}

	if ((nBytesReceived == length::SET_TFTP + 1) && (memcmp(m_pReceiveBuffer, cmd::SET_TFTP, length::SET_TFTP) == 0)) {
		EnableTFTP(m_pReceiveBuffer[length::SET_TFTP] != '0');
		return;
	}

	if (memcmp(m_pReceiveBuffer, cmd::REQUEST_RELOAD, length::REQUEST_RELOAD) == 0) {
		Hardware::Get()->Reboot();
		return;
	}

	if ((nBytesReceived == length::REQUEST_DELETE + 3) && (memcmp(m_pReceiveBuffer, cmd::REQUEST_DELETE, length::REQUEST_DELETE) == 0)) {
		m_pReceiveBuffer[length::REQUEST_DELETE + 3] = '\0';

		if (DmxSerial::Get()->DeleteFile(reinterpret_cast<char *>(&m_pReceiveBuffer[length::REQUEST_DELETE]))) {
			Network::Get()->SendTo(m_nHandle, "Success\n",  8, nIPAddressFrom, UDP::PORT);
		} else {
			const auto *pError = strerror(errno);
			const auto nLength = snprintf(reinterpret_cast<char *>(m_pReceiveBuffer), UDP_DATA_SIZE - 1, "%s\n", pError);
			Network::Get()->SendTo(m_nHandle, m_pReceiveBuffer, nLength, nIPAddressFrom, UDP::PORT);
		}
		return;
	}
}
