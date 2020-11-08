/**
 * @file remoteconfig.h
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

#ifndef REMOTECONFIG_H_
#define REMOTECONFIG_H_

#include <stdint.h>

#if defined (ARTNET_NODE_MULTI)
# define ARTNET_NODE
#endif

#if defined (E131_BRIDGE_MULTI)
# define E131_BRIDGE
#endif

#if defined (PIXEL_MULTI)
# define PIXEL
#endif

#if defined (DMXSEND_MULTI)
# define DMXSEND
#endif

#include "spiflashstore.h"

#include "tftpfileserver.h"

enum TRemoteConfig {
	REMOTE_CONFIG_ARTNET,
	REMOTE_CONFIG_E131,
	REMOTE_CONFIG_OSC,
	REMOTE_CONFIG_LTC,
	REMOTE_CONFIG_OSC_CLIENT,
	REMOTE_CONFIG_RDMNET_LLRP_ONLY,
	REMOTE_CONFIG_SHOWFILE,
	REMOTE_CONFIG_LAST
};

enum TRemoteConfigMode {
	REMOTE_CONFIG_MODE_DMX,
	REMOTE_CONFIG_MODE_RDM,
	REMOTE_CONFIG_MODE_MONITOR,
	REMOTE_CONFIG_MODE_PIXEL,
	REMOTE_CONFIG_MODE_TIMECODE,
	REMOTE_CONFIG_MODE_OSC,
	REMOTE_CONFIG_MODE_CONFIG,
	REMOTE_CONFIG_MODE_STEPPER,
	REMOTE_CONFIG_MODE_PLAYER,
	REMOTE_CONFIG_MODE_ARTNET,
	REMOTE_CONFIG_MODE_SERIAL,
	REMOTE_CONFIG_MODE_RGBPANEL,
	REMOTE_CONFIG_MODE_LAST
};

enum TTxtFile {
	TXT_FILE_RCONFIG,
	TXT_FILE_NETWORK,
	TXT_FILE_ARTNET,
	TXT_FILE_E131,
	TXT_FILE_OSC,
	TXT_FILE_PARAMS,
	TXT_FILE_DEVICES,
	TXT_FILE_LTC,
	TXT_FILE_TCNET,
	TXT_FILE_OSC_CLIENT,
	TXT_FILE_DISPLAY_UDF,
	TXT_FILE_LTCDISPLAY,
	TXT_FILE_MONITOR,
#if defined(STEPPER)
	TXT_FILE_SPARKFUN,
	TXT_FILE_MOTOR0,
	TXT_FILE_MOTOR1,
	TXT_FILE_MOTOR2,
	TXT_FILE_MOTOR3,
	TXT_FILE_MOTOR4,
	TXT_FILE_MOTOR5,
	TXT_FILE_MOTOR6,
	TXT_FILE_MOTOR7,
#endif
	TXT_FILE_SHOW,
	TXT_FILE_SERIAL,
	TXT_FILE_GPS,
	TXT_FILE_RGBPANEL,
	TXT_FILE_LAST
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
	char aDisplayName[REMOTE_CONFIG_DISPLAY_NAME_LENGTH];
}__attribute__((packed));

class RemoteConfig {
public:
	RemoteConfig(TRemoteConfig tRemoteConfig, TRemoteConfigMode tRemoteConfigMode, uint8_t nOutputs = 0);
	~RemoteConfig();

	void SetDisable(bool bDisable = true);
	bool GetDisable() {
		return m_bDisable;
	}

	void SetDisableWrite(bool bDisableWrite = true) {
		m_bDisableWrite = bDisableWrite;
	}
	bool GetDisableWrite() {
		return m_bDisableWrite;
	}

	void SetEnableReboot(bool bEnableReboot = true) {
		m_bEnableReboot = bEnableReboot;
	}
	bool GetEnableReboot() {
		return m_bEnableReboot;
	}

	void SetEnableUptime(bool bEnableUptime = true) {
		m_bEnableUptime = bEnableUptime;
	}
	bool GetEnableUptime() {
		return m_bEnableUptime;
	}

	void SetDisplayName(const char *pDisplayName);

	bool IsReboot() {
		return m_bIsReboot;
	}

	void Reboot() {
		HandleReboot();
	}

	void Run();

	void TftpExit();

	uint32_t HandleGet(void *pBuffer = nullptr, uint32_t nBufferLength = 0);
	void HandleTxtFile(void *pBuffer = nullptr, uint32_t nBufferLength = 0);

	static uint32_t GetIndex(const void *p, uint32_t &nLength);
	static TStore GetStore(TTxtFile tTxtFile);
	static RemoteConfig *Get() {
		return s_pThis;
	}

private:
	void HandleReboot();

	void HandleList();
	void HandleUptime();
	void HandleVersion();

	void HandleGetRconfigTxt(uint32_t& nSize);
	void HandleGetNetworkTxt(uint32_t& nSize);

#if defined (ARTNET_NODE)
	void HandleGetArtnetTxt(uint32_t& nSize);
#endif

#if defined (E131_BRIDGE)
	void HandleGetE131Txt(uint32_t& nSize);
#endif

#if defined (OSC_SERVER)
	void HandleGetOscTxt(uint32_t& nSize);
#endif

#if defined (DMXSEND)
	void HandleGetParamsTxt(uint32_t& nSize);
#endif

#if defined (PIXEL)
	void HandleGetDevicesTxt(uint32_t& nSize);
#endif

#if defined (LTC_READER)
	void HandleGetLtcTxt(uint32_t& nSize);
	void HandleGetLtcDisplayTxt(uint32_t& nSize);
	void HandleGetTCNetTxt(uint32_t& nSize);
	void HandleGetGpsTxt(uint32_t& nSize);
#endif

#if defined (DMX_MONITOR)
	void HandleGetMonTxt(uint32_t& nSize);
#endif

#if defined (OSC_CLIENT)
	void HandleGetOscClntTxt(uint32_t& nSize);
#endif

#if defined(DISPLAY_UDF)
	void HandleGetDisplayTxt(uint32_t& nSize);
#endif

#if defined(STEPPER)
	void HandleGetSparkFunTxt(uint32_t& nSize);
	void HandleGetMotorTxt(uint32_t nMotorIndex, uint32_t& nSize);
#endif

#if defined(SHOWFILE)
	void HandleGetShowTxt(uint32_t& nSize);
#endif

#if defined (DMXSERIAL)
	void HandleGetSerialTxt(uint32_t& nSize);
#endif

#if defined (RGB_PANEL)
	void HandleGetRgbPanelTxt(uint32_t& nSize);
#endif

#if defined (RDM_RESPONDER)
#endif

	void HandleTxtFileRconfig();
	void HandleTxtFileNetwork();

#if defined (ARTNET_NODE)
	void HandleTxtFileArtnet();
#endif

#if defined (E131_BRIDGE)
	void HandleTxtFileE131();
#endif

#if defined (OSC_SERVER)
	void HandleTxtFileOsc();
#endif

#if defined (DMXSEND)
	void HandleTxtFileParams();
#endif

#if defined (PIXEL)
	void HandleTxtFileDevices();
#endif

#if defined (LTC_READER)
	void HandleTxtFileLtc();
	void HandleTxtFileLtcDisplay();
	void HandleTxtFileTCNet();
	void HandleTxtFileGps();
#endif

#if defined (DMX_MONITOR)
	void HandleTxtFileMon();
#endif

#if defined (OSC_CLIENT)
	void HandleTxtFileOscClient();
#endif

#if defined(DISPLAY_UDF)
	void HandleTxtFileDisplay();
#endif

#if defined(STEPPER)
	void HandleTxtFileSparkFun();
	void HandleTxtFileMotor(uint32_t nMotorIndex);
#endif

#if defined(SHOWFILE)
	void HandleTxtFileShow();
#endif

#if defined (DMXSERIAL)
	void HandleTxtFileSerial();
#endif

#if defined (RGB_PANEL)
	void HandleTxtFileRgbPanel();
#endif

#if defined (RDM_RESPONDER)
#endif

	void HandleDisplaySet();
	void HandleDisplayGet();

	void HandleStoreSet();
	void HandleStoreGet();

	void HandleTftpSet();
	void HandleTftpGet();

private:
	TRemoteConfig m_tRemoteConfig;
	TRemoteConfigMode m_tRemoteConfigMode;
	uint8_t m_nOutputs;
	bool m_bDisable{false};
	bool m_bDisableWrite{false};
	bool m_bEnableReboot{false};
	bool m_bEnableUptime{false};
	bool m_bEnableTFTP{false};
	TFTPFileServer *m_pTFTPFileServer{nullptr};
	uint8_t *m_pTFTPBuffer{nullptr};
	char m_aId[REMOTE_CONFIG_ID_LENGTH];
	int32_t m_nIdLength{0};
	struct TRemoteConfigListBin m_tRemoteConfigListBin;
	int32_t m_nHandle{-1};
	char *m_pUdpBuffer{nullptr};
	uint32_t m_nIPAddressFrom{0};
	uint16_t m_nBytesReceived{0};
	TRemoteConfigHandleMode m_tRemoteConfigHandleMode{REMOTE_CONFIG_HANDLE_MODE_TXT};
	uint8_t *m_pStoreBuffer{nullptr};
	bool m_bIsReboot{false};

	static RemoteConfig *s_pThis;
};

#endif /* REMOTECONFIG_H_ */
