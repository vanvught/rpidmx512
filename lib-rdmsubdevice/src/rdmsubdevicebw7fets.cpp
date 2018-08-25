#if defined (HAVE_SPI)
/**
 * @file rdmsubdevicebw7fets.cpp
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

#include "rdmsubdevicebw7fets.h"

#include "bw_spi_7fets.h"

#define DMX_FOOTPRINT	7

static RDMPersonality *s_RDMPersonalities[] = {new RDMPersonality("Digital output 7-lines", DMX_FOOTPRINT)};

RDMSubDeviceBw7fets::RDMSubDeviceBw7fets(uint16_t nDmxStartAddress, char nChipSselect, uint8_t nSlaveAddress, uint32_t nSpiSpeed) :
	RDMSubDevice("bw_spi_7fets", nDmxStartAddress),
	m_nData(0)
{
	m_tDeviceInfo.chip_select = (spi_cs_t) nChipSselect;
	m_tDeviceInfo.slave_address = nSlaveAddress;
	m_tDeviceInfo.speed_hz = nSpiSpeed;

	SetDmxFootprint(DMX_FOOTPRINT);
	SetPersonalities(s_RDMPersonalities, 1);
}

RDMSubDeviceBw7fets::~RDMSubDeviceBw7fets(void) {
}

bool RDMSubDeviceBw7fets::Initialize(void) {
	const bool IsConnected = bw_spi_7fets_start(&m_tDeviceInfo);

	if (IsConnected) {
		bw_spi_7fets_output(&m_tDeviceInfo, (uint8_t) 0);
	}

#ifndef NDEBUG
	printf("%s:%s IsConnected=%d\n", __FILE__, __FUNCTION__, (int) IsConnected);
#endif

	return IsConnected;
}

void RDMSubDeviceBw7fets::Start(void) {
}

void RDMSubDeviceBw7fets::Stop(void) {
	bw_spi_7fets_output(&m_tDeviceInfo, (uint8_t) 0);
	m_nData = 0;
}

void RDMSubDeviceBw7fets::Data(const uint8_t* pData, uint16_t nLength) {
	uint8_t nData = 0;
	const uint16_t nDmxStartAddress = GetDmxStartAddress();

	for (uint16_t i = nDmxStartAddress - 1, j = 0; (i < nLength) && (j < DMX_FOOTPRINT); i++, j++) {
		if ((pData[i] & (uint8_t) 0x80) != 0) {	// 0-127 is off, 128-255 is on
			nData = nData | (uint8_t) (1 << j);
		}
	}

	if (m_nData != nData) {
		bw_spi_7fets_output(&m_tDeviceInfo, nData);
		m_nData = nData;
	}
}

void RDMSubDeviceBw7fets::UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) {
	if (tUpdateEvent == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
		Stop();
	}
}
#endif
