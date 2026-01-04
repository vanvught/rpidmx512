/**
 * @file tcp.cpp
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#pragma GCC optimize("-fprefetch-loop-arrays")

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "net/netif.h"
#include "net_config.h"
#include "net/tcp.h"
#include "net/protocol/ieee.h"
#include "net/protocol/tcp.h"
#include "net_memcpy.h"
#include "net_private.h"
#include "datasegmentqueue.h"
#include "hal.h"
#include "hal_millis.h"
#include "firmware/debug/debug_dump.h"
#include "firmware/debug/debug_debug.h"

namespace net::tcp
{
#define TCP_RX_MSS (TCP_DATA_SIZE)
#define TCP_RX_MAX_ENTRIES (1U << 1) // Must always be a power of 2
#define TCP_RX_MAX_ENTRIES_MASK (TCP_RX_MAX_ENTRIES - 1)
#define TCP_MAX_RX_WND (TCP_RX_MAX_ENTRIES * TCP_RX_MSS);
#define TCP_TX_MSS (TCP_DATA_SIZE)

/**
 * Transmission control block (TCB)
 */
struct tcb
{
    uint8_t local_ip[IPv4_ADDR_LEN];
    uint8_t remote_ip[IPv4_ADDR_LEN];

    uint16_t nLocalPort;
    uint16_t nRemotePort;

    uint8_t remoteEthAddr[ETH_ADDR_LEN];

    /* Send Sequence Variables */
    struct
    {
        uint32_t UNA; /* send unacknowledged */
        uint32_t NXT; /* send next */
        uint32_t WND; /* send window */
        uint16_t UP;  /* send urgent pointer */
        uint32_t WL1; /* segment sequence number used for last window update */
        uint32_t WL2; /* segment acknowledgment number used for last window */
    } SND;

    uint32_t ISS; /* initial send sequence number */

    struct
    {
        uint32_t recent; /* holds a timestamp to be echoed in TSecr whenever a segment is sent */
    } TS;

    uint16_t SendMSS;

    struct
    {
        uint8_t* data;
        uint32_t size;
    } TX;

    /* Receive Sequence Variables */
    struct
    {
        uint32_t NXT; /* receive next */
        uint16_t WND; /* receive window */
        uint16_t UP;  /* receive urgent pointer */
    } RCV;

    uint32_t IRS; /* initial receive sequence number */

    uint8_t state;

    bool did_send_ack_or_data;
};

struct SendInfo
{
    uint32_t SEQ;
    uint32_t ACK;
    uint8_t CTL;
};

struct TransmissionQueue
{
    tcb* pTcb;
    DataSegmentQueue dataSegmentQueue;
};

struct PortInfo
{
    tcb TCB[TCP_MAX_TCBS_ALLOWED];
#if defined(TCP_TX_QUEUE_SIZE)
    TransmissionQueue transmissionQueue;
#endif
    TcpCallbackFunctionPtr callback;
    uint16_t nLocalPort;
};

static struct PortInfo s_Ports[TCP_MAX_PORTS_ALLOWED] SECTION_NETWORK ALIGNED;
static uint16_t s_id SECTION_NETWORK ALIGNED;
static struct t_tcp s_tcp SECTION_NETWORK ALIGNED;

#if !defined(NDEBUG)
static const char* s_aStateName[] = {"CLOSED",     "LISTEN",     "SYN-SENT", "SYN-RECEIVED", "ESTABLISHED", "FIN-WAIT-1",
                                     "FIN-WAIT-2", "CLOSE-WAIT", "CLOSING",  "LAST-ACK",     "TIME-WAIT"};

static uint8_t new_state(struct tcb* p_tcb, uint8_t state, const char* func, const char* file, unsigned line)
{
    assert(p_tcb->state < sizeof s_aStateName / sizeof s_aStateName[0]);
    assert(state < sizeof s_aStateName / sizeof s_aStateName[0]);

    printf("%s() %s, line %i: %s -> %s\n", func, file, line, s_aStateName[p_tcb->state], s_aStateName[state]);

    return p_tcb->state = state;
}

static void unexpected_state(uint32_t nState, const uint32_t nLine)
{
    printf("Unexpected state %s at line %u\n", s_aStateName[nState], nLine);
}

#define NEW_STATE(tcb, STATE) new_state(tcb, STATE, __func__, __FILE__, __LINE__)
#define UNEXPECTED_STATE() unexpected_state(pTCB->state, __LINE__)
#define CLIENT_NOT_IMPLEMENTED assert(0)
#else
static void NEW_STATE(struct tcb* pTcb, const uint8_t state)
{
    pTcb->state = state;
}
#define UNEXPECTED_STATE() ((void)0)
#define CLIENT_NOT_IMPLEMENTED ((void)0)
#endif

/**
 * https://www.rfc-editor.org/rfc/rfc9293.html#name-header-format
 */

enum Control : uint8_t
{
    URG = 0x20, ///< Urgent Pointer field significant
    ACK = 0x10, ///< Acknowledgment field significant
    PSH = 0x08, ///< Acknowledgment
    RST = 0x04, ///< Reset the connection
    SYN = 0x02, ///< Synchronize sequence numbers
    FIN = 0x01  ///< No more data from sender
};

/**
 * https://www.rfc-editor.org/rfc/rfc9293.html#name-specific-option-definitions
 * Mandatory Option Set: https://www.rfc-editor.org/rfc/rfc9293.html#table-1
 */

enum Option
{
    kKindEnd = 0,      ///< End of option list
    kKindNop = 1,      ///< No-Operation
    kKindMss = 2,      ///< Maximum Segment Size
    kKindTimestamp = 8 ///< RFC 7323 Timestamp value, Timestamp echo reply (2*4 byte)
};

