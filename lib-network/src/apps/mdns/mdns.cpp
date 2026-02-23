/**
 * @file mdns.cpp
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_NET_APPS_MDNS)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <cassert>

#include "network.h"
#include "apps/mdns.h"
#include "core/protocol/ip4.h"
#include "core/protocol/dns.h"
#include "core/protocol/iana.h"
#include "firmware/debug/debug_debug.h"

namespace network::apps::mdns
{
#if !defined(MDNS_SERVICE_RECORDS_MAX)
static constexpr auto kServiceRecordsMax = 8;
#else
static constexpr uint32_t kServiceRecordsMax = MDNS_SERVICE_RECORDS_MAX;
#endif

static constexpr size_t kDomainMaxlen = 256;
static constexpr size_t kLabelMaxlen = 63;
static constexpr size_t kTxtMaxlen = 256;

static constexpr char kDomainLocal[] = {5, 'l', 'o', 'c', 'a', 'l', 0};
#if defined(CONFIG_MDNS_DOMAIN_REVERSE)
static constexpr char kDomainReverse[] = {7, 'i', 'n', '-', 'a', 'd', 'd', 'r', 4, 'a', 'r', 'p', 'a', 0};
#endif
static constexpr char kDomainUdp[] = {4, '_', 'u', 'd', 'p'};
static constexpr char kDomainTcp[] = {4, '_', 't', 'c', 'p'};
static constexpr char kDomainConfig[] = {7, '_', 'c', 'o', 'n', 'f', 'i', 'g'};
static constexpr char kDomainTftp[] = {5, '_', 't', 'f', 't', 'p'};
static constexpr char kDomainHttp[] = {5, '_', 'h', 't', 't', 'p'};
static constexpr char kDomainRdmnetLlrp[] = {12, '_', 'r', 'd', 'm', 'n', 'e', 't', '-', 'l', 'l', 'r', 'p'};
static constexpr char kDomainNtp[] = {4, '_', 'n', 't', 'p'};
static constexpr char kDomainMidi[] = {11, '_', 'a', 'p', 'p', 'l', 'e', '-', 'm', 'i', 'd', 'i'};
static constexpr char kDomainOsc[] = {4, '_', 'o', 's', 'c'};
static constexpr char kDomainDdp[] = {4, '_', 'd', 'd', 'p'};
static constexpr char kDomainPp[] = {3, '_', 'p', 'p'};

struct HostReply
{
    static constexpr uint32_t kA = 0x01;
    static constexpr uint32_t kPtr = 0x02;
};

struct ServiceReply
{
    static constexpr uint32_t kTypePtr = 0x10;
    static constexpr uint32_t kNamePtr = 0x20;
    static constexpr uint32_t kSrv = 0x40;
    static constexpr uint32_t kTxt = 0x80;
};

enum class OpCodes
{
    kQuery = 0,
    kIQuery = 1,
    kStatus = 2,
    kNotify = 4,
    kUpdate = 5
};

enum class Protocols
{
    kUdp,
    kTcp
};

struct Service
{
    const char* domain;
    const uint16_t kLength;
    const Protocols kProtocols;
    const uint16_t kPortDefault;
};

static constexpr Service kServices[]{{kDomainConfig, sizeof(kDomainConfig), Protocols::kUdp, 0x2905},
                                     {kDomainTftp, sizeof(kDomainTftp), Protocols::kUdp, 69},
                                     {kDomainHttp, sizeof(kDomainHttp), Protocols::kTcp, 80},
                                     {kDomainRdmnetLlrp, sizeof(kDomainRdmnetLlrp), Protocols::kUdp, 5569},
                                     {kDomainNtp, sizeof(kDomainNtp), Protocols::kUdp, 123},
                                     {kDomainMidi, sizeof(kDomainMidi), Protocols::kUdp, 5004},
                                     {kDomainOsc, sizeof(kDomainOsc), Protocols::kUdp, 0},
                                     {kDomainDdp, sizeof(kDomainDdp), Protocols::kUdp, 4048},
                                     {kDomainPp, sizeof(kDomainPp), Protocols::kUdp, 5078}};

struct Domain
{
    uint8_t a_name[kDomainMaxlen];
    uint16_t length;

    void AddLabel(const char* label, size_t label_length)
    {
        assert(!(label_length > kLabelMaxlen));
        assert(!(label_length > 0 && (1 + label_length + length >= kDomainMaxlen)));
        assert(!(label_length == 0 && (1U + length > kDomainMaxlen)));

        a_name[length] = static_cast<uint8_t>(label_length);
        length++;

        memcpy(&a_name[length], label, label_length);
        length += static_cast<uint16_t>(label_length);
    }

    void AddProtocol(mdns::Protocols protocols)
    {
        if (protocols == mdns::Protocols::kUdp)
        {
            memcpy(&a_name[length], kDomainUdp, sizeof(kDomainUdp));
            length += static_cast<uint16_t>(sizeof(kDomainUdp));
            return;
        }

        memcpy(&a_name[length], kDomainTcp, sizeof(kDomainTcp));
        length += static_cast<uint16_t>(sizeof(kDomainTcp));
    }

    void AddDotLocal()
    {
        memcpy(&a_name[length], kDomainLocal, sizeof(kDomainLocal));
        length += static_cast<uint16_t>(sizeof(kDomainLocal));
    }

    void Print(bool new_line = false)
    {
        auto const* name = a_name;
        while (*name && (name < &a_name[length]))
        {
            const auto kLength = static_cast<size_t>(*name);
            name++;
            printf("%.*s.", static_cast<int>(kLength), name);
            name += kLength;
        }

        if (new_line)
        {
            putchar('\n');
        }
    }

    friend bool operator==(Domain const& domain_a, Domain const& domain_b)
    {
        if (domain_a.length != domain_b.length)
        {
            return false;
        }

        auto const* name_a = domain_a.a_name;
        auto const* name_b = domain_b.a_name;

        while (*name_a && *name_b && (name_a < &domain_a.a_name[domain_a.length]))
        {
            if (*name_a != *name_b)
            {
                return false;
            }

            auto length = static_cast<size_t>(*name_a);
            name_a++;
            name_b++;

            if (strncasecmp(reinterpret_cast<const char*>(name_a), reinterpret_cast<const char*>(name_b), length) != 0)
            {
                return false;
            }

            name_a += length;
            name_b += length;
        }

        return true;
    }
};

static constexpr Domain kDomainDnssd{{9, '_', 's', 'e', 'r', 'v', 'i', 'c', 'e', 's', 7, '_', 'd', 'n', 's', '-', 's', 'd', 4, '_', 'u', 'd', 'p', 5, 'l', 'o', 'c', 'a', 'l', 0}, 10 + 8 + 5 + 6 + 1};

static ServiceRecord s_service_records[kServiceRecordsMax];
static uint8_t s_records_data[network::dns::kMulticastMessageSize];
static uint32_t s_host_replies;
static uint32_t s_service_replies;
static int32_t s_handle;
static uint32_t s_n_remote_ip;
static uint32_t s_n_bytes_received;
static uint8_t* s_p_receive_buffer;
static uint16_t s_n_remote_port;
static bool s_is_unicast;
static bool s_is_legacy_query;

static void CreateServiceDomain(mdns::Domain& domain, ServiceRecord const& service_record, bool include_name)
{
    DEBUG_ENTRY();

    domain.length = 0;

    if (include_name)
    {
        if (service_record.name != nullptr)
        {
            domain.AddLabel(service_record.name, strlen(service_record.name));
        }
        else
        {
            domain.AddLabel(network::iface::HostName(), strlen(network::iface::HostName()));
        }
    }

    const auto kIndex = static_cast<uint32_t>(service_record.services);

    memcpy(&domain.a_name[domain.length], kServices[kIndex].domain, kServices[kIndex].kLength);
    domain.length += kServices[kIndex].kLength;

    domain.AddProtocol(kServices[kIndex].kProtocols);
    domain.AddDotLocal();

    DEBUG_EXIT();
}

static void CreateHostDomain(Domain& domain)
{
    domain.length = 0;
    domain.AddLabel(network::iface::HostName(), strlen(network::iface::HostName()));
    domain.AddDotLocal();
}

#if defined(CONFIG_MDNS_DOMAIN_REVERSE)
static void CreateReverseDomain(Domain& domain)
{
    DEBUG_ENTRY();

    domain.length = 0;
    auto primary_ip = network::GetPrimaryIp();
    const auto* const kIp = reinterpret_cast<const uint8_t*>(&primary_ip);

    for (int32_t i = ip4::kAddressLength - 1; i >= 0; i--)
    {
        char buffer[3];
        uint32_t length = 1;

        auto d = kIp[i];

        const auto kH = d / 100U;

        if (kH != 0)
        {
            length = 3;
            buffer[0] = '0' + static_cast<char>(kH);
            d -= static_cast<uint8_t>(kH * 100U);
        }

        const auto kT = d / 10U;

        if (kT != 0)
        {
            length = std::max(static_cast<uint32_t>(2), length);
        }
        else
        {
            length = std::max(static_cast<uint32_t>(1), length);
        }

        buffer[1] = '0' + static_cast<char>(kT);
        buffer[2] = '0' + static_cast<char>(d % 10U);

        domain.AddLabel(&buffer[3 - length], length);
    }

    memcpy(&domain.a_name[domain.length], kDomainReverse, sizeof(kDomainReverse));
    domain.length += static_cast<uint16_t>(sizeof(kDomainReverse));

    DEBUG_EXIT();
}
#endif

/*
 * https://opensource.apple.com/source/mDNSResponder/mDNSResponder-26.2/mDNSCore/mDNS.c.auto.html
 * mDNSlocal const mDNSu8 *FindCompressionPointer(const mDNSu8 *const base, const mDNSu8 *const end, const mDNSu8 *const domname)
 */
