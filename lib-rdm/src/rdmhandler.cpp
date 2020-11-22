/**
 * @file rdmhandler.cpp
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

#include <algorithm>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <cassert>

#include "rdmhandler.h"

#include "rdmdeviceresponder.h"
#include "rdmsensors.h"
#include "rdmsubdevices.h"

#include "rdmidentify.h"
#include "rdmslotinfo.h"
#include "rdmmessage.h"

#include "rdm.h"
#include "rdm_e120.h"

#include "hardware.h"

#include "debug.h"

enum TPowerState {
	POWER_STATE_FULL_OFF = 0x00,	///< Completely disengages power to device. Device can no longer respond.
	POWER_STATE_SHUTDOWN = 0x01,	///< Reduced power mode, may require device reset to return to normal operation. Device still responds to messages.
	POWER_STATE_STANDBY = 0x02,		///< Reduced power mode. Device can return to NORMAL without a reset. Device still responds to messages.
	POWER_STATE_NORMAL = 0xFF,		///< Normal Operating Mode.
};

RDMHandler::RDMHandler(bool bIsRdm): m_bIsRDM(bIsRdm) {
}

void RDMHandler::HandleString(const char *pString, uint32_t nLength) {
	auto *RdmMessage = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	RdmMessage->param_data_length = nLength;

	for (uint32_t i = 0; i < nLength; i++) {
		RdmMessage->param_data[i] = static_cast<uint8_t>(pString[i]);
	}
}

void RDMHandler::CreateRespondMessage(uint8_t nResponseType, uint16_t nReason) {
	DEBUG1_ENTRY
	unsigned i;

	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);
	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->start_code = E120_SC_RDM;
	pRdmDataOut->sub_start_code = pRdmDataIn->sub_start_code;
	pRdmDataOut->transaction_number = pRdmDataIn->transaction_number;
	pRdmDataOut->message_count = 0;	//rdm_queued_message_get_count(); //FIXME rdm_queued_message_get_count
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
		pRdmDataOut->param_data[0] = (nReason >> 8);
		pRdmDataOut->param_data[1] = nReason;
		break;
	default:
		// forces timeout
		return;
		// Unreachable code: break;
	}

	const uint8_t *uid_device = RDMDeviceResponder::Get()->GetUID();

	for (i = 0; i < RDM_UID_SIZE; i++) {
		pRdmDataOut->destination_uid[i] = pRdmDataIn->source_uid[i];
		pRdmDataOut->source_uid[i] = uid_device[i];
	}

	uint16_t rdm_checksum = 0;

	for (i = 0; i < pRdmDataOut->message_length; i++) {
		rdm_checksum += m_pRdmDataOut[i];
	}

	m_pRdmDataOut[i++] = rdm_checksum >> 8;
	m_pRdmDataOut[i] = rdm_checksum & 0XFF;

	DEBUG1_EXIT
}

void RDMHandler::RespondMessageAck() {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if(pRdmDataIn->param_data_length == 0) {
		pRdmDataIn->message_length = RDM_MESSAGE_MINIMUM_SIZE;
	}

	CreateRespondMessage(E120_RESPONSE_TYPE_ACK);
}

void RDMHandler::RespondMessageNack(uint16_t nReason) {
	CreateRespondMessage(E120_RESPONSE_TYPE_NACK_REASON, nReason);
}

const RDMHandler::TPidDefinition RDMHandler::PID_DEFINITIONS[] {
//  {E120_QUEUED_MESSAGE,              	&RDMHandler::GetQueuedMessage,           	nullptr,               				1, true , false},
	{E120_SUPPORTED_PARAMETERS,        	&RDMHandler::GetSupportedParameters,      	nullptr,             				0, false, true , false},
	{E120_DEVICE_INFO,                	&RDMHandler::GetDeviceInfo,               	nullptr,                			0, false, true , true },
	{E120_PRODUCT_DETAIL_ID_LIST, 	   	&RDMHandler::GetProductDetailIdList,     	nullptr,							0, true , true , false},
	{E120_DEVICE_MODEL_DESCRIPTION,    	&RDMHandler::GetDeviceModelDescription,		nullptr,                 			0, true , true , true },
	{E120_MANUFACTURER_LABEL,          	&RDMHandler::GetManufacturerLabel,         	nullptr,                        	0, true , true , true },
	{E120_DEVICE_LABEL,                	&RDMHandler::GetDeviceLabel,               	&RDMHandler::SetDeviceLabel,		0, true , true , true },
	{E120_FACTORY_DEFAULTS,            	&RDMHandler::GetFactoryDefaults,          	&RDMHandler::SetFactoryDefaults,	0, true , true , true },
	{E120_LANGUAGE_CAPABILITIES,       	&RDMHandler::GetLanguage,			        nullptr,                 			0, true , true , false},
	{E120_LANGUAGE,						&RDMHandler::GetLanguage,			        &RDMHandler::SetLanguage,           0, true , true , false},
	{E120_SOFTWARE_VERSION_LABEL,		&RDMHandler::GetSoftwareVersionLabel,   	nullptr,                  			0, false, true , false},
	{E120_BOOT_SOFTWARE_VERSION_ID,		&RDMHandler::GetBootSoftwareVersionId, 		nullptr,                 			0, true , true , false},
	{E120_BOOT_SOFTWARE_VERSION_LABEL,	&RDMHandler::GetBootSoftwareVersionLabel,	nullptr,                   			0, true , true , false},
	{E120_DMX_PERSONALITY,		      	&RDMHandler::GetPersonality,               	&RDMHandler::SetPersonality,        0, true , true , false},
	{E120_DMX_PERSONALITY_DESCRIPTION,	&RDMHandler::GetPersonalityDescription,    	nullptr,                 			1, true , true , false},
	{E120_DMX_START_ADDRESS,           	&RDMHandler::GetDmxStartAddress,          	&RDMHandler::SetDmxStartAddress,	0, false, true , false},
	{E120_SLOT_INFO,					&RDMHandler::GetSlotInfo,					nullptr,							0, true , true , false},
	{E120_SLOT_DESCRIPTION,				&RDMHandler::GetSlotDescription,			nullptr,							2, true , true , false},
	{E120_SENSOR_DEFINITION,		   	&RDMHandler::GetSensorDefinition,			nullptr,							1, true , true , false},
	{E120_SENSOR_VALUE,				   	&RDMHandler::GetSensorValue,				&RDMHandler::SetSensorValue,		1, true , true , false},
	{E120_RECORD_SENSORS,			   	nullptr,									&RDMHandler::SetRecordSensors,	 	0, true , true , false},
	{E120_DEVICE_HOURS,                	&RDMHandler::GetDeviceHours,    	      	&RDMHandler::SetDeviceHours,       	0, true , true , false},
	{E120_REAL_TIME_CLOCK,		       	&RDMHandler::GetRealTimeClock,  			&RDMHandler::SetRealTimeClock,    	0, true , true , false},
	{E120_IDENTIFY_DEVICE,		       	&RDMHandler::GetIdentifyDevice,		    	&RDMHandler::SetIdentifyDevice,    	0, false, true , true },
	{E120_RESET_DEVICE,			    	nullptr,                                	&RDMHandler::SetResetDevice,       	0, true , true , true },
	{E120_POWER_STATE,					&RDMHandler::GetPowerState,					&RDMHandler::SetPowerState,			0, true , true , false},
	{E137_1_IDENTIFY_MODE,			   	&RDMHandler::GetIdentifyMode,				&RDMHandler::SetIdentifyMode,		0, true , true , false},
	{E137_2_LIST_INTERFACES,			&RDMHandler::GetInterfaceList,				nullptr,							0, false, false, true },
	{E137_2_INTERFACE_LABEL,			&RDMHandler::GetInterfaceName,				nullptr,							4, false, false, true },
	{E137_2_INTERFACE_HARDWARE_ADDRESS_TYPE1,&RDMHandler::GetHardwareAddress,		nullptr,							4, false, false, true },
	{E137_2_IPV4_DHCP_MODE,				&RDMHandler::GetDHCPMode,					&RDMHandler::SetDHCPMode,			4, false, false, true },
	{E137_2_IPV4_ZEROCONF_MODE,			&RDMHandler::GetZeroconf,					&RDMHandler::SetZeroconf,			4, false, false, true },
	{E137_2_IPV4_CURRENT_ADDRESS,		&RDMHandler::GetAddressNetmask,				nullptr,							4, false, false, true },
	{E137_2_INTERFACE_RENEW_DHCP, 		nullptr,									&RDMHandler::RenewDhcp,				4, false, false, true },
	{E137_2_IPV4_STATIC_ADDRESS,		&RDMHandler::GetStaticAddress,				&RDMHandler::SetStaticAddress,		4, false, false, true },
	{E137_2_INTERFACE_APPLY_CONFIGURATION,nullptr,									&RDMHandler::ApplyConfiguration,	4, false, false, true },
	{E137_2_DNS_IPV4_NAME_SERVER,		&RDMHandler::GetNameServers,				nullptr,							1, false, false, true },
	{E137_2_IPV4_DEFAULT_ROUTE,			&RDMHandler::GetDefaultRoute,				&RDMHandler::SetDefaultRoute,		4, false, false, true },
	{E137_2_DNS_HOSTNAME,               &RDMHandler::GetHostName,                   &RDMHandler::SetHostName,           0, false, false, true },
	{E137_2_DNS_DOMAIN_NAME,			&RDMHandler::GetDomainName,					&RDMHandler::SetDomainName,			0, false, false, true }
};

const RDMHandler::TPidDefinition RDMHandler::PID_DEFINITIONS_SUB_DEVICES[] {
	{E120_SUPPORTED_PARAMETERS,        &RDMHandler::GetSupportedParameters,			nullptr,                   			0, true, true ,  false},
	{E120_DEVICE_INFO,                 &RDMHandler::GetDeviceInfo,					nullptr,                   			0, true, true ,  false},
	{E120_PRODUCT_DETAIL_ID_LIST, 	   &RDMHandler::GetProductDetailIdList,			nullptr,							0, true, true ,  false},
	{E120_SOFTWARE_VERSION_LABEL,      &RDMHandler::GetSoftwareVersionLabel,		nullptr,                    		0, true, true ,  false},
	{E120_DMX_PERSONALITY,		       &RDMHandler::GetPersonality,            		&RDMHandler::SetPersonality,        0, true, true ,  false},
	{E120_DMX_PERSONALITY_DESCRIPTION, &RDMHandler::GetPersonalityDescription,		nullptr,                        	1, true, true ,  false},
	{E120_DMX_START_ADDRESS,           &RDMHandler::GetDmxStartAddress,          	&RDMHandler::SetDmxStartAddress,	0, true, true ,  false},
	{E120_IDENTIFY_DEVICE,		       &RDMHandler::GetIdentifyDevice,		    	&RDMHandler::SetIdentifyDevice,		0, true, true ,  false}
};

/**
 * @param pRdmDataIn RDM with no Start Code
 * @param pRdmDataOut RDM with the Start Code or it is Discover Message
 */
