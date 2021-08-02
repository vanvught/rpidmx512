/**
 * @file rdmdeviceresponder.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
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

using namespace lightset;

static constexpr char LANGUAGE[2] = { 'e', 'n' };

#if defined(H3)
# include "h3_board.h"
  static constexpr char DEVICE_LABEL[] = H3_BOARD_NAME " RDM Responder";
#elif defined (RASPPI)
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
		m_nDmxStartAddressFactoryDefault(Dmx::START_ADDRESS_DEFAULT),
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
	m_nSoftwareVersionLength = static_cast<uint8_t>(RDMSoftwareVersion::GetVersionLength());
	if (m_pLightSet == nullptr) {
		m_nDmxStartAddressFactoryDefault = lightset::Dmx::ADDRESS_INVALID;
	} else {
		m_nDmxStartAddressFactoryDefault = m_pLightSet->GetDmxStartAddress();
	}

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
	m_tRDMDeviceInfo.device_model[0] = static_cast<uint8_t>(nDeviceModel >> 8);
	m_tRDMDeviceInfo.device_model[1] = static_cast<uint8_t>(nDeviceModel);
	m_tRDMDeviceInfo.product_category[0] =static_cast<uint8_t>( nProductCategory >> 8);
	m_tRDMDeviceInfo.product_category[1] = static_cast<uint8_t>(nProductCategory);
	m_tRDMDeviceInfo.software_version[0] = static_cast<uint8_t>(nSoftwareVersionId >> 24);
	m_tRDMDeviceInfo.software_version[1] = static_cast<uint8_t>(nSoftwareVersionId >> 16);
	m_tRDMDeviceInfo.software_version[2] = static_cast<uint8_t>(nSoftwareVersionId >> 8);
	m_tRDMDeviceInfo.software_version[3] = static_cast<uint8_t>(nSoftwareVersionId);
	if (m_pLightSet == nullptr) {
		m_tRDMDeviceInfo.dmx_footprint[0] = 0;
		m_tRDMDeviceInfo.dmx_footprint[1] = 0;
	} else {
		m_tRDMDeviceInfo.dmx_footprint[0] = static_cast<uint8_t>(m_pLightSet->GetDmxFootprint() >> 8);
		m_tRDMDeviceInfo.dmx_footprint[1] = static_cast<uint8_t>( m_pLightSet->GetDmxFootprint());
	}
	m_tRDMDeviceInfo.dmx_start_address[0] = static_cast<uint8_t>(m_nDmxStartAddressFactoryDefault >> 8);
	m_tRDMDeviceInfo.dmx_start_address[1] = static_cast<uint8_t>(m_nDmxStartAddressFactoryDefault);
	m_tRDMDeviceInfo.current_personality = m_nCurrentPersonalityFactoryDefault;
	m_tRDMDeviceInfo.personality_count = m_pRDMPersonality == nullptr ? 0 : 1;
	m_tRDMDeviceInfo.sub_device_count[0] = static_cast<uint8_t>(nSubDevices >> 8);
	m_tRDMDeviceInfo.sub_device_count[1] = static_cast<uint8_t>(nSubDevices);
	m_tRDMDeviceInfo.sensor_count = m_RDMSensors.GetCount();

	memcpy(&m_tRDMSubDeviceInfo, &m_tRDMDeviceInfo, sizeof(struct TRDMDeviceInfo));

	m_nCheckSum = CalculateChecksum();

	DEBUG_EXIT
}
