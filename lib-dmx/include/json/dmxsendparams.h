/**
 * @file dmxsendparams.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef JSON_DMXSENDPARAMS_H_
#define JSON_DMXSENDPARAMS_H_

#include "configurationstore.h"
#include "json/dmxsendparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"

namespace json
{
class DmxSendParams : public JsonParamsBase<DmxSendParams>
{
   public:
    DmxSendParams();

    DmxSendParams(const DmxSendParams&) = delete;
    DmxSendParams& operator=(const DmxSendParams&) = delete;

    DmxSendParams(DmxSendParams&&) = delete;
    DmxSendParams& operator=(DmxSendParams&&) = delete;

    void Load() { JsonParamsBase::Load(DmxSendParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetBreakTime(const char* val, uint32_t len);
    static void SetMabTime(const char* val, uint32_t len);
    static void SetRefreshRate(const char* val, uint32_t len);
    static void SetSlotsCount(const char* val, uint32_t len);

    static constexpr json::Key kDmxSendKeys[] = {
		MakeKey(SetBreakTime, DmxSendParamsConst::kBreakTime), 
		MakeKey(SetMabTime, DmxSendParamsConst::kMabTime), 
		MakeKey(SetRefreshRate, DmxSendParamsConst::kRefreshRate),
        MakeKey(SetSlotsCount, DmxSendParamsConst::kSlotsCount)
    };

    inline static common::store::DmxSend store_dmx_send;

    friend class JsonParamsBase<DmxSendParams>;
};
} // namespace json

#endif  // JSON_DMXSENDPARAMS_H_
