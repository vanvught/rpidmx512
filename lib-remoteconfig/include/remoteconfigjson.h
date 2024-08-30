/**
 * @file remoteconfig.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef REMOTECONFIGJSON_H_
#define REMOTECONFIGJSON_H_

#include <cstdint>

namespace remoteconfig {
uint32_t json_get_list(char *pOutBuffer, const uint32_t nOutBufferSize);
uint32_t json_get_version(char *pOutBuffer, const uint32_t nOutBufferSize);
uint32_t json_get_uptime(char *pOutBuffer, const uint32_t nOutBufferSize);
uint32_t json_get_display(char *pOutBuffer, const uint32_t nOutBufferSize);
uint32_t json_get_directory(char *pOutBuffer, const uint32_t nOutBufferSize);
namespace net {
uint32_t json_get_phystatus(char *pOutBuffer, const uint32_t nOutBufferSize);
}  // namespace net
namespace dmx {
uint32_t json_get_ports(char *pOutBuffer, const uint32_t nOutBufferSize);
uint32_t json_get_portstatus(const char cPort, char *pOutBuffer, const uint32_t nOutBufferSize);
}  // namespace dmx
namespace rdm {
uint32_t json_get_rdm(char *pOutBuffer, const uint32_t nOutBufferSize);
uint32_t json_get_queue(char *pOutBuffer, const uint32_t nOutBufferSize);
uint32_t json_get_portstatus(char *pOutBuffer, const uint32_t nOutBufferSize);
uint32_t json_get_tod(const char cPort, char *pOutBuffer, const uint32_t nOutBufferSize);
}  // namespace rdm
namespace storage {
uint32_t json_get_directory(char *pOutBuffer, const uint32_t nOutBufferSize);
}  // namespace storage
namespace dsa {
uint32_t json_get_portstatus(char *pOutBuffer, const uint32_t nOutBufferSize);
uint32_t json_get_vlantable(char *pOutBuffer, const uint32_t nOutBufferSize);
}  // namespace dsa
namespace showfile {
uint32_t json_get_status(char *pOutBuffer, const uint32_t nOutBufferSize);
uint32_t json_get_directory(char *pOutBuffer, const uint32_t nOutBufferSize);
void json_set_status(const char *pBuffer, const uint32_t nBufferSize);
void json_delete(const char *pBuffer, const uint32_t nBufferSize);
}  // namespace showfile
namespace timedate {
uint32_t json_get_timeofday(char *pOutBuffer, const uint32_t nOutBufferSize);
void json_set_timeofday(const char *pBuffer, const uint32_t nBufferSize);
}  // namespace timedate
namespace rtc {
uint32_t json_get_rtc(char *pOutBuffer, const uint32_t nOutBufferSize);
void json_set_rtc(const char *pBuffer, const uint32_t nBufferSize);
}  // namespace rtc
namespace artnet {
namespace controller {
uint32_t json_get_polltable(char *pOutBuffer, const uint32_t nOutBufferSize);
}  // namespace controller
}  // namespace artnet
namespace pixel {
uint32_t json_get_types(char *pOutBuffer, const uint32_t nOutBufferSize);
uint32_t json_get_status(char *pOutBuffer, const uint32_t nOutBufferSize);
}  // namespace pixel
}  // namespace remoteconfig

#endif /* REMOTECONFIGJSON_H_ */
