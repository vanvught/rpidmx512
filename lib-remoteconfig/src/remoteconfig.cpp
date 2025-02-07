/**
 * @file remoteconfig.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cstring>
#include <cassert>

#include "remoteconfig.h"

#include "firmwareversion.h"

#include "hardware.h"
#include "network.h"
#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
# include "net/apps/mdns.h"
#endif
#include "display.h"

#include "properties.h"
#include "propertiesconfig.h"

#include "remoteconfigjson.h"

#include "configstore.h"

/* rconfig.txt */
#include "remoteconfigparams.h"
/* env.txt */
#include "envparams.h"
/* network.txt */
#include "networkparams.h"

#if defined(DISPLAY_UDF)
/* display.txt */
# include "displayudfparams.h"
#endif

/**
 * NODE_
 */

#if defined (NODE_ARTNET)
/* artnet.txt */
# include "artnetnode.h"
# include "artnetparams.h"
#endif

#if defined (NODE_E131)
/* e131.txt */
# include "e131params.h"
#endif

#if defined (NODE_OSC_CLIENT)
/* oscclnt.txt */
# include "oscclientparams.h"
#endif

#if defined (NODE_OSC_SERVER)
/* osc.txt */
# include "oscserverparams.h"
#endif

#if defined (NODE_LTC_SMPTE)
/* ltc.txt */
# include "ltcparams.h"
/* ldisplay.txt */
# include "ltcdisplayparams.h"
/* tcnet.txt */
# include "tcnetparams.h"
/* gps.txt */
# include "gpsparams.h"
/* etc.txt */
# include "ltcetcparams.h"
#endif

#if defined(NODE_SHOWFILE)
/* show.txt */
# include "showfileparams.h"
#endif

#if defined(NODE_NODE)
/* node.txt */
# include "node.h"
# include "nodeparams.h"
#endif

/**
 * OUTPUT_
 */

#if defined (OUTPUT_DMX_SEND)
/* params.txt */
# include "dmxparams.h"
#endif

#if defined (OUTPUT_DMX_PIXEL)
/* devices.txt */
# include "pixeldmxparams.h"
#endif
#if defined (OUTPUT_DMX_TLC59711)
/* devices.txt */
# include "tlc59711dmxparams.h"
#endif

#if defined (OUTPUT_DMX_MONITOR)
/* mon.txt */
# include "dmxmonitorparams.h"
#endif

#if defined(OUTPUT_DMX_STEPPER)
/* sparkfun.txt */
# include "sparkfundmxparams.h"
/* motor%.txt */
# include "modeparams.h"
# include "motorparams.h"
# include "l6470params.h"
#endif

#if defined (OUTPUT_DMX_SERIAL)
/* serial.txt */
# include "dmxserialparams.h"
#endif

#if defined (OUTPUT_RGB_PANEL)
/* rgbpanel.txt */
# include "rgbpanelparams.h"
#endif

#if defined (OUTPUT_DMX_PCA9685)
/* pca9685.txt */
# include "pca9685dmxparams.h"
#endif

/**
 * RDM_
 */

#if defined (RDM_RESPONDER)
/* rdm_device.txt */
# include "rdmdeviceparams.h"
/* sensors.txt */
# include "rdmsensorsparams.h"
/* "subdev.txt" */
# if defined (CONFIG_RDM_ENABLE_SUBDEVICES)
#  include "rdmsubdevicesparams.h"
# endif
#endif

#include "debug.h"


namespace remoteconfig::udp {
static constexpr auto PORT = 0x2905;
namespace get {
enum class Command {
	REBOOT,
	LIST,
	VERSION,
	DISPLAY,
#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
	UPTIME,
# if (defined (NODE_ARTNET) || defined (NODE_NODE)) && (defined (RDM_CONTROLLER) || defined (RDM_RESPONDER))
	RDM,
# endif
	GET,
#endif
	TFTP,
	FACTORY
};
}  // namespace get
namespace set {
enum class Command {
#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
# if (defined (NODE_ARTNET) || defined (NODE_NODE)) && (defined (RDM_CONTROLLER) || defined (RDM_RESPONDER))
	RDM,
# endif
#endif
	TFTP,
	DISPLAY
};
}  // namespace set
} // namespace remoteconfig::udp


