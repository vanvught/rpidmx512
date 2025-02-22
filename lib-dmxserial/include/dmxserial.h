/**
 * @file dmxserial.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXSERIAL_H_
#define DMXSERIAL_H_

#include <cstdint>

#include "dmxserialchanneldata.h"
#include "dmxserialtftp.h"
#include "../src/serial/serial.h"

#include "dmxnode.h"

namespace DmxSerialDefaults {
	static constexpr auto TYPE = serial::type::UART;
	static constexpr auto UART_BAUD = 115200;
	static constexpr auto UART_BITS = hal::uart::BITS_8;
	static constexpr auto UART_PARITY = hal::uart::PARITY_NONE;
	static constexpr auto UART_STOPBITS = hal::uart::STOP_1BIT;
	static constexpr auto SPI_SPEED_HZ = 1000000; ///< 1 MHz
	static constexpr auto SPI_MODE = 0;
	static constexpr auto I2C_ADDRESS = 0x30;
	static constexpr auto I2C_SPEED_MODE = serial::i2c::FAST;
}

#define DMXSERIAL_FILE_PREFIX	"chl"
#define DMXSERIAL_FILE_SUFFIX	".txt"

namespace DmxSerialFile {
	static constexpr auto NAME_LENGTH = sizeof(DMXSERIAL_FILE_PREFIX "NNN" DMXSERIAL_FILE_SUFFIX) - 1;
	static constexpr auto MIN_NUMBER = 1;
	static constexpr auto MAX_NUMBER = dmxnode::UNIVERSE_SIZE;
}

class DmxSerial {
public:
	DmxSerial();
	~DmxSerial();

	void Init();

	void Start(const uint32_t nPortIndex);
	void Stop(const uint32_t nPortIndex);

	void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true);
	void Sync(const uint32_t nPortIndex);
	void Sync();

	uint32_t GetUserData() { return 0; }		///< Art-Net ArtPollReply
	uint32_t GetRefreshRate() { return 0; }		///< Art-Net ArtPollReply

	void Blackout([[maybe_unused]] bool bBlackout) {}
	void FullOn() {}

	bool SetDmxStartAddress([[maybe_unused]] const uint16_t nDmxStartAddress) {
		return false;
	}

	uint16_t GetDmxStartAddress() {
		return dmxnode::START_ADDRESS_DEFAULT;
	}

	uint16_t GetDmxFootprint() {
		return dmxnode::UNIVERSE_SIZE;
	}

	bool GetSlotInfo([[maybe_unused]] const uint16_t nSlotOffset, dmxnode::SlotInfo &slotInfo) {
		slotInfo.nType = 0x00; // ST_PRIMARY
		slotInfo.nCategory = 0x0001; // SD_INTENSITY
		return true;
	}

	void Print();

	const char *GetSerialType() {
		return Serial::GetType(m_Serial.GetType());
	}

	uint32_t GetFilesCount() {
		return m_nFilesCount;
	}

	void EnableTFTP(bool bEnableTFTP);

	bool DeleteFile(int32_t nFileNumber);
	bool DeleteFile(const char *pFileNumber);

	void Input(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort);

	static bool FileNameCopyTo(char *pFileName, uint32_t nLength, int32_t nFileNumber);
	static bool CheckFileName(const char *pFileName, int32_t &nFileNumber);

	static DmxSerial *Get() {
		return s_pThis;
	}

private:
	void ScanDirectory();
	void Update(const uint8_t *pData, const uint32_t nLength);

	void static StaticCallbackFunction(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		s_pThis->Input(pBuffer, nSize, nFromIp, nFromPort);
	}

private:
	Serial m_Serial;
	uint32_t m_nFilesCount { 0 };
	int32_t m_aFileIndex[DmxSerialFile::MAX_NUMBER];
	int32_t m_nHandle { -1 };
	DmxSerialChannelData *m_pDmxSerialChannelData[DmxSerialFile::MAX_NUMBER];
	uint16_t m_nDmxLastSlot { dmxnode::UNIVERSE_SIZE };
	uint8_t m_DmxData[dmxnode::UNIVERSE_SIZE];
	struct SyncData {
		uint8_t data[dmxnode::UNIVERSE_SIZE];
		uint32_t nLength;
	};
	SyncData m_SyncData;
	bool m_bEnableTFTP { false };
	DmxSerialTFTP *m_pDmxSerialTFTP { nullptr };

	static inline DmxSerial *s_pThis;
};

#endif /* DMXSERIAL_H_ */
