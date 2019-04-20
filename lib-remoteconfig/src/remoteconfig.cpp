/**
 * @file remoteconfig.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "hardware.h"
#include "network.h"

#include "display.h"

#include "spiflashstore.h"

/* rconfig.txt */
#include "remoteconfigparams.h"
#include "storeremoteconfig.h"
/* network.txt */
#include "networkparams.h"
/* artnet.txt */
#include "artnet4params.h"
/* params.txt */
#include "dmxparams.h"
/* e131.txt */
#include "e131params.h"
/* osc.txt */
#include "oscserverparms.h"
#include "storeoscserver.h"
/* devices.txt */
#include "tlc59711dmxparams.h"
#include "ws28xxdmxparams.h"

#include "debug.h"

#ifndef MIN
 #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

static const char sRemoteConfigs[REMOTE_CONFIG_LAST][12] ALIGNED = { "Art-Net", "sACN E1.31", "OSC" };
static const char sRemoteConfigModes[REMOTE_CONFIG_MODE_LAST][8] ALIGNED = { "DMX", "RDM", "Monitor", "Pixel" };

static const char sRequestReboot[] ALIGNED = "?reboot##";
#define REQUEST_REBOOT_LENGTH (sizeof(sRequestReboot)/sizeof(sRequestReboot[0]) - 1)

static const char sRequestList[] ALIGNED = "?list#";
#define REQUEST_LIST_LENGTH (sizeof(sRequestList)/sizeof(sRequestList[0]) - 1)

static const char sRequestGet[] ALIGNED = "?get#";
#define REQUEST_GET_LENGTH (sizeof(sRequestGet)/sizeof(sRequestGet[0]) - 1)

static const char sRequestUptime[] ALIGNED = "?uptime#";
#define REQUEST_UPTIME_LENGTH (sizeof(sRequestUptime)/sizeof(sRequestUptime[0]) - 1)

static const char sSetDisplay[] ALIGNED = "!display#";
#define SET_DISPLAY_LENGTH (sizeof(sSetDisplay)/sizeof(sSetDisplay[0]) - 1)

enum TTxtFile {
	TXT_FILE_RCONFIG,
	TXT_FILE_NETWORK,
	TXT_FILE_ARTNET,
	TXT_FILE_E131,
	TXT_FILE_OSC,
	TXT_FILE_PARAMS,
	TXT_FILE_DEVICES,
	TXT_FILE_LAST
};

static const char sTxtFile[TXT_FILE_LAST][12] =          { "rconfig.txt", "network.txt", "artnet.txt", "e131.txt", "osc.txt", "params.txt", "devices.txt" };
static const uint8_t sTxtFileNameLength[TXT_FILE_LAST] = {  11,            11,            10,           8,          7,         10,           11};

#define UDP_PORT			0x2905
#define UDP_BUFFER_SIZE		512
#define UDP_DATA_MIN_SIZE	MIN(MIN(MIN(MIN(REQUEST_REBOOT_LENGTH, REQUEST_LIST_LENGTH),REQUEST_GET_LENGTH),REQUEST_UPTIME_LENGTH),SET_DISPLAY_LENGTH)

RemoteConfig::RemoteConfig(TRemoteConfig tRemoteConfig, TRemoteConfigMode tRemoteConfigMode, uint8_t nOutputs):
	m_bDisable(false),
	m_bDisableWrite(false),
	m_bEnableReboot(false),
	m_bEnableUptime(false),
	m_nIdLength(0),
	m_nHandle(-1),
	m_pUdpBuffer(0),
	m_nIPAddressFrom(0),
	m_nBytesReceived(0)

{
	assert(tRemoteConfig < REMOTE_CONFIG_LAST);
	assert(tRemoteConfigMode < REMOTE_CONFIG_MODE_LAST);

	memset(m_aId, 0, sizeof m_aId);

	m_nIdLength = snprintf(m_aId, sizeof(m_aId) - 1, "" IPSTR ",%s,%s,%d\n", IP2STR(Network::Get()->GetIp()), sRemoteConfigs[tRemoteConfig], sRemoteConfigModes[tRemoteConfigMode], nOutputs);

	DEBUG_PRINTF("%d:[%s]", m_nIdLength, m_aId);

	m_nHandle = Network::Get()->Begin(UDP_PORT);
	assert(m_nHandle != -1);

	m_pUdpBuffer = new uint8_t[UDP_BUFFER_SIZE];
	assert(m_pUdpBuffer != 0);
}