constexpr struct RemoteConfig::Commands RemoteConfig::s_GET[] = {
		{ &RemoteConfig::HandleReboot,      "reboot##",  8, false },
		{ &RemoteConfig::HandleList,        "list#",     5, false },
		{ &RemoteConfig::HandleVersion,     "version#",  8, false },
		{ &RemoteConfig::HandleDisplayGet,  "display#",  8, false },
#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
		{ &RemoteConfig::HandleUptime,      "uptime#",   7, false },
# if (defined (NODE_ARTNET) || defined (NODE_NODE)) && (defined (RDM_CONTROLLER) || defined (RDM_RESPONDER))
		{ &RemoteConfig::HandleRdmGet,  	"rdm#",  	 4, false },
# endif
		{ &RemoteConfig::HandleGetNoParams, "get#",      4, true },
#endif
		{ &RemoteConfig::HandleTftpGet,     "tftp#",     5, false },
		{ &RemoteConfig::HandleFactory,     "factory##", 9, false }
};

constexpr struct RemoteConfig::Commands RemoteConfig::s_SET[] = {
#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
# if (defined (NODE_ARTNET) || defined (NODE_NODE)) && (defined (RDM_CONTROLLER) || defined (RDM_RESPONDER))
		{ &RemoteConfig::HandleRdmSet,  	"rdm#",     4, true },
# endif
#endif
		{ &RemoteConfig::HandleTftpSet,    "tftp#",     5, true },
		{ &RemoteConfig::HandleDisplaySet, "display#",  8, true }
};

static constexpr char s_Node[static_cast<uint32_t>(remoteconfig::Node::LAST)][18] = { "Art-Net", "sACN E1.31", "OSC Server", "LTC", "OSC Client", "RDMNet LLRP Only", "Showfile", "MIDI", "DDP", "PixelPusher", "Node", "Bootloader TFTP", "RDM Responder" };
static constexpr char s_Output[static_cast<uint32_t>(remoteconfig::Output::LAST)][12] = { "DMX", "RDM", "Monitor", "Pixel", "TimeCode", "OSC", "Config", "Stepper", "Player", "Art-Net", "Serial", "RGB Panel", "PWM" };

RemoteConfig::RemoteConfig(const remoteconfig::Node node, const remoteconfig::Output output, const uint32_t nActiveOutputs):
	m_Node(node),
	m_Output(output),
	m_nActiveOutputs(nActiveOutputs)
{
	DEBUG_ENTRY

	assert(node < remoteconfig::Node::LAST);
	assert(output < remoteconfig::Output::LAST);

	assert(s_pThis == nullptr);
	s_pThis = this;

	Network::Get()->MacAddressCopyTo(s_RemoteConfigListBin.aMacAddress);
	s_RemoteConfigListBin.nNode = static_cast<uint8_t>(node);
	s_RemoteConfigListBin.nOutput = static_cast<uint8_t>(output);
	s_RemoteConfigListBin.nActiveOutputs = static_cast<uint8_t>(nActiveOutputs);
	s_RemoteConfigListBin.aDisplayName[0] = '\0';

	m_nHandle = Network::Get()->Begin(remoteconfig::udp::PORT, RemoteConfig::StaticCallbackFunction);
	assert(m_nHandle != -1);

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
	mdns_service_record_add(nullptr, mdns::Services::CONFIG);

# if defined(ENABLE_TFTP_SERVER)
	mdns_service_record_add(nullptr, mdns::Services::TFTP);
# endif

# if defined (ENABLE_HTTPD)
	m_pHttpDaemon = new HttpDaemon;
	assert(m_pHttpDaemon != nullptr);
# endif
#endif

	DEBUG_EXIT
}

RemoteConfig::~RemoteConfig() {
	DEBUG_ENTRY

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
# if defined (ENABLE_HTTPD)
	if (m_pHttpDaemon != nullptr) {
		delete m_pHttpDaemon;
	}
# endif

	mdns_service_record_delete(mdns::Services::CONFIG);
#endif

	Network::Get()->End(remoteconfig::udp::PORT);
	m_nHandle = -1;

	DEBUG_EXIT
}

const char *RemoteConfig::GetStringNode() const {
	return s_Node[s_RemoteConfigListBin.nNode];

}
const char *RemoteConfig::GetStringOutput() const {
	return s_Output[s_RemoteConfigListBin.nOutput];
}

