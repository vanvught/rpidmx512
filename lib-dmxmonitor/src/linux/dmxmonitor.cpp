/**
 * @file dmxmonitor.cpp
 *
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdio>
#include <time.h>
#include <sys/time.h>
#include <cassert>

#include "dmxmonitor.h"
#include "dmxmonitorstore.h"

#include "debug.h"

using namespace dmxmonitor;

DMXMonitor::DMXMonitor() {
	for (uint32_t nPortIndex = 0; nPortIndex < dmxmonitor::output::text::MAX_PORTS; nPortIndex++) {
		memset(&m_Data[nPortIndex], 0, sizeof(struct Data));
	}

	for (uint32_t i = 0; i < sizeof(m_bIsStarted); i++) {
		m_bIsStarted[i] = false;
	}
}

bool DMXMonitor::SetDmxStartAddress(uint16_t nDmxStartAddress)  {
	if (nDmxStartAddress > (512 - m_nMaxChannels)) {
		return false;
	}

	DmxMonitorStore::SaveDmxStartAddress(nDmxStartAddress);

	m_nDmxStartAddress = nDmxStartAddress;
	return true;
}

void DMXMonitor::DisplayDateTime(const uint32_t nPortIndex, const char *pString) {
	assert(nPortIndex < output::text::MAX_PORTS);

	struct timeval tv;
	gettimeofday(&tv, nullptr);
	auto *tm = localtime(&tv.tv_sec);

	printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d.%.6d %s:%c\n",
			tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec,
			static_cast<int>(tv.tv_usec),
			pString,
			nPortIndex + 'A');
}

void DMXMonitor::Start(const uint32_t nPortIndex) {
	assert(nPortIndex < output::text::MAX_PORTS);

	if (m_bIsStarted[nPortIndex]) {
		return;
	}

	m_bIsStarted[nPortIndex] = true;
	DisplayDateTime(nPortIndex, "Start");
}

void DMXMonitor::Stop(const uint32_t nPortIndex) {
	assert(nPortIndex < output::text::MAX_PORTS);

	if (!m_bIsStarted[nPortIndex]) {
		return;
	}

	m_bIsStarted[nPortIndex] = false;
	DisplayDateTime(nPortIndex, "Stop");
}

void DMXMonitor::SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate) {
	assert(nPortIndex < output::text::MAX_PORTS);

	if (doUpdate) {
		Update(nPortIndex, pData, nLength);
	} else {
		memcpy(m_Data[nPortIndex].data, pData, nLength);
		m_Data[nPortIndex].nLength = nLength;
	}
}

void DMXMonitor::Update(const uint32_t nPortIndex, const uint8_t *pData, const uint32_t nLength) {
	assert(nPortIndex < output::text::MAX_PORTS);

	struct timeval tv;
	uint32_t i, j;

	gettimeofday(&tv, nullptr);
	auto *tm = localtime(&tv.tv_sec);

	printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d.%.6d DMX:%c %d:%d:%d ",
			tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec,
			static_cast<int>(tv.tv_usec),
			nPortIndex + 'A',
			static_cast<int>(nLength),
			static_cast<int>(m_nMaxChannels),
			static_cast<int>(m_nDmxStartAddress));

	for (i = static_cast<uint32_t>(m_nDmxStartAddress - 1), j = 0; (i < nLength) && (j < m_nMaxChannels); i++, j++) {
		switch (m_Format) {
		case Format::PCT:
			printf("%3d ", ((pData[i] * 100)) / 255);
			break;
		case Format::DEC:
			printf("%3d ", pData[i]);
			break;
		default:
			printf("%.2x ", pData[i]);
			break;
		}
	}

	for (; j < m_nMaxChannels; j++) {
		printf("-- ");
	}

	puts("");
}
