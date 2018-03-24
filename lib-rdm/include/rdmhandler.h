/**
 * @file rdmhandler.h
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

#ifndef RDMHANDLER_H_
#define RDMHANDLER_H_

#include <stdint.h>

#include "rdmdeviceresponder.h"
#include "rdmmessage.h"
#include "rdmqueuedmessage.h"

enum TPowerState {
	POWER_STATE_FULL_OFF = 0x00, ///< Completely disengages power to device. Device can no longer respond.
	POWER_STATE_SHUTDOWN = 0x01, ///< Reduced power mode, may require device reset to return to normal operation. Device still responds to messages.
	POWER_STATE_STANDBY = 0x02,	///< Reduced power mode. Device can return to NORMAL without a reset. Device still responds to messages.
	POWER_STATE_NORMAL = 0xFF,	///< Normal Operating Mode.
};

class RDMHandler {
public:
	RDMHandler(RDMDeviceResponder *pRDMDeviceResponder);
	~RDMHandler(void);

	void HandleData(const uint8_t* pRdmDataIn, uint8_t *pRdmDataOut);

private:
	void Handlers(bool bIsBroadcast, uint8_t nCommandClass, uint16_t nParamId, uint8_t nParamDataLength, uint16_t nSubDevice);

	typedef struct {
		const uint16_t pid;
		void (RDMHandler::*pGetHandler)(uint16_t nSubDevice);
		void (RDMHandler::*pSetHandler)(bool IsBroadcast, uint16_t nSubDevice);
		const uint8_t nGetArgumentSize;
		const bool bIncludeInSupportedParams;
	} pid_definition;

	static const pid_definition PID_DEFINITIONS[];
	static const pid_definition PID_DEFINITIONS_SUB_DEVICES[];

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
	void GetIdentifyMode(uint16_t nSubDevice);
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
	void SetIdentifyMode(bool IsBroadcast, uint16_t nSubDevice);

private:
	void CreateRespondMessage(uint8_t nResponseType, uint16_t nReason = 0);
	void RespondMessageAck(void);
	void RespondMessageNack(uint16_t nReason);

private:
	void HandleString(const char *pString, uint8_t nLenght);

private:
	RDMDeviceResponder *m_pRDMDeviceResponder;
	RDMQueuedMessage  m_RDMQueuedMessage;
	bool m_IsMuted;
	uint8_t *m_pRdmDataIn;
	uint8_t *m_pRdmDataOut;
};

#endif /* RDMHANDLER_H_ */
