/**
 * @file showfile.h
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

#ifndef SHOWFILE_H_
#define SHOWFILE_H_

#include <stdio.h>

#include "showfileprotocolhandler.h"
#include "showfiledisplay.h"
#include "showfiletftp.h"

enum class ShowFileStatus {
	IDLE,
	RUNNING,
	STOPPED,
	ENDED,
	UNDEFINED
};

enum class ShowFileFormats {
	OLA,
	DUMMY,
	UNDEFINED
};

enum class ShowFileProtocols {
	SACN,
	ARTNET,
	UNDEFINED
};

#define SHOWFILE_PREFIX	"show"
#define SHOWFILE_SUFFIX	".txt"

struct ShowFileFile {
	static constexpr auto NAME_LENGTH = sizeof(SHOWFILE_PREFIX "NN" SHOWFILE_SUFFIX) - 1;
	static constexpr auto MAX_NUMBER = 99;
};

class ShowFile {
public:
	ShowFile(void);
	virtual ~ShowFile(void) {}

	void Start(void);
	void Stop(void);
	void Resume(void);
	void Run(void);
	void Print(void);

	void SetShowFileStatus(ShowFileStatus tShowFileStatus);

	void SetProtocolHandler(ShowFileProtocolHandler *pShowFileProtocolHandler) {
		m_pShowFileProtocolHandler = pShowFileProtocolHandler;
	}
	ShowFileProtocolHandler *GetProtocolHandler(void) {
		return m_pShowFileProtocolHandler;
	}

	void SetShowFile(uint8_t nShowFileNumber);

	const char *GetShowFileName(void) {
		return static_cast<const char *>(m_aShowFileName);
	}

	uint8_t GetShowFile(void) {
		return m_nShowFileNumber;
	}

	bool DeleteShowFile(uint8_t nShowFileNumber);

	void DoLoop(bool bDoLoop) {
		m_bDoLoop = bDoLoop;
	}
	bool GetDoLoop(void) {
		return m_bDoLoop;
	}

	void BlackOut(void);

	void SetMaster(uint32_t nMaster) {
		if (m_pShowFileProtocolHandler != 0) {
			m_pShowFileProtocolHandler->DmxMaster(nMaster);
		}
	}

	enum ShowFileStatus GetStatus(void) {
		return m_tShowFileStatus;
	}

	void SetShowFileDisplay(ShowFileDisplay *pShowFileDisplay) {
		m_pShowFileDisplay = pShowFileDisplay;
	}

	void EnableTFTP(bool bEnableTFTP);
	bool IsTFTPEnabled(void) {
		return m_bEnableTFTP;
	}

	void UpdateDisplayStatus(void) {
		if (m_pShowFileDisplay != 0) {
			m_pShowFileDisplay->ShowShowFileStatus();
		}
	}

	static ShowFileFormats GetFormat(const char *pString);
	static const char *GetFormat(ShowFileFormats tFormat);
	static bool CheckShowFileName(const char *pShowFileName, uint8_t& nShowFileNumber);
	static bool ShowFileNameCopyTo(char *pShowFileName, uint32_t nLength, uint8_t nShowFileNumber);

	static ShowFile* Get(void) {
		return s_pThis;
	}

protected:
	virtual void ShowFileStart(void)=0;
	virtual void ShowFileStop(void)=0;
	virtual void ShowFileResume(void)=0;
	virtual void ShowFileRun(void)=0;
	virtual void ShowFilePrint(void)=0;

protected:
	uint8_t m_nShowFileNumber = ShowFileFile::MAX_NUMBER + 1;
	bool m_bDoLoop = false;
	FILE *m_pShowFile = 0;
	ShowFileProtocolHandler *m_pShowFileProtocolHandler = 0;
	ShowFileDisplay *m_pShowFileDisplay = 0;

private:
	ShowFileStatus m_tShowFileStatus = ShowFileStatus::IDLE;
	char m_aShowFileName[ShowFileFile::NAME_LENGTH + 1]; // Including '\0'
	bool m_bEnableTFTP = false;
	ShowFileTFTP *m_pShowFileTFTP = 0;

	static ShowFile *s_pThis;
};

#endif /* SHOWFILE_H_ */
