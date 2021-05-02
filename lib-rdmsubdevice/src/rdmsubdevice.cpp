/**
 * @file rdmsubdevice.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "rdmsubdevice.h"

RDMSubDevice::RDMSubDevice(const char* pLabel, uint16_t nDmxStartAddress, uint8_t nPersonalityCurrent):
	m_pRDMPersonalities(nullptr),
	m_IsFactoryDefaults(true),
	m_nCheckSum(0),
	m_nDmxStartAddressFactoryDefault(nDmxStartAddress),
	m_nCurrentPersonalityFactoryDefault(nPersonalityCurrent)
{
	for (uint32_t i = 0; (i < RDM_DEVICE_LABEL_MAX_LENGTH) && pLabel[i] != 0; i++) {
		m_aLabelFactoryDefault[i] = pLabel[i];
	}

	SetLabel(pLabel);

	m_tSubDevicesInfo.dmx_start_address = nDmxStartAddress;
	m_tSubDevicesInfo.current_personality = nPersonalityCurrent;

	m_tSubDevicesInfo.sensor_count = 0;
	m_tSubDevicesInfo.personality_count = 0;

	m_nCheckSum = CalculateChecksum();
}

RDMSubDevice::~RDMSubDevice() {
}

void RDMSubDevice::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	m_tSubDevicesInfo.dmx_start_address = nDmxStartAddress;
	UpdateEvent(RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS);
}

void RDMSubDevice::SetDmxFootprint(uint16_t nDmxFootprint) {
	m_tSubDevicesInfo.dmx_footprint = nDmxFootprint;
}

void RDMSubDevice::SetPersonalities(RDMPersonality **pRDMPersonalities, uint8_t nPersonalityCount) {
	assert(pRDMPersonalities != nullptr);

	m_tSubDevicesInfo.personality_count = nPersonalityCount;
	m_pRDMPersonalities = pRDMPersonalities;

	UpdateEvent(RDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY);
}

RDMPersonality* RDMSubDevice::GetPersonality(uint8_t nPersonality) {
	assert(nPersonality != 0);
	assert(nPersonality <= m_tSubDevicesInfo.personality_count);

	return m_pRDMPersonalities[nPersonality - 1];
}

void RDMSubDevice::GetLabel(struct TRDMDeviceInfoData* pInfoData) {
	assert(pInfoData != nullptr);

	pInfoData->data = m_tSubDevicesInfo.device_label;
	pInfoData->length = m_tSubDevicesInfo.device_label_length;
}

void RDMSubDevice::SetLabel(const char *pLabel) {
	assert(pLabel != nullptr);
	uint32_t i;

	for (i = 0; (i < RDM_DEVICE_LABEL_MAX_LENGTH) && (pLabel[i] != 0); i++) {
		m_tSubDevicesInfo.device_label[i] = pLabel[i];
	}

	m_tSubDevicesInfo.device_label_length = i;
}

void RDMSubDevice::SetLabel(const char *pLabel, uint8_t nLabelLength) {
	assert(pLabel != nullptr);
	uint32_t i;

	for (i = 0; (i < RDM_DEVICE_LABEL_MAX_LENGTH) && (i < nLabelLength); i++) {
		m_tSubDevicesInfo.device_label[i] = pLabel[i];
	}

	m_tSubDevicesInfo.device_label_length = i;
}

void RDMSubDevice::SetPersonalityCurrent(uint8_t nCurrent) {
	m_tSubDevicesInfo.current_personality = nCurrent;
}

bool RDMSubDevice::GetFactoryDefaults() {
	if (m_IsFactoryDefaults) {
		if (m_nCheckSum != CalculateChecksum()) {
			m_IsFactoryDefaults = false;
		}
	}

	return m_IsFactoryDefaults;
}

void RDMSubDevice::SetFactoryDefaults() {
	if (m_IsFactoryDefaults) {
		return;
	}

	SetLabel(m_aLabelFactoryDefault);

	m_tSubDevicesInfo.dmx_start_address = m_nDmxStartAddressFactoryDefault;
	m_tSubDevicesInfo.current_personality = m_nCurrentPersonalityFactoryDefault;

	m_IsFactoryDefaults = true;
}

uint16_t RDMSubDevice::CalculateChecksum() {
	uint16_t nChecksum = m_tSubDevicesInfo.dmx_start_address;
	nChecksum += m_tSubDevicesInfo.current_personality;

	for (uint32_t i = 0; i < m_tSubDevicesInfo.device_label_length; i++) {
		nChecksum += static_cast<uint16_t>(m_tSubDevicesInfo.device_label[i]);
	}

	return nChecksum;
}

void RDMSubDevice::UpdateEvent(__attribute__((unused)) TRDMSubDeviceUpdateEvent tUpdateEvent) {
	// override
}
