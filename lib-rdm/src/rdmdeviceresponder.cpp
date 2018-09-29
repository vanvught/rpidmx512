/**
 * @file rdmdeviceresponder.cpp
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
#include <assert.h>

#include "rdmdeviceresponder.h"
#include "rdmdevice.h"
#include "rdmsoftwareversion.h"
#include "rdmpersonality.h"

#include "hardware.h"

#include "rdm_e120.h"

#if defined (BARE_METAL)
 #include "util.h"
#elif defined(__circle__)
 #include "circle/util.h"
#else
 #include <string.h>
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#define DMX_UNIVERSE_SIZE 512

#if defined(H3)
 #include "h3_board.h"
 static const char DEVICE_LABEL[] ALIGNED = H3_BOARD_NAME " RDM Responder";
#elif defined (RASPPI) || defined(__circle__) || defined (BARE_METAL)
 static const char DEVICE_LABEL[] ALIGNED = "Raspberry Pi RDM Responder";
#elif defined (__CYGWIN__)
 static const char DEVICE_LABEL[] ALIGNED = "Cygwin RDM Responder";
#elif defined (__linux__)
 static const char DEVICE_LABEL[] ALIGNED = "Linux RDM Responder";
#elif defined (__APPLE__)
 static const char DEVICE_LABEL[] ALIGNED = "MacOS RDM Responder";
#else
 static const char DEVICE_LABEL[] ALIGNED = "RDM Responder";
#endif

static const char LANGUAGE[2] = { 'e', 'n' };

RDMDeviceResponder::RDMDeviceResponder(RDMPersonality *pRDMPersonality, LightSet *pLightSet, bool EnableSubDevices) :
		m_pRDMPersonality(pRDMPersonality),
		m_pLightSet(pLightSet),
		m_IsSubDevicesEnabled(EnableSubDevices),
		m_IsFactoryDefaults(true),
		m_nCheckSum(0),
		m_nDmxStartAddressFactoryDefault(DMX_DEFAULT_START_ADDRESS),
		m_nCurrentPersonalityFactoryDefault(RDM_DEFAULT_CURRENT_PERSONALITY)
{
	assert(pLightSet != 0);
	struct TRDMDeviceInfoData info;

	info.data = (uint8_t *) DEVICE_LABEL;
	info.length = sizeof(DEVICE_LABEL) - 1;

	RDMDevice::SetLabel(&info);

	m_aLanguage[0] = LANGUAGE[0];
	m_aLanguage[1] = LANGUAGE[1];

	memset(&m_tRDMDeviceInfo, 0, sizeof (struct TRDMDeviceInfo));
	memset(&m_tRDMSubDeviceInfo, 0, sizeof (struct TRDMDeviceInfo));

	m_pSoftwareVersion = (char *)RDMSoftwareVersion::GetVersion();
	m_nSoftwareVersionLength = RDMSoftwareVersion::GetVersionLength();

	const uint32_t nSoftwareVersionId = RDMSoftwareVersion::GetVersionId();
	const uint16_t nDeviceModel = Hardware::Get()->GetBoardId();

	m_nDmxStartAddressFactoryDefault = m_pLightSet->GetDmxStartAddress();

	(void) Load();
	Dump();

	m_RDMSensors.Init();

	if (m_IsSubDevicesEnabled) {
		m_RDMSubDevices.Init();
	}

	// Do not move
	const uint16_t nProductCategory = GetProductCategory();
	const uint16_t nSubDevices = m_RDMSubDevices.GetCount();

	m_tRDMDeviceInfo.protocol_major = (uint8_t) (E120_PROTOCOL_VERSION >> 8);
	m_tRDMDeviceInfo.protocol_minor = (uint8_t) E120_PROTOCOL_VERSION;
	m_tRDMDeviceInfo.device_model[0] = (uint8_t) (nDeviceModel >> 8);
	m_tRDMDeviceInfo.device_model[1] = (uint8_t) nDeviceModel;
	m_tRDMDeviceInfo.product_category[0] = (uint8_t) (nProductCategory >> 8);
	m_tRDMDeviceInfo.product_category[1] = (uint8_t) nProductCategory;
	m_tRDMDeviceInfo.software_version[0] = (uint8_t) (nSoftwareVersionId >> 24);
	m_tRDMDeviceInfo.software_version[1] = (uint8_t) (nSoftwareVersionId >> 16);
	m_tRDMDeviceInfo.software_version[2] = (uint8_t) (nSoftwareVersionId >> 8);
	m_tRDMDeviceInfo.software_version[3] = (uint8_t) nSoftwareVersionId;
	m_tRDMDeviceInfo.dmx_footprint[0] = (uint8_t) (m_pLightSet->GetDmxFootprint() >> 8);
	m_tRDMDeviceInfo.dmx_footprint[1] = (uint8_t) m_pLightSet->GetDmxFootprint();
	m_tRDMDeviceInfo.dmx_start_address[0] = (uint8_t) (m_nDmxStartAddressFactoryDefault >> 8);
	m_tRDMDeviceInfo.dmx_start_address[1] = (uint8_t) m_nDmxStartAddressFactoryDefault;
	m_tRDMDeviceInfo.current_personality = m_nCurrentPersonalityFactoryDefault;
	m_tRDMDeviceInfo.personality_count = m_pRDMPersonality == 0 ? (uint8_t) 0 : (uint8_t) 1;
	m_tRDMDeviceInfo.sub_device_count[0] = (uint8_t) (nSubDevices >> 8);
	m_tRDMDeviceInfo.sub_device_count[1] = (uint8_t) nSubDevices;
	m_tRDMDeviceInfo.sensor_count = m_RDMSensors.GetCount();

	memcpy(&m_tRDMSubDeviceInfo, &m_tRDMDeviceInfo, sizeof (struct TRDMDeviceInfo));

	m_nCheckSum = CalculateChecksum();
}

RDMDeviceResponder::~RDMDeviceResponder(void) {
	memset(&m_tRDMDeviceInfo, 0, sizeof (struct TRDMDeviceInfo));
	memset(&m_tRDMSubDeviceInfo, 0, sizeof (struct TRDMDeviceInfo));
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
		m_tRDMDeviceInfo.dmx_start_address[0] = (uint8_t) (nDmxStartAddress >> 8);
		m_tRDMDeviceInfo.dmx_start_address[1] = (uint8_t) nDmxStartAddress;
	}
}

uint8_t RDMDeviceResponder::GetPersonalityCount(uint16_t nSubDevice) {
	if (nSubDevice != RDM_ROOT_DEVICE) {
		return m_RDMSubDevices.GetPersonalityCount(nSubDevice);
	}

	return m_pRDMPersonality == 0 ? (uint8_t) 0 : (uint8_t) 1;
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

uint16_t RDMDeviceResponder::GetSubDeviceCount(void) {
	return (m_tRDMDeviceInfo.sub_device_count[0] << 8) + m_tRDMDeviceInfo.sub_device_count[1];
}

uint8_t RDMDeviceResponder::GetSensorCount(uint16_t nSubDevice) {
	return m_tRDMDeviceInfo.sensor_count;
}

const struct TRDMSensorDefintion* RDMDeviceResponder::GetSensorDefinition(uint8_t nSensor) {
	return m_RDMSensors.GetDefintion(nSensor);
}

const struct TRDMSensorValues* RDMDeviceResponder::GetSensorValues(uint8_t nSensor) {
	return m_RDMSensors.GetValues(nSensor);
}

void RDMDeviceResponder::SetSensorValues(uint8_t nSensor) {
	 m_RDMSensors.SetSensorValues(nSensor);
}

void RDMDeviceResponder::SetSensorRecord(uint8_t nSensor) {
	m_RDMSensors.SetSensorRecord(nSensor);
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

		if (sub_device_info != 0) {
			m_tRDMSubDeviceInfo.dmx_footprint[0] = (uint8_t) (sub_device_info->dmx_footprint >> 8);
			m_tRDMSubDeviceInfo.dmx_footprint[1] = (uint8_t) sub_device_info->dmx_footprint;
			m_tRDMSubDeviceInfo.current_personality = sub_device_info->current_personality;
			m_tRDMSubDeviceInfo.personality_count = sub_device_info->personality_count;
			m_tRDMSubDeviceInfo.dmx_start_address[0] = (uint8_t) (sub_device_info->dmx_start_address >> 8);
			m_tRDMSubDeviceInfo.dmx_start_address[1] = (uint8_t) sub_device_info->dmx_start_address;
			m_tRDMSubDeviceInfo.sensor_count = sub_device_info->sensor_count;
		}

		return &m_tRDMSubDeviceInfo;
	}

	return &m_tRDMDeviceInfo;
}

void RDMDeviceResponder::SetLabel(uint16_t nSubDevice, const uint8_t *pLabel, uint8_t nLabelLength) {
	struct TRDMDeviceInfoData info;

	if (nLabelLength > RDM_DEVICE_LABEL_MAX_LENGTH) {
		nLabelLength = RDM_DEVICE_LABEL_MAX_LENGTH;
	}

	if (nSubDevice != RDM_ROOT_DEVICE) {
		m_RDMSubDevices.SetLabel(nSubDevice, pLabel, nLabelLength);
		return;
	}

	info.data = (uint8_t *) pLabel;
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
	m_aLanguage[0] = (char) aLanguage[0];
	m_aLanguage[1] = (char) aLanguage[1];
}

uint16_t RDMDeviceResponder::CalculateChecksum(void) {
	struct TRDMDeviceInfoData LabelInfo;

	uint16_t checksum = (m_tRDMDeviceInfo.dmx_start_address[0] >> 8) + m_tRDMDeviceInfo.dmx_start_address[1];
	checksum += m_tRDMDeviceInfo.current_personality;

	RDMDevice::GetLabel(&LabelInfo);

	for (uint8_t i = 0; i < LabelInfo.length; i++) {
		checksum += (uint16_t) LabelInfo.data[i];
	}

	return checksum;
}

bool RDMDeviceResponder::GetFactoryDefaults(void) {
	if (m_IsFactoryDefaults) {
		if (m_nCheckSum != CalculateChecksum()) {
			m_IsFactoryDefaults = false;
			return false;
		}
	}

	return m_RDMSubDevices.GetFactoryDefaults();
}

void RDMDeviceResponder::SetFactoryDefaults(void) {
	if (!m_IsFactoryDefaults) {
		struct TRDMDeviceInfoData info;

		info.data = (uint8_t *) DEVICE_LABEL;
		info.length = sizeof(DEVICE_LABEL) - 1;

		RDMDevice::SetLabel(&info);

		(void) Load();

		SetPersonalityCurrent(RDM_ROOT_DEVICE, m_nCurrentPersonalityFactoryDefault);
		SetDmxStartAddress(RDM_ROOT_DEVICE, m_nDmxStartAddressFactoryDefault);

		memcpy(&m_tRDMSubDeviceInfo, &m_tRDMDeviceInfo, sizeof(struct TRDMDeviceInfo));

		m_IsFactoryDefaults = true;

	}

	m_RDMSubDevices.SetFactoryDefaults();
}
