/**
 * @file remoteconfig.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

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

#if defined (ARTNET_NODE)
 /* artnet.txt */
 #include "artnetparams.h"
 #include "storeartnet.h"
 #include "artnet4params.h"
 #include "storeartnet4.h"
#endif
#if defined (E131_BRIDGE)
 /* e131.txt */
 #include "e131params.h"
 #include "storee131.h"
#endif
#if defined (OSC_SERVER)
 /* osc.txt */
 #include "oscserverparms.h"
 #include "storeoscserver.h"
#endif
#if defined (DMXSEND)
 /* params.txt */
 #include "dmxparams.h"
 #include "storedmxsend.h"
#endif
#if defined (PIXEL)
 /* devices.txt */
 #include "ws28xxdmxparams.h"
 #include "storews28xxdmx.h"
 #include "tlc59711dmxparams.h"
 #include "storetlc59711.h"
#endif
#if defined (LTC_READER)
 /* ltc.txt */
 #include "ltcparams.h"
 #include "storeltc.h"
 /* ldisplay.txt */
 #include "ltcdisplayparams.h"
 #include "storeltcdisplay.h"
 /* tcnet.txt */
 #include "tcnetparams.h"
 #include "storetcnet.h"
#endif
#if defined (DMX_MONITOR)
 #include "dmxmonitorparams.h"
 #include "storemonitor.h"
#endif
#if defined (OSC_CLIENT)
 /* oscclnt.txt */
 #include "oscclientparams.h"
 #include "storeoscclient.h"
#endif
#if defined(DISPLAY_UDF)
 /* display.txt */
 #include "displayudfparams.h"
 #include "storedisplayudf.h"
#endif
#if defined(STEPPER)
 /* sparkfun.txt */
 #include "sparkfundmxparams.h"
 #include "storesparkfundmx.h"
 /* motor%.txt */
 #include "modeparams.h"
 #include "motorparams.h"
 #include "l6470params.h"
 #include "storemotors.h"
#endif
#if defined(RDM_RESPONDER)
 /* rdm_device.txt */
 #include "rdmdeviceparams.h"
 #include "storerdmdevice.h"
#endif
#if defined(SHOWFILE)
 /* show.txt */
 #include "showfileparams.h"
 #include "storeshowfile.h"
#endif
#if defined (DMXSERIAL)
 /* serial.txt */
 #include "dmxserialparams.h"
 #include "storedmxserial.h"
#endif

// nuc-i5:~/uboot-spi/u-boot$ grep CONFIG_BOOTCOMMAND include/configs/sunxi-common.h
// #define CONFIG_BOOTCOMMAND "sf probe; sf read 48000000 180000 22000; bootm 48000000"
#define FIRMWARE_MAX_SIZE	0x22000

#include "tftpfileserver.h"
#include "spiflashinstall.h"

#include "debug.h"

#ifndef MIN
 #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

static const char sRemoteConfigs[REMOTE_CONFIG_LAST][18] ALIGNED = { "Art-Net", "sACN E1.31", "OSC Server", "LTC", "OSC Client", "RDMNet LLRP Only", "Showfile" };
static const char sRemoteConfigModes[REMOTE_CONFIG_MODE_LAST][12] ALIGNED = { "DMX", "RDM", "Monitor", "Pixel", "TimeCode", "OSC", "Config", "Stepper", "Player", "Art-Net", "Serial" };

static const char sRequestReboot[] ALIGNED = "?reboot##";
#define REQUEST_REBOOT_LENGTH (sizeof(sRequestReboot)/sizeof(sRequestReboot[0]) - 1)

static const char sRequestList[] ALIGNED = "?list#";
#define REQUEST_FILES_LENGTH (sizeof(sRequestList)/sizeof(sRequestList[0]) - 1)

static const char sRequestGet[] ALIGNED = "?get#";
#define REQUEST_GET_LENGTH (sizeof(sRequestGet)/sizeof(sRequestGet[0]) - 1)

static const char sRequestUptime[] ALIGNED = "?uptime#";
#define REQUEST_UPTIME_LENGTH (sizeof(sRequestUptime)/sizeof(sRequestUptime[0]) - 1)

static const char sRequestVersion[] ALIGNED = "?version#";
#define REQUEST_VERSION_LENGTH (sizeof(sRequestVersion)/sizeof(sRequestVersion[0]) - 1)

static const char sRequestStore[] ALIGNED = "?store#";
#define REQUEST_STORE_LENGTH (sizeof(sRequestStore)/sizeof(sRequestStore[0]) - 1)

static const char sSetStore[] ALIGNED = "!store#";
#define SET_STORE_LENGTH (sizeof(sSetStore)/sizeof(sSetStore[0]) - 1)

static const char sGetDisplay[] ALIGNED = "?display#";
#define GET_DISPLAY_LENGTH (sizeof(sGetDisplay)/sizeof(sGetDisplay[0]) - 1)

static const char sSetDisplay[] ALIGNED = "!display#";
#define SET_DISPLAY_LENGTH (sizeof(sSetDisplay)/sizeof(sSetDisplay[0]) - 1)

static const char sGetTFTP[] ALIGNED = "?tftp#";
#define GET_TFTP_LENGTH (sizeof(sGetTFTP)/sizeof(sGetTFTP[0]) - 1)

static const char sSetTFTP[] ALIGNED = "!tftp#";
#define SET_TFTP_LENGTH (sizeof(sSetTFTP)/sizeof(sSetTFTP[0]) - 1)

