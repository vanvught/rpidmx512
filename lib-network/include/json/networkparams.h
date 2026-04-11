/**
 * @file networkparams.h
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef JSON_NETWORKPARAMS_H_
#define JSON_NETWORKPARAMS_H_

#include <cstdint>

#include "configurationstore.h"
#include "json/json_key.h"
#include "json/networkparamsconst.h"
#include "json/json_params_base.h"

namespace json
{
class NetworkParams : public JsonParamsBase<NetworkParams>
{
   public:
    NetworkParams();

    NetworkParams(const NetworkParams&) = delete;
    NetworkParams& operator=(const NetworkParams&) = delete;

    NetworkParams(NetworkParams&&) = delete;
    NetworkParams& operator=(NetworkParams&&) = delete;

    void Load() { JsonParamsBase::Load(NetworkParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetUseStaticIp(const char* val, uint32_t len);
    static void SetIpAddress(const char* val, uint32_t len);
    static void SetNetMask(const char* val, uint32_t len);
    static void SetDefaultGateway(const char* val, uint32_t len);
    static void SetHostname(const char* val, uint32_t len);
    static void SetNtpServer(const char* val, uint32_t len);
    
   	static constexpr json::Key kNetworkKeys[] = {
	json::MakeKey(SetUseStaticIp, NetworkParamsConst::kUseStaticIp), 
	json::MakeKey(SetIpAddress, NetworkParamsConst::kIpAddress), 
	json::MakeKey(SetNetMask, NetworkParamsConst::kNetMask),
	json::MakeKey(SetDefaultGateway, NetworkParamsConst::kDefaultGateway),
	json::MakeKey(SetHostname, NetworkParamsConst::kHostname),
	json::MakeKey(SetNtpServer, NetworkParamsConst::kNtpServer)
    };

    inline static common::store::Network store_network;

    friend class JsonParamsBase<NetworkParams>;
};
} // namespace json

#endif  // JSON_NETWORKPARAMS_H_
