#if defined (HAVE_SPI)
/**
 * @file rdmsubdevicemcp4902.cpp
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
#include <assert.h>

/*
 * MCP4902: Dual 8-Bit Voltage Output DAC
 */

#include "rdmsubdevicemcp4902.h"

#include "mcp4902.h"

#define DMX_FOOTPRINT	2

static RDMPersonality *s_RDMPersonalities[] = {new RDMPersonality("Analog output 2-lines", DMX_FOOTPRINT)};

RDMSubDeviceMCP4902::RDMSubDeviceMCP4902(uint16_t nDmxStartAddress, char nChipSselect, uint8_t nSlaveAddress, uint32_t nSpiSpeed) :
	RDMSubDevice("mcp4902",nDmxStartAddress),
	m_nDataA(0),
	m_nDataB(0)
{
	m_tDeviceInfo.chip_select = (spi_cs_t) nChipSselect;
	m_tDeviceInfo.slave_address = nSlaveAddress;
	m_tDeviceInfo.speed_hz = nSpiSpeed;

	SetDmxFootprint(DMX_FOOTPRINT);
	SetPersonalities(s_RDMPersonalities, 1);
}

RDMSubDeviceMCP4902::~RDMSubDeviceMCP4902(void) {
}

bool RDMSubDeviceMCP4902::Initialize(void) {
	const bool IsConnected = mcp4902_start(&m_tDeviceInfo);

#ifndef NDEBUG
	printf("%s:%s IsConnected=%d\n", __FILE__, __FUNCTION__, (int) IsConnected);
#endif

	return IsConnected;
}

void RDMSubDeviceMCP4902::Start(void) {
}

void RDMSubDeviceMCP4902::Stop(void) {
	mcp4902_write_ab(&m_tDeviceInfo, (uint8_t) 0, (uint8_t) 0);
	m_nDataA = (uint8_t) 0;
	m_nDataB = (uint8_t) 0;
}

void RDMSubDeviceMCP4902::Data(const uint8_t* pData, uint16_t nLength) {
	assert(nLength <= 512);

	uint16_t nOffset = GetDmxStartAddress() - 1;

	if (nOffset < nLength) {
		const uint8_t nDataA = pData[nOffset];

		if (nDataA != m_nDataA) {
			mcp4902_write_a(&m_tDeviceInfo, nDataA);
			m_nDataA = nDataA;
		}

		nOffset++;

		if (nOffset < nLength) {
			const uint8_t nDataB = pData[nOffset];

			if (nDataB != m_nDataB) {
				mcp4902_write_b(&m_tDeviceInfo, nDataB);
				m_nDataB = nDataB;
			}
		}
	}
}

void RDMSubDeviceMCP4902::UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) {
	if (tUpdateEvent == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
		Stop();
	}
}
#endif
