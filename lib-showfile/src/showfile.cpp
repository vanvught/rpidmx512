/**
 * @file showfile.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <dirent.h>
#include <unistd.h>
#include <cassert>

#include "showfile.h"
#include "showfiletftp.h"
#include "showfiledisplay.h"

#if defined CONFIG_USB_HOST_MSC
# include "device/usb/host.h"
#endif

#include "hardware.h"

#include "debug.h"

ShowFile *ShowFile::s_pThis;

#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
ShowFile::ShowFile(uint16_t nPortIncoming, uint16_t nPortOutgoing): m_showFileOSC(nPortIncoming, nPortOutgoing) {
#else
ShowFile::ShowFile() {
#endif
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	m_aShowFileNameCurrent[0] = '\0';

	DEBUG_EXIT
}

void ShowFile::OpenFile(const showfile::Mode mode, const uint32_t nShowFileNumber) {
	DEBUG_ENTRY

	if (showfile::filename_copyto(m_aShowFileNameCurrent, sizeof(m_aShowFileNameCurrent), nShowFileNumber)) {
#if defined CONFIG_USB_HOST_MSC
		if (usb::host::get_status() != usb::host::Status::READY) {
			DEBUG_EXIT
			return;
		}
#endif

		if (m_pShowFile != nullptr) {
			if (fclose(m_pShowFile) != 0) {
				perror("fclose()");
			}
			m_pShowFile = nullptr;
		}

		m_pShowFile = fopen(m_aShowFileNameCurrent, mode == showfile::Mode::RECORDER ? "w" : "r");

		if (m_pShowFile == nullptr) {
			perror(const_cast<char *>(m_aShowFileNameCurrent));
			m_aShowFileNameCurrent[0] = '\0';
		} else {
			m_nShowFileCurrent = nShowFileNumber;
			m_Mode = mode;
		}

		showfile::display_filename(m_aShowFileNameCurrent, m_nShowFileCurrent);
		showfile::display_status();

		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
	return;
}

void ShowFile::SetPlayerShowFileCurrent(const uint32_t nShowFileNumber) {
	DEBUG_ENTRY
	if (m_Status != showfile::Status::IDLE) {
		DEBUG_EXIT
		return;
	}

	DEBUG_PRINTF("nShowFileNumber=%u", nShowFileNumber);

	OpenFile(showfile::Mode::PLAYER, nShowFileNumber);

	DEBUG_EXIT
}

#if !defined (CONFIG_SHOWFILE_DISABLE_RECORD)
void ShowFile::SetRecorderShowFileCurrent(const uint32_t nShowFileNumber) {
	DEBUG_ENTRY

	if (m_Status != showfile::Status::IDLE) {
		DEBUG_EXIT
		return;
	}

	DEBUG_PRINTF("nShowFileNumber=%u", nShowFileNumber);

	if (AddShow(nShowFileNumber)) {
		OpenFile(showfile::Mode::RECORDER, nShowFileNumber);

		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}
#endif

bool ShowFile::DeleteShowFile(const uint32_t nShowFileNumber) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nShowFileNumber=%u", nShowFileNumber);

	char aFileName[showfile::FILE_NAME_LENGTH + 1U];

	if (showfile::filename_copyto(aFileName, sizeof(aFileName), nShowFileNumber)) {
		if (nShowFileNumber == m_nShowFileCurrent) {
			if (fclose(m_pShowFile) != 0) {
				perror("fclose()");
			}
			m_pShowFile = nullptr;
			m_nShowFileCurrent = -1;
		}
		const auto nResult = unlink(aFileName);
		DEBUG_PRINTF("nResult=%d", nResult);
		DEBUG_EXIT
		return (nResult == 0);
	}

	DEBUG_EXIT
	return false;
}

bool ShowFile::GetShowFileSize(const uint32_t nShowFileNumber, uint32_t &nSize) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nShowFileNumber=%u", nShowFileNumber);

	char aFileName[showfile::FILE_NAME_LENGTH + 1U];

	if (showfile::filename_copyto(aFileName, sizeof(aFileName), nShowFileNumber)) {
		auto *pFile = fopen(aFileName, "r");
		if (pFile != nullptr) {
			if (fseek(pFile, 0L, SEEK_END) == 0) {
				nSize = ftell(pFile);
				fclose(pFile);
				DEBUG_PRINTF("nSize=%u", nSize);
				DEBUG_EXIT
				return true;
			}
		} else {
			perror("fopen()");
		}
	}

	DEBUG_EXIT
	return false;
}

bool ShowFile::AddShow(const uint32_t nShowFileNumber) {
	DEBUG_ENTRY

	if (m_nShows == (showfile::FILE_MAX_NUMBER)) {
		DEBUG_EXIT
		return false;
	}

	if (m_nShows == 0) {
		m_nShowFileNumber[0] = static_cast<int8_t>(nShowFileNumber);
	} else {
		for (auto &Show : m_nShowFileNumber) {
			if (Show == static_cast<int32_t>(nShowFileNumber)) {
				DEBUG_EXIT
				return true;
			}
		}

		int32_t i = m_nShows - 1;
		while ((static_cast<int32_t>(nShowFileNumber) < m_nShowFileNumber[i]) && i >= 0) {
			m_nShowFileNumber[i + 1] = m_nShowFileNumber[i];
			i--;
		}

		m_nShowFileNumber[i + 1] = static_cast<int8_t>(nShowFileNumber);
	}

	m_nShows++;

	DEBUG_EXIT
	return true;
}

void ShowFile::LoadShows() {
	DEBUG_ENTRY

#if defined (CONFIG_USB_HOST_MSC)
	auto *dirp = opendir("0:/");
#else
	auto *dirp = opendir(".");
#endif

	if (dirp == nullptr) {
		perror("opendir");
		DEBUG_EXIT
		return;
	}

	for (auto &FileIndex : m_nShowFileNumber) {
		FileIndex = -1;
	}

	m_nShows = 0;

	struct dirent *dp;

    do {
        if ((dp = readdir(dirp)) != nullptr) {
        	if (dp->d_type == DT_DIR) {
        		continue;
        	}

          	uint32_t nShowFileNumber;
        	if (!showfile::filename_check(dp->d_name, nShowFileNumber)) {
                continue;
            }

            DEBUG_PRINTF("Found %s", dp->d_name);

            if (!AddShow(nShowFileNumber)) {
            	break;
            }
        }
    } while (dp != nullptr);

	closedir(dirp);

    DEBUG_EXIT
}

void ShowFile::EnableTFTP([[maybe_unused]] bool bEnableTFTP) {
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

		LoadShows();
		SetPlayerShowFileCurrent(m_nShowFileCurrent);
		SetStatus(showfile::Status::IDLE);
	}

	showfile::display_status();
#endif

	DEBUG_EXIT
}

void ShowFile::SetStatus(const showfile::Status Status) {
	DEBUG_ENTRY

	if (Status == m_Status) {
		DEBUG_EXIT
		return;
	}

	m_Status = Status;

	switch (m_Status) {
		case showfile::Status::IDLE:
			ShowFileFormat::DoRunCleanupProcess(true);
			Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
			break;
		case showfile::Status::PLAYING:
		case showfile::Status::RECORDING:
			ShowFileFormat::DoRunCleanupProcess(false);
			Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
			break;
		case showfile::Status::STOPPED:
		case showfile::Status::ENDED:
			ShowFileFormat::DoRunCleanupProcess(true);
			Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
			break;
		default:
			break;
	}

	showfile::display_status();

	DEBUG_EXIT
}
