/**
 * @file mdns.h
 *
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

#ifndef APPS_MDNS_H_
#define APPS_MDNS_H_

#include <cstdint>

namespace network::apps::mdns
{
static constexpr uint32_t kMdnsResponseTtl = 3600; ///< (in seconds)

enum class Services
{
    kConfig,
    kTftp,
    kHttp,
    kRdmnetLlrp,
    kNtp,
    kMidi,
    kOsc,
    kDdp,
    kPp,
    kLastNotUsed
};

struct ServiceRecord
{
    char* name;
    char* text_content;
    uint16_t text_content_length;
    uint16_t port;
    Services services;
};

void Init();
void Start();
void Stop();

bool ServiceRecordAdd(const char* name, Services service, const char* text_content = nullptr, uint16_t port = 0);
bool ServiceRecordDelete(Services service);

void SendAnnouncement(uint32_t ttl);
} // namespace network::apps::mdns

#endif // APPS_MDNS_H_
