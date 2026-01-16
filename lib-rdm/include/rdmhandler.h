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
#include <cstddef>
#if defined(CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
#include <cstring>
#endif

#if defined(NODE_RDMNET_LLRP_ONLY)
#if defined(CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
#undef CONFIG_RDM_ENABLE_MANUFACTURER_PIDS
#endif
#endif

#if defined(ENABLE_RDM_QUEUED_MSG)
#include "rdmqueuedmessage.h"
#endif

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

namespace rdmhandler
{
struct ParameterDescription
{
    const uint16_t pid;
    const uint8_t pdl_size;
    const uint8_t data_type;
    const uint8_t command_class;
    const uint8_t type;
    const uint8_t unit;
    const uint8_t prefix;
    const uint32_t min_value;
    const uint32_t default_value;
    const uint32_t max_value;
    const char* description;
    const uint8_t pdl;
} PACKED;

struct ManufacturerParamData
{
    uint8_t nPdl;
    uint8_t* pParamData;
};
	
bool HandleManufactureerPidGet(uint16_t pid, const ManufacturerParamData* in, ManufacturerParamData* out, uint16_t& reason);
bool HandleManufactureerPidSet(bool is_broadcast, uint16_t pid, const ParameterDescription& parameter_description, const ManufacturerParamData* in, struct ManufacturerParamData* out, uint16_t& reason);

inline constexpr uint32_t kDeviceDescriptionMaxLength = 32;

template <uint16_t Value> struct ManufacturerPid
{
    static constexpr uint16_t kCode = __builtin_bswap16(Value);
    static_assert(kCode >= __builtin_bswap16(0x8000) && kCode <= __builtin_bswap16(0xFFDF), "The manufacturer specific PID must be in range 0x8000 to 0xFFDF");
};

template <typename T, size_t N> struct Description
{
    static constexpr auto kSize = N - 1U;
    static_assert(kSize <= kDeviceDescriptionMaxLength, "Description is too long");
    static constexpr char const* kValue = T::kDescription;
};
} // namespace rdmhandler

class RDMHandler {
public:
	explicit RDMHandler(bool rdm = true);

	void HandleData(const uint8_t *pRdmDataIn, uint8_t *pRdmDataOut);

private:
	void CreateRespondMessage(const uint8_t nResponseType, const uint16_t nReason);
	void RespondMessageAck();
	void RespondMessageNack(const uint16_t nReason);
	void HandleString(const char *pString, const uint32_t nLength);
	void Handlers(bool bIsBroadcast, uint8_t nCommandClass, uint16_t nParamId, uint8_t nParamDataLength, uint16_t subdevice);

