/**
 * @file dmxserial.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "dmxserialchanneldata.h"
#include "dmxserialtftp.h"

#include "lightset.h"
#include "serial.h"

enum TDmxSerialDefaults {
	DMXSERIAL_DEFAULT_TYPE = SERIAL_TYPE_UART,
	DMXSERIAL_DEFAULT_UART_BAUD = 115200,
	DMXSERIAL_DEFAULT_UART_BITS = 8,
	DMXSERIAL_DEFAULT_UART_PARITY = SERIAL_UART_PARITY_NONE,
	DMXSERIAL_DEFAULT_UART_STOPBITS = 1,
	DMXSERIAL_DEFAULT_SPI_SPEED_HZ = 1000000, ///< 1 MHz
	DMXSERIAL_DEFAULT_SPI_MODE = 0,
	DMXSERIAL_DEFAULT_I2C_ADDRESS = 0x30,
	DMXSERIAL_DEFAULT_I2C_SPEED_MODE = SERIAL_I2C_SPEED_MODE_FAST
};

#define DMXSERIAL_FILE_PREFIX	"chl"
#define DMXSERIAL_FILE_SUFFIX	".txt"

enum TDmxSerialFiles {
	DMXSERIAL_FILE_NAME_LENGTH = sizeof(DMXSERIAL_FILE_PREFIX "NNN" DMXSERIAL_FILE_SUFFIX) - 1,
	DMXSERIAL_FILE_MIN_NUMBER = 1,
	DMXSERIAL_FILE_MAX_NUMBER = DMX_UNIVERSE_SIZE
};

class DmxSerial: public LightSet {
public:
	DmxSerial(void);
	~DmxSerial(void);

	void Init(void);

	void Start(uint8_t nPort);
	void Stop(uint8_t nPort);

	void SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLength);

	void Run(void);

	void Print(void);

	const char *GetSerialType(void) {
		return Serial::GetType(m_Serial.GetType());
	}

	uint32_t GetFilesCount(void) {
		return m_nFilesCount;
	}

	void EnableTFTP(bool bEnableTFTP);

	bool DeleteFile(uint16_t nFileNumber);
	bool DeleteFile(const char *pFileNumber);

	static bool FileNameCopyTo(char *pFileName, uint32_t nLength, uint16_t nFileNumber);
	static bool CheckFileName(const char *pFileName, uint16_t &nFileNumber);

	static DmxSerial *Get(void) {
		return s_pThis;
	}

private:
	void ScanDirectory(void);
	void HandleUdp(void);

private:
	Serial m_Serial;
	uint32_t m_nFilesCount;
	int16_t m_aFileIndex[DMXSERIAL_FILE_MAX_NUMBER];
	DmxSerialChannelData *m_pDmxSerialChannelData[DMXSERIAL_FILE_MAX_NUMBER];
	uint16_t m_nDmxLastSlot;
	uint8_t m_DmxData[DMX_UNIVERSE_SIZE];
	bool m_bEnableTFTP;
	DmxSerialTFTP *m_pDmxSerialTFTP;
	int m_nHandle;

	static DmxSerial *s_pThis;
};

#endif /* DMXSERIAL_H_ */
