/**
 * @file rdmhandler.h
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

#ifndef RDMHANDLER_H_
#define RDMHANDLER_H_

#include <stdint.h>

#include "rdmmessage.h"
#include "rdmqueuedmessage.h"

class RDMHandler {
public:
	RDMHandler(bool bRDM = true);

	void HandleData(const uint8_t *pRdmDataIn, uint8_t *pRdmDataOut);

private:
	void Handlers(bool bIsBroadcast, uint8_t nCommandClass, uint16_t nParamId, uint8_t nParamDataLength, uint16_t nSubDevice);

	typedef struct {
		const uint16_t nPid;
		void (RDMHandler::*pGetHandler)(uint16_t nSubDevice);
		void (RDMHandler::*pSetHandler)(bool IsBroadcast, uint16_t nSubDevice);
		const uint8_t nGetArgumentSize;
		const bool bIncludeInSupportedParams;
		const bool bRDM;
		const bool bRDMNet;
	} TPidDefinition;

	static const TPidDefinition PID_DEFINITIONS[];
	static const TPidDefinition PID_DEFINITIONS_SUB_DEVICES[];

	// Get
	void GetQueuedMessage(uint16_t nSubDevice);
	void GetSupportedParameters(uint16_t nSubDevice);
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
	void GetIdentifyDevice(uint16_t nSubDevice);
	void GetRealTimeClock(uint16_t nSubDevice);
	void GetPowerState(uint16_t nSubDevice);
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
	void SetIdentifyDevice(bool IsBroadcast, uint16_t nSubDevice);
	void SetRealTimeClock(bool IsBroadcast, uint16_t nSubDevice);
	void SetResetDevice(bool IsBroadcast, uint16_t nSubDevice);
	void SetPowerState(bool IsBroadcast, uint16_t nSubDevice);
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
	void CreateRespondMessage(uint8_t nResponseType, uint16_t nReason = 0);
	void RespondMessageAck();
	void RespondMessageNack(uint16_t nReason);
	void HandleString(const char *pString, uint32_t nLenght);

private:
	bool m_bIsRDM;
	RDMQueuedMessage m_RDMQueuedMessage;
	bool m_IsMuted{false};
	uint8_t *m_pRdmDataIn{nullptr};
	uint8_t *m_pRdmDataOut{nullptr};
};

#endif /* RDMHANDLER_H_ */