void RemoteConfig::SetDisable(bool bDisable) {
	DEBUG_ENTRY

	if (bDisable && !m_bDisable) {
		Network::Get()->End(remoteconfig::udp::PORT);
		m_nHandle = -1;
#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
		mdns_service_record_delete(mdns::Services::CONFIG);
#endif
		m_bDisable = true;
	} else if (!bDisable && m_bDisable) {
		m_nHandle = Network::Get()->Begin(remoteconfig::udp::PORT, RemoteConfig::StaticCallbackFunction);
		assert(m_nHandle != -1);
#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
		mdns_service_record_add(nullptr, mdns::Services::CONFIG);
#endif
		m_bDisable = false;
	}

	DEBUG_PRINTF("m_bDisable=%d", m_bDisable);
	DEBUG_EXIT
}

void RemoteConfig::SetDisplayName(const char *pDisplayName) {
	DEBUG_ENTRY

	strncpy(s_RemoteConfigListBin.aDisplayName, pDisplayName, remoteconfig::DISPLAY_NAME_LENGTH - 1);
	s_RemoteConfigListBin.aDisplayName[remoteconfig::DISPLAY_NAME_LENGTH - 1] = '\0';

#ifndef NDEBUG
	debug_dump(&s_RemoteConfigListBin, sizeof s_RemoteConfigListBin);
#endif

	DEBUG_EXIT
}

void RemoteConfig::Input(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, [[maybe_unused]] uint16_t nFromPort) {
	m_pUdpBuffer = const_cast<char *>(reinterpret_cast<const char *>(pBuffer));
	m_nBytesReceived = nSize;
	m_nIPAddressFrom = nFromIp;
#ifndef NDEBUG
	debug_dump(m_pUdpBuffer, m_nBytesReceived);
#endif

	if (m_pUdpBuffer[m_nBytesReceived - 1] == '\n') {
		m_nBytesReceived--;
	}

	const Commands *pHandler = nullptr;

	if (m_pUdpBuffer[0] == '?') {
		m_nBytesReceived--;
		for (uint32_t i = 0; i < (sizeof(s_GET) / sizeof(s_GET[0])); i++) {
			if ((s_GET[i].bGreaterThan) && (m_nBytesReceived <= s_GET[i].nLength)) {
				continue;
			}
			if ((!s_GET[i].bGreaterThan) && (m_nBytesReceived != s_GET[i].nLength)) {
				continue;
			}
			if (memcmp(&m_pUdpBuffer[1], s_GET[i].pCmd, s_GET[i].nLength) == 0) {
				pHandler = &s_GET[i];
				break;
			}
		}

		if (pHandler != nullptr) {
			(this->*(pHandler->pHandler))();
			return;
		}

		Network::Get()->SendTo(m_nHandle, "ERROR#?\n", 8, m_nIPAddressFrom, remoteconfig::udp::PORT);
		return;
	}

	if (!m_bDisableWrite) {
		if (m_pUdpBuffer[0] == '#') {
#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
			HandleSet(nullptr, 0);
#endif
			return;
		} else if (m_pUdpBuffer[0] == '!') {
			m_nBytesReceived--;
			for (uint32_t i = 0; i < (sizeof(s_SET) / sizeof(s_SET[0])); i++) {
				if ((s_SET[i].bGreaterThan) && (m_nBytesReceived <= s_SET[i].nLength)) {
					continue;
				}
				if ((!s_SET[i].bGreaterThan) && ((m_nBytesReceived - 1U) != s_SET[i].nLength)) {
					continue;
				}
				if (memcmp(&m_pUdpBuffer[1], s_SET[i].pCmd, s_SET[i].nLength) == 0) {
					pHandler = &s_SET[i];
					break;
				}
			}

			if (pHandler != nullptr) {
				(this->*(pHandler->pHandler))();
				return;
			}

			Network::Get()->SendTo(m_nHandle, "ERROR#!\n", 8, m_nIPAddressFrom, remoteconfig::udp::PORT);
			return;
		}
	}
}

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
void RemoteConfig::HandleUptime() {
	DEBUG_ENTRY

	if (!m_bEnableUptime) {
		DEBUG_EXIT
		return;
	}

	const auto nUptime = Hardware::Get()->GetUpTime();
	const auto nLength = snprintf(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE - 1, "uptime: %us\n", static_cast<unsigned int>(nUptime));

	Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, static_cast<uint32_t>(nLength), m_nIPAddressFrom, remoteconfig::udp::PORT);

	DEBUG_EXIT
}
#endif

