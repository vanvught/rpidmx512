/**
 * @file tftpdaemon.cpp
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

/*
 * https://tools.ietf.org/html/rfc1350
 */

#include <stdint.h>
#include <string.h>
#include <cassert>

#include "tftpdaemon.h"

#include "network.h"

#include "debug.h"

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

namespace min {
	static constexpr auto FILENAME_MODE_LEN = (1 + 1 + 1 + 1);
}

namespace max {
	static constexpr auto FILENAME_LEN = 128;
	static constexpr auto MODE_LEN = 16;
	static constexpr auto FILENAME_MODE_LEN = (FILENAME_LEN + 1 + MODE_LEN + 1);
	static constexpr auto DATA_LEN = 512;
	static constexpr auto ERRMSG_LEN = 128;
}

#if  !defined (PACKED)
 #define PACKED __attribute__((packed))
#endif

struct TTFTPReqPacket {
	uint16_t OpCode;
	char FileNameMode[max::FILENAME_MODE_LEN];
} PACKED;

struct TTFTPAckPacket {
	uint16_t OpCode;
	uint16_t BlockNumber;
} PACKED;

struct TTFTPErrorPacket {
	uint16_t OpCode;
	uint16_t ErrorCode;
	char ErrMsg[max::ERRMSG_LEN];
} PACKED;

struct TTFTPDataPacket {
	uint16_t OpCode;
	uint16_t BlockNumber;
	uint8_t Data[max::DATA_LEN];
} PACKED;

TFTPDaemon *TFTPDaemon::s_pThis = nullptr;

TFTPDaemon::TFTPDaemon()
		
{
	DEBUG_ENTRY
	DEBUG_PRINTF("s_pThis=%p", s_pThis);

	if (s_pThis != nullptr) {
		s_pThis->Exit();
	}

	s_pThis = this;

	DEBUG_PRINTF("s_pThis=%p", s_pThis);

	assert(Network::Get() != nullptr);
	memset(m_Buffer, 0, sizeof(m_Buffer));

	DEBUG_EXIT
}

TFTPDaemon::~TFTPDaemon() {
	DEBUG_ENTRY
	DEBUG_PRINTF("s_pThis=%p", s_pThis);

	Network::Get()->End(TFTP_UDP_PORT);

	s_pThis = nullptr;

	DEBUG_EXIT
}