static uint8_t* FindCompressionPointer(const uint8_t* const kBase, const uint8_t* const kEnd, const uint8_t* const kDomname)
{
    const auto* result = kEnd - *kDomname - 1;

    while (result >= kBase)
    {
        if (result[0] == kDomname[0] && result[1] == kDomname[1])
        {
            const auto* name = kDomname;
            const auto* targ = result;

            while (targ + *name < kEnd)
            {
                // First see if this label matches
                int i;

                for (i = 0; i <= *name; i++)
                {
                    if (targ[i] != name[i])
                    {
                        break;
                    }
                }

                if (i <= *name)
                {
                    break; // If label did not match, bail out
                }

                targ += 1 + *name; // Else, did match, so advance target pointer
                name += 1 + *name; // and proceed to check next label

                if (*name == 0 && *targ == 0)
                { // If no more labels, we found a match!
                    return const_cast<uint8_t*>(result);
                }

                if (*name == 0)
                { // If no more labels to match, we failed, so bail out
                    break;
                }

                // The label matched, so now follow the pointer (if appropriate) and then see if the next label matches
                if (targ[0] < 0x40)
                {
                    continue; // If length value, continue to check next label
                }

                if (targ[0] < 0xC0)
                {
                    break; // If 40-BF, not valid
                }

                if (targ + 1 >= kEnd)
                {
                    break; // Second byte not present!
                }

                const uint8_t* pointertarget = kBase + ((static_cast<uint16_t>(targ[0] & 0x3F)) << 8) + targ[1];

                if (targ < pointertarget)
                {
                    break; // Pointertarget must point *backwards* in the packet
                }

                if (pointertarget[0] >= 0x40)
                {
                    break; // Pointertarget must point to a valid length byte
                }

                targ = pointertarget;
            }
        }

        result--; // We failed to match at this search position, so back up the tentative result pointer and try again
    }

    return nullptr;
}

