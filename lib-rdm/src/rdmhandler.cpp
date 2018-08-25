/**
 * @file rdmhandler.cpp
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
#include <stdio.h>
#include <assert.h>

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

#ifndef MAX
 #define MAX(a,b)	(((a) > (b)) ? (a) : (b))
 #define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#include "rdmhandler.h"

#include "rdmidentify.h"
#include "rdmsensor.h"
#include "rdmslotinfo.h"
#include "rdmmessage.h"

#include "rdm.h"
#include "rdm_e120.h"

#include "hardware.h"

#include "debug.h"

enum {
	DMX_UNIVERSE_SIZE = 512
};

void RDMHandler::HandleString(const char *pString, uint8_t nLength) {
	struct TRdmMessage *RdmMessage = (struct TRdmMessage *) m_pRdmDataOut;

	RdmMessage->param_data_length = nLength;

	for (unsigned i = 0; i < (unsigned) nLength; i++) {
		RdmMessage->param_data[i] = (uint8_t) pString[i];
	}
}

void RDMHandler::CreateRespondMessage(uint8_t nResponseType, uint16_t nReason) {
	DEBUG1_ENTRY
	unsigned i;

	struct TRdmMessageNoSc *pRdmDataIn = (struct TRdmMessageNoSc *)m_pRdmDataIn;

#ifndef NDEBUG
	uint8_t dummy[512];
	dummy[0] = E120_SC_RDM;
	memcpy(&dummy[1], m_pRdmDataIn, pRdmDataIn->message_length);
	RDMMessage::Print(dummy);
#endif

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *) m_pRdmDataOut;

	pRdmDataOut->start_code = E120_SC_RDM;
	pRdmDataOut->sub_start_code = pRdmDataIn->sub_start_code;
	pRdmDataOut->transaction_number = pRdmDataIn->transaction_number;
	pRdmDataOut->message_count = 0;//rdm_queued_message_get_count(); //FIXME rdm_queued_message_get_count
	pRdmDataOut->sub_device[0] = pRdmDataIn->sub_device[0];
	pRdmDataOut->sub_device[1] = pRdmDataIn->sub_device[1];
	pRdmDataOut->command_class = pRdmDataIn->command_class + 1;
	pRdmDataOut->param_id[0] = pRdmDataIn->param_id[0];
	pRdmDataOut->param_id[1] = pRdmDataIn->param_id[1];

	switch (nResponseType) {
	case E120_RESPONSE_TYPE_ACK:
		pRdmDataOut->message_length = RDM_MESSAGE_MINIMUM_SIZE + pRdmDataOut->param_data_length;
		pRdmDataOut->slot16.response_type = E120_RESPONSE_TYPE_ACK;
		break;
	case E120_RESPONSE_TYPE_NACK_REASON:
	case E120_RESPONSE_TYPE_ACK_TIMER:
		pRdmDataOut->message_length = RDM_MESSAGE_MINIMUM_SIZE + 2;
		pRdmDataOut->slot16.response_type = nResponseType;
		pRdmDataOut->param_data_length = 2;
		pRdmDataOut->param_data[0] = (uint8_t) (nReason >> 8);
		pRdmDataOut->param_data[1] = (uint8_t) nReason;
		break;
	default:
		// forces timeout
		return;
		// Unreachable code: break;
	}

	const uint8_t *uid_device = (uint8_t *)m_pRDMDeviceResponder->GetUID();

	for (i = 0; i < (unsigned) RDM_UID_SIZE; i++) {
		pRdmDataOut->destination_uid[i] = pRdmDataIn->source_uid[i];
		pRdmDataOut->source_uid[i] = uid_device[i];
	}

	uint16_t rdm_checksum = 0;

	for (i = 0; i < (unsigned) pRdmDataOut->message_length; i++) {
		rdm_checksum += m_pRdmDataOut[i];
	}

	m_pRdmDataOut[i++] = rdm_checksum >> 8;
	m_pRdmDataOut[i] = rdm_checksum & 0XFF;

	DEBUG1_EXIT
}

void RDMHandler::RespondMessageAck(void) {
	struct TRdmMessageNoSc *pRdmDataIn = (struct TRdmMessageNoSc *)m_pRdmDataIn;

	if(pRdmDataIn->param_data_length == 0) {
		pRdmDataIn->message_length = RDM_MESSAGE_MINIMUM_SIZE;
	}

	CreateRespondMessage(E120_RESPONSE_TYPE_ACK);
}

void RDMHandler::RespondMessageNack(uint16_t nReason) {
	CreateRespondMessage(E120_RESPONSE_TYPE_NACK_REASON, nReason);
}

const RDMHandler::pid_definition RDMHandler::PID_DEFINITIONS[] {
	{E120_QUEUED_MESSAGE,              	&RDMHandler::GetQueuedMessage,           	0,                   				1, true },
	{E120_SUPPORTED_PARAMETERS,        	&RDMHandler::GetSupportedParameters,      	0,             						0, false},
	{E120_DEVICE_INFO,                	&RDMHandler::GetDeviceInfo,               	0,                					0, false},
	{E120_PRODUCT_DETAIL_ID_LIST, 	   	&RDMHandler::GetProductDetailIdList,     	0,									0, true },
	{E120_DEVICE_MODEL_DESCRIPTION,    	&RDMHandler::GetDeviceModelDescription,		0,                 					0, true },
	{E120_MANUFACTURER_LABEL,          	&RDMHandler::GetManufacturerLabel,         	0,                        			0, true },
	{E120_DEVICE_LABEL,                	&RDMHandler::GetDeviceLabel,               	&RDMHandler::SetDeviceLabel,		0, true },
	{E120_FACTORY_DEFAULTS,            	&RDMHandler::GetFactoryDefaults,          	&RDMHandler::SetFactoryDefaults,	0, true },
	{E120_LANGUAGE_CAPABILITIES,       	&RDMHandler::GetLanguage,			        0,                 					0, true },
	{E120_LANGUAGE,						&RDMHandler::GetLanguage,			        &RDMHandler::SetLanguage,           0, true },
	{E120_SOFTWARE_VERSION_LABEL,		&RDMHandler::GetSoftwareVersionLabel,   	0,                  				0, false},
	{E120_BOOT_SOFTWARE_VERSION_ID,		&RDMHandler::GetBootSoftwareVersionId, 		0,                 					0, true },
	{E120_BOOT_SOFTWARE_VERSION_LABEL,	&RDMHandler::GetBootSoftwareVersionLabel,	0,                   				0, true },
	{E120_DMX_PERSONALITY,		      	&RDMHandler::GetPersonality,               	&RDMHandler::SetPersonality,        0, true },
	{E120_DMX_PERSONALITY_DESCRIPTION,	&RDMHandler::GetPersonalityDescription,    	0,                 					1, true },
	{E120_DMX_START_ADDRESS,           	&RDMHandler::GetDmxStartAddress,          	&RDMHandler::SetDmxStartAddress,	0, false},
	{E120_SLOT_INFO,					&RDMHandler::GetSlotInfo,					0,									0, true },
	{E120_SLOT_DESCRIPTION,				&RDMHandler::GetSlotDescription,			0,									2, true },
	{E120_SENSOR_DEFINITION,		   	&RDMHandler::GetSensorDefinition,			0,									1, true },
	{E120_SENSOR_VALUE,				   	&RDMHandler::GetSensorValue,				&RDMHandler::SetSensorValue,		1, true },
	{E120_RECORD_SENSORS,			   	0,											&RDMHandler::SetRecordSensors,	 	0, true },
	{E120_DEVICE_HOURS,                	&RDMHandler::GetDeviceHours,    	      	&RDMHandler::SetDeviceHours,       	0, true },
	{E120_REAL_TIME_CLOCK,		       	&RDMHandler::GetRealTimeClock,  			&RDMHandler::SetRealTimeClock,    	0, true },
	{E120_IDENTIFY_DEVICE,		       	&RDMHandler::GetIdentifyDevice,		    	&RDMHandler::SetIdentifyDevice,    	0, false},
	{E120_RESET_DEVICE,			    	0,                                			&RDMHandler::SetResetDevice,       	0, true },
	{E120_POWER_STATE,					&RDMHandler::GetPowerState,					&RDMHandler::SetPowerState,			0, true },
	{E137_1_IDENTIFY_MODE,			   	&RDMHandler::GetIdentifyMode,				&RDMHandler::SetIdentifyMode,		0, true }
};

const RDMHandler::pid_definition RDMHandler::PID_DEFINITIONS_SUB_DEVICES[] {
	{E120_SUPPORTED_PARAMETERS,        &RDMHandler::GetSupportedParameters,			0,                       			0, true },
	{E120_DEVICE_INFO,                 &RDMHandler::GetDeviceInfo,					0,                        			0, true },
	{E120_PRODUCT_DETAIL_ID_LIST, 	   &RDMHandler::GetProductDetailIdList,			0,						 			0, true },
	{E120_SOFTWARE_VERSION_LABEL,      &RDMHandler::GetSoftwareVersionLabel,		0,                       			0, true },
	{E120_DMX_PERSONALITY,		       &RDMHandler::GetPersonality,            		&RDMHandler::SetPersonality,        0, true },
	{E120_DMX_PERSONALITY_DESCRIPTION, &RDMHandler::GetPersonalityDescription,		0,                        			1, true },
	{E120_DMX_START_ADDRESS,           &RDMHandler::GetDmxStartAddress,          	&RDMHandler::SetDmxStartAddress,	0, true },
	{E120_IDENTIFY_DEVICE,		       &RDMHandler::GetIdentifyDevice,		    	&RDMHandler::SetIdentifyDevice,		0, true }
};

RDMHandler::RDMHandler(RDMDeviceResponder *pRDMDeviceResponder): m_pRDMDeviceResponder(pRDMDeviceResponder), m_IsMuted(false), m_pRdmDataIn(0), m_pRdmDataOut(0) {
}

RDMHandler::~RDMHandler(void) {
}

/**
 *
 * @param pRdmDataIn RDM with no Start Code
 * @param pRdmDataOut RDM with the Start Code or it is Discover Message
 */