void RemoteConfig::HandleVersion() {
	DEBUG_ENTRY

	const auto *p = FirmwareVersion::Get()->GetPrint();
	const auto nLength = snprintf(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE - 1, "version:%s\n", p);
	Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, static_cast<uint32_t>(nLength), m_nIPAddressFrom, remoteconfig::udp::PORT);

	DEBUG_EXIT
}

void RemoteConfig::HandleList() {
	DEBUG_ENTRY

	constexpr auto nCmdLength = s_GET[static_cast<uint32_t>(remoteconfig::udp::get::Command::LIST)].nLength;
	auto *pListResponse = &m_pUdpBuffer[nCmdLength + 2U];
	const auto nListResponseBufferLength = remoteconfig::udp::BUFFER_SIZE - (nCmdLength + 2U);
	int32_t nListLength;

	if (s_RemoteConfigListBin.aDisplayName[0] != '\0') {
		nListLength = snprintf(pListResponse, nListResponseBufferLength - 1, "" IPSTR ",%s,%s,%u,%s\n",
				IP2STR(Network::Get()->GetIp()),
				s_Node[static_cast<uint32_t>(m_Node)],
				s_Output[static_cast<uint32_t>(m_Output)],
				static_cast<unsigned int>(m_nActiveOutputs),
				s_RemoteConfigListBin.aDisplayName);
	} else {
		nListLength = snprintf(pListResponse, nListResponseBufferLength - 1, "" IPSTR ",%s,%s,%u\n",
				IP2STR(Network::Get()->GetIp()),
				s_Node[static_cast<uint32_t>(m_Node)],
				s_Output[static_cast<uint32_t>(m_Output)],
				static_cast<unsigned int>(m_nActiveOutputs));
	}

	Network::Get()->SendTo(m_nHandle, pListResponse, static_cast<uint32_t>(nListLength), m_nIPAddressFrom, remoteconfig::udp::PORT);

	DEBUG_EXIT
}

void RemoteConfig::HandleDisplaySet() {
	DEBUG_ENTRY

	constexpr auto nCmdLength = s_SET[static_cast<uint32_t>(remoteconfig::udp::set::Command::DISPLAY)].nLength;

	if (m_nBytesReceived != (nCmdLength + 1U)) {
		DEBUG_EXIT
		return;
	}

	Display::Get()->SetSleep(m_pUdpBuffer[nCmdLength + 1U] == '0');

	DEBUG_PRINTF("%c", m_pUdpBuffer[nCmdLength + 1]);
	DEBUG_EXIT
}

void RemoteConfig::HandleDisplayGet() {
	DEBUG_ENTRY

	const bool isOn = !(Display::Get()->isSleep());
	const auto nLength = snprintf(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE - 1, "display:%s\n", isOn ? "On" : "Off");

	Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, static_cast<uint32_t>(nLength), m_nIPAddressFrom, remoteconfig::udp::PORT);

	DEBUG_EXIT
}

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
#if (defined (NODE_ARTNET) || defined (NODE_NODE)) && (defined (RDM_CONTROLLER) || defined (RDM_RESPONDER))
void RemoteConfig::HandleRdmSet() {
	DEBUG_ENTRY

	const auto nCmdLength = s_SET[static_cast<uint32_t>(remoteconfig::udp::set::Command::RDM)].nLength;

	if (m_nBytesReceived != (nCmdLength + 1U)) {
		DEBUG_EXIT
		return;
	}

	ArtNetNode::Get()->SetRdm(m_pUdpBuffer[nCmdLength + 1U] != '0');

	DEBUG_PRINTF("%c", m_pUdpBuffer[nCmdLength + 1U]);
	DEBUG_EXIT
}

void RemoteConfig::HandleRdmGet() {
	DEBUG_ENTRY

	const bool isOn = ArtNetNode::Get()->GetRdm();
	const auto nLength = snprintf(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE - 1, "rdm:%s\n", isOn ? "On" : "Off");

	Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, static_cast<uint32_t>(nLength), m_nIPAddressFrom, remoteconfig::udp::PORT);

	DEBUG_EXIT
}
#endif
/**
 * GET
 */