/*
 * https://opensource.apple.com/source/mDNSResponder/mDNSResponder-26.2/mDNSCore/mDNS.c.auto.html
 * mDNSlocal mDNSu8 *putDomainNameAsLabels(const DNSMessage *const msg, mDNSu8 *ptr, const mDNSu8 *const limit, const domainname *const name)
 */
static uint8_t* PutDomainNameAsLabels(uint8_t* ptr, Domain const& domain)
{
    const uint8_t* const kBase = s_records_data;
    const auto* np = domain.a_name;
    uint8_t* pointer = nullptr;
    const auto* const kSearchlimit = ptr;

    while (*np)
    {
        pointer = FindCompressionPointer(kBase, kSearchlimit, np);

        if (pointer != nullptr)
        {
            auto offset = static_cast<uint16_t>(pointer - kBase);
            *ptr++ = static_cast<uint8_t>(0xC0 | (offset >> 8));
            *ptr++ = static_cast<uint8_t>(offset);
            return ptr;
        }
        else
        {
            auto len = *np++;
            *ptr++ = len;
            for (uint32_t i = 0; i < len; i++)
            {
                *ptr++ = *np++;
            }
        }
    }

    *ptr++ = 0;
    return ptr;
}

static uint8_t* AddQuestion(uint8_t* destination, const mdns::Domain& domain, network::dns::RRType type, bool do_flush)
{
    auto* dst = PutDomainNameAsLabels(destination, domain);

    *reinterpret_cast<volatile uint16_t*>(dst) = __builtin_bswap16(static_cast<uint16_t>(type));
    dst += 2;
    *reinterpret_cast<uint16_t*>(dst) = __builtin_bswap16((do_flush ? network::dns::RRClass::kFlush : 0) | network::dns::RRClass::kInternet);
    dst += 2;

    return dst;
}