#define UDP_PORT			0x2905
#define UDP_BUFFER_SIZE		1024
#define UDP_DATA_MIN_SIZE	MIN(MIN(MIN(MIN(REQUEST_REBOOT_LENGTH, REQUEST_FILES_LENGTH),REQUEST_GET_LENGTH),REQUEST_UPTIME_LENGTH),SET_DISPLAY_LENGTH)

RemoteConfig *RemoteConfig::s_pThis = 0;

RemoteConfig::RemoteConfig(TRemoteConfig tRemoteConfig, TRemoteConfigMode tRemoteConfigMode, uint8_t nOutputs):
	m_tRemoteConfig(tRemoteConfig),
	m_tRemoteConfigMode(tRemoteConfigMode),
	m_nOutputs(nOutputs),
	m_bDisable(false),
	m_bDisableWrite(false),
	m_bEnableReboot(false),
	m_bEnableUptime(false),
	m_bEnableTFTP(false),
	m_pTFTPFileServer(0),
	m_pTFTPBuffer(0),
	m_nIdLength(0),
	m_nHandle(-1),
	m_pUdpBuffer(0),
	m_nIPAddressFrom(0),
	m_nBytesReceived(0),
	m_tRemoteConfigHandleMode(REMOTE_CONFIG_HANDLE_MODE_TXT),
	m_pStoreBuffer(0),
	m_bIsReboot(false)

{
	assert(tRemoteConfig < REMOTE_CONFIG_LAST);
	assert(tRemoteConfigMode < REMOTE_CONFIG_MODE_LAST);

	s_pThis = this;

	Network::Get()->MacAddressCopyTo(m_tRemoteConfigListBin.aMacAddress);
	m_tRemoteConfigListBin.nType = tRemoteConfig;
	m_tRemoteConfigListBin.nMode = tRemoteConfigMode;
	m_tRemoteConfigListBin.nActiveUniverses = nOutputs;
	m_tRemoteConfigListBin.aDisplayName[0] = '\0';

#ifndef NDEBUG
	DEBUG_PUTS("m_tRemoteConfigListBin");
	debug_dump(&m_tRemoteConfigListBin, sizeof m_tRemoteConfigListBin);
#endif

	m_nHandle = Network::Get()->Begin(UDP_PORT);
	assert(m_nHandle != -1);

	m_pUdpBuffer = new char[UDP_BUFFER_SIZE];
	assert(m_pUdpBuffer != 0);

	m_pStoreBuffer = new uint8_t[UDP_BUFFER_SIZE];
	assert(m_pStoreBuffer != 0);
}

RemoteConfig::~RemoteConfig(void) {
	delete [] m_pStoreBuffer;
	m_pStoreBuffer = 0;

	delete [] m_pUdpBuffer;
	m_pUdpBuffer = 0;
}

void RemoteConfig::SetDisable(bool bDisable) {
	if (bDisable && !m_bDisable) {
//		Network::Get()->End(UDP_PORT);
//		m_nHandle = -1;
		m_bDisable = true;
	} else if (!bDisable && m_bDisable) {
//		m_nHandle = Network::Get()->Begin(UDP_PORT);
//		assert(m_nHandle != -1);
		m_bDisable = false;
	}

	DEBUG_PRINTF("m_bDisable=%d", (int) m_bDisable);
}

void RemoteConfig::SetDisplayName(const char *pDisplayName) {
	DEBUG_ENTRY

	strncpy(m_tRemoteConfigListBin.aDisplayName, pDisplayName, REMOTE_CONFIG_DISPLAY_NAME_LENGTH - 1);
	m_tRemoteConfigListBin.aDisplayName[REMOTE_CONFIG_DISPLAY_NAME_LENGTH - 1] = '\0';

#ifndef NDEBUG
	debug_dump(&m_tRemoteConfigListBin, sizeof m_tRemoteConfigListBin);
#endif

	DEBUG_EXIT
}

int RemoteConfig::Run(void) {
	uint16_t nForeignPort;

	if (__builtin_expect((m_bDisable), 1)) {
		return 0;
	}

	if (__builtin_expect((m_pTFTPFileServer != 0), 0)) {
		m_pTFTPFileServer->Run();
	}

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, m_pUdpBuffer, UDP_BUFFER_SIZE, &m_nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((m_nBytesReceived < UDP_DATA_MIN_SIZE), 1)) {
		return 0;
	}

#ifndef NDEBUG
	debug_dump(m_pUdpBuffer, m_nBytesReceived);
