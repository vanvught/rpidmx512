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
#include <stdbool.h>

#include "showfileprotocolhandler.h"
#include "showfiledisplay.h"
#include "showfiletftp.h"

enum TShowFileStatus {
	SHOWFILE_STATUS_IDLE,
	SHOWFILE_STATUS_RUNNING,
	SHOWFILE_STATUS_STOPPED,
	SHOWFILE_STATUS_ENDED,
	SHOWFILE_STATUS_UNDEFINED
};

enum TShowFileFormats {
	SHOWFILE_FORMAT_OLA,
	SHOWFILE_FORMAT_DUMMY,
	SHOWFILE_FORMAT_UNDEFINED
};

enum TShowFileProtocols {
	SHOWFILE_PROTOCOL_SACN,
	SHOWFILE_PROTOCOL_ARTNET,
	SHOWFILE_PROTOCOL_UNDEFINED
};

#define SHOWFILE_PREFIX	"show"
#define SHOWFILE_SUFFIX	".txt"

enum TShowFileFile {
	SHOWFILE_FILE_NAME_LENGTH = sizeof(SHOWFILE_PREFIX "NN" SHOWFILE_SUFFIX) - 1,
	SHOWFILE_FILE_MAX_NUMBER = 99
};

class ShowFile {
public:
	ShowFile(void);
	virtual ~ShowFile(void) {}

	virtual void Start(void)=0;
	virtual void Stop(void)=0;
	virtual void Resume(void)=0;
	virtual void Process(void)=0;
	virtual void Print(void)=0;

	void Run(void);

	void SetHandlers(ShowFileProtocolHandler *pShowFileProtocolHandler) {
		m_pShowFileProtocolHandler = pShowFileProtocolHandler;
	}

	void SetShowFile(uint8_t nShowFileNumber);

	const char *GetShowFileName(void) {
		return (const char*)m_aShowFileName;
	}

	uint8_t GetShowFile(void) {
		return m_nShowFileNumber;
	}

	bool DeleteShowFile(uint8_t nShowFileNumber);

	void DoLoop(bool bDoLoop) {
		m_bDoLoop = bDoLoop;
	}

	void BlackOut(void);

	enum TShowFileStatus GetStatus(void) {
		return m_tShowFileStatus;
	}

	void SetShowFileDisplay(ShowFileDisplay *pShowFileDisplay) {
		m_pShowFileDisplay = pShowFileDisplay;
	}

	void EnableTFTP(bool bEnableTFTP);

public:
	static TShowFileFormats GetFormat(const char *pString);
	static const char *GetFormat(TShowFileFormats tFormat);
	static bool CheckShowFileName(const char *pShowFileName, uint8_t& nShowFileNumber);
	static bool ShowFileNameCopyTo(char *pShowFileName, uint32_t nLength, uint8_t nShowFileNumber);

protected:
	uint8_t m_nShowFileNumber;
	enum TShowFileStatus m_tShowFileStatus;
	FILE *m_pShowFile;
	ShowFileProtocolHandler *m_pShowFileProtocolHandler;
	bool m_bDoLoop;
	ShowFileDisplay *m_pShowFileDisplay;

private:
	uint8_t m_aShowFileName[SHOWFILE_FILE_NAME_LENGTH + 1]; // Inluding '\0'
	bool m_bEnableTFTP;
	ShowFileTFTP *m_pShowFileTFTP;

public:
	static ShowFile* Get(void) {
		return s_pThis;
	}

private:
	static ShowFile *s_pThis;
};

#endif /* SHOWFILE_H_ */