void RDMHandler::HandleData(const uint8_t* pRdmDataIn, uint8_t *pRdmDataOut) {
	DEBUG_ENTRY

	assert(pRdmDataIn != 0);
	assert(pRdmDataOut != 0);

	pRdmDataOut[0] = 0xFF; // Invalidate outgoing message;

	m_pRdmDataIn = (uint8_t* )pRdmDataIn;
	m_pRdmDataOut = (uint8_t* )pRdmDataOut;

	bool bIsRdmPacketForMe = false;
	bool bIsRdmPacketBroadcast = false;
	bool bIsRdmPacketVendorcast = false;

	struct TRdmMessageNoSc *pRdmRequest = (struct TRdmMessageNoSc *) pRdmDataIn;

	const uint8_t command_class = pRdmRequest->command_class;
	const uint16_t param_id = (pRdmRequest->param_id[0] << 8) + pRdmRequest->param_id[1];
	const uint8_t *uid_device = m_pRDMDeviceResponder->GetUID();

	DEBUG_PRINTF("Command class [%.2X]:%d, param_id [%.2x%.2x]:%d, tn:%d", command_class, command_class, pRdmRequest->param_id[0], pRdmRequest->param_id[1], param_id, pRdmRequest->transaction_number);

	if (memcmp(pRdmRequest->destination_uid, UID_ALL, RDM_UID_SIZE) == 0) {
		bIsRdmPacketBroadcast = true;
	}

	if ((memcmp(pRdmRequest->destination_uid, uid_device, 2) == 0) && (memcmp(&pRdmRequest->destination_uid[2], UID_ALL, 4) == 0)) {
		bIsRdmPacketVendorcast = true;
		bIsRdmPacketForMe = true;
	} else if (memcmp(pRdmRequest->destination_uid, uid_device, RDM_UID_SIZE) == 0) {
		bIsRdmPacketForMe = true;
	}

	if ((!bIsRdmPacketForMe) && (!bIsRdmPacketBroadcast)) {
		// Ignore RDM packet
	} else if (command_class == E120_DISCOVERY_COMMAND) {

		if (param_id == E120_DISC_UNIQUE_BRANCH) {

			if (!m_IsMuted) {

				if ((memcmp(pRdmRequest->param_data, uid_device, RDM_UID_SIZE) <= 0) && (memcmp(uid_device, pRdmRequest->param_data + 6, RDM_UID_SIZE) <= 0)) {

					DEBUG_PUTS("E120_DISC_UNIQUE_BRANCH");

					struct TRdmDiscoveryMsg *p = (struct TRdmDiscoveryMsg *)pRdmDataOut;
					uint16_t rdm_checksum = 6 * 0xFF;

					uint8_t i = 0;
					for (i = 0; i < 7; i++) {
						p->header_FE[i] = 0xFE;
					}
					p->header_AA = 0xAA;

					for (i = 0; i < 6; i++) {
						p->masked_device_id[i + i] = uid_device[i] | 0xAA;
						p->masked_device_id[i + i + 1] = uid_device[i] | 0x55;
						rdm_checksum += uid_device[i];
					}

					p->checksum[0] = (rdm_checksum >> 8) | 0xAA;
					p->checksum[1] = (rdm_checksum >> 8) | 0x55;
					p->checksum[2] = (rdm_checksum & 0xFF) | 0xAA;
					p->checksum[3] = (rdm_checksum & 0xFF) | 0x55;

					return;
				}
			}
		} else if (param_id == E120_DISC_UN_MUTE) {

			DEBUG_PUTS("E120_DISC_UN_MUTE\n");

			if (pRdmRequest->param_data_length != 0) {
				/* The response RESPONSE_TYPE_NACK_REASON shall only be used in conjunction
				 * with the Command Classes GET_COMMAND_RESPONSE & SET_COMMAND_RESPONSE.
				 */
				return;
			}

			m_IsMuted = false;

			if (!bIsRdmPacketBroadcast && bIsRdmPacketForMe) {
				struct TRdmMessage *pRdmResponse = (struct TRdmMessage *) m_pRdmDataOut;

				pRdmResponse->param_data_length = 2;
				pRdmResponse->param_data[0] = 0x00;	// Control Field
				pRdmResponse->param_data[1] = 0x00;	// Control Field

				if (m_pRDMDeviceResponder->GetSubDeviceCount() != 0) {
					pRdmResponse->param_data[1] |= RDM_CONTROL_FIELD_SUB_DEVICE_FLAG;
				}

				RespondMessageAck();
				return;
			}
		} else if (param_id == E120_DISC_MUTE) {

			DEBUG_PUTS("E120_DISC_MUTE");

			if (pRdmRequest->param_data_length != 0) {
				/* The response RESPONSE_TYPE_NACK_REASON shall only be used in conjunction
				 * with the Command Classes GET_COMMAND_RESPONSE & SET_COMMAND_RESPONSE.
				 */
				return;
			}

			m_IsMuted = true;

			if (bIsRdmPacketForMe) {
				struct TRdmMessage *pRdmResponse = (struct TRdmMessage *) m_pRdmDataOut;

				pRdmResponse->param_data_length = 2;
				pRdmResponse->param_data[0] = 0x00;	// Control Field
				pRdmResponse->param_data[1] = 0x00;	// Control Field

				if (m_pRDMDeviceResponder->GetSubDeviceCount() != 0) {
					pRdmResponse->param_data[1] |= RDM_CONTROL_FIELD_SUB_DEVICE_FLAG;
				}

				RespondMessageAck();
				return;
			}
		}
	} else {
		uint16_t sub_device = (pRdmRequest->sub_device[0] << 8) + pRdmRequest->sub_device[1];
		Handlers(bIsRdmPacketBroadcast || bIsRdmPacketVendorcast, command_class, param_id, pRdmRequest->param_data_length, sub_device);
	}

	DEBUG_EXIT
}