RemoteConfig::~RemoteConfig(void) {
	delete [] m_pUdpBuffer;
	m_pUdpBuffer = 0;
}

void RemoteConfig::SetDisable(bool bDisable) {
	if (bDisable && !m_bDisable) {
		Network::Get()->End(UDP_PORT);
		m_nHandle = -1;
		m_bDisable = true;
	} else if (!bDisable && m_bDisable) {
		m_nHandle = Network::Get()->Begin(UDP_PORT);
		assert(m_nHandle != -1);

		m_bDisable = false;
	}

	DEBUG_PRINTF("m_bDisable=%d", (int) m_bDisable);
}

void RemoteConfig::SetDisableWrite(bool bDisableWrite) {
	m_bDisableWrite = bDisableWrite;

	DEBUG_PRINTF("m_bDisableWrite=%d", (int) m_bDisableWrite);
}

void RemoteConfig::SetEnableReboot(bool bEnableReboot) {
	m_bEnableReboot = bEnableReboot;

	DEBUG_PRINTF("m_bEnableReboot=%d", (int) m_bEnableReboot);
}

void RemoteConfig::SetEnableUptime(bool bEnableUptime) {
	m_bEnableUptime = bEnableUptime;

	DEBUG_PRINTF("m_bEnableUptime=%d", (int) m_bEnableUptime);
}

void RemoteConfig::SetDisplayName(const char *pDisplayName) {
	DEBUG_ENTRY

	DEBUG_PRINTF("%d:[%s]", m_nIdLength, m_aId);

	// BUG there is sanity check for adding multiple times
	uint32_t nSize = strlen(pDisplayName);

	if ((m_nIdLength + nSize + 2) < REMOTE_CONFIG_ID_LENGTH) {
		m_aId[m_nIdLength - 1] = ',';
		strcpy((char *) &m_aId[m_nIdLength], pDisplayName);
		m_aId[m_nIdLength + nSize] = '\n';
	}

	m_nIdLength += (nSize + 1);

	DEBUG_PRINTF("%d:[%s]", m_nIdLength, m_aId);

	DEBUG_EXIT
}

int RemoteConfig::Run(void) {
	uint16_t nForeignPort;

	if (__builtin_expect ((m_bDisable), 0)) {
		return 0;
	}

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, m_pUdpBuffer, (uint16_t) UDP_BUFFER_SIZE, &m_nIPAddressFrom, &nForeignPort);

	if (m_nBytesReceived < (int) UDP_DATA_MIN_SIZE) {
		return 0;
	}

#ifndef NDEBUG
	debug_dump((void *)m_pUdpBuffer, m_nBytesReceived);
#endif

	if (m_pUdpBuffer[0] == '?') {
		DEBUG_PUTS("?");
		if ((m_bEnableReboot) && (memcmp(m_pUdpBuffer, sRequestReboot, REQUEST_REBOOT_LENGTH) == 0)) {
			HandleReboot();
		} else if ((m_bEnableUptime) && (memcmp(m_pUdpBuffer, sRequestUptime, REQUEST_UPTIME_LENGTH) == 0)) {
			HandleUptime();
		} else if (memcmp(m_pUdpBuffer, sRequestList, REQUEST_LIST_LENGTH) == 0) {
			HandleList();
		} else if ((m_nBytesReceived > REQUEST_GET_LENGTH) && (memcmp(m_pUdpBuffer, sRequestGet, REQUEST_GET_LENGTH) == 0)) {
			HandleGet();
		} else {
#ifndef NDEBUG
			Network::Get()->SendTo(m_nHandle, (const uint8_t *)"?#ERROR#\n", 9, m_nIPAddressFrom, (uint16_t) UDP_PORT);
#endif
		}
	} else if ((!m_bDisableWrite) && (m_pUdpBuffer[0] == '#')) {
		DEBUG_PUTS("#");
		HandleTxtFile();
	} else if ((m_nBytesReceived >= (SET_DISPLAY_LENGTH + 1)) && (memcmp(m_pUdpBuffer, sSetDisplay, SET_DISPLAY_LENGTH) == 0)) {
		DEBUG_PUTS(sSetDisplay);
		HandleSetDisplay();
	} else {
		return 0;
	}

	return m_nBytesReceived;
}

void RemoteConfig::HandleReboot(void) {
	DEBUG_ENTRY

	while (SpiFlashStore::Get()->Flash())
		;

	Display::Get()->Cls();
	Display::Get()->Write(2, "Rebooting ...");

	Hardware::Get()->Reboot();

	DEBUG_EXIT
}