static uint32_t AddAnswerSrv(mdns::ServiceRecord const& service_record, uint8_t* destination, uint32_t ttl)
{
    DEBUG_ENTRY();

    Domain domain;
    CreateServiceDomain(domain, service_record, true);

    auto* dst = AddQuestion(destination, domain, network::dns::RRType::kSrv, true);

    *reinterpret_cast<uint32_t*>(dst) = __builtin_bswap32(ttl);
    dst += 4;
    auto* lengt_pointer = dst;
    dst += 2;
    *reinterpret_cast<uint32_t*>(dst) = 0; // Priority and Weight
    dst += 4;
    *reinterpret_cast<uint16_t*>(dst) = service_record.port;
    dst += 2;
    auto* begin = dst;

    CreateHostDomain(domain);
    dst = PutDomainNameAsLabels(dst, domain);

    *reinterpret_cast<uint16_t*>(lengt_pointer) = __builtin_bswap16(static_cast<uint16_t>(6U + dst - begin));

    DEBUG_EXIT();
    return static_cast<uint32_t>(dst - destination);
}

static uint32_t AddAnswerTxt(mdns::ServiceRecord const& service_record, uint8_t* destination, uint32_t ttl)
{
    DEBUG_ENTRY();

    Domain domain;
    CreateServiceDomain(domain, service_record, true);

    auto* dst = AddQuestion(destination, domain, network::dns::RRType::kTxt, true);

    *reinterpret_cast<uint32_t*>(dst) = __builtin_bswap32(ttl);
    dst += 4;

    if (service_record.text_content == nullptr)
    {
        *reinterpret_cast<uint16_t*>(dst) = __builtin_bswap16(0x0001); // Data length
        dst += 2;
        *dst = 0; // Text length
        dst++;
    }
    else
    {
        const auto kSize = service_record.text_content_length;
        *reinterpret_cast<uint16_t*>(dst) = __builtin_bswap16(static_cast<uint16_t>(1U + kSize)); // Data length
        dst += 2;
        *dst = static_cast<uint8_t>(kSize); // Text length
        dst++;
        memcpy(reinterpret_cast<char*>(dst), service_record.text_content, service_record.text_content_length);
        dst += kSize;
    }

    DEBUG_EXIT();
    return static_cast<uint32_t>(dst - destination);
}

static uint32_t AddAnswerPtr(mdns::ServiceRecord const& service_record, uint8_t* destination, uint32_t ttl)
{
    DEBUG_ENTRY();

    Domain domain;
    CreateServiceDomain(domain, service_record, false);

    auto* dst = AddQuestion(destination, domain, network::dns::RRType::kPtr, false);

    *reinterpret_cast<uint32_t*>(dst) = __builtin_bswap32(ttl);
    dst += 4;
    auto* lengt_pointer = dst;
    dst += 2;
    auto* begin = dst;

    CreateServiceDomain(domain, service_record, true);
    dst = PutDomainNameAsLabels(dst, domain);

    *reinterpret_cast<uint16_t*>(lengt_pointer) = __builtin_bswap16(static_cast<uint16_t>(dst - begin));

    DEBUG_EXIT();
    return static_cast<uint32_t>(dst - destination);
}

static uint32_t AddAnswerDnsdPtr(mdns::ServiceRecord const& service_record, uint8_t* destination, uint32_t ttl)
{
    DEBUG_ENTRY();

    auto* dst = AddQuestion(destination, kDomainDnssd, network::dns::RRType::kPtr, false);

    *reinterpret_cast<uint32_t*>(dst) = __builtin_bswap32(ttl);
    dst += 4;
    auto* lengt_pointer = dst;
    dst += 2;
    auto* begin = dst;

    Domain domain;

    CreateServiceDomain(domain, service_record, false);
    dst = PutDomainNameAsLabels(dst, domain);

    *reinterpret_cast<uint16_t*>(lengt_pointer) = __builtin_bswap16(static_cast<uint16_t>(dst - begin));

    DEBUG_EXIT();
    return static_cast<uint32_t>(dst - destination);
}

