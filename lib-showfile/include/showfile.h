/**
 * @file showfile.h
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

#ifndef SHOWFILE_H_
#define SHOWFILE_H_

#include <cstdio>

#include "showfileprotocolhandler.h"
#include "showfiledisplay.h"
#include "showfiletftp.h"

#include "debug.h"

namespace showfile {
enum class Status {
	IDLE, RUNNING, STOPPED, ENDED, UNDEFINED
};

enum class Formats {
	OLA, DUMMY, UNDEFINED
};

enum class Protocols {
	SACN, ARTNET, INTERNAL, UNDEFINED
};

#define SHOWFILE_PREFIX	"show"
#define SHOWFILE_SUFFIX	".txt"

struct File {
	static constexpr auto NAME_LENGTH = sizeof(SHOWFILE_PREFIX "NN" SHOWFILE_SUFFIX) - 1;
	static constexpr auto MAX_NUMBER = 99;
};
}  // namespace showfile

class ShowFile {
public:
	ShowFile();
	virtual ~ShowFile() {}

	void Start() {
		DEBUG_ENTRY

		EnableTFTP(false);

		if (m_pShowFile != nullptr) {
			ShowFileStart();
			SetStatus(showfile::Status::RUNNING);
		} else {
			SetStatus(showfile::Status::STOPPED);
		}

		DEBUG_EXIT
	}

	void Stop() {
		DEBUG_ENTRY

		if (m_pShowFile != nullptr) {
			ShowFileStop();
			SetStatus(showfile::Status::STOPPED);
		}

		DEBUG_EXIT
	}

	void Resume() {
		DEBUG_ENTRY

		if (m_pShowFile != nullptr) {
			ShowFileResume();
			SetStatus(showfile::Status::RUNNING);
		}

		DEBUG_EXIT
	}

	void Run() {
		if (m_Status == showfile::Status::RUNNING) {
			ShowFileRun();
			return;
		}
#if !defined(CONFIG_SHOWFILE_DISABLE_TFTP)
		if (m_pShowFileTFTP != nullptr) {
			m_pShowFileTFTP->Run();
		}
#endif
	}

	void Print() {
		printf("[%s]\n", m_aShowFileName);
		printf("%s\n", m_bDoLoop ? "Looping" : "Not looping");
		ShowFilePrint();
	}

	void SetStatus(showfile::Status Status);

	showfile::Status GetStatus() const {
		return m_Status;
	}

	void SetProtocolHandler(ShowFileProtocolHandler *pShowFileProtocolHandler) {
		m_pShowFileProtocolHandler = pShowFileProtocolHandler;
	}

	ShowFileProtocolHandler *GetProtocolHandler() const {
		return m_pShowFileProtocolHandler;
	}

	void SetShowFile(uint32_t nShowFileNumber);

	const char *GetShowFileName() const {
		return static_cast<const char *>(m_aShowFileName);
	}

	uint32_t GetShowFile() const {
		return m_nShowFileNumber;
	}

	bool DeleteShowFile(uint32_t nShowFileNumber);

	void DoLoop(bool bDoLoop) {
		m_bDoLoop = bDoLoop;
	}

	bool GetDoLoop() const {
		return m_bDoLoop;
	}

	void BlackOut() {
		if (m_pShowFileProtocolHandler != nullptr) {
			Stop();
			m_pShowFileProtocolHandler->DmxBlackout();
		}
	}

	void SetMaster(uint32_t nMaster) {
		if (m_pShowFileProtocolHandler != nullptr) {
			m_pShowFileProtocolHandler->DmxMaster(nMaster);
		}
	}

	void SetShowFileDisplay(ShowFileDisplay *pShowFileDisplay) {
		m_pShowFileDisplay = pShowFileDisplay;
	}

	void EnableTFTP(bool bEnableTFTP);

	bool IsTFTPEnabled() const 	{
#if !defined(CONFIG_SHOWFILE_DISABLE_TFTP)
		return m_bEnableTFTP;
#else
		return false;
#endif
	}

	void UpdateDisplayStatus() {
		if (m_pShowFileDisplay != nullptr) {
			m_pShowFileDisplay->ShowShowFileStatus();
		}
	}

	static showfile::Formats GetFormat(const char *pString);
	static const char *GetFormat(showfile::Formats Format);
	static bool CheckShowFileName(const char *pShowFileName, uint32_t& nShowFileNumber);
	static bool ShowFileNameCopyTo(char *pShowFileName, uint32_t nLength, uint32_t nShowFileNumber);

	static ShowFile* Get() {
		return s_pThis;
	}

protected:
	virtual void ShowFileStart()=0;
	virtual void ShowFileStop()=0;
	virtual void ShowFileResume()=0;
	virtual void ShowFileRun()=0;
	virtual void ShowFilePrint()=0;

protected:
	uint32_t m_nShowFileNumber { showfile::File::MAX_NUMBER + 1 };
	bool m_bDoLoop { false };
	FILE *m_pShowFile { nullptr };
	ShowFileProtocolHandler *m_pShowFileProtocolHandler { nullptr };
	ShowFileDisplay *m_pShowFileDisplay { nullptr };

private:
	showfile::Status m_Status { showfile::Status::IDLE };
	char m_aShowFileName[showfile::File::NAME_LENGTH + 1]; // Including '\0'
#if !defined(CONFIG_SHOWFILE_DISABLE_TFTP)
	bool m_bEnableTFTP { false };
	ShowFileTFTP *m_pShowFileTFTP { nullptr };
#endif
	static ShowFile *s_pThis;
};

#endif /* SHOWFILE_H_ */
