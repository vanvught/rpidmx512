/**
 * @file oscclientparams.h
 *
 */
 /* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
 

#ifndef JSON_OSCCLIENTPARAMS_H_
#define JSON_OSCCLIENTPARAMS_H_

#include "configurationstore.h"
#include "json/oscclientparamsconst.h"
#include "json/oscparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"

namespace json
{
class OscClientParams : public JsonParamsBase<OscClientParams>
{
   public:
    OscClientParams();

    OscClientParams(const OscClientParams&) = delete;
    OscClientParams& operator=(const OscClientParams&) = delete;

    OscClientParams(OscClientParams&&) = delete;
    OscClientParams& operator=(OscClientParams&&) = delete;

    void Load() { JsonParamsBase::Load(OscClientParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetIncomingPort(const char* val, uint32_t len);
    static void SetOutgoingPort(const char* val, uint32_t len);
    static void SetServerIp(const char* val, uint32_t len);
    static void SetPingDisable(const char* val, uint32_t len);
    static void SetPingDelay(const char* val, uint32_t len);
    static void SetCmd(const char* key, uint32_t key_len, const char* val, uint32_t val_len);
    static void SetLed(const char* key, uint32_t key_len, const char* val, uint32_t val_len);

    static constexpr json::Key kOscClientKeys[] = 
    {
		MakeKey(SetIncomingPort, OscParamsConst::kIncomingPort),
		MakeKey(SetOutgoingPort, OscParamsConst::kOutgoingPort),
        MakeKey(SetServerIp, OscClientParamsConst::kServerIp),   
        MakeKey(SetPingDisable, OscClientParamsConst::kPingDisable),
        MakeKey(SetPingDelay, OscClientParamsConst::kPingDelay), 
        MakeKey(SetCmd, OscClientParamsConst::kCmd[0]),
        MakeKey(SetCmd, OscClientParamsConst::kCmd[1]),          
        MakeKey(SetCmd, OscClientParamsConst::kCmd[2]),
        MakeKey(SetCmd, OscClientParamsConst::kCmd[3]),          
        MakeKey(SetCmd, OscClientParamsConst::kCmd[4]),
        MakeKey(SetCmd, OscClientParamsConst::kCmd[5]),          
        MakeKey(SetCmd, OscClientParamsConst::kCmd[6]),
        MakeKey(SetCmd, OscClientParamsConst::kCmd[7]),
        MakeKey(SetLed, OscClientParamsConst::kLed[0]),
        MakeKey(SetLed, OscClientParamsConst::kLed[1]),          
        MakeKey(SetLed, OscClientParamsConst::kLed[2]),
        MakeKey(SetLed, OscClientParamsConst::kLed[3]),          
        MakeKey(SetLed, OscClientParamsConst::kLed[4]),
        MakeKey(SetLed, OscClientParamsConst::kLed[5]),          
        MakeKey(SetLed, OscClientParamsConst::kLed[6]),
        MakeKey(SetLed, OscClientParamsConst::kLed[7])
    };

    inline static common::store::OscClient store_oscclient;

    friend class JsonParamsBase<OscClientParams>;
};
} // namespace json

#endif  // JSON_OSCCLIENTPARAMS_H_
