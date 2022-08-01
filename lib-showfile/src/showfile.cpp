/**
 * @file showfile.cpp
 *
 */
/* Copyright (C) 2020-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdio>
#include <unistd.h>
#include <cassert>

#include "showfile.h"
#include "showfiletftp.h"

#include "ledblink.h"

#include "debug.h"

ShowFile *ShowFile::s_pThis;

ShowFile::ShowFile() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	m_aShowFileName[0] = '\0';

	DEBUG_EXIT
}

void ShowFile::SetShowFile(uint32_t nShowFileNumber) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nShowFileNumber=%u", nShowFileNumber);

	if (nShowFileNumber <= showfile::File::MAX_NUMBER) {
		ShowFileStop();

		if (m_pShowFile != nullptr) {
			if (fclose(m_pShowFile) != 0) {
				perror("fclose(m_pShowFile)");
			}
			m_pShowFile = nullptr;
		}

		m_nShowFileNumber = nShowFileNumber;

		ShowFileNameCopyTo(m_aShowFileName, sizeof(m_aShowFileName), nShowFileNumber);

		DEBUG_PRINTF("m_aShowFileName=[%s]", m_aShowFileName);

		m_pShowFile = fopen(m_aShowFileName, "r");

		if (m_pShowFile == nullptr) {
			perror(const_cast<char *>(m_aShowFileName));
			m_aShowFileName[0] = '\0';
		}

		if (m_pShowFileDisplay != nullptr) {
			m_pShowFileDisplay->ShowFileName(m_aShowFileName, nShowFileNumber);
			m_pShowFileDisplay->ShowShowFileStatus();
		}
	}

	DEBUG_EXIT
}

bool ShowFile::DeleteShowFile(__attribute__((unused)) uint32_t nShowFileNumber) {
	DEBUG_ENTRY
#if !defined(CONFIG_SHOWFILE_DISABLE_TFTP)
	DEBUG_PRINTF("nShowFileNumber=%u, m_bEnableTFTP=%d", nShowFileNumber, m_bEnableTFTP);

	if (!m_bEnableTFTP) {
		DEBUG_EXIT
		return false;
	}

	char aFileName[showfile::File::NAME_LENGTH + 1];

	if (ShowFileNameCopyTo(aFileName, sizeof(aFileName), nShowFileNumber)) {
		const int nResult = unlink(aFileName);
		DEBUG_PRINTF("nResult=%d", nResult);
		DEBUG_EXIT
		return (nResult == 0);
	}
#endif
	DEBUG_EXIT
	return false;
}

void ShowFile::EnableTFTP(__attribute__((unused))bool bEnableTFTP) {
	DEBUG_ENTRY
#if !defined(CONFIG_SHOWFILE_DISABLE_TFTP)

	if (bEnableTFTP == m_bEnableTFTP) {
		DEBUG_EXIT
		return;
	}

	m_bEnableTFTP = bEnableTFTP;

	if (m_bEnableTFTP) {
		assert(m_pShowFileTFTP == nullptr);

		Stop();

		if (m_pShowFile != nullptr) {
			if (fclose(m_pShowFile) != 0) {
				perror("fclose(m_pShowFile)");
			}
			m_pShowFile = nullptr;
		}

		m_pShowFileTFTP = new ShowFileTFTP;
		assert(m_pShowFileTFTP != nullptr);
	} else {
		assert(m_pShowFileTFTP != nullptr);

		delete m_pShowFileTFTP;
		m_pShowFileTFTP = nullptr;

		SetShowFile(m_nShowFileNumber);
		SetStatus(showfile::Status::IDLE);
	}

	UpdateDisplayStatus();
#endif
	DEBUG_EXIT
}

void ShowFile::SetStatus(showfile::Status Status) {
	DEBUG_ENTRY

	if (Status == m_Status) {
		DEBUG_EXIT
		return;
	}

	m_Status = Status;

	switch (m_Status) {
		case showfile::Status::IDLE:
			m_pShowFileProtocolHandler->DoRunCleanupProcess(true);
			LedBlink::Get()->SetMode(ledblink::Mode::NORMAL);
			break;
		case showfile::Status::RUNNING:
			m_pShowFileProtocolHandler->DoRunCleanupProcess(false);
			LedBlink::Get()->SetMode(ledblink::Mode::DATA);
			break;
		case showfile::Status::STOPPED:
		case showfile::Status::ENDED:
			m_pShowFileProtocolHandler->DoRunCleanupProcess(true);
			LedBlink::Get()->SetMode(ledblink::Mode::NORMAL);
			break;
		default:
			break;
	}

	UpdateDisplayStatus();

	DEBUG_EXIT
}
