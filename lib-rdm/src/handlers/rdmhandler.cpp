/**
 * @file rdmhandler.cpp
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <time.h>
#include <cassert>

#include "rdmhandler.h"
#include "rdmdevice.h"
#include "rdmidentify.h"
#include "rdmslotinfo.h"
#include "rdmconst.h"
#include "rdm_e120.h"
#if defined(RDM_RESPONDER)
#include "rdmdeviceresponder.h"
#include "rdmsensors.h"
#include "rdmsubdevices.h"
#endif
#include "e120.h"
#include "rdm_message_print.h"
#include "hal.h"
#include "hal_rtc.h"
#include "hal_boardinfo.h"
#include "display.h"
#include "firmware/debug/debug_debug.h"

enum class PowerState : uint8_t
{
    FULL_OFF = 0x00,  ///< Completely disengages power to device. Device can no longer respond.
    kShutdown = 0x01, ///< Reduced power mode, may require device reset to return to normal operation. Device still responds to messages.
    STANDBY = 0x02,   ///< Reduced power mode. Device can return to NORMAL without a reset. Device still responds to messages.
    NORMAL = 0xFF,    ///< Normal Operating Mode.
};

enum class ResetMode : uint8_t
{
    kWarm = 0x01,
    COLD = 0xFF ///< A cold reset is the equivalent of removing and reapplying power to the device.
};

const RDMHandler::PidDefinition RDMHandler::PID_DEFINITIONS[]{
    {E120_DEVICE_INFO, &RDMHandler::GetDeviceInfo, nullptr, 0, false, true, true},
    {E120_DEVICE_MODEL_DESCRIPTION, &RDMHandler::GetDeviceModelDescription, nullptr, 0, true, true, true},
    {E120_MANUFACTURER_LABEL, &RDMHandler::GetManufacturerLabel, nullptr, 0, true, true, true},
    {E120_DEVICE_LABEL, &RDMHandler::GetDeviceLabel, &RDMHandler::SetDeviceLabel, 0, true, true, true},
    {E120_FACTORY_DEFAULTS, &RDMHandler::GetFactoryDefaults, &RDMHandler::SetFactoryDefaults, 0, true, true, true},
    {E120_IDENTIFY_DEVICE, &RDMHandler::GetIdentifyDevice, &RDMHandler::SetIdentifyDevice, 0, false, true, true},
    {E120_RESET_DEVICE, nullptr, &RDMHandler::SetResetDevice, 0, true, true, true},
#if defined(RDM_RESPONDER)
#if defined(ENABLE_RDM_QUEUED_MSG)
    {E120_QUEUED_MESSAGE, &RDMHandler::GetQueuedMessage, nullptr, 1, true, false},
#endif
    {E120_SUPPORTED_PARAMETERS, &RDMHandler::GetSupportedParameters, nullptr, 0, false, true, false},
#if defined(CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
    {E120_PARAMETER_DESCRIPTION, &RDMHandler::GetParameterDescription, nullptr, 2, false, true, false},
#endif
    {E120_PRODUCT_DETAIL_ID_LIST, &RDMHandler::GetProductDetailIdList, nullptr, 0, true, true, false},
    {E120_LANGUAGE_CAPABILITIES, &RDMHandler::GetLanguage, nullptr, 0, true, true, false},
    {E120_LANGUAGE, &RDMHandler::GetLanguage, &RDMHandler::SetLanguage, 0, true, true, false},
    {E120_SOFTWARE_VERSION_LABEL, &RDMHandler::GetSoftwareVersionLabel, nullptr, 0, false, true, false},
    {E120_BOOT_SOFTWARE_VERSION_ID, &RDMHandler::GetBootSoftwareVersionId, nullptr, 0, true, true, false},
    {E120_BOOT_SOFTWARE_VERSION_LABEL, &RDMHandler::GetBootSoftwareVersionLabel, nullptr, 0, true, true, false},
    {E120_DMX_PERSONALITY, &RDMHandler::GetPersonality, &RDMHandler::SetPersonality, 0, true, true, false},
    {E120_DMX_PERSONALITY_DESCRIPTION, &RDMHandler::GetPersonalityDescription, nullptr, 1, true, true, false},
    {E120_DMX_START_ADDRESS, &RDMHandler::GetDmxStartAddress, &RDMHandler::SetDmxStartAddress, 0, false, true, false},
    {E120_SLOT_INFO, &RDMHandler::GetSlotInfo, nullptr, 0, true, true, false},
    {E120_SLOT_DESCRIPTION, &RDMHandler::GetSlotDescription, nullptr, 2, true, true, false},
    {E120_SENSOR_DEFINITION, &RDMHandler::GetSensorDefinition, nullptr, 1, true, true, false},
    {E120_SENSOR_VALUE, &RDMHandler::GetSensorValue, &RDMHandler::SetSensorValue, 1, true, true, false},
    {E120_RECORD_SENSORS, nullptr, &RDMHandler::SetRecordSensors, 0, true, true, false},
    {E120_DEVICE_HOURS, &RDMHandler::GetDeviceHours, &RDMHandler::SetDeviceHours, 0, true, true, false},
    {E120_DISPLAY_INVERT, &RDMHandler::GetDisplayInvert, &RDMHandler::SetDisplayInvert, 0, true, true, false},
    {E120_DISPLAY_LEVEL, &RDMHandler::GetDisplayLevel, &RDMHandler::SetDisplayLevel, 0, true, true, false},
    {E120_REAL_TIME_CLOCK, &RDMHandler::GetRealTimeClock, &RDMHandler::SetRealTimeClock, 0, true, true, false},
    {E120_POWER_STATE, &RDMHandler::GetPowerState, &RDMHandler::SetPowerState, 0, true, true, false},
#if defined(CONFIG_RDM_ENABLE_SELF_TEST)
    {E120_PERFORM_SELFTEST, &RDMHandler::GetPerformSelfTest, &RDMHandler::SetPerformSelfTest, 0, true, true, false},
    {E120_SELF_TEST_DESCRIPTION, &RDMHandler::GetSelfTestDescription, nullptr, 1, true, true, false},
#endif
#if defined(ENABLE_RDM_PRESET_PLAYBACK)
    {E120_PRESET_PLAYBACK, &RDMHandler::GetPresetPlayback, &RDMHandler::SetPresetPlayback, 0, true, true, false},
#endif
    {E137_1_IDENTIFY_MODE, &RDMHandler::GetIdentifyMode, &RDMHandler::SetIdentifyMode, 0, true, true, false},
#endif
#if defined(NODE_RDMNET_LLRP_ONLY)
    {E137_2_LIST_INTERFACES, &RDMHandler::GetInterfaceList, nullptr, 0, false, false, true},
    {E137_2_INTERFACE_LABEL, &RDMHandler::GetInterfaceName, nullptr, 4, false, false, true},
    {E137_2_INTERFACE_HARDWARE_ADDRESS_TYPE1, &RDMHandler::GetHardwareAddress, nullptr, 4, false, false, true},
    {E137_2_IPV4_DHCP_MODE, &RDMHandler::GetDHCPMode, &RDMHandler::SetDHCPMode, 4, false, false, true},
    {E137_2_IPV4_ZEROCONF_MODE, &RDMHandler::GetZeroconf, &RDMHandler::SetAutoIp, 4, false, false, true},
    {E137_2_IPV4_CURRENT_ADDRESS, &RDMHandler::GetAddressNetmask, nullptr, 4, false, false, true},
    {E137_2_INTERFACE_RENEW_DHCP, nullptr, &RDMHandler::RenewDhcp, 4, false, false, true},
    {E137_2_IPV4_STATIC_ADDRESS, &RDMHandler::GetStaticAddress, &RDMHandler::SetStaticAddress, 4, false, false, true},
    {E137_2_INTERFACE_APPLY_CONFIGURATION, nullptr, &RDMHandler::ApplyConfiguration, 4, false, false, true},
    {E137_2_DNS_IPV4_NAME_SERVER, &RDMHandler::GetNameServers, nullptr, 1, false, false, true},
    {E137_2_IPV4_DEFAULT_ROUTE, &RDMHandler::GetDefaultRoute, &RDMHandler::SetDefaultRoute, 0, false, false, true},
    {E137_2_DNS_HOSTNAME, &RDMHandler::GetHostName, &RDMHandler::SetHostName, 0, false, false, true},
    {E137_2_DNS_DOMAIN_NAME, &RDMHandler::GetDomainName, &RDMHandler::SetDomainName, 0, false, false, true}
#endif
};

const RDMHandler::PidDefinition RDMHandler::PID_DEFINITIONS_SUB_DEVICES[]{
    {E120_DEVICE_INFO, &RDMHandler::GetDeviceInfo, nullptr, 0, true, true, false},
    {E120_SOFTWARE_VERSION_LABEL, &RDMHandler::GetSoftwareVersionLabel, nullptr, 0, true, true, false},
    {E120_IDENTIFY_DEVICE, &RDMHandler::GetIdentifyDevice, &RDMHandler::SetIdentifyDevice, 0, true, true, false},
#if defined(RDM_RESPONDER)
    {E120_SUPPORTED_PARAMETERS, &RDMHandler::GetSupportedParameters, nullptr, 0, true, true, false},
    {E120_PRODUCT_DETAIL_ID_LIST, &RDMHandler::GetProductDetailIdList, nullptr, 0, true, true, false},
    {E120_DMX_PERSONALITY, &RDMHandler::GetPersonality, &RDMHandler::SetPersonality, 0, true, true, false},
    {E120_DMX_PERSONALITY_DESCRIPTION, &RDMHandler::GetPersonalityDescription, nullptr, 1, true, true, false},
    {E120_DMX_START_ADDRESS, &RDMHandler::GetDmxStartAddress, &RDMHandler::SetDmxStartAddress, 0, true, true, false}
#endif
};

#if defined(CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
#if defined(CONFIG_RDM_MANUFACTURER_PIDS_SET)
const RDMHandler::PidDefinition RDMHandler::PID_DEFINITION_MANUFACTURER_GENERAL{
    0, &RDMHandler::GetManufacturerPid, &RDMHandler::SetManufacturerPid, 0, false, true, false};
#else
const RDMHandler::PidDefinition RDMHandler::PID_DEFINITION_MANUFACTURER_GENERAL{0, &RDMHandler::GetManufacturerPid, nullptr, 0, false, true, false};
#endif
#endif

RDMHandler::RDMHandler(bool bIsRdm) : is_rdm_(bIsRdm)
{
    DEBUG_ENTRY();

#if defined(CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
#ifndef NDEBUG
    for (uint32_t i = 0; i < GetParameterDescriptionCount(); i++)
    {
        printf("0x%.4x [%.*s]\n", __builtin_bswap16(PARAMETER_DESCRIPTIONS[i].pid), PARAMETER_DESCRIPTIONS[i].pdl - 0x14,
               PARAMETER_DESCRIPTIONS[i].description);
    }
#endif
#endif

    DEBUG_EXIT();
}

void RDMHandler::HandleString(const char* string, uint32_t length)
{
    auto* message = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    message->param_data_length = static_cast<uint8_t>(length);

    for (uint32_t i = 0; i < length; i++)
    {
        message->param_data[i] = static_cast<uint8_t>(string[i]);
    }
}

void RDMHandler::CreateRespondMessage(uint8_t nResponseType, uint16_t nReason)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);
    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->start_code = E120_SC_RDM;
    pRdmDataOut->sub_start_code = pRdmDataIn->sub_start_code;
    pRdmDataOut->transaction_number = pRdmDataIn->transaction_number;
    pRdmDataOut->message_count = 0; // rdm_queued_message_get_count(); //FIXME rdm_queued_message_get_count
    pRdmDataOut->sub_device[0] = pRdmDataIn->sub_device[0];
    pRdmDataOut->sub_device[1] = pRdmDataIn->sub_device[1];
    pRdmDataOut->command_class = static_cast<uint8_t>(pRdmDataIn->command_class + 1);
    pRdmDataOut->param_id[0] = pRdmDataIn->param_id[0];
    pRdmDataOut->param_id[1] = pRdmDataIn->param_id[1];

    switch (nResponseType)
    {
        case E120_RESPONSE_TYPE_ACK:
            pRdmDataOut->message_length = static_cast<uint8_t>(RDM_MESSAGE_MINIMUM_SIZE + pRdmDataOut->param_data_length);
            pRdmDataOut->slot16.response_type = E120_RESPONSE_TYPE_ACK;
            break;
        case E120_RESPONSE_TYPE_NACK_REASON:
        case E120_RESPONSE_TYPE_ACK_TIMER:
            pRdmDataOut->message_length = static_cast<uint8_t>(RDM_MESSAGE_MINIMUM_SIZE + 2);
            pRdmDataOut->slot16.response_type = nResponseType;
            pRdmDataOut->param_data_length = 2;
            pRdmDataOut->param_data[0] = static_cast<uint8_t>(nReason >> 8);
            pRdmDataOut->param_data[1] = static_cast<uint8_t>(nReason);
            break;
        default:
            // forces timeout
            return;
            // Unreachable code: break;
    }

    const auto* const kUid = RdmDevice::Get().GetUID();

    for (uint32_t i = 0; i < RDM_UID_SIZE; i++)
    {
        pRdmDataOut->destination_uid[i] = pRdmDataIn->source_uid[i];
        pRdmDataOut->source_uid[i] = kUid[i];
    }

    uint16_t rdm_checksum = 0;
    uint32_t i;

    for (i = 0; i < pRdmDataOut->message_length; i++)
    {
        rdm_checksum = static_cast<uint16_t>(rdm_checksum + m_pRdmDataOut[i]);
    }

    m_pRdmDataOut[i++] = static_cast<uint8_t>(rdm_checksum >> 8);
    m_pRdmDataOut[i] = static_cast<uint8_t>(rdm_checksum & 0XFF);
}

void RDMHandler::RespondMessageAck()
{
    CreateRespondMessage(E120_RESPONSE_TYPE_ACK, 0);
}

void RDMHandler::RespondMessageNack(uint16_t reason)
{
    CreateRespondMessage(E120_RESPONSE_TYPE_NACK_REASON, reason);
}

/**
 * @param pRdmDataIn RDM with no Start Code
 * @param pRdmDataOut RDM with the Start Code or it is Discover Message
 */