static constexpr auto kOptionMssLength = 4U;
static constexpr auto kOptionTimestampLength = 10U;

/*
 * RFC 793, Page 21
 */

enum
{
    STATE_CLOSED,       /* Is fictional because it represents the state when there is no TCB,
           and therefore, no connection. */
    STATE_LISTEN,       /* Represents waiting for a connection request from any
           remote TCP and port. */
    STATE_SYN_SENT,     /* Represents waiting for a matching connection request
         after having sent a connection request. */
    STATE_SYN_RECEIVED, /* Represents waiting for a confirming connection
     request acknowledgment after having both received and sent
     a connection request. */
    STATE_ESTABLISHED,  /* Represents an open connection, data received can be
      delivered to the user.  The normal state for the data transfer phase
      of the connection. */
    STATE_FIN_WAIT_1,   /* Represents waiting for a connection termination request
       from the remote TCP, or an acknowledgment of the connection
       termination request previously sent. */
    STATE_FIN_WAIT_2,   /* Represents waiting for a connection termination request
        from the remote TCP.*/
    STATE_CLOSE_WAIT,   /* Represents waiting for a connection termination request
        from the local user. */
    STATE_CLOSING,      /* Represents waiting for a connection termination request
           acknowledgment from the remote TCP. */
    STATE_LAST_ACK,     /* represents waiting for an acknowledgment of the
         connection termination request previously sent to the remote TCP
         (which includes an acknowledgment of its connection termination
         request). */
    STATE_TIME_WAIT     /* Represents waiting for enough time to pass to be sure
         the remote TCP received the acknowledgment of its connection
         termination request. */
};

#define offset2octets(x) (((x) >> 4) * 4)

static constexpr bool SeqLt(uint32_t x, uint32_t y)
{
    return static_cast<int32_t>(x - y) < 0;
}

static constexpr bool SeqLeq(uint32_t x, uint32_t y)
{
    return static_cast<int32_t>(x - y) <= 0;
}

static constexpr bool SeqGt(uint32_t x, uint32_t y)
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

static constexpr bool SeqBetweenL(uint32_t l, uint32_t x, uint32_t h)
{
    return SeqLeq(l, x) && SeqLt(x, h); // low border inclusive
}

static constexpr bool SeqBetweenH(uint32_t l, uint32_t x, uint32_t h)
{
    return SeqLt(l, x) && SeqLeq(x, h); // high border inclusive
}

static constexpr bool SeqBetweenLh(uint32_t l, uint32_t x, uint32_t h)
{
    return SeqLeq(l, x) && SeqLeq(x, h); // both borders inclusive
}

typedef union pcast32
{
    uint32_t u32;
    uint8_t u8[4];
} _pcast32;

static uint32_t TcpGetSeqnum(struct t_tcp* const p_tcp)
{
    _pcast32 src;
    memcpy(src.u8, &p_tcp->tcp.seqnum, 4);
    return src.u32;
}

static uint32_t TcpGetAcknum(struct t_tcp* const kTcp)
{
    _pcast32 src;
    memcpy(src.u8, &kTcp->tcp.acknum, 4);
    return src.u32;
}

static void TcpBswap32AcknumSeqnum(struct t_tcp* p_tcp)
{
    _pcast32 src;

    src.u32 = __builtin_bswap32(TcpGetAcknum(p_tcp));
    memcpy(&p_tcp->tcp.acknum, src.u8, 4);

    src.u32 = __builtin_bswap32(TcpGetSeqnum(p_tcp));
    memcpy(&p_tcp->tcp.seqnum, src.u8, 4);
}

static void TcpInitTcb(struct tcb* pTcb, uint16_t nLocalPort)
{
    std::memset(pTcb, 0, sizeof(struct tcb));

    pTcb->nLocalPort = nLocalPort;

    pTcb->ISS = hal::Millis();

    pTcb->RCV.WND = TCP_MAX_RX_WND;

    pTcb->SND.UNA = pTcb->ISS;
    pTcb->SND.NXT = pTcb->ISS;
    pTcb->SND.WL2 = pTcb->ISS;

    NEW_STATE(pTcb, STATE_LISTEN);
}

__attribute__((cold)) void Init()
{
    DEBUG_ENTRY();

    /* Ethernet */
    std::memcpy(s_tcp.ether.src, netif::globals::netif_default.hwaddr, ETH_ADDR_LEN);
    s_tcp.ether.type = __builtin_bswap16(ETHER_TYPE_IPv4);
    /* IPv4 */
    s_tcp.ip4.ver_ihl = 0x45;
    s_tcp.ip4.tos = 0;
    s_tcp.ip4.flags_froff = __builtin_bswap16(IPv4_FLAG_DF);
    s_tcp.ip4.ttl = 64;
    s_tcp.ip4.proto = IPv4_PROTO_TCP;

    DEBUG_EXIT();
}

void Shutdown()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

/* TCP Checksum Pseudo Header */
struct tcpPseudo
{
    uint8_t srcIp[IPv4_ADDR_LEN];
    uint8_t dstIp[IPv4_ADDR_LEN];
    uint8_t zero;
    uint8_t proto;
    uint16_t length;
};

static constexpr uint32_t kTcpPseudoLen = 12;

static uint16_t TcpChecksumPseudoHeader(struct t_tcp* pTcp, const struct tcb* pTcb, uint16_t length)
{
    uint8_t buf[kTcpPseudoLen];
    // Store current data before TCP header in temporary buffer
    auto* pseu = reinterpret_cast<struct tcpPseudo*>(reinterpret_cast<uint8_t*>(&pTcp->tcp) - kTcpPseudoLen);
    memcpy(buf, pseu, kTcpPseudoLen);

    // Generate TCP psuedo header
    std::memcpy(pseu->srcIp, pTcb->local_ip, IPv4_ADDR_LEN);
    std::memcpy(pseu->dstIp, pTcb->remote_ip, IPv4_ADDR_LEN);
    pseu->zero = 0;
    pseu->proto = IPv4_PROTO_TCP;
    pseu->length = __builtin_bswap16(length);

    const auto nSum = Chksum(pseu, length + kTcpPseudoLen);

    // Restore data before TCP header from temporary buffer
    memcpy(pseu, buf, kTcpPseudoLen);

    return nSum;
}