bool TFTPDaemon::Run() {

	if (m_nState == TFTPState::INIT) {
		if (m_nFromPort != 0) {
			Network::Get()->End(m_nFromPort);
			m_nIdx = -1;
			m_nFromPort = 0;
		}

		m_nIdx = Network::Get()->Begin(TFTP_UDP_PORT);
		DEBUG_PRINTF("m_nIdx=%d", m_nIdx);

		m_nBlockNumber = 0;
		m_nState = TFTPState::WAITING_RQ;
		m_bIsLastBlock = false;
		memset(&m_Buffer, 0, sizeof(struct TTFTPReqPacket));
	} else {
		m_nLength = Network::Get()->RecvFrom(m_nIdx, &m_Buffer, sizeof(m_Buffer), &m_nFromIp, &m_nFromPort);

		switch (m_nState) {
		case TFTPState::WAITING_RQ:
			if (m_nLength > min::FILENAME_MODE_LEN) {
				HandleRequest();
			}
			break;
		case TFTPState::RRQ_SEND_PACKET:
			DoRead();
			break;
		case TFTPState::RRQ_RECV_ACK:
			if (m_nLength == sizeof(struct TTFTPAckPacket)) {
				HandleRecvAck();
			}
			break;
		case TFTPState::WRQ_RECV_PACKET:
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

void TFTPDaemon::HandleRequest() {
	auto *packet = reinterpret_cast<struct TTFTPReqPacket *>(&m_Buffer);

	const uint16_t nOpCode = __builtin_bswap16(packet->OpCode);

	if ((nOpCode != OP_CODE_RRQ && nOpCode != OP_CODE_WRQ)) {
		SendError(ERROR_CODE_ILL_OPER, "Invalid operation");
		return;
	}

	const char *pFileName = packet->FileNameMode;
	const size_t nNameLen = strlen(pFileName);

	if (!(1 <= nNameLen && nNameLen <= max::FILENAME_LEN)) {
		SendError(ERROR_CODE_OTHER, "Invalid file name");
		return;
	}

	const char *pMode = &packet->FileNameMode[nNameLen + 1];
	TFTPMode tMode;

	if (strncmp(pMode, "octet", 5) == 0) {
		tMode = TFTPMode::BINARY;
	} else if (strncmp(pMode, "netascii", 8) == 0) {
		tMode = TFTPMode::ASCII;
	} else {
		SendError(ERROR_CODE_ILL_OPER, "Invalid operation");
		return;
	}

	DEBUG_PRINTF("Incoming %s request from " IPSTR " %s %s", nOpCode == OP_CODE_RRQ ? "read" : "write", IP2STR(m_nFromIp), pFileName, pMode);

	switch (nOpCode) {
		case OP_CODE_RRQ:
			if(!FileOpen(pFileName, tMode)) {
				SendError(ERROR_CODE_NO_FILE, "File not found");
				m_nState = TFTPState::WAITING_RQ;
			} else {
				Network::Get()->End(TFTP_UDP_PORT);
				m_nIdx = Network::Get()->Begin(m_nFromPort);
				m_nState = TFTPState::RRQ_SEND_PACKET;
				DoRead();
			}
			break;
		case OP_CODE_WRQ:
			if(!FileCreate(pFileName, tMode)) {
				SendError(ERROR_CODE_ACCESS, "Access violation");
				m_nState = TFTPState::WAITING_RQ;
			} else {
				Network::Get()->End(TFTP_UDP_PORT);
				m_nIdx = Network::Get()->Begin(m_nFromPort);
				m_nState = TFTPState::WRQ_SEND_ACK;
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
	strncpy(ErrorPacket.ErrMsg, pErrorMessage, sizeof(ErrorPacket.ErrMsg) - 1);

	Network::Get()->SendTo(m_nIdx, &ErrorPacket, sizeof ErrorPacket, m_nFromIp, m_nFromPort);
}

void TFTPDaemon::DoRead() {
	auto *pDataPacket = reinterpret_cast<struct TTFTPDataPacket*>(&m_Buffer);

	if (m_nState == TFTPState::RRQ_SEND_PACKET) {
		m_nDataLength = FileRead(pDataPacket->Data, max::DATA_LEN, ++m_nBlockNumber);

		pDataPacket->OpCode = __builtin_bswap16(OP_CODE_DATA);
		pDataPacket->BlockNumber = __builtin_bswap16(m_nBlockNumber);

		m_nPacketLength = sizeof pDataPacket->OpCode + sizeof pDataPacket->BlockNumber + m_nDataLength;
		m_bIsLastBlock = m_nDataLength < max::DATA_LEN;

		if (m_bIsLastBlock) {
			FileClose();
		}

		DEBUG_PRINTF("m_nDataLength=%d, m_nPacketLength=%d, m_bIsLastBlock=%d", m_nDataLength, m_nPacketLength, m_bIsLastBlock);
	}

	DEBUG_PRINTF("Sending to " IPSTR ":%d", IP2STR(m_nFromIp), m_nFromPort);

	Network::Get()->SendTo(m_nIdx, &m_Buffer, m_nPacketLength, m_nFromIp, m_nFromPort);

	m_nState = TFTPState::RRQ_RECV_ACK;
}

void TFTPDaemon::HandleRecvAck() {
	auto *pAckPacket = reinterpret_cast<struct TTFTPAckPacket*>(&m_Buffer);

	if (pAckPacket->OpCode == __builtin_bswap16(OP_CODE_ACK)) {

		DEBUG_PRINTF("Incoming from " IPSTR ", BlockNumber=%d, m_nBlockNumber=%d", IP2STR(m_nFromIp), __builtin_bswap16(pAckPacket->BlockNumber), m_nBlockNumber	);

		if (pAckPacket->BlockNumber == __builtin_bswap16(m_nBlockNumber)) {
			m_nState = m_bIsLastBlock ? TFTPState::INIT : TFTPState::RRQ_SEND_PACKET;
		}
	}
}

void TFTPDaemon::DoWriteAck() {
	auto *pAckPacket = reinterpret_cast<struct TTFTPAckPacket*>(&m_Buffer);

	pAckPacket->OpCode = __builtin_bswap16(OP_CODE_ACK);
	pAckPacket->BlockNumber =  __builtin_bswap16(m_nBlockNumber);
	m_nState = m_bIsLastBlock ? TFTPState::INIT : TFTPState::WRQ_RECV_PACKET;

	DEBUG_PRINTF("Sending to " IPSTR ":%d, m_nState=%d", IP2STR(m_nFromIp), m_nFromPort, m_nState);

	Network::Get()->SendTo(m_nIdx, &m_Buffer, sizeof(struct TTFTPAckPacket), m_nFromIp, m_nFromPort);
}

void TFTPDaemon::HandleRecvData() {
	auto *pDataPacket = reinterpret_cast<struct TTFTPDataPacket*>(&m_Buffer);

	if (pDataPacket->OpCode == __builtin_bswap16(OP_CODE_DATA)) {
		m_nDataLength = m_nLength - 4;
		m_nBlockNumber = __builtin_bswap16(pDataPacket->BlockNumber);

		DEBUG_PRINTF("Incoming from " IPSTR ", m_nLength=%d, m_nBlockNumber=%d, m_nDataLength=%d", IP2STR(m_nFromIp), m_nLength, m_nBlockNumber,m_nDataLength);

		if (m_nDataLength == FileWrite(pDataPacket->Data, m_nDataLength, m_nBlockNumber)) {

			if (m_nDataLength < max::DATA_LEN) {
				m_bIsLastBlock = true;
				FileClose();
			}

			DoWriteAck();
		} else {
			SendError(ERROR_CODE_DISK_FULL, "Write failed");
			m_nState = TFTPState::INIT;
		}
	}
}
