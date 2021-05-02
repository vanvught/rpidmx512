/**
 * @file showfileosc.cpp
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
#include <stdio.h>
#include <dirent.h>
#include <cassert>

#include "showfileosc.h"
#include "showfileconst.h"
#include "showfile.h"

#include "oscsimplemessage.h"
#include "oscsimplesend.h"

#include "network.h"

#include "showfile.h"

#include "debug.h"

static constexpr auto MAX_BUFFER_SIZE = ShowFileOSCMax::CMD_LENGTH;

namespace cmd {
	static constexpr char PATH[] = "/showfile/";
	static constexpr char START[] = "start";
	static constexpr char STOP[] = "stop";
	static constexpr char RESUME[] = "resume";
	static constexpr char SHOW[] = "show";
	static constexpr char LOOP[] = "loop";
	static constexpr char BO[] = "blackout";
	static constexpr char MASTER[] = "master";
	static constexpr char TFTP[] = "tftp";
	static constexpr char DELETE[] = "delete";
	// TouchOSC specific
	static constexpr char RELOAD[] = "reload";
	static constexpr char INDEX[] = "index";
}

namespace length {
	static constexpr auto PATH = sizeof(cmd::PATH) - 1;
	static constexpr auto START = sizeof(cmd::START) - 1;
	static constexpr auto STOP = sizeof(cmd::STOP) - 1;
	static constexpr auto RESUME = sizeof(cmd::RESUME) - 1;
	static constexpr auto SHOW = sizeof(cmd::SHOW) - 1;
	static constexpr auto LOOP = sizeof(cmd::LOOP) - 1;
	static constexpr auto BO = sizeof(cmd::BO) - 1;
	static constexpr auto MASTER = sizeof(cmd::MASTER) - 1;
	static constexpr auto TFTP = sizeof(cmd::TFTP) - 1;
	static constexpr auto DELETE = sizeof(cmd::DELETE) - 1;
	// TouchOSC specific
	static constexpr auto RELOAD = sizeof(cmd::RELOAD) - 1;
	static constexpr auto INDEX = sizeof(cmd::INDEX) - 1;
}

ShowFileOSC *ShowFileOSC::s_pThis = nullptr;

ShowFileOSC::ShowFileOSC() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	m_pBuffer = new char[MAX_BUFFER_SIZE];
	assert(m_pBuffer != nullptr);

	for (uint32_t i = 0; i < ShowFileOSCMax::FILES_ENTRIES; i++) {
		m_aFileIndex[i] = -1;
	}

	DEBUG_EXIT
}

ShowFileOSC::~ShowFileOSC() {
	delete[] m_pBuffer;
	m_pBuffer = nullptr;
}

void ShowFileOSC::Start() {
	DEBUG_ENTRY

	m_nHandle = Network::Get()->Begin(m_nPortIncoming);
	assert(m_nHandle != -1);

	DEBUG_EXIT
}

void ShowFileOSC::Stop() {
	DEBUG_ENTRY

	//TODO Implement ShowFileOSC::Stop

	DEBUG_EXIT
}

void ShowFileOSC::Run() {
	const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, m_pBuffer, MAX_BUFFER_SIZE, &m_nRemoteIp, &m_nRemotePort);

	if (__builtin_expect((nBytesReceived <= length::PATH), 1)) {
		return;
	}

	if (memcmp(m_pBuffer, cmd::PATH, length::PATH) == 0) {
		DEBUG_PRINTF("[%s] %d,%d %s", m_pBuffer, static_cast<int>(strlen(m_pBuffer)), static_cast<int>(length::PATH), &m_pBuffer[length::PATH]);

		if (memcmp(&m_pBuffer[length::PATH], cmd::START, length::START) == 0) {
			ShowFile::Get()->Start();
			SendStatus();
			DEBUG_PUTS("ActionStart");
			return;
		}

		if (memcmp(&m_pBuffer[length::PATH], cmd::STOP, length::STOP) == 0){
			ShowFile::Get()->Stop();
			SendStatus();
			DEBUG_PUTS("ActionStop");
			return;
		}

		if (memcmp(&m_pBuffer[length::PATH], cmd::RESUME, length::RESUME) == 0) {
			ShowFile::Get()->Resume();
			SendStatus();
			DEBUG_PUTS("ActionResume");
			return;
		}

		if (memcmp(&m_pBuffer[length::PATH], cmd::SHOW, length::SHOW) == 0) {
			OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

			const int nValue = Msg.GetInt(0);

			if (nValue <= ShowFileFile::MAX_NUMBER) {
				ShowFile::Get()->SetShowFile(nValue);
				SendStatus();
			}

			DEBUG_PRINTF("Show %d", nValue);
			return;
		}

		if (memcmp(&m_pBuffer[length::PATH], cmd::LOOP, length::LOOP) == 0) {
			OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

			int nValue;

			if (Msg.GetType(0) == osc::type::INT32) {
				nValue = Msg.GetInt(0);
			} else if (Msg.GetType(0) == osc::type::FLOAT) { // TouchOSC
				nValue = Msg.GetFloat(0);
			} else {
				return;
			}

			ShowFile::Get()->DoLoop(nValue != 0);
			SendStatus();
			ShowFile::Get()->UpdateDisplayStatus();

			DEBUG_PRINTF("Loop %d", nValue != 0);
			return;
		}

		if (memcmp(&m_pBuffer[length::PATH], cmd::BO, length::BO) == 0) {
			ShowFile::Get()->BlackOut();
			SendStatus();
			DEBUG_PUTS("Blackout");
			return;
		}

		if (memcmp(&m_pBuffer[length::PATH], cmd::MASTER, length::MASTER) == 0) {
			OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

			int nValue;

			if (Msg.GetType(0) == osc::type::INT32) {
				nValue = Msg.GetInt(0);
			} else if (Msg.GetType(0) == osc::type::FLOAT) { // TouchOSC
				nValue = Msg.GetFloat(0);
			} else {
				return;
			}

			if ((nValue >= 0) &&  (nValue <= 255)) {
				ShowFile::Get()->SetMaster(static_cast<uint32_t>(nValue));
			}

			DEBUG_PRINTF("Master %d", nValue);
			return;
		}

		if (memcmp(&m_pBuffer[length::PATH], cmd::TFTP, length::TFTP) == 0) {
			OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

			int nValue;

			if (Msg.GetType(0) == osc::type::INT32) {
				nValue = Msg.GetInt(0);
			} else if (Msg.GetType(0) == osc::type::FLOAT) { // TouchOSC
				nValue = Msg.GetFloat(0);
			} else {
				return;
			}

			ShowFile::Get()->EnableTFTP(nValue != 0);
			SendStatus();

			DEBUG_PRINTF("TFTP %d", nValue != 0);
			return;
		}


		if (memcmp(&m_pBuffer[length::PATH], cmd::DELETE, length::DELETE) == 0) {
			OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

			int nValue = 255;

			if (Msg.GetType(0) == osc::type::INT32){
				nValue = Msg.GetInt(0);
			} else {
				return;
			}

			DEBUG_PRINTF("nValue=%d", nValue);

			if (nValue <= ShowFileFile::MAX_NUMBER) {
				char aShowFileName[ShowFileFile::NAME_LENGTH + 1];

				ShowFile::ShowFileNameCopyTo(aShowFileName, sizeof(aShowFileName), nValue);

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

		// TouchOSC
		if (memcmp(&m_pBuffer[length::PATH], cmd::RELOAD, length::RELOAD) == 0) {

			Reload();

			DEBUG_PUTS("Reload");
			return;
		}

		// TouchOSC
		if (memcmp(&m_pBuffer[length::PATH], cmd::INDEX, length::INDEX) == 0) {
			OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

			if (Msg.GetType(0) != osc::type::FLOAT){
				return;
			}

			const auto nIndex = static_cast<uint32_t>(Msg.GetFloat(0));

			DEBUG_PRINTF("Index %u", nIndex);

			if (nIndex >= ShowFileOSCMax::FILES_ENTRIES) {
				return;
			}

			auto nShow = m_aFileIndex[nIndex];

			if (nShow < 0) {
				return;
			}

			if (nShow <= ShowFileFile::MAX_NUMBER) {
				ShowFile::Get()->SetShowFile(nShow);
				SendStatus();
			}

			return;
		}

	}
}

// TouchOSC
void ShowFileOSC::SendStatus() {
	OscSimpleSend MsgName(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/name", "s", ShowFile::Get()->GetShowFileName());

	const auto tStatus = ShowFile::Get()->GetStatus();
	assert(tStatus != ShowFileStatus::UNDEFINED);

	OscSimpleSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/status", "s", ShowFileConst::STATUS[static_cast<int>(tStatus)]);
}

// TouchOSC
void ShowFileOSC::Reload() {
    DIR *dirp;
    struct dirent *dp;
    uint32_t nIndex = 0;


    if ((dirp = opendir(".")) == nullptr) {
		perror("couldn't open '.'");

		for (uint32_t i = 0; i < ShowFileOSCMax::FILES_ENTRIES; i++) {
			m_aFileIndex[i] = -1;
		}

		return;
	}

    do {
        if ((dp = readdir(dirp)) != nullptr) {
        	if (dp->d_type == DT_DIR) {
        		continue;
        	}

          	uint8_t nShowFileNumber;
        	if (!ShowFile::CheckShowFileName(dp->d_name, nShowFileNumber)) {
                continue;
            }

        	m_aFileIndex[nIndex] = nShowFileNumber;

            DEBUG_PRINTF("[%d] found %s", nIndex, dp->d_name);

            nIndex++;

            if (nIndex == ShowFileOSCMax::FILES_ENTRIES) {
            	break;
            }
        }
    } while (dp != nullptr);

    // Sort
	for (uint32_t i = 0; i < nIndex; i++) {
		for (uint32_t j = 0; j < nIndex; j++) {
			if (m_aFileIndex[j] > m_aFileIndex[i]) {
				int32_t tmp = m_aFileIndex[i];
				m_aFileIndex[i] = m_aFileIndex[j];
				m_aFileIndex[j] = tmp;
			}
		}
	}

	char aPath[64];
	char aValue[4];
	uint32_t i;

    for (i = 0; i < nIndex; i++) {
    	snprintf(aPath, sizeof(aPath) - 1, "/showfile/%d/show", i);
    	snprintf(aValue, sizeof(aValue) - 1, "%.2d", m_aFileIndex[i]);

    	OscSimpleSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, aPath, "s", aValue);
    	DEBUG_PRINTF("%s s %s", aPath, aValue);
    }

	for (; i < ShowFileOSCMax::FILES_ENTRIES; i++) {
		snprintf(aPath, sizeof(aPath) - 1, "/showfile/%d/show", i);
		OscSimpleSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, aPath, "s", "  ");
		m_aFileIndex[i] = -1;
	}

	closedir(dirp);

	SendStatus();
}

void ShowFileOSC::Print() {
	printf("OSC Server\n");
	printf(" Path : [%s]\n", cmd::PATH);
	printf(" Incoming port : %u\n", m_nPortIncoming);
	printf(" Outgoing port : %u\n", m_nPortOutgoing);
}
