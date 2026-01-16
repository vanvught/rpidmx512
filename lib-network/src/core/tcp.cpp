/**
 * @file tcp.cpp
 */
/* Copyright (C) 2021-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
 * Disclaimer: This code has implemented the server side only.
 * Server side states:
 * LISTEN -> ESTABLISHED -> CLOSE_WAIT -> LAST_ACK -> CLOSED:LISTEN
 *
 * Retransmission implemented:
 * - Tracks each outgoing segment that consumes sequence space (data, SYN, FIN)
 * - Copies payload into a fixed pool for later resend
 * - On ACK: pops fully-acked segments from the head, frees payload blocks
 * - On timeout: retransmits the oldest unacked segment, exponential backoff
 * - Drops connection after kTcpRtxMaxRetry
 *
 * Not implemented (by design):
 * - RX buffering
 * - RTT measurement / Jacobson-Karels RTO
 * - Fast retransmit (dupACK counting)
 * - SACK-based partial ack handling
 * - Congestion control / cwnd
 * - Zero-window probing
 */

#if defined(DEBUG_NET_TCP)
#undef NDEBUG
#endif

#pragma GCC diagnostic push
#if (__GNUC__ < 10)
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")

namespace console
{
void Error(const char*);
}

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <cassert>

#include "core/netif.h"
#include "net_config.h"
#include "core/protocol/ieee.h"
#include "core/protocol/tcp.h"
#include "net_private.h"
#include "hal.h"
#include "hal_millis.h"
#include "firmware/debug/debug_debug.h"
#include "network_tcp.h"
#include "core/protocol/ethernet.h"
#include "core/ip4/arp.h"
#include "core/protocol/ip4.h"
#include "network_memory.h"
#include "network_tcp_datasegmentqueue.h"

namespace network::tcp
{
static constexpr auto kAdvertisedRxWnd = kTcpDataMss;
// Retransmission support
static constexpr uint32_t kTcpRtoInitialMs = 1000;
static constexpr uint32_t kTcpRtoMaxMs = 60000;
static constexpr uint32_t kTcpRtxMaxRetry = 5;
static constexpr uint32_t kTcpUnackMax = 8;

struct RtxSeg
{
    uint32_t seq;
    uint16_t len;
    uint16_t consumed;
    uint8_t ctl;
    uint8_t retries;
    uint32_t last_sent;
    uint16_t pool_idx; // 0xFFFF = no payload
};

struct RtxQueue
{
    RtxSeg q[kTcpUnackMax];
    uint8_t head;
    uint8_t count;
};

// RFC 793: 2*MSL. Pick a value that matches your environment.
// Common stacks use 60s or 120s. Embedded often uses 30s..60s.
constexpr uint32_t kTimeWaitMs = 60000; // example 60s

struct Listener
{
    bool in_use;                     // True if this listener slot is active.
    uint16_t local_port;             // Port we listen on.
    network::tcp::CallbackListen cb; // App callback for connections/events.

    // Optional: backlog, security flags, user context pointer, etc.
};

///< Transmission control block (TCB)
struct Tcb
{
    network::tcp::CallbackListen cb_listen;
    network::tcp::CallbackConnect cb_connect;

    uint8_t local_ip[network::ip4::kAddressLength];
    uint8_t remote_ip[network::ip4::kAddressLength];

    uint16_t local_port; // Port we listen on.
    uint16_t remote_port;

    uint8_t remote_eth_addr[ethernet::kAddressLength];

    // Send Sequence Variables
    struct
    {
        uint32_t UNA; // send unacknowledged	// NOLINT
        uint32_t NXT; // send next				// NOLINT
        uint32_t WND; // send window 			// NOLINT
        uint16_t UP;  // send urgent pointer 	// NOLINT
        uint32_t WL1; // segment sequence number used for last window update	// NOLINT
        uint32_t WL2; // segment acknowledgment number used for last window		// NOLINT
    } SND;            // NOLINT

    uint32_t ISS; // initial send sequence number // NOLINT

    struct
    {
        uint32_t recent; // holds a timestamp to be echoed in TSecr whenever a segment is sent
    } TS;                // NOLINT

    uint16_t SendMSS; // NOLINT

    struct
    {
        uint8_t* data;
        uint32_t size;
    } TX; // NOLINT

    // Receive Sequence Variables
    struct
    {
        uint32_t NXT; // receive next // NOLINT
        uint16_t WND; // receive window // NOLINT
        uint16_t UP;  // receive urgent pointer // NOLINT
    } RCV;            // NOLINT

    uint32_t IRS; // initial receive sequence number // NOLINT

    uint8_t state;

    bool did_send_ack_or_data;
    bool in_use; // True if this listener slot is active.

    network::tcp::datasegment::Queue tx_queue;

    uint32_t timewait_deadline;