static uint32_t AddAnswerA(uint8_t* destination, uint32_t ttl)
{
    DEBUG_ENTRY();

    Domain domain;
    CreateHostDomain(domain);

    auto* dst = AddQuestion(destination, domain, network::dns::RRType::kA, true);

    *reinterpret_cast<uint32_t*>(dst) = __builtin_bswap32(ttl);
    dst += 4;
    *reinterpret_cast<uint16_t*>(dst) = __builtin_bswap16(4); // Data length
    dst += 2;
    *reinterpret_cast<uint32_t*>(dst) = network::GetPrimaryIp();
    dst += 4;

    DEBUG_EXIT();
    return static_cast<uint32_t>(dst - destination);
}

#if defined(CONFIG_MDNS_DOMAIN_REVERSE)
static uint32_t AddAnswerHostv4Ptr(uint8_t* destination, uint32_t ttl)
{
    DEBUG_ENTRY();

    Domain domain;
    CreateReverseDomain(domain);

    auto* dst = AddQuestion(destination, domain, network::dns::RRType::kPtr, true);

    *reinterpret_cast<uint32_t*>(dst) = __builtin_bswap32(ttl);
    dst += 4;
    auto* lengt_pointer = dst;
    dst += 2;
    auto* begin = dst;

    CreateHostDomain(domain);
    dst = PutDomainNameAsLabels(dst, domain);

    *reinterpret_cast<uint16_t*>(lengt_pointer) = __builtin_bswap16(static_cast<uint16_t>(dst - begin));

    DEBUG_EXIT();
    return static_cast<uint32_t>(dst - destination);
}
#endif

/*
 * https://opensource.apple.com/source/mDNSResponder/mDNSResponder-26.2/mDNSCore/mDNS.c.auto.html
 * mDNSlocal const mDNSu8 *getDomainName(const DNSMessage *const msg, const mDNSu8 *ptr, const mDNSu8 *const end, domainname *const name)
 *
 * Routine to fetch an FQDN from the DNS message, following compression pointers if necessary.
 */
static const uint8_t* GetDomainName(const uint8_t* const kMsg, const uint8_t* ptr, const uint8_t* const kEnd, uint8_t* const kName)
{
    const uint8_t* nextbyte = nullptr;                // Record where we got to before we started following pointers
    uint8_t* np = kName;                              // Name pointer
    const uint8_t* const kLimit = np + kDomainMaxlen; // Limit so we don't overrun buffer

    if (ptr < reinterpret_cast<const uint8_t*>(kMsg) || ptr >= kEnd)
    {
        DEBUG_PUTS("Illegal ptr not within packet boundaries");
        return nullptr;
    }

    *np = 0; // Tentatively place the root label here (may be overwritten if we have more labels)

    while (1) // Read sequence of labels
    {
        const auto kLen = *ptr++; // Read length of this label
        if (kLen == 0) break;     // If length is zero, that means this name is complete
        switch (kLen & 0xC0)
        {
            int i;
            uint16_t offset;

            case 0x00:
                if (ptr + kLen >= kEnd) // Remember: expect at least one more byte for the root label
                {
                    DEBUG_PUTS("Malformed domain name (overruns packet end)");
                    return nullptr;
                }
                if (np + 1 + kLen >= kLimit) // Remember: expect at least one more byte for the root label
                {
                    DEBUG_PUTS("Malformed domain name (more than 255 characters)");
                    return nullptr;
                }
                *np++ = kLen;
                for (i = 0; i < kLen; i++) *np++ = *ptr++;
                *np = 0; // Tentatively place the root label here (may be overwritten if we have more labels)
                break;

            case 0x40:
                DEBUG_PRINTF("Extended EDNS0 label types 0x40 not supported in name %.*s", static_cast<int>(kLen), kName);
                return nullptr;

            case 0x80:
                DEBUG_PRINTF("Illegal label length 0x80 in domain name %.*s", static_cast<int>(kLen), kName);
                return nullptr;

            case 0xC0:
                offset = static_cast<uint16_t>(((static_cast<uint16_t>(kLen & 0x3F)) << 8) | *ptr++);
                if (!nextbyte) nextbyte = ptr; // Record where we got to before we started following pointers
                ptr = reinterpret_cast<const uint8_t*>(kMsg) + offset;
                if (ptr < reinterpret_cast<const uint8_t*>(kMsg) || ptr >= kEnd)
                {
                    DEBUG_PUTS("Illegal compression pointer not within packet boundaries");
                    return nullptr;
                }
                if (*ptr & 0xC0)
                {
                    DEBUG_PUTS("Compression pointer must point to real label");
                    return nullptr;
                }
                break;
        }
    }

    if (nextbyte)
    {
        return (nextbyte);
    }
    else
    {
        return (ptr);
    }
}

