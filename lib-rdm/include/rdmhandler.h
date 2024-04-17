/**
 * @file rdmhandler.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMHANDLER_H_
#define RDMHANDLER_H_

#include <cstdint>
#include <cstring>

#if defined (NODE_RDMNET_LLRP_ONLY)
# if defined (CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
#  undef CONFIG_RDM_ENABLE_MANUFACTURER_PIDS
# endif
#endif

#if defined (ENABLE_RDM_QUEUED_MSG)
# include "rdmqueuedmessage.h"
#endif

#if defined (CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
# include "rdm_manufacturer_pid.h"
#endif

class RDMHandler {
public:
	RDMHandler(bool bRDM = true);

	void HandleData(const uint8_t *pRdmDataIn, uint8_t *pRdmDataOut);

private:
	void CreateRespondMessage(const uint8_t nResponseType, const uint16_t nReason);
	void RespondMessageAck();
	void RespondMessageNack(const uint16_t nReason);
	void HandleString(const char *pString, const uint32_t nLength);
	void Handlers(bool bIsBroadcast, uint8_t nCommandClass, uint16_t nParamId, uint8_t nParamDataLength, uint16_t nSubDevice);

	// Get
#if defined (ENABLE_RDM_QUEUED_MSG)
	void GetQueuedMessage(uint16_t nSubDevice);
#endif
	void GetSupportedParameters(uint16_t nSubDevice);
#if defined (CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
	void GetParameterDescription(uint16_t nSubDevice);
	void GetManufacturerPid(uint16_t nSubDevice);
#endif
	void GetDeviceInfo(uint16_t nSubDevice);
	void GetProductDetailIdList(uint16_t nSubDevice);
	void GetDeviceModelDescription(uint16_t nSubDevice);
	void GetManufacturerLabel(uint16_t nSubDevice);
	void GetDeviceLabel(uint16_t nSubDevice);
	void GetFactoryDefaults(uint16_t nSubDevice);
	void GetLanguage(uint16_t nSubDevice);
	void GetSoftwareVersionLabel(uint16_t nSubDevice);
	void GetBootSoftwareVersionId(uint16_t nSubDevice);
	void GetBootSoftwareVersionLabel(uint16_t nSubDevice);
	void GetPersonality(uint16_t nSubDevice);
	void GetPersonalityDescription(uint16_t nSubDevice);
	void GetDmxStartAddress(uint16_t nSubDevice);
	void GetSlotInfo(uint16_t nSubDevice);
	void GetSlotDescription(uint16_t nSubDevice);
	void GetSensorDefinition(uint16_t nSubDevice);
	void GetSensorValue(uint16_t nSubDevice);
	void GetDeviceHours(uint16_t nSubDevice);
	void GetDisplayInvert(uint16_t nSubDevice);
	void GetDisplayLevel(uint16_t nSubDevice);
	void GetIdentifyDevice(uint16_t nSubDevice);
	void GetRealTimeClock(uint16_t nSubDevice);
	void GetPowerState(uint16_t nSubDevice);
#if defined (CONFIG_RDM_ENABLE_SELF_TEST)
	void GetPerformSelfTest(uint16_t nSubDevice);
	void GetSelfTestDescription(uint16_t nSubDevice);
#endif
#if defined (ENABLE_RDM_PRESET_PLAYBACK)
	void GetPresetPlayback(uint16_t nSubDevice);
#endif
	// ANSI E1.37-1
	void GetIdentifyMode(uint16_t nSubDevice);
	// ANSI E1.37-2 – 2015
	void GetInterfaceList(uint16_t nSubDevice);
	void GetInterfaceName(uint16_t nSubDevice);
	void GetHardwareAddress(uint16_t nSubDevice);
	void GetDHCPMode(uint16_t nSubDevice);
	void GetZeroconf(uint16_t nSubDevice);
	void GetAddressNetmask(uint16_t nSubDevice);
	void GetStaticAddress(uint16_t nSubDevice);
	void GetDefaultRoute(uint16_t nSubDevice);
	void GetNameServers(uint16_t nSubDevice);
	void GetHostName(uint16_t nSubDevice);
	void GetDomainName(uint16_t nSubDevice);
	// Set
	void SetDeviceLabel(bool IsBroadcast, uint16_t nSubDevice);
	void SetFactoryDefaults(bool IsBroadcast, uint16_t nSubDevice);
	void SetLanguage(bool IsBroadcast, uint16_t nSubDevice);
	void SetPersonality(bool IsBroadcast, uint16_t nSubDevice);
	void SetDmxStartAddress(bool IsBroadcast, uint16_t nSubDevice);
	void SetSensorValue(bool IsBroadcast, uint16_t nSubDevice);
	void SetRecordSensors(bool IsBroadcast, uint16_t nSubDevice);
	void SetDeviceHours(bool IsBroadcast, uint16_t nSubDevice);
	void SetDisplayInvert(bool IsBroadcast, uint16_t nSubDevice);
	void SetDisplayLevel(bool IsBroadcast, uint16_t nSubDevice);
	void SetIdentifyDevice(bool IsBroadcast, uint16_t nSubDevice);
	void SetRealTimeClock(bool IsBroadcast, uint16_t nSubDevice);
	void SetResetDevice(bool IsBroadcast, uint16_t nSubDevice);
	void SetPowerState(bool IsBroadcast, uint16_t nSubDevice);
#if defined (CONFIG_RDM_ENABLE_SELF_TEST)
	void SetPerformSelfTest(bool IsBroadcast, uint16_t nSubDevice);
#endif
#if defined (ENABLE_RDM_PRESET_PLAYBACK)
	void SetPresetPlayback(bool IsBroadcast, uint16_t nSubDevice);
#endif
#if defined (CONFIG_RDM_ENABLE_MANUFACTURER_PIDS) && defined (CONFIG_RDM_MANUFACTURER_PIDS_SET)
	void SetManufacturerPid(bool IsBroadcast, uint16_t nSubDevice);
#endif
	// ANSI E1.37-1
	void SetIdentifyMode(bool IsBroadcast, uint16_t nSubDevice);
	// ANSI E1.37-2 – 2015
	void SetDHCPMode(bool IsBroadcast, uint16_t nSubDevice);
	void SetZeroconf(bool IsBroadcast, uint16_t nSubDevice);
	void RenewDhcp(bool IsBroadcast, uint16_t nSubDevice);
	void SetStaticAddress(bool IsBroadcast, uint16_t nSubDevice);
	void SetDefaultRoute(bool IsBroadcast, uint16_t nSubDevice);
	void ApplyConfiguration(bool IsBroadcast, uint16_t nSubDevice);
	void SetHostName(bool IsBroadcast, uint16_t nSubDevice);
	void SetDomainName(bool IsBroadcast, uint16_t nSubDevice);
	// ANSI E1.37-2 – 2015
	bool CheckInterfaceID(const struct TRdmMessageNoSc *pRdmDataIn);

private:
	bool m_bIsRDM;
	bool m_IsMuted { false };
	uint8_t *m_pRdmDataIn { nullptr };
	uint8_t *m_pRdmDataOut { nullptr };
#if defined (ENABLE_RDM_QUEUED_MSG)
	RDMQueuedMessage m_RDMQueuedMessage;
#endif

	struct PidDefinition {
		const uint16_t nPid;
		void (RDMHandler::*pGetHandler)(uint16_t nSubDevice);
		void (RDMHandler::*pSetHandler)(bool IsBroadcast, uint16_t nSubDevice);
		const uint8_t nGetArgumentSize;
		const bool bIncludeInSupportedParams;
		const bool bRDM;
		const bool bRDMNet;
	} ;

	static const PidDefinition PID_DEFINITIONS[];
	static const PidDefinition PID_DEFINITIONS_SUB_DEVICES[];
#if defined (CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
	static const PidDefinition PID_DEFINITION_MANUFACTURER_GENERAL;
	static const rdm::ParameterDescription PARAMETER_DESCRIPTIONS[];

	uint32_t GetParameterDescriptionCount() const;
	void CopyParameterDescription(const uint32_t nIndex, uint8_t *pParamData) {
		const auto nSize = sizeof(struct rdm::ParameterDescription) - sizeof(const char *) - sizeof(const uint8_t);
		memcpy(pParamData, &PARAMETER_DESCRIPTIONS[nIndex], nSize);
		memcpy(&pParamData[nSize], PARAMETER_DESCRIPTIONS[nIndex].description, PARAMETER_DESCRIPTIONS[nIndex].pdl - nSize);
	}
#endif
};

#endif /* RDMHANDLER_H_ */
