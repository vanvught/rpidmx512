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
#include <assert.h>

#include "showfileosc.h"
#include "showfileconst.h"
#include "showfile.h"

#include "oscmessage.h"
#include "oscsend.h"

#include "network.h"

#include "showfile.h"

#include "debug.h"

enum {
	OSCSERVER_MAX_BUFFER = 1024
};

constexpr char aPath[] = "/showfile/";
#define PATH_LENGTH	 	(sizeof(aPath) - 1)

constexpr char aStart[] = "start";
#define START_LENGTH 	(sizeof(aStart) - 1)

constexpr char aStop[] = "stop";
#define STOP_LENGTH 	(sizeof(aStop) - 1)

constexpr char aResume[] = "resume";
#define RESUME_LENGTH 	(sizeof(aResume) - 1)

constexpr char aShow[] = "show";
#define SHOW_LENGTH 	(sizeof(aShow) - 1)

constexpr char aLoop[] = "loop";
#define LOOP_LENGTH 	(sizeof(aLoop) - 1)

constexpr char aBlackout[] = "blackout";
#define BO_LENGTH 		(sizeof(aBlackout) - 1)

constexpr char aMaster[] = "master";
#define MASTER_LENGTH 	(sizeof(aMaster) - 1)

constexpr char aTftp[] = "tftp";
#define TFTP_LENGTH 	(sizeof(aTftp) - 1)

constexpr char aDelete[] = "delete";
#define DELETE_LENGTH 	(sizeof(aDelete) - 1)

// TouchOSC
constexpr char aReload[] = "reload";
#define RELOAD_LENGTH 	(sizeof(aReload) - 1)
// TouchOSC
constexpr char aIndex[] = "index";
#define INDEX_LENGTH 	(sizeof(aIndex) - 1)

ShowFileOSC *ShowFileOSC::s_pThis = 0;

ShowFileOSC::ShowFileOSC(void):
	m_nPortIncoming(OSCSERVER_PORT_DEFAULT_INCOMING),
	m_nPortOutgoing(OSCSERVER_PORT_DEFAULT_OUTGOING),
	m_nHandle(-1),
	m_nRemoteIp(0),
	m_nRemotePort(0)
{
	DEBUG_ENTRY

	assert(s_pThis == 0);
	s_pThis = this;

	m_pBuffer = new char[OSCSERVER_MAX_BUFFER];
	assert(m_pBuffer != 0);

	for (uint32_t i = 0; i < OSCSERVER_FILES_ENTRIES_MAX; i++) {
		m_aFileIndex[i] = -1;
	}

	DEBUG_EXIT
}

ShowFileOSC::~ShowFileOSC(void) {
	delete[] m_pBuffer;
	m_pBuffer = 0;
}

void ShowFileOSC::Start(void) {
	DEBUG_ENTRY

	m_nHandle = Network::Get()->Begin(m_nPortIncoming);
	assert(m_nHandle != -1);

	DEBUG_EXIT
}