void RDMHandler::Handlers(bool bIsBroadcast, uint8_t nCommandClass, uint16_t nParamId, uint8_t nParamDataLength, uint16_t nSubDevice) {
	DEBUG1_ENTRY

	pid_definition const *pid_handler = 0;

	if (nCommandClass != E120_GET_COMMAND && nCommandClass != E120_SET_COMMAND) {
		RespondMessageNack(E120_NR_UNSUPPORTED_COMMAND_CLASS);
		return;
	}

	const uint16_t sub_device_count = m_pRDMDeviceResponder->GetSubDeviceCount();

	if ((nSubDevice > sub_device_count) && (nSubDevice != E120_SUB_DEVICE_ALL_CALL)) {
		RespondMessageNack(E120_NR_SUB_DEVICE_OUT_OF_RANGE);
		return;
	}

	uint8_t i;
	for (i = 0; i < sizeof(PID_DEFINITIONS) / sizeof(PID_DEFINITIONS[0]); ++i) {
		if (PID_DEFINITIONS[i].pid == nParamId)
			pid_handler = &PID_DEFINITIONS[i];
	}

	if (!pid_handler) {
		RespondMessageNack(E120_NR_UNKNOWN_PID);
		DEBUG1_EXIT
		return;
	}

	if (nCommandClass == E120_GET_COMMAND) {
		if (bIsBroadcast) {
			DEBUG1_EXIT
			return;
		}

		if (nSubDevice == E120_SUB_DEVICE_ALL_CALL) {
			RespondMessageNack(E120_NR_SUB_DEVICE_OUT_OF_RANGE);
			DEBUG1_EXIT
			return;
		}

		if (!pid_handler->pGetHandler) {
			RespondMessageNack(E120_NR_UNSUPPORTED_COMMAND_CLASS);
			DEBUG1_EXIT
			return;
		}

		if (nParamDataLength != pid_handler->nGetArgumentSize) {
			RespondMessageNack(E120_NR_FORMAT_ERROR);
			DEBUG1_EXIT
			return;
		}

		(this->*(pid_handler->pGetHandler))(nSubDevice);
	} else {

		if (!pid_handler->pSetHandler) {
			RespondMessageNack(E120_NR_UNSUPPORTED_COMMAND_CLASS);
			return;
		}

		(this->*(pid_handler->pSetHandler))(bIsBroadcast, nSubDevice);
	}

	DEBUG1_EXIT
}