uint32_t RemoteConfig::HandleGet(void *pBuffer, uint32_t nBufferLength) {
	DEBUG_ENTRY
	DEBUG_PRINTF("pBuffer=%p, nBufferLength=%u", pBuffer, nBufferLength);

	uint32_t nSize;
	int32_t nIndex;

	constexpr auto nCmdLength = s_GET[static_cast<uint32_t>(remoteconfig::udp::get::Command::GET)].nLength;

	if (pBuffer == nullptr) {
		nSize = remoteconfig::udp::BUFFER_SIZE - nCmdLength;
		assert(m_pUdpBuffer != nullptr);
		nIndex = GetIndex(&m_pUdpBuffer[nCmdLength + 1], nSize);
	} else {
		m_pUdpBuffer = reinterpret_cast<char *>(pBuffer);
		nSize = nBufferLength;
		nIndex = GetIndex(pBuffer, nSize);
	}

	if (nIndex < 0) {
		if (pBuffer == nullptr) {
			Network::Get()->SendTo(m_nHandle, "ERROR#?get\n", 11, m_nIPAddressFrom, remoteconfig::udp::PORT);
		} else {
			memcpy(pBuffer, "ERROR#?get\n", std::min(static_cast<uint32_t>(11), nBufferLength));
		}
		DEBUG_EXIT
		return 12;
	}

	auto *handler = &s_TXT[nIndex];
	(this->*(handler->GetHandler))(nSize);

	if (pBuffer == nullptr) {
		Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, nSize, m_nIPAddressFrom, remoteconfig::udp::PORT);
	} else {
		memcpy(pBuffer, m_pUdpBuffer, std::min(nSize, nBufferLength));
	}

	DEBUG_EXIT
	return nSize;
}

void RemoteConfig::HandleGetRconfigTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetEnvTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	EnvParams envParams;
	envParams.Builder(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetNetworkTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	NetworkParams networkParams;
	networkParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

#if defined (NODE_ARTNET)
void RemoteConfig::HandleGetArtnetTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	ArtNetParams artnetParams;
	artnetParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (NODE_E131)
void RemoteConfig::HandleGetE131Txt(uint32_t& nSize) {
	DEBUG_ENTRY

	E131Params e131params;
	e131params.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (NODE_OSC_SERVER)
void RemoteConfig::HandleGetOscTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	OSCServerParams oscServerParams;
	oscServerParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (NODE_OSC_CLIENT)
void RemoteConfig::HandleGetOscClntTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	OscClientParams oscClientParams;
	oscClientParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (RDM_RESPONDER)
void RemoteConfig::HandleGetRdmDeviceTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	RDMDeviceParams rdmDeviceParams;
	rdmDeviceParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetRdmSensorsTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	RDMSensorsParams rdmSensorsParams;
	rdmSensorsParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

# if defined (CONFIG_RDM_ENABLE_SUBDEVICES)
void RemoteConfig::HandleGetRdmSubdevTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	RDMSubDevicesParams rdmSubDevicesParams;
	rdmSubDevicesParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
# endif
#endif

#if defined (OUTPUT_DMX_SEND)
void RemoteConfig::HandleGetParamsTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DmxParams dmxparams;
	dmxparams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_PIXEL) || defined (OUTPUT_DMX_TLC59711)
void RemoteConfig::HandleGetDevicesTxt(uint32_t& nSize) {
	DEBUG_ENTRY

# if defined (OUTPUT_DMX_TLC59711)
	bool bIsSetLedType = false;

	TLC59711DmxParams tlc5911params;
	tlc5911params.Load();
#  if defined (OUTPUT_DMX_PIXEL)
	if ((bIsSetLedType = tlc5911params.IsSetLedType()) == true) {
#  endif
		tlc5911params.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);
#  if defined (OUTPUT_DMX_PIXEL)
	}
#  endif

	if (!bIsSetLedType) {
# endif
#if defined (OUTPUT_DMX_PIXEL)
		PixelDmxParams pixelDmxParams;
		pixelDmxParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);
#endif
# if defined (OUTPUT_DMX_TLC59711)
	}
