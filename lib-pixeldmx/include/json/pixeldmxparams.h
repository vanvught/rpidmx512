/**
 * @file pixeldmxparams.h
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
 

#ifndef JSON_PIXELDMXPARAMS_H_
#define JSON_PIXELDMXPARAMS_H_

#include "configurationstore.h"
#include "json/json_key.h"
#include "json/dmxledparamsconst.h"
#include "json/pixeldmxparamsconst.h"
#include "json/json_params_base.h"
#include "pixeldmxparamsconst.h"

namespace json
{
class PixelDmxParams : public JsonParamsBase<PixelDmxParams>
{
   public:
    PixelDmxParams();

    PixelDmxParams(const PixelDmxParams&) = delete;
    PixelDmxParams& operator=(const PixelDmxParams&) = delete;

    PixelDmxParams(PixelDmxParams&&) = delete;
    PixelDmxParams& operator=(PixelDmxParams&&) = delete;

    void Load() { JsonParamsBase::Load(json::DmxLedParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetType(const char* val, uint32_t len);
    static void SetMap(const char* val, uint32_t len);
    static void SetCount(const char* val, uint32_t len);
    static void SetGroupingCount(const char* val, uint32_t len);
    static void SetLowCode(const char* val, uint32_t len);
    static void SetHighCode(const char* val, uint32_t len);
#if defined(OUTPUT_DMX_PIXEL_MULTI)
    static void SetActiveOutputs(const char* val, uint32_t len);
#endif
    static void SetTestPattern(const char* val, uint32_t len);
    static void SetSpiSpeedHz(const char* val, uint32_t len);
    static void SetGlobalBrightness(const char* val, uint32_t len);
    static void SetStartUniPort(const char* key, uint32_t key_len, const char* val, uint32_t val_len);
#if !defined(OUTPUT_DMX_PIXEL_MULTI)
	static void SetDmxStartAddress(const char* val, uint32_t len);
#endif
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)    
    static void SetGammaCorrection(const char* val, uint32_t len);
    static void SetGammaValue(const char* val, uint32_t len);
#endif

    static constexpr json::Key kPixelDmxKeys[] = {
	MakeKey(SetType, DmxLedParamsConst::kType), 
	MakeKey(SetMap, DmxLedParamsConst::kMap), 
	MakeKey(SetCount, DmxLedParamsConst::kCount),
	MakeKey(SetGroupingCount, DmxLedParamsConst::kGroupingCount),
	MakeKey(SetLowCode, DmxLedParamsConst::kT0H),
	MakeKey(SetHighCode, DmxLedParamsConst::kT1H),
#if defined(OUTPUT_DMX_PIXEL_MULTI)
	MakeKey(SetActiveOutputs, DmxLedParamsConst::kActiveOutputPorts),
#endif
	MakeKey(SetTestPattern, DmxLedParamsConst::kTestPattern),
	MakeKey(SetSpiSpeedHz, DmxLedParamsConst::kSpiSpeedHz),
	MakeKey(SetGlobalBrightness, DmxLedParamsConst::kGlobalBrightness),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[0]),
#if (CONFIG_DMXNODE_PIXEL_MAX_PORTS > 1)
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[1]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[2]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[3]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[4]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[5]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[6]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[7]),
#endif
#if CONFIG_DMXNODE_PIXEL_MAX_PORTS == 16
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[8]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[9]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[10]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[11]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[12]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[13]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[14]),
	MakeKey(SetStartUniPort, PixelDmxParamsConst::kStartUniPort[15]),
#endif
#if defined(RDM_RESPONDER)
	MakeKey(SetDmxStartAddress, PixelDmxParamsConst::kDmxStartAddress),
#endif
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
   MakeKey(SetGammaCorrection, DmxLedParamsConst::kGammaCorrection),
   MakeKey(SetGammaValue, DmxLedParamsConst::kGammaValue)
#endif   
    };

    inline static common::store::DmxLed store_dmxled;

    friend class JsonParamsBase<PixelDmxParams>;
};
} // namespace json

#endif  // JSON_PIXELDMXPARAMS_H_
