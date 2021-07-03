/**
 * @file rdmdeviceresponder.h
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

#ifndef RDMDEVICERESPONDER_H_
#define RDMDEVICERESPONDER_H_

#include <cstdint>
#include <cstring>

#include "rdm.h"
#include "rdmdevice.h"
#include "rdmpersonality.h"
#include "rdmsensors.h"
#include "rdmsubdevices.h"
#include "rdmfactorydefaults.h"

#include "lightset.h"

enum {
	RDM_DEFAULT_CURRENT_PERSONALITY = 1
};

class RDMDeviceResponder: public RDMDevice {
public:
	RDMDeviceResponder(RDMPersonality *pRDMPersonality, LightSet *pLightSet);

	void Init();
	void Print();

	// E120_DEVICE_INFO				0x0060
	struct TRDMDeviceInfo *GetDeviceInfo(uint16_t nSubDevice = RDM_ROOT_DEVICE) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			const auto *sub_device_info = m_RDMSubDevices.GetInfo(nSubDevice);

			if (sub_device_info != nullptr) {
				m_tRDMSubDeviceInfo.dmx_footprint[0] = static_cast<uint8_t>(sub_device_info->dmx_footprint >> 8);
				m_tRDMSubDeviceInfo.dmx_footprint[1] = static_cast<uint8_t>(sub_device_info->dmx_footprint);
				m_tRDMSubDeviceInfo.current_personality = sub_device_info->current_personality;
				m_tRDMSubDeviceInfo.personality_count = sub_device_info->personality_count;
				m_tRDMSubDeviceInfo.dmx_start_address[0] = static_cast<uint8_t>(sub_device_info->dmx_start_address >> 8);
				m_tRDMSubDeviceInfo.dmx_start_address[1] =  static_cast<uint8_t>(sub_device_info->dmx_start_address);
				m_tRDMSubDeviceInfo.sensor_count = sub_device_info->sensor_count;
			}

			return &m_tRDMSubDeviceInfo;
		}

		return &m_tRDMDeviceInfo;
	}

	// E120_DEVICE_LABEL			0x0082
	void SetLabel(uint16_t nSubDevice, const char *pLabel, uint8_t nLabelLength) {
		struct TRDMDeviceInfoData info;

		if (nLabelLength > RDM_DEVICE_LABEL_MAX_LENGTH) {
			nLabelLength = RDM_DEVICE_LABEL_MAX_LENGTH;
		}

		if (nSubDevice != RDM_ROOT_DEVICE) {
			m_RDMSubDevices.SetLabel(nSubDevice, pLabel, nLabelLength);
			return;
		}

		info.data = const_cast<char*>(pLabel);
		info.length = nLabelLength;

		RDMDevice::SetLabel(&info);
	}

	void GetLabel(uint16_t nSubDevice, struct TRDMDeviceInfoData *pInfo) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			m_RDMSubDevices.GetLabel(nSubDevice, pInfo);
			return;
		}

		RDMDevice::GetLabel(pInfo);
	}

	// E120_FACTORY_DEFAULTS		0x0090
	void SetFactoryDefaults() {
		RDMDevice::SetFactoryDefaults();

		SetPersonalityCurrent(RDM_ROOT_DEVICE, m_nCurrentPersonalityFactoryDefault);
		SetDmxStartAddress(RDM_ROOT_DEVICE, m_nDmxStartAddressFactoryDefault);

		memcpy(&m_tRDMSubDeviceInfo, &m_tRDMDeviceInfo, sizeof(struct TRDMDeviceInfo));

		m_RDMSubDevices.SetFactoryDefaults();

		m_IsFactoryDefaults = true;

		if (m_pRDMFactoryDefaults != nullptr) {
			m_pRDMFactoryDefaults->Set();
		}
	}

	bool GetFactoryDefaults() {
		if (m_IsFactoryDefaults) {
			if (!RDMDevice::GetFactoryDefaults()) {
				m_IsFactoryDefaults = false;
				return false;
			}

			if (m_nCheckSum != CalculateChecksum()) {
				m_IsFactoryDefaults = false;
				return false;
			}

			if (!m_RDMSubDevices.GetFactoryDefaults()) {
				m_IsFactoryDefaults = false;
				return false;
			}
		}

		return m_IsFactoryDefaults;
	}

	// E120_LANGUAGE				0x00B0
	void SetLanguage(const char aLanguage[2]) {
		m_aLanguage[0] = aLanguage[0];
		m_aLanguage[1] = aLanguage[1];
	}
	const char* GetLanguage() const {
		return m_aLanguage;
	}

	// E120_SOFTWARE_VERSION_LABEL	0x00C0
	const char* GetSoftwareVersion() const {
		return m_pSoftwareVersion;
	}
	uint8_t GetSoftwareVersionLength() const {
		return m_nSoftwareVersionLength;
	}

	// E120_DMX_START_ADDRESS		0x00F0
	void SetDmxStartAddress(uint16_t nSubDevice, uint16_t nDmxStartAddress) {
		if (nDmxStartAddress == 0 || nDmxStartAddress > lightset::Dmx::UNIVERSE_SIZE)
			return;

		if (nSubDevice != RDM_ROOT_DEVICE) {
			m_RDMSubDevices.SetDmxStartAddress(nSubDevice, nDmxStartAddress);
			return;
		}

		if (m_pLightSet->SetDmxStartAddress(nDmxStartAddress)) {
			m_tRDMDeviceInfo.dmx_start_address[0] = static_cast<uint8_t>(nDmxStartAddress >> 8);
			m_tRDMDeviceInfo.dmx_start_address[1] = static_cast<uint8_t>(nDmxStartAddress);
		}
	}

	uint16_t GetDmxStartAddress(uint16_t nSubDevice = RDM_ROOT_DEVICE) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			return m_RDMSubDevices.GetDmxStartAddress(nSubDevice);
		}

		return static_cast<uint16_t>((m_tRDMDeviceInfo.dmx_start_address[0] << 8) + m_tRDMDeviceInfo.dmx_start_address[1]);
	}

	// E120_SLOT_INFO				0x0120
	bool GetSlotInfo(uint16_t nSubDevice,uint16_t nSlotOffset, lightset::SlotInfo &tSlotInfo) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			return false; // TODO GetSlotInfo SubDevice
		}

		return m_pLightSet->GetSlotInfo(nSlotOffset, tSlotInfo);
	}

	uint16_t GetDmxFootPrint(uint16_t nSubDevice = RDM_ROOT_DEVICE) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			return m_RDMSubDevices.GetDmxFootPrint(nSubDevice);
		}

		return static_cast<uint16_t>((m_tRDMSubDeviceInfo.dmx_footprint[0] << 8) + m_tRDMSubDeviceInfo.dmx_footprint[1]);
	}

	// Personalities
	RDMPersonality* GetPersonality(uint16_t nSubDevice = RDM_ROOT_DEVICE, uint8_t nPersonality = 1) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			return m_RDMSubDevices.GetPersonality(nSubDevice, nPersonality);
		}

		return m_pRDMPersonality;
	}

	uint8_t GetPersonalityCount(uint16_t nSubDevice = RDM_ROOT_DEVICE) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			return m_RDMSubDevices.GetPersonalityCount(nSubDevice);
		}

		return m_pRDMPersonality == nullptr ? 0 : 1;
	}

	void SetPersonalityCurrent(uint16_t nSubDevice, uint8_t nPersonality) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			m_RDMSubDevices.SetPersonalityCurrent(nSubDevice, nPersonality);
			return;
		}

		m_tRDMDeviceInfo.current_personality = nPersonality;
	}

	uint8_t GetPersonalityCurrent(uint16_t nSubDevice = RDM_ROOT_DEVICE) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			return m_RDMSubDevices.GetPersonalityCurrent(nSubDevice);
		}

		return m_tRDMDeviceInfo.current_personality;
	}

	// Handler
	void SetRDMFactoryDefaults(RDMFactoryDefaults *pRDMFactoryDefaults) {
		m_pRDMFactoryDefaults = pRDMFactoryDefaults;
	}

	static RDMDeviceResponder* Get() {
		return s_pThis;
	}

private:
	uint16_t CalculateChecksum() {
		auto nChecksum = static_cast<uint16_t>((m_tRDMDeviceInfo.dmx_start_address[0] >> 8) + m_tRDMDeviceInfo.dmx_start_address[1]);
		nChecksum = static_cast<uint16_t>(nChecksum + m_tRDMDeviceInfo.current_personality);
		return nChecksum;
	}

private:
	RDMSensors m_RDMSensors;
	RDMSubDevices m_RDMSubDevices;
	RDMPersonality *m_pRDMPersonality;
	LightSet* m_pLightSet;
	char* m_pSoftwareVersion;
	uint8_t m_nSoftwareVersionLength;
	struct TRDMDeviceInfo m_tRDMDeviceInfo;
	struct TRDMDeviceInfo m_tRDMSubDeviceInfo;
	char m_aLanguage[2];
	//
	bool m_IsFactoryDefaults;
	uint16_t m_nCheckSum;
	uint16_t m_nDmxStartAddressFactoryDefault;
	uint8_t m_nCurrentPersonalityFactoryDefault;
	//
	RDMFactoryDefaults *m_pRDMFactoryDefaults;

	static RDMDeviceResponder *s_pThis;
};

#endif /* RDMDEVICERESPONDER_H_ */