#endif

	if (m_pUdpBuffer[m_nBytesReceived - 1] == '\n') {
		DEBUG_PUTS("\'\\n\'");
		m_nBytesReceived--;
	}

	if (m_pUdpBuffer[0] == '?') {
		DEBUG_PUTS("?");
		if ((m_bEnableReboot) && (memcmp(m_pUdpBuffer, sRequestReboot, REQUEST_REBOOT_LENGTH) == 0)) {
			HandleReboot();
		} else if ((m_bEnableUptime) && (memcmp(m_pUdpBuffer, sRequestUptime, REQUEST_UPTIME_LENGTH) == 0)) {
			HandleUptime();
		} else if (memcmp(m_pUdpBuffer, sRequestVersion, REQUEST_VERSION_LENGTH) == 0) {
			HandleVersion();
		} else if (memcmp(m_pUdpBuffer, sRequestList, REQUEST_FILES_LENGTH) == 0) {
			HandleList();
		} else if ((m_nBytesReceived > REQUEST_GET_LENGTH) && (memcmp(m_pUdpBuffer, sRequestGet, REQUEST_GET_LENGTH) == 0)) {
			HandleGet();
		} else if ((m_nBytesReceived > REQUEST_STORE_LENGTH) && (memcmp(m_pUdpBuffer, sRequestStore, REQUEST_STORE_LENGTH) == 0)) {
			HandleStoreGet();
		} else if ((m_nBytesReceived >= GET_DISPLAY_LENGTH) && (memcmp(m_pUdpBuffer, sGetDisplay, GET_DISPLAY_LENGTH) == 0)) {
			HandleDisplayGet();
		} else if ((m_nBytesReceived >= GET_TFTP_LENGTH) && (memcmp(m_pUdpBuffer, sGetTFTP, GET_TFTP_LENGTH) == 0)) {
			HandleTftpGet();
		} else {
#ifndef NDEBUG
			Network::Get()->SendTo(m_nHandle, "?#ERROR#\n", 9, m_nIPAddressFrom, UDP_PORT);
#endif
		}
	} else if (!m_bDisableWrite) {
		if (m_pUdpBuffer[0] == '#') {
			DEBUG_PUTS("#");
			m_tRemoteConfigHandleMode = REMOTE_CONFIG_HANDLE_MODE_TXT;
			HandleTxtFile();
		} else if (m_pUdpBuffer[0] == '!') {
			DEBUG_PUTS("!");
			if ((m_nBytesReceived >= SET_DISPLAY_LENGTH) && (memcmp(m_pUdpBuffer, sSetDisplay, SET_DISPLAY_LENGTH) == 0)) {
				DEBUG_PUTS(sSetDisplay);
				HandleDisplaySet();
			} else if ((m_nBytesReceived == SET_TFTP_LENGTH + 1) && (memcmp(m_pUdpBuffer, sSetTFTP, SET_TFTP_LENGTH) == 0)) {
				DEBUG_PUTS(sSetTFTP);
				HandleTftpSet();
			} else if ((m_nBytesReceived > SET_STORE_LENGTH) && (memcmp(m_pUdpBuffer, sSetStore, SET_STORE_LENGTH) == 0)) {
				DEBUG_PUTS(sSetStore);
				m_tRemoteConfigHandleMode = REMOTE_CONFIG_HANDLE_MODE_BIN;
				HandleTxtFile();
			} else {
#ifndef NDEBUG
				Network::Get()->SendTo(m_nHandle, "!#ERROR#\n", 9, m_nIPAddressFrom, UDP_PORT);
#endif
				return 0;
			}
		}
	} else {
		return 0;
	}

	return m_nBytesReceived;
}

