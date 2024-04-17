/**
 * @file rdmsubdevice.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMSUBDEVICE_H_
#define RDMSUBDEVICE_H_

#include "rdmdevice.h"
#include "rdmpersonality.h"

struct TRDMSubDevicesInfo {
	uint16_t dmx_footprint;
	uint16_t dmx_start_address;
	uint8_t current_personality;
	uint8_t personality_count;
	char aLabel[RDM_DEVICE_LABEL_MAX_LENGTH];
	uint8_t nLabelLength;
	uint8_t sensor_count;
};

enum TRDMSubDeviceUpdateEvent {
	RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS,
	RDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY
};

class RDMSubDevice {
public:
	RDMSubDevice(const char* pLabel, uint16_t nDmxStartAddress = 1, uint8_t nPersonalityCurrent = 1):
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

	virtual ~RDMSubDevice() = default;

	void SetDmxStartAddress(uint16_t nDmxStartAddress) {
		m_tSubDevicesInfo.dmx_start_address = nDmxStartAddress;
		UpdateEvent(RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS);
	}

	uint16_t GetDmxStartAddress() const {
		return m_tSubDevicesInfo.dmx_start_address;
	}

	uint8_t GetPersonalityCurrent() const {
		return m_tSubDevicesInfo.current_personality;
	}

	void SetPersonalityCurrent(uint8_t nCurrent) {
		m_tSubDevicesInfo.current_personality = nCurrent;
	}

	void GetLabel(struct TRDMDeviceInfoData* pInfoData) {
		assert(pInfoData != nullptr);

		pInfoData->data = m_tSubDevicesInfo.aLabel;
		pInfoData->length = m_tSubDevicesInfo.nLabelLength;
	}

	void SetLabel(const char *pLabel) {
		assert(pLabel != nullptr);
		uint32_t i;

		for (i = 0; (i < RDM_DEVICE_LABEL_MAX_LENGTH) && (pLabel[i] != 0); i++) {
			m_tSubDevicesInfo.aLabel[i] = pLabel[i];
		}

		m_tSubDevicesInfo.nLabelLength = static_cast<uint8_t>(i);
	}

	void SetLabel(const char *pLabel, uint8_t nLabelLength) {
		assert(pLabel != nullptr);
		uint32_t i;

		for (i = 0; (i < RDM_DEVICE_LABEL_MAX_LENGTH) && (i < nLabelLength); i++) {
			m_tSubDevicesInfo.aLabel[i] = pLabel[i];
		}

		m_tSubDevicesInfo.nLabelLength = static_cast<uint8_t>(i);
	}

	struct TRDMSubDevicesInfo* GetInfo() {
		return &m_tSubDevicesInfo;
	}

	RDMPersonality *GetPersonality(uint8_t nPersonality) {
		assert(nPersonality != 0);
		assert(nPersonality <= m_tSubDevicesInfo.personality_count);

		return m_pRDMPersonalities[nPersonality - 1];
	}

	uint8_t GetPersonalityCount() const {
		return m_tSubDevicesInfo.personality_count;
	}

	uint16_t GetDmxFootPrint() const {
		return m_tSubDevicesInfo.dmx_footprint;
	}

	bool GetFactoryDefaults() {
		if (m_IsFactoryDefaults) {
			if (m_nCheckSum != CalculateChecksum()) {
				m_IsFactoryDefaults = false;
			}
		}

		return m_IsFactoryDefaults;
	}

	void SetFactoryDefaults() {
		if (m_IsFactoryDefaults) {
			return;
		}

		SetLabel(m_aLabelFactoryDefault);

		m_tSubDevicesInfo.dmx_start_address = m_nDmxStartAddressFactoryDefault;
		m_tSubDevicesInfo.current_personality = m_nCurrentPersonalityFactoryDefault;

		m_IsFactoryDefaults = true;
	}

	virtual bool Initialize()=0;
	virtual void Start()= 0;
	virtual void Stop()= 0;
	virtual void Data(const uint8_t *pDdata, uint32_t nLength)=0;

protected:
	void SetDmxFootprint(uint16_t nDmxFootprint) {
		m_tSubDevicesInfo.dmx_footprint = nDmxFootprint;
	}

	void SetPersonalities(RDMPersonality **pRDMPersonalities, uint8_t nPersonalityCount) {
		assert(pRDMPersonalities != nullptr);

		m_tSubDevicesInfo.personality_count = nPersonalityCount;
		m_pRDMPersonalities = pRDMPersonalities;

		UpdateEvent(RDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY);
	}

private:
	virtual void UpdateEvent([[maybe_unused]] TRDMSubDeviceUpdateEvent tUpdateEvent) {}
	uint16_t CalculateChecksum() {
		uint16_t nChecksum = m_tSubDevicesInfo.dmx_start_address;
		nChecksum = static_cast<uint16_t>(nChecksum + m_tSubDevicesInfo.current_personality);

		for (uint32_t i = 0; i < m_tSubDevicesInfo.nLabelLength; i++) {
			nChecksum = static_cast<uint16_t>(nChecksum + m_tSubDevicesInfo.aLabel[i]);
		}

		return nChecksum;
	}

private:
	RDMPersonality **m_pRDMPersonalities { nullptr };
	bool m_IsFactoryDefaults { true };
	uint16_t m_nCheckSum { 0 };
	uint16_t m_nDmxStartAddressFactoryDefault;
	uint8_t m_nCurrentPersonalityFactoryDefault;
	TRDMSubDevicesInfo m_tSubDevicesInfo;
	char m_aLabelFactoryDefault[RDM_DEVICE_LABEL_MAX_LENGTH];
};

#endif /* RDMSUBDEVICE_H_ */
