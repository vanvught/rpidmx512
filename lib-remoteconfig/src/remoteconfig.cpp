/**
 * @file remoteconfig.cpp
 *
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "remoteconfig.h"

#include "firmwareversion.h"

#include "hardware.h"
#include "network.h"
#include "display.h"

#include "properties.h"
#include "propertiesconfig.h"

#if defined(ENABLE_JSON_ONLY)
# if !defined(DISABLE_BIN)
#  define DISABLE_BIN
# endif
#endif

#include "remoteconfigjson.h"

#include "spiflashstore.h"

/* rconfig.txt */
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"
/* network.txt */
#include "networkparams.h"
#include "storenetwork.h"

#if defined(DISPLAY_UDF)
/* display.txt */
# include "displayudfparams.h"
# include "storedisplayudf.h"
#endif

/**
 * NODE_
 */

#if defined (NODE_ARTNET)
/* artnet.txt */
# include "artnetparams.h"
# include "storeartnet.h"
#endif

#if defined (NODE_E131)
/* e131.txt */
# include "e131params.h"
# include "storee131.h"
#endif

#if defined (NODE_OSC_CLIENT)
/* oscclnt.txt */
# include "oscclientparams.h"
# include "storeoscclient.h"
#endif

#if defined (NODE_OSC_SERVER)
/* osc.txt */
# include "oscserverparams.h"
# include "storeoscserver.h"
#endif

#if defined (NODE_LTC_SMPTE)
/* ltc.txt */
# include "ltcparams.h"
# include "storeltc.h"
/* ldisplay.txt */
# include "ltcdisplayparams.h"
# include "storeltcdisplay.h"
/* tcnet.txt */
# include "tcnetparams.h"
# include "storetcnet.h"
/* gps.txt */
# include "gpsparams.h"
# include "storegps.h"
/* etc.txt */
# include "ltcetcparams.h"
# include "storeltcetc.h"
#endif

#if defined(NODE_SHOWFILE)
/* show.txt */
# include "showfileparams.h"
# include "storeshowfile.h"
#endif

#if defined(NODE_DDP_DISPLAY)
/* ddpdisp.txt */
# include "ddpdisplayparams.h"
# include "storeddpdisplay.h"
#endif

/**
 * OUTPUT_
 */

#if defined (OUTPUT_DMX_SEND)
/* params.txt */
# include "dmxparams.h"
# include "storedmxsend.h"
#endif

#if defined (OUTPUT_DMX_PIXEL)
/* devices.txt */
# include "ws28xxdmxparams.h"
# include "storews28xxdmx.h"
#endif
#if defined (OUTPUT_DMX_TLC59711)
/* devices.txt */
# include "tlc59711dmxparams.h"
# include "storetlc59711.h"
#endif

#if defined (OUTPUT_DMX_MONITOR)
/* mon.txt */
# include "dmxmonitorparams.h"
# include "storemonitor.h"
#endif

#if defined(OUTPUT_DMX_STEPPER)
/* sparkfun.txt */
# include "sparkfundmxparams.h"
# include "storesparkfundmx.h"
/* motor%.txt */
# include "modeparams.h"
# include "motorparams.h"
# include "l6470params.h"
# include "storemotors.h"
#endif

#if defined (OUTPUT_DMX_SERIAL)
/* serial.txt */
# include "dmxserialparams.h"
# include "storedmxserial.h"
#endif

#if defined (OUTPUT_RGB_PANEL)
/* rgbpanel.txt */
# include "rgbpanelparams.h"
# include "storergbpanel.h"
#endif

/**
 * RDM_
 */

#if defined (RDM_RESPONDER)
/* sensors.txt */
# include "rdmsensorsparams.h"
# include "storerdmsensors.h"
/* "subdev.txt" */
# include "rdmsubdevicesparams.h"
# include "storerdmdevice.h"
#endif

#include "debug.h"

namespace remoteconfig {
namespace udp {
static constexpr auto PORT = 0x2905;
namespace get {
enum class Command {
	REBOOT,
	LIST,
	LIST_BROADCAST,
	UPTIME,
	VERSION,
	DISPLAY,
	GET,
#if !defined(DISABLE_BIN)
	STORE,
#endif
	TFTP,
	FACTORY
};
}  // namespace get
namespace set {
enum class Command {
#if !defined(DISABLE_BIN)
	STORE,
#endif
	TFTP,
	DISPLAY
};
}  // namespace set
}  // namespace udp
}  // namespace remoteconfig

const struct RemoteConfig::Commands RemoteConfig::s_GET[] = {
		{ &RemoteConfig::HandleReboot,      "reboot##",  8, false },
		{ &RemoteConfig::HandleList,        "list#",     5, false },
		{ &RemoteConfig::HandleList,        "list#*",    6, false },
		{ &RemoteConfig::HandleUptime,      "uptime#",   7, false },
		{ &RemoteConfig::HandleVersion,     "version#",  8, false },
		{ &RemoteConfig::HandleDisplayGet,  "display#",  8, false },
		{ &RemoteConfig::HandleGetNoParams, "get#",      4, true },
#if !defined(DISABLE_BIN)
		{ &RemoteConfig::HandleStoreGet,    "store#",    6, true },
#endif
		{ &RemoteConfig::HandleTftpGet,     "tftp#",     5, false },
		{ &RemoteConfig::HandleFactory,     "factory##", 9, false }
};