void RemoteConfig::HandleUptime() {
	DEBUG_ENTRY

	const uint32_t nUptime = Hardware::Get()->GetUpTime();

	if (m_nBytesReceived == REQUEST_UPTIME_LENGTH) {
		const uint32_t nLength = snprintf(m_pUdpBuffer, UDP_BUFFER_SIZE - 1, "uptime: %ds\n", static_cast<int>(nUptime));
		Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, nLength, m_nIPAddressFrom, UDP_PORT);
	} else if (m_nBytesReceived == REQUEST_UPTIME_LENGTH + 3) {
		DEBUG_PUTS("Check for \'bin\' parameter");
		if (memcmp(&m_pUdpBuffer[REQUEST_UPTIME_LENGTH], "bin", 3) == 0) {
			Network::Get()->SendTo(m_nHandle, &nUptime, sizeof(uint32_t) , m_nIPAddressFrom, UDP_PORT);
		}
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleVersion() {
	DEBUG_ENTRY

	if (m_nBytesReceived == REQUEST_VERSION_LENGTH) {
		const char *p = FirmwareVersion::Get()->GetPrint();
		const uint32_t nLength = snprintf(m_pUdpBuffer, UDP_BUFFER_SIZE, "version:%s", p);
		Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, nLength, m_nIPAddressFrom, UDP_PORT);
	} else if (m_nBytesReceived == REQUEST_VERSION_LENGTH + 3) {
		DEBUG_PUTS("Check for \'bin\' parameter");
		if (memcmp(&m_pUdpBuffer[REQUEST_VERSION_LENGTH], "bin", 3) == 0) {
			const uint8_t *p = reinterpret_cast<const uint8_t*>(FirmwareVersion::Get()->GetVersion());
			Network::Get()->SendTo(m_nHandle, p, sizeof(struct TFirmwareVersion) , m_nIPAddressFrom, UDP_PORT);
		}
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleList(void) {
	DEBUG_ENTRY

	if (m_tRemoteConfigListBin.aDisplayName[0] != '\0') {
		m_nIdLength = snprintf(m_aId, sizeof(m_aId) - 1, "" IPSTR ",%s,%s,%d,%s\n", IP2STR(Network::Get()->GetIp()), sRemoteConfigs[m_tRemoteConfig], sRemoteConfigModes[m_tRemoteConfigMode], m_nOutputs, m_tRemoteConfigListBin.aDisplayName);
	} else {
		m_nIdLength = snprintf(m_aId, sizeof(m_aId) - 1, "" IPSTR ",%s,%s,%d\n", IP2STR(Network::Get()->GetIp()), sRemoteConfigs[m_tRemoteConfig], sRemoteConfigModes[m_tRemoteConfigMode], m_nOutputs);
	}

	if (m_nBytesReceived == REQUEST_FILES_LENGTH) {
		Network::Get()->SendTo(m_nHandle, m_aId, m_nIdLength, m_nIPAddressFrom, UDP_PORT);
	} else if (m_nBytesReceived == REQUEST_FILES_LENGTH + 3) {
		DEBUG_PUTS("Check for \'bin\' parameter");
		if (memcmp(&m_pUdpBuffer[REQUEST_FILES_LENGTH], "bin", 3) == 0) {
			Network::Get()->SendTo(m_nHandle, &m_tRemoteConfigListBin, sizeof(struct TRemoteConfigListBin) , m_nIPAddressFrom, UDP_PORT);
		}
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleDisplaySet() {
	DEBUG_ENTRY

	DEBUG_PRINTF("%c", m_pUdpBuffer[SET_DISPLAY_LENGTH]);

	Display::Get()->SetSleep(m_pUdpBuffer[SET_DISPLAY_LENGTH] == '0');

	DEBUG_EXIT
}

void RemoteConfig::HandleDisplayGet() {
	DEBUG_ENTRY

	const bool isOn = !(Display::Get()->isSleep());

	if (m_nBytesReceived == GET_DISPLAY_LENGTH) {
		const uint32_t nLength = snprintf(m_pUdpBuffer, UDP_BUFFER_SIZE - 1, "display:%s\n", isOn ? "On" : "Off");
		Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, nLength, m_nIPAddressFrom, UDP_PORT);
	} else if (m_nBytesReceived == GET_DISPLAY_LENGTH + 3) {
		DEBUG_PUTS("Check for \'bin\' parameter");
		if (memcmp(&m_pUdpBuffer[GET_DISPLAY_LENGTH], "bin", 3) == 0) {
			Network::Get()->SendTo(m_nHandle, &isOn, sizeof(bool) , m_nIPAddressFrom, UDP_PORT);
		}
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleStoreGet(void) {
	DEBUG_ENTRY

	uint32_t nLenght = UDP_BUFFER_SIZE - REQUEST_STORE_LENGTH;

	const uint32_t nIndex = GetIndex(&m_pUdpBuffer[REQUEST_STORE_LENGTH], nLenght);

	if (nIndex != TXT_FILE_LAST) {
		SpiFlashStore::Get()->CopyTo(GetStore(static_cast<TTxtFile>(nIndex)), m_pUdpBuffer, nLenght);
	} else {
		Network::Get()->SendTo(m_nHandle, "?store#ERROR#\n", 12, m_nIPAddressFrom, UDP_PORT);
		return;
	}

#ifndef NDEBUG
	debug_dump(m_pUdpBuffer, nLenght);
#endif
	Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, nLenght, m_nIPAddressFrom, UDP_PORT);

	DEBUG_EXIT
}

void RemoteConfig::HandleGet(void) {
	DEBUG_ENTRY

	uint32_t nSize = UDP_BUFFER_SIZE - REQUEST_GET_LENGTH;

	const uint32_t i = GetIndex(static_cast<void*>(&m_pUdpBuffer[REQUEST_GET_LENGTH]), nSize);

	switch (i) {
	case TXT_FILE_RCONFIG:
		HandleGetRconfigTxt(nSize);
		break;
	case TXT_FILE_NETWORK:
		HandleGetNetworkTxt(nSize);
		break;
#if defined (ARTNET_NODE)
	case TXT_FILE_ARTNET:
		HandleGetArtnetTxt(nSize);
		break;
#endif
#if defined (E131_BRIDGE)
	case TXT_FILE_E131:
		HandleGetE131Txt(nSize);
		break;
#endif
#if defined (OSC_SERVER)
	case TXT_FILE_OSC:
		HandleGetOscTxt(nSize);
		break;
#endif
#if defined (DMXSEND)
	case TXT_FILE_PARAMS:
		HandleGetParamsTxt(nSize);
		break;
#endif
#if defined (PIXEL)
	case TXT_FILE_DEVICES:
		HandleGetDevicesTxt(nSize);
		break;
#endif
#if defined (LTC_READER)
	case TXT_FILE_LTC:
		HandleGetLtcTxt(nSize);
		break;
	case TXT_FILE_LTCDISPLAY:
		HandleGetLtcDisplayTxt(nSize);
		break;
	case TXT_FILE_TCNET:
		HandleGetTCNetTxt(nSize);
		break;
#endif
#if defined (DMX_MONITOR)
	case TXT_FILE_MONITOR:
		HandleGetMonTxt(nSize);
		break;
#endif
#if defined (OSC_CLIENT)
	case TXT_FILE_OSC_CLIENT:
		HandleGetOscClntTxt(nSize);
		break;
#endif
#if defined(DISPLAY_UDF)
	case TXT_FILE_DISPLAY_UDF:
		HandleGetDisplayTxt(nSize);
		break;
#endif
#if defined(STEPPER)
	case TXT_FILE_SPARKFUN:
		HandleGetSparkFunTxt(nSize);
		break;
	case TXT_FILE_MOTOR0:
	case TXT_FILE_MOTOR1:
	case TXT_FILE_MOTOR2:
	case TXT_FILE_MOTOR3:
		HandleGetMotorTxt((i - TXT_FILE_MOTOR0), nSize);
		break;
#endif
#if defined(RDM_RESPONDER)
	case TXT_FILE_RDM:
		HandleGetRdmTxt(nSize);
		break;
#endif
#if defined(SHOWFILE)
	case TXT_FILE_SHOW:
		HandleGetShowTxt(nSize);
		break;
#endif
#if defined (DMXSERIAL)
	case TXT_FILE_SERIAL:
		HandleGetSerialTxt(nSize);
		break;
#endif
	default:
		Network::Get()->SendTo(m_nHandle, "?get#ERROR#\n", 12, m_nIPAddressFrom, UDP_PORT);
		DEBUG_EXIT
		return;
		__builtin_unreachable ();
		break;
	}

#ifndef NDEBUG
	debug_dump(m_pUdpBuffer, nSize);
#endif
	Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, nSize, m_nIPAddressFrom, UDP_PORT);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetRconfigTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	RemoteConfigParams remoteConfigParams(StoreRemoteConfig::Get());
	remoteConfigParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetNetworkTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	NetworkParams networkParams(SpiFlashStore::Get()->GetStoreNetwork());
	networkParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

#if defined (ARTNET_NODE)
void RemoteConfig::HandleGetArtnetTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	uint32_t nSizeArtNet3 = 0;

	ArtNetParams artnetparams(SpiFlashStore::Get()->GetStoreArtNet());
	artnetparams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSizeArtNet3);

	DEBUG_PRINTF("nSizeArtNet3=%d", nSizeArtNet3);

	uint32_t nSizeArtNet4 = 0;

	ArtNet4Params artnet4params(SpiFlashStore::Get()->GetStoreArtNet4());
	artnet4params.Save(m_pUdpBuffer + nSizeArtNet3, UDP_BUFFER_SIZE - nSizeArtNet3, nSizeArtNet4);

	DEBUG_PRINTF("nSizeArtNet4=%d", nSizeArtNet4);

	nSize = nSizeArtNet3 + nSizeArtNet4;

	DEBUG_EXIT
}
#endif

#if defined (E131_BRIDGE)
void RemoteConfig::HandleGetE131Txt(uint32_t& nSize) {
	DEBUG_ENTRY

	E131Params e131params(StoreE131::Get());
	e131params.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OSC_SERVER)
void RemoteConfig::HandleGetOscTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	OSCServerParams oscServerParams(StoreOscServer::Get());
	oscServerParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (OSC_CLIENT)
void RemoteConfig::HandleGetOscClntTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	OscClientParams oscClientParams(StoreOscClient::Get());
	oscClientParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (DMXSEND)
void RemoteConfig::HandleGetParamsTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DMXParams dmxparams(StoreDmxSend::Get());
	dmxparams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (PIXEL)
void RemoteConfig::HandleGetDevicesTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	bool bIsSetLedType = false;

	TLC59711DmxParams tlc5911params(StoreTLC59711::Get());

	if (tlc5911params.Load()) {
		if ((bIsSetLedType = tlc5911params.IsSetLedType()) == true) {
			tlc5911params.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		}
	}

	if (!bIsSetLedType) {
		WS28xxDmxParams ws28xxparms(StoreWS28xxDmx::Get());
		ws28xxparms.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
	}

	DEBUG_EXIT
}
#endif

#if defined (LTC_READER)
void RemoteConfig::HandleGetLtcTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	LtcParams ltcParams(StoreLtc::Get());
	ltcParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetLtcDisplayTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	LtcDisplayParams ltcDisplayParams(StoreLtcDisplay::Get());
	ltcDisplayParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetTCNetTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	TCNetParams tcnetParams(StoreTCNet::Get());
	tcnetParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(DMX_MONITOR)
void RemoteConfig::HandleGetMonTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DMXMonitorParams monitorParams(StoreMonitor::Get());
	monitorParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(DISPLAY_UDF)
void RemoteConfig::HandleGetDisplayTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DisplayUdfParams displayParams(StoreDisplayUdf::Get());
	displayParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(STEPPER)
void RemoteConfig::HandleGetSparkFunTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	SparkFunDmxParams sparkFunParams(StoreSparkFunDmx::Get());
	sparkFunParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetMotorTxt(uint32_t nMotorIndex, uint32_t& nSize) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nMotorIndex=%d", (int) nMotorIndex);

	uint32_t nSizeSparkFun = 0;

	SparkFunDmxParams sparkFunParams(StoreSparkFunDmx::Get());
	sparkFunParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSizeSparkFun, nMotorIndex);

	DEBUG_PRINTF("nSizeSparkFun=%d", nSizeSparkFun);

	uint32_t nSizeMode = 0;

	ModeParams modeParams(StoreMotors::Get());
	modeParams.Save(nMotorIndex, m_pUdpBuffer + nSizeSparkFun, UDP_BUFFER_SIZE - nSizeSparkFun, nSizeMode);

	DEBUG_PRINTF("nSizeMode=%d", nSizeMode);

	uint32_t nSizeMotor = 0;

	MotorParams motorParams(StoreMotors::Get());
	motorParams.Save(nMotorIndex, m_pUdpBuffer + nSizeSparkFun + nSizeMode, UDP_BUFFER_SIZE - nSizeSparkFun - nSizeMode, nSizeMotor);

	DEBUG_PRINTF("nSizeMotor=%d", nSizeMotor);

	uint32_t nSizeL6470 = 0;

	L6470Params l6470Params(StoreMotors::Get());
	l6470Params.Save(nMotorIndex, m_pUdpBuffer + nSizeSparkFun + nSizeMode + nSizeMotor, UDP_BUFFER_SIZE - nSizeSparkFun - nSizeMode - nSizeMotor, nSizeL6470);

	DEBUG_PRINTF("nSizeL6470=%d", nSizeL6470);

	nSize = nSizeSparkFun + nSizeMode + nSizeMotor + nSizeL6470;

	DEBUG_EXIT
}
#endif