void RDMHandler::HandleData(const uint8_t* pRdmDataIn, uint8_t* pRdmDataOut)
{
    DEBUG_ENTRY();

    assert(pRdmDataIn != nullptr);
    assert(pRdmDataOut != nullptr);

    pRdmDataOut[0] = 0xFF; // Invalidate outgoing message;

    m_pRdmDataIn = const_cast<uint8_t*>(pRdmDataIn);
    m_pRdmDataOut = pRdmDataOut;

    auto* pRdmRequest = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if ((pRdmRequest->slot16.port_id == 0) || (pRdmRequest->message_count != 0))
    {
        DEBUG_EXIT();
        return;
    }

#ifndef NDEBUG
    rdm::MessagePrintNoStartcode(pRdmDataIn);
#endif

    const auto* const kUid = RdmDevice::Get().GetUID();

    auto bIsRdmPacketBroadcast = (memcmp(pRdmRequest->destination_uid, UID_ALL, RDM_UID_SIZE) == 0);
    auto bIsRdmPacketForMe = false;

    if (!bIsRdmPacketBroadcast)
    {
        if (!(memcmp(pRdmRequest->destination_uid, kUid, 2) == 0))
        {
            DEBUG_EXIT();
            return;
        }

        bIsRdmPacketBroadcast = (memcmp(&pRdmRequest->destination_uid[2], UID_ALL, 4) == 0);

        if (!bIsRdmPacketBroadcast)
        {
            bIsRdmPacketForMe = (memcmp(&pRdmRequest->destination_uid[2], &kUid[2], 4) == 0);
        }
    }

    DEBUG_PRINTF("ForMe=%d, Broadcast=%d, Muted=%d", bIsRdmPacketForMe, bIsRdmPacketBroadcast, is_muted_);

    const auto nCommandClass = pRdmRequest->command_class;
    const auto nParamId = static_cast<uint16_t>((pRdmRequest->param_id[0] << 8) + pRdmRequest->param_id[1]);

    if ((!bIsRdmPacketForMe) && (!bIsRdmPacketBroadcast))
    {
        // Ignore RDM packet
    }
    else if (nCommandClass == E120_DISCOVERY_COMMAND)
    {
        if (nParamId == E120_DISC_UNIQUE_BRANCH)
        {
            if (!is_muted_)
            {
                if ((memcmp(pRdmRequest->param_data, kUid, RDM_UID_SIZE) <= 0) && (memcmp(kUid, pRdmRequest->param_data + 6, RDM_UID_SIZE) <= 0))
                {
                    DEBUG_PUTS("E120_DISC_UNIQUE_BRANCH");

                    auto* p = reinterpret_cast<struct TRdmDiscoveryMsg*>(pRdmDataOut);
                    uint16_t rdm_checksum = 6 * 0xFF;

                    for (uint32_t i = 0; i < 7; i++)
                    {
                        p->header_FE[i] = 0xFE;
                    }

                    p->header_AA = 0xAA;

                    for (uint32_t i = 0; i < 6; i++)
                    {
                        p->masked_device_id[i + i] = kUid[i] | 0xAA;
                        p->masked_device_id[i + i + 1] = kUid[i] | 0x55;
                        rdm_checksum = static_cast<uint16_t>(rdm_checksum + kUid[i]);
                    }

                    p->checksum[0] = static_cast<uint8_t>((rdm_checksum >> 8) | 0xAA);
                    p->checksum[1] = static_cast<uint8_t>((rdm_checksum >> 8) | 0x55);
                    p->checksum[2] = static_cast<uint8_t>((rdm_checksum & 0xFF) | 0xAA);
                    p->checksum[3] = static_cast<uint8_t>((rdm_checksum & 0xFF) | 0x55);

                    DEBUG_EXIT();
                    return;
                }
            }
        }
        else if (nParamId == E120_DISC_UN_MUTE)
        {
            DEBUG_PUTS("E120_DISC_UN_MUTE");

            if (pRdmRequest->param_data_length != 0)
            {
                /* The response RESPONSE_TYPE_NACK_REASON shall only be used in conjunction
                 * with the Command Classes GET_COMMAND_RESPONSE & SET_COMMAND_RESPONSE.
                 */
                return;
            }

            is_muted_ = false;

            if (!bIsRdmPacketBroadcast && bIsRdmPacketForMe)
            {
                auto* response = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

                response->param_data_length = 2;
                response->param_data[0] = 0x00; // Control Field
                response->param_data[1] = 0x00; // Control Field

#if defined(RDM_RESPONDER)
                if (RDMSubDevices::Get()->GetCount() != 0)
                {
                    response->param_data[1] |= RDM_CONTROL_FIELD_SUB_DEVICE_FLAG;
                }
#endif
                RespondMessageAck();
                return;
            }
        }
        else if (nParamId == E120_DISC_MUTE)
        {
            DEBUG_PUTS("E120_DISC_MUTE");

            if (pRdmRequest->param_data_length != 0)
            {
                /* The response RESPONSE_TYPE_NACK_REASON shall only be used in conjunction
                 * with the Command Classes GET_COMMAND_RESPONSE & SET_COMMAND_RESPONSE.
                 */
                return;
            }

            is_muted_ = true;

            if (!bIsRdmPacketBroadcast && bIsRdmPacketForMe)
            {
                auto* response = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

                response->param_data_length = 2;
                response->param_data[0] = 0x00; // Control Field
                response->param_data[1] = 0x00; // Control Field

#if defined(RDM_RESPONDER)
                if (RDMSubDevices::Get()->GetCount() != 0)
                {
                    response->param_data[1] |= RDM_CONTROL_FIELD_SUB_DEVICE_FLAG;
                }
#endif

                RespondMessageAck();
                return;
            }
        }
    }
    else
    {
        auto sub_device = static_cast<uint16_t>((pRdmRequest->sub_device[0] << 8) + pRdmRequest->sub_device[1]);
        Handlers(bIsRdmPacketBroadcast, nCommandClass, nParamId, pRdmRequest->param_data_length, sub_device);
    }

    DEBUG_EXIT();
}

