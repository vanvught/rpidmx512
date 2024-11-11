/**
 * @file showfileosc.cpp
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

#if defined (DEBUG_SHOWFILEOSC)
# undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <dirent.h>
#include <cassert>

#include "showfileosc.h"
#include "showfile.h"

#include "oscsimplemessage.h"
#include "oscsimplesend.h"

#include "network.h"

#include "debug.h"

namespace cmd {
	static constexpr char START[] = "start";
	static constexpr char STOP[] = "stop";
	static constexpr char RESUME[] = "resume";
	static constexpr char SHOW[] = "show";
	static constexpr char LOOP[] = "loop";
	static constexpr char BO[] = "blackout";
#if defined (CONFIG_SHOWFILE_ENABLE_MASTER)
	static constexpr char MASTER[] = "master";
#endif
	static constexpr char TFTP[] = "tftp";
	static constexpr char DELETE[] = "delete";
	// TouchOSC specific
	static constexpr char RELOAD[] = "reload";
	static constexpr char INDEX[] = "index";
}

namespace length {
	static constexpr uint32_t START = sizeof(cmd::START) - 1;
	static constexpr uint32_t STOP = sizeof(cmd::STOP) - 1;
	static constexpr uint32_t RESUME = sizeof(cmd::RESUME) - 1;
	static constexpr uint32_t SHOW = sizeof(cmd::SHOW) - 1;
	static constexpr uint32_t LOOP = sizeof(cmd::LOOP) - 1;
	static constexpr uint32_t BO = sizeof(cmd::BO) - 1;
#if defined (CONFIG_SHOWFILE_ENABLE_MASTER)
	static constexpr uint32_t MASTER = sizeof(cmd::MASTER) - 1;
#endif
	static constexpr uint32_t TFTP = sizeof(cmd::TFTP) - 1;
	static constexpr uint32_t DELETE = sizeof(cmd::DELETE) - 1;
	// TouchOSC specific
	static constexpr uint32_t RELOAD = sizeof(cmd::RELOAD) - 1;
	static constexpr uint32_t INDEX = sizeof(cmd::INDEX) - 1;
}

void ShowFileOSC::Process() {
	DEBUG_PRINTF("[%s] %d,%d %s", m_pBuffer, static_cast<int>(strlen(reinterpret_cast<const char *>(m_pBuffer))), static_cast<int>(showfileosc::PATH_LENGTH), &m_pBuffer[showfileosc::PATH_LENGTH]);

	if (memcmp(&m_pBuffer[showfileosc::PATH_LENGTH], cmd::START, length::START) == 0) {
		ShowFile::Get()->Play();
		SendStatus();
		DEBUG_PUTS("ActionStart");
		return;
	}

	if (memcmp(&m_pBuffer[showfileosc::PATH_LENGTH], cmd::STOP, length::STOP) == 0){
		ShowFile::Get()->Stop();
		SendStatus();
		DEBUG_PUTS("ActionStop");
		return;
	}

	if (memcmp(&m_pBuffer[showfileosc::PATH_LENGTH], cmd::RESUME, length::RESUME) == 0) {
		ShowFile::Get()->Resume();
		SendStatus();
		DEBUG_PUTS("ActionResume");
		return;
	}

	if (memcmp(&m_pBuffer[showfileosc::PATH_LENGTH], cmd::SHOW, length::SHOW) == 0) {
		OscSimpleMessage Msg(m_pBuffer, m_nBytesReceived);

		const auto nValue = static_cast<uint32_t>(Msg.GetInt(0));

		if (nValue <= showfile::FILE_MAX_NUMBER) {
			ShowFile::Get()->SetPlayerShowFileCurrent(nValue);
			SendStatus();
		}

		DEBUG_PRINTF("Show %d", nValue);
		return;
	}

	if (memcmp(&m_pBuffer[showfileosc::PATH_LENGTH], cmd::LOOP, length::LOOP) == 0) {
		OscSimpleMessage Msg(m_pBuffer, m_nBytesReceived);

		int nValue;

		if (Msg.GetType(0) == osc::type::INT32) {
			nValue = Msg.GetInt(0);
		} else if (Msg.GetType(0) == osc::type::FLOAT) { // TouchOSC
			nValue = static_cast<int>(Msg.GetFloat(0));
		} else {
			return;
		}

		ShowFile::Get()->DoLoop(nValue != 0);
		SendStatus();
		showfile::display_status();

		DEBUG_PRINTF("Loop %d", nValue != 0);
		return;
	}

	if (memcmp(&m_pBuffer[showfileosc::PATH_LENGTH], cmd::BO, length::BO) == 0) {
		ShowFile::Get()->BlackOut();
		SendStatus();
		DEBUG_PUTS("Blackout");
		return;
	}

#if defined (CONFIG_SHOWFILE_ENABLE_MASTER)
	if (memcmp(&m_pBuffer[showfileosc::PATH_LENGTH], cmd::MASTER, length::MASTER) == 0) {
		OscSimpleMessage Msg(m_pBuffer, m_nBytesReceived);

		int nValue;

		if (Msg.GetType(0) == osc::type::INT32) {
			nValue = Msg.GetInt(0);
		} else if (Msg.GetType(0) == osc::type::FLOAT) { // TouchOSC
			nValue = static_cast<int>(Msg.GetFloat(0));
		} else {
			return;
		}

		if ((nValue >= 0) &&  (nValue <= 255)) {
			ShowFile::Get()->SetMaster(static_cast<uint32_t>(nValue));
		}

		DEBUG_PRINTF("Master %d", nValue);
		return;
	}
#endif

	if (memcmp(&m_pBuffer[showfileosc::PATH_LENGTH], cmd::TFTP, length::TFTP) == 0) {
		OscSimpleMessage Msg(m_pBuffer, m_nBytesReceived);

		int nValue;

		if (Msg.GetType(0) == osc::type::INT32) {
			nValue = Msg.GetInt(0);
		} else if (Msg.GetType(0) == osc::type::FLOAT) { // TouchOSC
			nValue = static_cast<int>(Msg.GetFloat(0));
		} else {
			return;
		}

		ShowFile::Get()->EnableTFTP(nValue != 0);
		SendStatus();

		DEBUG_PRINTF("TFTP %d", nValue != 0);
		return;
	}


	if (memcmp(&m_pBuffer[showfileosc::PATH_LENGTH], cmd::DELETE, length::DELETE) == 0) {
		OscSimpleMessage Msg(m_pBuffer, m_nBytesReceived);

		uint32_t nValue = 255;

		if (Msg.GetType(0) == osc::type::INT32){
			nValue = static_cast<uint32_t>(Msg.GetInt(0));
		} else {
			return;
		}

		DEBUG_PRINTF("nValue=%d", nValue);

		if (nValue <= showfile::FILE_MAX_NUMBER) {
			char aShowFileName[showfile::FILE_NAME_LENGTH + 1];

			showfile::filename_copyto(aShowFileName, sizeof(aShowFileName), nValue);

			OscSimpleSend MsgName(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/name", "s", aShowFileName);

			if (ShowFile::Get()->DeleteShowFile(nValue)) {
				OscSimpleSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/status", "s", "Deleted");
			} else {
				OscSimpleSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/status", "s", "Not deleted");
			}
		}

		DEBUG_PRINTF("Delete %d", nValue);
		return;
	}

	if (memcmp(&m_pBuffer[showfileosc::PATH_LENGTH], cmd::INDEX, length::INDEX) == 0) {
		OscSimpleMessage Msg(m_pBuffer, m_nBytesReceived);

		if (Msg.GetType(0) != osc::type::FLOAT){
			return;
		}

		const auto nIndex = static_cast<uint32_t>(Msg.GetFloat(0));

		DEBUG_PRINTF("Index %u", nIndex);

		const auto nShow = ShowFile::Get()->GetPlayerShowFile(nIndex);

		if (nShow < 0) {
			return;
		}

		DEBUG_PRINTF("nShow %d", nShow);

		ShowFile::Get()->SetPlayerShowFileCurrent(static_cast<uint32_t>(nShow));
		SendStatus();

		return;
	}

	// TouchOSC
	if (memcmp(&m_pBuffer[showfileosc::PATH_LENGTH], cmd::RELOAD, length::RELOAD) == 0) {
		ShowFiles();
		DEBUG_PUTS("Reload");
		return;
	}
}

// TouchOSC
void ShowFileOSC::SendStatus() {
	OscSimpleSend MsgName(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/name", "s", ShowFile::Get()->GetShowFileNameCurrent());

	const auto status = ShowFile::Get()->GetStatus();
	assert(status != showfile::Status::UNDEFINED);

	OscSimpleSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/status", "s", showfile::STATUS[static_cast<int>(status)]);
}

// TouchOSC
void ShowFileOSC::ShowFiles() {
	char aPath[64];
	char aValue[4];

	uint32_t i;

    for (i = 0; i < ShowFile::Get()->GetShows(); i++) {
    	snprintf(aPath, sizeof(aPath) - 1, "/showfile/%u/show", static_cast<unsigned int>(i));
    	snprintf(aValue, sizeof(aValue) - 1, "%.2u", static_cast<unsigned int>(ShowFile::Get()->GetPlayerShowFile(i)));
    	OscSimpleSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, aPath, "s", aValue);
    }

	for (; i < showfileosc::MAX_FILES_ENTRIES; i++) {
		snprintf(aPath, sizeof(aPath) - 1, "/showfile/%u/show", static_cast<unsigned int>(i));
		OscSimpleSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, aPath, "s", "  ");
	}

	SendStatus();
}