void RDMHandler::GetQueuedMessage(/*@unused@*/uint16_t nSubDevice) {
	m_RDMQueuedMessage.Handler(m_pRdmDataOut);
	RespondMessageAck();
}

void RDMHandler::GetSupportedParameters(uint16_t nSubDevice) {
	uint8_t nSupportedParams = 0;
	pid_definition *pPidDefinitions;
	int nTableSize = 0;
	int i,j;

	if (nSubDevice != 0) {
		pPidDefinitions = (pid_definition *)&PID_DEFINITIONS_SUB_DEVICES[0];
		nTableSize = (int) (sizeof(PID_DEFINITIONS_SUB_DEVICES) / sizeof(PID_DEFINITIONS_SUB_DEVICES[0]));
	} else {
		pPidDefinitions = (pid_definition *)&PID_DEFINITIONS[0];
		nTableSize = (int) (sizeof(PID_DEFINITIONS) / sizeof(PID_DEFINITIONS[0]));
	}

	for (i = 0; i < nTableSize; i++) {
		if (pPidDefinitions[i].bIncludeInSupportedParams)
			nSupportedParams++;
	}

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;

	pRdmDataOut->param_data_length = (2 * nSupportedParams);

	j = 0;
	for (i = 0;	i < nTableSize; i++)
	{
		if (pPidDefinitions[i].bIncludeInSupportedParams)
		{
			pRdmDataOut->param_data[j+j] = (uint8_t)(pPidDefinitions[i].pid >> 8);
			pRdmDataOut->param_data[j+j+1] = (uint8_t)pPidDefinitions[i].pid;
			j++;
		}
	}

	RespondMessageAck();
}

void RDMHandler::GetDeviceInfo(uint16_t nSubDevice) {
	const struct TRDMDeviceInfo *pRdmDeviceInfoRequested = m_pRDMDeviceResponder->GetDeviceInfo(nSubDevice);

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;
	struct TRDMDeviceInfo *pDeviceInfoOut = (struct TRDMDeviceInfo *)pRdmDataOut->param_data;

	pRdmDataOut->param_data_length = sizeof(struct TRDMDeviceInfo);
	memcpy(pDeviceInfoOut, pRdmDeviceInfoRequested, sizeof(struct TRDMDeviceInfo));

	RespondMessageAck();
}

void RDMHandler::GetProductDetailIdList(uint16_t nSubDevice) {
	const uint16_t product_detail = m_pRDMDeviceResponder->GetProductDetail();

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;

	pRdmDataOut->param_data_length = 2;
	pRdmDataOut->param_data[0] = (uint8_t) (product_detail >> 8);
	pRdmDataOut->param_data[1] = (uint8_t) (product_detail & 0xFF);

	RespondMessageAck();
}

void RDMHandler::GetDeviceModelDescription(uint16_t nSubDevice) {
	uint8_t nBoardNameLength;
	const char *pBoardModel = Hardware::Get()->GetBoardName(nBoardNameLength);

	HandleString(pBoardModel, nBoardNameLength);
	RespondMessageAck();
}

void RDMHandler::GetManufacturerLabel(uint16_t nSubDevice) {
	struct TRDMDeviceInfoData label;

	m_pRDMDeviceResponder->GetManufacturerName(&label);

	HandleString((char *)label.data, label.length);
	RespondMessageAck();
}

void RDMHandler::GetDeviceLabel(uint16_t nSubDevice) {
	struct TRDMDeviceInfoData label;

	m_pRDMDeviceResponder->GetLabel(nSubDevice, &label);

	HandleString((const char *)label.data, label.length);
	RespondMessageAck();
}

void RDMHandler::SetDeviceLabel(bool IsBroadcast, uint16_t nSubDevice) {
	DEBUG2_ENTRY
	struct TRdmMessageNoSc *pRdmDataIn = (struct TRdmMessageNoSc *)m_pRdmDataIn;
	uint8_t device_label_length;
	uint8_t *device_label;

	if (pRdmDataIn->param_data_length > RDM_DEVICE_LABEL_MAX_LENGTH) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	device_label_length = pRdmDataIn->param_data_length;
	device_label= &pRdmDataIn->param_data[0];

	m_pRDMDeviceResponder->SetLabel(nSubDevice, device_label, device_label_length);

	if(IsBroadcast) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;
	pRdmDataOut->param_data_length = 0;
	RespondMessageAck();
}

void RDMHandler::GetFactoryDefaults(uint16_t nSubDevice) {
	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;

	pRdmDataOut->param_data_length = 1;
	pRdmDataOut->param_data[0] = m_pRDMDeviceResponder->GetFactoryDefaults() ? (uint8_t) 1 : (uint8_t) 0;

	RespondMessageAck();
}

