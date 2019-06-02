/**
 * @file tftpdaemon.cpp
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

/*
 * https://tools.ietf.org/html/rfc1350
 */

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "tftpdaemon.h"

#include "network.h"
#include "debug.h"

enum TState {
	STATE_INIT,
	STATE_WAITING_RQ,
	STATE_RRQ_SEND_PACKET,
	STATE_RRQ_RECV_ACK,
	STATE_WRQ_SEND_ACK,
	STATE_WRQ_RECV_PACKET
};

enum TOpCode {
	OP_CODE_RRQ = 1,			///< Read request (RRQ)
	OP_CODE_WRQ = 2,			///< Write request (WRQ)
	OP_CODE_DATA = 3,			///< Data (DATA)
	OP_CODE_ACK = 4,			///< Acknowledgment (ACK)
	OP_CODE_ERROR = 5			///< Error (ERROR)
};

enum TErrorCode {
	ERROR_CODE_OTHER = 0,		///< Not defined, see error message (if any).
	ERROR_CODE_NO_FILE = 1,		///< File not found.
	ERROR_CODE_ACCESS = 2,		///< Access violation.
	ERROR_CODE_DISK_FULL = 3,	///< Disk full or allocation exceeded.
	ERROR_CODE_ILL_OPER = 4,	///< Illegal TFTP operation.
	ERROR_CODE_INV_ID = 5,		///< Unknown transfer ID.
	ERROR_CODE_EXISTS = 6,		///< File already exists.
	ERROR_CODE_INV_USER = 7		///< No such user.
};

#define TFTP_UDP_PORT			69
#define MAX_FILENAME_LEN		128
#define MAX_MODE_LEN			16
#define MIN_FILENAME_MODE_LEN	(1+1+1+1)
#define MAX_FILENAME_MODE_LEN	(MAX_FILENAME_LEN+1+MAX_MODE_LEN+1)
#define MAX_DATA_LEN			512
#define MAX_ERRMSG_LEN			128

#if  !defined (PACKED)
 #define PACKED __attribute__((packed))
#endif

struct TTFTPReqPacket {
	uint16_t OpCode;
	char FileNameMode[MAX_FILENAME_MODE_LEN];
} PACKED;

struct TTFTPAckPacket {
	uint16_t OpCode;
	uint16_t BlockNumber;
} PACKED;

struct TTFTPErrorPacket {
	uint16_t OpCode;
	uint16_t ErrorCode;
	char ErrMsg[MAX_ERRMSG_LEN];
} PACKED;

struct TTFTPDataPacket {
	uint16_t OpCode;
	uint16_t BlockNumber;
	uint8_t Data[MAX_DATA_LEN];
} PACKED;

TFTPDaemon::TFTPDaemon(void):
		m_nState(STATE_INIT),
		m_nIdx(-1),
		m_nFromIp(0),
		m_nFromPort(0),
		m_nLength(0),
		m_nBlockNumber(0),
		m_nDataLength(0),
		m_nPacketLength(0),
		m_bIsLastBlock(false)
{
	assert(Network::Get() != 0);
	memset(m_Buffer, 0, sizeof(m_Buffer));
}

TFTPDaemon::~TFTPDaemon(void) {
	Network::Get()->End(TFTP_UDP_PORT);
}

