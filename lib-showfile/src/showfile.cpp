/**
 * @file showfile.cpp
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

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "showfile.h"
#include "showfiletftp.h"

#include "debug.h"

ShowFile *ShowFile::s_pThis = 0;

ShowFile::ShowFile(void) :
	m_nShowFileNumber(255),
	m_tShowFileStatus(SHOWFILE_STATUS_IDLE),
	m_pShowFile(0),
	m_pShowFileProtocolHandler(0),
	m_bDoLoop(false),
	m_pShowFileDisplay(0),
	m_bEnableTFTP(false),
	m_pShowFileTFTP(0)
{
	DEBUG_ENTRY

	s_pThis = this;

	m_aShowFileName[0] = '\0';

	DEBUG_EXIT
}

void ShowFile::SetShowFile(uint8_t nShowFileNumber) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nShowFileNumber=%u", nShowFileNumber);

	if (nShowFileNumber <= SHOWFILE_FILE_MAX_NUMBER) {
		Stop();

		if (m_pShowFile != 0) {
			fclose(m_pShowFile);
			m_pShowFile = 0;
		}

		m_nShowFileNumber = nShowFileNumber;

		ShowFileNameCopyTo((char *)m_aShowFileName, sizeof(m_aShowFileName), nShowFileNumber);

		DEBUG_PRINTF("m_aShowFileName=[%s]", m_aShowFileName);

		m_pShowFile = fopen((const char *) m_aShowFileName, "r");

		if (m_pShowFile == 0) {
			perror((const char *) m_aShowFileName);
			m_aShowFileName[0] = '\0';
		}

		if (m_pShowFileDisplay != 0) {
			m_pShowFileDisplay->ShowFileName((const char*) m_aShowFileName, nShowFileNumber);
			m_pShowFileDisplay->ShowShowFileStatus();
		}
	}

	DEBUG_EXIT
}

void ShowFile::BlackOut(void) {
	if (m_pShowFileProtocolHandler != 0) {
		Stop();
		m_pShowFileProtocolHandler->DmxBlackout();
	}
}

void ShowFile::Print(void) {
	printf("[%s]\n", (const char *) m_aShowFileName);
	printf("%s\n", m_bDoLoop ? "Looping" : "Not looping");
}

bool ShowFile::DeleteShowFile(uint8_t nShowFileNumber) {
	DEBUG_PRINTF("nShowFileNumber=%u", (unsigned) nShowFileNumber);

	char aFileName[SHOWFILE_FILE_NAME_LENGTH + 1];

	if (ShowFileNameCopyTo(aFileName, sizeof(aFileName), nShowFileNumber)) {
		const int nResult = unlink((const char *)aFileName);
		DEBUG_PRINTF("nResult=%d", nResult);
		DEBUG_EXIT
		return (nResult == 0);
	}

	DEBUG_EXIT
	return false;
}

void ShowFile::EnableTFTP(bool bEnableTFTP) {
	if (bEnableTFTP == m_bEnableTFTP) {
		return;
	}

	m_bEnableTFTP = bEnableTFTP;

	if (m_bEnableTFTP) {
		assert(m_pShowFileTFTP == 0);

		m_pShowFileTFTP = new ShowFileTFTP;
		assert(m_pShowFileTFTP != 0);
	} else {
		assert(m_pShowFileTFTP != 0);

		delete m_pShowFileTFTP;
		m_pShowFileTFTP = 0;
	}
}

void ShowFile::Run(void) {
	Process();

	if (m_pShowFileTFTP == 0) {
		return;
	}

	m_pShowFileTFTP->Run();
}