void Start()
{
    DEBUG_ENTRY();

    network::igmp::JoinGroup(s_handle, network::dns::kMulticastAddress);
    network::iface::SetDomainName(&kDomainLocal[1]);

    mdns::SendAnnouncement(kMdnsResponseTtl);

    Domain domain;
    CreateHostDomain(domain);
    domain.Print(true);

    DEBUG_EXIT();
}

void Stop()
{
    DEBUG_ENTRY();

    mdns::SendAnnouncement(0);

    for (auto& record : s_service_records)
    {
        if (record.name != nullptr)
        {
            delete[] record.name;
        }

        if (record.text_content != nullptr)
        {
            delete[] record.text_content;
        }
    }

    network::igmp::LeaveGroup(s_handle, network::dns::kMulticastAddress);
    network::udp::End(network::iana::Ports::kPortMdns);
    s_handle = -1;

    DEBUG_EXIT();
}

static void Send(uint32_t length)
{
    if (!s_is_unicast)
    {
        network::udp::Send(s_handle, s_records_data, length, network::dns::kMulticastAddress, network::iana::Ports::kPortMdns);
        return;
    }

    network::udp::Send(s_handle, s_records_data, length, s_n_remote_ip, s_n_remote_port);
}

static void SendAnswerLocalIpAddress(uint16_t trans_action_id, uint32_t ttl)
{
    DEBUG_ENTRY();

    uint32_t answers = 0;
    uint8_t* dst = reinterpret_cast<uint8_t*>(&s_records_data) + sizeof(struct network::dns::Header);

#if defined(CONFIG_MDNS_DOMAIN_REVERSE)
    if ((HostReply::kPtr & s_host_replies) == HostReply::kPtr)
    {
        if (s_is_legacy_query)
        {
            Domain domain;
            CreateReverseDomain(domain);
            dst = AddQuestion(dst, domain, network::dns::RRType::kPtr, false);
        }
    }
#endif

    if ((HostReply::kA & s_host_replies) == HostReply::kA)
    {
        answers++;
        dst += AddAnswerA(dst, ttl);
    }
#if defined(CONFIG_MDNS_DOMAIN_REVERSE)
    if ((HostReply::kPtr & s_host_replies) == HostReply::kPtr)
    {
        answers++;
        dst += AddAnswerHostv4Ptr(dst, ttl);
    }
#endif

    auto* header = reinterpret_cast<network::dns::Header*>(&s_records_data);

    header->xid = trans_action_id;
    header->flag1 = network::dns::Flag1::kResponse | network::dns::Flag1::kAuthorative;
    header->flag2 = 0;
#if defined(CONFIG_MDNS_DOMAIN_REVERSE)
    header->query_count = __builtin_bswap16(static_cast<uint16_t>(s_is_legacy_query));
#else
    header->query_count = 0;
#endif
    header->answer_count = __builtin_bswap16(static_cast<uint16_t>(answers));
    header->authority_count = 0;
    header->additional_count = 0;

    const auto kSize = static_cast<uint16_t>(dst - reinterpret_cast<uint8_t*>(header));
    Send(kSize);

    DEBUG_EXIT();
}