static void TcpSendSegment(struct tcb* pTcb, const struct SendInfo& send_info)
{
    pTcb->did_send_ack_or_data = true;

    uint32_t data_offset = 5; /*  Data Offset:  4 bits
    The number of 32 bit words in the TCP Header.  This indicates where
    the data begins.  The TCP header (even one including options) is an
    integral number of 32 bits long. */
    assert(data_offset * 4 == TCP_HEADER_SIZE);

    if (send_info.CTL & Control::SYN)
    {
        data_offset++;
    }

    data_offset += 3; // Option::KIND_TIMESTAMP

    const auto kHeaderLength = data_offset * 4;
    const auto kTcpLength = kHeaderLength + pTcb->TX.size;

    /* Ethernet */
    std::memcpy(s_tcp.ether.dst, pTcb->remoteEthAddr, ETH_ADDR_LEN);
    /* IPv4 */
    s_tcp.ip4.id = s_id++;
    s_tcp.ip4.len = __builtin_bswap16(static_cast<uint16_t>(kTcpLength + sizeof(struct ip4_header)));
    std::memcpy(s_tcp.ip4.src, pTcb->local_ip, IPv4_ADDR_LEN);
    std::memcpy(s_tcp.ip4.dst, pTcb->remote_ip, IPv4_ADDR_LEN);
    s_tcp.ip4.chksum = 0;
#if !defined(CHECKSUM_BY_HARDWARE)
    s_tcp.ip4.chksum = Chksum(reinterpret_cast<void*>(&s_tcp.ip4), 20);
#endif
    // TCP
    s_tcp.tcp.srcpt = pTcb->nLocalPort;
    s_tcp.tcp.dstpt = pTcb->nRemotePort;
    s_tcp.tcp.seqnum = send_info.SEQ;
    s_tcp.tcp.acknum = send_info.ACK;
    s_tcp.tcp.offset = static_cast<uint8_t>(data_offset << 4);
    s_tcp.tcp.control = send_info.CTL;
    s_tcp.tcp.window = pTcb->RCV.WND;
    s_tcp.tcp.urgent = pTcb->SND.UP;
    s_tcp.tcp.checksum = 0;

    auto* data = reinterpret_cast<uint8_t*>(&s_tcp.tcp.data);

    /* Add options */
    if (send_info.CTL & Control::SYN)
    {
        *data++ = Option::kKindMss;
        *data++ = kOptionMssLength;
        *(reinterpret_cast<uint16_t*>(data)) = __builtin_bswap16(TCP_RX_MSS);
        data += 2;
    }

    *data++ = Option::kKindNop;
    *data++ = Option::kKindNop;
    *data++ = Option::kKindTimestamp;
    *data++ = 10;
    const auto kMillis = __builtin_bswap32(hal::Millis());
    memcpy(data, &kMillis, 4);
    data += 4;
    memcpy(data, &pTcb->TS.recent, 4);
    data += 4;

    DEBUG_PRINTF("SEQ=%u, ACK=%u, kTcpLength=%u, data_offset=%u, p_tcb->TX.size=%u", s_tcp.tcp.seqnum, s_tcp.tcp.acknum, kTcpLength, data_offset,
                 pTcb->TX.size);

    if (pTcb->TX.data != nullptr)
    {
        for (uint32_t i = 0; i < pTcb->TX.size; i++)
        {
            *data++ = pTcb->TX.data[i];
        }
    }

    s_tcp.tcp.srcpt = __builtin_bswap16(s_tcp.tcp.srcpt);
    s_tcp.tcp.dstpt = __builtin_bswap16(s_tcp.tcp.dstpt);
    TcpBswap32AcknumSeqnum(&s_tcp);
    s_tcp.tcp.window = __builtin_bswap16(s_tcp.tcp.window);
    s_tcp.tcp.urgent = __builtin_bswap16(s_tcp.tcp.urgent);

    s_tcp.tcp.checksum = TcpChecksumPseudoHeader(&s_tcp, pTcb, static_cast<uint16_t>(kTcpLength));

    emac_eth_send(reinterpret_cast<void*>(&s_tcp), kTcpLength + sizeof(struct ip4_header) + sizeof(struct ether_header));
}

static void SendReset(struct t_tcp* pTcp, struct tcb* pTcb)
{
    DEBUG_ENTRY();

    if (pTcp->tcp.control & Control::RST)
    {
        DEBUG_EXIT();
        return;
    }

    struct SendInfo info;
    info.CTL = Control::RST;

    if (pTcp->tcp.control & Control::ACK)
    {
        info.SEQ = TcpGetAcknum(pTcp);
    }
    else
    {
        info.SEQ = 0;
        info.CTL |= Control::ACK;
    }

    uint32_t data_length = 0;

    if (pTcp->tcp.control & Control::SYN)
    {
        data_length++;
    }

    if (pTcp->tcp.control & Control::FIN)
    {
        data_length++;
    }

    info.ACK = TcpGetSeqnum(pTcp) + data_length;

    TcpSendSegment(pTcb, info);

    DEBUG_EXIT();
}