const struct RemoteConfig::Commands RemoteConfig::s_SET[] = {
#if !defined(DISABLE_BIN)
		{ &RemoteConfig::HandleStoreSet,   "store#",    6, true },
#endif
		{ &RemoteConfig::HandleTftpSet,    "tftp#",     5, true },
		{ &RemoteConfig::HandleDisplaySet, "display#",  8, true }
};

using namespace remoteconfig;

static constexpr char s_Node[static_cast<uint32_t>(Node::LAST)][18] = { "Art-Net", "sACN E1.31", "OSC Server", "LTC", "OSC Client", "RDMNet LLRP Only", "Showfile", "MIDI", "DDP" };
static constexpr char s_Output[static_cast<uint32_t>(Output::LAST)][12] = { "DMX", "RDM", "Monitor", "Pixel", "TimeCode", "OSC", "Config", "Stepper", "Player", "Art-Net", "Serial", "RGB Panel" };

RemoteConfig *RemoteConfig::s_pThis;
RemoteConfig::ListBin RemoteConfig::s_RemoteConfigListBin;
char *RemoteConfig::s_pUdpBuffer;
#if !defined(DISABLE_BIN)
uint8_t RemoteConfig::s_StoreBuffer[udp::BUFFER_SIZE];
#endif

RemoteConfig::RemoteConfig(Node node, Output output, uint32_t nActiveOutputs): m_tNode(node), m_tOutput(output), m_nActiveOutputs(nActiveOutputs) {
	DEBUG_ENTRY

	assert(node < Node::LAST);
	assert(output < Output::LAST);

	assert(s_pThis == nullptr);
	s_pThis = this;

	Network::Get()->MacAddressCopyTo(s_RemoteConfigListBin.aMacAddress);
	s_RemoteConfigListBin.nNode = static_cast<uint8_t>(node);
	s_RemoteConfigListBin.nOutput = static_cast<uint8_t>(output);
	s_RemoteConfigListBin.nActiveOutputs = static_cast<uint8_t>(nActiveOutputs);
	s_RemoteConfigListBin.aDisplayName[0] = '\0';

#ifndef NDEBUG
	DEBUG_PUTS("s_RemoteConfigListBin");
	debug_dump(&s_RemoteConfigListBin, sizeof s_RemoteConfigListBin);
#endif

	m_nHandle = Network::Get()->Begin(udp::PORT);
	assert(m_nHandle != -1);

#if defined(ENABLE_HTTPD)
	m_HttpDaemon.Start();
#endif

	DEBUG_EXIT
}

RemoteConfig::~RemoteConfig() {
	DEBUG_ENTRY

	Network::Get()->End(udp::PORT);
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
		Network::Get()->End(udp::PORT);
		m_nHandle = -1;
		m_bDisable = true;
	} else if (!bDisable && m_bDisable) {
		m_nHandle = Network::Get()->Begin(udp::PORT);
		assert(m_nHandle != -1);
		m_bDisable = false;
	}

	DEBUG_PRINTF("m_bDisable=%d", m_bDisable);
	DEBUG_EXIT
}

void RemoteConfig::SetDisplayName(const char *pDisplayName) {
	DEBUG_ENTRY

	strncpy(s_RemoteConfigListBin.aDisplayName, pDisplayName, DISPLAY_NAME_LENGTH - 1);
	s_RemoteConfigListBin.aDisplayName[DISPLAY_NAME_LENGTH - 1] = '\0';

#ifndef NDEBUG
	debug_dump(&s_RemoteConfigListBin, sizeof s_RemoteConfigListBin);
#endif

	DEBUG_EXIT
}

void RemoteConfig::Run() {
	if (__builtin_expect((m_bDisable), 1)) {
		return;
	}

#if defined (ENABLE_TFTP_SERVER)
	if (__builtin_expect((m_pTFTPFileServer != nullptr), 0)) {
		m_pTFTPFileServer->Run();
	}
#endif

#if defined (ENABLE_HTTPD)
	m_HttpDaemon.Run();
#endif

	uint16_t nForeignPort;
	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&s_pUdpBuffer)), &m_nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived < 4), 1)) {
		return;
	}