static void SendMessage(mdns::ServiceRecord const& record, uint16_t transaction_id, uint32_t ttl)
{
    DEBUG_ENTRY();

    uint32_t answers = 0;
    auto* dst = reinterpret_cast<uint8_t*>(&s_records_data) + sizeof(struct network::dns::Header);

    if ((s_service_replies & ServiceReply::kTypePtr) == ServiceReply::kTypePtr)
    {
        answers++;
        dst += AddAnswerDnsdPtr(record, dst, ttl);
    }

    if ((s_service_replies & ServiceReply::kNamePtr) == ServiceReply::kNamePtr)
    {
        answers++;
        dst += AddAnswerPtr(record, dst, ttl);
    }

    if ((s_service_replies & ServiceReply::kSrv) == ServiceReply::kSrv)
    {
        answers++;
        dst += AddAnswerSrv(record, dst, ttl);
    }

    if ((s_service_replies & ServiceReply::kTxt) == ServiceReply::kTxt)
    {
        answers++;
        dst += AddAnswerTxt(record, dst, ttl);
    }

    dst += AddAnswerA(dst, ttl);

    auto* header = reinterpret_cast<network::dns::Header*>(&s_records_data);

    header->xid = transaction_id;
    header->flag1 = network::dns::Flag1::kResponse | network::dns::Flag1::kAuthorative;
    header->flag2 = 0;
    header->query_count = 0;
    header->answer_count = __builtin_bswap16(static_cast<uint16_t>(answers));
    header->authority_count = __builtin_bswap16(0);
    header->additional_count = __builtin_bswap16(1);

    const auto kSize = static_cast<uint16_t>(dst - reinterpret_cast<uint8_t*>(header));
    Send(kSize);

    DEBUG_EXIT();
}

void SendAnnouncement(uint32_t ttl)
{
    DEBUG_ENTRY();

    s_n_remote_port = network::iana::Ports::kPortMdns; // FIXME Hack ;-)
    s_host_replies = HostReply::kA;

    SendAnswerLocalIpAddress(0, ttl);

    for (auto& record : s_service_records)
    {
        if (record.services < Services::kLastNotUsed)
        {
            s_service_replies = ServiceReply::kTypePtr | ServiceReply::kNamePtr | ServiceReply::kSrv | ServiceReply::kTxt;
            SendMessage(record, 0, ttl);
        }
    }

    DEBUG_EXIT();
}

bool ServiceRecordAdd(const char* name, mdns::Services services, const char* text, uint16_t port)
{
    DEBUG_ENTRY();
    assert(services < mdns::Services::kLastNotUsed);

    for (auto& record : s_service_records)
    {
        if (record.services == Services::kLastNotUsed)
        {
            if (name != nullptr)
            {
                const auto kLength = std::min(kLabelMaxlen, strlen(name));
                if (kLength == 0)
                {
                    assert(0);
                    return false;
                }

                record.name = new char[1 + kLength];

                assert(record.name != nullptr);
                memcpy(record.name, name, kLength);
                record.name[kLength] = '\0';
            }

            record.services = services;

            if (port == 0)
            {
                record.port = __builtin_bswap16(kServices[static_cast<uint32_t>(services)].kPortDefault);
            }
            else
            {
                record.port = __builtin_bswap16(port);
            }

            if (text != nullptr)
            {
                const auto kLength = std::min(kTxtMaxlen, strlen(text));
                record.text_content = new char[kLength];

                assert(record.text_content != nullptr);
                memcpy(record.text_content, text, kLength);

                record.text_content_length = static_cast<uint16_t>(kLength);
            }

            s_n_remote_port = network::iana::Ports::kPortMdns; // FIXME Hack ;-)

            s_service_replies = ServiceReply::kTypePtr | ServiceReply::kNamePtr | ServiceReply::kSrv | ServiceReply::kTxt;

            SendMessage(record, 0, kMdnsResponseTtl);

            Domain domain;
            CreateServiceDomain(domain, record, false);
            domain.Print();

            printf(" %d %.*s\n", __builtin_bswap16(record.port), record.text_content_length, record.text_content == nullptr ? "" : record.text_content);
            return true;
        }
    }

    assert(0);
    return false;
}

bool ServiceRecordDelete(mdns::Services service)
{
    DEBUG_ENTRY();
    assert(service < mdns::Services::kLastNotUsed);

    for (auto& record : s_service_records)
    {
        if (record.services == service)
        {
            SendMessage(record, 0, 0);

            if (record.name != nullptr)
            {
                delete[] record.name;
            }

            if (record.text_content != nullptr)
            {
                delete[] record.text_content;
            }

            DEBUG_EXIT();
            return true;
        }
    }

    DEBUG_EXIT();
    return false;
}