void RDMHandler::Handlers(bool bIsBroadcast, uint8_t nCommandClass, uint16_t nParamId, uint8_t nParamDataLength, uint16_t sub_device)
{
    DEBUG_ENTRY();

    if (nCommandClass != E120_GET_COMMAND && nCommandClass != E120_SET_COMMAND)
    {
        RespondMessageNack(E120_NR_UNSUPPORTED_COMMAND_CLASS);
        DEBUG_EXIT();
        return;
    }

#if defined(RDM_RESPONDER)
    const auto kSubDeviceCount = RDMSubDevices::Get()->GetCount();
#else
    const auto kSubDeviceCount = 0U;
#endif

    if ((sub_device > kSubDeviceCount) && (sub_device != E120_SUB_DEVICE_ALL_CALL))
    {
        RespondMessageNack(E120_NR_SUB_DEVICE_OUT_OF_RANGE);
        DEBUG_EXIT();
        return;
    }

    PidDefinition const* pid_handler = nullptr;
    auto bRDM = false;
    auto bRDMNet = false;

    for (auto& defintion : PID_DEFINITIONS)
    {
        if (defintion.nPid == nParamId)
        {
            pid_handler = &defintion;
            bRDM = defintion.bRDM;
            bRDMNet = defintion.bRDMNet;
            break;
        }
    }

#if defined(CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
    if (!pid_handler)
    {
        for (uint32_t i = 0; i < GetParameterDescriptionCount(); i++)
        {
            if (PARAMETER_DESCRIPTIONS[i].pid == __builtin_bswap16(nParamId))
            {
                pid_handler = &PID_DEFINITION_MANUFACTURER_GENERAL;
                bRDM = true;
                bRDMNet = false;
                break;
            }
        }
    }
#endif

    if (!pid_handler)
    {
        RespondMessageNack(E120_NR_UNKNOWN_PID);
        DEBUG_EXIT();
        return;
    }

    if (is_rdm_)
    {
        if (!bRDM)
        {
            RespondMessageNack(E120_NR_UNKNOWN_PID);
            DEBUG_EXIT();
            return;
        }
    }
    else
    {
        if (!bRDMNet)
        {
            RespondMessageNack(E120_NR_UNKNOWN_PID);
            DEBUG_EXIT();
            return;
        }
    }

    if (nCommandClass == E120_GET_COMMAND)
    {
        if (bIsBroadcast)
        {
            DEBUG_EXIT();
            return;
        }

        if (sub_device == E120_SUB_DEVICE_ALL_CALL)
        {
            RespondMessageNack(E120_NR_SUB_DEVICE_OUT_OF_RANGE);
            DEBUG_EXIT();
            return;
        }

        if (!pid_handler->pGetHandler)
        {
            RespondMessageNack(E120_NR_UNSUPPORTED_COMMAND_CLASS);
            DEBUG_EXIT();
            return;
        }

        if (nParamDataLength != pid_handler->nGetArgumentSize)
        {
            RespondMessageNack(E120_NR_FORMAT_ERROR);
            DEBUG_EXIT();
            return;
        }

        (this->*(pid_handler->pGetHandler))(sub_device);
    }
    else
    {
        if (!pid_handler->pSetHandler)
        {
            RespondMessageNack(E120_NR_UNSUPPORTED_COMMAND_CLASS);
            DEBUG_EXIT();
            return;
        }

        (this->*(pid_handler->pSetHandler))(bIsBroadcast, sub_device);
    }

    DEBUG_EXIT();
}