    // Retransmission
    RtxQueue rtx;
    uint32_t rtx_deadline;
    uint32_t rtx_rto;
};

struct SendInfo
{
    uint32_t SEQ; // NOLINT
    uint32_t ACK; // NOLINT
    uint8_t CTL;  // NOLINT
};

static void RtxClear(Tcb* tcb)
{
    while (tcb->rtx.count > 0)
    {
        auto& r = tcb->rtx.q[tcb->rtx.head];
        network::memory::Allocator::Instance().Free(r.pool_idx);
        tcb->rtx.head = (tcb->rtx.head + 1) % kTcpUnackMax;
        tcb->rtx.count--;
    }
    tcb->rtx_deadline = 0;
}

static struct Header s_eth_frame SECTION_NETWORK ALIGNED;
static uint16_t s_id SECTION_NETWORK ALIGNED;
static struct Listener s_listeners[TCP_MAX_PORTS_ALLOWED] SECTION_NETWORK ALIGNED;
static struct Tcb s_tcbs[TCP_MAX_TCBS_ALLOWED] SECTION_NETWORK ALIGNED;

#ifndef NDEBUG
static const char* const kStateName[] = {"CLOSED", "LISTEN", "SYN-SENT", "SYN-RECEIVED", "ESTABLISHED", "FIN-WAIT-1", "FIN-WAIT-2", "CLOSE-WAIT", "CLOSING", "LAST-ACK", "TIME-WAIT"};

static uint8_t NewState(struct Tcb* p_tcb, uint8_t state, const char* func, const char* file, unsigned line)
{
    assert(p_tcb->state < sizeof kStateName / sizeof kStateName[0]);
    assert(state < sizeof kStateName / sizeof kStateName[0]);

    printf("%s() %s, line %i: %s -> %s\n", func, file, line, kStateName[p_tcb->state], kStateName[state]);

    return p_tcb->state = state;
}

static void UnexpectedState(uint32_t state, uint32_t line)
{
    printf("Unexpected state %s at line %u\n", kStateName[state], line);
}

#define NEW_STATE(tcb, STATE) NewState(tcb, STATE, __func__, __FILE__, __LINE__)
#define UNEXPECTED_STATE() UnexpectedState(tcb->state, __LINE__)
#define CLIENT_NOT_IMPLEMENTED assert(0)
#else
static void NEW_STATE(struct Tcb* tcb, uint8_t state) // NOLINT
{
    tcb->state = state;
}
#define UNEXPECTED_STATE() ((void)0)
#define CLIENT_NOT_IMPLEMENTED ((void)0)
#endif

///< https://www.rfc-editor.org/rfc/rfc9293.html#name-header-format
enum Control : uint8_t
{
    URG = 0x20, ///< Urgent Pointer field significant	// NOLINT
    ACK = 0x10, ///< Acknowledgment field significant	// NOLINT
    PSH = 0x08, ///< Acknowledgment						// NOLINT
    RST = 0x04, ///< Reset the connection				// NOLINT
    SYN = 0x02, ///< Synchronize sequence numbers		// NOLINT
    FIN = 0x01  ///< No more data from sender			// NOLINT
};

///< https://www.rfc-editor.org/rfc/rfc9293.html#name-specific-option-definitions
///< Mandatory Option Set: https://www.rfc-editor.org/rfc/rfc9293.html#table-1
enum Option
{
    kKindEnd = 0,      ///< End of option list
    kKindNop = 1,      ///< No-Operation
    kKindMss = 2,      ///< Maximum Segment Size
    kKindTimestamp = 8 ///< RFC 7323 Timestamp value, Timestamp echo reply (2*4 byte)
};

static constexpr auto kOptionMssLength = 4U;
static constexpr auto kOptionTimestampLength = 10U;

///<  RFC 793, Page 21
enum
{
    kStateClosed,      ///< Is fictional because it represents the state when there is no TCB, and therefore, no connection.
    kStateListen,      ///< Represents waiting for a connection request from any remote TCP and port.
    kStateSynSent,     ///< Represents waiting for a matching connection request after having sent a connection request.
    kStateSynReceived, ///< Represents waiting for a confirming connection request acknowledgment after having both received and sent a connection request.
    kStateEstablished, ///< Represents an open connection, data received can be delivered to the user.  The normal state for the data transfer phase of the connection.
    kStateFinWait1,    ///< Represents waiting for a connection termination request from the remote TCP, or an acknowledgment of the connection termination request previously sent.
    kStateFinWait2,    ///< Represents waiting for a connection termination request from the remote TCP.
    kStateCloseWait,   ///< Represents waiting for a connection termination request from the local user.
    kStateClosing,     ///< Represents waiting for a connection termination request acknowledgment from the remote TCP.
    kStateLastAck,     ///< Represents waiting for an acknowledgment of the connection termination request previously sent to the remote TCP (which includes an acknowledgment of its connection termination request).
    kStateTimeWait     ///< Represents waiting for enough time to pass to be sure the remote TCP received the acknowledgment of its connection termination request.
};

#define offset2octets(x) (((x) >> 4) * 4)

static constexpr bool Lt(uint32_t x, uint32_t y)
{
    return static_cast<int32_t>(x - y) < 0;
}

static constexpr bool Leq(uint32_t x, uint32_t y)
{
    return static_cast<int32_t>(x - y) <= 0;
}

static constexpr bool Gt(uint32_t x, uint32_t y)
{
    return static_cast<int32_t>(x - y) > 0;
}

// static constexpr bool SEQ_GEQ(uint32_t x, uint32_t y) {
//	return static_cast<int32_t>(x - y) >= 0;
// }
//
// static constexpr bool SEQ_BETWEEN(uint32_t l, uint32_t x, uint32_t h) {
//	return SEQ_LT(l, x) && SEQ_LT(x, h);
// }

static constexpr bool BetweenL(uint32_t l, uint32_t x, uint32_t h)
{
    return Leq(l, x) && Lt(x, h); // low border inclusive
}

static constexpr bool BetweenH(uint32_t l, uint32_t x, uint32_t h)
{
    return Lt(l, x) && Leq(x, h); // high border inclusive
}

static constexpr bool BetweenLh(uint32_t l, uint32_t x, uint32_t h)
{
    return Leq(l, x) && Leq(x, h); // both borders inclusive
}

typedef union pcast32
{
    uint32_t u32;
    uint8_t u8[4];
} _pcast32;

static uint32_t TcpGetSeqnum(struct Header* const kTcp)
{
    _pcast32 src;
    memcpy(src.u8, &kTcp->tcp.seqnum, 4);
    return src.u32;
}

static uint32_t TcpGetAcknum(struct Header* const kTcp)
{
    _pcast32 src;
    memcpy(src.u8, &kTcp->tcp.acknum, 4);
    return src.u32;
}

static void TcpSwap32AcknumSeqnum(struct Header* const kTcp)
{
    _pcast32 src;

    src.u32 = __builtin_bswap32(TcpGetAcknum(kTcp));
    memcpy(&kTcp->tcp.acknum, src.u8, 4);

    src.u32 = __builtin_bswap32(TcpGetSeqnum(kTcp));
    memcpy(&kTcp->tcp.seqnum, src.u8, 4);
}

static void TcpInitTcb(struct Tcb* tcb, uint16_t local_port)
{
    tcb->local_port = local_port;

    tcb->ISS = hal::Millis();

    tcb->RCV.WND = kAdvertisedRxWnd;

    tcb->SND.UNA = tcb->ISS;
    tcb->SND.NXT = tcb->ISS;
    tcb->SND.WL2 = tcb->ISS;

    NEW_STATE(tcb, kStateListen);
}

static void RtxOnAck(Tcb* tcb, uint32_t ack)
{
    while (tcb->rtx.count > 0)
    {
        auto& r = tcb->rtx.q[tcb->rtx.head];
        if (Leq(r.seq + r.consumed, ack))
        {
            memory::Allocator::Instance().Free(r.pool_idx);
            tcb->rtx.head = (tcb->rtx.head + 1) % kTcpUnackMax;
            tcb->rtx.count--;
        }
        else
        {
            break;
        }
    }

    if (tcb->rtx.count == 0)
    {
        tcb->rtx_deadline = 0;
    }
    else
    {
        tcb->rtx_deadline = hal::Millis() + tcb->rtx_rto;
    }
}

__attribute__((cold)) void Init()
{
    DEBUG_ENTRY();

    // Ethernet
    std::memcpy(s_eth_frame.ether.src, netif::global::netif_default.hwaddr, ethernet::kAddressLength);
    s_eth_frame.ether.type = __builtin_bswap16(network::ethernet::Type::kIPv4);
    // IPv4
    s_eth_frame.ip4.ver_ihl = 0x45;
    s_eth_frame.ip4.tos = 0;
    s_eth_frame.ip4.flags_froff = __builtin_bswap16(network::ip4::Flags::kFlagDf);
    s_eth_frame.ip4.ttl = 64;
    s_eth_frame.ip4.proto = network::ip4::Proto::kTcp;

    DEBUG_EXIT();
}

///< TCP Checksum Pseudo Header
struct TcpPseudo
{
    uint8_t src_ip[network::ip4::kAddressLength];
    uint8_t dst_ip[network::ip4::kAddressLength];
    uint8_t zero;
    uint8_t proto;
    uint16_t length;
};

static constexpr uint32_t kTcpPseudoLen = 12;

static uint16_t TcpChecksumPseudoHeader(struct Header* eth_frame, const struct Tcb* const kTcb, uint16_t length)
{
    uint8_t buf[kTcpPseudoLen];
    // Store current data before TCP header in temporary buffer
    auto* pseu = reinterpret_cast<struct TcpPseudo*>(reinterpret_cast<uint8_t*>(&eth_frame->tcp) - kTcpPseudoLen);
    memcpy(buf, pseu, kTcpPseudoLen);

    // Generate TCP psuedo header
    std::memcpy(pseu->src_ip, kTcb->local_ip, network::ip4::kAddressLength);
    std::memcpy(pseu->dst_ip, kTcb->remote_ip, network::ip4::kAddressLength);
    pseu->zero = 0;
    pseu->proto = network::ip4::Proto::kTcp;
    pseu->length = __builtin_bswap16(length);

    const auto kSum = network::Chksum(pseu, length + kTcpPseudoLen);

    // Restore data before TCP header from temporary buffer
    memcpy(pseu, buf, kTcpPseudoLen);

    return kSum;
}

static constexpr uint8_t kZeromac[network::ethernet::kAddressLength] = {0, 0, 0, 0, 0, 0};

static void Ip4SendSegment(const Tcb* tcb, void* data, uint32_t size)
{
    if (memcmp(tcb->remote_eth_addr, kZeromac, network::ethernet::kAddressLength) != 0) // Server
    {
        return emac_eth_send(data, size);
    }

    // Client the destination Ethernet address (MAC) is not known
    _pcast32 src;
    memcpy(src.u8, tcb->remote_ip, 4);

    network::arp::Send(data, size, src.u32);
}

static void SendSegment(Tcb* tcb, const SendInfo& send_info, bool track_rtx = true)
{
    tcb->did_send_ack_or_data = true;

    uint32_t opt_bytes = 0;

    if (send_info.CTL & Control::SYN) opt_bytes += 4; // MSS
    opt_bytes += 12;                                  // TSopt (NOP,NOP,TS)

    assert((opt_bytes % 4) == 0);

    const uint32_t kHeaderLength = kHeaderSize + opt_bytes;
    const uint32_t kDataOffset = kHeaderLength / 4; // TCP data offset field
    const auto kTcpLength = kHeaderLength + tcb->TX.size;

    // Ethernet
    std::memcpy(s_eth_frame.ether.dst, tcb->remote_eth_addr, ethernet::kAddressLength);
    // IPv4
    s_eth_frame.ip4.id = s_id++;
    s_eth_frame.ip4.len = __builtin_bswap16(static_cast<uint16_t>(kTcpLength + sizeof(struct network::ip4::Ip4Header)));
    std::memcpy(s_eth_frame.ip4.src, tcb->local_ip, network::ip4::kAddressLength);
    std::memcpy(s_eth_frame.ip4.dst, tcb->remote_ip, network::ip4::kAddressLength);
    s_eth_frame.ip4.chksum = 0;
#if !defined(CHECKSUM_BY_HARDWARE)
    s_eth_frame.ip4.chksum = network::Chksum(reinterpret_cast<void*>(&s_eth_frame.ip4), 20);
#endif
    // TCP
    s_eth_frame.tcp.srcpt = tcb->local_port;
    s_eth_frame.tcp.dstpt = tcb->remote_port;
    s_eth_frame.tcp.seqnum = send_info.SEQ;
    s_eth_frame.tcp.acknum = send_info.ACK;
    s_eth_frame.tcp.offset = static_cast<uint8_t>(kDataOffset << 4);
    s_eth_frame.tcp.control = send_info.CTL;
    s_eth_frame.tcp.window = tcb->RCV.WND;
    s_eth_frame.tcp.urgent = tcb->SND.UP;
    s_eth_frame.tcp.checksum = 0;

    auto* data = reinterpret_cast<uint8_t*>(&s_eth_frame.tcp.data);

    // Add options
    if (send_info.CTL & Control::SYN)
    {
        *data++ = Option::kKindMss;
        *data++ = kOptionMssLength;
        *(reinterpret_cast<uint16_t*>(data)) = __builtin_bswap16(kTcpDataMss);
        data += 2;
    }

    *data++ = Option::kKindNop;
    *data++ = Option::kKindNop;
    *data++ = Option::kKindTimestamp;
    *data++ = 10;
    const auto kMillis = __builtin_bswap32(hal::Millis());
    memcpy(data, &kMillis, 4);
    data += 4;
    memcpy(data, &tcb->TS.recent, 4);
    data += 4;

    DEBUG_PRINTF("SEQ=%u, ACK=%u, kTcpLength=%u, kDataOffset=%u, tcb->TX.size=%u", s_eth_frame.tcp.seqnum, s_eth_frame.tcp.acknum, kTcpLength, kDataOffset, tcb->TX.size);

    if (tcb->TX.data != nullptr)
    {
        for (uint32_t i = 0; i < tcb->TX.size; i++)
        {
            *data++ = tcb->TX.data[i];
        }
    }

    s_eth_frame.tcp.srcpt = __builtin_bswap16(s_eth_frame.tcp.srcpt);
    s_eth_frame.tcp.dstpt = __builtin_bswap16(s_eth_frame.tcp.dstpt);
    TcpSwap32AcknumSeqnum(&s_eth_frame);
    s_eth_frame.tcp.window = __builtin_bswap16(s_eth_frame.tcp.window);
    s_eth_frame.tcp.urgent = __builtin_bswap16(s_eth_frame.tcp.urgent);

    s_eth_frame.tcp.checksum = TcpChecksumPseudoHeader(&s_eth_frame, tcb, static_cast<uint16_t>(kTcpLength));

    Ip4SendSegment(tcb, reinterpret_cast<void*>(&s_eth_frame), kTcpLength + sizeof(struct network::ip4::Ip4Header) + sizeof(struct ethernet::Header));

    // ---- Retransmission tracking ----
    const bool kConsumesSeq = (tcb->TX.size != 0) || (send_info.CTL & Control::SYN) || (send_info.CTL & Control::FIN);

    if (track_rtx && kConsumesSeq && tcb->rtx.count < kTcpUnackMax)
    {
        auto& r = tcb->rtx.q[(tcb->rtx.head + tcb->rtx.count) % kTcpUnackMax];
        r.seq = send_info.SEQ;
        r.len = static_cast<uint16_t>(tcb->TX.size);
        r.consumed = r.len + ((send_info.CTL & Control::SYN) ? 1U : 0U) + ((send_info.CTL & Control::FIN) ? 1U : 0U);
        r.ctl = send_info.CTL;
        r.retries = 0;
        r.last_sent = hal::Millis();
        r.pool_idx = (r.len != 0) ? memory::Allocator::Instance().Allocate(tcb->TX.data, r.len) : 0xFFFF;

        tcb->rtx.count++;

        if (tcb->rtx.count == 1)
        {
            tcb->rtx_rto = kTcpRtoInitialMs;
            tcb->rtx_deadline = r.last_sent + tcb->rtx_rto;
        }
    }
}

static void SendReset(struct Header* eth_frame, struct Tcb* const kTcb)
{
    DEBUG_ENTRY();

    if (eth_frame->tcp.control & Control::RST)
    {
        DEBUG_EXIT();
        return;
    }

    struct SendInfo info;
    info.CTL = Control::RST;

    if (eth_frame->tcp.control & Control::ACK)
    {
        info.SEQ = TcpGetAcknum(eth_frame);
    }
    else
    {
        info.SEQ = 0;
        info.CTL |= Control::ACK;
    }

    uint32_t data_length = 0;

    if (eth_frame->tcp.control & Control::SYN)
    {
        data_length++;
    }

    if (eth_frame->tcp.control & Control::FIN)
    {
        data_length++;
    }

    info.ACK = TcpGetSeqnum(eth_frame) + data_length;

    SendSegment(kTcb, info);

    DEBUG_EXIT();
}

static bool SendData(struct Tcb* tcb, const uint8_t* buffer, uint32_t length, bool is_last_segment)
{
    assert(length != 0);
    assert(length <= static_cast<uint32_t>(kTcpDataMss));
    assert(length <= tcb->SND.WND);

    DEBUG_PRINTF("length=%u, pTCB->SND.WND=%u", length, tcb->SND.WND);

    tcb->TX.data = const_cast<uint8_t*>(buffer);
    tcb->TX.size = length;

    struct SendInfo info;
    info.SEQ = tcb->SND.NXT;
    info.ACK = tcb->RCV.NXT;
    info.CTL = Control::ACK;

    if (is_last_segment)
    {
        info.CTL |= Control::PSH;
    }

    SendSegment(tcb, info);

    tcb->TX.data = nullptr;
    tcb->TX.size = 0;

    tcb->SND.NXT += length;
    tcb->SND.WND -= length;

    return false;
}

struct Options
{
    uint8_t kind;
    uint8_t length;
    uint8_t data;
};

static void ScanOptions(struct Header* eth_frame, struct Tcb* const kTcb, int32_t data_offset)
{
    const auto* const kTcpHeaderEnd = reinterpret_cast<uint8_t*>(&eth_frame->tcp) + data_offset;

    auto* options = reinterpret_cast<struct Options*>(eth_frame->tcp.data);

    while (reinterpret_cast<uint8_t*>(options + 2) <= kTcpHeaderEnd)
    {
        switch (options->kind)
        {
            case Option::kKindEnd:
                return;
                break;
            case Option::kKindNop:
                options = reinterpret_cast<struct Options*>(reinterpret_cast<uint8_t*>(options) + 1);
                break;
            case Option::kKindMss:
                if ((options->length == kOptionMssLength) && ((reinterpret_cast<uint8_t*>(options) + kOptionMssLength) <= kTcpHeaderEnd))
                {
                    const auto* p = &options->data;
                    auto mss = (p[0] << 8) + p[1];
                    // RFC 1122 section 4.2.2.6
                    mss = std::min(static_cast<int32_t>(mss + 20), static_cast<int32_t>(kTcpDataMss)) - kHeaderSize; // - IP_OPTION_SIZE;
                    kTcb->SendMSS = static_cast<uint16_t>(mss);
                }
                options = reinterpret_cast<struct Options*>(reinterpret_cast<uint8_t*>(options) + options->length);
                break;
            case Option::kKindTimestamp: // RFC 7323  3.  TCP Timestamps Option
                if ((options->length == kOptionTimestampLength) && ((reinterpret_cast<uint8_t*>(options) + kOptionTimestampLength) <= kTcpHeaderEnd))
                {
                    _pcast32 tsval;
                    memcpy(tsval.u8, &options->data, 4);
#ifndef NDEBUG
                    auto bIgnore = true;
#endif
                    if (eth_frame->tcp.control & Control::SYN)
                    {
                        kTcb->TS.recent = tsval.u32;
#ifndef NDEBUG
                        bIgnore = false;
#endif
                    }
                    else if ((__builtin_bswap32(tsval.u32) > __builtin_bswap32(kTcb->TS.recent)))
                    { // TODO(a)
                        kTcb->TS.recent = tsval.u32;
#ifndef NDEBUG
                        bIgnore = false;
#endif
                    }

                    DEBUG_PRINTF("TSVal=%u [ignore:%c]", __builtin_bswap32(tsval.u32), bIgnore ? 'Y' : 'N');
                }
                options = reinterpret_cast<struct Options*>(reinterpret_cast<uint8_t*>(options) + options->length);
                break;
            default:
                options = reinterpret_cast<struct Options*>(reinterpret_cast<uint8_t*>(options) + options->length);
                break;
        }
    }
}

static void FreeTcb(Tcb* tcb);

__attribute__((hot)) void Run()
{
    for (auto& tcb : s_tcbs)
    {
        if (!tcb.in_use)
        {
            continue;
        }

        // Server-side close handling (unchanged logic, now per-connection)
        if (tcb.state == kStateCloseWait)
        {
            SendInfo info;
            info.SEQ = tcb.SND.NXT;
            info.ACK = tcb.RCV.NXT;
            info.CTL = Control::FIN | Control::ACK;

            SendSegment(&tcb, info);

            NEW_STATE(&tcb, kStateLastAck);

            tcb.SND.NXT++;
        }

        // Client-side  TIME-WAIT expiry
        if (tcb.state == kStateTimeWait)
        {
            if (tcb.timewait_deadline != 0 && hal::Millis() >= tcb.timewait_deadline)
            {
                NEW_STATE(&tcb, kStateClosed);
                FreeTcb(&tcb);
                continue;
            }

            // RX path handles incoming segments in TIME-WAIT.
            continue;
        }

        // Flush per-connection queue
        auto& q = tcb.tx_queue;

        while (!q.IsEmpty() && q.GetFront().length <= tcb.SND.WND)
        {
            const auto& seg = q.GetFront();
            SendData(&tcb, seg.buffer, seg.length, seg.is_last_segment);
            q.Pop();
        }

        // ---- Retransmission timeout ----
        if (tcb.rtx.count > 0 && tcb.rtx_deadline != 0 && hal::Millis() >= tcb.rtx_deadline)
        {
            auto& r = tcb.rtx.q[tcb.rtx.head];

            SendInfo info;
            info.SEQ = r.seq;
            info.ACK = tcb.RCV.NXT;
            info.CTL = r.ctl | Control::ACK;

            tcb.TX.data = nullptr;
            tcb.TX.size = 0;

            if (r.pool_idx != 0xFFFF)
            {
                tcb.TX.data = memory::Allocator::Instance().Get(r.pool_idx, tcb.TX.size);
            }

            SendSegment(&tcb, info, false);

            tcb.TX.data = nullptr;
            tcb.TX.size = 0;

            r.last_sent = hal::Millis();
            r.retries++;

            if (r.retries > kTcpRtxMaxRetry)
            {
                FreeTcb(&tcb);
                continue;
            }

            tcb.rtx_rto = std::min(tcb.rtx_rto * 2U, kTcpRtoMaxMs);
            tcb.rtx_deadline = hal::Millis() + tcb.rtx_rto;
        }
    }
}

static Listener* FindListenerByPort(uint16_t local_port)
{
    for (auto& l : s_listeners)
    {
        if (l.in_use && l.local_port == local_port)
        {
            return &l;
        }
    }
    return nullptr;
}

static Tcb* FindActiveConn(const Header* const kEthFrame, uint32_t* out_index)
{
    for (uint32_t i = 0; i < TCP_MAX_TCBS_ALLOWED; ++i)
    {
        auto* c = &s_tcbs[i];

        if (!c->in_use)
        {
            continue;
        }

        // Match only non-listen/non-closed connections.
        // (You will not keep LISTEN in the TCB long-term, but for server bring-up
        // you may set it temporarily when creating a new conn.)
        if (c->state == kStateClosed)
        {
            continue;
        }

        // For server bring-up, matching remote ip+port is enough because local port is fixed.
        if (c->local_port == kEthFrame->tcp.dstpt && c->remote_port == kEthFrame->tcp.srcpt && std::memcmp(c->remote_ip, kEthFrame->ip4.src, network::ip4::kAddressLength) == 0)
        {
            if (out_index != nullptr)
            {
                *out_index = i;
            }
            return c;
        }
    }

    return nullptr;
}

// Allocate and initialize a TCB from the global pool.
// Returns nullptr if no free slot is available.
// If out_index != nullptr, it receives the connection handle.
static Tcb* AllocTcb(uint16_t local_port, uint32_t* out_index)
{
    for (uint32_t i = 0; i < TCP_MAX_TCBS_ALLOWED; ++i)
    {
        Tcb* c = &s_tcbs[i];

        // Free slot = not in use
        if (!c->in_use)
        {
            std::memset(c, 0, sizeof(*c));
            // Mark allocated FIRST to avoid reentrancy issues
            // if Input() is ever called from interrupt context.
            c->in_use = true;

            // Initialize all TCP state for this connection.
            // This resets sequence numbers, windows, state, etc.
            TcpInitTcb(c, local_port);

            // Return handle to caller
            if (out_index != nullptr)
            {
                *out_index = i;
            }

            return c;
        }
    }

    DEBUG_PUTS("No free TCB slots");
    return nullptr;
}

static Tcb* AcceptNewConnection(const Header* tcp_segment, uint32_t* out_index)
{
    // 1. Must have a listener for this destination port
    auto* listener = FindListenerByPort(tcp_segment->tcp.dstpt);
    if (listener == nullptr)
    {
        return nullptr;
    }

    // 2. Allocate and initialize TCB
    //    AllocTcb() already:
    //    - zeroes the TCB
    //    - sets nLocalPort
    //    - initializes ISS, SND/RCV windows
    //    - sets STATE_LISTEN
    auto* tcb = AllocTcb(tcp_segment->tcp.dstpt, out_index);
    if (tcb == nullptr)
    {
        return nullptr;
    }

    // 3. Fill in peer-specific tuple fields
    tcb->remote_port = tcp_segment->tcp.srcpt;
    std::memcpy(tcb->remote_ip, tcp_segment->ip4.src, network::ip4::kAddressLength);

    // 4. Server learns remote MAC from inbound Ethernet frame
    std::memcpy(tcb->remote_eth_addr, tcp_segment->ether.src, ethernet::kAddressLength);

    // 5. Attach the listener's callback to this connection
    tcb->cb_listen = listener->cb;

    // STATE_LISTEN is already set by TcpInitTcb()
    return tcb;
}

static inline void EnterTimeWait(Tcb* tcb)
{
    NEW_STATE(tcb, kStateTimeWait);

    tcb->timewait_deadline = hal::Millis() + kTimeWaitMs;

    // Turn off other timers
    tcb->rtx.count = 0;    // drop unacked queue
    tcb->rtx_deadline = 0; // disable rtx timer
    tcb->rtx_rto = 0;
}

// Frees a TCB slot back to the global pool.
static void FreeTcb(Tcb* tcb)
{
    assert(tcb != nullptr);

    RtxClear(tcb);
    std::memset(tcb, 0, sizeof(*tcb));
    tcb->state = kStateClosed; // keep this in case CLOSED != 0
}

// https://www.rfc-editor.org/rfc/rfc9293.html#name-segment-arrives
__attribute__((hot)) void Input(struct Header* eth_frame)
{
    // Convert ports to host endian early (as you already do).
    eth_frame->tcp.srcpt = __builtin_bswap16(eth_frame->tcp.srcpt);
    eth_frame->tcp.dstpt = __builtin_bswap16(eth_frame->tcp.dstpt);

    DEBUG_PRINTF(IPSTR ":%d[%d]", eth_frame->ip4.src[0], eth_frame->ip4.src[1], eth_frame->ip4.src[2], eth_frame->ip4.src[3], eth_frame->tcp.dstpt, eth_frame->tcp.srcpt);

    // Special case reject for 443 unchanged
    if (eth_frame->tcp.dstpt == 443 && (eth_frame->tcp.control & Control::SYN))
    {
        Tcb temp;
        std::memset(&temp, 0, sizeof(temp));

        temp.local_port = eth_frame->tcp.dstpt;
        std::memcpy(temp.local_ip, eth_frame->ip4.dst, network::ip4::kAddressLength);

        temp.remote_port = eth_frame->tcp.srcpt;
        std::memcpy(temp.remote_ip, eth_frame->ip4.src, network::ip4::kAddressLength);
        std::memcpy(temp.remote_eth_addr, eth_frame->ether.src, ethernet::kAddressLength);

        TcpSwap32AcknumSeqnum(eth_frame);
        SendReset(eth_frame, &temp);

        DEBUG_PUTS("Rejected HTTPS port 443 with RST");
        DEBUG_EXIT();
        return;
    }

    // Compute data offset early (your code does this later; either is fine).
    const auto kDataOffset = offset2octets(eth_frame->tcp.offset);

    // Parse lengths and swap seq/ack later as you already do.

    // --- Find existing connection or accept new ---
    uint32_t conn_index = 0;
    Tcb* tcb = FindActiveConn(eth_frame, &conn_index);

    const bool kIsSyn = (eth_frame->tcp.control & Control::SYN) != 0;
    const bool kIsAck = (eth_frame->tcp.control & Control::ACK) != 0;

    if (tcb == nullptr)
    {
        // No existing connection. Only a bare SYN can create a new connection.
        if (kIsSyn && !kIsAck)
        {
            tcb = AcceptNewConnection(eth_frame, &conn_index);
        }

        // If still no TCB, behave like CLOSED state: send RST.
        if (tcb == nullptr)
        {
            Tcb temp;
            std::memset(&temp, 0, sizeof(temp));

            temp.local_port = eth_frame->tcp.dstpt;
            std::memcpy(temp.local_ip, eth_frame->ip4.dst, network::ip4::kAddressLength);

            temp.remote_port = eth_frame->tcp.srcpt;
            std::memcpy(temp.remote_ip, eth_frame->ip4.src, network::ip4::kAddressLength);
            std::memcpy(temp.remote_eth_addr, eth_frame->ether.src, ethernet::kAddressLength);

            TcpSwap32AcknumSeqnum(eth_frame);

            ScanOptions(eth_frame, &temp, kDataOffset);
            SendReset(eth_frame, &temp);

            DEBUG_PUTS("No listener / no conn -> RST");
            DEBUG_EXIT();
            return;
        }
    }

    // From here on, pTCB points to the correct global TCB.
    // conn_index is your new connection_handle.
    DEBUG_PRINTF("conn_handle=%u", conn_index);

    // Now proceed with your existing logic:

    const auto kTcplen = static_cast<uint16_t>(__builtin_bswap16(eth_frame->ip4.len) - sizeof(struct network::ip4::Ip4Header));
    const auto kDataLength = static_cast<uint16_t>(kTcplen - kDataOffset);

    TcpSwap32AcknumSeqnum(eth_frame);
    eth_frame->tcp.window = __builtin_bswap16(eth_frame->tcp.window);
    eth_frame->tcp.urgent = __builtin_bswap16(eth_frame->tcp.urgent);

    const auto SEG_LEN = kDataLength;             // NOLINT
    const auto SEG_ACK = TcpGetAcknum(eth_frame); // NOLINT
    const auto SEG_SEQ = TcpGetSeqnum(eth_frame); // NOLINT
    const auto SEG_WND = eth_frame->tcp.window;   // NOLINT

    DEBUG_PRINTF("%u:[%s] %c%c%c%c%c%c SEQ=%u, ACK=%u, tcplen=%u, data_offset=%u, data_length=%u", conn_index, kStateName[tcb->state], eth_frame->tcp.control & Control::URG ? 'U' : '-', eth_frame->tcp.control & Control::ACK ? 'A' : '-',
                 eth_frame->tcp.control & Control::PSH ? 'P' : '-', eth_frame->tcp.control & Control::RST ? 'R' : '-', eth_frame->tcp.control & Control::SYN ? 'S' : '-', eth_frame->tcp.control & Control::FIN ? 'F' : '-', SEG_SEQ, SEG_ACK,
                 kTcplen, kDataOffset, kDataLength);

    ScanOptions(eth_frame, tcb, kDataOffset);
    auto is_acceptable = false;

    // Server
    if (tcb->state == kStateListen)
    {
        std::memcpy(tcb->local_ip, eth_frame->ip4.dst, network::ip4::kAddressLength);

        tcb->remote_port = eth_frame->tcp.srcpt;
        std::memcpy(tcb->remote_ip, eth_frame->ip4.src, network::ip4::kAddressLength);
        std::memcpy(tcb->remote_eth_addr, eth_frame->ether.src, ethernet::kAddressLength);

        // First: ignore RST
        if (eth_frame->tcp.control & Control::RST)
        {
            DEBUG_EXIT();
            return;
        }

        // Second: if ACK in LISTEN, send RST
        if (eth_frame->tcp.control & Control::ACK)
        {
            SendReset(eth_frame, tcb);
            DEBUG_EXIT();
            return;
        }

        // Third: SYN -> create SYN_RECEIVED
        if (eth_frame->tcp.control & Control::SYN)
        {
            tcb->IRS = SEG_SEQ;
            tcb->RCV.NXT = SEG_SEQ + 1;
            tcb->SND.UNA = tcb->ISS;
            tcb->SND.NXT = tcb->ISS + 1;

            SendInfo si{.SEQ = tcb->ISS, .ACK = tcb->RCV.NXT, .CTL = Control::SYN | Control::ACK};
            SendSegment(tcb, si);

            NEW_STATE(tcb, kStateSynReceived);
            return;
        }

        DEBUG_EXIT();
        return;
    }

    // Client
    if (tcb->state == kStateSynSent)
    {
        const auto kCtl = eth_frame->tcp.control;
        const bool kAckPresent = (kCtl & Control::ACK);
        const bool kRst = (kCtl & Control::RST);
        const bool kSyn = (kCtl & Control::SYN);

        bool ack_ok = false;

        if (kAckPresent)
        {
            ack_ok = BetweenH(tcb->ISS, SEG_ACK, tcb->SND.NXT); // ensure this matches RFC boundaries
            if (!ack_ok)
            {
                if (!kRst)
                {
                    // <SEQ=SEG.ACK><CTL=RST>
                    SendReset(eth_frame, tcb);
                }
                return; // drop
            }
        }

        if (kRst)
        {
            if (kAckPresent && ack_ok)
            {
                NEW_STATE(tcb, kStateClosed);
                tcb->cb_connect(UINT32_MAX, network::tcp::Event::kError);
            }
            return; // drop
        }

        if (!kSyn)
        {
            return; // drop
        }

        // SYN is present:
        tcb->IRS = SEG_SEQ;
        tcb->RCV.NXT = SEG_SEQ + 1;

        if (kAckPresent)
        {
            if (Gt(SEG_ACK, tcb->SND.UNA)) tcb->SND.UNA = SEG_ACK;

            // Our SYN is ACKed -> ESTABLISHED
            NEW_STATE(tcb, kStateEstablished);

            tcb->SND.WND = SEG_WND;
            tcb->SND.WL1 = SEG_SEQ;
            tcb->SND.WL2 = SEG_ACK;

            SendInfo si{.SEQ = tcb->SND.NXT, .ACK = tcb->RCV.NXT, .CTL = Control::ACK};
            SendSegment(tcb, si);

            tcb->cb_connect(conn_index, network::tcp::Event::kConnected);
            return;
        }
        else
        {
            // Simultaneous open
            NEW_STATE(tcb, kStateSynReceived);
            SendInfo si{.SEQ = tcb->ISS, .ACK = tcb->RCV.NXT, .CTL = Control::SYN | Control::ACK};
            SendSegment(tcb, si);
            return;
        }
    }

    ///< https://www.rfc-editor.org/rfc/rfc9293.html#name-other-states
    switch (tcb->state)
    {
        case kStateSynReceived:
        case kStateEstablished:
        case kStateFinWait1:
        case kStateFinWait2:
        case kStateCloseWait:
        case kStateClosing:
        case kStateLastAck:
        case kStateTimeWait:
        {
            // There are four cases for the acceptability test for an incoming segment.
            /*
             * RCV.WND:
             * - The receiver’s window size, representing how many more bytes of data it is willing to accept.
             * - A window size of 0 means the receiver cannot currently accept more data (e.g., due to buffer constraints).
             *
             * RCV.NXT:
             * - The sequence number of the next expected byte of data from the sender.
             *
             * SEG_SEQ:
             * - The sequence number of the first byte in the received segment.
             *
             * SEG_LEN:
             * - The length of the data in the received segment (payload size).
             */

            DEBUG_PRINTF("RCV.WND=%u, SEG_LEN=%u, RCV.NXT=%u, SEG_SEQ=%u", tcb->RCV.WND, SEG_LEN, tcb->RCV.NXT, SEG_SEQ);

            if (tcb->RCV.WND > 0)
            {
                if (SEG_LEN == 0)
                {
                    // Case 2: SEG_LEN = 0 RCV.WND > 0 -> RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
                    /*
                     * Condition:
                     * - The received segment is empty (SEG_LEN = 0).
                     * - The receiver's window size is greater than 0.
                     *
                     * Even though the segment contains no data, it might carry control flags (e.g., SYN, FIN) that need to be processed.
                     * It must lie within the allowable sequence range dictated by the receiver’s window.
                     */
                    if (BetweenL(tcb->RCV.NXT, SEG_SEQ, tcb->RCV.NXT + tcb->RCV.WND))
                    {
                        is_acceptable = true;
                    }
                }
                else
                {
                    // Case 4: SEG_LEN > 0 RCV.WND > 0 ->
                    // RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
                    // or
                    // RCV.NXT =< SEG.SEQ+SEG.LEN-1 < RCV.NXT+RCV.WND
                    /*
                     * Condition:
                     * - The received segment contains data (SEG_LEN > 0).
                     * - The receiver’s window size is greater than 0.
                     *
                     * Segments can be partially within the window,
                     * so either the start or the end of the segment must fall within the acceptable range.
                     */
                    if (BetweenL(tcb->RCV.NXT, SEG_SEQ, tcb->RCV.NXT + tcb->RCV.WND) || BetweenL(tcb->RCV.NXT, SEG_SEQ + SEG_LEN - 1, tcb->RCV.NXT + tcb->RCV.WND))
                    {
                        is_acceptable = true;
                    }
                }
            }
            else
            {
                // Case 1: SEG_LEN = 0 RCV.WND = 0 -> SEG.SEQ = RCV.NXT
                /*
                 * Condition:
                 *  - The received segment is empty (SEG_LEN = 0).
                 *  - The receiver's window size is 0.
                 *
                 *  Even though the window is closed,
                 *  the receiver still acknowledges control packets (e.g., ACKs or FIN) that match RCV.NXT.
                 */
                if (SEG_LEN == 0)
                {
                    if (SEG_SEQ == tcb->RCV.NXT)
                    {
                        is_acceptable = true;
                    }
                }
                // Case 3: SEG_LEN > 0 RCV.WND = 0 -> not acceptable
                /*
                 * Condition:
                 * - The received segment contains data (SEG_LEN > 0).
                 * - The receiver's window size is 0.
                 *
                 * The receiver has no buffer space available, so it cannot accept any data.
                 * The sender should wait until the window opens (indicated by an updated ACK from the receiver).
                 */
            }

            DEBUG_PRINTF("is_acceptable=%d", is_acceptable);

            if (!is_acceptable)
            {
                // If an incoming segment is not acceptable, an acknowledgment should be sent in reply
                // (unless the RST bit is set, if so drop the segment and return)
                // <SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK>
                if (eth_frame->tcp.control & Control::RST)
                {
                    FreeTcb(tcb);
                    DEBUG_EXIT();
                    return;
                }

                SendInfo send_info;
                send_info.SEQ = tcb->SND.NXT;
                send_info.ACK = tcb->RCV.NXT;
                send_info.CTL = Control::ACK;

                SendSegment(tcb, send_info);

                DEBUG_EXIT();
                return;
            }

            // second check the RST bit, *//* Page 70 */
            if (eth_frame->tcp.control & Control::RST)
            {
                switch (tcb->state)
                {
                    case kStateSynReceived:
                        FreeTcb(tcb);
                        break;
                    case kStateEstablished:
                    case kStateFinWait1:
                    case kStateFinWait2:
                    case kStateCloseWait:
                        /* If the RST bit is set then, any outstanding RECEIVEs and SEND
                         * should receive "reset" responses.  All segment queues should be
                         * flushed.  Users should also receive an unsolicited general
                         * "connection reset" signal.  Enter the CLOSED state, delete the
                         * TCB, and return. */
                        FreeTcb(tcb);
                        break;
                    case kStateClosing:
                    case kStateLastAck:
                    case kStateTimeWait:
                        /* If the RST bit is set then, enter the CLOSED state, delete the
                         * TCB, and return. */
                        FreeTcb(tcb);
                        break;
                    default:
                        assert(0);
                        break;
                }

                DEBUG_EXIT();
                return;
            }

            // third check security and precedence. No code needed here

            // fourth, check the SYN bit,  /* Page 71 */
            if (eth_frame->tcp.control & Control::SYN)
            {
                // RFC 1122 section 4.2.2.20 (e)
                if (tcb->state == kStateSynReceived)
                {
                    FreeTcb(tcb);
                    DEBUG_EXIT();
                    return;
                }

                SendReset(eth_frame, tcb);

                DEBUG_PUTS("pTcp->tcp.control & Control::SYN");
            }

            /*  fifth check the ACK field, */ /* Page 72 */
            if (!(eth_frame->tcp.control & Control::ACK))
            {
                // if the ACK bit is off drop the segment and return
                DEBUG_EXIT();
                return;
            }

            switch (tcb->state)
            {
                case kStateSynReceived:
                    // If SND.UNA =< SEG.ACK =< SND.NXT then enter ESTABLISHED state and continue processing.
                    if (BetweenLh(tcb->SND.UNA, SEG_ACK, tcb->SND.NXT))
                    {
                        // RFC 1122 section 4.2.2.20 (f)
                        tcb->SND.WND = SEG_WND;
                        tcb->SND.WL1 = SEG_SEQ;
                        tcb->SND.WL2 = SEG_ACK;

                        tcb->SND.UNA = SEG_ACK; // got ACK for SYN

                        RtxOnAck(tcb, SEG_ACK); // Retransmission ACK handling

                        NEW_STATE(tcb, kStateEstablished);
                        DEBUG_EXIT();
                        return;
                    }
                    else
                    {
                        // <SEQ=SEG.ACK><CTL=RST>
                        SendReset(eth_frame, tcb);

                        DEBUG_PUTS("<SEQ=SEG.ACK><CTL=RST>");
                    }
                    break;
                case kStateEstablished:
                case kStateFinWait1:
                case kStateFinWait2:
                case kStateCloseWait:
                case kStateClosing:
                    DEBUG_PRINTF("SND.UNA=%u, SEG_ACK=%u, SND.NXT=%u", tcb->SND.UNA, SEG_ACK, tcb->SND.NXT);

                    if (BetweenH(tcb->SND.UNA, SEG_ACK, tcb->SND.NXT))
                    {
                        auto bytes_ack = SEG_ACK - tcb->SND.UNA;
                        tcb->SND.UNA = SEG_ACK;

                        RtxOnAck(tcb, SEG_ACK); // Retransmission ACK handling

                        if (SEG_ACK == tcb->SND.NXT)
                        {
                            DEBUG_PUTS("All segments are acknowledged");
                        }

                        auto state = tcb->state;

                        // If our FIN has been ACKed, FIN_WAIT_1 -> FIN_WAIT_2
                        if (state == kStateFinWait1 && SEG_ACK == tcb->SND.NXT)
                        {
                            NEW_STATE(tcb, kStateFinWait2);
                        }

                        if (state == kStateFinWait1 || state == kStateClosing)
                        {
                            bytes_ack--;
                            DEBUG_PUTS("Acknowledged FIN does not count");
                        }

                        if (state == kStateEstablished && bytes_ack == 1)
                        {
                            bytes_ack--;
                        }

                        // update send window
                        if (Lt(tcb->SND.WL1, SEG_SEQ) || (tcb->SND.WL1 == SEG_SEQ && Leq(tcb->SND.WL2, SEG_ACK)))
                        {
                            tcb->SND.WND = SEG_WND;
                            tcb->SND.WL1 = SEG_SEQ;
                            tcb->SND.WL2 = SEG_ACK;
                        }
                    }
                    else if (Leq(SEG_ACK, tcb->SND.UNA))
                    { // RFC 1122 section 4.2.2.20 (g)
                        DEBUG_PUTS("Ignore duplicate ACK");
                        if (BetweenLh(tcb->SND.UNA, SEG_ACK, tcb->SND.NXT))
                        {
                            // ... but update send window
                            if (Lt(tcb->SND.WL1, SEG_SEQ) || (tcb->SND.WL1 == SEG_SEQ && Leq(tcb->SND.WL2, SEG_ACK)))
                            {
                                tcb->SND.WND = SEG_WND;
                                tcb->SND.WL1 = SEG_SEQ;
                                tcb->SND.WL2 = SEG_ACK;
                            }
                        }
                    }
                    else if (Gt(SEG_ACK, tcb->SND.NXT))
                    {
                        DEBUG_PRINTF("SEG_ACK=%u, SND.NXT=%u", SEG_ACK, tcb->SND.NXT);

                        SendInfo send_info;
                        send_info.SEQ = tcb->SND.NXT;
                        send_info.ACK = tcb->RCV.NXT;
                        send_info.CTL = Control::ACK;

                        SendSegment(tcb, send_info);

                        DEBUG_EXIT();
                        return;
                    }
                    break;
                case kStateLastAck:
                    if (SEG_ACK == tcb->SND.NXT)
                    { // if our FIN is now acknowledged
                        FreeTcb(tcb);
                    }
                    break;
                case kStateTimeWait:
                    if (SEG_ACK == tcb->SND.NXT)
                    { // if our FIN is now acknowledged
                        SendInfo send_info;
                        send_info.SEQ = tcb->SND.NXT;
                        send_info.ACK = tcb->RCV.NXT;
                        send_info.CTL = Control::ACK;

                        SendSegment(tcb, send_info);
                        CLIENT_NOT_IMPLEMENTED;
                    }
                    break;
                default:
                    UNEXPECTED_STATE();
                    break;
            }

            // sixth, check the URG bit. No code needed here

            // seventh, process the segment text
            // sixth, check the URG bit. No code needed here

            // seventh, process the segment text
            switch (tcb->state)
            {
                case kStateEstablished:
                case kStateFinWait1:
                case kStateFinWait2:
                {
                    if (kDataLength > 0)
                    {
                        // Stack does not support out-of-order segments.
                        // Therefore the only acceptable data segment is exactly at RCV.NXT.
                        if (SEG_SEQ == tcb->RCV.NXT)
                        {
                            // Update receive sequence and window immediately upon accepting data.
                            // (in-order only).
                            tcb->RCV.NXT += kDataLength;
                            tcb->RCV.WND = kAdvertisedRxWnd;

                            // Application callback may send ACK/data itself.
                            // We track if it did, to avoid duplicate ACK.
                            tcb->did_send_ack_or_data = false;

                            // The callback is attached per-connection
                            // (copied from the Listener when the connection was accepted).
                            assert(tcb->cb_listen != nullptr);
                            tcb->cb_listen(conn_index, reinterpret_cast<uint8_t*>(&eth_frame->tcp) + kDataOffset, kDataLength);

                            if (!tcb->did_send_ack_or_data)
                            {
                                // Send acknowledgment (ACK-only segment).
                                const SendInfo kAck{.SEQ = tcb->SND.NXT, .ACK = tcb->RCV.NXT, .CTL = Control::ACK};
                                SendSegment(tcb, kAck);
                            }
                        }
                        else
                        {
                            // Out-of-order segment: send duplicate ACK for current RCV.NXT.
                            const SendInfo kAck{.SEQ = tcb->SND.NXT, .ACK = tcb->RCV.NXT, .CTL = Control::ACK};
                            SendSegment(tcb, kAck);

                            DEBUG_PUTS("Out of order");
                            DEBUG_EXIT();
                            return;
                        }
                    }
                    break;
                }

                default:
                    break;
            }

            /* eighth, check the FIN bit, */ /* Page 75 */
            /*
             Do not process the FIN if the state is CLOSED, LISTEN or SYN-SENT
             since the SEG.SEQ cannot be validated; drop the segment and return.
             */

            if ((tcb->state == kStateClosed) || (tcb->state == kStateListen) || (tcb->state == kStateSynSent))
            {
                DEBUG_EXIT();
                return;
            }

            if (!(eth_frame->tcp.control & Control::FIN))
            {
                DEBUG_EXIT();
                return;
            }

            /*
             If the FIN bit is set, signal the user "connection closing" and
             return any pending RECEIVEs with same message, advance RCV.NXT
             over the FIN, and send an acknowledgment for the FIN.  Note that
             FIN implies PUSH for any segment text not yet delivered to the
             user.
             */

            tcb->RCV.NXT = tcb->RCV.NXT + 1;

            SendInfo send_info;
            send_info.SEQ = tcb->SND.NXT;
            send_info.ACK = tcb->RCV.NXT;
            send_info.CTL = Control::ACK;

            SendSegment(tcb, send_info);

            switch (tcb->state)
            {
                case kStateSynReceived:
                case kStateEstablished:
                    NEW_STATE(tcb, kStateCloseWait);
                    break;
                case kStateFinWait1:
                    /*
                     If our FIN has been ACKed (perhaps in this segment), then
                     enter TIME-WAIT, start the time-wait timer, turn off the other
                     timers; otherwise enter the CLOSING state.
                     */
                    // if ACK acknowledges everything we sent including FIN
                    if (Leq(tcb->SND.NXT, SEG_ACK))
                    { /* if our FIN is now acknowledged */
                        EnterTimeWait(tcb);
                    }
                    else
                    {
                        NEW_STATE(tcb, kStateClosing);
                    }
                    break;
                case kStateFinWait2:
                    // Enter the TIME-WAIT state.  Start the time-wait timer, turn off the other timers.
                    EnterTimeWait(tcb);
                    break;
                case kStateCloseWait:
                    // Remain in the CLOSE-WAIT state.
                case kStateClosing:
                    // Remain in the CLOSING state.
                case kStateLastAck:
                    // Remain in the LAST-ACK state.
                    break;
                case kStateTimeWait:
                {
                    // Always ACK what we have
                    SendInfo si{.SEQ = tcb->SND.NXT, .ACK = tcb->RCV.NXT, .CTL = Control::ACK};
                    SendSegment(tcb, si, false);

                    // Restart TIME-WAIT timer
                    tcb->timewait_deadline = hal::Millis() + kTimeWaitMs;
                    return;
                }

                break;
                default:
                    assert(0);
                    break;
            }
        }
        break;
        default:
            assert(0);
            break;
    }

    DEBUG_EXIT();
}

static Listener* AllocListenerSlot()
{
    for (auto& l : s_listeners)
    {
        if (!l.in_use)
        {
            l.in_use = true;
            return &l;
        }
    }
    return nullptr;
}

// Public API: start listening on a port.
bool Listen(uint16_t local_port, network::tcp::CallbackListen cb)
{
    // One listener per port in this simple design.
    if (FindListenerByPort(local_port) != nullptr)
    {
        return false; // already listening
    }

    Listener* l = AllocListenerSlot();

    if (l == nullptr)
    {
        return false; // no space
    }

    l->local_port = local_port;
    l->cb = cb;

    return true;
}

// Public API:
bool Unlisten(uint16_t local_port)
{
    Listener* l = FindListenerByPort(local_port);

    if (l == nullptr)
    {
        return false;
    }

    l->cb = nullptr;
    l->local_port = 0;
    l->in_use = false;

    return true;
}

// IANA Recommendation	49152 to 65535
static constexpr uint16_t kLocalPortRangeStart = 49152;
static constexpr uint16_t kLocalPortRangeEnd = 65535;
static uint16_t s_local_port = kLocalPortRangeStart;

// Client
int32_t Connect(uint32_t remote_ip, uint16_t remote_port, CallbackConnect cb_connect, CallbackListen cb_listen)
{
    const auto& netif = netif::global::netif_default;
    if (__builtin_expect((netif.ip.addr == 0), 0))
    {
        console::Error("Connect: No ip!");
        return -1;
    }

    uint32_t out_index = 0;

    auto* tcb = AllocTcb(remote_port, &out_index);
    if (tcb == nullptr)
    {
        console::Error("Connect: No TCB!");
        return -2;
    }

    tcb->local_port = s_local_port;
    if (s_local_port++ == kLocalPortRangeEnd)
    {
        s_local_port = kLocalPortRangeStart;
    }

    tcb->remote_port = remote_port;

    _pcast32 ip;
    ip.u32 = netif.ip.addr;
    memcpy(tcb->local_ip, ip.u8, 4);

    ip.u32 = remote_ip;
    memcpy(tcb->remote_ip, ip.u8, 4);

    tcb->cb_listen = cb_listen;
    tcb->cb_connect = cb_connect;

    tcb->SND.UNA = tcb->ISS;
    tcb->SND.NXT = tcb->ISS + 1;

    NEW_STATE(tcb, kStateSynSent);

    SendInfo info{};
    info.CTL = Control::SYN;
    info.SEQ = tcb->ISS;
    info.ACK = 0;

    SendSegment(tcb, info);

    return 0;
}

int32_t Close(ConnHandle conn_handle) // graceful FIN
{
    if (conn_handle >= TCP_MAX_TCBS_ALLOWED)
    {
        console::Error("Close: Connection handle!");
        return -1;
    }

    auto* c = &s_tcbs[conn_handle];

    if (!c->in_use || c->state == kStateClosed)
    {
        console::Error("Close: TCB!");
        return -1;
    }

    // If we’re already closing/closed-ish, treat as success (idempotent close).
    switch (c->state)
    {
        case kStateFinWait1:
        case kStateFinWait2:
        case kStateClosing:
        case kStateLastAck:
        case kStateTimeWait:
            return 0;

        default:
            break;
    }

    // We only support graceful close from states where FIN makes sense here.
    if (c->state != kStateEstablished && c->state != kStateCloseWait)
    {
        console::Error("Close: Not graceful!");
        return -1;
    }

    // No payload for a pure FIN segment.
    c->TX.data = nullptr;
    c->TX.size = 0;

    SendInfo info{};
    info.SEQ = c->SND.NXT;
    info.ACK = c->RCV.NXT;
    info.CTL = static_cast<uint8_t>(Control::FIN | Control::ACK);

    SendSegment(c, info);

    // FIN consumes 1 sequence number.
    c->SND.NXT += 1;

    if (c->state == kStateEstablished)
    {
        NEW_STATE(c, kStateFinWait1);
    }
    else /* kStateCloseWait */
    {
        NEW_STATE(c, kStateLastAck);
    }

    return 0;
}

// Public API:
void Abort(uint32_t conn_handle)
{
    assert(conn_handle < TCP_MAX_TCBS_ALLOWED);

    auto* c = &s_tcbs[conn_handle];

    if (!c->in_use || c->state == kStateClosed)
    {
        return;
    }

    SendInfo info;
    info.CTL = Control::RST;
    info.SEQ = c->SND.NXT;
    info.ACK = c->RCV.NXT;

    SendSegment(c, info);

    FreeTcb(c);
}

// Public API:
int32_t Send(ConnHandle conn_handle, const uint8_t* buffer, uint32_t length)
{
    DEBUG_ENTRY();
    assert(buffer != nullptr);

    if (conn_handle >= TCP_MAX_TCBS_ALLOWED)
    {
        return -1;
    }

    auto* c = &s_tcbs[conn_handle];

    // If this slot isn't in use, it's a stale/invalid handle.
    if (!c->in_use)
    {
        return -1;
    }

    // For now, only allow sending in states where your legacy code expects it.
    // Most stacks allow in ESTABLISHED and also in CLOSE_WAIT (server can still send).
    if (c->state != kStateEstablished && c->state != kStateCloseWait)
    {
        return -1;
    }

    DEBUG_PRINTF("%u -> %u", static_cast<uint32_t>(conn_handle), length);

    const auto* p = buffer;

    // Legacy behavior preserved:
    // Only send if the FULL remaining length fits inside SND.WND.
    // NOTE: Many stacks instead send min(length, wnd)
    while ((length > 0) && (length <= c->SND.WND))
    {
        const uint32_t kWriteLen = (length > kTcpDataMss) ? kTcpDataMss : length;
        const bool kIsLast = (length < kTcpDataMss);

        SendData(c, p, kWriteLen, kIsLast);

        p += kWriteLen;
        length -= kWriteLen;
    }

    if (length == 0)
    {
        DEBUG_EXIT();
        return 0; // everything sent immediately
    }

    auto& q = c->tx_queue;

    if (!q.IsEmpty())
    {
        // Already queued something.
        DEBUG_EXIT();
        return -2;
    }

    while (length > 0)
    {
        if (q.IsFull())
        {
            // Can't queue everything.
            DEBUG_EXIT();
            return -2;
        }

        const uint32_t kWriteLen = (length > kTcpDataMss) ? kTcpDataMss : length;
        const bool kIsLast = (length < kTcpDataMss);

        q.Push(p, kWriteLen, kIsLast);

        p += kWriteLen;
        length -= kWriteLen;
    }

    DEBUG_EXIT();
    return 1; // queued
}
} // namespace network::tcp