static void HandleQuestions(uint32_t questions)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("questions=%u", questions);

    s_host_replies = 0;
    s_is_unicast = (s_n_remote_port != network::iana::Ports::kPortMdns);
    s_is_legacy_query = s_is_unicast && (questions == 1);

    const auto kTransactionID = s_is_legacy_query ? *reinterpret_cast<uint16_t*>(&s_p_receive_buffer[0]) : static_cast<uint16_t>(0);

    uint32_t offset = sizeof(struct network::dns::Header);

    for (uint32_t i = 0; i < questions; i++)
    {
        Domain resource_domain;

        auto* result = GetDomainName(s_p_receive_buffer, &s_p_receive_buffer[offset], &s_p_receive_buffer[s_n_bytes_received], resource_domain.a_name);
        if (result == nullptr)
        {
            DEBUG_EXIT();
            return;
        }

        resource_domain.length = static_cast<uint16_t>(result - &s_p_receive_buffer[offset]);
        offset += resource_domain.length;

        const auto kType = static_cast<network::dns::RRType>(__builtin_bswap16(*reinterpret_cast<uint16_t*>(&s_p_receive_buffer[offset])));
        offset += 2;

        const auto kClass = __builtin_bswap16(*reinterpret_cast<uint16_t*>(&s_p_receive_buffer[offset])) & 0x7F;
        offset += 2;

#ifndef NDEBUG
        resource_domain.Print();
        printf(" ==> Type : %d, Class: %d\n", static_cast<int>(kType), static_cast<int>(kClass));
#endif

        if ((kClass != network::dns::RRClass::kInternet) && (kClass != network::dns::RRClass::kAny))
        {
            continue;
        }

        /*
         * Check host
         */

        Domain domain_host;

        if ((kType == network::dns::RRType::kA) || (kType == network::dns::RRType::kAll))
        {
            DEBUG_PUTS("");
            CreateHostDomain(domain_host);

            if (domain_host == resource_domain)
            {
                s_host_replies = s_host_replies | HostReply::kA;
            }
        }

#if defined(CONFIG_MDNS_DOMAIN_REVERSE)
        if (kType == network::dns::RRType::kPtr || kType == network::dns::RRType::kAll)
        {
            DEBUG_PUTS("");
            CreateReverseDomain(domain_host);

            if (domain_host == resource_domain)
            {
                s_host_replies = s_host_replies | HostReply::kPtr;
            }
        }
#endif

        for (auto& record : s_service_records)
        {
            if (record.services < Services::kLastNotUsed)
            {
                /*
                 * Check service
                 */

                s_service_replies = 0;
                Domain service_domain;

                if (kType == network::dns::RRType::kPtr || kType == network::dns::RRType::kAll)
                {
                    if (kDomainDnssd == resource_domain)
                    {
                        s_service_replies = s_service_replies | ServiceReply::kTypePtr;
                    }

                    CreateServiceDomain(service_domain, record, false);

                    if (service_domain == resource_domain)
                    {
                        s_service_replies = s_service_replies | ServiceReply::kNamePtr;
                        s_service_replies = s_service_replies | ServiceReply::kSrv;
                        s_service_replies = s_service_replies | ServiceReply::kTxt;
                    }
                }

                CreateServiceDomain(service_domain, record, true);

                if (service_domain == resource_domain)
                {
                    if ((kType == network::dns::RRType::kSrv) || (kType == network::dns::RRType::kAll))
                    {
                        s_service_replies = s_service_replies | ServiceReply::kSrv;
                    }

                    if ((kType == network::dns::RRType::kTxt) || (kType == network::dns::RRType::kAll))
                    {
                        s_service_replies = s_service_replies | ServiceReply::kTxt;
                    }
                }

                if (s_service_replies != 0)
                {
                    SendMessage(record, kTransactionID, kMdnsResponseTtl);
                }
            }
        }
    }

    if (s_host_replies != 0)
    {
        DEBUG_PUTS("");
        SendAnswerLocalIpAddress(kTransactionID, kMdnsResponseTtl);
    }

    DEBUG_EXIT();
}

static void Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
{
    s_p_receive_buffer = const_cast<uint8_t*>(buffer);
    s_n_bytes_received = size;
    s_n_remote_ip = from_ip;
    s_n_remote_port = from_port;

    const auto* const kHeader = reinterpret_cast<network::dns::Header*>(s_p_receive_buffer);
    const auto kFlag1 = kHeader->flag1;

    if ((kFlag1 >> 3) & 0xF)
    {
        return;
    }

    HandleQuestions(static_cast<uint32_t>(__builtin_bswap16(kHeader->query_count)));
}

void Init()
{
    DEBUG_ENTRY();

    for (auto& record : s_service_records)
    {
        record.services = Services::kLastNotUsed;
    }

    s_handle = network::udp::Begin(network::iana::Ports::kPortMdns, Input);
    assert(s_handle != -1);

    DEBUG_EXIT();
}
} // namespace network::apps::mdns
