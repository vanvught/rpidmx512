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

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#define OSCSERVER_MAX_BUFFER 		1024

static const char sPath[] ALIGNED = "/showfile/";
#define PATH_LENGTH 			(sizeof(sPath)/sizeof(sPath[0]) - 1)

static const char sStart[] ALIGNED = "start";
#define START_LENGTH 			(sizeof(sStart)/sizeof(sStart[0]) - 1)

static const char sStop[] ALIGNED = "stop";
#define STOP_LENGTH 			(sizeof(sStop)/sizeof(sStop[0]) - 1)

static const char sResume[] ALIGNED = "resume";
#define RESUME_LENGTH 			(sizeof(sResume)/sizeof(sResume[0]) - 1)

static const char sShow[] ALIGNED = "show";
#define SHOW_LENGTH 			(sizeof(sShow)/sizeof(sShow[0]) - 1)

static const char sLoop[] ALIGNED = "loop";
#define LOOP_LENGTH 			(sizeof(sLoop)/sizeof(sLoop[0]) - 1)

static const char sBlackout[] ALIGNED = "blackout";
#define BO_LENGTH 				(sizeof(sBlackout)/sizeof(sBlackout[0]) - 1)

static const char sTftp[] ALIGNED = "tftp";
#define TFTP_LENGTH 			(sizeof(sTftp)/sizeof(sTftp[0]) - 1)

static const char sDelete[] ALIGNED = "delete";
#define DELETE_LENGTH 			(sizeof(sDelete)/sizeof(sDelete[0]) - 1)

static const char sReload[] ALIGNED = "reload";
#define RELOAD_LENGTH 			(sizeof(sReload)/sizeof(sReload[0]) - 1)

static const char sIndex[] ALIGNED = "index";
#define INDEX_LENGTH 			(sizeof(sIndex)/sizeof(sIndex[0]) - 1)

ShowFileOSC *ShowFileOSC::s_pThis = 0;

ShowFileOSC::ShowFileOSC(void):
	m_nPortIncoming(OSCSERVER_PORT_DEFAULT_INCOMING),
	m_nPortOutgoing(OSCSERVER_PORT_DEFAULT_OUTGOING),
	m_nHandle(-1),
	m_nRemoteIp(0),
	m_nRemotePort(0)
{
	DEBUG_ENTRY

	s_pThis = this;

	m_pBuffer = new uint8_t[OSCSERVER_MAX_BUFFER];
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
	const int nBytesReceived = Network::Get()->RecvFrom(m_nHandle, m_pBuffer, OSCSERVER_MAX_BUFFER, &m_nRemoteIp, &m_nRemotePort);

	if (__builtin_expect((nBytesReceived <= (int) PATH_LENGTH), 1)) {
		return;
	}

	if (memcmp((const char *) m_pBuffer, sPath, PATH_LENGTH) == 0) {
		DEBUG_PRINTF("[%s] %d,%d %s", m_pBuffer, (int) strlen((const char *)m_pBuffer), PATH_LENGTH, &m_pBuffer[PATH_LENGTH]);

		if (memcmp(&m_pBuffer[PATH_LENGTH], sStart, START_LENGTH) == 0) {
			ShowFile::Get()->Start();
			SendStatus();
			DEBUG_PUTS("ActionStart");
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], sStop, STOP_LENGTH) == 0){
			ShowFile::Get()->Stop();
			SendStatus();
			DEBUG_PUTS("ActionStop");
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], sResume, RESUME_LENGTH) == 0) {
			ShowFile::Get()->Resume();
			SendStatus();
			DEBUG_PUTS("ActionResume");
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], sShow, SHOW_LENGTH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			const int nValue = Msg.GetInt(0);

			if (nValue <= SHOWFILE_FILE_MAX_NUMBER) {
				ShowFile::Get()->SetShowFile((uint8_t) nValue);
				SendStatus();
			}

			DEBUG_PRINTF("Show %d", nValue);
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], sLoop, LOOP_LENGTH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			const int nValue = Msg.GetInt(0);

			ShowFile::Get()->DoLoop(nValue != 0);
			SendStatus();

			DEBUG_PRINTF("Loop %d", nValue != 0);
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], sBlackout, BO_LENGTH) == 0) {
			ShowFile::Get()->BlackOut();
			SendStatus();
			DEBUG_PUTS("Blackout");
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], sTftp, TFTP_LENGTH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			const int nValue = Msg.GetInt(0);

			ShowFile::Get()->EnableTFTP(nValue != 0);
			SendStatus();

			DEBUG_PRINTF("TFTP %d", nValue != 0);
			return;
		}


		if (memcmp(&m_pBuffer[PATH_LENGTH], sDelete, DELETE_LENGTH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			int nValue = 255;

			if (Msg.GetType(0) == OSC_INT32){
				nValue = Msg.GetInt(0);
			} else {
				return;
			}

			DEBUG_PRINTF("nValue=%u", (unsigned) nValue);

			if (nValue <= SHOWFILE_FILE_MAX_NUMBER) {
				char aShowFileName[SHOWFILE_FILE_NAME_LENGTH + 1];

				ShowFile::ShowFileNameCopyTo((char *)aShowFileName, sizeof(aShowFileName), nValue);

				OSCSend MsgName(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/name", "s", aShowFileName);

				if (ShowFile::Get()->DeleteShowFile((uint8_t) nValue)) {
					OSCSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/status", "s", "Deleted");
				} else {
					OSCSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/status", "s", "Not deleted");
				}
			}

			DEBUG_PRINTF("Delete %d", nValue);
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], sReload, RELOAD_LENGTH) == 0) {

			Reload();

			DEBUG_PUTS("Reload");
			return;
		}

		if (memcmp(&m_pBuffer[PATH_LENGTH], sIndex, INDEX_LENGTH) == 0) {
			OSCMessage Msg(m_pBuffer, nBytesReceived);

			if (Msg.GetType(0) != OSC_FLOAT){
				return;
			}

			const uint32_t nIndex =  (uint32_t) Msg.GetFloat(0);

			DEBUG_PRINTF("Index %u", nIndex);

			if (nIndex >= OSCSERVER_FILES_ENTRIES_MAX) {
				return;
			}

			int32_t nShow = m_aFileIndex[nIndex];

			if (nShow < 0) {
				return;
			}

			if (nShow <= SHOWFILE_FILE_MAX_NUMBER) {
				ShowFile::Get()->SetShowFile((uint8_t) nShow);
				SendStatus();
			}

			return;
		}

	}
}

void ShowFileOSC::SendStatus(void) {
	OSCSend MsgName(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/name", "s", ShowFile::Get()->GetShowFileName());

	const enum TShowFileStatus tStatus = ShowFile::Get()->GetStatus();
	assert(tStatus != SHOWFILE_STATUS_UNDEFINED);

	OSCSend MsgStatus(m_nHandle, m_nRemoteIp, m_nPortOutgoing, "/showfile/status", "s", ShowFileConst::STATUS[tStatus]);
}

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

	(void) closedir(dirp);

	SendStatus();
}

void ShowFileOSC::Print(void) {
	printf("OSC Server\n");
	printf(" Path : [%s]\n", sPath);
	printf(" Incoming port : %u\n", m_nPortIncoming);
	printf(" Outgoing port : %u\n", m_nPortOutgoing);
}
