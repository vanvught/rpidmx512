/**
 * @file ltcetcparams.h
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

#ifndef JSON_LTCETCPARAMS_H_
#define JSON_LTCETCPARAMS_H_

#include "configurationstore.h"
#include "json/ltcetcparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"

namespace json
{
class LtcEtcParams : public JsonParamsBase<LtcEtcParams>
{
   public:
    LtcEtcParams();

    LtcEtcParams(const LtcEtcParams&) = delete;
    LtcEtcParams& operator=(const LtcEtcParams&) = delete;

    LtcEtcParams(LtcEtcParams&&) = delete;
    LtcEtcParams& operator=(LtcEtcParams&&) = delete;

    void Load() { JsonParamsBase::Load(LtcEtcParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetDestinationIp(const char* val, uint32_t len);
    static void SetDestinationPort(const char* val, uint32_t len);
    static void SetSourceMulticastIp(const char* val, uint32_t len);
    static void SetSourcePort(const char* val, uint32_t len);
    static void SetUdpTerminator(const char* val, uint32_t len);

    static constexpr json::Key kLtcEtcKeys[] = {
		MakeKey(SetDestinationIp, LtcEtcParamsConst::kDestinationIp),
		MakeKey(SetDestinationPort, LtcEtcParamsConst::kDestinationPort),
		MakeKey(SetSourceMulticastIp, LtcEtcParamsConst::kSourceMulticastIp),
		MakeKey(SetSourcePort, LtcEtcParamsConst::kDestinationPort),
		MakeKey(SetUdpTerminator, LtcEtcParamsConst::kUdpTerminator)
    };

    inline static common::store::LtcEtc store_ltcetc;

    friend class JsonParamsBase<LtcEtcParams>;
};
} // namespace json

#endif  // JSON_LTCETCPARAMS_H_
