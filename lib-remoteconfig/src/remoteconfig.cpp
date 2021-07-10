/**
 * @file remoteconfig.cpp
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "display.h"

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
# include "artnet4params.h"
# include "storeartnet4.h"
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
# include "oscserverparms.h"
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
# include "tlc59711dmxparams.h"
# include "storetlc59711.h"
#endif

#if defined (OUTPUT_DMX_MONITOR)
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

#if defined (RDM_RESPONDER)
/* sensors.txt */
# include "rdmsensorsparams.h"
# include "storerdmsensors.h"
/* "subdev.txt" */
# include "rdmsubdevicesparams.h"
# include "storerdmdevice.h"
#endif

#if !defined(DISABLE_TFTP)
// nuc-i5:~/uboot-spi/u-boot$ grep CONFIG_BOOTCOMMAND include/configs/sunxi-common.h
// #define CONFIG_BOOTCOMMAND "sf probe; sf read 48000000 180000 22000; bootm 48000000"
# define FIRMWARE_MAX_SIZE	0x22000
# include "tftp/tftpfileserver.h"
# include "spiflashinstall.h"
#endif

#include "debug.h"

namespace remoteconfig {
namespace udp {
static constexpr auto PORT = 0x2905;

namespace cmd {

namespace get {
static constexpr char REBOOT[] = "?reboot##";
static constexpr char LIST[] = "?list#";
static constexpr char GET[] = "?get#";
static constexpr char UPTIME[] = "?uptime#";
static constexpr char VERSION[] = "?version#";
static constexpr char STORE[] = "?store#";
static constexpr char DISPLAY[] = "?display#";
#if !defined(DISABLE_TFTP)
static constexpr char TFTP[] = "?tftp#";
#endif
static constexpr char FACTORY[] = "?factory##";
namespace length {
static constexpr auto REBOOT = sizeof(cmd::get::REBOOT) - 1;
static constexpr auto LIST = sizeof(cmd::get::LIST) - 1;
static constexpr auto GET = sizeof(cmd::get::GET) - 1;
static constexpr auto UPTIME = sizeof(cmd::get::UPTIME) - 1;
static constexpr auto VERSION = sizeof(cmd::get::VERSION) - 1;
static constexpr auto STORE = sizeof(cmd::get::STORE) - 1;
static constexpr auto DISPLAY = sizeof(cmd::get::DISPLAY) - 1;
#if !defined(DISABLE_TFTP)
static constexpr auto TFTP = sizeof(cmd::get::TFTP) - 1;
#endif
static constexpr auto FACTORY = sizeof(cmd::get::FACTORY) - 1;
}  // namespace length
}  // namespace get

namespace set {
static constexpr char STORE[] = "!store#";
static constexpr char DISPLAY[] = "!display#";
#if !defined(DISABLE_TFTP)
static constexpr char TFTP[] = "!tftp#";
#endif
namespace length {
static constexpr auto STORE = sizeof(cmd::set::STORE) - 1;
static constexpr auto DISPLAY = sizeof(cmd::set::DISPLAY) - 1;
#if !defined(DISABLE_TFTP)
static constexpr auto TFTP = sizeof(cmd::set::TFTP) - 1;
#endif
}  // namespace length
}  // namespace set

}  // namespace cmd
}  // namespace udp
}  // namespace remoteconfig

using namespace remoteconfig;

static constexpr char s_Node[static_cast<uint32_t>(Node::LAST)][18] = { "Art-Net", "sACN E1.31", "OSC Server", "LTC", "OSC Client", "RDMNet LLRP Only", "Showfile", "DDP" };
static constexpr char s_Output[static_cast<uint32_t>(Output::LAST)][12] = { "DMX", "RDM", "Monitor", "Pixel", "TimeCode", "OSC", "Config", "Stepper", "Player", "Art-Net", "Serial", "RGB Panel" };

RemoteConfig *RemoteConfig::s_pThis = nullptr;
ListBin RemoteConfig::s_RemoteConfigListBin;
char RemoteConfig::s_aId[remoteconfig::ID_LENGTH];
char RemoteConfig::s_UdpBuffer[udp::BUFFER_SIZE];
uint8_t RemoteConfig::s_StoreBuffer[udp::BUFFER_SIZE];

RemoteConfig::RemoteConfig(Node tNode, Output tMode, uint32_t nOutputs): m_tNode(tNode), m_tOutput(tMode), m_nOutputs(nOutputs) {
	DEBUG_ENTRY

	assert(tNode < Node::LAST);
	assert(tMode < Output::LAST);

	assert(s_pThis == nullptr);
	s_pThis = this;

	Network::Get()->MacAddressCopyTo(s_RemoteConfigListBin.aMacAddress);
	s_RemoteConfigListBin.nNode = static_cast<uint8_t>(tNode);
	s_RemoteConfigListBin.nOutput = static_cast<uint8_t>(tMode);
	s_RemoteConfigListBin.nActiveUniverses = static_cast<uint8_t>(nOutputs);
	s_RemoteConfigListBin.aDisplayName[0] = '\0';

#ifndef NDEBUG
	DEBUG_PUTS("s_RemoteConfigListBin");
	debug_dump(&s_RemoteConfigListBin, sizeof s_RemoteConfigListBin);
#endif

	m_nHandle = Network::Get()->Begin(udp::PORT);
	assert(m_nHandle != -1);

	DEBUG_EXIT
}

