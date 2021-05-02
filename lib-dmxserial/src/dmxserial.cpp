/**
 * @file dmxserial.cpp
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
#include <unistd.h>
#include <cassert>

#include "dmxserial.h"
#include "dmxserial_internal.h"
#include "dmxserialtftp.h"

#include "lightset.h"

#include "network.h"

#include "debug.h"

DmxSerial *DmxSerial::s_pThis = nullptr;

DmxSerial::DmxSerial() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	for (uint32_t i = 0; i < DmxSerialFile::MAX_NUMBER; i++) {
		m_aFileIndex[i] = -1;
		m_pDmxSerialChannelData[i] = nullptr;
	}

	memset(m_DmxData, 0, sizeof(m_DmxData));
}

DmxSerial::~DmxSerial() {
	for (uint32_t i = 0; i < m_nFilesCount; i++) {
		if (m_pDmxSerialChannelData[i] != nullptr) {
			delete m_pDmxSerialChannelData[i];
		}
	}

	Network::Get()->End(UDP::PORT);

	s_pThis = nullptr;
}

void DmxSerial::Init() {
	// UDP Request
	m_nHandle = Network::Get()->Begin(UDP::PORT);
	assert(m_nHandle != -1);

	ScanDirectory();
	m_Serial.Init();
}

void DmxSerial::Start(__attribute__((unused)) uint8_t nPort) {
	// No actions here
}

void DmxSerial::Stop(__attribute__((unused)) uint8_t nPort) {
	// No actions here
}

void DmxSerial::SetData(__attribute__((unused)) uint8_t nPort, const uint8_t *pData, __attribute__((unused)) uint16_t nLength) {

	for (uint32_t nIndex = 0; nIndex < m_nFilesCount; nIndex++) {
		const int32_t nOffset = m_aFileIndex[nIndex] - 1;

		if (m_DmxData[nOffset] != pData[nOffset]) {
			m_DmxData[nOffset] = pData[nOffset];

//			DEBUG_PRINTF("nPort=%d, nIndex=%d, m_aFileIndex[nIndex]=%d, nOffset=%d, m_DmxData[nOffset]=%d", nPort, nIndex, m_aFileIndex[nIndex], nOffset, m_DmxData[nOffset]);

			if (m_pDmxSerialChannelData[nIndex] != nullptr) {
				uint32_t nLength;
				const uint8_t *pSerialData = m_pDmxSerialChannelData[nIndex]->GetData(m_DmxData[nOffset], nLength);

				if (nLength == 0) {
					continue;
				}

				m_Serial.Send(pSerialData, nLength);
			}
		}
	}
}

void DmxSerial::Print() {
	m_Serial.Print();

	printf("Files : %d\n", m_nFilesCount);
	printf("DMX\n");
	printf(" First channel : %d\n", m_aFileIndex[0]);
	printf(" Last channel  : %d\n", m_nDmxLastSlot);
}

void DmxSerial::ScanDirectory() {
	// We can only run this once, for now
	assert(m_pDmxSerialChannelData[0] == nullptr);

    DIR *dirp;
    struct dirent *dp;
    m_nFilesCount = 0;

    if ((dirp = opendir(".")) == nullptr) {
		perror("couldn't open '.'");

		for (uint32_t i = 0; i < DmxSerialFile::MAX_NUMBER; i++) {
			m_aFileIndex[i] = -1;
		}

		return;
	}

    do {
        if ((dp = readdir(dirp)) != nullptr) {
        	if (dp->d_type == DT_DIR) {
        		continue;
        	}

          	int16_t nFileNumber;
        	if (!CheckFileName(dp->d_name, nFileNumber)) {
                continue;
            }

        	m_aFileIndex[m_nFilesCount] = nFileNumber;

            DEBUG_PRINTF("[%d] found %s", m_nFilesCount, dp->d_name);

            m_nFilesCount++;

            if (m_nFilesCount == DmxSerialFile::MAX_NUMBER) {
            	break;
            }
        }
    } while (dp != nullptr);

    // Sort
	for (uint32_t i = 0; i < m_nFilesCount; i++) {
		for (uint32_t j = 0; j < m_nFilesCount; j++) {
			if (m_aFileIndex[j] > m_aFileIndex[i]) {
				int16_t tmp = m_aFileIndex[i];
				m_aFileIndex[i] = m_aFileIndex[j];
				m_aFileIndex[j] = tmp;
			}
		}
	}

	m_nDmxLastSlot = static_cast<uint16_t>(m_aFileIndex[m_nFilesCount - 1]);

	for (uint32_t i = m_nFilesCount; i < DmxSerialFile::MAX_NUMBER; i++) {
		m_aFileIndex[i] = -1;
	}

	static_cast<void>(closedir(dirp));

#ifndef NDEBUG
	printf("%d\n", m_nFilesCount);
#endif

	for (uint32_t nIndex = 0; nIndex < m_nFilesCount; nIndex++) {
#ifndef NDEBUG
		printf("\tnIndex=%d -> %d\n", nIndex, m_aFileIndex[nIndex]);
#endif

		m_pDmxSerialChannelData[nIndex] = new DmxSerialChannelData;
		assert(m_pDmxSerialChannelData[nIndex] != nullptr);

		char pBuffer[16];
		snprintf(pBuffer, sizeof(pBuffer) - 1, DMXSERIAL_FILE_PREFIX "%.3d" DMXSERIAL_FILE_SUFFIX, m_aFileIndex[nIndex]);
		DEBUG_PUTS(pBuffer);
		m_pDmxSerialChannelData[nIndex]->Parse(pBuffer);
	}

#ifndef NDEBUG
	for (uint32_t nIndex = 0; nIndex < m_nFilesCount; nIndex++) {
		printf("\tnIndex=%d -> %d\n", nIndex, m_aFileIndex[nIndex]);
		m_pDmxSerialChannelData[nIndex]->Dump();
	}
#endif
}

void DmxSerial::EnableTFTP(bool bEnableTFTP) {
	DEBUG_ENTRY

	if (bEnableTFTP == m_bEnableTFTP) {
		DEBUG_EXIT
		return;
	}

	DEBUG_PRINTF("bEnableTFTP=%d", bEnableTFTP);

	m_bEnableTFTP = bEnableTFTP;

	if (m_bEnableTFTP) {
		assert(m_pDmxSerialTFTP == nullptr);
		m_pDmxSerialTFTP = new DmxSerialTFTP;
		assert(m_pDmxSerialTFTP != nullptr);
	} else {
		assert(m_pDmxSerialTFTP != nullptr);
		delete m_pDmxSerialTFTP;
		m_pDmxSerialTFTP = nullptr;
	}

	DEBUG_EXIT
}

void DmxSerial::Run() {
	HandleUdp();

	if (m_pDmxSerialTFTP == nullptr) {
		return;
	}

	m_pDmxSerialTFTP->Run();
}

bool DmxSerial::DeleteFile(int16_t nFileNumber) {
	DEBUG_PRINTF("nFileNumber=%u", nFileNumber);

	char aFileName[DmxSerialFile::NAME_LENGTH + 1];

	if (FileNameCopyTo(aFileName, sizeof(aFileName), nFileNumber)) {
		const int nResult = unlink(aFileName);
		DEBUG_PRINTF("nResult=%d", nResult);
		DEBUG_EXIT
		return (nResult == 0);
	}

	DEBUG_EXIT
	return false;
}

bool DmxSerial::DeleteFile(const char *pFileNumber) {
	DEBUG_PUTS(pFileNumber);

	if (strlen(pFileNumber) != 3) {
		return false;
	}

	int16_t nFileNumber = (pFileNumber[0] - '0') * 100;
	nFileNumber += (pFileNumber[1] - '0') * 10;
	nFileNumber += (pFileNumber[2] - '0');

	if (nFileNumber > DmxSerialFile::MAX_NUMBER) {
		return false;
	}

	return DeleteFile(nFileNumber);
}
