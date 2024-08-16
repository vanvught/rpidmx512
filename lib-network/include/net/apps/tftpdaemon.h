/**
 * @file tftpdaemon.h
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

#ifndef NET_APPS_TFTPDAEMON_H_
#define NET_APPS_TFTPDAEMON_H_

#include <cstdint>
#include <cstddef>

namespace tftp {
enum class Mode {
	BINARY, ASCII
};
}  // namespace tftp

class TFTPDaemon {
public:
	TFTPDaemon();
	virtual ~TFTPDaemon();

	void Run();

	virtual bool FileOpen(const char *pFileName, tftp::Mode mode)=0;
	virtual bool FileCreate(const char *pFileName, tftp::Mode mode)=0;
	virtual bool FileClose()=0;
	virtual size_t FileRead(void *pBuffer, size_t nCount, unsigned nBlockNumber)=0;
	virtual size_t FileWrite(const void *pBuffer, size_t nCount, unsigned nBlockNumber)=0;

	virtual void Exit()=0;

private:
	void HandleRequest();
	void HandleRecvAck();
	void HandleRecvData();
	void SendError (const uint16_t nsErrorCode, const char *pErrorMessage);
	void DoRead();
	void DoWriteAck();

private:
	enum class TFTPState {
		INIT,
		WAITING_RQ,
		RRQ_SEND_PACKET,
		RRQ_RECV_ACK,
		WRQ_SEND_ACK,
		WRQ_RECV_PACKET
	};
	TFTPState m_nState { TFTPState::INIT };
	int m_nIdx { -1 };
	uint8_t *m_pBuffer { nullptr };
	uint32_t m_nFromIp { 0 };
	uint32_t m_nLength { 0 };
	uint32_t m_nDataLength { 0 };
	uint32_t m_nPacketLength { 0 };
	uint16_t m_nFromPort { 0 };
	uint16_t m_nBlockNumber { 0 };
	bool m_bIsLastBlock { false };

	static TFTPDaemon* Get() {
		return s_pThis;
	}

private:
	static TFTPDaemon *s_pThis;
};

#endif /* NET_APPS_TFTPDAEMON_H_ */