#if defined(ENABLE_RDM_QUEUED_MSG)
void RDMHandler::GetQueuedMessage([[maybe_unused]] uint16_t sub_device)
{
    m_RDMQueuedMessage.Handler(m_pRdmDataOut);
    RespondMessageAck();
}
#endif

#if !defined(NODE_RDMNET_LLRP_ONLY)
void RDMHandler::GetSupportedParameters(uint16_t sub_device)
{
    uint8_t nSupportedParams = 0;
    PidDefinition* pPidDefinitions;
    uint32_t nTableSize = 0;

    if (sub_device != 0)
    {
        pPidDefinitions = const_cast<PidDefinition*>(&PID_DEFINITIONS_SUB_DEVICES[0]);
        nTableSize = sizeof(PID_DEFINITIONS_SUB_DEVICES) / sizeof(PID_DEFINITIONS_SUB_DEVICES[0]);
    }
    else
    {
        pPidDefinitions = const_cast<PidDefinition*>(&PID_DEFINITIONS[0]);
        nTableSize = sizeof(PID_DEFINITIONS) / sizeof(PID_DEFINITIONS[0]);
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    uint32_t j = 0;

    for (uint32_t i = 0; i < nTableSize; i++)
    {
        if (pPidDefinitions[i].bIncludeInSupportedParams)
        {
            nSupportedParams++;
            pRdmDataOut->param_data[j + j] = static_cast<uint8_t>(pPidDefinitions[i].nPid >> 8);
            pRdmDataOut->param_data[j + j + 1] = static_cast<uint8_t>(pPidDefinitions[i].nPid);
            j++;
        }
    }

#if defined(CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
    const auto nSupportedParamsManufacturer = GetParameterDescriptionCount();

    nSupportedParams = static_cast<uint8_t>(nSupportedParams + nSupportedParamsManufacturer);

    for (uint32_t i = 0; i < nSupportedParamsManufacturer; i++)
    {
        pRdmDataOut->param_data[j + j] = static_cast<uint8_t>(PARAMETER_DESCRIPTIONS[i].pid); ///< The PIDs are swapped
        pRdmDataOut->param_data[j + j + 1] = static_cast<uint8_t>(PARAMETER_DESCRIPTIONS[i].pid >> 8);
        j++;
    }
#endif

    pRdmDataOut->param_data_length = static_cast<uint8_t>(2 * nSupportedParams);

    RespondMessageAck();
}

#if defined(CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
void RDMHandler::GetParameterDescription([[maybe_unused]] uint16_t sub_device)
{
    const auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);
    const auto nPid = static_cast<uint16_t>(pRdmDataIn->param_data[0] + (pRdmDataIn->param_data[1] << 8));

    if (!(nPid >= __builtin_bswap16(0x8000) && nPid <= __builtin_bswap16(0xFFDF)))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    const auto nSupportedParamsManufacturer = GetParameterDescriptionCount();

    for (uint32_t i = 0; i < nSupportedParamsManufacturer; i++)
    {
        if (PARAMETER_DESCRIPTIONS[i].pid == nPid)
        {
            auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

            pRdmDataOut->param_data_length = PARAMETER_DESCRIPTIONS[i].pdl;
            CopyParameterDescription(i, pRdmDataOut->param_data);

            RespondMessageAck();
            return;
        }
    }

    RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
}

