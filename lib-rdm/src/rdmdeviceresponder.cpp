/**
 * @file rdmdeviceresponder.cpp
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
#include <string.h>
#include <cassert>

#include "rdmdeviceresponder.h"
#include "rdmdevice.h"

#include "rdmsensors.h"
#include "rdmsubdevices.h"

#include "rdmsoftwareversion.h"
#include "rdmpersonality.h"
#include "rdmfactorydefaults.h"

#include "lightset.h"

#include "hardware.h"

#include "rdm_e120.h"

#include "debug.h"

static constexpr char LANGUAGE[2] = { 'e', 'n' };

#if defined(H3)
 #include "h3_board.h"
 static constexpr char DEVICE_LABEL[] = H3_BOARD_NAME " RDM Responder";
#elif defined (RASPPI) || defined (BARE_METAL)
 static constexpr char DEVICE_LABEL[] = "Raspberry Pi RDM Responder";
#elif defined (__CYGWIN__)
 static constexpr char DEVICE_LABEL[] = "Cygwin RDM Responder";
#elif defined (__linux__)
 static constexpr char DEVICE_LABEL[] = "Linux RDM Responder";
#elif defined (__APPLE__)
 static constexpr char DEVICE_LABEL[] = "MacOS RDM Responder";
#else
 static constexpr char DEVICE_LABEL[] = "RDM Responder";
#endif

RDMDeviceResponder *RDMDeviceResponder::s_pThis = nullptr;

RDMDeviceResponder::RDMDeviceResponder(RDMPersonality *pRDMPersonality, LightSet *pLightSet) :
		m_pRDMPersonality(pRDMPersonality),
		m_pLightSet(pLightSet),
		m_IsFactoryDefaults(true),
		m_nCheckSum(0),
		m_nDmxStartAddressFactoryDefault(DMX_START_ADDRESS_DEFAULT),
		m_nCurrentPersonalityFactoryDefault(RDM_DEFAULT_CURRENT_PERSONALITY),
		m_pRDMFactoryDefaults(nullptr)
{
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	m_aLanguage[0] = LANGUAGE[0];
	m_aLanguage[1] = LANGUAGE[1];

	memset(&m_tRDMDeviceInfo, 0, sizeof (struct TRDMDeviceInfo));
	memset(&m_tRDMSubDeviceInfo, 0, sizeof (struct TRDMDeviceInfo));

	m_pSoftwareVersion = const_cast<char*>(RDMSoftwareVersion::GetVersion());
	m_nSoftwareVersionLength = RDMSoftwareVersion::GetVersionLength();
	m_nDmxStartAddressFactoryDefault = m_pLightSet->GetDmxStartAddress();

	struct TRDMDeviceInfoData info;

	info.data = const_cast<char*>(DEVICE_LABEL);
	info.length = sizeof(DEVICE_LABEL) - 1;

	RDMDevice::SetLabel(&info);

	DEBUG_EXIT
}

void RDMDeviceResponder::Init() {
	DEBUG_ENTRY

	RDMDevice::Init();

	const auto nSoftwareVersionId = RDMSoftwareVersion::GetVersionId();
	const auto nDeviceModel = Hardware::Get()->GetBoardId();
	const auto nProductCategory = RDMDevice::GetProductCategory();
	const auto nSubDevices = m_RDMSubDevices.GetCount();

	m_tRDMDeviceInfo.protocol_major = (E120_PROTOCOL_VERSION >> 8);
	m_tRDMDeviceInfo.protocol_minor = static_cast<uint8_t>(E120_PROTOCOL_VERSION);
	m_tRDMDeviceInfo.device_model[0] = nDeviceModel >> 8;
	m_tRDMDeviceInfo.device_model[1] = nDeviceModel;
	m_tRDMDeviceInfo.product_category[0] = nProductCategory >> 8;
	m_tRDMDeviceInfo.product_category[1] = nProductCategory;
	m_tRDMDeviceInfo.software_version[0] = nSoftwareVersionId >> 24;
	m_tRDMDeviceInfo.software_version[1] = nSoftwareVersionId >> 16;
	m_tRDMDeviceInfo.software_version[2] = nSoftwareVersionId >> 8;
	m_tRDMDeviceInfo.software_version[3] = nSoftwareVersionId;
	m_tRDMDeviceInfo.dmx_footprint[0] = (m_pLightSet->GetDmxFootprint() >> 8);
	m_tRDMDeviceInfo.dmx_footprint[1] = m_pLightSet->GetDmxFootprint();
	m_tRDMDeviceInfo.dmx_start_address[0] = (m_nDmxStartAddressFactoryDefault >> 8);
	m_tRDMDeviceInfo.dmx_start_address[1] = m_nDmxStartAddressFactoryDefault;
	m_tRDMDeviceInfo.current_personality = m_nCurrentPersonalityFactoryDefault;
	m_tRDMDeviceInfo.personality_count = m_pRDMPersonality == nullptr ? 0 : 1;
	m_tRDMDeviceInfo.sub_device_count[0] = (nSubDevices >> 8);
	m_tRDMDeviceInfo.sub_device_count[1] = nSubDevices;
	m_tRDMDeviceInfo.sensor_count = m_RDMSensors.GetCount();

	memcpy(&m_tRDMSubDeviceInfo, &m_tRDMDeviceInfo, sizeof(struct TRDMDeviceInfo));

	m_nCheckSum = CalculateChecksum();

	DEBUG_EXIT
}

uint16_t RDMDeviceResponder::GetDmxFootPrint(uint16_t nSubDevice) {
	if (nSubDevice != RDM_ROOT_DEVICE) {
		return m_RDMSubDevices.GetDmxFootPrint(nSubDevice);
	}

	return (m_tRDMSubDeviceInfo.dmx_footprint[0] << 8) + m_tRDMSubDeviceInfo.dmx_footprint[1];
}

uint16_t RDMDeviceResponder::GetDmxStartAddress(uint16_t nSubDevice) {
	if (nSubDevice != RDM_ROOT_DEVICE) {
		return m_RDMSubDevices.GetDmxStartAddress(nSubDevice);
	}

	return (m_tRDMDeviceInfo.dmx_start_address[0] << 8) + m_tRDMDeviceInfo.dmx_start_address[1];
}

void RDMDeviceResponder::SetDmxStartAddress(uint16_t nSubDevice, uint16_t nDmxStartAddress) {
	if (nDmxStartAddress == 0 || nDmxStartAddress > DMX_UNIVERSE_SIZE)
		return;

	if (nSubDevice != RDM_ROOT_DEVICE) {
		m_RDMSubDevices.SetDmxStartAddress(nSubDevice, nDmxStartAddress);
		return;
	}

	if (m_pLightSet->SetDmxStartAddress(nDmxStartAddress)) {
		m_tRDMDeviceInfo.dmx_start_address[0] = (nDmxStartAddress >> 8);
		m_tRDMDeviceInfo.dmx_start_address[1] = nDmxStartAddress;
	}
}

uint8_t RDMDeviceResponder::GetPersonalityCount(uint16_t nSubDevice) {
	if (nSubDevice != RDM_ROOT_DEVICE) {
		return m_RDMSubDevices.GetPersonalityCount(nSubDevice);
	}

	return m_pRDMPersonality == nullptr ? 0 : 1;
}

uint8_t RDMDeviceResponder::GetPersonalityCurrent(uint16_t nSubDevice) {
	if (nSubDevice != RDM_ROOT_DEVICE) {
		return m_RDMSubDevices.GetPersonalityCurrent(nSubDevice);
	}

	return m_tRDMDeviceInfo.current_personality;
}

void RDMDeviceResponder::SetPersonalityCurrent(uint16_t nSubDevice, uint8_t nPersonality) {
	if (nSubDevice != RDM_ROOT_DEVICE) {
		m_RDMSubDevices.SetPersonalityCurrent(nSubDevice, nPersonality);
		return;
	}

	m_tRDMDeviceInfo.current_personality = nPersonality;
}

RDMPersonality* RDMDeviceResponder::GetPersonality(uint16_t nSubDevice,  uint8_t nPersonality) {
	if (nSubDevice != RDM_ROOT_DEVICE) {
		return m_RDMSubDevices.GetPersonality(nSubDevice, nPersonality);
	}

	return m_pRDMPersonality;
}

bool RDMDeviceResponder::GetSlotInfo(uint16_t nSubDevice, uint16_t nSlotOffset, struct TLightSetSlotInfo& tSlotInfo) {
	if (nSubDevice != RDM_ROOT_DEVICE) {
		return false; // TODO GetSlotInfo SubDevice
	}

	return m_pLightSet->GetSlotInfo(nSlotOffset, tSlotInfo);
}

struct TRDMDeviceInfo* RDMDeviceResponder::GetDeviceInfo(uint16_t nSubDevice) {
	if (nSubDevice != RDM_ROOT_DEVICE) {
		const struct TRDMSubDevicesInfo *sub_device_info = m_RDMSubDevices.GetInfo(nSubDevice);

		if (sub_device_info != nullptr) {
			m_tRDMSubDeviceInfo.dmx_footprint[0] = (sub_device_info->dmx_footprint >> 8);
			m_tRDMSubDeviceInfo.dmx_footprint[1] = sub_device_info->dmx_footprint;
			m_tRDMSubDeviceInfo.current_personality = sub_device_info->current_personality;
			m_tRDMSubDeviceInfo.personality_count = sub_device_info->personality_count;
			m_tRDMSubDeviceInfo.dmx_start_address[0] = (sub_device_info->dmx_start_address >> 8);
			m_tRDMSubDeviceInfo.dmx_start_address[1] =  sub_device_info->dmx_start_address;
			m_tRDMSubDeviceInfo.sensor_count = sub_device_info->sensor_count;
		}

		return &m_tRDMSubDeviceInfo;
	}

	return &m_tRDMDeviceInfo;
}

void RDMDeviceResponder::SetLabel(uint16_t nSubDevice, const char *pLabel, uint8_t nLabelLength) {
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

void RDMDeviceResponder::GetLabel(uint16_t nSubDevice, struct TRDMDeviceInfoData* pInfo) {

	if (nSubDevice != RDM_ROOT_DEVICE) {
		m_RDMSubDevices.GetLabel(nSubDevice, pInfo);
		return;
	}

	RDMDevice::GetLabel(pInfo);
}

void RDMDeviceResponder::SetLanguage(const char aLanguage[2]) {
	m_aLanguage[0] = aLanguage[0];
	m_aLanguage[1] = aLanguage[1];
}

uint16_t RDMDeviceResponder::CalculateChecksum() {
	uint16_t nChecksum = (m_tRDMDeviceInfo.dmx_start_address[0] >> 8) + m_tRDMDeviceInfo.dmx_start_address[1];
	nChecksum += m_tRDMDeviceInfo.current_personality;

	return nChecksum;
}

bool RDMDeviceResponder::GetFactoryDefaults() {
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

void RDMDeviceResponder::SetFactoryDefaults() {
	DEBUG_ENTRY

	RDMDevice::SetFactoryDefaults();

	SetPersonalityCurrent(RDM_ROOT_DEVICE, m_nCurrentPersonalityFactoryDefault);
	SetDmxStartAddress(RDM_ROOT_DEVICE, m_nDmxStartAddressFactoryDefault);

	memcpy(&m_tRDMSubDeviceInfo, &m_tRDMDeviceInfo, sizeof(struct TRDMDeviceInfo));

	m_RDMSubDevices.SetFactoryDefaults();

	m_IsFactoryDefaults = true;

	if (m_pRDMFactoryDefaults != nullptr) {
		DEBUG_PUTS("");
		m_pRDMFactoryDefaults->Set();
	}

	DEBUG_EXIT
}
