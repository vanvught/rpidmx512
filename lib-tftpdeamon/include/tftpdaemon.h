/**
 * @file tftpdaemon.h
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

#ifndef TFTPDAEMON_H_
#define TFTPDAEMON_H_

#include <stdbool.h>
#include <stdint.h>

enum TTFTPMode {
	TFTP_MODE_BINARY,
	TFTP_MODE_ASCII
};

class TFTPDaemon {
public:
	TFTPDaemon(void);
	virtual ~TFTPDaemon(void);

	bool Run(void);

	virtual bool FileOpen(const char *pFileName, TTFTPMode tMode)=0;
	virtual bool FileCreate(const char *pFileName, TTFTPMode tMode)=0;
	virtual bool FileClose(void)=0;
	virtual int FileRead(void *pBuffer, unsigned nCount, unsigned nBlockNumber)=0;
	virtual int FileWrite(const void *pBuffer, unsigned nCount, unsigned nBlockNumber)=0;

private:
	void HandleRequest(void);
	void HandleRecvAck(void);
	void HandleRecvData(void);
	void SendError (uint16_t usErrorCode, const char *pErrorMessage);
	void DoRead(void);
	void DoWriteAck(void);

private:
	int m_nState;
	int m_nIdx;
	uint8_t m_Buffer[528];
	uint32_t m_nFromIp;
	uint16_t m_nFromPort;
	uint16_t m_nLength;
	uint16_t m_nBlockNumber;
	uint16_t m_nDataLength;
	uint16_t m_nPacketLength;
	bool m_bIsLastBlock;
};

#endif /* TFTPDAEMON_H_ */