void RDMHandler::GetManufacturerPid([[maybe_unused]] uint16_t sub_device)
{
    const auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);
    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    const auto nPid = static_cast<uint16_t>(pRdmDataIn->param_id[0] + (pRdmDataIn->param_id[1] << 8));
    const struct rdmhandler::ManufacturerParamData pIn = {pRdmDataIn->param_data_length, const_cast<uint8_t*>(pRdmDataIn->param_data)};
    struct rdmhandler::ManufacturerParamData pOut = {0, pRdmDataOut->param_data};
    uint16_t nReason;

    if (rdmhandler::HandleManufactureerPidGet(nPid, &pIn, &pOut, nReason))
    {
        pRdmDataOut->param_data_length = pOut.nPdl;
        RespondMessageAck();
        return;
    }

    RespondMessageNack(nReason);
}
#if defined(CONFIG_RDM_MANUFACTURER_PIDS_SET)
void RDMHandler::SetManufacturerPid(bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    const auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);
    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    const auto nPid = static_cast<uint16_t>(pRdmDataIn->param_id[0] + (pRdmDataIn->param_id[1] << 8));
    const struct rdmhandler::ManufacturerParamData pIn = {pRdmDataIn->param_data_length, const_cast<uint8_t*>(pRdmDataIn->param_data)};
    struct rdmhandler::ManufacturerParamData pOut = {0, pRdmDataOut->param_data};
    uint16_t nReason = E120_NR_UNKNOWN_PID;

    for (uint32_t nIndex = 0; nIndex < GetParameterDescriptionCount(); nIndex++)
    {
        if (PARAMETER_DESCRIPTIONS[nIndex].pid == nPid)
        {
            if (rdmhandler::HandleManufactureerPidSet(is_broadcast, nPid, PARAMETER_DESCRIPTIONS[nIndex], &pIn, &pOut, nReason))
            {
                pRdmDataOut->param_data_length = pOut.nPdl;
                RespondMessageAck();
                return;
            }
        }
    }

    RespondMessageNack(nReason);
}
#endif // CONFIG_RDM_MANUFACTURER_PIDS_SET
#endif // CONFIG_RDM_ENABLE_MANUFACTURER_PIDS
#endif

void RDMHandler::GetDeviceInfo([[maybe_unused]] uint16_t sub_device)
{
#if defined(RDM_RESPONDER)
    const auto* const kDeviceInfo = RDMDeviceResponder::Get()->GetDeviceInfo(sub_device);
#else
    const auto* const kDeviceInfo = RdmDevice::Get().GetDeviceInfo();
#endif
    auto* data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    auto* device_info_out = reinterpret_cast<struct rdm::DeviceInfo*>(data_out->param_data);

    data_out->param_data_length = sizeof(struct rdm::DeviceInfo);
    memcpy(device_info_out, kDeviceInfo, sizeof(struct rdm::DeviceInfo));

    RespondMessageAck();
}

void RDMHandler::GetFactoryDefaults([[maybe_unused]] uint16_t sub_device)
{
    auto* data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    data_out->param_data_length = 1;
#if defined(RDM_RESPONDER)
    data_out->param_data[0] = RDMDeviceResponder::Get()->GetFactoryDefaults() ? 1 : 0;
#else
    data_out->param_data[0] = RdmDevice::Get().GetFactoryDefaults();
#endif
    RespondMessageAck();
}

void RDMHandler::SetFactoryDefaults(bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* data_in = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (data_in->param_data_length != 0)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    RdmDevice::Get().SetFactoryDefaults();

    if (is_broadcast)
    {
        return;
    }

    auto* data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    data_out->param_data_length = 0;

    RespondMessageAck();
}

#if defined(RDM_RESPONDER)
void RDMHandler::GetProductDetailIdList([[maybe_unused]] uint16_t sub_device)
{
    const auto kProductDetail = RdmDevice::Get().GetProductDetail();

    auto* data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    data_out->param_data_length = 2;
    data_out->param_data[0] = static_cast<uint8_t>(kProductDetail >> 8);
    data_out->param_data[1] = static_cast<uint8_t>(kProductDetail & 0xFF);

    RespondMessageAck();
}
#endif

void RDMHandler::GetDeviceModelDescription([[maybe_unused]] uint16_t sub_device)
{
    uint8_t board_name_length;
    const char* board_model = hal::BoardName(board_name_length);

    HandleString(board_model, board_name_length);
    RespondMessageAck();
}

void RDMHandler::GetManufacturerLabel([[maybe_unused]] uint16_t sub_device)
{
    rdm::DeviceInfoData label;

    RdmDevice::Get().GetManufacturerName(&label);

    HandleString(label.data, label.length);
    RespondMessageAck();
}

void RDMHandler::GetDeviceLabel([[maybe_unused]] uint16_t sub_device)
{
    rdm::DeviceInfoData label;

#if defined(RDM_RESPONDER)
    RDMDeviceResponder::Get()->GetLabel(sub_device, &label);
#else
    RdmDevice::Get().GetLabel(&label);
#endif
    HandleString(label.data, label.length);
    RespondMessageAck();
}

void RDMHandler::SetDeviceLabel(bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* data_in = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (data_in->param_data_length > RDM_DEVICE_LABEL_MAX_LENGTH)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

#if defined(RDM_RESPONDER)
    RDMDeviceResponder::Get()->SetLabel(sub_device, reinterpret_cast<char*>(&data_in->param_data[0]), data_in->param_data_length);
#else
    struct rdm::DeviceInfoData info;
    info.data = reinterpret_cast<char*>(&data_in->param_data[0]);
    info.length = data_in->param_data_length;

    RdmDevice::Get().SetLabel(&info);
#endif

    if (is_broadcast)
    {
        return;
    }

    auto* data_out = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    data_out->param_data_length = 0;
    RespondMessageAck();
}

void RDMHandler::GetSoftwareVersionLabel([[maybe_unused]] uint16_t sub_device)
{
    const auto* const kSoftwareVersion = FirmwareVersion::Get()->GetSoftwareVersion();
    const auto kSoftwareVersionLength = firmwareversion::length::kSoftwareVersion;;

    HandleString(kSoftwareVersion, kSoftwareVersionLength);
    RespondMessageAck();
}

void RDMHandler::SetResetDevice([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 1)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    const auto kMode = static_cast<ResetMode>(pRdmDataIn->param_data[0]);

    if ((kMode != ResetMode::kWarm) && (kMode != ResetMode::COLD))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    pRdmDataOut->param_data_length = 0;

    if (kMode == ResetMode::COLD)
    {
        RespondMessageNack(E120_NR_WRITE_PROTECT);
        return;
    }

    if (!hal::Reboot())
    {
        RespondMessageNack(E120_NR_WRITE_PROTECT);
    }
}