static bool SendData(struct tcb* pTCB, const uint8_t* buffer, uint32_t length, bool is_last_segment)
{
    assert(length != 0);
    assert(length <= static_cast<uint32_t>(TCP_DATA_SIZE));
    assert(length <= pTCB->SND.WND);

    DEBUG_PRINTF("length=%u, pTCB->SND.WND=%u", length, pTCB->SND.WND);

    pTCB->TX.data = const_cast<uint8_t*>(buffer);
    pTCB->TX.size = length;

    struct SendInfo info;
    info.SEQ = pTCB->SND.NXT;
    info.ACK = pTCB->RCV.NXT;
    info.CTL = Control::ACK;
    if (is_last_segment)
    {
        info.CTL |= Control::PSH;
    }

    TcpSendSegment(pTCB, info);

    pTCB->TX.data = nullptr;
    pTCB->TX.size = 0;

    pTCB->SND.NXT += length;
    pTCB->SND.WND -= length;

    return false;
}

struct Options
{
    uint8_t kind;
    uint8_t length;
    uint8_t data;
};

static void TcpScanOptions(struct t_tcp* pTcp, struct tcb* pTcb, int32_t nDataOffset)
{
    const auto* const kTcpHeaderEnd = reinterpret_cast<uint8_t*>(&pTcp->tcp) + nDataOffset;

    auto* options = reinterpret_cast<struct Options*>(pTcp->tcp.data);

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
                    auto nMSS = (p[0] << 8) + p[1];
                    // RFC 1122 section 4.2.2.6
                    nMSS = std::min(static_cast<int32_t>(nMSS + 20), static_cast<int32_t>(TCP_TX_MSS)) - TCP_HEADER_SIZE; // - IP_OPTION_SIZE;
                    pTcb->SendMSS = static_cast<uint16_t>(nMSS);
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
                    if (pTcp->tcp.control & Control::SYN)
                    {
                        pTcb->TS.recent = tsval.u32;
#ifndef NDEBUG
                        bIgnore = false;
#endif
                    }
                    else if ((__builtin_bswap32(tsval.u32) > __builtin_bswap32(pTcb->TS.recent)))
                    { // TODO(a)
                        pTcb->TS.recent = tsval.u32;
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

__attribute__((hot)) void Run()
{
    for (auto& port : s_Ports)
    {
        for (auto& tcb : port.TCB)
        {
            if (tcb.state == STATE_CLOSE_WAIT)
            {
                DEBUG_PRINTF(":%u", tcb.nRemotePort);
                SendInfo info;
                info.SEQ = tcb.SND.NXT;
                info.ACK = tcb.RCV.NXT;
                info.CTL = Control::FIN | Control::ACK;

                TcpSendSegment(&tcb, info);

                NEW_STATE(&tcb, STATE_LAST_ACK);

                tcb.SND.NXT++;
            }
        }
#if defined(TCP_TX_QUEUE_SIZE)
        auto& transmission_queue = port.transmissionQueue;
        auto& data_segment_queue = transmission_queue.dataSegmentQueue;

        while (!data_segment_queue.IsEmpty() && data_segment_queue.GetFront().length <= transmission_queue.pTcb->SND.WND)
        {
            const auto& segment = data_segment_queue.GetFront();
            SendData(transmission_queue.pTcb, segment.buffer, segment.length, segment.is_last_segment);
            data_segment_queue.Pop();
        }
#endif
    }
}

static bool FindActiveTcb(const t_tcp* pTcp, uint32_t index_port, uint32_t& nIndexTCB)
{
    for (nIndexTCB = 0; nIndexTCB < TCP_MAX_TCBS_ALLOWED; nIndexTCB++)
    {
        const auto* pTCB = &s_Ports[index_port].TCB[nIndexTCB];

        if (pTCB->state == STATE_LISTEN)
        {
            continue;
        }

        if (pTCB->nRemotePort == pTcp->tcp.srcpt && memcmp(pTCB->remote_ip, pTcp->ip4.src, IPv4_ADDR_LEN) == 0)
        {
            return true;
        }
    }

    return false;
}

static bool FindListeningTcb(uint32_t index_port, uint32_t& nIndexTCB)
{
    for (nIndexTCB = 0; nIndexTCB < TCP_MAX_TCBS_ALLOWED; nIndexTCB++)
    {
        const auto* pTCB = &s_Ports[index_port].TCB[nIndexTCB];

        if (pTCB->state == STATE_LISTEN)
        {
            DEBUG_PUTS("pTCB->state == STATE_LISTEN");
            return true;
        }
    }

    return false;
}

static void FindTcb(const t_tcp* pTcp, uint32_t& nIndexPort, uint32_t& nIndexTCB)
{
    DEBUG_ENTRY();
    // Search each port for a match with the destination port
    for (nIndexPort = 0; nIndexPort < TCP_MAX_PORTS_ALLOWED; nIndexPort++)
    {
        // Search for an existing active TCB matching the source IP and port
        if (FindActiveTcb(pTcp, nIndexPort, nIndexTCB))
        {
            DEBUG_EXIT();
            return;
        }

        // If no matching TCB, find an available TCB in listening state
        if (FindListeningTcb(nIndexPort, nIndexTCB))
        {
            DEBUG_EXIT();
            return;
        }

        DEBUG_PUTS("If no available TCB, trigger retransmission");
        DEBUG_EXIT();
        return;
    }
}

/**
 * https://www.rfc-editor.org/rfc/rfc9293.html#name-segment-arrives
 */
__attribute__((hot)) void Input(struct t_tcp* pTcp)
{
    pTcp->tcp.srcpt = __builtin_bswap16(pTcp->tcp.srcpt);
    pTcp->tcp.dstpt = __builtin_bswap16(pTcp->tcp.dstpt);

    DEBUG_PRINTF(IPSTR ":%d[%d]", pTcp->ip4.src[0], pTcp->ip4.src[1], pTcp->ip4.src[2], pTcp->ip4.src[3], pTcp->tcp.dstpt, pTcp->tcp.srcpt);

    // Special case: immediately reject connections to port 443
    if (pTcp->tcp.dstpt == 443 && (pTcp->tcp.control & Control::SYN))
    {
        struct tcb TCB;
        std::memset(&TCB, 0, sizeof(struct tcb));

        TCB.nLocalPort = pTcp->tcp.dstpt;
        std::memcpy(TCB.local_ip, pTcp->ip4.dst, IPv4_ADDR_LEN);

        TCB.nRemotePort = pTcp->tcp.srcpt;
        std::memcpy(TCB.remote_ip, pTcp->ip4.src, IPv4_ADDR_LEN);
        std::memcpy(TCB.remoteEthAddr, pTcp->ether.src, ETH_ADDR_LEN);

        TcpBswap32AcknumSeqnum(pTcp);

        SendReset(pTcp, &TCB);
        DEBUG_PUTS("Rejected HTTPS port 443 with RST");
        DEBUG_EXIT();
        return;
    }

    uint32_t index_port = 0;

    for (index_port = 0; index_port < TCP_MAX_PORTS_ALLOWED; index_port++)
    {
        if (s_Ports[index_port].nLocalPort == pTcp->tcp.dstpt)
        {
            break;
        }
    }

    uint32_t index_tcb = 0;

    if (index_port != TCP_MAX_PORTS_ALLOWED)
    {
        FindTcb(pTcp, index_port, index_tcb);
    }

    DEBUG_PRINTF("nIndexPort=%u, nIndexTCB=%u", index_port, index_tcb);

    const auto kDataOffset = offset2octets(pTcp->tcp.offset);

    // https://www.rfc-editor.org/rfc/rfc9293.html#name-closed-state
    // CLOSED (i.e., TCB does not exist)
    if (index_port == TCP_MAX_PORTS_ALLOWED)
    {
        struct tcb TCB;

        std::memset(&TCB, 0, sizeof(struct tcb));

        TCB.nLocalPort = pTcp->tcp.dstpt;
        std::memcpy(TCB.local_ip, pTcp->ip4.dst, IPv4_ADDR_LEN);

        TCB.nRemotePort = pTcp->tcp.srcpt;
        std::memcpy(TCB.remote_ip, pTcp->ip4.src, IPv4_ADDR_LEN);
        std::memcpy(TCB.remoteEthAddr, pTcp->ether.src, ETH_ADDR_LEN);

        TcpBswap32AcknumSeqnum(pTcp);

        TcpScanOptions(pTcp, &TCB, kDataOffset);
        SendReset(pTcp, &TCB);

        DEBUG_PUTS("TCP_MAX_PORTS_ALLOWED");
        DEBUG_EXIT();
        return;
    }

    const auto kTcplen = static_cast<uint16_t>(__builtin_bswap16(pTcp->ip4.len) - sizeof(struct ip4_header));
    const auto kDataLength = static_cast<uint16_t>(kTcplen - kDataOffset);

    TcpBswap32AcknumSeqnum(pTcp);
    pTcp->tcp.window = __builtin_bswap16(pTcp->tcp.window);
    pTcp->tcp.urgent = __builtin_bswap16(pTcp->tcp.urgent);

    const auto SEG_LEN = kDataLength;        // NOLINT
    const auto SEG_ACK = TcpGetAcknum(pTcp); // NOLINT
    const auto SEG_SEQ = TcpGetSeqnum(pTcp); // NOLINT
    const auto SEG_WND = pTcp->tcp.window;   // NOLINT

    auto* pTCB = &s_Ports[index_port].TCB[index_tcb];

    DEBUG_PRINTF("%u:%u:[%s] %c%c%c%c%c%c SEQ=%u, ACK=%u, tcplen=%u, data_offset=%u, data_length=%u", index_port, index_tcb, s_aStateName[pTCB->state],
                 pTcp->tcp.control & Control::URG ? 'U' : '-', pTcp->tcp.control & Control::ACK ? 'A' : '-', pTcp->tcp.control & Control::PSH ? 'P' : '-',
                 pTcp->tcp.control & Control::RST ? 'R' : '-', pTcp->tcp.control & Control::SYN ? 'S' : '-', pTcp->tcp.control & Control::FIN ? 'F' : '-',
                 SEG_SEQ, SEG_ACK, kTcplen, kDataOffset, kDataLength);

    TcpScanOptions(pTcp, pTCB, kDataOffset);

    // https://www.rfc-editor.org/rfc/rfc9293.html#name-listen-state
    if (pTCB->state == STATE_LISTEN)
    {
        std::memcpy(pTCB->local_ip, pTcp->ip4.dst, IPv4_ADDR_LEN);

        pTCB->nRemotePort = pTcp->tcp.srcpt;
        std::memcpy(pTCB->remote_ip, pTcp->ip4.src, IPv4_ADDR_LEN);
        std::memcpy(pTCB->remoteEthAddr, pTcp->ether.src, ETH_ADDR_LEN);

        // First, check for a RST
        // An incoming RST should be ignored.
        if (pTcp->tcp.control & Control::RST)
        {
            DEBUG_EXIT();
            return;
        }

        // Second, check for an ACK
        // Any acknowledgment is bad if it arrives on a connection still in the LISTEN state.
        // RST -> <SEQ=SEG.ACK><CTL=RST>
        if (pTcp->tcp.control & Control::ACK)
        {
            SendReset(pTcp, pTCB);

            DEBUG_PUTS("pTcp->tcp.control & Control::ACK");
            DEBUG_EXIT();
            return;
        }

        // Third, check for a SYN
        // We skip security check
        if (pTcp->tcp.control & Control::SYN)
        {
            // Set RCV.NXT to SEG.SEQ+1, IRS is set to SEG.SEQ
            pTCB->RCV.NXT = SEG_SEQ + 1;
            pTCB->IRS = SEG_SEQ;

            // <SEQ=ISS><ACK=RCV.NXT><CTL=SYN,ACK>
            SendInfo send_info;
            send_info.SEQ = pTCB->ISS;
            send_info.ACK = pTCB->RCV.NXT;
            send_info.CTL = Control::SYN | Control::ACK;
            TcpSendSegment(pTCB, send_info);

            // SND.NXT is set to ISS+1 and SND.UNA to ISS. The connection state should be changed to SYN-RECEIVED.
            pTCB->SND.NXT = pTCB->ISS + 1;
            pTCB->SND.UNA = pTCB->ISS;

            NEW_STATE(pTCB, STATE_SYN_RECEIVED);
            DEBUG_EXIT();
            return;
        }

        // Fourth, other data or control
        DEBUG_PUTS("This should not be reached.");
        DEBUG_EXIT();
        return;
    }

    // We skip SYN-SENT STATE as we are server only.

    ///< https://www.rfc-editor.org/rfc/rfc9293.html#name-other-states
    switch (pTCB->state)
    {
        case STATE_SYN_RECEIVED:
        case STATE_ESTABLISHED:
        case STATE_FIN_WAIT_1:
        case STATE_FIN_WAIT_2:
        case STATE_CLOSE_WAIT:
        case STATE_CLOSING:
        case STATE_LAST_ACK:
        case STATE_TIME_WAIT:
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
            auto is_acceptable = false;

            DEBUG_PRINTF("RCV.WND=%u, SEG_LEN=%u, RCV.NXT=%u, SEG_SEQ=%u", pTCB->RCV.WND, SEG_LEN, pTCB->RCV.NXT, SEG_SEQ);

            if (pTCB->RCV.WND > 0)
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
                    if (SeqBetweenL(pTCB->RCV.NXT, SEG_SEQ, pTCB->RCV.NXT + pTCB->RCV.WND))
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
                    if (SeqBetweenL(pTCB->RCV.NXT, SEG_SEQ, pTCB->RCV.NXT + pTCB->RCV.WND) ||
                        SeqBetweenL(pTCB->RCV.NXT, SEG_SEQ + SEG_LEN - 1, pTCB->RCV.NXT + pTCB->RCV.WND))
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
                    if (SEG_SEQ == pTCB->RCV.NXT)
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
                if (pTcp->tcp.control & Control::RST)
                {
                    TcpInitTcb(pTCB, pTCB->nLocalPort);
                    DEBUG_EXIT();
                    return;
                }

                SendInfo send_info;
                send_info.SEQ = pTCB->SND.NXT;
                send_info.ACK = pTCB->RCV.NXT;
                send_info.CTL = Control::ACK;
                TcpSendSegment(pTCB, send_info);

                DEBUG_EXIT();
                return;
            }

            // second check the RST bit, *//* Page 70 */
            if (pTcp->tcp.control & Control::RST)
            {
                switch (pTCB->state)
                {
                    case STATE_SYN_RECEIVED:
                        TcpInitTcb(pTCB, pTCB->nLocalPort);
                        break;
                    case STATE_ESTABLISHED:
                    case STATE_FIN_WAIT_1:
                    case STATE_FIN_WAIT_2:
                    case STATE_CLOSE_WAIT:
                        /* If the RST bit is set then, any outstanding RECEIVEs and SEND
                         * should receive "reset" responses.  All segment queues should be
                         * flushed.  Users should also receive an unsolicited general
                         * "connection reset" signal.  Enter the CLOSED state, delete the
                         * TCB, and return. */
                        TcpInitTcb(pTCB, pTCB->nLocalPort);
                        break;
                    case STATE_CLOSING:
                    case STATE_LAST_ACK:
                    case STATE_TIME_WAIT:
                        /* If the RST bit is set then, enter the CLOSED state, delete the
                         * TCB, and return. */
                        TcpInitTcb(pTCB, pTCB->nLocalPort);
                        break;
                    default:
                        assert(0);
                        break;
                }

                DEBUG_EXIT();
                return;
            }

            // third check security and precedence. No code needed here

            /* fourth, check the SYN bit, */ /* Page 71 */
            if (pTcp->tcp.control & Control::SYN)
            {
                // RFC 1122 section 4.2.2.20 (e)
                if (pTCB->state == STATE_SYN_RECEIVED)
                {
                    TcpInitTcb(pTCB, pTCB->nLocalPort);
                    DEBUG_EXIT();
                    return;
                }

                SendReset(pTcp, pTCB);

                DEBUG_PUTS("pTcp->tcp.control & Control::SYN");
            }

            /*  fifth check the ACK field, */ /* Page 72 */
            if (!(pTcp->tcp.control & Control::ACK))
            {
                // if the ACK bit is off drop the segment and return
                DEBUG_EXIT();
                return;
            }

            switch (pTCB->state)
            {
                case STATE_SYN_RECEIVED:
                    /*  If SND.UNA =< SEG.ACK =< SND.NXT then enter ESTABLISHED state
                     *and continue processing. */
                    if (SeqBetweenLh(pTCB->SND.UNA, SEG_ACK, pTCB->SND.NXT))
                    {
                        // RFC 1122 section 4.2.2.20 (f)
                        pTCB->SND.WND = SEG_WND;
                        pTCB->SND.WL1 = SEG_SEQ;
                        pTCB->SND.WL2 = SEG_ACK;

                        pTCB->SND.UNA = SEG_ACK; // got ACK for SYN

                        NEW_STATE(pTCB, STATE_ESTABLISHED);
                        DEBUG_EXIT();
                        return;
                    }
                    else
                    {
                        // <SEQ=SEG.ACK><CTL=RST>
                        SendReset(pTcp, pTCB);

                        DEBUG_PUTS("<SEQ=SEG.ACK><CTL=RST>");
                    }
                    break;
                case STATE_ESTABLISHED:
                case STATE_FIN_WAIT_1:
                case STATE_FIN_WAIT_2:
                case STATE_CLOSE_WAIT:
                case STATE_CLOSING:
                    DEBUG_PRINTF("SND.UNA=%u, SEG_ACK=%u, SND.NXT=%u", pTCB->SND.UNA, SEG_ACK, pTCB->SND.NXT);

                    if (SeqBetweenH(pTCB->SND.UNA, SEG_ACK, pTCB->SND.NXT))
                    {
                        auto bytes_ack = SEG_ACK - pTCB->SND.UNA;
                        pTCB->SND.UNA = SEG_ACK;

                        if (SEG_ACK == pTCB->SND.NXT)
                        {
                            DEBUG_PUTS("/* all segments are acknowledged */");
                        }

                        if (pTCB->state == STATE_FIN_WAIT_1 || pTCB->state == STATE_CLOSING)
                        {
                            bytes_ack--;
                            DEBUG_PUTS("/* acknowledged FIN does not count */");
                        }

                        if (pTCB->state == STATE_ESTABLISHED && bytes_ack == 1)
                        {
                            bytes_ack--;
                        }

                        // update send window
                        if (SeqLt(pTCB->SND.WL1, SEG_SEQ) || (pTCB->SND.WL1 == SEG_SEQ && SeqLeq(pTCB->SND.WL2, SEG_ACK)))
                        {
                            pTCB->SND.WND = SEG_WND;
                            pTCB->SND.WL1 = SEG_SEQ;
                            pTCB->SND.WL2 = SEG_ACK;
                        }
                    }
                    else if (SeqLeq(SEG_ACK, pTCB->SND.UNA))
                    { /* RFC 1122 section 4.2.2.20 (g) */
                        DEBUG_PUTS("/* ignore duplicate ACK */");
                        if (SeqBetweenLh(pTCB->SND.UNA, SEG_ACK, pTCB->SND.NXT))
                        {
                            // ... but update send window
                            if (SeqLt(pTCB->SND.WL1, SEG_SEQ) || (pTCB->SND.WL1 == SEG_SEQ && SeqLeq(pTCB->SND.WL2, SEG_ACK)))
                            {
                                pTCB->SND.WND = SEG_WND;
                                pTCB->SND.WL1 = SEG_SEQ;
                                pTCB->SND.WL2 = SEG_ACK;
                            }
                        }
                    }
                    else if (SeqGt(SEG_ACK, pTCB->SND.NXT))
                    {
                        DEBUG_PRINTF("SEG_ACK=%u, SND.NXT=%u", SEG_ACK, pTCB->SND.NXT);

                        SendInfo send_info;
                        send_info.SEQ = pTCB->SND.NXT;
                        send_info.ACK = pTCB->RCV.NXT;
                        send_info.CTL = Control::ACK;

                        TcpSendSegment(pTCB, send_info);

                        DEBUG_EXIT();
                        return;
                    }
                    break;
                case STATE_LAST_ACK:
                    if (SEG_ACK == pTCB->SND.NXT)
                    { // if our FIN is now acknowledged
                        TcpInitTcb(pTCB, pTCB->nLocalPort);
                    }
                    break;
                case STATE_TIME_WAIT:
                    if (SEG_ACK == pTCB->SND.NXT)
                    { // if our FIN is now acknowledged
                        SendInfo send_info;
                        send_info.SEQ = pTCB->SND.NXT;
                        send_info.ACK = pTCB->RCV.NXT;
                        send_info.CTL = Control::ACK;

                        TcpSendSegment(pTCB, send_info);
                        CLIENT_NOT_IMPLEMENTED;
                    }
                    break;
                default:
                    UNEXPECTED_STATE();
                    break;
            }

            // sixth, check the URG bit. No code needed here

            // seventh, process the segment text
            switch (pTCB->state)
            {
                case STATE_ESTABLISHED:
                case STATE_FIN_WAIT_1:
                case STATE_FIN_WAIT_2:
                    if (kDataLength > 0)
                    {
                        if (SEG_SEQ == pTCB->RCV.NXT)
                        {
                            assert(s_Ports[index_port].callback != nullptr);
                            // Update sequence and window
                            pTCB->RCV.NXT += kDataLength;
                            pTCB->RCV.WND -= kDataLength;

                            pTCB->did_send_ack_or_data = false;
                            s_Ports[index_port].callback(index_tcb, reinterpret_cast<uint8_t*>(&pTcp->tcp) + kDataOffset, kDataLength);

                            if (!pTCB->did_send_ack_or_data)
                            {
                                // Send acknowledgment
                                SendInfo send_info;
                                send_info.SEQ = pTCB->SND.NXT;
                                send_info.ACK = pTCB->RCV.NXT;
                                send_info.CTL = Control::ACK;
                                TcpSendSegment(pTCB, send_info);
                            }
                        }
                        else
                        {
                            SendInfo send_info;
                            send_info.SEQ = pTCB->SND.NXT;
                            send_info.ACK = pTCB->RCV.NXT;
                            send_info.CTL = Control::ACK;

                            TcpSendSegment(pTCB, send_info);

                            DEBUG_PUTS("Out of order");
                            DEBUG_EXIT();
                            return;
                        }
                    }
                    break;
                default:
                    break;
            }

            /* eighth, check the FIN bit, */ /* Page 75 */
            /*
             Do not process the FIN if the state is CLOSED, LISTEN or SYN-SENT
             since the SEG.SEQ cannot be validated; drop the segment and return.
             */

            if ((pTCB->state == STATE_CLOSED) || (pTCB->state == STATE_LISTEN) || (pTCB->state == STATE_SYN_SENT))
            {
                DEBUG_EXIT();
                return;
            }

            if (!(pTcp->tcp.control & Control::FIN))
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

            pTCB->RCV.NXT = pTCB->RCV.NXT + 1;

            SendInfo send_info;
            send_info.SEQ = pTCB->SND.NXT;
            send_info.ACK = pTCB->RCV.NXT;
            send_info.CTL = Control::ACK;

            TcpSendSegment(pTCB, send_info);

            switch (pTCB->state)
            {
                case STATE_SYN_RECEIVED:
                case STATE_ESTABLISHED:
                    NEW_STATE(pTCB, STATE_CLOSE_WAIT);
                    break;
                case STATE_FIN_WAIT_1:
                    /*
                     If our FIN has been ACKed (perhaps in this segment), then
                     enter TIME-WAIT, start the time-wait timer, turn off the other
                     timers; otherwise enter the CLOSING state.
                     */
                    if (SEG_ACK == pTCB->SND.NXT)
                    { /* if our FIN is now acknowledged */
                        NEW_STATE(pTCB, STATE_TIME_WAIT);
                        CLIENT_NOT_IMPLEMENTED;
                    }
                    else
                    {
                        NEW_STATE(pTCB, STATE_CLOSING);
                    }
                    break;
                case STATE_FIN_WAIT_2:
                    // Enter the TIME-WAIT state.  Start the time-wait timer, turn off the other timers.
                    NEW_STATE(pTCB, STATE_TIME_WAIT);
                    CLIENT_NOT_IMPLEMENTED;
                    break;
                case STATE_CLOSE_WAIT:
                    // Remain in the CLOSE-WAIT state.
                case STATE_CLOSING:
                    // Remain in the CLOSING state.
                case STATE_LAST_ACK:
                    // Remain in the LAST-ACK state.
                    break;
                case STATE_TIME_WAIT:
                    CLIENT_NOT_IMPLEMENTED;
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

// --> Public API's

int32_t Begin(uint16_t local_port, TcpCallbackFunctionPtr callback)
{
    DEBUG_PRINTF("local_port=%u", local_port);

    for (int32_t i = 0; i < TCP_MAX_PORTS_ALLOWED; i++)
    {
        if (s_Ports[i].nLocalPort == local_port)
        {
            return i;
        }

        if (s_Ports[i].nLocalPort == 0)
        {
            s_Ports[i].callback = callback;
            s_Ports[i].nLocalPort = local_port;

            for (uint32_t index_tcb = 0; index_tcb < TCP_MAX_TCBS_ALLOWED; index_tcb++)
            {
                // create transmission control block's (TCB)
                TcpInitTcb(&s_Ports[i].TCB[index_tcb], local_port);
            }

#if defined(TCP_TX_QUEUE_SIZE)
            s_Ports[i].transmissionQueue.pTcb = nullptr;
#endif
            DEBUG_PRINTF("i=%d, local_port=%d[%x]", i, local_port, local_port);
            return i;
        }
    }

#ifndef NDEBUG
    console::Error("tcp::Begin");
#endif
    return -1;
}

int32_t End([[maybe_unused]] int32_t handle)
{
    assert(0);
    return 0;
}

void Write(int32_t handle_listen, const uint8_t* buffer, uint32_t length, uint32_t handle_connection)
{
    assert(handle_listen >= 0);
    assert(handle_listen < TCP_MAX_PORTS_ALLOWED);
    assert(buffer != nullptr);
    assert(handle_connection < TCP_MAX_TCBS_ALLOWED);

    DEBUG_ENTRY();
    DEBUG_PRINTF("%u:%u -> %u", handle_listen, handle_connection, length);
    debug::Dump(buffer, length > 16 ? 16 : length);

    auto* pTCB = &s_Ports[handle_listen].TCB[handle_connection];
    assert(pTCB != nullptr);

    const auto* p = buffer;

    while ((length > 0) && (length <= pTCB->SND.WND))
    {
        const auto kWriteLength = (length > TCP_DATA_SIZE) ? TCP_DATA_SIZE : length;
        const bool kIsLastSegment = (length < TCP_DATA_SIZE);

        SendData(pTCB, p, kWriteLength, kIsLastSegment);

        p += kWriteLength;
        length -= kWriteLength;
    }

    if (length > 0)
    {
#if defined(TCP_TX_QUEUE_SIZE)
        auto& transmission_queue = s_Ports[handle_listen].transmissionQueue;
        auto& data_segment_queue = transmission_queue.dataSegmentQueue;
        assert(data_segment_queue.IsEmpty());

        transmission_queue.pTcb = pTCB;

        while (length > 0)
        {
            assert(!data_segment_queue.IsFull());
            const auto kWriteLength = (length > TCP_DATA_SIZE) ? TCP_DATA_SIZE : length;
            const bool kIsLastSegment = (length < TCP_DATA_SIZE);
            data_segment_queue.Push(p, kWriteLength, kIsLastSegment);
            p += kWriteLength;
            length -= kWriteLength;
        }
#else
        assert(0);
#endif
    }

    DEBUG_EXIT();
}

void Abort(int32_t handle_listen, uint32_t handle_connection)
{
    assert(handle_listen >= 0);
    assert(handle_listen < TCP_MAX_PORTS_ALLOWED);
    assert(handle_connection < TCP_MAX_TCBS_ALLOWED);

    auto* tcb = &s_Ports[handle_listen].TCB[handle_connection];
    assert(tcb != nullptr);

    struct SendInfo info;
    info.CTL = Control::RST;
    info.SEQ = tcb->SND.NXT;
    info.ACK = tcb->RCV.NXT;

    TcpSendSegment(tcb, info);
}

} // namespace net::tcp
// <---