RemoteConfig::~RemoteConfig() {
	DEBUG_ENTRY

	Network::Get()->End(udp::PORT);
	m_nHandle = -1;

	DEBUG_EXIT
}

void RemoteConfig::SetDisable(bool bDisable) {
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

#if !defined(DISABLE_TFTP)
	if (__builtin_expect((m_pTFTPFileServer != nullptr), 0)) {
		m_pTFTPFileServer->Run();
	}
#endif

	uint16_t nForeignPort;
	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, s_UdpBuffer, udp::BUFFER_SIZE, &m_nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived < 4), 1)) {
		return;
	}

#ifndef NDEBUG
	debug_dump(s_UdpBuffer, m_nBytesReceived);
#endif

	if (s_UdpBuffer[m_nBytesReceived - 1] == '\n') {
		m_nBytesReceived--;
	}

	if (s_UdpBuffer[0] == '?') {
		DEBUG_PUTS("?");

		if (m_bEnableReboot && (memcmp(s_UdpBuffer, udp::cmd::get::REBOOT, udp::cmd::get::length::REBOOT) == 0)) {
			HandleReboot();
			__builtin_unreachable();
			return;
		}

		if (m_bEnableUptime && (memcmp(s_UdpBuffer, udp::cmd::get::UPTIME, udp::cmd::get::length::UPTIME) == 0)) {
			HandleUptime();
			return;
		}

		if (memcmp(s_UdpBuffer, udp::cmd::get::VERSION, udp::cmd::get::length::VERSION) == 0) {
			HandleVersion();
			return;
		}

		if (memcmp(s_UdpBuffer, udp::cmd::get::LIST, udp::cmd::get::length::LIST) == 0) {
			HandleList();
			return;
		}

		if ((m_nBytesReceived > udp::cmd::get::length::GET) && (memcmp(s_UdpBuffer, udp::cmd::get::GET, udp::cmd::get::length::GET) == 0)) {
			HandleGet();
			return;
		}

		if ((m_nBytesReceived > udp::cmd::get::length::STORE) && (memcmp(s_UdpBuffer, udp::cmd::get::STORE, udp::cmd::get::length::STORE) == 0)) {
			HandleStoreGet();
			return;
		}

		if ((m_nBytesReceived >= udp::cmd::get::length::DISPLAY) && (memcmp(s_UdpBuffer, udp::cmd::get::DISPLAY, udp::cmd::get::length::DISPLAY) == 0)) {
			HandleDisplayGet();
			return;
		}
#if !defined(DISABLE_TFTP)
		if ((m_nBytesReceived >= udp::cmd::get::length::TFTP) && (memcmp(s_UdpBuffer, udp::cmd::get::TFTP, udp::cmd::get::length::TFTP) == 0)) {
			HandleTftpGet();
			return;
		}
#endif
		if (m_bEnableFactory && (memcmp(s_UdpBuffer, udp::cmd::get::FACTORY, udp::cmd::get::length::FACTORY) == 0)) {
			HandleFactory();
			return;
		}

		Network::Get()->SendTo(m_nHandle, "?#ERROR#\n", 9, m_nIPAddressFrom, udp::PORT);

		return;
	}

	if (!m_bDisableWrite) {
		if (s_UdpBuffer[0] == '#') {
			DEBUG_PUTS("#");
			m_tHandleMode = HandleMode::TXT;
			HandleTxtFile();
		} else if (s_UdpBuffer[0] == '!') {
			DEBUG_PUTS("!");
			if ((m_nBytesReceived >= udp::cmd::set::length::DISPLAY) && (memcmp(s_UdpBuffer, udp::cmd::set::DISPLAY, udp::cmd::set::length::DISPLAY) == 0)) {
				DEBUG_PUTS(udp::cmd::set::DISPLAY);
				HandleDisplaySet();
			}
#if !defined(DISABLE_TFTP)
			else if ((m_nBytesReceived == udp::cmd::set::length::TFTP + 1) && (memcmp(s_UdpBuffer, udp::cmd::set::TFTP, udp::cmd::set::length::TFTP) == 0)) {
				DEBUG_PUTS(udp::cmd::set::TFTP);
				HandleTftpSet();
			}
#endif
			else if ((m_nBytesReceived > udp::cmd::set::length::STORE) && (memcmp(s_UdpBuffer, udp::cmd::set::STORE, udp::cmd::set::length::STORE) == 0)) {
				DEBUG_PUTS(udp::cmd::set::STORE);
				m_tHandleMode = HandleMode::BIN;
				HandleTxtFile();
			} else {
				Network::Get()->SendTo(m_nHandle, "!#ERROR#\n", 9, m_nIPAddressFrom, udp::PORT);
			}
		}

		return;
	}
}

