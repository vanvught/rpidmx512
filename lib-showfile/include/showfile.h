/**
 * @file showfile.h
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

#ifndef SHOWFILE_H_
#define SHOWFILE_H_

#include <cstdio>

#include "showfileconst.h"
#include "showfiletftp.h"
#include "showfileformat.h"
#include "showfileprotocol.h"

#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
# include "showfileosc.h"
#endif

#if defined (OUTPUT_DMX_PIXEL) || defined (OUTPUT_DMX_PIXEL_MULTI)
# define CONFIG_SHOWFILE_ENABLE_MASTER
#endif

#include "debug.h"

namespace showfile {
bool filename_copyto(char *pShowFileName, const uint32_t nLength, const uint32_t nShowFileNumber);
bool filename_check(const char *pShowFileName, uint32_t &nShowFileNumber);
}  // namespace showfile

class ShowFile final: public ShowFileFormat {
public:
#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
	ShowFile(uint16_t nPortIncoming = osc::port::DEFAULT_INCOMING, uint16_t nPortOutgoing = osc::port::DEFAULT_OUTGOING);
#else
	ShowFile();
#endif

	void Play() {
		DEBUG_ENTRY

		if ((m_Mode == showfile::Mode::RECORDER) || (m_Status == showfile::Status::PLAYING) || (m_Status == showfile::Status::RECORDING)) {
			DEBUG_EXIT
			return;
		}

		EnableTFTP(false);

		if (m_pShowFile != nullptr) {
			ShowFileFormat::ShowFileStart();
			SetStatus(showfile::Status::PLAYING);
		} else {
			SetStatus(showfile::Status::IDLE);
		}

		DEBUG_EXIT
	}

	void Stop() {
		DEBUG_ENTRY

		if (m_pShowFile != nullptr) {
			ShowFileFormat::ShowFileStop();

			if ((m_Status == showfile::Status::STOPPED) || (m_Status == showfile::Status::RECORDING)) {
				if (m_Status == showfile::Status::RECORDING) {
					fclose(m_pShowFile);
				}
				SetStatus(showfile::Status::IDLE);
			} else {
				SetStatus(showfile::Status::STOPPED);
			}
		}

		DEBUG_EXIT
	}

	void Resume() {
		DEBUG_ENTRY

		if (m_Status != showfile::Status::STOPPED) {
			DEBUG_EXIT
			return;
		}

		if (m_pShowFile != nullptr) {
			ShowFileFormat::ShowFileResume();
			SetStatus(showfile::Status::PLAYING);
		}

		DEBUG_EXIT
	}

#if !defined (CONFIG_SHOWFILE_DISABLE_RECORD)
	void Record() {
		DEBUG_ENTRY

		if ((m_Mode == showfile::Mode::PLAYER) || (m_Status != showfile::Status::IDLE)) {
			DEBUG_EXIT
			return;
		}

		if (m_pShowFile != nullptr) {
			ShowFileFormat::ShowFileRecord();
			SetStatus(showfile::Status::RECORDING);
		} else {
			SetStatus(showfile::Status::IDLE);
		}

		DEBUG_EXIT
	}
#endif

	void Run() {
		ShowFileFormat::ShowFileRun(m_Status == showfile::Status::PLAYING);
#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
		m_showFileOSC.Run();
#endif
#if !defined(CONFIG_SHOWFILE_DISABLE_TFTP)
		if (m_pShowFileTFTP != nullptr) {
			m_pShowFileTFTP->Run();
		}
#endif
	}

	void Print() {
		puts("Showfile");
		if (m_aShowFileNameCurrent[0] != '\0') {
			printf(" %s\n", m_aShowFileNameCurrent);
		}
		if (m_bAutoPlay) {
			puts(" Auto play");
		}
		printf(" %s\n", m_bDoLoop ? "Looping" : "Not looping");
#if defined (CONFIG_SHOWFILE_DISABLE_RECORD)
		puts(" Recorder is disabled.");
#endif
		ShowFileFormat::ShowFilePrint();
#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
		m_showFileOSC.Print();
#endif
	}

	showfile::Mode GetMode() const {
		return m_Mode;
	}

	void SetStatus(const showfile::Status Status);

	showfile::Status GetStatus() const {
		return m_Status;
	}

	void LoadShows();

	void UnloadShows() {
		m_nShows = 0;

		for (auto &FileIndex : m_nShowFileNumber) {
			FileIndex = -1;
		}

		m_nShowFileCurrent = showfile::FILE_MAX_NUMBER + 1U;
	}

	void SetPlayerShowFileCurrent(const uint32_t nShowFileNumber);

#if !defined (CONFIG_SHOWFILE_DISABLE_RECORD)
	void SetRecorderShowFileCurrent(const uint32_t nShowFileNumber);
#endif

	const char *GetShowFileNameCurrent() const {
		return static_cast<const char *>(m_aShowFileNameCurrent);
	}

	uint32_t GetShowFileCurrent() const {
		return m_nShowFileCurrent;
	}

	uint32_t GetShows() {
		if (m_nShows == 0) {
			LoadShows();
		}
		return m_nShows;
	}

	int32_t GetPlayerShowFile(const uint32_t nIndex) const {
		if (nIndex < sizeof(m_nShowFileNumber) / sizeof(m_nShowFileNumber[0]) ) {
			return m_nShowFileNumber[nIndex];
		}

		return -1;
	}

	bool DeleteShowFile(const uint32_t nShowFileNumber);

	bool GetShowFileSize(const uint32_t nShowFileNumber, uint32_t &nSize);

	void DoLoop(const bool bDoLoop) {
		m_bDoLoop = bDoLoop;
	}

	bool GetDoLoop() const {
		return m_bDoLoop;
	}

	void SetAutoStart(const bool bAutoPlay) {
		m_bAutoPlay = bAutoPlay;
	}

	bool IsAutoStart() const {
		return m_bAutoPlay;
	}

	bool IsSyncDisabled() {
		return ShowFileFormat::IsSyncDisabled();
	}

	void BlackOut() {
#if defined (CONFIG_SHOWFILE_ENABLE_MASTER)
		Stop();
		ShowFileFormat::BlackOut();
#endif
	}

	void SetMaster([[maybe_unused]] const uint32_t nMaster) {
#if defined (CONFIG_SHOWFILE_ENABLE_MASTER)
		ShowFileFormat::SetMaster(nMaster);
#endif
	}

	/*
	 * TFTP
	 */

	void EnableTFTP(bool bEnableTFTP);

	bool IsTFTPEnabled() const 	{
#if !defined(CONFIG_SHOWFILE_DISABLE_TFTP)
		return m_bEnableTFTP;
#else
		return false;
#endif
	}

	/*
	 * OSC
	 */