#ifndef NDEBUG
//	debug_dump(s_pUdpBuffer, m_nBytesReceived);
#endif

	if (s_pUdpBuffer[m_nBytesReceived - 1] == '\n') {
		m_nBytesReceived--;
	}

	const Commands *pHandler = nullptr;

	if (s_pUdpBuffer[0] == '?') {
		m_nBytesReceived--;
		for (uint32_t i = 0; i < (sizeof(s_GET) / sizeof(s_GET[0])); i++) {
			if ((s_GET[i].bGreaterThan) && (m_nBytesReceived <= s_GET[i].nLength)) {
				continue;
			}
			if ((!s_GET[i].bGreaterThan) && (m_nBytesReceived != s_GET[i].nLength)) {
				continue;
			}
			if (memcmp(&s_pUdpBuffer[1], s_GET[i].pCmd, s_GET[i].nLength) == 0) {
				pHandler = &s_GET[i];
				break;
			}
		}

		if (pHandler != nullptr) {
			(this->*(pHandler->pHandler))();
			return;
		}

		Network::Get()->SendTo(m_nHandle, "ERROR#?\n", 8, m_nIPAddressFrom, udp::PORT);
		return;
	}

	if (!m_bDisableWrite) {
		if (s_pUdpBuffer[0] == '#') {
			m_tHandleMode = HandleMode::TXT;
			HandleSet(nullptr, 0);
			return;
		} else if (s_pUdpBuffer[0] == '!') {
			m_nBytesReceived--;
			for (uint32_t i = 0; i < (sizeof(s_SET) / sizeof(s_SET[0])); i++) {
				if ((s_SET[i].bGreaterThan) && (m_nBytesReceived <= s_SET[i].nLength)) {
					continue;
				}
				if ((!s_SET[i].bGreaterThan) && ((m_nBytesReceived - 1U) != s_SET[i].nLength)) {
					continue;
				}
				if (memcmp(&s_pUdpBuffer[1], s_SET[i].pCmd, s_SET[i].nLength) == 0) {
					pHandler = &s_SET[i];
					break;
				}
			}

			if (pHandler != nullptr) {
				(this->*(pHandler->pHandler))();
				return;
			}

			Network::Get()->SendTo(m_nHandle, "ERROR#!\n", 8, m_nIPAddressFrom, udp::PORT);
			return;
		}
	}
}