void RemoteConfig::HandleUptime() {
	DEBUG_ENTRY

	const auto nUptime = Hardware::Get()->GetUpTime();

	if (m_nBytesReceived == udp::cmd::get::length::UPTIME) {
		const auto nLength = snprintf(s_UdpBuffer, udp::BUFFER_SIZE - 1, "uptime: %ds\n", static_cast<int>(nUptime));
		Network::Get()->SendTo(m_nHandle, s_UdpBuffer, static_cast<uint16_t>(nLength), m_nIPAddressFrom, udp::PORT);
	} else if (m_nBytesReceived == udp::cmd::get::length::UPTIME + 3) {
		if (memcmp(&s_UdpBuffer[udp::cmd::get::length::UPTIME], "bin", 3) == 0) {
			Network::Get()->SendTo(m_nHandle, &nUptime, sizeof(uint32_t) , m_nIPAddressFrom, udp::PORT);
		}
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleVersion() {
	DEBUG_ENTRY

	if (m_nBytesReceived == udp::cmd::get::length::VERSION) {
		const auto *p = FirmwareVersion::Get()->GetPrint();
		const auto nLength = snprintf(s_UdpBuffer, udp::BUFFER_SIZE, "version:%s", p);
		Network::Get()->SendTo(m_nHandle, s_UdpBuffer, static_cast<uint16_t>(nLength), m_nIPAddressFrom, udp::PORT);
	} else if (m_nBytesReceived == udp::cmd::get::length::VERSION + 3) {
		if (memcmp(&s_UdpBuffer[udp::cmd::get::length::VERSION], "bin", 3) == 0) {
			const auto *p = reinterpret_cast<const uint8_t*>(FirmwareVersion::Get()->GetVersion());
			Network::Get()->SendTo(m_nHandle, p, sizeof(struct TFirmwareVersion) , m_nIPAddressFrom, udp::PORT);
		}
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleList() {
	DEBUG_ENTRY

	if (s_RemoteConfigListBin.aDisplayName[0] != '\0') {
		m_nIdLength = snprintf(s_aId, sizeof(s_aId) - 1, "" IPSTR ",%s,%s,%d,%s\n", IP2STR(Network::Get()->GetIp()), s_Node[static_cast<uint32_t>(m_tNode)], s_Output[static_cast<uint32_t>(m_tOutput)], m_nOutputs, s_RemoteConfigListBin.aDisplayName);
	} else {
		m_nIdLength = snprintf(s_aId, sizeof(s_aId) - 1, "" IPSTR ",%s,%s,%d\n", IP2STR(Network::Get()->GetIp()), s_Node[static_cast<uint32_t>(m_tNode)], s_Output[static_cast<uint32_t>(m_tOutput)], m_nOutputs);
	}

	if (m_nBytesReceived == udp::cmd::get::length::LIST) {
		Network::Get()->SendTo(m_nHandle, s_aId, static_cast<uint16_t>(m_nIdLength), m_nIPAddressFrom, udp::PORT);
	} else if (m_nBytesReceived == udp::cmd::get::length::LIST + 3) {
		if (memcmp(&s_UdpBuffer[udp::cmd::get::length::LIST], "bin", 3) == 0) {
			Network::Get()->SendTo(m_nHandle, &s_RemoteConfigListBin, sizeof(struct ListBin) , m_nIPAddressFrom, udp::PORT);
		}
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleDisplaySet() {
	DEBUG_ENTRY
	DEBUG_PRINTF("%c", s_UdpBuffer[udp::cmd::set::length::DISPLAY]);

	Display::Get()->SetSleep(s_UdpBuffer[udp::cmd::set::length::DISPLAY] == '0');

	DEBUG_EXIT
}

void RemoteConfig::HandleDisplayGet() {
	DEBUG_ENTRY

	const bool isOn = !(Display::Get()->isSleep());

	if (m_nBytesReceived == udp::cmd::get::length::DISPLAY) {
		const auto nLength = snprintf(s_UdpBuffer, udp::BUFFER_SIZE - 1, "display:%s\n", isOn ? "On" : "Off");
		Network::Get()->SendTo(m_nHandle, s_UdpBuffer, static_cast<uint16_t>(nLength), m_nIPAddressFrom, udp::PORT);
	} else if (m_nBytesReceived == udp::cmd::get::length::DISPLAY + 3) {
		if (memcmp(&s_UdpBuffer[udp::cmd::get::length::DISPLAY], "bin", 3) == 0) {
			Network::Get()->SendTo(m_nHandle, &isOn, sizeof(bool) , m_nIPAddressFrom, udp::PORT);
		}
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleStoreGet() {
	DEBUG_ENTRY

	uint32_t nLength = udp::BUFFER_SIZE - udp::cmd::get::length::STORE;

	const auto nIndex = GetIndex(&s_UdpBuffer[udp::cmd::get::length::STORE], nLength);

	if (nIndex != TxtFile::LAST) {
		SpiFlashStore::Get()->CopyTo(GetStore(static_cast<TxtFile>(nIndex)), s_UdpBuffer, nLength);
	} else {
		Network::Get()->SendTo(m_nHandle, "?store#ERROR#\n", 12, m_nIPAddressFrom, udp::PORT);
		return;
	}

#ifndef NDEBUG
	debug_dump(s_UdpBuffer, static_cast<uint16_t>(nLength));
#endif
	Network::Get()->SendTo(m_nHandle, s_UdpBuffer, static_cast<uint16_t>(nLength), m_nIPAddressFrom, udp::PORT);

	DEBUG_EXIT
}

uint32_t RemoteConfig::HandleGet(void *pBuffer, uint32_t nBufferLength) {
	DEBUG_ENTRY

	uint32_t nSize;
	TxtFile nIndex;

	if (pBuffer == nullptr) {
		nSize = udp::BUFFER_SIZE - udp::cmd::get::length::GET;
		nIndex = GetIndex(&s_UdpBuffer[udp::cmd::get::length::GET], nSize);
	} else {
		nSize = nBufferLength;
		nIndex = GetIndex(pBuffer, nSize);
	}

	switch (nIndex) {
	case TxtFile::RCONFIG:
		HandleGetRconfigTxt(nSize);
		break;
	case TxtFile::NETWORK:
		HandleGetNetworkTxt(nSize);
		break;
#if defined (NODE_ARTNET)
	case TxtFile::ARTNET:
		HandleGetArtnetTxt(nSize);
		break;
#endif
#if defined (NODE_E131)
	case TxtFile::E131:
		HandleGetE131Txt(nSize);
		break;
#endif
#if defined (NODE_OSC_SERVER)
	case TxtFile::OSC_SERVER:
		HandleGetOscTxt(nSize);
		break;
#endif
#if defined (OUTPUT_DMX_SEND)
	case TxtFile::PARAMS:
		HandleGetParamsTxt(nSize);
		break;
#endif
#if defined (OUTPUT_DMX_PIXEL)
	case TxtFile::DEVICES:
		HandleGetDevicesTxt(nSize);
		break;
#endif
#if defined (NODE_LTC_SMPTE)
	case TxtFile::LTC:
		HandleGetLtcTxt(nSize);
		break;
	case TxtFile::LTCDISPLAY:
		HandleGetLtcDisplayTxt(nSize);
		break;
	case TxtFile::TCNET:
		HandleGetTCNetTxt(nSize);
		break;
	case TxtFile::GPS:
		HandleGetGpsTxt(nSize);
		break;
#endif
#if defined (OUTPUT_DMX_MONITOR)
	case TxtFile::MONITOR:
		HandleGetMonTxt(nSize);
		break;
#endif
#if defined (NODE_OSC_CLIENT)
	case TxtFile::OSC_CLIENT:
		HandleGetOscClntTxt(nSize);
		break;
#endif
#if defined(DISPLAY_UDF)
	case TxtFile::DISPLAY:
		HandleGetDisplayTxt(nSize);
		break;
#endif
#if defined(OUTPUT_DMX_STEPPER)
	case TxtFile::SPARKFUN:
		HandleGetSparkFunTxt(nSize);
		break;
	case TxtFile::MOTOR0:
	case TxtFile::MOTOR1:
	case TxtFile::MOTOR2:
	case TxtFile::MOTOR3:
		HandleGetMotorTxt(static_cast<uint32_t>(nIndex) - static_cast<uint32_t>(TxtFile::MOTOR0), nSize);
		break;
#endif
#if defined(NODE_SHOWFILE)
	case TxtFile::SHOW:
		HandleGetShowTxt(nSize);
		break;
#endif
#if defined (OUTPUT_DMX_SERIAL)
	case TxtFile::SERIAL:
		HandleGetSerialTxt(nSize);
		break;
#endif
#if defined (OUTPUT_RGB_PANEL)
	case TxtFile::RGBPANEL:
		HandleGetRgbPanelTxt(nSize);
		break;
#endif
#if defined (NODE_DDP_DISPLAY)
	case TxtFile::DDPDISP:
		HandleGetDdpDisplayTxt(nSize);
		break;
#endif
	default:
		if (pBuffer == nullptr) {
			Network::Get()->SendTo(m_nHandle, "?get#ERROR#\n", 12, m_nIPAddressFrom, udp::PORT);
		} else {
			DEBUG_PUTS("");
			memcpy(pBuffer, "?get#ERROR#\n", std::min((12U), nBufferLength));
		}
		DEBUG_EXIT
		return 12;
		__builtin_unreachable ();
		break;
	}

#ifndef NDEBUG
	debug_dump(s_UdpBuffer, static_cast<uint16_t>(nSize));
#endif

	if (pBuffer == nullptr) {
		Network::Get()->SendTo(m_nHandle, s_UdpBuffer, static_cast<uint16_t>(nSize), m_nIPAddressFrom, udp::PORT);
	} else {
		memcpy(pBuffer, s_UdpBuffer, std::min(nSize, nBufferLength));
	}

	DEBUG_EXIT
	return nSize;
}

void RemoteConfig::HandleGetRconfigTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	RemoteConfigParams remoteConfigParams(StoreRemoteConfig::Get());
	remoteConfigParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetNetworkTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	NetworkParams networkParams(StoreNetwork::Get());
	networkParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

#if defined (NODE_ARTNET)
void RemoteConfig::HandleGetArtnetTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	uint32_t nSizeArtNet3 = 0;

	assert(StoreArtNet::Get() != nullptr);
	ArtNetParams artnetparams(StoreArtNet::Get());
	artnetparams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSizeArtNet3);

	uint32_t nSizeArtNet4 = 0;

	assert(StoreArtNet4::Get() != nullptr);
	ArtNet4Params artnet4params(StoreArtNet4::Get());
	artnet4params.Save(s_UdpBuffer + nSizeArtNet3, udp::BUFFER_SIZE - nSizeArtNet3, nSizeArtNet4);

	nSize = nSizeArtNet3 + nSizeArtNet4;

	DEBUG_EXIT
}
#endif

#if defined (NODE_E131)
void RemoteConfig::HandleGetE131Txt(uint32_t& nSize) {
	DEBUG_ENTRY

	E131Params e131params(StoreE131::Get());
	e131params.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (NODE_OSC_SERVER)
void RemoteConfig::HandleGetOscTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	OSCServerParams oscServerParams(StoreOscServer::Get());
	oscServerParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (NODE_OSC_CLIENT)
void RemoteConfig::HandleGetOscClntTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	OscClientParams oscClientParams(StoreOscClient::Get());
	oscClientParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_SEND)
void RemoteConfig::HandleGetParamsTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DMXParams dmxparams(StoreDmxSend::Get());
	dmxparams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_PIXEL)
void RemoteConfig::HandleGetDevicesTxt(uint32_t& nSize) {
	DEBUG_ENTRY

# if defined (RDM_RESPONDER)
	bool bIsSetLedType = false;

	TLC59711DmxParams tlc5911params(StoreTLC59711::Get());

	if (tlc5911params.Load()) {
		if ((bIsSetLedType = tlc5911params.IsSetLedType()) == true) {
			tlc5911params.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);
		}
	}

	if (!bIsSetLedType) {
# endif
		WS28xxDmxParams ws28xxparms(StoreWS28xxDmx::Get());
		ws28xxparms.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);
# if defined (RDM_RESPONDER)
	}
#endif

	DEBUG_EXIT
}
#endif

#if defined (NODE_LTC_SMPTE)
void RemoteConfig::HandleGetLtcTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	LtcParams ltcParams(StoreLtc::Get());
	ltcParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetLtcDisplayTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	LtcDisplayParams ltcDisplayParams(StoreLtcDisplay::Get());
	ltcDisplayParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetTCNetTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	TCNetParams tcnetParams(StoreTCNet::Get());
	tcnetParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetGpsTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	GPSParams gpsParams(StoreGPS::Get());
	gpsParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(OUTPUT_DMX_MONITOR)
void RemoteConfig::HandleGetMonTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DMXMonitorParams monitorParams(StoreMonitor::Get());
	monitorParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(DISPLAY_UDF)
void RemoteConfig::HandleGetDisplayTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DisplayUdfParams displayParams(StoreDisplayUdf::Get());
	displayParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(OUTPUT_DMX_STEPPER)
void RemoteConfig::HandleGetSparkFunTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	SparkFunDmxParams sparkFunParams(StoreSparkFunDmx::Get());
	sparkFunParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetMotorTxt(uint32_t nMotorIndex, uint32_t& nSize) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nMotorIndex=%d", nMotorIndex);

	uint32_t nSizeSparkFun = 0;

	SparkFunDmxParams sparkFunParams(StoreSparkFunDmx::Get());
	sparkFunParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSizeSparkFun, nMotorIndex);

	DEBUG_PRINTF("nSizeSparkFun=%d", nSizeSparkFun);

	uint32_t nSizeMode = 0;

	ModeParams modeParams(StoreMotors::Get());
	modeParams.Save(nMotorIndex, s_UdpBuffer + nSizeSparkFun, udp::BUFFER_SIZE - nSizeSparkFun, nSizeMode);

	DEBUG_PRINTF("nSizeMode=%d", nSizeMode);

	uint32_t nSizeMotor = 0;

	MotorParams motorParams(StoreMotors::Get());
	motorParams.Save(nMotorIndex, s_UdpBuffer + nSizeSparkFun + nSizeMode, udp::BUFFER_SIZE - nSizeSparkFun - nSizeMode, nSizeMotor);

	DEBUG_PRINTF("nSizeMotor=%d", nSizeMotor);

	uint32_t nSizeL6470 = 0;

	L6470Params l6470Params(StoreMotors::Get());
	l6470Params.Save(nMotorIndex, s_UdpBuffer + nSizeSparkFun + nSizeMode + nSizeMotor, udp::BUFFER_SIZE - nSizeSparkFun - nSizeMode - nSizeMotor, nSizeL6470);

	DEBUG_PRINTF("nSizeL6470=%d", nSizeL6470);

	nSize = nSizeSparkFun + nSizeMode + nSizeMotor + nSizeL6470;

	DEBUG_EXIT
}
#endif

#if defined(NODE_SHOWFILE)
void RemoteConfig::HandleGetShowTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	ShowFileParams showFileParams(StoreShowFile::Get());
	showFileParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_SERIAL)
