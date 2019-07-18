/**
 * @file remoteconfig.h
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

#ifndef REMOTECONFIG_H_
#define REMOTECONFIG_H_

#include <stdint.h>

#include "tftpfileserver.h"

enum TRemoteConfig {
	REMOTE_CONFIG_ARTNET,
	REMOTE_CONFIG_E131,
	REMOTE_CONFIG_OSC,
	REMOTE_CONFIG_LTC,
	REMOTE_CONFIG_OSC_CLIENT,
	REMOTE_CONFIG_LAST
};

enum TRemoteConfigMode {
	REMOTE_CONFIG_MODE_DMX,
	REMOTE_CONFIG_MODE_RDM,
	REMOTE_CONFIG_MODE_MONITOR,
	REMOTE_CONFIG_MODE_PIXEL,
	REMOTE_CONFIG_MODE_TIMECODE,
	REMOTE_CONFIG_MODE_OSC,
	REMOTE_CONFIG_MODE_LAST
};

enum {
	REMOTE_CONFIG_DISPLAY_NAME_LENGTH = 24,
	REMOTE_CONFIG_ID_LENGTH = (32 + REMOTE_CONFIG_DISPLAY_NAME_LENGTH + 2) // +2, comma and \n
};

enum TRemoteConfigHandleMode {
	REMOTE_CONFIG_HANDLE_MODE_TXT,
	REMOTE_CONFIG_HANDLE_MODE_BIN
};

struct TRemoteConfigListBin {
	uint8_t aMacAddress[6];
	uint8_t nType;				// TRemoteConfig
	uint8_t nMode;				// TRemoteConfigMode
	uint8_t nActiveUniverses;
	uint8_t aDisplayName[REMOTE_CONFIG_DISPLAY_NAME_LENGTH];
}__attribute__((packed));

class RemoteConfig {
public:
	RemoteConfig(TRemoteConfig tRemoteConfig, TRemoteConfigMode tRemoteConfigMode, uint8_t nOutputs = 1);
	~RemoteConfig(void);

	void SetDisable(bool bDisable = true);
	void SetDisableWrite(bool bDisableWrite = true);
	void SetEnableReboot(bool bDisableReboot = true);
	void SetEnableUptime(bool bDisableReboot = true);
	void SetDisplayName(const char *pDisplayName);

	int Run(void);

private:
	uint32_t GetIndex(const void *p);

	void HandleReboot(void);
	void HandleList(void);
	void HandleUptime(void);
	void HandleVersion(void);

	void HandleGet(void);
	void HandleGetRconfigTxt(uint32_t& nSize);
	void HandleGetNetworkTxt(uint32_t& nSize);
	void HandleGetArtnetTxt(uint32_t& nSize);
	void HandleGetE131Txt(uint32_t& nSize);
#if defined (OSC_SERVER)
	void HandleGetOscTxt(uint32_t& nSize);
#endif
	void HandleGetParamsTxt(uint32_t& nSize);
	void HandleGetDevicesTxt(uint32_t& nSize);
#if defined (LTC_READER)
	void HandleGetLtcTxt(uint32_t& nSize);
	void HandleGetTCNetTxt(uint32_t& nSize);
#endif
#if defined (OSC_CLIENT)
	void HandleGetOscClntTxt(uint32_t& nSize);
#endif

	void HandleTxtFile(void);
	void HandleTxtFileRconfig(void);
	void HandleTxtFileNetwork(void);
	void HandleTxtFileArtnet(void);
	void HandleTxtFileE131(void);
#if defined (OSC_SERVER)
	void HandleTxtFileOsc(void);
#endif
	void HandleTxtFileParams(void);
	void HandleTxtFileDevices(void);
#if defined (LTC_READER)
	void HandleTxtFileLtc(void);
	void HandleTxtFileTCNet(void);
#endif
#if defined (OSC_CLIENT)
	void HandleTxtFileOscClient(void);
#endif

	void HandleDisplaySet(void);
	void HandleDisplayGet(void);

	void HandleStoreSet(void);
	void HandleStoreGet(void);

	void HandleTftpSet(void);
	void HandleTftpGet(void);

private:
	bool m_bDisable;
	bool m_bDisableWrite;
	bool m_bEnableReboot;
	bool m_bEnableUptime;
	bool m_bEnableTFTP;
	TFTPFileServer* m_pTFTPFileServer;
	alignas(uint32_t) uint8_t *m_pTFTPBuffer;
	char m_aId[REMOTE_CONFIG_ID_LENGTH];
	uint8_t m_nIdLength;
	struct TRemoteConfigListBin m_tRemoteConfigListBin;
	int32_t m_nHandle;
	alignas(uint32_t) uint8_t *m_pUdpBuffer;
	uint32_t m_nIPAddressFrom;
	uint16_t m_nBytesReceived;
	TRemoteConfigHandleMode m_tRemoteConfigHandleMode;
	alignas(uint32_t) uint8_t *m_pStoreBuffer;
};

#endif /* REMOTECONFIG_H_ */