void ShowFileOSC::Stop(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void ShowFileOSC::Run(void) {
	const uint16_t nBytesReceived = Network::Get()->RecvFrom(m_nHandle, m_pBuffer, OSCSERVER_MAX_BUFFER, &m_nRemoteIp, &m_nRemotePort);

	if (__builtin_expect((nBytesReceived <= PATH_LENGTH), 1)) {
		return;
	}

	if (memcmp(m_pBuffer, aPath, PATH_LENGTH) == 0) {
		DEBUG_PRINTF("[%s] %d,%d %s", m_pBuffer, static_cast<int>(strlen(m_pBuffer)), static_cast<int>(PATH_LENGTH), &m_pBuffer[PATH_LENGTH]);

		if (memcmp(&m_pBuffer[PATH_LENGTH], aStart, START_LENGTH) == 0) {
			ShowFile::Get()->Start();
			SendStatus();
			DEBUG_PUTS("ActionStart");
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], aStop, STOP_LENGTH) == 0){
			ShowFile::Get()->Stop();
			SendStatus();
			DEBUG_PUTS("ActionStop");
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], aResume, RESUME_LENGTH) == 0) {
			ShowFile::Get()->Resume();
			SendStatus();
			DEBUG_PUTS("ActionResume");
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], aShow, SHOW_LENGTH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			const int nValue = Msg.GetInt(0);

			if (nValue <= SHOWFILE_FILE_MAX_NUMBER) {
				ShowFile::Get()->SetShowFile(nValue);
				SendStatus();
			}

			DEBUG_PRINTF("Show %d", nValue);
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], aLoop, LOOP_LENGTH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			const int nValue = Msg.GetInt(0);

			ShowFile::Get()->DoLoop(nValue != 0);
			SendStatus();
			ShowFile::Get()->UpdateDisplayStatus();

			DEBUG_PRINTF("Loop %d", nValue != 0);
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], aBlackout, BO_LENGTH) == 0) {
			ShowFile::Get()->BlackOut();
			SendStatus();
			DEBUG_PUTS("Blackout");
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], aMaster, MASTER_LENGTH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			int nValue;

			if (Msg.GetType(0) == OSC_INT32) {
				nValue = Msg.GetInt(0);
			} else if (Msg.GetType(0) == OSC_FLOAT) { // TouchOSC
				nValue = Msg.GetFloat(0);
			} else {
				return;
			}

			if ((nValue >= 0) &&  (nValue <= 255)) {
				ShowFile::Get()->SetMaster(nValue);
			}

			DEBUG_PRINTF("Master %d", nValue);
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], aTftp, TFTP_LENGTH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			const int nValue = Msg.GetInt(0);

			ShowFile::Get()->EnableTFTP(nValue != 0);
			SendStatus();

			DEBUG_PRINTF("TFTP %d", nValue != 0);
			return;
		}


		if (memcmp(&m_pBuffer[PATH_LENGTH], aDelete, DELETE_LENGTH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			int nValue = 255;

			if (Msg.GetType(0) == OSC_INT32){
				nValue = Msg.GetInt(0);
			} else {
				return;
			}

			DEBUG_PRINTF("nValue=%d", nValue);

			if (nValue <= SHOWFILE_FILE_MAX_NUMBER) {
				char aShowFileName[SHOWFILE_FILE_NAME_LENGTH + 1];

				ShowFile::ShowFileNameCopyTo(aShowFileName, sizeof(aShowFileName), nValue);

				OSCSend MsgName(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/name", "s", aShowFileName);

				if (ShowFile::Get()->DeleteShowFile(nValue)) {
					OSCSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/status", "s", "Deleted");
				} else {
					OSCSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/status", "s", "Not deleted");
				}
			}

			DEBUG_PRINTF("Delete %d", nValue);
			return;
		}

		// TouchOSC
		if (memcmp(&m_pBuffer[PATH_LENGTH], aReload, RELOAD_LENGTH) == 0) {

			Reload();

			DEBUG_PUTS("Reload");
			return;
		}

		// TouchOSC
		if (memcmp(&m_pBuffer[PATH_LENGTH], aIndex, INDEX_LENGTH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			if (Msg.GetType(0) != OSC_FLOAT){
				return;
			}

			const uint32_t nIndex = static_cast<uint32_t>(Msg.GetFloat(0));

			DEBUG_PRINTF("Index %u", nIndex);

			if (nIndex >= OSCSERVER_FILES_ENTRIES_MAX) {
				return;
			}

			int32_t nShow = m_aFileIndex[nIndex];

			if (nShow < 0) {
				return;
			}

			if (nShow <= SHOWFILE_FILE_MAX_NUMBER) {
				ShowFile::Get()->SetShowFile(nShow);
				SendStatus();
			}

			return;
		}

	}
}

// TouchOSC
void ShowFileOSC::SendStatus(void) {
	OSCSend MsgName(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/name", "s", ShowFile::Get()->GetShowFileName());

	const enum TShowFileStatus tStatus = ShowFile::Get()->GetStatus();
	assert(tStatus != SHOWFILE_STATUS_UNDEFINED);

	OSCSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/status", "s", ShowFileConst::STATUS[tStatus]);
}

// TouchOSC
void ShowFileOSC::Reload(void) {
    DIR *dirp;
    struct dirent *dp;
    uint32_t nIndex = 0;


    if ((dirp = opendir(".")) == NULL) {
		perror("couldn't open '.'");

		for (uint32_t i = 0; i < OSCSERVER_FILES_ENTRIES_MAX; i++) {
			m_aFileIndex[i] = -1;
		}

		return;
	}

    do {
        if ((dp = readdir(dirp)) != NULL) {
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

            if (nIndex == OSCSERVER_FILES_ENTRIES_MAX) {
            	break;
            }
        }
    } while (dp != NULL);

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

    	OSCSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, aPath, "s", aValue);
    	DEBUG_PRINTF("%s s %s", aPath, aValue);
    }

	for (; i < OSCSERVER_FILES_ENTRIES_MAX; i++) {
		snprintf(aPath, sizeof(aPath) - 1, "/showfile/%d/show", i);
		OSCSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, aPath, "s", "  ");
		m_aFileIndex[i] = -1;
	}

	static_cast<void>(closedir(dirp));

	SendStatus();
}

void ShowFileOSC::Print(void) {
	printf("OSC Server\n");
	printf(" Path : [%s]\n", aPath);
	printf(" Incoming port : %u\n", m_nPortIncoming);
	printf(" Outgoing port : %u\n", m_nPortOutgoing);
}