bool TFTPDaemon::Run(void) {

	if (m_nState == STATE_INIT) {
		if (m_nFromPort != 0) {
			Network::Get()->End(m_nFromPort);
			m_nIdx = -1;
			m_nFromPort = 0;
		}

		m_nIdx = Network::Get()->Begin(TFTP_UDP_PORT);
		DEBUG_PRINTF("m_nIdx=%d", m_nIdx);

		m_nBlockNumber = 0;
		m_nState = STATE_WAITING_RQ;
		m_bIsLastBlock = false;
		memset(&m_Buffer, 0, sizeof(struct TTFTPReqPacket));
	} else {
		m_nLength = Network::Get()->RecvFrom(m_nIdx, (uint8_t *) &m_Buffer, sizeof(m_Buffer), &m_nFromIp, &m_nFromPort);

		switch (m_nState) {
		case STATE_WAITING_RQ:
			if (m_nLength > MIN_FILENAME_MODE_LEN) {
				HandleRequest();
			}
			break;
		case STATE_RRQ_SEND_PACKET:
			DoRead();
			break;
		case STATE_RRQ_RECV_ACK:
			if (m_nLength == sizeof(struct TTFTPAckPacket)) {
				HandleRecvAck();
			}
			break;
		case STATE_WRQ_RECV_PACKET:
			if (m_nLength <= sizeof(struct TTFTPDataPacket)) {
				HandleRecvData();
			}
			break;
		default:
			assert(0);
			break;
		}

	}

	return true;
}

void TFTPDaemon::HandleRequest(void) {
	struct TTFTPReqPacket *packet = (struct TTFTPReqPacket *) &m_Buffer;

	const uint16_t nOpCode = __builtin_bswap16(packet->OpCode);

	if ((nOpCode != OP_CODE_RRQ && nOpCode != OP_CODE_WRQ)) {
		SendError(ERROR_CODE_ILL_OPER, "Invalid operation");
		return;
	}

	const char *pFileName = packet->FileNameMode;
	const size_t nNameLen = strlen(pFileName);

	if (!(1 <= nNameLen && nNameLen <= MAX_FILENAME_LEN)) {
		SendError(ERROR_CODE_OTHER, "Invalid file name");
		return;
	}

	const char *pMode = &packet->FileNameMode[nNameLen + 1];
	TTFTPMode tMode;

	if (strncmp(pMode, "octet", 5) == 0) {
		tMode = TFTP_MODE_BINARY;
	} else if (strncmp(pMode, "netascii", 8) == 0) {
		tMode = TFTP_MODE_ASCII;
	} else {
		SendError(ERROR_CODE_ILL_OPER, "Invalid operation");
		return;
	}

	DEBUG_PRINTF("Incoming %s request from " IPSTR " %s %s", nOpCode == OP_CODE_RRQ ? "read" : "write", IP2STR(m_nFromIp), pFileName, pMode);

	switch (nOpCode) {
		case OP_CODE_RRQ:
			if(!FileOpen(pFileName, tMode)) {
				SendError(ERROR_CODE_NO_FILE, "File not found");
				m_nState = STATE_WAITING_RQ;
			} else {
				Network::Get()->End(TFTP_UDP_PORT);
				m_nIdx = Network::Get()->Begin(m_nFromPort);
				m_nState = STATE_RRQ_SEND_PACKET;
				DoRead();
			}
			break;
		case OP_CODE_WRQ:
			if(!FileCreate(pFileName, tMode)) {
				SendError(ERROR_CODE_ACCESS, "Access violation");
				m_nState = STATE_WAITING_RQ;
			} else {
				Network::Get()->End(TFTP_UDP_PORT);
				m_nIdx = Network::Get()->Begin(m_nFromPort);
				m_nState = STATE_WRQ_SEND_ACK;
				DoWriteAck();
			}
			break;
		default:
			assert(0);
			break;
	}
}

void TFTPDaemon::SendError (uint16_t nErrorCode, const char *pErrorMessage) {
	TTFTPErrorPacket ErrorPacket;

	ErrorPacket.OpCode = __builtin_bswap16 (OP_CODE_ERROR);
	ErrorPacket.ErrorCode = __builtin_bswap16 (nErrorCode);
	strncpy(ErrorPacket.ErrMsg, pErrorMessage, sizeof(ErrorPacket.ErrMsg));

	Network::Get()->SendTo(m_nIdx, (uint8_t *)&ErrorPacket, sizeof ErrorPacket, m_nFromIp, m_nFromPort);
}