void RemoteConfig::HandleUptime() {
	DEBUG_ENTRY

	const uint64_t nUptime = Hardware::Get()->GetUpTime();
	const uint32_t nLength = snprintf((char *)m_pUdpBuffer, UDP_BUFFER_SIZE, "uptime:%ds\n", (int) nUptime);

	Network::Get()->SendTo(m_nHandle, (const uint8_t *)m_pUdpBuffer, nLength, m_nIPAddressFrom, (uint16_t) UDP_PORT);

	DEBUG_EXIT
}

void RemoteConfig::HandleList(void) {
	DEBUG_ENTRY

	Network::Get()->SendTo(m_nHandle, (const uint8_t *) m_aId, (uint16_t) m_nIdLength, m_nIPAddressFrom, (uint16_t) UDP_PORT);

	DEBUG_EXIT
}

void RemoteConfig::HandleSetDisplay() {
	DEBUG_ENTRY

	DEBUG_PRINTF("%c", m_pUdpBuffer[SET_DISPLAY_LENGTH]);

	Display::Get()->SetSleep(m_pUdpBuffer[SET_DISPLAY_LENGTH] == '0');

	DEBUG_EXIT
}

void RemoteConfig::HandleGet(void) {
	DEBUG_ENTRY

	uint32_t nSize = 0;
	const uint8_t *a1 = &m_pUdpBuffer[REQUEST_GET_LENGTH];

	uint32_t i;

	for(i = 0; i < TXT_FILE_LAST; i++) {
		if (memcmp((void *) a1, sTxtFile[i], sTxtFileNameLength[i]) == 0) {
				break;
		}
	}

	switch (i) {
	case TXT_FILE_RCONFIG:
		HandleGetRconfigTxt(nSize);
		break;
	case TXT_FILE_NETWORK:
		HandleGetNetworkTxt(nSize);
		break;
	case TXT_FILE_ARTNET:
		HandleGetArtnetTxt(nSize);
		break;
	case TXT_FILE_E131:
		HandleGetE131Txt(nSize);
		break;
	case TXT_FILE_OSC:
		HandleGetOscTxt(nSize);
		break;
	case TXT_FILE_PARAMS:
		HandleGetParamsTxt(nSize);
		break;
	case TXT_FILE_DEVICES:
		HandleGetDevicesTxt(nSize);
		break;
	default:
#ifndef NDEBUG
		Network::Get()->SendTo(m_nHandle, (const uint8_t *) "?get#ERROR#\n", 12, m_nIPAddressFrom, (uint16_t) UDP_PORT);
#endif
		DEBUG_EXIT
		return;
		break;
	}

#ifndef NDEBUG
	debug_dump((void *)m_pUdpBuffer, nSize);
#endif
	Network::Get()->SendTo(m_nHandle, m_pUdpBuffer, nSize, m_nIPAddressFrom, (uint16_t) UDP_PORT);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetRconfigTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	RemoteConfigParams remoteConfigParams((RemoteConfigParamsStore *) StoreRemoteConfig::Get());
	remoteConfigParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetNetworkTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	NetworkParams networkParams((NetworkParamsStore *) SpiFlashStore::Get()->GetStoreNetwork());
	networkParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetArtnetTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	uint32_t nSizeArtNet3 = 0;

	ArtNetParams artnetparams((ArtNetParamsStore *) SpiFlashStore::Get()->GetStoreArtNet());
	artnetparams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSizeArtNet3);

	DEBUG_PRINTF("nSizeArtNet3=%d", nSizeArtNet3);

	uint32_t nSizeArtNet4 = 0;

	ArtNet4Params artnet4params((ArtNet4ParamsStore *) SpiFlashStore::Get()->GetStoreArtNet4());
	artnet4params.Save(m_pUdpBuffer + nSizeArtNet3, UDP_BUFFER_SIZE - nSizeArtNet3, nSizeArtNet4);

	DEBUG_PRINTF("nSizeArtNet4=%d", nSizeArtNet4);

	nSize = nSizeArtNet3 + nSizeArtNet4;

	DEBUG_EXIT
}

void RemoteConfig::HandleGetE131Txt(uint32_t& nSize) {
	DEBUG_ENTRY

	E131Params e131params((E131ParamsStore *) SpiFlashStore::Get()->GetStoreE131());
	e131params.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetOscTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	OSCServerParams oscServerParams((OSCServerParamsStore *)StoreOscServer::Get());
	oscServerParams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetParamsTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	DMXParams dmxparams((DMXParamsStore *) SpiFlashStore::Get()->GetStoreDmxSend());
	dmxparams.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);

	DEBUG_EXIT
}