void RemoteConfig::HandleUptime() {
	DEBUG_ENTRY

	if (!m_bEnableUptime) {
		DEBUG_EXIT
		return;
	}

	const auto nUptime = Hardware::Get()->GetUpTime();
	const auto nCmdLength = s_GET[static_cast<uint32_t>(udp::get::Command::UPTIME)].nLength;

	if (m_nBytesReceived == nCmdLength) {
		const auto nLength = snprintf(s_pUdpBuffer, udp::BUFFER_SIZE - 1, "uptime: %us\n", nUptime);
		Network::Get()->SendTo(m_nHandle, s_pUdpBuffer, static_cast<uint16_t>(nLength), m_nIPAddressFrom, udp::PORT);
		DEBUG_EXIT
		return;
	}
#if !defined(DISABLE_BIN)
	else if (m_nBytesReceived == nCmdLength + 3) {
		if (memcmp(&s_pUdpBuffer[nCmdLength], "bin", 3) == 0) {
			Network::Get()->SendTo(m_nHandle, &nUptime, sizeof(uint32_t) , m_nIPAddressFrom, udp::PORT);
			DEBUG_EXIT
			return;
		}
	}
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleVersion() {
	DEBUG_ENTRY
	const auto nCmdLength = s_GET[static_cast<uint32_t>(udp::get::Command::VERSION)].nLength;

	DEBUG_PRINTF("%u:%u", m_nBytesReceived, nCmdLength);

	if (m_nBytesReceived == nCmdLength) {
		const auto *p = FirmwareVersion::Get()->GetPrint();
		const auto nLength = snprintf(s_pUdpBuffer, udp::BUFFER_SIZE - 1, "version:%s", p);
		Network::Get()->SendTo(m_nHandle, s_pUdpBuffer, static_cast<uint16_t>(nLength), m_nIPAddressFrom, udp::PORT);
		DEBUG_EXIT
		return;
	}
#if !defined(DISABLE_BIN)
	else if (m_nBytesReceived == nCmdLength + 3) {
		if (memcmp(&s_pUdpBuffer[nCmdLength], "bin", 3) == 0) {
			const auto *p = reinterpret_cast<const uint8_t*>(FirmwareVersion::Get()->GetVersion());
			Network::Get()->SendTo(m_nHandle, p, sizeof(struct firmwareversion::Info) , m_nIPAddressFrom, udp::PORT);
			DEBUG_EXIT
			return;
		}
	}
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleList() {
	DEBUG_ENTRY

	const auto nCmdLength = s_GET[static_cast<uint32_t>(udp::get::Command::LIST)].nLength;
	auto *pListResponse = &s_pUdpBuffer[nCmdLength + 2U];
	const auto nListResponseBufferLength = udp::BUFFER_SIZE - (nCmdLength + 2U);
	int32_t nListLength;

	if (s_RemoteConfigListBin.aDisplayName[0] != '\0') {
		nListLength = snprintf(pListResponse, nListResponseBufferLength - 1, "" IPSTR ",%s,%s,%d,%s\n",
				IP2STR(Network::Get()->GetIp()),
				s_Node[static_cast<uint32_t>(m_tNode)],
				s_Output[static_cast<uint32_t>(m_tOutput)],
				m_nActiveOutputs,
				s_RemoteConfigListBin.aDisplayName);
	} else {
		nListLength = snprintf(pListResponse, nListResponseBufferLength - 1, "" IPSTR ",%s,%s,%d\n",
				IP2STR(Network::Get()->GetIp()),
				s_Node[static_cast<uint32_t>(m_tNode)],
				s_Output[static_cast<uint32_t>(m_tOutput)],
				m_nActiveOutputs);
	}

	if (m_nBytesReceived == nCmdLength) {
		Network::Get()->SendTo(m_nHandle, pListResponse, static_cast<uint16_t>(nListLength), m_nIPAddressFrom, udp::PORT);
		DEBUG_EXIT
		return;
	} else if (m_nBytesReceived == nCmdLength + 1) {
		DEBUG_PRINTF("%c", nCmdLength + 1);
		if (s_pUdpBuffer[nCmdLength + 1] == '*') {
			Network::Get()->SendTo(m_nHandle, pListResponse, static_cast<uint16_t>(nListLength), network::IP4_BROADCAST, udp::PORT);
			DEBUG_EXIT
			return;
		}
	}
#if !defined(DISABLE_BIN)
	else if (m_nBytesReceived == nCmdLength + 3) {
		if (memcmp(&s_pUdpBuffer[nCmdLength], "bin", 3) == 0) {
			Network::Get()->SendTo(m_nHandle, &s_RemoteConfigListBin, sizeof(struct ListBin) , m_nIPAddressFrom, udp::PORT);
			DEBUG_EXIT
			return;
		}
	} else if (m_nBytesReceived == nCmdLength + 4) {
		if (memcmp(&s_pUdpBuffer[nCmdLength], "bin*", 4) == 0) {
			Network::Get()->SendTo(m_nHandle, &s_RemoteConfigListBin, sizeof(struct ListBin) , network::IP4_BROADCAST, udp::PORT);
			DEBUG_EXIT
			return;
		}
	}
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleDisplaySet() {
	DEBUG_ENTRY

	const auto nCmdLength = s_SET[static_cast<uint32_t>(udp::set::Command::DISPLAY)].nLength;

	if (m_nBytesReceived != (nCmdLength + 1)) {
		DEBUG_EXIT
		return;
	}

	Display::Get()->SetSleep(s_pUdpBuffer[nCmdLength + 1] == '0');

	DEBUG_PRINTF("%c", s_pUdpBuffer[nCmdLength + 1]);
	DEBUG_EXIT
}

void RemoteConfig::HandleDisplayGet() {
	DEBUG_ENTRY

	const auto nCmdLength = s_GET[static_cast<uint32_t>(udp::get::Command::DISPLAY)].nLength;
	const bool isOn = !(Display::Get()->isSleep());

	if (m_nBytesReceived == nCmdLength) {
		const auto nLength = snprintf(s_pUdpBuffer, udp::BUFFER_SIZE - 1, "display:%s\n", isOn ? "On" : "Off");
		Network::Get()->SendTo(m_nHandle, s_pUdpBuffer, static_cast<uint16_t>(nLength), m_nIPAddressFrom, udp::PORT);
	}
#if !defined(DISABLE_BIN)
	else if (m_nBytesReceived == nCmdLength + 3) {
		if (memcmp(&s_pUdpBuffer[nCmdLength], "bin", 3) == 0) {
			Network::Get()->SendTo(m_nHandle, &isOn, sizeof(bool) , m_nIPAddressFrom, udp::PORT);
		}
	}
#endif

	DEBUG_EXIT
}

/**
 * GET
 */

#if !defined(DISABLE_BIN)
void RemoteConfig::HandleStoreGet() {
	DEBUG_ENTRY

	const auto nCmdLength = s_GET[static_cast<uint32_t>(udp::get::Command::STORE)].nLength;
	uint32_t nLength = udp::BUFFER_SIZE - nCmdLength;
	const auto nIndex = GetIndex(&s_pUdpBuffer[nCmdLength], nLength);

	if (nIndex >= 0) {
		SpiFlashStore::Get()->CopyTo(s_TXT[nIndex].nStore, s_pUdpBuffer, nLength);
	} else {
		Network::Get()->SendTo(m_nHandle, "ERROR#?store\n", 13, m_nIPAddressFrom, udp::PORT);
		return;
	}

#ifndef NDEBUG
//	debug_dump(s_pUdpBuffer, static_cast<uint16_t>(nLength));
#endif
	Network::Get()->SendTo(m_nHandle, s_pUdpBuffer, static_cast<uint16_t>(nLength), m_nIPAddressFrom, udp::PORT);

	DEBUG_EXIT
}
#endif

uint32_t RemoteConfig::HandleGet(void *pBuffer, uint32_t nBufferLength) {
	DEBUG_ENTRY

	uint32_t nSize;
	int32_t nIndex;

	const auto nCmdLength = s_GET[static_cast<uint32_t>(udp::get::Command::GET)].nLength;

	if (pBuffer == nullptr) {
		nSize = udp::BUFFER_SIZE - nCmdLength;
		assert(s_pUdpBuffer != nullptr);
		nIndex = GetIndex(&s_pUdpBuffer[nCmdLength + 1], nSize);
	} else {
		s_pUdpBuffer = reinterpret_cast<char *>(pBuffer);
		nSize = nBufferLength;
		nIndex = GetIndex(pBuffer, nSize);
	}

	if (nIndex < 0) {
		if (pBuffer == nullptr) {
			Network::Get()->SendTo(m_nHandle, "ERROR#?get\n", 11, m_nIPAddressFrom, udp::PORT);
		} else {
			memcpy(pBuffer, "ERROR#?get\n", std::min(11U, nBufferLength));
		}
		DEBUG_EXIT
		return 12;
	}

	auto *handler = &s_TXT[nIndex];
	(this->*(handler->GetHandler))(nSize);

	if (pBuffer == nullptr) {
		Network::Get()->SendTo(m_nHandle, s_pUdpBuffer, static_cast<uint16_t>(nSize), m_nIPAddressFrom, udp::PORT);
	} else {
		memcpy(pBuffer, s_pUdpBuffer, std::min(nSize, nBufferLength));
	}

	DEBUG_EXIT
	return nSize;
}

void RemoteConfig::HandleGetRconfigTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	RemoteConfigParams remoteConfigParams(StoreRemoteConfig::Get());
	remoteConfigParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetNetworkTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	NetworkParams networkParams(StoreNetwork::Get());
	networkParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

#if defined (NODE_ARTNET)
void RemoteConfig::HandleGetArtnetTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	assert(StoreArtNet::Get() != nullptr);
	ArtNetParams artnetParams(StoreArtNet::Get());
	artnetParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (NODE_E131)
void RemoteConfig::HandleGetE131Txt(uint32_t& nSize) {
	DEBUG_ENTRY

	assert(StoreE131::Get() != nullptr);
	E131Params e131params(StoreE131::Get());
	e131params.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (NODE_OSC_SERVER)
void RemoteConfig::HandleGetOscTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	OSCServerParams oscServerParams(StoreOscServer::Get());
	oscServerParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (NODE_OSC_CLIENT)
void RemoteConfig::HandleGetOscClntTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	OscClientParams oscClientParams(StoreOscClient::Get());
	oscClientParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_SEND)
void RemoteConfig::HandleGetParamsTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DmxParams dmxparams(StoreDmxSend::Get());
	dmxparams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_PIXEL) || defined (OUTPUT_DMX_TLC59711)
void RemoteConfig::HandleGetDevicesTxt(uint32_t& nSize) {
	DEBUG_ENTRY

# if defined (OUTPUT_DMX_TLC59711)
	bool bIsSetLedType = false;

	TLC59711DmxParams tlc5911params(StoreTLC59711::Get());

	if (tlc5911params.Load()) {
#  if defined (OUTPUT_DMX_PIXEL)
		if ((bIsSetLedType = tlc5911params.IsSetLedType()) == true) {
#  endif
			tlc5911params.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
#  if defined (OUTPUT_DMX_PIXEL)
		}
#  endif
	}

	if (!bIsSetLedType) {
# endif
#if defined (OUTPUT_DMX_PIXEL)
		WS28xxDmxParams ws28xxparms(StoreWS28xxDmx::Get());
		ws28xxparms.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
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

	LtcParams ltcParams(StoreLtc::Get());
	ltcParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetLdisplayTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	LtcDisplayParams ltcDisplayParams(StoreLtcDisplay::Get());
	ltcDisplayParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetTCNetTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	TCNetParams tcnetParams(StoreTCNet::Get());
	tcnetParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetGpsTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	GPSParams gpsParams(StoreGPS::Get());
	gpsParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetLtcEtcTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	LtcEtcParams ltcEtcParams(StoreLtcEtc::Get());
	ltcEtcParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(OUTPUT_DMX_MONITOR)
void RemoteConfig::HandleGetMonTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DMXMonitorParams monitorParams(StoreMonitor::Get());
	monitorParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(DISPLAY_UDF)
void RemoteConfig::HandleGetDisplayTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DisplayUdfParams displayParams(StoreDisplayUdf::Get());
	displayParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(OUTPUT_DMX_STEPPER)
void RemoteConfig::HandleGetSparkFunTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	SparkFunDmxParams sparkFunParams(StoreSparkFunDmx::Get());
	sparkFunParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetMotorTxt(uint32_t nMotorIndex, uint32_t& nSize) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nMotorIndex=%d", nMotorIndex);

	uint32_t nSizeSparkFun = 0;

	SparkFunDmxParams sparkFunParams(StoreSparkFunDmx::Get());
	sparkFunParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSizeSparkFun, nMotorIndex);

	DEBUG_PRINTF("nSizeSparkFun=%d", nSizeSparkFun);

	uint32_t nSizeMode = 0;

	ModeParams modeParams(StoreMotors::Get());
	modeParams.Save(nMotorIndex, s_pUdpBuffer + nSizeSparkFun, udp::BUFFER_SIZE - nSizeSparkFun, nSizeMode);

	DEBUG_PRINTF("nSizeMode=%d", nSizeMode);

	uint32_t nSizeMotor = 0;

	MotorParams motorParams(StoreMotors::Get());
	motorParams.Save(nMotorIndex, s_pUdpBuffer + nSizeSparkFun + nSizeMode, udp::BUFFER_SIZE - nSizeSparkFun - nSizeMode, nSizeMotor);

	DEBUG_PRINTF("nSizeMotor=%d", nSizeMotor);

	uint32_t nSizeL6470 = 0;

	L6470Params l6470Params(StoreMotors::Get());
	l6470Params.Save(nMotorIndex, s_pUdpBuffer + nSizeSparkFun + nSizeMode + nSizeMotor, udp::BUFFER_SIZE - nSizeSparkFun - nSizeMode - nSizeMotor, nSizeL6470);

	DEBUG_PRINTF("nSizeL6470=%d", nSizeL6470);

	nSize = nSizeSparkFun + nSizeMode + nSizeMotor + nSizeL6470;

	DEBUG_EXIT
}
#endif

#if defined(NODE_SHOWFILE)
void RemoteConfig::HandleGetShowTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	ShowFileParams showFileParams(StoreShowFile::Get());
	showFileParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_SERIAL)
void RemoteConfig::HandleGetSerialTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DmxSerialParams dmxSerialParams(StoreDmxSerial::Get());
	dmxSerialParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_RGB_PANEL)