void RDMHandler::SetFactoryDefaults(bool IsBroadcast, uint16_t nSubDevice) {
	struct TRdmMessageNoSc *rdm_command = (struct TRdmMessageNoSc *)m_pRdmDataIn;

	if (rdm_command->param_data_length != 0) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	m_pRDMDeviceResponder->SetFactoryDefaults();

	if(IsBroadcast) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *) m_pRdmDataOut;
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

void RDMHandler::GetLanguage(uint16_t nSubDevice) {
	const char *pLanguage = m_pRDMDeviceResponder->GetLanguage();

	HandleString(pLanguage, RDM_DEVICE_SUPPORTED_LANGUAGE_LENGTH);
	RespondMessageAck();
}

void RDMHandler::SetLanguage(bool IsBroadcast, uint16_t nSubDevice) {
	struct TRdmMessageNoSc *pRdmDataIn = (struct TRdmMessageNoSc *) m_pRdmDataIn;

	if (pRdmDataIn->param_data_length != RDM_DEVICE_SUPPORTED_LANGUAGE_LENGTH) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const char *pSupportedLanguage = m_pRDMDeviceResponder->GetLanguage();

	if ((pRdmDataIn->param_data[0] != pSupportedLanguage[0]) || (pRdmDataIn->param_data[1] != pSupportedLanguage[1])) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	if (IsBroadcast) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *) m_pRdmDataOut;
	pRdmDataOut->param_data_length = (uint8_t) 0;

	RespondMessageAck();
}

void RDMHandler::GetSoftwareVersionLabel(uint16_t nSubDevice) {
	const char *software_version = m_pRDMDeviceResponder->GetSoftwareVersion();
	const uint8_t software_version_length = m_pRDMDeviceResponder->GetSoftwareVersionLength();

	HandleString(software_version, software_version_length);
	RespondMessageAck();
}

void RDMHandler::GetBootSoftwareVersionId(uint16_t nSubDevice) {
	struct TRdmMessageNoSc *pRdmDataIn = (struct TRdmMessageNoSc *) m_pRdmDataIn;

	if (pRdmDataIn->param_data_length != 0) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	uint32_t boot_software_version_id = Hardware::Get()->GetReleaseId();

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *) m_pRdmDataOut;

	pRdmDataOut->param_data_length = RDM_BOOT_SOFTWARE_VERSION_ID_LENGTH;
	pRdmDataOut->param_data[0] = (uint8_t) (boot_software_version_id >> 24);
	pRdmDataOut->param_data[1] = (uint8_t) (boot_software_version_id >> 16);
	pRdmDataOut->param_data[2] = (uint8_t) (boot_software_version_id >> 8);
	pRdmDataOut->param_data[3] = (uint8_t) boot_software_version_id;

	RespondMessageAck();

}

void RDMHandler::GetBootSoftwareVersionLabel(uint16_t nSubDevice) {
	uint8_t nVersionLength;
	const char *pVersion = Hardware::Get()->GetSysName(nVersionLength);

	HandleString(pVersion, MIN(RDM_BOOT_SOFTWARE_VERSION_LABEL_MAX_LENGTH, nVersionLength));
	RespondMessageAck();
}

void RDMHandler::GetPersonality(uint16_t nSubDevice) {
	const uint8_t nCurrent = m_pRDMDeviceResponder->GetPersonalityCurrent(nSubDevice);
	const uint8_t nCount = m_pRDMDeviceResponder->GetPersonalityCount(nSubDevice);

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;

	pRdmDataOut->param_data_length = 2;
	pRdmDataOut->param_data[0] = nCurrent;
	pRdmDataOut->param_data[1] = nCount;

	RespondMessageAck();
}