void RemoteConfig::HandleGetDevicesTxt(uint32_t& nSize) {
	DEBUG_ENTRY

	bool bIsSetLedType = false;

	TLC59711DmxParams tlc5911params((TLC59711DmxParamsStore *) SpiFlashStore::Get()->GetStoreTLC59711());

	if (tlc5911params.Load()) {
		if ((bIsSetLedType = tlc5911params.IsSetLedType()) == true) {
			tlc5911params.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
		}
	}

	if (!bIsSetLedType) {
		WS28xxDmxParams ws28xxparms((WS28xxDmxParamsStore *) SpiFlashStore::Get()->GetStoreWS28xxDmx());
		ws28xxparms.Save(m_pUdpBuffer, UDP_BUFFER_SIZE, nSize);
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFile(void) {
	DEBUG_ENTRY

	debug_dump((void *)m_pUdpBuffer, m_nBytesReceived);

	const uint8_t *a1 = &m_pUdpBuffer[1];
	uint32_t i;

	for(i = 0; i < TXT_FILE_LAST; i++) {
		if (memcmp((void *) a1, sTxtFile[i], sTxtFileNameLength[i]) == 0) {
				break;
		}
	}

	switch (i) {
	case TXT_FILE_RCONFIG:
		HandleTxtFileRconfig();
		break;
	case TXT_FILE_NETWORK:
		HandleTxtFileNetwork();
		break;
	case TXT_FILE_ARTNET:
		HandleTxtFileArtnet();
		break;
	case TXT_FILE_E131:
		HandleTxtFileE131();
		break;
	case TXT_FILE_OSC:
		HandleTxtFileOsc();
		break;
	case TXT_FILE_PARAMS:
		HandleTxtFileParams();
		break;
	case TXT_FILE_DEVICES:
		HandleTxtFileDevices();
		break;
	default:
		break;
	}

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileRconfig(void) {
	DEBUG_ENTRY

	RemoteConfigParams remoteConfigParams((RemoteConfigParamsStore *) StoreRemoteConfig::Get());
	remoteConfigParams.Load((const char *) m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	remoteConfigParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileNetwork(void) {
	DEBUG_ENTRY

	NetworkParams params((NetworkParamsStore *) SpiFlashStore::Get()->GetStoreNetwork());
	params.Load((const char *) m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	params.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileArtnet(void) {
	DEBUG_ENTRY

	ArtNet4Params artnet4params((ArtNet4ParamsStore *) SpiFlashStore::Get()->GetStoreArtNet4());
	artnet4params.Load((const char *) m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	artnet4params.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileE131(void) {
	DEBUG_ENTRY

	E131Params e131params((E131ParamsStore *) SpiFlashStore::Get()->GetStoreE131());
	e131params.Load((const char *) m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	e131params.Dump();
#endif
	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileOsc(void) {
	DEBUG_ENTRY

	OSCServerParams oscServerParams((OSCServerParamsStore *)StoreOscServer::Get());
	oscServerParams.Load((const char *) m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	oscServerParams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileParams(void) {
	DEBUG_ENTRY

	DMXParams dmxparams((DMXParamsStore *) SpiFlashStore::Get()->GetStoreDmxSend());
	dmxparams.Load((const char *) m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	dmxparams.Dump();
#endif

	DEBUG_EXIT
}

void RemoteConfig::HandleTxtFileDevices(void) {
	DEBUG_ENTRY

	TLC59711DmxParams tlc59711params((TLC59711DmxParamsStore *) SpiFlashStore::Get()->GetStoreTLC59711());
	tlc59711params.Load((const char *)m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
	tlc59711params.Dump();
#endif
	DEBUG_PRINTF("tlc5911params.IsSetLedType()=%d", tlc59711params.IsSetLedType());

	if (!tlc59711params.IsSetLedType()) {
		WS28xxDmxParams ws28xxparms((WS28xxDmxParamsStore *) SpiFlashStore::Get()->GetStoreWS28xxDmx());
		ws28xxparms.Load((const char *) m_pUdpBuffer, m_nBytesReceived);
#ifndef NDEBUG
		ws28xxparms.Dump();
#endif
	}

	DEBUG_EXIT
}