	// Get
#if defined (ENABLE_RDM_QUEUED_MSG)
	void GetQueuedMessage(uint16_t subdevice);
#endif
	void GetSupportedParameters(uint16_t subdevice);
#if defined (CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
	void GetParameterDescription(uint16_t subdevice);
	void GetManufacturerPid(uint16_t subdevice);
#endif
	void GetDeviceInfo(uint16_t subdevice);
	void GetProductDetailIdList(uint16_t subdevice);
	void GetDeviceModelDescription(uint16_t subdevice);
	void GetManufacturerLabel(uint16_t subdevice);
	void GetDeviceLabel(uint16_t subdevice);
	void GetFactoryDefaults(uint16_t subdevice);
	void GetLanguage(uint16_t subdevice);
	void GetSoftwareVersionLabel(uint16_t subdevice);
	void GetBootSoftwareVersionId(uint16_t subdevice);
	void GetBootSoftwareVersionLabel(uint16_t subdevice);
	void GetPersonality(uint16_t subdevice);
	void GetPersonalityDescription(uint16_t subdevice);
	void GetDmxStartAddress(uint16_t subdevice);
	void GetSlotInfo(uint16_t subdevice);
	void GetSlotDescription(uint16_t subdevice);
	void GetSensorDefinition(uint16_t subdevice);
	void GetSensorValue(uint16_t subdevice);
	void GetDeviceHours(uint16_t subdevice);
	void GetDisplayInvert(uint16_t subdevice);
	void GetDisplayLevel(uint16_t subdevice);
	void GetIdentifyDevice(uint16_t subdevice);
	void GetRealTimeClock(uint16_t subdevice);
	void GetPowerState(uint16_t subdevice);
#if defined (CONFIG_RDM_ENABLE_SELF_TEST)
	void GetPerformSelfTest(uint16_t subdevice);
	void GetSelfTestDescription(uint16_t subdevice);
#endif
#if defined (ENABLE_RDM_PRESET_PLAYBACK)
	void GetPresetPlayback(uint16_t subdevice);
#endif
	// ANSI E1.37-1
	void GetIdentifyMode(uint16_t subdevice);
	// ANSI E1.37-2 – 2015
	void GetInterfaceList(uint16_t subdevice);
	void GetInterfaceName(uint16_t subdevice);
	void GetHardwareAddress(uint16_t subdevice);
	void GetDHCPMode(uint16_t subdevice);
	void GetZeroconf(uint16_t subdevice);
	void GetAddressNetmask(uint16_t subdevice);
	void GetStaticAddress(uint16_t subdevice);
	void GetDefaultRoute(uint16_t subdevice);
	void GetNameServers(uint16_t subdevice);
	void GetHostName(uint16_t subdevice);
	void GetDomainName(uint16_t subdevice);
	// Set
	void SetDeviceLabel(bool is_broadcast, uint16_t subdevice);
	void SetFactoryDefaults(bool is_broadcast, uint16_t subdevice);
	void SetLanguage(bool is_broadcast, uint16_t subdevice);
	void SetPersonality(bool is_broadcast, uint16_t subdevice);
	void SetDmxStartAddress(bool is_broadcast, uint16_t subdevice);
	void SetSensorValue(bool is_broadcast, uint16_t subdevice);
	void SetRecordSensors(bool is_broadcast, uint16_t subdevice);
	void SetDeviceHours(bool is_broadcast, uint16_t subdevice);
	void SetDisplayInvert(bool is_broadcast, uint16_t subdevice);
	void SetDisplayLevel(bool is_broadcast, uint16_t subdevice);
	void SetIdentifyDevice(bool is_broadcast, uint16_t subdevice);
	void SetRealTimeClock(bool is_broadcast, uint16_t subdevice);
	void SetResetDevice(bool is_broadcast, uint16_t subdevice);
	void SetPowerState(bool is_broadcast, uint16_t subdevice);
#if defined (CONFIG_RDM_ENABLE_SELF_TEST)
	void SetPerformSelfTest(bool is_broadcast, uint16_t subdevice);
#endif
#if defined (ENABLE_RDM_PRESET_PLAYBACK)
	void SetPresetPlayback(bool is_broadcast, uint16_t subdevice);
#endif
#if defined (CONFIG_RDM_ENABLE_MANUFACTURER_PIDS) && defined (CONFIG_RDM_MANUFACTURER_PIDS_SET)
	void SetManufacturerPid(bool is_broadcast, uint16_t subdevice);
#endif
	// ANSI E1.37-1
	void SetIdentifyMode(bool is_broadcast, uint16_t subdevice);
	// ANSI E1.37-2 – 2015
	void SetDHCPMode(bool is_broadcast, uint16_t subdevice);
	void SetAutoIp(bool is_broadcast, uint16_t subdevice);
	void RenewDhcp(bool is_broadcast, uint16_t subdevice);
	void SetStaticAddress(bool is_broadcast, uint16_t subdevice);
	void SetDefaultRoute(bool is_broadcast, uint16_t subdevice);
	void ApplyConfiguration(bool is_broadcast, uint16_t subdevice);
	void SetHostName(bool is_broadcast, uint16_t subdevice);
	void SetDomainName(bool is_broadcast, uint16_t subdevice);
	// ANSI E1.37-2 – 2015
	bool CheckInterfaceID(const struct TRdmMessageNoSc *pRdmDataIn);
	
private:
	bool is_rdm_;
	bool is_muted_ { false };
	uint8_t *m_pRdmDataIn { nullptr };
	uint8_t *m_pRdmDataOut { nullptr };
#if defined (ENABLE_RDM_QUEUED_MSG)
	RDMQueuedMessage m_RDMQueuedMessage;
#endif

struct PidDefinition
{
    const uint16_t nPid;
    void (RDMHandler::*pGetHandler)(uint16_t subdevice);
    void (RDMHandler::*pSetHandler)(bool is_broadcast, uint16_t subdevice);
    const uint8_t nGetArgumentSize;
    const bool bIncludeInSupportedParams;
    const bool bRDM;
    const bool bRDMNet;
};

    static constexpr uint8_t PdlParameterDescription(size_t n)
    {
        return static_cast<uint8_t>(sizeof(struct rdmhandler::ParameterDescription) - sizeof(const char*) - sizeof(const uint8_t) + n);
    }

    static const PidDefinition PID_DEFINITIONS[];
	static const PidDefinition PID_DEFINITIONS_SUB_DEVICES[];
#if defined (CONFIG_RDM_ENABLE_MANUFACTURER_PIDS)
	static const PidDefinition PID_DEFINITION_MANUFACTURER_GENERAL;
	static const rdmhandler::ParameterDescription PARAMETER_DESCRIPTIONS[];

	uint32_t GetParameterDescriptionCount() const;
	void CopyParameterDescription(uint32_t nIndex, uint8_t *pParamData) {
		const auto kSize = sizeof(struct rdmhandler::ParameterDescription) - sizeof(const char *) - sizeof(const uint8_t);
		memcpy(pParamData, &PARAMETER_DESCRIPTIONS[nIndex], kSize);
		memcpy(&pParamData[kSize], PARAMETER_DESCRIPTIONS[nIndex].description, PARAMETER_DESCRIPTIONS[nIndex].pdl - kSize);
	}
#endif
};

#endif  // RDMHANDLER_H_