#endif

	DEBUG_EXIT
}
#endif

#if defined (NODE_LTC_SMPTE)
void RemoteConfig::HandleGetLtcTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	LtcParams ltcParams;
	ltcParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetLdisplayTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	LtcDisplayParams ltcDisplayParams;
	ltcDisplayParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetTCNetTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	TCNetParams tcnetParams;
	tcnetParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetGpsTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	GPSParams gpsParams;
	gpsParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetLtcEtcTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	LtcEtcParams ltcEtcParams;
	ltcEtcParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(OUTPUT_DMX_MONITOR)
void RemoteConfig::HandleGetMonTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DMXMonitorParams monitorParams;
	monitorParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(DISPLAY_UDF)
void RemoteConfig::HandleGetDisplayTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DisplayUdfParams displayParams;
	displayParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(OUTPUT_DMX_STEPPER)
void RemoteConfig::HandleGetSparkFunTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	SparkFunDmxParams sparkFunParams;
	sparkFunParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetMotorTxt(uint32_t nMotorIndex, uint32_t& nSize) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nMotorIndex=%d", nMotorIndex);

	uint32_t nSizeSparkFun = 0;

	SparkFunDmxParams sparkFunParams;
	sparkFunParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSizeSparkFun, nMotorIndex);

	DEBUG_PRINTF("nSizeSparkFun=%d", nSizeSparkFun);

	uint32_t nSizeMode = 0;

	ModeParams modeParams;
	modeParams.Save(nMotorIndex, m_pUdpBuffer + nSizeSparkFun, remoteconfig::udp::BUFFER_SIZE - nSizeSparkFun, nSizeMode);

	DEBUG_PRINTF("nSizeMode=%d", nSizeMode);

	uint32_t nSizeMotor = 0;

	MotorParams motorParams;
	motorParams.Save(nMotorIndex, m_pUdpBuffer + nSizeSparkFun + nSizeMode, remoteconfig::udp::BUFFER_SIZE - nSizeSparkFun - nSizeMode, nSizeMotor);

	DEBUG_PRINTF("nSizeMotor=%d", nSizeMotor);

	uint32_t nSizeL6470 = 0;

	L6470Params l6470Params;
	l6470Params.Save(nMotorIndex, m_pUdpBuffer + nSizeSparkFun + nSizeMode + nSizeMotor, remoteconfig::udp::BUFFER_SIZE - nSizeSparkFun - nSizeMode - nSizeMotor, nSizeL6470);

	DEBUG_PRINTF("nSizeL6470=%d", nSizeL6470);

	nSize = nSizeSparkFun + nSizeMode + nSizeMotor + nSizeL6470;

	DEBUG_EXIT
}
#endif

#if defined(NODE_SHOWFILE)
void RemoteConfig::HandleGetShowTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	ShowFileParams showFileParams;
	showFileParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(NODE_NODE)
void RemoteConfig::HandleGetNodeTxt(const node::Personality personality, uint32_t& nSize) {
	DEBUG_ENTRY

	NodeParams nodeParams(personality);
	nodeParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_SERIAL)
void RemoteConfig::HandleGetSerialTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DmxSerialParams dmxSerialParams;
	dmxSerialParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_RGB_PANEL)
void RemoteConfig::HandleGetRgbPanelTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	RgbPanelParams rgbPanelParams;
	rgbPanelParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_PCA9685)