void TFTPDaemon::DoRead(void) {
	struct TTFTPDataPacket *packet = (struct TTFTPDataPacket *) &m_Buffer;

	if (m_nState == STATE_RRQ_SEND_PACKET) {
		m_nDataLength = FileRead(packet->Data, MAX_DATA_LEN, ++m_nBlockNumber);

		packet->OpCode = __builtin_bswap16(OP_CODE_DATA);
		packet->BlockNumber = __builtin_bswap16(m_nBlockNumber);

		m_nPacketLength = sizeof packet->OpCode + sizeof packet->BlockNumber + m_nDataLength;
		m_bIsLastBlock = m_nDataLength < MAX_DATA_LEN;

		if (m_bIsLastBlock) {
			FileClose();
		}

		DEBUG_PRINTF("m_nDataLength=%d, m_nPacketLength=%d, m_bIsLastBlock=%d", m_nDataLength, m_nPacketLength, m_bIsLastBlock);
	}

	DEBUG_PRINTF("Sending to " IPSTR ":%d", IP2STR(m_nFromIp), m_nFromPort);

	Network::Get()->SendTo(m_nIdx, (uint8_t *) &m_Buffer, m_nPacketLength, m_nFromIp, m_nFromPort);

	m_nState = STATE_RRQ_RECV_ACK;
}

void TFTPDaemon::HandleRecvAck(void) {
	struct TTFTPAckPacket *packet = (struct TTFTPAckPacket *) &m_Buffer;

	if (packet->OpCode == __builtin_bswap16(OP_CODE_ACK)) {

		DEBUG_PRINTF("Incoming from " IPSTR ", BlockNumber=%d, m_nBlockNumber=%d", IP2STR(m_nFromIp), __builtin_bswap16(packet->BlockNumber), m_nBlockNumber	);

		if (packet->BlockNumber == __builtin_bswap16(m_nBlockNumber)) {
			m_nState = m_bIsLastBlock ? STATE_INIT : STATE_RRQ_SEND_PACKET;
		}
	}
}

void TFTPDaemon::DoWriteAck(void) {
	struct TTFTPAckPacket *packet = (struct TTFTPAckPacket *) &m_Buffer;

	packet->OpCode = __builtin_bswap16(OP_CODE_ACK);
	packet->BlockNumber =  __builtin_bswap16(m_nBlockNumber);
	m_nState = m_bIsLastBlock ? STATE_INIT : STATE_WRQ_RECV_PACKET;

	DEBUG_PRINTF("Sending to " IPSTR ":%d, m_nState=%d", IP2STR(m_nFromIp), m_nFromPort, m_nState);

	Network::Get()->SendTo(m_nIdx, (uint8_t *) &m_Buffer, sizeof(struct TTFTPAckPacket), m_nFromIp, m_nFromPort);
}

void TFTPDaemon::HandleRecvData(void) {
	struct TTFTPDataPacket *packet = (struct TTFTPDataPacket *) &m_Buffer;

	if (packet->OpCode == __builtin_bswap16(OP_CODE_DATA)) {
		m_nDataLength = m_nLength - 4;
		m_nBlockNumber = __builtin_bswap16(packet->BlockNumber);

		DEBUG_PRINTF("Incoming from " IPSTR ", m_nLength=%d, m_nBlockNumber=%d, m_nDataLength=%d", IP2STR(m_nFromIp), m_nLength, m_nBlockNumber,m_nDataLength);

		if (m_nDataLength == FileWrite(packet->Data, m_nDataLength, m_nBlockNumber)) {

			if (m_nDataLength < MAX_DATA_LEN) {
				m_bIsLastBlock = true;
				FileClose();
			}

			DoWriteAck();
		} else {
			SendError(ERROR_CODE_DISK_FULL, "Write failed");
			m_nState = STATE_INIT;
		}
	}
}