void RDMHandler::GetIdentifyDevice([[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data_length = 1;

    assert(RDMIdentify::Get() != nullptr);
    pRdmDataOut->param_data[0] = RDMIdentify::Get()->IsEnabled() ? 1 : 0;

    RespondMessageAck();
}

void RDMHandler::SetIdentifyDevice(bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 1)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    if ((pRdmDataIn->param_data[0] != RDM_IDENTIFY_STATE_OFF) && (pRdmDataIn->param_data[0] != RDM_IDENTIFY_STATE_ON))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    if (pRdmDataIn->param_data[0] == RDM_IDENTIFY_STATE_OFF)
    {
        RDMIdentify::Get()->Off();
    }
    else
    {
        RDMIdentify::Get()->On();
    }

    if (is_broadcast)
    {
        return;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    pRdmDataOut->param_data_length = 0;
    RespondMessageAck();
}

#if defined(RDM_RESPONDER)
void RDMHandler::GetLanguage([[maybe_unused]] uint16_t sub_device)
{
    const auto* pLanguage = RDMDeviceResponder::Get()->GetLanguage();

    HandleString(pLanguage, RDM_DEVICE_SUPPORTED_LANGUAGE_LENGTH);
    RespondMessageAck();
}

void RDMHandler::SetLanguage(bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != RDM_DEVICE_SUPPORTED_LANGUAGE_LENGTH)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    const char* pSupportedLanguage = RDMDeviceResponder::Get()->GetLanguage();

    if ((pRdmDataIn->param_data[0] != pSupportedLanguage[0]) || (pRdmDataIn->param_data[1] != pSupportedLanguage[1]))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    if (is_broadcast)
    {
        return;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    pRdmDataOut->param_data_length = 0;

    RespondMessageAck();
}

void RDMHandler::GetBootSoftwareVersionId([[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 0)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    uint32_t boot_software_version_id = hal::kReleaseId;

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data_length = RDM_BOOT_SOFTWARE_VERSION_ID_LENGTH;
    pRdmDataOut->param_data[0] = static_cast<uint8_t>(boot_software_version_id >> 24);
    pRdmDataOut->param_data[1] = static_cast<uint8_t>(boot_software_version_id >> 16);
    pRdmDataOut->param_data[2] = static_cast<uint8_t>(boot_software_version_id >> 8);
    pRdmDataOut->param_data[3] = static_cast<uint8_t>(boot_software_version_id);

    RespondMessageAck();
}

void RDMHandler::GetBootSoftwareVersionLabel([[maybe_unused]] uint16_t sub_device)
{
    uint8_t sys_name_length;
    const auto* sys_name = hal::SysName(sys_name_length);

    HandleString(sys_name, std::min(static_cast<uint8_t>(RDM_BOOT_SOFTWARE_VERSION_LABEL_MAX_LENGTH), sys_name_length));
    RespondMessageAck();
}

void RDMHandler::GetPersonality(uint16_t sub_device)
{
    const auto nCurrent = RDMDeviceResponder::Get()->GetPersonalityCurrent(sub_device);
    const auto nCount = RDMDeviceResponder::Get()->GetPersonalityCount(sub_device);

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data_length = 2;
    pRdmDataOut->param_data[0] = nCurrent;
    pRdmDataOut->param_data[1] = nCount;

    RespondMessageAck();
}

void RDMHandler::SetPersonality([[maybe_unused]] bool is_broadcast, uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 1)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    const auto personality = pRdmDataIn->param_data[0];
    const auto max_personalities = RDMDeviceResponder::Get()->GetPersonalityCount(sub_device);

    if ((personality == 0) || (personality > max_personalities))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    RDMDeviceResponder::Get()->SetPersonalityCurrent(sub_device, personality);

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    pRdmDataOut->param_data_length = 0;

    RespondMessageAck();
}

void RDMHandler::GetPersonalityDescription(uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);
    const auto nPersonality = pRdmDataIn->param_data[0];
    const auto nMaxPersonalities = RDMDeviceResponder::Get()->GetPersonalityCount(sub_device);

    if ((nPersonality == 0) || (nPersonality > nMaxPersonalities))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    const auto nSlots = RDMDeviceResponder::Get()->GetPersonality(sub_device, nPersonality)->GetSlots();

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data[0] = nPersonality;
    pRdmDataOut->param_data[1] = static_cast<uint8_t>(nSlots >> 8);
    pRdmDataOut->param_data[2] = static_cast<uint8_t>(nSlots);

    auto* pDst = reinterpret_cast<char*>(&pRdmDataOut->param_data[3]);
    uint8_t length = rdm::personality::DESCRIPTION_MAX_LENGTH;

    RDMDeviceResponder::Get()->GetPersonality(sub_device, nPersonality)->DescriptionCopyTo(pDst, length);

    pRdmDataOut->param_data_length = static_cast<uint8_t>(3 + length);

    RespondMessageAck();
}

void RDMHandler::GetDmxStartAddress(uint16_t sub_device)
{
    const auto dmx_start_address = RDMDeviceResponder::Get()->GetDmxStartAddress(sub_device);

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data_length = 2;
    pRdmDataOut->param_data[0] = static_cast<uint8_t>(dmx_start_address >> 8);
    pRdmDataOut->param_data[1] = static_cast<uint8_t>(dmx_start_address);

    RespondMessageAck();
}

void RDMHandler::SetDmxStartAddress(bool is_broadcast, uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 2)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    const auto nDmxStartAddress = static_cast<uint16_t>((pRdmDataIn->param_data[0] << 8) + pRdmDataIn->param_data[1]);

    if ((nDmxStartAddress == 0) || (nDmxStartAddress > dmxnode::kUniverseSize))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    RDMDeviceResponder::Get()->SetDmxStartAddress(sub_device, nDmxStartAddress);

    if (is_broadcast)
    {
        return;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    pRdmDataOut->param_data_length = 0;

    RespondMessageAck();
}

void RDMHandler::GetSensorDefinition([[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    const auto nSensorRequested = pRdmDataIn->param_data[0];

    if ((nSensorRequested == RDM_SENSORS_ALL) || (nSensorRequested + 1 > RDMSensors::Get()->GetCount()))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    const auto* sensor_definition = RDMSensors::Get()->GetDefintion(nSensorRequested);

    if (nSensorRequested != sensor_definition->sensor)
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    pRdmDataOut->param_data[0] = nSensorRequested;
    pRdmDataOut->param_data[1] = sensor_definition->type;
    pRdmDataOut->param_data[2] = sensor_definition->unit;
    pRdmDataOut->param_data[3] = sensor_definition->prefix;
    pRdmDataOut->param_data[4] = static_cast<uint8_t>(sensor_definition->range_min >> 8);
    pRdmDataOut->param_data[5] = static_cast<uint8_t>(sensor_definition->range_min);
    pRdmDataOut->param_data[6] = static_cast<uint8_t>(sensor_definition->range_max >> 8);
    pRdmDataOut->param_data[7] = static_cast<uint8_t>(sensor_definition->range_max);
    pRdmDataOut->param_data[8] = static_cast<uint8_t>(sensor_definition->normal_min >> 8);
    pRdmDataOut->param_data[9] = static_cast<uint8_t>(sensor_definition->normal_min);
    pRdmDataOut->param_data[10] = static_cast<uint8_t>(sensor_definition->normal_max >> 8);
    pRdmDataOut->param_data[11] = static_cast<uint8_t>(sensor_definition->normal_max);
    pRdmDataOut->param_data[12] = sensor_definition->recorded_supported;

    int i = 13;

    for (int j = 0; j < sensor_definition->length; j++)
    {
        pRdmDataOut->param_data[i++] = static_cast<uint8_t>(sensor_definition->description[j]);
    }

    pRdmDataOut->param_data_length = static_cast<uint8_t>(i);

    RespondMessageAck();
}

void RDMHandler::GetSensorValue([[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 1)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    const auto nSensorRequested = pRdmDataIn->param_data[0];

    if ((nSensorRequested == RDM_SENSORS_ALL) || (nSensorRequested + 1 > RDMSensors::Get()->GetCount()))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    const auto* pSensorValues = RDMSensors::Get()->GetValues(nSensorRequested);

    if (nSensorRequested != pSensorValues->sensor_requested)
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    pRdmDataOut->param_data_length = 9;
    pRdmDataOut->message_length = RDM_MESSAGE_MINIMUM_SIZE + 9;
    pRdmDataOut->param_data[0] = pSensorValues->sensor_requested;
    pRdmDataOut->param_data[1] = static_cast<uint8_t>(pSensorValues->present >> 8);
    pRdmDataOut->param_data[2] = static_cast<uint8_t>(pSensorValues->present);
    pRdmDataOut->param_data[3] = static_cast<uint8_t>(pSensorValues->lowest_detected >> 8);
    pRdmDataOut->param_data[4] = static_cast<uint8_t>(pSensorValues->lowest_detected);
    pRdmDataOut->param_data[5] = static_cast<uint8_t>(pSensorValues->highest_detected >> 8);
    pRdmDataOut->param_data[6] = static_cast<uint8_t>(pSensorValues->highest_detected);
    pRdmDataOut->param_data[7] = static_cast<uint8_t>(pSensorValues->recorded >> 8);
    pRdmDataOut->param_data[8] = static_cast<uint8_t>(pSensorValues->recorded);

    RespondMessageAck();
}

void RDMHandler::SetSensorValue(bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 1)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    const auto nSensorRequested = pRdmDataIn->param_data[0];

    if ((nSensorRequested != RDM_SENSORS_ALL) && (nSensorRequested + 1 > RDMSensors::Get()->GetCount()))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    RDMSensors::Get()->SetValues(nSensorRequested);

    if (is_broadcast)
    {
        return;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    if (nSensorRequested == RDM_SENSORS_ALL)
    {
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

    const auto* sensor_value = RDMSensors::Get()->GetValues(nSensorRequested);

    pRdmDataOut->param_data_length = 9;
    pRdmDataOut->message_length = RDM_MESSAGE_MINIMUM_SIZE + 9;
    pRdmDataOut->param_data[0] = sensor_value->sensor_requested;
    pRdmDataOut->param_data[1] = static_cast<uint8_t>(sensor_value->present >> 8);
    pRdmDataOut->param_data[2] = static_cast<uint8_t>(sensor_value->present);
    pRdmDataOut->param_data[3] = static_cast<uint8_t>(sensor_value->lowest_detected >> 8);
    pRdmDataOut->param_data[4] = static_cast<uint8_t>(sensor_value->lowest_detected);
    pRdmDataOut->param_data[5] = static_cast<uint8_t>(sensor_value->highest_detected >> 8);
    pRdmDataOut->param_data[6] = static_cast<uint8_t>(sensor_value->highest_detected);
    pRdmDataOut->param_data[7] = static_cast<uint8_t>(sensor_value->recorded >> 8);
    pRdmDataOut->param_data[8] = static_cast<uint8_t>(sensor_value->recorded);

    RespondMessageAck();
}

void RDMHandler::SetRecordSensors(bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 1)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    const auto nSensorRequested = pRdmDataIn->param_data[0];

    if ((nSensorRequested == RDM_SENSORS_ALL) && (RDMSensors::Get()->GetCount() == 0))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    if ((nSensorRequested != RDM_SENSORS_ALL) && (nSensorRequested + 1 > RDMSensors::Get()->GetCount()))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    RDMSensors::Get()->SetRecord(nSensorRequested);

    if (is_broadcast)
    {
        return;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    pRdmDataOut->param_data_length = 0;

    RespondMessageAck();
}

void RDMHandler::GetDeviceHours([[maybe_unused]] uint16_t sub_device)
{
    uint64_t device_hours = hal::Uptime() / 3600U;
    // The value for the Device Hours field shall be unsigned and not roll over when maximum value is reached.
    if (device_hours > UINT32_MAX)
    {
        device_hours = UINT32_MAX;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data_length = RDM_DEVICE_HOURS_SIZE;
    pRdmDataOut->param_data[0] = static_cast<uint8_t>(device_hours >> 24);
    pRdmDataOut->param_data[1] = static_cast<uint8_t>(device_hours >> 16);
    pRdmDataOut->param_data[2] = static_cast<uint8_t>(device_hours >> 8);
    pRdmDataOut->param_data[3] = static_cast<uint8_t>(device_hours);

    RespondMessageAck();
}

void RDMHandler::SetDeviceHours([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    RespondMessageNack(E120_NR_WRITE_PROTECT);
}

void RDMHandler::GetDisplayInvert([[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data_length = 1;

    assert(Display::Get() != nullptr);
    pRdmDataOut->param_data[0] = Display::Get()->GetFlipVertically() ? 1 : 0;

    RespondMessageAck();
}

void RDMHandler::SetDisplayInvert([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 1)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    const auto nInvert = pRdmDataIn->param_data[0];

    if (nInvert == 2)
    { // Auto
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
    }

    assert(Display::Get() != nullptr);
    Display::Get()->SetFlipVertically(nInvert == 1);

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    pRdmDataOut->param_data_length = 0;

    RespondMessageAck();
}

void RDMHandler::GetDisplayLevel([[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data_length = 1;

    assert(Display::Get() != nullptr);

    pRdmDataOut->param_data[0] = Display::Get()->GetContrast();

    RespondMessageAck();
}

void RDMHandler::SetDisplayLevel([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 1)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    const auto nContrast = pRdmDataIn->param_data[0];

    assert(Display::Get() != nullptr);

    if (nContrast != 0)
    {
        Display::Get()->SetSleep(false);
    }
    else
    {
        Display::Get()->SetSleep(true);
    }

    Display::Get()->SetContrast(nContrast);

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    pRdmDataOut->param_data_length = 0;

    RespondMessageAck();
}

void RDMHandler::GetRealTimeClock([[maybe_unused]] uint16_t sub_device)
{
    const auto ltime = time(nullptr);
    const auto* tm = localtime(&ltime);
    const auto year = static_cast<uint16_t>(tm->tm_year + 1900);

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data[0] = static_cast<uint8_t>(year >> 8);
    pRdmDataOut->param_data[1] = static_cast<uint8_t>(year);
    pRdmDataOut->param_data[2] = static_cast<uint8_t>(tm->tm_mon + 1); // 0..11
    pRdmDataOut->param_data[3] = static_cast<uint8_t>(tm->tm_mday);
    pRdmDataOut->param_data[4] = static_cast<uint8_t>(tm->tm_hour);
    pRdmDataOut->param_data[5] = static_cast<uint8_t>(tm->tm_min);
    pRdmDataOut->param_data[6] = static_cast<uint8_t>(tm->tm_sec);

    pRdmDataOut->param_data_length = 7;

    RespondMessageAck();
}

void RDMHandler::SetRealTimeClock(bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 7)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    struct tm tTime;

    tTime.tm_year = ((pRdmDataIn->param_data[0] << 8) + pRdmDataIn->param_data[1]) - 1900;
    tTime.tm_mon = pRdmDataIn->param_data[2] - 1; // 0..11
    tTime.tm_mday = pRdmDataIn->param_data[3];
    tTime.tm_hour = pRdmDataIn->param_data[4];
    tTime.tm_min = pRdmDataIn->param_data[5];
    tTime.tm_sec = pRdmDataIn->param_data[6];

    if ((!is_broadcast) && (!hal::rtc::Set(&tTime)))
    {
        RespondMessageNack(E120_NR_WRITE_PROTECT);
    }

    if (is_broadcast)
    {
        return;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    pRdmDataOut->param_data_length = 0;

    RespondMessageAck();
}

void RDMHandler::GetPowerState([[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data_length = 1;
    pRdmDataOut->param_data[0] = E120_POWER_STATE_NORMAL;

    RespondMessageAck();
}

void RDMHandler::SetPowerState([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 1)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    const auto nState = static_cast<PowerState>(pRdmDataIn->param_data[0]);

    if ((nState != PowerState::NORMAL) && (nState > PowerState::STANDBY))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    if (nState == PowerState::NORMAL)
    {
        pRdmDataOut->param_data_length = 0;
        RespondMessageAck();
        return;
    }

#if defined(ENABLE_POWER_STATE_FULL_OFF)
    if (nState == PowerState::FULL_OFF)
    {
        assert(Hardware::Get() != 0);
        if (!Hardware::Get()->PowerOff())
        {
            RespondMessageNack(E120_NR_WRITE_PROTECT);
            return;
        }
    }
#endif

    RespondMessageNack(E120_NR_WRITE_PROTECT);
}

#if defined(CONFIG_RDM_ENABLE_SELF_TEST)
#include "rdm_selftest.h"

void RDMHandler::GetPerformSelfTest([[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data_length = 1;
    pRdmDataOut->param_data[0] = rdm::selftest::Get() != 0 ? 1 : 0;

    RespondMessageAck();
}

void RDMHandler::SetPerformSelfTest([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 1)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    if (!rdm::selftest::Set(pRdmDataIn->param_data[0]))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    pRdmDataOut->param_data_length = 0;

    RespondMessageAck();
}

void RDMHandler::GetSelfTestDescription([[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);
    uint32_t length;
    const auto* pText = rdm::selftest::GetDescription(pRdmDataIn->param_data[0], length);

    if (pText == nullptr)
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    if (length > 32)
    {
        length = 32;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data_length = static_cast<uint8_t>(length + 1);

    pRdmDataOut->param_data[0] = pRdmDataIn->param_data[0];

    for (uint32_t i = 0; i < length; i++)
    {
        pRdmDataOut->param_data[i + 1] = static_cast<uint8_t>(pText[i]);
    }

    RespondMessageAck();
}
#endif // CONFIG_RDM_ENABLE_SELF_TEST

#if defined(ENABLE_RDM_PRESET_PLAYBACK)
#include "rdm_preset_playback.h"

void RDMHandler::GetPresetPlayback([[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    uint16_t mode;
    uint8_t nLevel;

    rdm::preset_playback::Get(mode, nLevel);

    pRdmDataOut->param_data_length = 3;
    pRdmDataOut->param_data[0] = static_cast<uint8_t>(mode >> 8);
    pRdmDataOut->param_data[1] = static_cast<uint8_t>(mode);
    pRdmDataOut->param_data[2] = nLevel;

    RespondMessageAck();
}

void RDMHandler::SetPresetPlayback([[maybe_unused]] bool is_broadcast, [[maybe_unused]] uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);

    if (pRdmDataIn->param_data_length != 3)
    {
        RespondMessageNack(E120_NR_FORMAT_ERROR);
        return;
    }

    const auto mode = static_cast<uint16_t>((pRdmDataIn->param_data[0] << 8) + pRdmDataIn->param_data[1]);
    const auto nLevel = pRdmDataIn->param_data[2];

    if (!rdm::preset_playback::Set(mode, nLevel))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    pRdmDataOut->param_data_length = 0;

    RespondMessageAck();
}
#endif // ENABLE_RDM_PRESET_PLAYBACK

void RDMHandler::GetSlotInfo(uint16_t sub_device)
{
    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
    const auto nDmxFootPrint = RDMDeviceResponder::Get()->GetDmxFootPrint(sub_device);
    dmxnode::SlotInfo slotInfo;

    uint32_t j = 0;

    for (uint32_t i = 0; i < std::min(static_cast<uint32_t>(nDmxFootPrint), static_cast<uint32_t>(46)); i++)
    {
        if (RDMDeviceResponder::Get()->GetSlotInfo(sub_device, static_cast<uint16_t>(i), slotInfo))
        {
            pRdmDataOut->param_data[j++] = static_cast<uint8_t>(i >> 8);
            pRdmDataOut->param_data[j++] = static_cast<uint8_t>(i);
            pRdmDataOut->param_data[j++] = slotInfo.type;
            pRdmDataOut->param_data[j++] = static_cast<uint8_t>(slotInfo.category >> 8);
            pRdmDataOut->param_data[j++] = static_cast<uint8_t>(slotInfo.category);
        }
    }

    pRdmDataOut->param_data_length = static_cast<uint8_t>(j);
    pRdmDataOut->message_length = static_cast<uint8_t>(RDM_MESSAGE_MINIMUM_SIZE + j);

    RespondMessageAck();
    return;
}

void RDMHandler::GetSlotDescription(uint16_t sub_device)
{
    auto* pRdmDataIn = reinterpret_cast<struct TRdmMessageNoSc*>(m_pRdmDataIn);
    const auto nSlotOffset = static_cast<uint16_t>((pRdmDataIn->param_data[0] << 8) + pRdmDataIn->param_data[1]);
    dmxnode::SlotInfo tSlotInfo;

    if (!RDMDeviceResponder::Get()->GetSlotInfo(sub_device, nSlotOffset, tSlotInfo))
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    uint32_t length;
    const auto* pText = RDMSlotInfo::GetCategoryText(nSlotOffset, tSlotInfo.category, length);

    if (pText == nullptr)
    {
        RespondMessageNack(E120_NR_DATA_OUT_OF_RANGE);
        return;
    }

    if (length > 32)
    {
        length = 32;
    }

    auto* pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

    pRdmDataOut->param_data_length = static_cast<uint8_t>(length + 2);

    pRdmDataOut->param_data[0] = pRdmDataIn->param_data[0];
    pRdmDataOut->param_data[1] = pRdmDataIn->param_data[1];

    for (uint32_t i = 0; i < length; i++)
    {
        pRdmDataOut->param_data[i + 2] = static_cast<uint8_t>(pText[i]);
    }

    RespondMessageAck();
}
#endif