void RDMHandler::SetPersonality(bool IsBroadcast, uint16_t nSubDevice) {
	struct TRdmMessageNoSc *pRdmDataIn = (struct TRdmMessageNoSc *)m_pRdmDataIn;

	if (pRdmDataIn->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const uint8_t personality = pRdmDataIn->param_data[0];
	const uint8_t max_personalities = m_pRDMDeviceResponder->GetPersonalityCount(nSubDevice);

	if ((personality == 0) || (personality > max_personalities)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	m_pRDMDeviceResponder->SetPersonalityCurrent(nSubDevice, personality);

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

void RDMHandler::GetPersonalityDescription(uint16_t nSubDevice) {
	struct TRdmMessageNoSc *rdm_command = (struct TRdmMessageNoSc *)m_pRdmDataIn;
	const uint8_t nPersonality = rdm_command->param_data[0];
	const uint8_t nMaxPersonalities = m_pRDMDeviceResponder->GetPersonalityCount(nSubDevice);

	if ((nPersonality == 0) || (nPersonality > nMaxPersonalities)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	const uint16_t nSlots = m_pRDMDeviceResponder->GetPersonality(nSubDevice, nPersonality)->GetSlots();

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;

	pRdmDataOut->param_data[0] = nPersonality;
	pRdmDataOut->param_data[1] = (uint8_t)(nSlots >> 8);
	pRdmDataOut->param_data[2] = (uint8_t)(nSlots);

	char *dst = (char *)&pRdmDataOut->param_data[3];
	uint8_t nLength = RDM_PERSONALITY_DESCRIPTION_MAX_LENGTH;

	m_pRDMDeviceResponder->GetPersonality(nSubDevice, nPersonality)->CopyTo(dst, nLength);

	pRdmDataOut->param_data_length = 3 + nLength;

	RespondMessageAck();
}

void RDMHandler::GetDmxStartAddress(uint16_t nSubDevice) {
	const uint16_t dmx_start_address = m_pRDMDeviceResponder->GetDmxStartAddress(nSubDevice);

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;

	pRdmDataOut->param_data_length = 2;
	pRdmDataOut->param_data[0] = (uint8_t)(dmx_start_address >> 8);
	pRdmDataOut->param_data[1] = (uint8_t)dmx_start_address;

	RespondMessageAck();
}

void RDMHandler::SetDmxStartAddress(bool IsBroadcast, uint16_t nSubDevice) {
	DEBUG2_ENTRY

	struct TRdmMessageNoSc *pRdmDataIn = (struct TRdmMessageNoSc *) m_pRdmDataIn;

	if (pRdmDataIn->param_data_length != 2) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const uint16_t nDmxStartAddress = (pRdmDataIn->param_data[0] << 8) + pRdmDataIn->param_data[1];

	if ((nDmxStartAddress == 0) || (nDmxStartAddress > DMX_UNIVERSE_SIZE)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	m_pRDMDeviceResponder->SetDmxStartAddress(nSubDevice, nDmxStartAddress);

	if(IsBroadcast) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *) m_pRdmDataOut;
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

void RDMHandler::GetSensorDefinition(uint16_t nSubDevice) {
	struct TRdmMessageNoSc *rdm_command = (struct TRdmMessageNoSc *)m_pRdmDataIn;

	const uint8_t nSensorRequested = rdm_command->param_data[0];

	if ((nSensorRequested == RDM_SENSORS_ALL) || (nSensorRequested + 1 > m_pRDMDeviceResponder->GetSensorCount())) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;
	const struct TRDMSensorDefintion *sensor_definition = m_pRDMDeviceResponder->GetSensorDefinition(nSensorRequested);

	if (nSensorRequested != sensor_definition->sensor) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	pRdmDataOut->param_data[0] = nSensorRequested;
	pRdmDataOut->param_data[1] = sensor_definition->type;
	pRdmDataOut->param_data[2] = sensor_definition->unit;
	pRdmDataOut->param_data[3] = sensor_definition->prefix;
	pRdmDataOut->param_data[4] = (uint8_t)(sensor_definition->range_min >> 8);
	pRdmDataOut->param_data[5] = (uint8_t)(sensor_definition->range_min);
	pRdmDataOut->param_data[6] = (uint8_t)(sensor_definition->range_max >> 8);
	pRdmDataOut->param_data[7] = (uint8_t)(sensor_definition->range_max);
	pRdmDataOut->param_data[8] = (uint8_t)(sensor_definition->normal_min >> 8);
	pRdmDataOut->param_data[9] = (uint8_t)(sensor_definition->normal_min);
	pRdmDataOut->param_data[10] = (uint8_t)(sensor_definition->normal_max >> 8);
	pRdmDataOut->param_data[11] = (uint8_t)(sensor_definition->normal_max);
	pRdmDataOut->param_data[12] = sensor_definition->recorded_supported;

	int i = 13;
	int j = 0;
	for (j = 0; j < sensor_definition->len; j++) {
		pRdmDataOut->param_data[i++] = sensor_definition->description[j];
	}

	pRdmDataOut->param_data_length = i;

	RespondMessageAck();
}

void RDMHandler::GetSensorValue(uint16_t nSubDevice) {
	struct TRdmMessageNoSc *rdm_command = (struct TRdmMessageNoSc *)m_pRdmDataIn;

	if (rdm_command->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const uint8_t nSensorRequested = rdm_command->param_data[0];


	if ((nSensorRequested == RDM_SENSORS_ALL) || (nSensorRequested + 1 > m_pRDMDeviceResponder->GetSensorCount())) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	struct TRdmMessage* pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;
	const struct TRDMSensorValues* sensor_value = m_pRDMDeviceResponder->GetSensorValues(nSensorRequested);

	if (nSensorRequested != sensor_value->sensor_requested) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	pRdmDataOut->param_data_length = 9;
	pRdmDataOut->message_length = RDM_MESSAGE_MINIMUM_SIZE + 9;
	pRdmDataOut->param_data[0] = sensor_value->sensor_requested;
	pRdmDataOut->param_data[1] = (uint8_t)(sensor_value->present >> 8);
	pRdmDataOut->param_data[2] = (uint8_t)sensor_value->present;
	pRdmDataOut->param_data[3] = (uint8_t)(sensor_value->lowest_detected >> 8);
	pRdmDataOut->param_data[4] = (uint8_t)sensor_value->lowest_detected;
	pRdmDataOut->param_data[5] = (uint8_t)(sensor_value->highest_detected >> 8);
	pRdmDataOut->param_data[6] = (uint8_t)sensor_value->highest_detected;
	pRdmDataOut->param_data[7] = (uint8_t)(sensor_value->recorded >> 8);
	pRdmDataOut->param_data[8] = (uint8_t)sensor_value->recorded;

	RespondMessageAck();
}


void RDMHandler::SetSensorValue(bool IsBroadcast, uint16_t nSubDevice) {
	struct TRdmMessageNoSc *rdm_command = (struct TRdmMessageNoSc *)m_pRdmDataIn;

	if (rdm_command->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const uint8_t nSensorRequested = rdm_command->param_data[0];

	if ((nSensorRequested != RDM_SENSORS_ALL) && (nSensorRequested + 1 > m_pRDMDeviceResponder->GetSensorCount())) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	m_pRDMDeviceResponder->SetSensorValues(nSensorRequested);

	if(IsBroadcast) {
		return;
	}

	struct TRdmMessage* pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;

	if (nSensorRequested == RDM_SENSORS_ALL) {
		pRdmDataOut->param_data_length = 9;
		pRdmDataOut->message_length = RDM_MESSAGE_MINIMUM_SIZE + 9;
		pRdmDataOut->param_data[0] = (uint8_t) 0;
		pRdmDataOut->param_data[1] = (uint8_t) 0;
		pRdmDataOut->param_data[2] = (uint8_t) 0;
		pRdmDataOut->param_data[3] = (uint8_t) 0;
		pRdmDataOut->param_data[4] = (uint8_t) 0;
		pRdmDataOut->param_data[5] = (uint8_t) 0;
		pRdmDataOut->param_data[6] = (uint8_t) 0;
		pRdmDataOut->param_data[7] = (uint8_t) 0;
		pRdmDataOut->param_data[8] = (uint8_t) 0;

		RespondMessageAck();
		return;
	}

	const struct TRDMSensorValues* sensor_value = m_pRDMDeviceResponder->GetSensorValues(nSensorRequested);

	pRdmDataOut->param_data_length = 9;
	pRdmDataOut->message_length = RDM_MESSAGE_MINIMUM_SIZE + 9;
	pRdmDataOut->param_data[0] = sensor_value->sensor_requested;
	pRdmDataOut->param_data[1] = (uint8_t) (sensor_value->present >> 8);
	pRdmDataOut->param_data[2] = (uint8_t) sensor_value->present;
	pRdmDataOut->param_data[3] = (uint8_t) (sensor_value->lowest_detected >> 8);
	pRdmDataOut->param_data[4] = (uint8_t) sensor_value->lowest_detected;
	pRdmDataOut->param_data[5] = (uint8_t) (sensor_value->highest_detected >> 8);
	pRdmDataOut->param_data[6] = (uint8_t) sensor_value->highest_detected;
	pRdmDataOut->param_data[7] = (uint8_t) (sensor_value->recorded >> 8);
	pRdmDataOut->param_data[8] = (uint8_t) sensor_value->recorded;

	RespondMessageAck();
}

void RDMHandler::SetRecordSensors(bool IsBroadcast, uint16_t nSubDevice) {
	struct TRdmMessageNoSc *rdm_command = (struct TRdmMessageNoSc *)m_pRdmDataIn;

	if (rdm_command->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const uint8_t nSensorRequested = rdm_command->param_data[0];

	if ((nSensorRequested == RDM_SENSORS_ALL) && (m_pRDMDeviceResponder->GetSensorCount() == 0)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	if ((nSensorRequested != RDM_SENSORS_ALL) && (nSensorRequested + 1 > m_pRDMDeviceResponder->GetSensorCount())) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	m_pRDMDeviceResponder->SetSensorRecord(nSensorRequested);

	if(IsBroadcast) {
		return;
	}

	struct TRdmMessage* pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

void RDMHandler::GetDeviceHours(uint16_t nSubDevice) {
	uint64_t device_hours = (uint64_t) (Hardware::Get()->GetUpTime() / 3600);
	// The value for the Device Hours field shall be unsigned and not roll over when maximum value is reached.
	if (device_hours > UINT32_MAX) {
		device_hours = UINT32_MAX;
	}

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;

	pRdmDataOut->param_data_length = RDM_DEVICE_HOURS_SIZE;
	pRdmDataOut->param_data[0] = (uint8_t)(device_hours >> 24);
	pRdmDataOut->param_data[1] = (uint8_t)(device_hours >> 16);
	pRdmDataOut->param_data[2] = (uint8_t)(device_hours >> 8);
	pRdmDataOut->param_data[3] = (uint8_t)device_hours;

	RespondMessageAck();
}

void RDMHandler::SetDeviceHours(bool IsBroadcast, uint16_t nSubDevice) {
	RespondMessageNack(E120_NR_WRITE_PROTECT);
}

void RDMHandler::GetIdentifyDevice(uint16_t nSubDevice) {
	struct TRdmMessage *rdm_response = (struct TRdmMessage *)m_pRdmDataOut;

	rdm_response->param_data_length = 1;

	assert(RDMIdentify::Get() != 0);
	rdm_response->param_data[0] = RDMIdentify::Get()->IsEnabled() ? (uint8_t) 1 : (uint8_t) 0;

	RespondMessageAck();
}

void RDMHandler::SetIdentifyDevice(bool IsBroadcast, uint16_t nSubDevice) {
	struct TRdmMessageNoSc *rdm_command = (struct TRdmMessageNoSc *)m_pRdmDataIn;

	if (rdm_command->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	if ((rdm_command->param_data[0] != RDM_IDENTIFY_STATE_OFF) && (rdm_command->param_data[0] != RDM_IDENTIFY_STATE_ON)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	assert(RDMIdentify::Get() != 0);
	if (rdm_command->param_data[0] == RDM_IDENTIFY_STATE_OFF) {
		RDMIdentify::Get()->Off();
	} else {
		RDMIdentify::Get()->On();
	}

	if(IsBroadcast) {
		return;
	}

	struct TRdmMessage *rdm_response = (struct TRdmMessage *)m_pRdmDataOut;
	rdm_response->param_data_length = 0;
	RespondMessageAck();
}

void RDMHandler::GetRealTimeClock(uint16_t nSubDevice) {
	struct THardwareTime local_time;
	Hardware::Get()->GetTime(&local_time);

	const uint16_t year = local_time.tm_year + 1900;

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;

	pRdmDataOut->param_data[0] = (uint8_t)(year >> 8);
	pRdmDataOut->param_data[1] = (uint8_t)(year);
	pRdmDataOut->param_data[2] = (uint8_t)(local_time.tm_mon + 1);	// 0..11
	pRdmDataOut->param_data[3] = (uint8_t)(local_time.tm_mday);
	pRdmDataOut->param_data[4] = (uint8_t)(local_time.tm_hour);
	pRdmDataOut->param_data[5] = (uint8_t)(local_time.tm_min);
	pRdmDataOut->param_data[6] = (uint8_t)(local_time.tm_sec);

	pRdmDataOut->param_data_length = 7;

	RespondMessageAck();
}

void RDMHandler::SetRealTimeClock(bool IsBroadcast, uint16_t nSubDevice) {
	struct TRdmMessageNoSc *rdm_command = (struct TRdmMessageNoSc *)m_pRdmDataIn;

	if (rdm_command->param_data_length != 7) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	struct THardwareTime tTime;

	tTime.tm_year = (rdm_command->param_data[0] << 8) + rdm_command->param_data[1];
	tTime.tm_mon = rdm_command->param_data[2] - 1;	// 0..11
	tTime.tm_mday = rdm_command->param_data[3];
	tTime.tm_hour = rdm_command->param_data[4];
	tTime.tm_min = rdm_command->param_data[5];
	tTime.tm_sec = rdm_command->param_data[6];

	if ((!IsBroadcast) && (!Hardware::Get()->SetTime(tTime))) {
		RespondMessageNack(E120_NR_WRITE_PROTECT);
	}

	if(IsBroadcast) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

void RDMHandler::SetResetDevice(bool IsBroadcast, uint16_t nSubDevice) {
	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;
	pRdmDataOut->param_data_length = 0;

	if(IsBroadcast == false) {
		RespondMessageAck();
	}

	if(!Hardware::Get()->Reboot()) {
		RespondMessageNack(E120_NR_WRITE_PROTECT);
	}
}

void RDMHandler::GetIdentifyMode(uint16_t nSubDevice) {
	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;

	pRdmDataOut->param_data_length = 1;

	assert(RDMIdentify::Get() != 0);
	pRdmDataOut->param_data[0] = RDMIdentify::Get()->GetMode();

	RespondMessageAck();
}

void RDMHandler::SetIdentifyMode(bool IsBroadcast, uint16_t nSubDevice) {
	struct TRdmMessageNoSc *rdm_command = (struct TRdmMessageNoSc *)m_pRdmDataIn;

	if (rdm_command->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	if ((rdm_command->param_data[0] != 0) && (rdm_command->param_data[0] != 0xFF)) {
		RespondMessageNack( E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	assert(RDMIdentify::Get() != 0);
	RDMIdentify::Get()->SetMode((TRdmIdentifyMode) rdm_command->param_data[0]);

	if(IsBroadcast) {
		return;
	}

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

void RDMHandler::GetPowerState(uint16_t nSubDevice) {
	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;

	pRdmDataOut->param_data_length = 1;
	pRdmDataOut->param_data[0] = E120_POWER_STATE_NORMAL;

	RespondMessageAck();
}

void RDMHandler::SetPowerState(bool IsBroadcast, uint16_t nSubDevice) {
	struct TRdmMessageNoSc *pRdmDataIn = (struct TRdmMessageNoSc *)m_pRdmDataIn;

	if (pRdmDataIn->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const uint8_t nState = pRdmDataIn->param_data[0];

	if ((nState != POWER_STATE_NORMAL) && (nState > POWER_STATE_STANDBY) ) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *)m_pRdmDataOut;

	if (nState == POWER_STATE_NORMAL) {
		pRdmDataOut->param_data_length = 0;
		RespondMessageAck();
		return;
	}

#if defined(ENABLE_POWER_STATE_FULL_OFF)
	if (nState == POWER_STATE_FULL_OFF) {
		assert(Hardware::Get() != 0);
		if(!Hardware::Get()->PowerOff()) {
			RespondMessageNack(E120_NR_WRITE_PROTECT);
			return;
		}
	}
#endif

	RespondMessageNack(E120_NR_WRITE_PROTECT);
}

void RDMHandler::GetSlotInfo(uint16_t nSubDevice) {
    struct TRdmMessage *rdm_response = (struct TRdmMessage *)m_pRdmDataOut;
	const uint16_t nDmxFootPrint = m_pRDMDeviceResponder->GetDmxFootPrint(nSubDevice);
	struct TLightSetSlotInfo tSlotInfo;

	uint16_t j = 0;

	for (uint16_t i = 0; i < nDmxFootPrint; i++) {
		if (m_pRDMDeviceResponder->GetSlotInfo(nSubDevice, i, tSlotInfo)) {
			rdm_response->param_data[j++] = (uint8_t) (i >> 8);
			rdm_response->param_data[j++] = (uint8_t) i;
			rdm_response->param_data[j++] = tSlotInfo.nType;
			rdm_response->param_data[j++] = (uint8_t) (tSlotInfo.nCategory >> 8);
			rdm_response->param_data[j++] = (uint8_t) tSlotInfo.nCategory;
		}
	}

	rdm_response->param_data_length = j;
	rdm_response->message_length = RDM_MESSAGE_MINIMUM_SIZE + j;

	RespondMessageAck();
	return;
}

void RDMHandler::GetSlotDescription(uint16_t nSubDevice) {
	struct TRdmMessageNoSc *pRdmDataIn = (struct TRdmMessageNoSc *) m_pRdmDataIn;
	const uint16_t nSlotOffset = (pRdmDataIn->param_data[0] << 8) + pRdmDataIn->param_data[1];
	struct TLightSetSlotInfo tSlotInfo;

	if(!m_pRDMDeviceResponder->GetSlotInfo(nSubDevice, nSlotOffset, tSlotInfo)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	uint8_t nLength;
	const char *pText = RDMSlotInfo::GetCategoryText(tSlotInfo.nCategory, nLength);

	if (pText == 0) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	if (nLength > 32) {
		nLength = 32;
	}

	struct TRdmMessage *pRdmDataOut = (struct TRdmMessage *) m_pRdmDataOut;

	pRdmDataOut->param_data_length = nLength + 2;

	pRdmDataOut->param_data[0] = pRdmDataIn->param_data[0];
	pRdmDataOut->param_data[1] = pRdmDataIn->param_data[1];

	for (unsigned i = 0; i < (unsigned) nLength; i++) {
		pRdmDataOut->param_data[i + 2] = (uint8_t) pText[i];
	}

	RespondMessageAck();
}