#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
	void SetOscPortIncoming(const uint16_t nPortIncoming) {
		m_showFileOSC.SetPortIncoming(nPortIncoming);
	}
	uint16_t GetOscPortIncoming() const {
		return m_showFileOSC.GetPortIncoming();
	}

	void SetOscPortOutgoing(const uint16_t nPortOutgoing) {
		m_showFileOSC.SetPortOutgoing(nPortOutgoing);
	}

	uint16_t GetOscPortOutgoing() const {
		return m_showFileOSC.GetPortOutgoing();
	}
#endif

	static ShowFile *Get() {
		return s_pThis;
	}

private:
	void OpenFile(const showfile::Mode mode, const uint32_t nShowFileNumber);
	bool AddShow(const uint32_t nShowFileNumber);

private:
#if defined (CONFIG_SHOWFILE_ENABLE_OSC)
	ShowFileOSC m_showFileOSC;
#endif
	showfile::Mode m_Mode {showfile::Mode::PLAYER };
	showfile::Status m_Status { showfile::Status::IDLE };
	char m_aShowFileNameCurrent[showfile::FILE_NAME_LENGTH + 1]; // Including '\0'
	uint32_t m_nShows { 0 };
	int32_t m_nShowFileNumber[showfile::FILE_MAX_NUMBER + 1];
	bool m_bAutoPlay { false };
#if !defined(CONFIG_SHOWFILE_DISABLE_TFTP)
	bool m_bEnableTFTP { false };
	ShowFileTFTP *m_pShowFileTFTP { nullptr };
#endif
	static ShowFile *s_pThis;
};

#endif /* SHOWFILE_H_ */