void RemoteConfig::HandleGetSerialTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DmxSerialParams dmxSerialParams(StoreDmxSerial::Get());
	dmxSerialParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_RGB_PANEL)
void RemoteConfig::HandleGetRgbPanelTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	RgbPanelParams rgbPanelParams(StoreRgbPanel::Get());
	rgbPanelParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (NODE_DDP_DISPLAY)
void RemoteConfig::HandleGetDdpDisplayTxt(uint32_t &nSize) {
	DEBUG_ENTRY

	DdpDisplayParams ddpDisplayParams(StoreDdpDisplay::Get());
	ddpDisplayParams.Save(s_UdpBuffer, udp::BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

/*
 *
 */

void RemoteConfig::HandleTxtFile(void *pBuffer, uint32_t nBufferLength) {
	DEBUG_ENTRY

	TxtFile i;
	uint32_t nLength;

	if (pBuffer == nullptr) {
		if (m_tHandleMode == HandleMode::TXT) {
			nLength = udp::BUFFER_SIZE - 1;
			i = GetIndex(&s_UdpBuffer[1], nLength);
		} else {
			nLength = udp::BUFFER_SIZE - udp::cmd::set::length::STORE;
			i = GetIndex(&s_UdpBuffer[udp::cmd::set::length::STORE], nLength);
			if (i < TxtFile::LAST) {
				m_nBytesReceived = static_cast<uint16_t>(m_nBytesReceived - nLength - udp::cmd::set::length::STORE);
				memcpy(s_StoreBuffer, &s_UdpBuffer[nLength + udp::cmd::set::length::STORE], udp::BUFFER_SIZE);
				debug_dump(s_StoreBuffer, m_nBytesReceived);
			} else {
				return;
			}
		}
	} else if (nBufferLength <= udp::BUFFER_SIZE){
		m_tHandleMode = HandleMode::TXT;
		memcpy(s_UdpBuffer, pBuffer, nBufferLength);
		m_nBytesReceived = static_cast<uint16_t>(nBufferLength);
		i = GetIndex(&s_UdpBuffer[1], nBufferLength);
	} else {
		return;
	}

	switch (i) {
	case TxtFile::RCONFIG:
		HandleTxtFileRconfig();
		break;
	case TxtFile::NETWORK:
		HandleTxtFileNetwork();
		break;
#if defined (NODE_ARTNET)
	case TxtFile::ARTNET:
		HandleTxtFileArtnet();
		break;
#endif
#if defined (NODE_E131)
	case TxtFile::E131:
		HandleTxtFileE131();
		break;
#endif
#if defined (NODE_OSC_SERVER)
	case TxtFile::OSC_SERVER:
		HandleTxtFileOsc();
		break;
#endif
#if defined (OUTPUT_DMX_SEND)
	case TxtFile::PARAMS:
		HandleTxtFileParams();
		break;
#endif
#if defined (OUTPUT_DMX_PIXEL)
	case TxtFile::DEVICES:
		HandleTxtFileDevices();
		break;
#endif
#if defined (NODE_LTC_SMPTE)
	case TxtFile::LTC:
		HandleTxtFileLtc();
		break;
	case TxtFile::LTCDISPLAY:
		HandleTxtFileLtcDisplay();
		break;
	case TxtFile::TCNET:
		HandleTxtFileTCNet();
		break;
	case TxtFile::GPS:
		HandleTxtFileGps();
		break;
#endif
#if defined (NODE_OSC_CLIENT)
	case TxtFile::OSC_CLIENT:
		HandleTxtFileOscClient();
		break;
#endif
#if defined(OUTPUT_DMX_MONITOR)
	case TxtFile::MONITOR:
		HandleTxtFileMon();
		break;
#endif
#if defined(DISPLAY_UDF)
	case TxtFile::DISPLAY:
		HandleTxtFileDisplay();
		break;
#endif
#if defined(OUTPUT_DMX_STEPPER)
	case TxtFile::SPARKFUN:
		HandleTxtFileSparkFun();
		break;
	case TxtFile::MOTOR0:
	case TxtFile::MOTOR1:
	case TxtFile::MOTOR2:
	case TxtFile::MOTOR3:
		HandleTxtFileMotor(static_cast<uint32_t>(i) - static_cast<uint32_t>(TxtFile::MOTOR0));
		break;
#endif
#if defined (NODE_SHOWFILE)
	case TxtFile::SHOW:
		HandleTxtFileShow();
		break;
#endif
#if defined (OUTPUT_DMX_SERIAL)
	case TxtFile::SERIAL:
		HandleTxtFileSerial();
		break;
#endif
#if defined (OUTPUT_RGB_PANEL)
	case TxtFile::RGBPANEL:
		HandleTxtFileRgbPanel();
		break;
#endif
#if defined (NODE_DDP_DISPLAY)
	case TxtFile::DDPDISP:
		HandleTxtFileDdpDisplay();
		break;
#endif
	default:
		break;
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileRconfig() {
	DEBUG_ENTRY

	RemoteConfigParams remoteConfigParams(StoreRemoteConfig::Get());

	if (m_tHandleMode == HandleMode::BIN) {
	        if (m_nBytesReceived == sizeof(struct TRemoteConfigParams)) {
		        uint32_t nSize;
		        remoteConfigParams.Builder(reinterpret_cast<const struct TRemoteConfigParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
		        m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
		        DEBUG_EXIT
			return;
		}
	}

	remoteConfigParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	remoteConfigParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileNetwork() {
	DEBUG_ENTRY

	NetworkParams params(StoreNetwork::Get());

	if (m_tHandleMode == HandleMode::BIN)
	{
		if (m_nBytesReceived == sizeof(struct TNetworkParams)) {
			uint32_t nSize;
			params.Builder(reinterpret_cast<const struct TNetworkParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	params.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	params.Dump();
#endif

	DEBUG_EXIT
}

#if defined (NODE_ARTNET)
void RemoteConfig::HandleTxtFileArtnet() {
	DEBUG_ENTRY
	static_assert(sizeof(struct TArtNet4Params) != sizeof(struct TArtNetParams), "");

	assert(StoreArtNet4::Get() != nullptr);
	ArtNet4Params artnet4params(StoreArtNet4::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TArtNetParams)) {
			ArtNetParams artnetparams(StoreArtNet::Get());
			uint32_t nSize;
			artnetparams.Builder(reinterpret_cast<const struct TArtNetParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else if (m_nBytesReceived == sizeof(struct TArtNet4Params)) {
			uint32_t nSize;
			artnet4params.Builder(reinterpret_cast<const struct TArtNet4Params*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	artnet4params.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	artnet4params.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (NODE_E131)
void RemoteConfig::HandleTxtFileE131() {
	DEBUG_ENTRY

	E131Params e131params(StoreE131::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TE131Params)) {
			uint32_t nSize;
			e131params.Builder(reinterpret_cast<const struct TE131Params*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
		
	}

	e131params.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	e131params.Dump();
#endif
	DEBUG_EXIT
}
#endif

#if defined (NODE_OSC_SERVER)
void RemoteConfig::HandleTxtFileOsc() {
	DEBUG_ENTRY

	OSCServerParams oscServerParams(StoreOscServer::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TOSCServerParams)) {
			uint32_t nSize;
			oscServerParams.Builder(reinterpret_cast<const struct TOSCServerParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
		
	}

	oscServerParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	oscServerParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (NODE_OSC_CLIENT)
void RemoteConfig::HandleTxtFileOscClient() {
	DEBUG_ENTRY

	OscClientParams oscClientParams(StoreOscClient::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TOscClientParams)) {
			uint32_t nSize;
			oscClientParams.Builder(reinterpret_cast<const struct TOscClientParams *>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	oscClientParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	oscClientParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_SEND)
void RemoteConfig::HandleTxtFileParams() {
	DEBUG_ENTRY

	DMXParams dmxparams(StoreDmxSend::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TDMXParams)) {
			uint32_t nSize;
			dmxparams.Builder(reinterpret_cast<const struct TDMXParams *>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	dmxparams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	dmxparams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_PIXEL)
void RemoteConfig::HandleTxtFileDevices() {
	DEBUG_ENTRY

# if defined (RDM_RESPONDER)
	static_assert(sizeof(struct TTLC59711DmxParams) != sizeof(struct TWS28xxDmxParams), "");

	TLC59711DmxParams tlc59711params(StoreTLC59711::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TTLC59711DmxParams)) {
			uint32_t nSize;
			tlc59711params.Builder(reinterpret_cast<const struct TTLC59711DmxParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	tlc59711params.Load(s_UdpBuffer, m_nBytesReceived);
#  ifndef NDEBUG
	tlc59711params.Dump();
#  endif
	DEBUG_PRINTF("tlc5911params.IsSetLedType()=%d", tlc59711params.IsSetLedType());

	if (!tlc59711params.IsSetLedType()) {
# endif
		WS28xxDmxParams ws28xxparms(StoreWS28xxDmx::Get());

		if (m_tHandleMode == HandleMode::BIN) {
			if (m_nBytesReceived == sizeof(struct TWS28xxDmxParams)) {
				uint32_t nSize;
				ws28xxparms.Builder(reinterpret_cast<const struct TWS28xxDmxParams *>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
				m_nBytesReceived = static_cast<uint16_t>(nSize);
			} else {
				DEBUG_EXIT
				return;
			}
		}

		ws28xxparms.Load(s_UdpBuffer, m_nBytesReceived);
# ifndef NDEBUG
		ws28xxparms.Dump();
# endif
# if defined (RDM_RESPONDER)
	}
# endif

	DEBUG_EXIT
}
#endif

#if defined (NODE_LTC_SMPTE)
void RemoteConfig::HandleTxtFileLtc() {
	DEBUG_ENTRY

	LtcParams ltcParams(StoreLtc::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TLtcParams)) {
			uint32_t nSize;
			ltcParams.Builder(reinterpret_cast<const struct TLtcParams *>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	ltcParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	ltcParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileLtcDisplay() {
	DEBUG_ENTRY

	LtcDisplayParams ltcDisplayParams(StoreLtcDisplay::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TLtcDisplayParams)) {
			uint32_t nSize;
			ltcDisplayParams.Builder(reinterpret_cast<const struct TLtcDisplayParams *>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	ltcDisplayParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	ltcDisplayParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileTCNet() {
	DEBUG_ENTRY

	TCNetParams tcnetParams(StoreTCNet::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TTCNetParams)) {
			uint32_t nSize;
			tcnetParams.Builder(reinterpret_cast<const struct TTCNetParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	tcnetParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	tcnetParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileGps() {
	DEBUG_ENTRY

	GPSParams gpsParams(StoreGPS::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TGPSParams)) {
			uint32_t nSize;
			gpsParams.Builder(reinterpret_cast<const struct TGPSParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	gpsParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	gpsParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined(OUTPUT_DMX_MONITOR)
void RemoteConfig::HandleTxtFileMon() {
	DEBUG_ENTRY

	DMXMonitorParams monitorParams(StoreMonitor::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TDMXMonitorParams)) {
			uint32_t nSize;
			monitorParams.Builder(reinterpret_cast<const struct TDMXMonitorParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	monitorParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	monitorParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined(DISPLAY_UDF)
void RemoteConfig::HandleTxtFileDisplay() {
	DEBUG_ENTRY

	DisplayUdfParams displayParams(StoreDisplayUdf::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TDisplayUdfParams)) {
			uint32_t nSize;
			displayParams.Builder(reinterpret_cast<const struct TDisplayUdfParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	displayParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	displayParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined(OUTPUT_DMX_STEPPER)
void RemoteConfig::HandleTxtFileSparkFun() {
	DEBUG_ENTRY

	SparkFunDmxParams sparkFunDmxParams(StoreSparkFunDmx::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TSparkFunDmxParams)) {
			uint32_t nSize;
			sparkFunDmxParams.Builder(reinterpret_cast<const struct TSparkFunDmxParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	sparkFunDmxParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	sparkFunDmxParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileMotor(uint32_t nMotorIndex) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nMotorIndex=%d", nMotorIndex);

	if (m_tHandleMode == HandleMode::BIN) {
		// TODO HandleTxtFileMotor HandleMode::BIN
		return;
	}

	SparkFunDmxParams sparkFunDmxParams(StoreSparkFunDmx::Get());
	sparkFunDmxParams.Load(nMotorIndex, s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	sparkFunDmxParams.Dump();
#endif

	ModeParams modeParams(StoreMotors::Get());
	modeParams.Load(nMotorIndex, s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	modeParams.Dump();
#endif

	MotorParams motorParams(StoreMotors::Get());
	motorParams.Load(nMotorIndex, s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	motorParams.Dump();
#endif

	L6470Params l6470Params(StoreMotors::Get());
	l6470Params.Load(nMotorIndex, s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	l6470Params.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (NODE_SHOWFILE)
void RemoteConfig::HandleTxtFileShow() {
	DEBUG_ENTRY

	ShowFileParams showFileParams(StoreShowFile::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TShowFileParams)) {
			uint32_t nSize;
			showFileParams.Builder(reinterpret_cast<const struct TShowFileParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	showFileParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	showFileParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_DMX_SERIAL)
void RemoteConfig::HandleTxtFileSerial() {
	DEBUG_ENTRY

	DmxSerialParams dmxSerialParams(StoreDmxSerial::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TDmxSerialParams)) {
			uint32_t nSize;
			dmxSerialParams.Builder(reinterpret_cast<const struct TDmxSerialParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	dmxSerialParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	dmxSerialParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (OUTPUT_RGB_PANEL)
void RemoteConfig::HandleTxtFileRgbPanel() {
	DEBUG_ENTRY

	RgbPanelParams rgbPanelParams(StoreRgbPanel::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TRgbPanelParams)) {
			uint32_t nSize;
			rgbPanelParams.Builder(reinterpret_cast<const struct TRgbPanelParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	rgbPanelParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	rgbPanelParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (NODE_DDP_DISPLAY)
void RemoteConfig::HandleTxtFileDdpDisplay() {
	DEBUG_ENTRY

	DdpDisplayParams ddpDisplayParams(StoreDdpDisplay::Get());

	if (m_tHandleMode == HandleMode::BIN) {
		if (m_nBytesReceived == sizeof(struct TDdpDisplayParams)) {
			uint32_t nSize;
			ddpDisplayParams.Builder(reinterpret_cast<const struct TDdpDisplayParams*>(s_StoreBuffer), s_UdpBuffer, udp::BUFFER_SIZE, nSize);
			m_nBytesReceived = static_cast<uint16_t>(nSize);
		} else {
			DEBUG_EXIT
			return;
		}
	}

	ddpDisplayParams.Load(s_UdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	ddpDisplayParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if !defined(DISABLE_TFTP)
void RemoteConfig::TftpExit() {
	DEBUG_ENTRY

	s_UdpBuffer[udp::cmd::set::length::TFTP] = '0';

	HandleTftpSet();

	DEBUG_EXIT
}

void RemoteConfig::HandleTftpSet() {
	DEBUG_ENTRY
	DEBUG_PRINTF("%c", s_UdpBuffer[udp::cmd::set::length::TFTP]);

	m_bEnableTFTP = (s_UdpBuffer[udp::cmd::set::length::TFTP] != '0');

	if (m_bEnableTFTP) {
		Display::Get()->SetSleep(false);
	}

	if (m_bEnableTFTP && (m_pTFTPFileServer == nullptr)) {
		puts("Create TFTP Server");

		m_pTFTPBuffer = new uint8_t[FIRMWARE_MAX_SIZE];
		assert(m_pTFTPBuffer != nullptr);

		m_pTFTPFileServer = new TFTPFileServer(m_pTFTPBuffer, FIRMWARE_MAX_SIZE);
		assert(m_pTFTPFileServer != nullptr);
		Display::Get()->TextStatus("TFTP On", Display7SegmentMessage::INFO_TFTP_ON);
	} else if (!m_bEnableTFTP && (m_pTFTPFileServer != nullptr)) {
		const uint32_t nFileSize = m_pTFTPFileServer->GetFileSize();
		DEBUG_PRINTF("nFileSize=%d, %d", nFileSize, m_pTFTPFileServer->isDone());

		bool bSucces = true;

		if (m_pTFTPFileServer->isDone()) {
			bSucces = SpiFlashInstall::Get()->WriteFirmware(m_pTFTPBuffer, nFileSize);

			if (!bSucces) {
				Display::Get()->TextStatus("Error: TFTP", Display7SegmentMessage::ERROR_TFTP);
			}
		}

		puts("Delete TFTP Server");

		delete m_pTFTPFileServer;
		m_pTFTPFileServer = nullptr;

		delete[] m_pTFTPBuffer;
		m_pTFTPBuffer = nullptr;

		if (bSucces) { // Keep error message
			Display::Get()->TextStatus("TFTP Off", Display7SegmentMessage::INFO_TFTP_OFF);
		}
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleTftpGet() {
	DEBUG_ENTRY

	if (m_nBytesReceived == udp::cmd::get::length::TFTP) {
		const int nLength = snprintf(s_UdpBuffer, udp::BUFFER_SIZE - 1, "tftp:%s\n", m_bEnableTFTP ? "On" : "Off");
		Network::Get()->SendTo(m_nHandle, s_UdpBuffer, static_cast<uint16_t>(nLength), m_nIPAddressFrom, udp::PORT);
	} else if (m_nBytesReceived == udp::cmd::get::length::TFTP + 3) {
		if (memcmp(&s_UdpBuffer[udp::cmd::get::length::TFTP], "bin", 3) == 0) {
			Network::Get()->SendTo(m_nHandle, &m_bEnableTFTP, sizeof(bool) , m_nIPAddressFrom, udp::PORT);
		}
	}

	DEBUG_EXIT
}
#endif
