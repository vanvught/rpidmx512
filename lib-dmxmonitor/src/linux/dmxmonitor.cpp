/**
 * @file dmxmonitor.cpp
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "dmxmonitor.h"

enum {
	DMX_DEFAULT_MAX_CHANNELS = 32,
	DMX_DEFAULT_START_ADDRESS = 1
};

DMXMonitor::DMXMonitor(void) :
	m_bIsStarted(false),
	m_nSlots(0),
	m_nDmxStartAddress(DMX_DEFAULT_START_ADDRESS),
	m_nMaxChannels(DMX_DEFAULT_MAX_CHANNELS)

{
	for (int i = 0; i < (int) (sizeof(m_Data) / sizeof(m_Data[0])); i++) {
		m_Data[i] = 0;
	}
}

DMXMonitor::~DMXMonitor(void) {
	this->Stop();
}

void DMXMonitor::DisplayDateTime(uint8_t nPort, const char *pString) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm tm = *localtime(&tv.tv_sec);
#if defined (__APPLE__)
	printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d.%.6d %s:%d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec, pString, nPort);
#else
	printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d.%.6ld %s:%d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec, pString, nPort);
#endif
}

void DMXMonitor::SetMaxDmxChannels(uint16_t nMaxChannels) {
	m_nMaxChannels = nMaxChannels;
}

uint16_t DMXMonitor::GetDmxFootprint(void) {
	return m_nMaxChannels;
}

bool DMXMonitor::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	if (nDmxStartAddress  > (512 - m_nMaxChannels)) {
		return false;
	}

	m_nDmxStartAddress = nDmxStartAddress;
	return true;
}

uint16_t DMXMonitor::GetDmxStartAddress(void) {
	return m_nDmxStartAddress;
}

void DMXMonitor::Start(uint8_t nPort) {
	if(m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;
	DisplayDateTime(nPort, "Start");
}

void DMXMonitor::Stop(uint8_t nPort) {
	if(!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;
	DisplayDateTime(nPort, "Stop");
}

void DMXMonitor::SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLength) {
	struct timeval tv;
	uint16_t i, j;

	gettimeofday(&tv, NULL);
	struct tm tm = *localtime(&tv.tv_sec);

#if defined (__APPLE__)
	printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d.%.6d DMX:%d %d:%d:%d ", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec, (int) nPort, (int) nLength, (int) m_nMaxChannels, (int) m_nDmxStartAddress);
#else
	printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d.%.6ld DMX:%d %d:%d:%d ", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec, (int) nPort, (int) nLength, (int) m_nMaxChannels, (int) m_nDmxStartAddress);
#endif

	for (i = m_nDmxStartAddress - 1, j = 0; (i < nLength) && (j < m_nMaxChannels); i++, j++) {
		printf("%.2x ", pData[i]);
	}

	for (; j < m_nMaxChannels; j++) {
		printf("-- ");
	}
	printf("\n");
}

