/**
 * @file oscserverparams.h
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
 

#ifndef JSON_OSCSERVERPARAMS_H_
#define JSON_OSCSERVERPARAMS_H_

#include "configurationstore.h"
#include "json/oscserverparamsconst.h"
#include "json/oscparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"

namespace json
{
class OscServerParams : public JsonParamsBase<OscServerParams>
{
   public:
    OscServerParams();

    OscServerParams(const OscServerParams&) = delete;
    OscServerParams& operator=(const OscServerParams&) = delete;

    OscServerParams(OscServerParams&&) = delete;
    OscServerParams& operator=(OscServerParams&&) = delete;

    void Load() { JsonParamsBase::Load(OscServerParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetIncomingPort(const char* val, uint32_t len);
    static void SetOutgoingPort(const char* val, uint32_t len);
    static void SetPath(const char* val, uint32_t len);
    static void SetPathInfo(const char* val, uint32_t len);
    static void SetPathBlackout(const char* val, uint32_t len);
    static void SetTransmission(const char* val, uint32_t len);

    static constexpr Key kOscServerKeys[] = {
		MakeKey(SetIncomingPort, OscParamsConst::kIncomingPort),
		MakeKey(SetOutgoingPort, OscParamsConst::kOutgoingPort),
		MakeKey(SetPath, OscServerParamsConst::kPath),
 		MakeKey(SetPathInfo, OscServerParamsConst::kPathInfo),
  		MakeKey(SetPathBlackout, OscServerParamsConst::kPathBlackout),
  		MakeKey(SetTransmission, OscServerParamsConst::kTransmission)
    };

    inline static common::store::OscServer store_oscserver;

    friend class JsonParamsBase<OscServerParams>;
};
} // namespace json

#endif  // JSON_OSCSERVERPARAMS_H_
