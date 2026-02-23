/** 
 * @file tcnetparams.h
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

#ifndef JSON_TCNETPARAMS_H_
#define JSON_TCNETPARAMS_H_

#include "configurationstore.h"
#include "json/tcnetparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"

namespace json
{
class TcNetParams : public JsonParamsBase<TcNetParams>
{
   public:
    TcNetParams();

    TcNetParams(const TcNetParams&) = delete;
    TcNetParams& operator=(const TcNetParams&) = delete;

    TcNetParams(TcNetParams&&) = delete;
    TcNetParams& operator=(TcNetParams&&) = delete;

    void Load() { JsonParamsBase::Load(TcNetParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetNodeName(const char* val, uint32_t len);
    static void SetLayer(const char* val, uint32_t len);
    static void SetTimecodeType(const char* val, uint32_t len);
    static void SetUseTimecode(const char* val, uint32_t len);

    static constexpr json::Key kTcNetKeys[] = {
		MakeKey(SetNodeName, TcNetParamsConst::kNodeName),
 		MakeKey(SetLayer, TcNetParamsConst::kLayer),
  		MakeKey(SetTimecodeType, TcNetParamsConst::kTimecodeType),
  		MakeKey(SetUseTimecode, TcNetParamsConst::kUseTimecode)
    };

    inline static common::store::TcNet store_tcnet;

    friend class JsonParamsBase<TcNetParams>;
};
} // namespace json

#endif  // JSON_TCNETPARAMS_H_
