/**
 * @file mdns.h
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

#ifndef NET_APPS_MDNS_H_
#define NET_APPS_MDNS_H_

#include <cstdint>

#include "../config/apps_config.h"

namespace mdns {
static constexpr uint32_t MDNS_RESPONSE_TTL = 3600;		///< (in seconds)

enum class Services {
	CONFIG, TFTP, HTTP, RDMNET_LLRP, NTP, MIDI, OSC, DDP, PP, LAST_NOT_USED
};

struct ServiceRecord {
	char *pName;
	char *pTextContent;
	uint16_t nTextContentLength;
	uint16_t nPort;
	mdns::Services services;
};
}  // namespace mdns

void mdns_init();
void mdns_start();
void mdns_stop();

bool mdns_service_record_add(const char *pName, const mdns::Services service, const char *pTextContent = nullptr, const uint16_t nPort = 0);
bool mdns_service_record_delete(const mdns::Services service);

void mdns_send_announcement(const uint32_t nTTL);

#endif /* NET_APPS_MDNS_H_ */