#if defined(RDM_RESPONDER)
void RemoteConfig::HandleGetRdmTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	RDMDeviceParams rdmDeviceParams(StoreRDMDevice::Get());
	rdmDeviceParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined(SHOWFILE)
void RemoteConfig::HandleGetShowTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	ShowFileParams showFileParams(StoreShowFile::Get());
	showFileParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

#if defined (DMXSERIAL)
void RemoteConfig::HandleGetSerialTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DmxSerialParams dmxSerialParams(StoreDmxSerial::Get());
	dmxSerialParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}
#endif

/*
 *
 */

void RemoteConfig::HandleTxtFile(void) {
	DEBUG_ENTRY

	uint32_t i;
	uint32_t nLength;

	if (m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_TXT) {
		 nLength = UDP_BUFFER_SIZE - 1;
		i = GetIndex(static_cast<void*>(&m_pUdpBuffer[1]), nLength);
	} else {
		nLength = UDP_BUFFER_SIZE - SET_STORE_LENGTH;
		i = GetIndex(static_cast<void*>(&m_pUdpBuffer[SET_STORE_LENGTH]), nLength);
		if (i < TXT_FILE_LAST) {
			memcpy(m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE);
		} else {
			return;
		}
	}

	switch (i) {
	case TXT_FILE_RCONFIG:
		HandleTxtFileRconfig();
		break;
	case TXT_FILE_NETWORK:
		HandleTxtFileNetwork();
		break;
#if defined (ARTNET_NODE)
	case TXT_FILE_ARTNET:
		HandleTxtFileArtnet();
		break;
#endif
#if defined (E131_BRIDGE)
	case TXT_FILE_E131:
		HandleTxtFileE131();
		break;
#endif
#if defined (OSC_SERVER)
	case TXT_FILE_OSC:
		HandleTxtFileOsc();
		break;
#endif
#if defined (DMXSEND)
	case TXT_FILE_PARAMS:
		HandleTxtFileParams();
		break;
#endif
#if defined (PIXEL)
	case TXT_FILE_DEVICES:
		HandleTxtFileDevices();
		break;
#endif
#if defined (LTC_READER)
	case TXT_FILE_LTC:
		HandleTxtFileLtc();
		break;
	case TXT_FILE_LTCDISPLAY:
		HandleTxtFileLtcDisplay();
		break;
	case TXT_FILE_TCNET:
		HandleTxtFileTCNet();
		break;
#endif
#if defined (OSC_CLIENT)
	case TXT_FILE_OSC_CLIENT:
		HandleTxtFileOscClient();
		break;
#endif
#if defined(DMX_MONITOR)
	case TXT_FILE_MONITOR:
		HandleTxtFileMon();
		break;
#endif
#if defined(DISPLAY_UDF)
	case TXT_FILE_DISPLAY_UDF:
		HandleTxtFileDisplay();
		break;
#endif
#if defined(STEPPER)
	case TXT_FILE_SPARKFUN:
		HandleTxtFileSparkFun();
		break;
	case TXT_FILE_MOTOR0:
	case TXT_FILE_MOTOR1:
	case TXT_FILE_MOTOR2:
	case TXT_FILE_MOTOR3:
		HandleTxtFileMotor(i - TXT_FILE_MOTOR0);
		break;
#endif
#if defined(RDM_RESPONDER)
	case TXT_FILE_RDM:
		HandleTxtFileRdm();
		break;
#endif
#if defined(SHOWFILE)
	case TXT_FILE_SHOW:
		HandleTxtFileShow();
		break;
#endif
#if defined (DMXSERIAL)
	case TXT_FILE_SERIAL:
		HandleTxtFileSerial();
		break;
#endif
	default:
		break;
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileRconfig(void) {
	DEBUG_ENTRY

	RemoteConfigParams remoteConfigParams(StoreRemoteConfig::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN) && (m_nBytesReceived == sizeof(struct TRemoteConfigParams)))  {
		uint32_t nSize;
		remoteConfigParams.Builder(reinterpret_cast<const struct TRemoteConfigParams*>(m_pStoreBuffer), m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	remoteConfigParams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	remoteConfigParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileNetwork(void) {
	DEBUG_ENTRY

	NetworkParams params(SpiFlashStore::Get()->GetStoreNetwork());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN) && (m_nBytesReceived == sizeof(struct TNetworkParams))){
		uint32_t nSize;
		params.Builder(reinterpret_cast<const struct TNetworkParams*>(m_pStoreBuffer), m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	params.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	params.Dump();
#endif

	DEBUG_EXIT
}

#if defined (ARTNET_NODE)
void RemoteConfig::HandleTxtFileArtnet(void) {
	DEBUG_ENTRY
	assert(sizeof(struct TArtNet4Params) != sizeof(struct TArtNetParams));

	ArtNet4Params artnet4params(SpiFlashStore::Get()->GetStoreArtNet4());

	if (m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN) {
		if (m_nBytesReceived == sizeof(struct TArtNetParams)) {
			ArtNetParams artnetparams(SpiFlashStore::Get()->GetStoreArtNet());
			uint32_t nSize;
			artnetparams.Builder(reinterpret_cast<const struct TArtNetParams*>(m_pStoreBuffer), m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		} else if (m_nBytesReceived == sizeof(struct TArtNet4Params)) {
			uint32_t nSize;
			artnet4params.Builder(reinterpret_cast<const struct TArtNet4Params*>(m_pStoreBuffer), m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		}
	}

	artnet4params.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	artnet4params.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (E131_BRIDGE)
void RemoteConfig::HandleTxtFileE131(void) {
	DEBUG_ENTRY

	E131Params e131params(StoreE131::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN)  && (m_nBytesReceived == sizeof(struct TE131Params))){
		uint32_t nSize;
		e131params.Builder((const struct TE131Params *)m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	e131params.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	e131params.Dump();
#endif
	DEBUG_EXIT
}
#endif

#if defined (OSC_SERVER)
void RemoteConfig::HandleTxtFileOsc(void) {
	DEBUG_ENTRY

	OSCServerParams oscServerParams(StoreOscServer::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN)  && (m_nBytesReceived == sizeof(struct TOSCServerParams))){
		uint32_t nSize;
		oscServerParams.Builder(reinterpret_cast<const struct TOSCServerParams*>(m_pStoreBuffer), m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	oscServerParams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	oscServerParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (OSC_CLIENT)
void RemoteConfig::HandleTxtFileOscClient(void) {
	DEBUG_ENTRY

	OscClientParams oscClientParams(StoreOscClient::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN)  && (m_nBytesReceived == sizeof(struct TOscClientParams))){
		uint32_t nSize;
		oscClientParams.Builder((const struct TOscClientParams *)m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	oscClientParams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	oscClientParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (DMXSEND)
void RemoteConfig::HandleTxtFileParams(void) {
	DEBUG_ENTRY

	DMXParams dmxparams(StoreDmxSend::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN)  && (m_nBytesReceived == sizeof(struct TDMXParams))){
		uint32_t nSize;
		dmxparams.Builder((const struct TDMXParams *)m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	dmxparams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	dmxparams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (PIXEL)
void RemoteConfig::HandleTxtFileDevices(void) {
	DEBUG_ENTRY
	assert(sizeof(struct TTLC59711DmxParams) != sizeof(struct TWS28xxDmxParams));

	TLC59711DmxParams tlc59711params(StoreTLC59711::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN)  && (m_nBytesReceived == sizeof(struct TTLC59711DmxParams))){
		uint32_t nSize;
		tlc59711params.Builder((const struct TTLC59711DmxParams *)m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	tlc59711params.Load((const char *)m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	tlc59711params.Dump();
#endif
	DEBUG_PRINTF("tlc5911params.IsSetLedType()=%d", tlc59711params.IsSetLedType());

	if (!tlc59711params.IsSetLedType()) {
		WS28xxDmxParams ws28xxparms(StoreWS28xxDmx::Get());

		if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN)  && (m_nBytesReceived == sizeof(struct TWS28xxDmxParams))){
			uint32_t nSize;
			ws28xxparms.Builder((const struct TWS28xxDmxParams *)m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
			m_nBytesReceived = nSize;
		}

		ws28xxparms.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
		ws28xxparms.Dump();
#endif
	}

	DEBUG_EXIT
}
#endif

#if defined (LTC_READER)
void RemoteConfig::HandleTxtFileLtc(void) {
	DEBUG_ENTRY

	LtcParams ltcParams(StoreLtc::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN)  && (m_nBytesReceived == sizeof(struct TLtcParams))){
		uint32_t nSize;
		ltcParams.Builder((const struct TLtcParams *)m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	ltcParams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	ltcParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileLtcDisplay(void) {
	DEBUG_ENTRY

	LtcDisplayParams ltcDisplayParams(StoreLtcDisplay::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN)  && (m_nBytesReceived == sizeof(struct TLtcDisplayParams))){
		uint32_t nSize;
		ltcDisplayParams.Builder((const struct TLtcDisplayParams *)m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	ltcDisplayParams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	ltcDisplayParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileTCNet(void) {
	DEBUG_ENTRY

	TCNetParams tcnetParams(StoreTCNet::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN) && (m_nBytesReceived == sizeof(struct TTCNetParams))){
		uint32_t nSize;
		tcnetParams.Builder((const struct TTCNetParams *)m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	tcnetParams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	tcnetParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined(DMX_MONITOR)
void RemoteConfig::HandleTxtFileMon(void) {
	DEBUG_ENTRY

	DMXMonitorParams monitorParams(StoreMonitor::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN) && (m_nBytesReceived == sizeof(struct TDMXMonitorParams))){
		uint32_t nSize;
		monitorParams.Builder((const struct TDMXMonitorParams *)m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	monitorParams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	monitorParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined(DISPLAY_UDF)
void RemoteConfig::HandleTxtFileDisplay(void) {
	DEBUG_ENTRY

	DisplayUdfParams displayParams(StoreDisplayUdf::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN) && (m_nBytesReceived == sizeof(struct TDisplayUdfParams))){
		uint32_t nSize;
		displayParams.Builder((const struct TDisplayUdfParams *)m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	displayParams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	displayParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined(STEPPER)
void RemoteConfig::HandleTxtFileSparkFun(void) {
	DEBUG_ENTRY

	SparkFunDmxParams sparkFunDmxParams(StoreSparkFunDmx::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN) && (m_nBytesReceived == sizeof(struct TSparkFunDmxParams))){
		uint32_t nSize;
		sparkFunDmxParams.Builder((const struct TSparkFunDmxParams *)m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	sparkFunDmxParams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	sparkFunDmxParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileMotor(uint32_t nMotorIndex) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nMotorIndex=%d", (int) nMotorIndex);

	if (m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN) {
		// TODO HandleTxtFileMotor REMOTE_CONFIG_HANDLE_MODE_BIN
		return;
	}

	SparkFunDmxParams sparkFunDmxParams(StoreSparkFunDmx::Get());
	sparkFunDmxParams.Load(nMotorIndex, m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	sparkFunDmxParams.Dump();
#endif

	ModeParams modeParams(StoreMotors::Get());
	modeParams.Load(nMotorIndex, m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	modeParams.Dump();
#endif

	MotorParams motorParams(StoreMotors::Get());
	motorParams.Load(nMotorIndex, m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	motorParams.Dump();
#endif

	L6470Params l6470Params(StoreMotors::Get());
	l6470Params.Load(nMotorIndex, m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	l6470Params.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined(RDM_RESPONDER)
void RemoteConfig::HandleTxtFileRdm(void) {
	DEBUG_ENTRY

	RDMDeviceParams rdmDeviceParams(StoreRDMDevice::Get());
	rdmDeviceParams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	rdmDeviceParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined(SHOWFILE)
void RemoteConfig::HandleTxtFileShow(void) {
	DEBUG_ENTRY

	ShowFileParams showFileParams(StoreShowFile::Get());
	showFileParams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	showFileParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

#if defined (DMXSERIAL)
void RemoteConfig::HandleTxtFileSerial(void) {
	DEBUG_ENTRY

	DmxSerialParams dmxSerialParams(StoreDmxSerial::Get());

	if ((m_tRemoteConfigHandleMode == REMOTE_CONFIG_HANDLE_MODE_BIN)  && (m_nBytesReceived == sizeof(struct TDmxSerialParams))){
		uint32_t nSize;
		dmxSerialParams.Builder((const struct TDmxSerialParams *)m_pStoreBuffer, m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		m_nBytesReceived = nSize;
	}

	dmxSerialParams.Load(m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	dmxSerialParams.Dump();
#endif

	DEBUG_EXIT
}
#endif

/**
 * TFTP Update firmware
 */

void RemoteConfig::TftpExit(void) {
	DEBUG_ENTRY

	m_pUdpBuffer[GET_TFTP_LENGTH] = '0';

	HandleTftpSet();

	DEBUG_EXIT
}

void RemoteConfig::HandleTftpSet(void) {
	DEBUG_ENTRY

	DEBUG_PRINTF("%c", m_pUdpBuffer[GET_TFTP_LENGTH]);

	m_bEnableTFTP = (m_pUdpBuffer[GET_TFTP_LENGTH] != '0');

	if (m_bEnableTFTP) {
		Display::Get()->SetSleep(false);
	}

	if (m_bEnableTFTP && (m_pTFTPFileServer == 0)) {
		printf("Create TFTP Server\n");

		m_pTFTPBuffer = new uint8_t[FIRMWARE_MAX_SIZE];
		assert(m_pTFTPBuffer != 0);

		m_pTFTPFileServer = new TFTPFileServer(m_pTFTPBuffer, FIRMWARE_MAX_SIZE);
		assert(m_pTFTPFileServer != 0);
		Display::Get()->TextStatus("TFTP On", DISPLAY_7SEGMENT_MSG_INFO_TFTP_ON);
	} else if (!m_bEnableTFTP && (m_pTFTPFileServer != 0)) {
		const uint32_t nFileSize = m_pTFTPFileServer->GetFileSize();
		DEBUG_PRINTF("nFileSize=%d, %d", (int) nFileSize, m_pTFTPFileServer->isDone());

		bool bSucces = true;

		if (m_pTFTPFileServer->isDone()) {
#ifndef NDEBUG
			debug_dump(m_pTFTPBuffer, 512);
#endif
			bSucces = SpiFlashInstall::Get()->WriteFirmware(m_pTFTPBuffer, nFileSize);

			if (!bSucces) {
				Display::Get()->TextStatus("Error: TFTP", DISPLAY_7SEGMENT_MSG_ERROR_TFTP);
			}
		}

		printf("Delete TFTP Server\n");

		delete m_pTFTPFileServer;
		m_pTFTPFileServer = 0;

		delete[] m_pTFTPBuffer;
		m_pTFTPBuffer = 0;

		if (bSucces) { // Keep error message
			Display::Get()->TextStatus("TFTP Off", DISPLAY_7SEGMENT_MSG_INFO_TFTP_OFF);
		}
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleTftpGet(void) {
	DEBUG_ENTRY

	if (m_nBytesReceived == GET_TFTP_LENGTH) {
		const uint32_t nLength = snprintf(m_pUdpBuffer, UDP_BUFFER_SIZE - 1, "tftp:%s\n", m_bEnableTFTP ? "On" : "Off");
		Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, nLength, m_nIPAddressFrom, UDP_PORT);
	} else if (m_nBytesReceived == GET_TFTP_LENGTH + 3) {
		DEBUG_PUTS("Check for \'bin\' parameter");
		if (memcmp(&m_pUdpBuffer[GET_TFTP_LENGTH], "bin", 3) == 0) {
			Network::Get()->SendTo(m_nHandle, &m_bEnableTFTP, sizeof(bool) , m_nIPAddressFrom, UDP_PORT);
		}
	}

	DEBUG_EXIT
}