void RDMHandler::HandleData(const uint8_t *pRdmDataIn, uint8_t *pRdmDataOut) {
	DEBUG_ENTRY

	assert(pRdmDataIn != nullptr);
	assert(pRdmDataOut != nullptr);

	pRdmDataOut[0] = 0xFF; // Invalidate outgoing message;

	m_pRdmDataIn = const_cast<uint8_t*>(pRdmDataIn);
	m_pRdmDataOut = pRdmDataOut;

	bool bIsRdmPacketForMe = false;
	bool bIsRdmPacketBroadcast = false;
	bool bIsRdmPacketVendorcast = false;

	auto *pRdmRequest = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	const uint8_t command_class = pRdmRequest->command_class;
	const uint16_t param_id = (pRdmRequest->param_id[0] << 8) + pRdmRequest->param_id[1];
	const uint8_t *uid_device = RDMDeviceResponder::Get()->GetUID();

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

					auto *p = reinterpret_cast<struct TRdmDiscoveryMsg*>(pRdmDataOut);
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
				auto *pRdmResponse = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

				pRdmResponse->param_data_length = 2;
				pRdmResponse->param_data[0] = 0x00;	// Control Field
				pRdmResponse->param_data[1] = 0x00;	// Control Field

				if (RDMSubDevices::Get()->GetCount() != 0) {
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
				auto *pRdmResponse = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

				pRdmResponse->param_data_length = 2;
				pRdmResponse->param_data[0] = 0x00;	// Control Field
				pRdmResponse->param_data[1] = 0x00;	// Control Field

				if (RDMSubDevices::Get()->GetCount() != 0) {
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

	TPidDefinition const *pid_handler = nullptr;
	bool bRDM;
	bool bRDMNet;

	if (nCommandClass != E120_GET_COMMAND && nCommandClass != E120_SET_COMMAND) {
		RespondMessageNack(E120_NR_UNSUPPORTED_COMMAND_CLASS);
		return;
	}

	const uint16_t sub_device_count = RDMSubDevices::Get()->GetCount();

	if ((nSubDevice > sub_device_count) && (nSubDevice != E120_SUB_DEVICE_ALL_CALL)) {
		RespondMessageNack(E120_NR_SUB_DEVICE_OUT_OF_RANGE);
		return;
	}

	for (uint32_t i = 0; i < sizeof(PID_DEFINITIONS) / sizeof(PID_DEFINITIONS[0]); ++i) {
		if (PID_DEFINITIONS[i].nPid == nParamId) {
			pid_handler = &PID_DEFINITIONS[i];
			bRDM = PID_DEFINITIONS[i].bRDM;
			bRDMNet = PID_DEFINITIONS[i].bRDMNet;
		}
	}

	if (!pid_handler) {
		RespondMessageNack(E120_NR_UNKNOWN_PID);
		DEBUG1_EXIT
		return;
	}

	if (m_bIsRDM) {
		if (!bRDM) {
			RespondMessageNack(E120_NR_UNKNOWN_PID);
			DEBUG1_EXIT
			return;
		}
	} else {
		if (!bRDMNet) {
			RespondMessageNack(E120_NR_UNKNOWN_PID);
			DEBUG1_EXIT
			return;
		}
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

void RDMHandler::GetQueuedMessage(__attribute__((unused)) uint16_t nSubDevice) {
	m_RDMQueuedMessage.Handler(m_pRdmDataOut);
	RespondMessageAck();
}

void RDMHandler::GetSupportedParameters(uint16_t nSubDevice) {
	uint8_t nSupportedParams = 0;
	TPidDefinition *pPidDefinitions;
	uint32_t nTableSize = 0;

	if (nSubDevice != 0) {
		pPidDefinitions = const_cast<TPidDefinition*>(&PID_DEFINITIONS_SUB_DEVICES[0]);
		nTableSize = sizeof(PID_DEFINITIONS_SUB_DEVICES) / sizeof(PID_DEFINITIONS_SUB_DEVICES[0]);
	} else {
		pPidDefinitions = const_cast<TPidDefinition*>(&PID_DEFINITIONS[0]);
		nTableSize = sizeof(PID_DEFINITIONS) / sizeof(PID_DEFINITIONS[0]);
	}

	for (uint32_t i = 0; i < nTableSize; i++) {
		if (pPidDefinitions[i].bIncludeInSupportedParams) {
			nSupportedParams++;
		}
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data_length = (2 * nSupportedParams);

	uint32_t j = 0;

	for (uint32_t i = 0; i < nTableSize; i++) {
		if (pPidDefinitions[i].bIncludeInSupportedParams) {
			pRdmDataOut->param_data[j + j] = (pPidDefinitions[i].nPid >> 8);
			pRdmDataOut->param_data[j + j + 1] = pPidDefinitions[i].nPid;
			j++;
		}
	}

	RespondMessageAck();
}

void RDMHandler::GetDeviceInfo(uint16_t nSubDevice) {
	const struct TRDMDeviceInfo *pRdmDeviceInfoRequested = RDMDeviceResponder::Get()->GetDeviceInfo(nSubDevice);

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	auto *pDeviceInfoOut = reinterpret_cast<struct TRDMDeviceInfo*>(pRdmDataOut->param_data);

	pRdmDataOut->param_data_length = sizeof(struct TRDMDeviceInfo);
	memcpy(pDeviceInfoOut, pRdmDeviceInfoRequested, sizeof(struct TRDMDeviceInfo));

	RespondMessageAck();
}

void RDMHandler::GetProductDetailIdList(__attribute__((unused)) uint16_t nSubDevice) {
	const uint16_t product_detail = RDMDeviceResponder::Get()->GetProductDetail();

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data_length = 2;
	pRdmDataOut->param_data[0] = (product_detail >> 8);
	pRdmDataOut->param_data[1] = (product_detail & 0xFF);

	RespondMessageAck();
}

void RDMHandler::GetDeviceModelDescription(__attribute__((unused)) uint16_t nSubDevice) {
	uint8_t nBoardNameLength;
	const char *pBoardModel = Hardware::Get()->GetBoardName(nBoardNameLength);

	HandleString(pBoardModel, nBoardNameLength);
	RespondMessageAck();
}

void RDMHandler::GetManufacturerLabel(__attribute__((unused)) uint16_t nSubDevice) {
	struct TRDMDeviceInfoData label;

	RDMDeviceResponder::Get()->GetManufacturerName(&label);

	HandleString(label.data, label.length);
	RespondMessageAck();
}

void RDMHandler::GetDeviceLabel(uint16_t nSubDevice) {
	struct TRDMDeviceInfoData label;

	RDMDeviceResponder::Get()->GetLabel(nSubDevice, &label);

	HandleString(label.data, label.length);
	RespondMessageAck();
}

void RDMHandler::SetDeviceLabel(bool IsBroadcast, uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length > RDM_DEVICE_LABEL_MAX_LENGTH) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	RDMDeviceResponder::Get()->SetLabel(nSubDevice, reinterpret_cast<char*>(&pRdmDataIn->param_data[0]), pRdmDataIn->param_data_length);

	if(IsBroadcast) {
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	pRdmDataOut->param_data_length = 0;
	RespondMessageAck();
}

void RDMHandler::GetFactoryDefaults(__attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data_length = 1;
	pRdmDataOut->param_data[0] = RDMDeviceResponder::Get()->GetFactoryDefaults() ? 1 : 0;

	RespondMessageAck();
}

void RDMHandler::SetFactoryDefaults(bool IsBroadcast, __attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 0) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	RDMDeviceResponder::Get()->SetFactoryDefaults();

	if(IsBroadcast) {
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

void RDMHandler::GetLanguage(__attribute__((unused)) uint16_t nSubDevice) {
	const char *pLanguage = RDMDeviceResponder::Get()->GetLanguage();

	HandleString(pLanguage, RDM_DEVICE_SUPPORTED_LANGUAGE_LENGTH);
	RespondMessageAck();
}

void RDMHandler::SetLanguage(bool IsBroadcast, __attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != RDM_DEVICE_SUPPORTED_LANGUAGE_LENGTH) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const char *pSupportedLanguage = RDMDeviceResponder::Get()->GetLanguage();

	if ((pRdmDataIn->param_data[0] != pSupportedLanguage[0]) || (pRdmDataIn->param_data[1] != pSupportedLanguage[1])) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	if (IsBroadcast) {
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

void RDMHandler::GetSoftwareVersionLabel(__attribute__((unused)) uint16_t nSubDevice) {
	const char *software_version = RDMDeviceResponder::Get()->GetSoftwareVersion();
	const uint8_t software_version_length = RDMDeviceResponder::Get()->GetSoftwareVersionLength();

	HandleString(software_version, software_version_length);
	RespondMessageAck();
}

void RDMHandler::GetBootSoftwareVersionId(__attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 0) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	uint32_t boot_software_version_id = Hardware::Get()->GetReleaseId();

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data_length = RDM_BOOT_SOFTWARE_VERSION_ID_LENGTH;
	pRdmDataOut->param_data[0] = (boot_software_version_id >> 24);
	pRdmDataOut->param_data[1] = (boot_software_version_id >> 16);
	pRdmDataOut->param_data[2] = (boot_software_version_id >> 8);
	pRdmDataOut->param_data[3] = boot_software_version_id;

	RespondMessageAck();
}

void RDMHandler::GetBootSoftwareVersionLabel(__attribute__((unused)) uint16_t nSubDevice) {
	uint8_t nVersionLength;
	const char *pVersion = Hardware::Get()->GetSysName(nVersionLength);

	HandleString(pVersion, std::min(static_cast<uint8_t>(RDM_BOOT_SOFTWARE_VERSION_LABEL_MAX_LENGTH), nVersionLength));
	RespondMessageAck();
}

void RDMHandler::GetPersonality(uint16_t nSubDevice) {
	const uint8_t nCurrent = RDMDeviceResponder::Get()->GetPersonalityCurrent(nSubDevice);
	const uint8_t nCount = RDMDeviceResponder::Get()->GetPersonalityCount(nSubDevice);

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data_length = 2;
	pRdmDataOut->param_data[0] = nCurrent;
	pRdmDataOut->param_data[1] = nCount;

	RespondMessageAck();
}

void RDMHandler::SetPersonality(__attribute__((unused)) bool IsBroadcast, uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const uint8_t personality = pRdmDataIn->param_data[0];
	const uint8_t max_personalities = RDMDeviceResponder::Get()->GetPersonalityCount(nSubDevice);

	if ((personality == 0) || (personality > max_personalities)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	RDMDeviceResponder::Get()->SetPersonalityCurrent(nSubDevice, personality);

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

void RDMHandler::GetPersonalityDescription(uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);
	const uint8_t nPersonality = pRdmDataIn->param_data[0];
	const uint8_t nMaxPersonalities = RDMDeviceResponder::Get()->GetPersonalityCount(nSubDevice);

	if ((nPersonality == 0) || (nPersonality > nMaxPersonalities)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	const uint16_t nSlots = RDMDeviceResponder::Get()->GetPersonality(nSubDevice, nPersonality)->GetSlots();

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data[0] = nPersonality;
	pRdmDataOut->param_data[1] = (nSlots >> 8);
	pRdmDataOut->param_data[2] = nSlots;

	char *pDst = reinterpret_cast<char*>(&pRdmDataOut->param_data[3]);
	uint8_t nLength = RDM_PERSONALITY_DESCRIPTION_MAX_LENGTH;

	RDMDeviceResponder::Get()->GetPersonality(nSubDevice, nPersonality)->DescriptionCopyTo(pDst, nLength);

	pRdmDataOut->param_data_length = 3 + nLength;

	RespondMessageAck();
}

void RDMHandler::GetDmxStartAddress(uint16_t nSubDevice) {
	const uint16_t dmx_start_address = RDMDeviceResponder::Get()->GetDmxStartAddress(nSubDevice);

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data_length = 2;
	pRdmDataOut->param_data[0] = (dmx_start_address >> 8);
	pRdmDataOut->param_data[1] = dmx_start_address;

	RespondMessageAck();
}

void RDMHandler::SetDmxStartAddress(bool IsBroadcast, uint16_t nSubDevice) {
	DEBUG2_ENTRY

	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 2) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const uint16_t nDmxStartAddress = (pRdmDataIn->param_data[0] << 8) + pRdmDataIn->param_data[1];

	if ((nDmxStartAddress == 0) || (nDmxStartAddress > DMX_UNIVERSE_SIZE)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	RDMDeviceResponder::Get()->SetDmxStartAddress(nSubDevice, nDmxStartAddress);

	if(IsBroadcast) {
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

void RDMHandler::GetSensorDefinition(__attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	const uint8_t nSensorRequested = pRdmDataIn->param_data[0];

	if ((nSensorRequested == RDM_SENSORS_ALL) || (nSensorRequested + 1 > RDMSensors::Get()->GetCount())) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	const struct TRDMSensorDefintion *sensor_definition = RDMSensors::Get()->GetDefintion(nSensorRequested);

	if (nSensorRequested != sensor_definition->sensor) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	pRdmDataOut->param_data[0] = nSensorRequested;
	pRdmDataOut->param_data[1] = sensor_definition->type;
	pRdmDataOut->param_data[2] = sensor_definition->unit;
	pRdmDataOut->param_data[3] = sensor_definition->prefix;
	pRdmDataOut->param_data[4] = static_cast<uint8_t>(sensor_definition->range_min >> 8);
	pRdmDataOut->param_data[5] = sensor_definition->range_min;
	pRdmDataOut->param_data[6] = static_cast<uint8_t>(sensor_definition->range_max >> 8);
	pRdmDataOut->param_data[7] = sensor_definition->range_max;
	pRdmDataOut->param_data[8] = static_cast<uint8_t>(sensor_definition->normal_min >> 8);
	pRdmDataOut->param_data[9] = (sensor_definition->normal_min);
	pRdmDataOut->param_data[10] = static_cast<uint8_t>(sensor_definition->normal_max >> 8);
	pRdmDataOut->param_data[11] = sensor_definition->normal_max;
	pRdmDataOut->param_data[12] = sensor_definition->recorded_supported;

	int i = 13;

	for (int j = 0; j < sensor_definition->len; j++) {
		pRdmDataOut->param_data[i++] = static_cast<uint8_t>(sensor_definition->description[j]);
	}

	pRdmDataOut->param_data_length = i;

	RespondMessageAck();
}

void RDMHandler::GetSensorValue(__attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const uint8_t nSensorRequested = pRdmDataIn->param_data[0];


	if ((nSensorRequested == RDM_SENSORS_ALL) || (nSensorRequested + 1 > RDMSensors::Get()->GetCount())) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	const struct TRDMSensorValues* sensor_value = RDMSensors::Get()->GetValues(nSensorRequested);

	if (nSensorRequested != sensor_value->sensor_requested) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	pRdmDataOut->param_data_length = 9;
	pRdmDataOut->message_length = RDM_MESSAGE_MINIMUM_SIZE + 9;
	pRdmDataOut->param_data[0] = sensor_value->sensor_requested;
	pRdmDataOut->param_data[1] = static_cast<uint8_t>(sensor_value->present >> 8);
	pRdmDataOut->param_data[2] = sensor_value->present;
	pRdmDataOut->param_data[3] = static_cast<uint8_t>(sensor_value->lowest_detected >> 8);
	pRdmDataOut->param_data[4] = sensor_value->lowest_detected;
	pRdmDataOut->param_data[5] = static_cast<uint8_t>(sensor_value->highest_detected >> 8);
	pRdmDataOut->param_data[6] = sensor_value->highest_detected;
	pRdmDataOut->param_data[7] = static_cast<uint8_t>(sensor_value->recorded >> 8);
	pRdmDataOut->param_data[8] = sensor_value->recorded;

	RespondMessageAck();
}


void RDMHandler::SetSensorValue(bool IsBroadcast, __attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const uint8_t nSensorRequested = pRdmDataIn->param_data[0];

	if ((nSensorRequested != RDM_SENSORS_ALL) && (nSensorRequested + 1 > RDMSensors::Get()->GetCount())) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	RDMSensors::Get()->SetValues(nSensorRequested);

	if(IsBroadcast) {
		return;
	}

	auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	if (nSensorRequested == RDM_SENSORS_ALL) {
		pRdmDataOut->param_data_length = 9;
		pRdmDataOut->message_length = RDM_MESSAGE_MINIMUM_SIZE + 9;
		pRdmDataOut->param_data[0] = 0;
		pRdmDataOut->param_data[1] = 0;
		pRdmDataOut->param_data[2] = 0;
		pRdmDataOut->param_data[3] = 0;
		pRdmDataOut->param_data[4] = 0;
		pRdmDataOut->param_data[5] = 0;
		pRdmDataOut->param_data[6] = 0;
		pRdmDataOut->param_data[7] = 0;
		pRdmDataOut->param_data[8] = 0;

		RespondMessageAck();
		return;
	}

	const struct TRDMSensorValues* sensor_value = RDMSensors::Get()->GetValues(nSensorRequested);

	pRdmDataOut->param_data_length = 9;
	pRdmDataOut->message_length = RDM_MESSAGE_MINIMUM_SIZE + 9;
	pRdmDataOut->param_data[0] = sensor_value->sensor_requested;
	pRdmDataOut->param_data[1] = static_cast<uint8_t>(sensor_value->present >> 8);
	pRdmDataOut->param_data[2] = sensor_value->present;
	pRdmDataOut->param_data[3] = static_cast<uint8_t>(sensor_value->lowest_detected >> 8);
	pRdmDataOut->param_data[4] = sensor_value->lowest_detected;
	pRdmDataOut->param_data[5] = static_cast<uint8_t>(sensor_value->highest_detected >> 8);
	pRdmDataOut->param_data[6] = sensor_value->highest_detected;
	pRdmDataOut->param_data[7] = static_cast<uint8_t>(sensor_value->recorded >> 8);
	pRdmDataOut->param_data[8] = sensor_value->recorded;

	RespondMessageAck();
}

void RDMHandler::SetRecordSensors(bool IsBroadcast, __attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const uint8_t nSensorRequested = pRdmDataIn->param_data[0];

	if ((nSensorRequested == RDM_SENSORS_ALL) && (RDMSensors::Get()->GetCount() == 0)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	if ((nSensorRequested != RDM_SENSORS_ALL) && (nSensorRequested + 1 > RDMSensors::Get()->GetCount())) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	RDMSensors::Get()->SetRecord(nSensorRequested);

	if(IsBroadcast) {
		return;
	}

	auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

void RDMHandler::GetDeviceHours(__attribute__((unused)) uint16_t nSubDevice) {
	uint64_t device_hours = Hardware::Get()->GetUpTime() / 3600;
	// The value for the Device Hours field shall be unsigned and not roll over when maximum value is reached.
	if (device_hours > UINT32_MAX) {
		device_hours = UINT32_MAX;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data_length = RDM_DEVICE_HOURS_SIZE;
	pRdmDataOut->param_data[0] = (device_hours >> 24);
	pRdmDataOut->param_data[1] = (device_hours >> 16);
	pRdmDataOut->param_data[2] = (device_hours >> 8);
	pRdmDataOut->param_data[3] = device_hours;

	RespondMessageAck();
}

void RDMHandler::SetDeviceHours(__attribute__((unused)) bool IsBroadcast, __attribute__((unused)) uint16_t nSubDevice) {
	RespondMessageNack(E120_NR_WRITE_PROTECT);
}

void RDMHandler::GetIdentifyDevice(__attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data_length = 1;

	assert(RDMIdentify::Get() != nullptr);
	pRdmDataOut->param_data[0] = RDMIdentify::Get()->IsEnabled() ? 1 : 0;

	RespondMessageAck();
}

void RDMHandler::SetIdentifyDevice(bool IsBroadcast, __attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	if ((pRdmDataIn->param_data[0] != RDM_IDENTIFY_STATE_OFF) && (pRdmDataIn->param_data[0] != RDM_IDENTIFY_STATE_ON)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	if (pRdmDataIn->param_data[0] == RDM_IDENTIFY_STATE_OFF) {
		RDMIdentify::Get()->Off();
	} else {
		RDMIdentify::Get()->On();
	}

	if(IsBroadcast) {
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	pRdmDataOut->param_data_length = 0;
	RespondMessageAck();
}

void RDMHandler::GetRealTimeClock(__attribute__((unused)) uint16_t nSubDevice) {
	struct tm local_time;
	Hardware::Get()->GetTime(&local_time);

	const uint16_t year = local_time.tm_year + 1900;

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data[0] = (year >> 8);
	pRdmDataOut->param_data[1] = year;
	pRdmDataOut->param_data[2] = local_time.tm_mon + 1;	// 0..11
	pRdmDataOut->param_data[3] = local_time.tm_mday;
	pRdmDataOut->param_data[4] = local_time.tm_hour;
	pRdmDataOut->param_data[5] = local_time.tm_min;
	pRdmDataOut->param_data[6] = local_time.tm_sec;

	pRdmDataOut->param_data_length = 7;

	RespondMessageAck();
}

void RDMHandler::SetRealTimeClock(bool IsBroadcast, __attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 7) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	struct tm tTime;

	tTime.tm_year = ((pRdmDataIn->param_data[0] << 8) + pRdmDataIn->param_data[1]) - 1900;
	tTime.tm_mon = pRdmDataIn->param_data[2] - 1;	// 0..11
	tTime.tm_mday = pRdmDataIn->param_data[3];
	tTime.tm_hour = pRdmDataIn->param_data[4];
	tTime.tm_min = pRdmDataIn->param_data[5];
	tTime.tm_sec = pRdmDataIn->param_data[6];

	if ((!IsBroadcast) && (!Hardware::Get()->SetTime(&tTime))) {
		RespondMessageNack(E120_NR_WRITE_PROTECT);
	}

	if(IsBroadcast) {
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();
}

void RDMHandler::SetResetDevice(bool IsBroadcast, __attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	pRdmDataOut->param_data_length = 0;

	if(IsBroadcast == false) {
		RespondMessageAck();
	}

	if(!Hardware::Get()->Reboot()) {
		RespondMessageNack(E120_NR_WRITE_PROTECT);
	}
}

void RDMHandler::GetPowerState(__attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data_length = 1;
	pRdmDataOut->param_data[0] = E120_POWER_STATE_NORMAL;

	RespondMessageAck();
}

void RDMHandler::SetPowerState(__attribute__((unused)) bool IsBroadcast, __attribute__((unused)) uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		return;
	}

	const uint8_t nState = pRdmDataIn->param_data[0];

	if ((nState != POWER_STATE_NORMAL) && (nState > POWER_STATE_STANDBY) ) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

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
    auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	const uint16_t nDmxFootPrint = RDMDeviceResponder::Get()->GetDmxFootPrint(nSubDevice);
	struct TLightSetSlotInfo tSlotInfo;

	uint32_t j = 0;

	for (uint32_t i = 0; i < nDmxFootPrint; i++) { // FIXME PDL
		if (RDMDeviceResponder::Get()->GetSlotInfo(nSubDevice, i, tSlotInfo)) {
			pRdmDataOut->param_data[j++] = (i >> 8);
			pRdmDataOut->param_data[j++] = i;
			pRdmDataOut->param_data[j++] = tSlotInfo.nType;
			pRdmDataOut->param_data[j++] = (tSlotInfo.nCategory >> 8);
			pRdmDataOut->param_data[j++] = tSlotInfo.nCategory;
		}
	}

	pRdmDataOut->param_data_length = j;
	pRdmDataOut->message_length = RDM_MESSAGE_MINIMUM_SIZE + j;

	RespondMessageAck();
	return;
}

void RDMHandler::GetSlotDescription(uint16_t nSubDevice) {
	auto *pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);
	const uint16_t nSlotOffset = (pRdmDataIn->param_data[0] << 8) + pRdmDataIn->param_data[1];
	struct TLightSetSlotInfo tSlotInfo;

	if(!RDMDeviceResponder::Get()->GetSlotInfo(nSubDevice, nSlotOffset, tSlotInfo)) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	uint8_t nLength;
	const char *pText = RDMSlotInfo::GetCategoryText(tSlotInfo.nCategory, nLength);

	if (pText == nullptr) {
		RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
		return;
	}

	if (nLength > 32) {
		nLength = 32;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data_length = nLength + 2;

	pRdmDataOut->param_data[0] = pRdmDataIn->param_data[0];
	pRdmDataOut->param_data[1] = pRdmDataIn->param_data[1];

	for (uint32_t i = 0; i < nLength; i++) {
		pRdmDataOut->param_data[i + 2] = static_cast<uint8_t>(pText[i]);
	}

	RespondMessageAck();
}
