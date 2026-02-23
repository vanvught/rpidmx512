/**
 * @file e131params.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef JSON_E131PARAMS_H_
#define JSON_E131PARAMS_H_

#include "configurationstore.h"
#include "json/e131paramsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"
#include "configurationstore.h"

#if !defined (MAX_ARRAY_SIZE)
#error
#endif

namespace json
{
class E131Params : public JsonParamsBase<E131Params>
{
   public:
    E131Params();

    E131Params(const E131Params&) = delete;
    E131Params& operator=(const E131Params&) = delete;

    E131Params(E131Params&&) = delete;
    E131Params& operator=(E131Params&&) = delete;

    void Load() { JsonParamsBase::Load(json::E131ParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
    static void SetPriority(const char* key, uint32_t key_len, const char* val, uint32_t val_len);

    static constexpr json::Key kE131PriorityKeys[] = {
        MakeKey(SetPriority, E131ParamsConst::kPriorityPort[0]),
#if (MAX_ARRAY_SIZE > 1)
        MakeKey(SetPriority, E131ParamsConst::kPriorityPort[1]),
#endif
#if (MAX_ARRAY_SIZE > 2)
        MakeKey(SetPriority, E131ParamsConst::kPriorityPort[2]),
#endif
#if (MAX_ARRAY_SIZE == 4)
        MakeKey(SetPriority, E131ParamsConst::kPriorityPort[3]),
#endif
    };

    inline static common::store::DmxNode store_dmxnode_;

    friend class JsonParamsBase<E131Params>;
};
} // namespace json

#endif  // JSON_E131PARAMS_H_
