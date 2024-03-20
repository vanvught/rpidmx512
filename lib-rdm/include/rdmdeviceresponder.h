/**
 * @file rdmdeviceresponder.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "rdmconst.h"
#include "rdmdevice.h"
#include "rdmidentify.h"
#include "rdmpersonality.h"
#include "rdmsensors.h"
#include "rdmsubdevices.h"

#include "lightset.h"

namespace rdm {
namespace device {
namespace responder {
static constexpr uint8_t DEFAULT_CURRENT_PERSONALITY = 1;

///< http://rdm.openlighting.org/pid/display?manufacturer=0&pid=96
struct DeviceInfo {
	uint8_t protocol_major;			///< The response for this field shall always be same regardless of whether this message is directed to the Root Device or a Sub-Device.
	uint8_t protocol_minor;			///< The response for this field shall always be same regardless of whether this message is directed to the Root Device or a Sub-Device.
	uint8_t device_model[2];		///< This field identifies the Device Model ID of the Root Device or the Sub-Device. The Manufacturer shall not use the same ID to represent more than one unique model type.
	uint8_t product_category[2];	///< Devices shall report a Product Category based on the products primary function.
	uint8_t software_version[4];	///< This field indicates the Software Version ID for the device. The Software Version ID is a 32-bit value determined by the Manufacturer.
	uint8_t dmx_footprint[2];		///< If the DEVICE_INFO message is directed to a Sub-Device, then the response for this field contains the DMX512 Footprint for that Sub-Device. If the message is sent to the Root Device, it is the Footprint for the Root Device itself. If the Device or Sub-Device does not utilize Null Start Code packets for any control or functionality then it shall report a Footprint of 0x0000.
	uint8_t current_personality;	///<
	uint8_t personality_count;		///<
	uint8_t dmx_start_address[2];	///< If the Device or Sub-Device that this message is directed to has a DMX512 Footprint of 0, then this field shall be set to 0xFFFF.
	uint8_t sub_device_count[2];	///< The response for this field shall always be same regardless of whether this message is directed to the Root Device or a Sub-Device.
	uint8_t sensor_count;			///< This field indicates the number of available sensors in a Root Device or Sub-Device. When this parameter is directed to a Sub-Device, the reply shall be identical for any Sub-Device owned by a specific Root Device.
};

void factorydefaults();
}  // namespace responder
}  // namespace device
}  // namespace rdm

class RDMDeviceResponder: public RDMDevice {
public:
	RDMDeviceResponder(RDMPersonality **pRDMPersonalities, const uint32_t nPersonalityCount, const uint32_t nCurrentPersonality = rdm::device::responder::DEFAULT_CURRENT_PERSONALITY);
	virtual ~RDMDeviceResponder() = default;

	void Init();
	void Print();

	// E120_DEVICE_INFO				0x0060
	struct rdm::device::responder::DeviceInfo *GetDeviceInfo(uint16_t nSubDevice = RDM_ROOT_DEVICE) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			const auto *sub_device_info = m_RDMSubDevices.GetInfo(nSubDevice);

			if (sub_device_info != nullptr) {
				m_SubDeviceInfo.dmx_footprint[0] = static_cast<uint8_t>(sub_device_info->dmx_footprint >> 8);
				m_SubDeviceInfo.dmx_footprint[1] = static_cast<uint8_t>(sub_device_info->dmx_footprint);
				m_SubDeviceInfo.current_personality = sub_device_info->current_personality;
				m_SubDeviceInfo.personality_count = sub_device_info->personality_count;
				m_SubDeviceInfo.dmx_start_address[0] = static_cast<uint8_t>(sub_device_info->dmx_start_address >> 8);
				m_SubDeviceInfo.dmx_start_address[1] =  static_cast<uint8_t>(sub_device_info->dmx_start_address);
				m_SubDeviceInfo.sensor_count = sub_device_info->sensor_count;
			}

			return &m_SubDeviceInfo;
		}

		//TODO FIXME Quick fix
		const auto nProductCategory = RDMDevice::GetProductCategory();
		m_DeviceInfo.product_category[0] =static_cast<uint8_t>( nProductCategory >> 8);
		m_DeviceInfo.product_category[1] = static_cast<uint8_t>(nProductCategory);

		return &m_DeviceInfo;
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

		assert(m_pRDMPersonalities != nullptr);

		SetPersonalityCurrent(RDM_ROOT_DEVICE, rdm::device::responder::DEFAULT_CURRENT_PERSONALITY);
		SetDmxStartAddress(RDM_ROOT_DEVICE, m_nDmxStartAddressFactoryDefault);

		memcpy(&m_SubDeviceInfo, &m_DeviceInfo, sizeof(struct rdm::device::responder::DeviceInfo));

		m_RDMSubDevices.SetFactoryDefaults();

		m_nCheckSum = CalculateChecksum();
		m_IsFactoryDefaults = true;

		rdm::device::responder::factorydefaults();
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
		DEBUG_ENTRY

		if (nDmxStartAddress == 0 || nDmxStartAddress > lightset::dmx::UNIVERSE_SIZE)
			return;

		if (nSubDevice != RDM_ROOT_DEVICE) {
			m_RDMSubDevices.SetDmxStartAddress(nSubDevice, nDmxStartAddress);
			return;
		}

		const auto *pPersonality = m_pRDMPersonalities[m_DeviceInfo.current_personality - 1];
		assert(pPersonality != nullptr);

		auto *pLightSet = pPersonality->GetLightSet();

		if (pLightSet != nullptr) {
			if (pLightSet->SetDmxStartAddress(nDmxStartAddress)) {
				m_DeviceInfo.dmx_start_address[0] = static_cast<uint8_t>(nDmxStartAddress >> 8);
				m_DeviceInfo.dmx_start_address[1] = static_cast<uint8_t>(nDmxStartAddress);
			}

			DmxStartAddressUpdate();
		}

		DEBUG_EXIT
	}

	uint16_t GetDmxStartAddress(uint16_t nSubDevice = RDM_ROOT_DEVICE) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			return m_RDMSubDevices.GetDmxStartAddress(nSubDevice);
		}

		return static_cast<uint16_t>((m_DeviceInfo.dmx_start_address[0] << 8) + m_DeviceInfo.dmx_start_address[1]);
	}

	// E120_SLOT_INFO				0x0120
	bool GetSlotInfo(uint16_t nSubDevice,uint16_t nSlotOffset, lightset::SlotInfo &tSlotInfo) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			return false; // TODO GetSlotInfo SubDevice
		}

		const auto *pPersonality = m_pRDMPersonalities[m_DeviceInfo.current_personality - 1];
		auto *pLightSet = pPersonality->GetLightSet();

		return pLightSet->GetSlotInfo(nSlotOffset, tSlotInfo);
	}

	uint16_t GetDmxFootPrint(uint16_t nSubDevice = RDM_ROOT_DEVICE) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			return m_RDMSubDevices.GetDmxFootPrint(nSubDevice);
		}

		return static_cast<uint16_t>((m_DeviceInfo.dmx_footprint[0] << 8) + m_DeviceInfo.dmx_footprint[1]);
	}

	// Personalities
	RDMPersonality* GetPersonality(uint16_t nSubDevice, uint8_t nPersonality) {
		assert(nPersonality >= 1);

		if (nSubDevice != RDM_ROOT_DEVICE) {
			return m_RDMSubDevices.GetPersonality(nSubDevice, nPersonality);
		}

		assert(nPersonality <= m_DeviceInfo.personality_count);

		return m_pRDMPersonalities[nPersonality - 1];
	}

	uint8_t GetPersonalityCount(uint16_t nSubDevice = RDM_ROOT_DEVICE) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			return m_RDMSubDevices.GetPersonalityCount(nSubDevice);
		}

		return m_DeviceInfo.personality_count;
	}

	void SetPersonalityCurrent(uint16_t nSubDevice, uint8_t nPersonality) {
		assert(nPersonality >= 1);

		if (nSubDevice != RDM_ROOT_DEVICE) {
			m_RDMSubDevices.SetPersonalityCurrent(nSubDevice, nPersonality);
			return;
		}

		m_DeviceInfo.current_personality = nPersonality;

		assert(nPersonality <= m_DeviceInfo.personality_count);

		const auto *pPersonality = m_pRDMPersonalities[nPersonality - 1];
		assert(pPersonality != nullptr);

		auto *pLightSet = pPersonality->GetLightSet();

		if (pLightSet != nullptr) {
			m_DeviceInfo.dmx_footprint[0] = static_cast<uint8_t>(pLightSet->GetDmxFootprint() >> 8);
			m_DeviceInfo.dmx_footprint[1] = static_cast<uint8_t>(pLightSet->GetDmxFootprint());
			m_DeviceInfo.dmx_start_address[0] = static_cast<uint8_t>(pLightSet->GetDmxStartAddress() >> 8);
			m_DeviceInfo.dmx_start_address[1] = static_cast<uint8_t>(pLightSet->GetDmxStartAddress());

			PersonalityUpdate(pLightSet);
		}
	}

	uint8_t GetPersonalityCurrent(uint16_t nSubDevice = RDM_ROOT_DEVICE) {
		if (nSubDevice != RDM_ROOT_DEVICE) {
			return m_RDMSubDevices.GetPersonalityCurrent(nSubDevice);
		}

		return m_DeviceInfo.current_personality;
	}

	static RDMDeviceResponder* Get() {
		return s_pThis;
	}

private:
	uint16_t CalculateChecksum() {
		auto nChecksum = static_cast<uint16_t>((m_DeviceInfo.dmx_start_address[0] >> 8) + m_DeviceInfo.dmx_start_address[1]);
		nChecksum = static_cast<uint16_t>(nChecksum + m_DeviceInfo.current_personality);
		return nChecksum;
	}

	virtual void PersonalityUpdate(LightSet *pLightSet);
	virtual void DmxStartAddressUpdate();

private:
	RDMIdentify m_RDMIdentify;
	RDMSensors m_RDMSensors;
	RDMSubDevices m_RDMSubDevices;
	RDMPersonality **m_pRDMPersonalities;
	char *m_pSoftwareVersion;
	uint8_t m_nSoftwareVersionLength;
	rdm::device::responder::DeviceInfo m_DeviceInfo;
	rdm::device::responder::DeviceInfo m_SubDeviceInfo;
	char m_aLanguage[2];
	//
	bool m_IsFactoryDefaults { true };
	uint16_t m_nCheckSum { 0 };
	uint16_t m_nDmxStartAddressFactoryDefault { lightset::dmx::START_ADDRESS_DEFAULT };

	static RDMDeviceResponder *s_pThis;
};

#endif /* RDMDEVICERESPONDER_H_ */
