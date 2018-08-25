#if defined (HAVE_SPI)
/**
 * @file rdmsubdevicebwdio.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif

#include "util.h"

#include "rdmsubdevicebwdio.h"

#include "bw_spi_dio.h"

#define DMX_FOOTPRINT	7

static RDMPersonality *s_RDMPersonalities[] = {new RDMPersonality("Digital output 7-lines", DMX_FOOTPRINT)};

RDMSubDeviceBwDio::RDMSubDeviceBwDio(uint16_t nDmxStartAddress, char nChipSselect, uint8_t nSlaveAddress, uint32_t nSpiSpeed) :
	RDMSubDevice("bw_spi_dio", nDmxStartAddress),
	m_nData(0)
{
	m_tDeviceInfo.chip_select = (spi_cs_t) nChipSselect;
	m_tDeviceInfo.slave_address = nSlaveAddress;
	m_tDeviceInfo.speed_hz = nSpiSpeed;

	SetDmxFootprint(DMX_FOOTPRINT);
	SetPersonalities(s_RDMPersonalities, 1);
}

RDMSubDeviceBwDio::~RDMSubDeviceBwDio(void) {
}

bool RDMSubDeviceBwDio::Initialize(void) {
	const bool IsConnected = bw_spi_dio_start(&m_tDeviceInfo);

	if (IsConnected) {
		bw_spi_dio_fsel_mask(&m_tDeviceInfo, 0x7F);
		bw_spi_dio_output(&m_tDeviceInfo, (uint8_t) 0);
	}

#ifndef NDEBUG
	printf("%s:%s IsConnected=%d\n", __FILE__, __FUNCTION__, (int) IsConnected);
#endif

	return IsConnected;
}

void RDMSubDeviceBwDio::Start(void) {
}

void RDMSubDeviceBwDio::Stop(void) {
	bw_spi_dio_output(&m_tDeviceInfo, (uint8_t) 0);
	m_nData = 0;
}

void RDMSubDeviceBwDio::Data(const uint8_t* pData, uint16_t nLength) {
	uint8_t nData = 0;
	const uint16_t nDmxStartAddress = GetDmxStartAddress();

	for (uint16_t i = nDmxStartAddress - 1, j = 0; (i < nLength) && (j < DMX_FOOTPRINT); i++, j++) {
		if ((pData[i] & (uint8_t) 0x80) != 0) {	// 0-127 is off, 128-255 is on
			nData = nData | (uint8_t) (1 << j);
		}
	}

#ifndef NDEBUG
	printf("%s:%s m_nData=%x, nData=%x\n", __FILE__, __FUNCTION__, m_nData, nData);
#endif

	if (m_nData != nData) {
		bw_spi_dio_output(&m_tDeviceInfo, nData);
		m_nData = nData;
	}
}

void RDMSubDeviceBwDio::UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) {
	if (tUpdateEvent == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
		Stop();
	}
}
#endif