void RemoteConfig::HandleGetPca9685Txt(uint32_t& nSize) {
	DEBUG_ENTRY

	PCA9685DmxParams pca9685DmxParams;
	pca9685DmxParams.Save(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#endif

/**
 * SET
 */

void RemoteConfig::HandleSet(void *pBuffer, uint32_t nBufferLength) {
	DEBUG_ENTRY

	int32_t nIndex;
	uint32_t nLength;

	if (pBuffer == nullptr) {
		nLength = remoteconfig::udp::BUFFER_SIZE - 1;
		nIndex = GetIndex(&m_pUdpBuffer[1], nLength);
	} else if (nBufferLength <= remoteconfig::udp::BUFFER_SIZE){
		if (PropertiesConfig::IsJSON() && (reinterpret_cast<char *>(pBuffer)[0] == '{')) {
			DEBUG_PUTS("JSON");
			int c;
			assert(nBufferLength > 1);
			if ((c = properties::convert_json_file(reinterpret_cast<char *>(pBuffer), static_cast<uint16_t>(nBufferLength - 1U), false)) <= 0) {
				DEBUG_EXIT
				return;
			}
			debug_dump(pBuffer, static_cast<uint16_t>(c));
			m_nBytesReceived = static_cast<uint16_t>(c);
		} else {
			m_nBytesReceived = static_cast<uint16_t>(nBufferLength);
		}
		m_pUdpBuffer = reinterpret_cast<char *>(pBuffer);
		nIndex = GetIndex(&m_pUdpBuffer[1], nBufferLength);
	} else {
		DEBUG_EXIT
		return;
	}

	if (nIndex >= 0) {
		DEBUG_PUTS("");
		auto *handler = &s_TXT[nIndex];
		(this->*(handler->SetHandler))();
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleSetRconfigTxt() {
	DEBUG_ENTRY

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load(m_pUdpBuffer, m_nBytesReceived);
	remoteConfigParams.Set(this);

	DEBUG_EXIT
}

void RemoteConfig::HandleSetEnvTxt() {
	DEBUG_ENTRY

	EnvParams params;
	params.LoadAndSet(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}

void RemoteConfig::HandleSetNetworkTxt() {
	DEBUG_ENTRY

	NetworkParams params;
	params.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}

#if defined (NODE_ARTNET)
void RemoteConfig::HandleSetArtnetTxt() {
	DEBUG_ENTRY

	ArtNetParams artnetParams;
	artnetParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined (NODE_E131)
void RemoteConfig::HandleSetE131Txt() {
	DEBUG_ENTRY

	E131Params e131params;
	e131params.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined (NODE_OSC_SERVER)
void RemoteConfig::HandleSetOscTxt() {
	DEBUG_ENTRY

	OSCServerParams oscServerParams;
	oscServerParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined (NODE_OSC_CLIENT)
void RemoteConfig::HandleSetOscClientTxt() {
	DEBUG_ENTRY

	OscClientParams oscClientParams;
	oscClientParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_SEND)
void RemoteConfig::HandleSetParamsTxt() {
	DEBUG_ENTRY

	DmxParams dmxparams;
	dmxparams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_PIXEL) || defined (OUTPUT_DMX_TLC59711)
void RemoteConfig::HandleSetDevicesTxt() {
	DEBUG_ENTRY

# if defined (OUTPUT_DMX_TLC59711)
	TLC59711DmxParams tlc59711params;
	tlc59711params.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_PRINTF("tlc5911params.IsSetLedType()=%d", tlc59711params.IsSetLedType());

	if (!tlc59711params.IsSetLedType()) {
# endif
#if defined (OUTPUT_DMX_PIXEL)
		PixelDmxParams pixelDmxParams;
		pixelDmxParams.Load(m_pUdpBuffer, m_nBytesReceived);
# endif
# if defined (OUTPUT_DMX_TLC59711)
	}
# endif

	DEBUG_EXIT
}
#endif

#if defined (NODE_LTC_SMPTE)
void RemoteConfig::HandleSetLtcTxt() {
	DEBUG_ENTRY

	LtcParams ltcParams;
	ltcParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}

void RemoteConfig::HandleSetLdisplayTxt() {
	DEBUG_ENTRY

	LtcDisplayParams ltcDisplayParams;
	ltcDisplayParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}

void RemoteConfig::HandleSetTCNetTxt() {
	DEBUG_ENTRY

	TCNetParams tcnetParams;
	tcnetParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}

void RemoteConfig::HandleSetGpsTxt() {
	DEBUG_ENTRY

	GPSParams gpsParams;
	gpsParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}

void RemoteConfig::HandleSetLtcEtcTxt() {
	DEBUG_ENTRY

	LtcEtcParams ltcEtcParams;
	ltcEtcParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined(OUTPUT_DMX_MONITOR)
void RemoteConfig::HandleSetMonTxt() {
	DEBUG_ENTRY

	DMXMonitorParams monitorParams;
	monitorParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined(DISPLAY_UDF)
void RemoteConfig::HandleSetDisplayTxt() {
	DEBUG_ENTRY

	DisplayUdfParams displayParams;
	displayParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined(OUTPUT_DMX_STEPPER)
void RemoteConfig::HandleSetSparkFunTxt() {
	DEBUG_ENTRY

	SparkFunDmxParams sparkFunDmxParams;
	sparkFunDmxParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}

void RemoteConfig::HandleSetMotorTxt(uint32_t nMotorIndex) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nMotorIndex=%d", nMotorIndex);

	SparkFunDmxParams sparkFunDmxParams;
	sparkFunDmxParams.Load(nMotorIndex, m_pUdpBuffer, m_nBytesReceived);

	ModeParams modeParams;
	modeParams.Load(nMotorIndex, m_pUdpBuffer, m_nBytesReceived);

	MotorParams motorParams;
	motorParams.Load(nMotorIndex, m_pUdpBuffer, m_nBytesReceived);

	L6470Params l6470Params;
	l6470Params.Load(nMotorIndex, m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined (NODE_SHOWFILE)
void RemoteConfig::HandleSetShowTxt() {
	DEBUG_ENTRY

	ShowFileParams showFileParams;
	showFileParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined (NODE_NODE)
void RemoteConfig::HandleSetNodeTxt(const node::Personality personality) {
	DEBUG_ENTRY

	NodeParams nodeParams(personality);
	nodeParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined (RDM_RESPONDER)
void RemoteConfig::HandleSetRdmDeviceTxt() {
	DEBUG_ENTRY

	RDMDeviceParams rdmDeviceParams;
	rdmDeviceParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}

void RemoteConfig::HandleSetRdmSensorsTxt() {
	DEBUG_ENTRY

	RDMSensorsParams rdmSensorsParams;
	rdmSensorsParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}

# if defined (CONFIG_RDM_ENABLE_SUBDEVICES)
void RemoteConfig::HandleSetRdmSubdevTxt() {
	DEBUG_ENTRY

	RDMSubDevicesParams rdmSubDevicesParams;
	rdmSubDevicesParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
# endif
#endif

#if defined (OUTPUT_DMX_SERIAL)
void RemoteConfig::HandleSetSerialTxt() {
	DEBUG_ENTRY

	DmxSerialParams dmxSerialParams;
	dmxSerialParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_RGB_PANEL)
void RemoteConfig::HandleSetRgbPanelTxt() {
	DEBUG_ENTRY

	RgbPanelParams rgbPanelParams;
	rgbPanelParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_PCA9685)
void RemoteConfig::HandleSetPca9685Txt() {
	DEBUG_ENTRY

	PCA9685DmxParams pca9685DmxParams;
	pca9685DmxParams.Load(m_pUdpBuffer, m_nBytesReceived);

	DEBUG_EXIT
}
#endif

void RemoteConfig::TftpExit() {
	DEBUG_ENTRY

	const auto nCmdLength = s_SET[static_cast<uint32_t>(remoteconfig::udp::set::Command::TFTP)].nLength;

	m_nBytesReceived = nCmdLength + 1U;
	m_pUdpBuffer[nCmdLength + 1] = '0';

	HandleTftpSet();

	DEBUG_EXIT
}

void RemoteConfig::HandleTftpSet() {
	DEBUG_ENTRY

	constexpr auto nCmdLength = s_SET[static_cast<uint32_t>(remoteconfig::udp::set::Command::TFTP)].nLength;

	if (m_nBytesReceived != (nCmdLength + 1U)) {
		DEBUG_EXIT
		return;
	}

	m_bEnableTFTP = (m_pUdpBuffer[nCmdLength + 1U] != '0');

	if (m_bEnableTFTP) {
		Display::Get()->SetSleep(false);
	}

	PlatformHandleTftpSet();

	DEBUG_EXIT
}

void RemoteConfig::HandleTftpGet() {
	DEBUG_ENTRY

	PlatformHandleTftpGet();

	const auto nLength = snprintf(m_pUdpBuffer, remoteconfig::udp::BUFFER_SIZE - 1, "tftp:%s\n", m_bEnableTFTP ? "On" : "Off");
	Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, static_cast<uint32_t>(nLength), m_nIPAddressFrom, remoteconfig::udp::PORT);

	DEBUG_EXIT
}