void RemoteConfig::HandleGetRgbPanelTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	RgbPanelParams rgbPanelParams(StoreRgbPanel::Get());
	rgbPanelParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (NODE_DDP_DISPLAY)
void RemoteConfig::HandleGetDdpDisplayTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DdpDisplayParams ddpDisplayParams(StoreDdpDisplay::Get());
	ddpDisplayParams.Save(s_pUdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

/**
 * SET
 */

void RemoteConfig::HandleSet(void *pBuffer, uint32_t nBufferLength) {
	DEBUG_ENTRY

	int32_t nIndex;
	uint32_t nLength;

	if (pBuffer == nullptr) {
#if !defined(DISABLE_BIN)
		if (m_tHandleMode == HandleMode::TXT) {
#endif
			nLength = udp::BUFFER_SIZE - 1;
			nIndex = GetIndex(&s_pUdpBuffer[1], nLength);
#if !defined(DISABLE_BIN)
		} else {
			const auto nCmdLength = s_SET[static_cast<uint32_t>(udp::set::Command::STORE)].nLength;
			nLength = udp::BUFFER_SIZE - nCmdLength;
			nIndex = GetIndex(&s_pUdpBuffer[nCmdLength], nLength);
			if (nIndex >= 0) {
				m_nBytesReceived = static_cast<uint16_t>(m_nBytesReceived - nLength - nCmdLength);
				memcpy(s_StoreBuffer, &s_pUdpBuffer[nLength + nCmdLength], udp::BUFFER_SIZE);
//				debug_dump(s_StoreBuffer, m_nBytesReceived);
			} else {
				DEBUG_EXIT
				return;
			}
		}
#endif
	} else if (nBufferLength <= udp::BUFFER_SIZE){
		DEBUG_PUTS("");
		m_tHandleMode = HandleMode::TXT;
		if (PropertiesConfig::IsJSON() && (reinterpret_cast<char *>(pBuffer)[0] == '{')) {
			DEBUG_PUTS("JSON");
			int c;
			assert(nBufferLength > 1);
			if ((c = properties::convert_json_file(reinterpret_cast<char *>(pBuffer), static_cast<uint16_t>(nBufferLength - 1U))) <= 0) {
				DEBUG_EXIT
				return;
			}
			debug_dump(pBuffer, static_cast<uint16_t>(c));
			m_nBytesReceived = static_cast<uint16_t>(c);
		} else {
			m_nBytesReceived = static_cast<uint16_t>(nBufferLength);
		}
		s_pUdpBuffer = reinterpret_cast<char *>(pBuffer);
		nIndex = GetIndex(&s_pUdpBuffer[1], nBufferLength);
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

void RemoteConfig::HandleSetRconfig() {
	DEBUG_ENTRY

	RemoteConfigParams remoteConfigParams(StoreRemoteConfig::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
	        if (m_nBytesReceived == sizeof(struct TRemoteConfigParams)) {
		        uint32_t nSize;
		        remoteConfigParams.Builder(reinterpret_cast<const struct TRemoteConfigParams*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
		        m_nBytesReceived = nSize;
		} else {
		        DEBUG_EXIT
			return;
		}
	}
#endif

	remoteConfigParams.Load(s_pUdpBuffer, m_nBytesReceived);
	remoteConfigParams.Set(this);
#ifndef NDEBUG
	remoteConfigParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleSetNetworkTxt() {
	DEBUG_ENTRY

	NetworkParams params(StoreNetwork::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TNetworkParams)) {
			uint32_t nSize;
			params.Builder(reinterpret_cast<const struct TNetworkParams*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	params.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	params.Dump();
#endif

	DEBUG_EXIT
}

#if defined (NODE_ARTNET)
void RemoteConfig::HandleSetArtnetTxt() {
	DEBUG_ENTRY

	assert(StoreArtNet::Get() != nullptr);
	ArtNetParams artnetParams(StoreArtNet::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct artnetparams::Params)) {
			uint32_t nSize;
			artnetParams.Builder(reinterpret_cast<const struct artnetparams::Params*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	artnetParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	artnetParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (NODE_E131)
void RemoteConfig::HandleSetE131Txt() {
	DEBUG_ENTRY

	assert(StoreE131::Get() != nullptr);
	E131Params e131params(StoreE131::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct e131params::Params)) {
			uint32_t nSize;
			e131params.Builder(reinterpret_cast<const struct e131params::Params*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
		
	}
#endif

	e131params.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	e131params.Dump();
#endif
	DEBUG_EXIT
}
#endif

#if defined (NODE_OSC_SERVER)
void RemoteConfig::HandleSetOscTxt() {
	DEBUG_ENTRY

	OSCServerParams oscServerParams(StoreOscServer::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct osc::server::Params)) {
			uint32_t nSize;
			oscServerParams.Builder(reinterpret_cast<const struct osc::server::Params*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	oscServerParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	oscServerParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (NODE_OSC_CLIENT)
void RemoteConfig::HandleSetOscClientTxt() {
	DEBUG_ENTRY

	OscClientParams oscClientParams(StoreOscClient::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TOscClientParams)) {
			uint32_t nSize;
			oscClientParams.Builder(reinterpret_cast<const struct TOscClientParams *>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	oscClientParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	oscClientParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_SEND)
void RemoteConfig::HandleSetParamsTxt() {
	DEBUG_ENTRY

	DmxParams dmxparams(StoreDmxSend::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TDmxParams)) {
			uint32_t nSize;
			dmxparams.Builder(reinterpret_cast<const struct TDmxParams *>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	dmxparams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	dmxparams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_PIXEL) || defined (OUTPUT_DMX_TLC59711)
void RemoteConfig::HandleSetDevicesTxt() {
	DEBUG_ENTRY

# if defined (OUTPUT_DMX_TLC59711)
#  if defined (OUTPUT_DMX_PIXEL)
	static_assert(sizeof(struct TTLC59711DmxParams) != sizeof(struct TWS28xxDmxParams), "");
#  endif
	TLC59711DmxParams tlc59711params(StoreTLC59711::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TTLC59711DmxParams)) {
			uint32_t nSize;
			tlc59711params.Builder(reinterpret_cast<const struct TTLC59711DmxParams*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	tlc59711params.Load(s_pUdpBuffer, m_nBytesReceived);
#  ifndef NDEBUG
	tlc59711params.Dump();
#  endif
	DEBUG_PRINTF("tlc5911params.IsSetLedType()=%d", tlc59711params.IsSetLedType());

	if (!tlc59711params.IsSetLedType()) {
# endif
#if defined (OUTPUT_DMX_PIXEL)
		WS28xxDmxParams ws28xxparms(StoreWS28xxDmx::Get());

#if !defined(DISABLE_BIN)
		if (m_tHandleMode == HandleMode::BIN) {
			if (m_nBytesReceived == sizeof(struct TWS28xxDmxParams)) {
				uint32_t nSize;
				ws28xxparms.Builder(reinterpret_cast<const struct TWS28xxDmxParams *>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
				m_nBytesReceived = nSize;
			} else {
				DEBUG_EXIT
				return;
			}
		}
#  endif
		ws28xxparms.Load(s_pUdpBuffer, m_nBytesReceived);
#  ifndef NDEBUG
		ws28xxparms.Dump();
#  endif
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

	LtcParams ltcParams(StoreLtc::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TLtcParams)) {
			uint32_t nSize;
			ltcParams.Builder(reinterpret_cast<const struct TLtcParams *>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	ltcParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	ltcParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleSetLdisplayTxt() {
	DEBUG_ENTRY

	LtcDisplayParams ltcDisplayParams(StoreLtcDisplay::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TLtcDisplayParams)) {
			uint32_t nSize;
			ltcDisplayParams.Builder(reinterpret_cast<const struct TLtcDisplayParams *>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	ltcDisplayParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	ltcDisplayParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleSetTCNetTxt() {
	DEBUG_ENTRY

	TCNetParams tcnetParams(StoreTCNet::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct tcnetparams::Params)) {
			uint32_t nSize;
			tcnetParams.Builder(reinterpret_cast<const struct tcnetparams::Params*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	tcnetParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	tcnetParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleSetGpsTxt() {
	DEBUG_ENTRY

	GPSParams gpsParams(StoreGPS::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TGPSParams)) {
			uint32_t nSize;
			gpsParams.Builder(reinterpret_cast<const struct TGPSParams*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	gpsParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	gpsParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleSetLtcEtcTxt() {
	DEBUG_ENTRY

	LtcEtcParams ltcEtcParams(StoreLtcEtc::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct ltcetcparams::Params)) {
			uint32_t nSize;
			ltcEtcParams.Builder(reinterpret_cast<const struct ltcetcparams::Params *>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	ltcEtcParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	ltcEtcParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined(OUTPUT_DMX_MONITOR)
void RemoteConfig::HandleSetMonTxt() {
	DEBUG_ENTRY

	DMXMonitorParams monitorParams(StoreMonitor::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TDMXMonitorParams)) {
			uint32_t nSize;
			monitorParams.Builder(reinterpret_cast<const struct TDMXMonitorParams*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	monitorParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	monitorParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined(DISPLAY_UDF)
void RemoteConfig::HandleSetDisplayTxt() {
	DEBUG_ENTRY

	DisplayUdfParams displayParams(StoreDisplayUdf::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct displayudfparams::Params)) {
			uint32_t nSize;
			displayParams.Builder(reinterpret_cast<const struct displayudfparams::Params*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	displayParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	displayParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined(OUTPUT_DMX_STEPPER)
void RemoteConfig::HandleSetSparkFunTxt() {
	DEBUG_ENTRY

	SparkFunDmxParams sparkFunDmxParams(StoreSparkFunDmx::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TSparkFunDmxParams)) {
			uint32_t nSize;
			sparkFunDmxParams.Builder(reinterpret_cast<const struct TSparkFunDmxParams*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	sparkFunDmxParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	sparkFunDmxParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleSetMotorTxt(uint32_t nMotorIndex) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nMotorIndex=%d", nMotorIndex);

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		// TODO HandleTxtFileMotor HandleMode::BIN
		return;
	}
#endif

	SparkFunDmxParams sparkFunDmxParams(StoreSparkFunDmx::Get());
	sparkFunDmxParams.Load(nMotorIndex, s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	sparkFunDmxParams.Dump();
#endif

	ModeParams modeParams(StoreMotors::Get());
	modeParams.Load(nMotorIndex, s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	modeParams.Dump();
#endif

	MotorParams motorParams(StoreMotors::Get());
	motorParams.Load(nMotorIndex, s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	motorParams.Dump();
#endif

	L6470Params l6470Params(StoreMotors::Get());
	l6470Params.Load(nMotorIndex, s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	l6470Params.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (NODE_SHOWFILE)
void RemoteConfig::HandleSetShowTxt() {
	DEBUG_ENTRY

	ShowFileParams showFileParams(StoreShowFile::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TShowFileParams)) {
			uint32_t nSize;
			showFileParams.Builder(reinterpret_cast<const struct TShowFileParams*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	showFileParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	showFileParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_SERIAL)
void RemoteConfig::HandleSetSerialTxt() {
	DEBUG_ENTRY

	DmxSerialParams dmxSerialParams(StoreDmxSerial::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TDmxSerialParams)) {
			uint32_t nSize;
			dmxSerialParams.Builder(reinterpret_cast<const struct TDmxSerialParams*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	dmxSerialParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	dmxSerialParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_RGB_PANEL)
void RemoteConfig::HandleSetRgbPanelTxt() {
	DEBUG_ENTRY

	RgbPanelParams rgbPanelParams(StoreRgbPanel::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TRgbPanelParams)) {
			uint32_t nSize;
			rgbPanelParams.Builder(reinterpret_cast<const struct TRgbPanelParams*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	rgbPanelParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	rgbPanelParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (NODE_DDP_DISPLAY)
void RemoteConfig::HandleSetDdpDisplayTxt() {
	DEBUG_ENTRY

	DdpDisplayParams ddpDisplayParams(StoreDdpDisplay::Get());

#if !defined(DISABLE_BIN)
	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TDdpDisplayParams)) {
			uint32_t nSize;
			ddpDisplayParams.Builder(reinterpret_cast<const struct TDdpDisplayParams*>(s_StoreBuffer), s_pUdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else {
			DEBUG_EXIT
			return;
		}
	}
#endif

	ddpDisplayParams.Load(s_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	ddpDisplayParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

void RemoteConfig::TftpExit() {
	DEBUG_ENTRY

	const auto nCmdLength = s_SET[static_cast<uint32_t>(udp::set::Command::TFTP)].nLength;

	m_nBytesReceived = (nCmdLength + 1U);
	s_pUdpBuffer[nCmdLength + 1] = '0';

	HandleTftpSet();

	DEBUG_EXIT
}

void RemoteConfig::HandleTftpSet() {
	DEBUG_ENTRY

	const auto nCmdLength = s_SET[static_cast<uint32_t>(udp::set::Command::TFTP)].nLength;

	if (m_nBytesReceived != (nCmdLength + 1)) {
		DEBUG_EXIT
		return;
	}

	m_bEnableTFTP = (s_pUdpBuffer[nCmdLength + 1] != '0');

	PlatformHandleTftpSet();

	DEBUG_EXIT
}

void RemoteConfig::HandleTftpGet() {
	DEBUG_ENTRY

	PlatformHandleTftpGet();

	const auto nCmdLength = s_GET[static_cast<uint32_t>(udp::get::Command::TFTP)].nLength;

	if (m_nBytesReceived == nCmdLength) {
		const auto nLength = snprintf(s_pUdpBuffer, udp::BUFFER_SIZE - 1, "tftp:%s\n", m_bEnableTFTP ? "On" : "Off");
		Network::Get()->SendTo(m_nHandle, s_pUdpBuffer, static_cast<uint16_t>(nLength), m_nIPAddressFrom, udp::PORT);
		DEBUG_EXIT
		return;
	}
#if !defined(DISABLE_BIN)
	else if (m_nBytesReceived == nCmdLength + 3) {
		if (memcmp(&s_pUdpBuffer[nCmdLength], "bin", 3) == 0) {
			Network::Get()->SendTo(m_nHandle, &m_bEnableTFTP, sizeof(bool) , m_nIPAddressFrom, udp::PORT);
			DEBUG_EXIT
			return;
		}
	}
#endif

	DEBUG_EXIT
}
