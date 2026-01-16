/**
 * @file tftpdaemon.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_NET_APPS_TFTP)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "network.h"
#include "apps/tftpdaemon.h"
#include "core/protocol/iana.h"
#include "firmware/debug/debug_debug.h"

static constexpr uint16_t kOpCodeRrq = 1;   ///< Read request (RRQ)
static constexpr uint16_t kOpCodeWrq = 2;   ///< Write request (WRQ)
static constexpr uint16_t kOpCodeData = 3;  ///< Data (DATA)
static constexpr uint16_t kOpCodeAck = 4;   ///< Acknowledgment (ACK)
static constexpr uint16_t kOpCodeError = 5; ///< Error (ERROR)

static constexpr uint16_t kErrorCodeOther = 0;    ///< Not defined, see error message (if any).
static constexpr uint16_t kErrorCodeNoFile = 1;   ///< File not found.
static constexpr uint16_t kErrorCodeAccess = 2;   ///< Access violation.
static constexpr uint16_t kErrorCodeDiskFull = 3; ///< Disk full or allocation exceeded.
static constexpr uint16_t kErrorCodeIllOper = 4;  ///< Illegal TFTP operation.
// static constexpr uint16_t ERROR_CODE_INV_ID  = 5;	///< Unknown transfer ID.
// static constexpr uint16_t ERROR_CODE_EXISTS  = 6;	///< File already exists.
// static constexpr uint16_t ERROR_CODE_INV_USER = 7;///< No such user.

namespace tftp
{
namespace min
{
static constexpr uint32_t kFilenameModeLen = (1 + 1 + 1 + 1);
}

namespace max
{
static constexpr uint32_t kFilenameLen = 128;
static constexpr uint32_t kModeLen = 16;
static constexpr uint32_t kFilenameModeLen = (kFilenameLen + 1 + kModeLen + 1);
static constexpr uint32_t kDataLen = 512;
static constexpr uint32_t kErrmsgLen = 128;
} // namespace max

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

struct ReqPacket
{
    uint16_t op_code;
    char file_name_mode[max::kFilenameModeLen];
} PACKED;

struct AckPacket
{
    uint16_t op_code;
    uint16_t block_number;
} PACKED;

struct ErrorPacket
{
    uint16_t op_code;
    uint16_t error_code;
    char err_msg[max::kErrmsgLen];
} PACKED;

struct DataPacket
{
    uint16_t op_code;
    uint16_t block_number;
    uint8_t data[max::kDataLen];
} PACKED;
} // namespace tftp

TFTPDaemon::TFTPDaemon()
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("s_this=%p", reinterpret_cast<void*>(s_this));

    if (s_this != nullptr)
    {
        s_this->Exit();
    }

    s_this = this;

    Init();

    DEBUG_PRINTF("s_this=%p", reinterpret_cast<void*>(s_this));
    DEBUG_EXIT();
}

TFTPDaemon::~TFTPDaemon()
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("s_this=%p", reinterpret_cast<void*>(s_this));

    network::udp::End(from_port_);

    s_this = nullptr;

    DEBUG_EXIT();
}

void TFTPDaemon::Init()
{
    DEBUG_ENTRY();
    assert(state_ == State::kInit);

    if (from_port_ != 0)
    {
        network::udp::End(from_port_);
        index_ = -1;
    }

    index_ = network::udp::Begin(network::iana::Ports::kPortTftp, TFTPDaemon::StaticCallbackFunction);
    DEBUG_PRINTF("index_=%d", index_);

    from_port_ = network::iana::Ports::kPortTftp;
    block_number_ = 0;
    state_ = State::kWaitingRq;
    is_last_block_ = false;

    DEBUG_EXIT();
}

void TFTPDaemon::Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
{
    buffer_ = const_cast<uint8_t*>(buffer);
    length_ = size;
    from_ip_ = from_ip;
    from_port_ = from_port;

    switch (state_)
    {
        case State::kWaitingRq:
            if (length_ > tftp::min::kFilenameModeLen)
            {
                HandleRequest();
            }
            break;
        case State::kRrqSendPacket:
            DoRead();
            break;
        case State::kRrqRecvAck:
            if (length_ == sizeof(struct tftp::AckPacket))
            {
                HandleRecvAck();
            }
            break;
        case State::kWrqRecvPacket:
            if (length_ <= sizeof(struct tftp::DataPacket))
            {
                HandleRecvData();
            }
            break;
        default:
            assert(0);
            __builtin_unreachable();
            break;
    }
}

void TFTPDaemon::HandleRequest()
{
    auto* const kPacket = reinterpret_cast<struct tftp::ReqPacket*>(buffer_);
    assert(kPacket != nullptr);

    const auto kOpCode = __builtin_bswap16(kPacket->op_code);

    if ((kOpCode != kOpCodeRrq && kOpCode != kOpCodeWrq))
    {
        SendError(kErrorCodeIllOper, "Invalid operation");
        return;
    }

    const char* const kFileName = kPacket->file_name_mode;
    const auto kFileNameLength = strlen(kFileName);

    if (!(1 <= kFileNameLength && kFileNameLength <= tftp::max::kFilenameLen))
    {
        SendError(kErrorCodeOther, "Invalid file name");
        return;
    }

    const char* const kMode = &kPacket->file_name_mode[kFileNameLength + 1];
    tftp::Mode mode;

    if (strncmp(kMode, "octet", 5) == 0)
    {
        mode = tftp::Mode::kBinary;
    }
    else if (strncmp(kMode, "netascii", 8) == 0)
    {
        mode = tftp::Mode::kAscii;
    }
    else
    {
        SendError(kErrorCodeIllOper, "Invalid operation");
        return;
    }

    DEBUG_PRINTF("Incoming %s request from " IPSTR " %s %s", kOpCode == kOpCodeRrq ? "read" : "write", IP2STR(from_ip_), kFileName, kMode);

    switch (kOpCode)
    {
        case kOpCodeRrq:
            if (!FileOpen(kFileName, mode))
            {
                SendError(kErrorCodeNoFile, "File not found");
                state_ = State::kWaitingRq;
            }
            else
            {
                network::udp::End(network::iana::Ports::kPortTftp);
                index_ = network::udp::Begin(from_port_, TFTPDaemon::StaticCallbackFunction);
                state_ = State::kRrqSendPacket;
                DoRead();
            }
            break;
        case kOpCodeWrq:
            if (!FileCreate(kFileName, mode))
            {
                SendError(kErrorCodeAccess, "Access violation");
                state_ = State::kWaitingRq;
            }
            else
            {
                network::udp::End(network::iana::Ports::kPortTftp);
                index_ = network::udp::Begin(from_port_, TFTPDaemon::StaticCallbackFunction);
                state_ = State::kWrqSendAck;
                DoWriteAck();
            }
            break;
        default:
            assert(0);
            __builtin_unreachable();
            break;
    }
}

void TFTPDaemon::SendError(uint16_t error_code, const char* error_message)
{
    tftp::ErrorPacket error_packet;

    error_packet.op_code = __builtin_bswap16(kOpCodeError);
    error_packet.error_code = __builtin_bswap16(error_code);
    strncpy(error_packet.err_msg, error_message, sizeof(error_packet.err_msg) - 1);

    network::udp::Send(index_, reinterpret_cast<const uint8_t*>(&error_packet), sizeof error_packet, from_ip_, from_port_);
}

void TFTPDaemon::DoRead()
{
    auto* const kDataPacket = reinterpret_cast<struct tftp::DataPacket*>(buffer_);
    assert(kDataPacket != nullptr);

    if (state_ == State::kRrqSendPacket)
    {
        data_length_ = FileRead(kDataPacket->data, tftp::max::kDataLen, ++block_number_);

        kDataPacket->op_code = __builtin_bswap16(kOpCodeData);
        kDataPacket->block_number = __builtin_bswap16(block_number_);

        packet_length_ = sizeof kDataPacket->op_code + sizeof kDataPacket->block_number + data_length_;
        is_last_block_ = data_length_ < tftp::max::kDataLen;

        if (is_last_block_)
        {
            FileClose();
        }

        DEBUG_PRINTF("data_length_=%u, packet_length_=%d, is_last_block_=%d", data_length_, packet_length_, is_last_block_);
    }

    DEBUG_PRINTF("Sending to " IPSTR ":%d", IP2STR(from_ip_), from_port_);

    network::udp::Send(index_, buffer_, packet_length_, from_ip_, from_port_);

    state_ = State::kRrqRecvAck;
}

void TFTPDaemon::HandleRecvAck()
{
    const auto* const kAckPacket = reinterpret_cast<struct tftp::AckPacket*>(buffer_);
    assert(kAckPacket != nullptr);

    if (kAckPacket->op_code == __builtin_bswap16(kOpCodeAck))
    {
        DEBUG_PRINTF("Incoming from " IPSTR ", block_number=%d, block_number_=%d", IP2STR(from_ip_), __builtin_bswap16(kAckPacket->block_number),
                     block_number_);

        if (kAckPacket->block_number == __builtin_bswap16(block_number_))
        {
            state_ = is_last_block_ ? State::kInit : State::kRrqSendPacket;
        }
    }
}

void TFTPDaemon::DoWriteAck()
{
    auto* const kAckPacket = reinterpret_cast<struct tftp::AckPacket*>(buffer_);
    assert(kAckPacket != nullptr);

    kAckPacket->op_code = __builtin_bswap16(kOpCodeAck);
    kAckPacket->block_number = __builtin_bswap16(block_number_);
    state_ = is_last_block_ ? State::kInit : State::kWrqRecvPacket;

    DEBUG_PRINTF("Sending to " IPSTR ":%d, state_=%d", IP2STR(from_ip_), from_port_, static_cast<int>(state_));

    network::udp::Send(index_, buffer_, sizeof(struct tftp::AckPacket), from_ip_, from_port_);

    if (state_ == State::kInit)
    {
        Init();
    }
}

void TFTPDaemon::HandleRecvData()
{
    const auto* const kDataPacket = reinterpret_cast<struct tftp::DataPacket*>(buffer_);
    assert(kDataPacket != nullptr);

    if (kDataPacket->op_code == __builtin_bswap16(kOpCodeData))
    {
        data_length_ = length_ - 4;
        block_number_ = __builtin_bswap16(kDataPacket->block_number);

        DEBUG_PRINTF("Incoming from " IPSTR ", length_=%u, block_number_=%d, data_length_=%u", IP2STR(from_ip_), length_, block_number_, data_length_);

        if (data_length_ == FileWrite(kDataPacket->data, data_length_, block_number_))
        {
            if (data_length_ < tftp::max::kDataLen)
            {
                is_last_block_ = true;
                FileClose();
            }

            DoWriteAck();
        }
        else
        {
            SendError(kErrorCodeDiskFull, "Write failed");
            state_ = State::kInit;
            Init();
        }
    }
}
