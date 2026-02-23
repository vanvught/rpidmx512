/**
 * @file dmxnodeparams.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef JSON_DMXNODEPARAMS_H_
#define JSON_DMXNODEPARAMS_H_

#include <cstdint>

#include "configurationstore.h"
#include "dmxnode.h"
#include "json/dmxnodeparamsconst.h"
#include "json/json_key.h"
#include "json/json_params_base.h"

#if !defined (MAX_ARRAY_SIZE)
#error
#endif

namespace json
{
class DmxNodeParams : public JsonParamsBase<DmxNodeParams>
{
   public:
    DmxNodeParams();

    DmxNodeParams(const DmxNodeParams&) = delete;
    DmxNodeParams& operator=(const DmxNodeParams&) = delete;

    DmxNodeParams(DmxNodeParams&&) = delete;
    DmxNodeParams& operator=(DmxNodeParams&&) = delete;

    void Load() { JsonParamsBase::Load(json::DmxNodeParamsConst::kFileName); }
    void Store(const char* buffer, uint32_t buffer_size);
    void Set();

   protected:
    void Dump();

   private:
   	static_assert(static_cast<uint32_t>(dmxnode::OutputStyle::kDelta) == 0);
    dmxnode::OutputStyle GetOutputStyleSet(uint8_t mask) const { return (store_dmxnode_.output_style & mask) == mask ? dmxnode::OutputStyle::kConstant : dmxnode::OutputStyle::kDelta; }

   private:
    static void SetPersonality(const char* val, uint32_t len);
    static void SetNodeName(const char* val, uint32_t len);
    static void SetFailsafe(const char* val, uint32_t len);
    static void SetDisableMergeTimeout(const char* val, uint32_t len);
    static void SetLabelPort(const char* key, uint32_t key_len, const char* val, uint32_t val_len);
    static void SetUniversePort(const char* key, uint32_t key_len, const char* val, uint32_t val_len);
    static void SetDirectionPort(const char* key, uint32_t key_len, const char* val, uint32_t val_len);
    static void SetMergeModePort(const char* key, uint32_t key_len, const char* val, uint32_t val_len);
    static void SetOutputStylePort(const char* key, uint32_t key_len, const char* val, uint32_t val_len);
    
    static constexpr json::Key kDmxNodeKeys[] = {
        MakeKey(SetPersonality, DmxNodeParamsConst::kPersonality),         
        MakeKey(SetNodeName, DmxNodeParamsConst::kNodeName),
        MakeKey(SetFailsafe, DmxNodeParamsConst::kFailsafe),         
        MakeKey(SetDisableMergeTimeout, DmxNodeParamsConst::kDisableMergeTimeout),
        MakeKey(SetLabelPort, DmxNodeParamsConst::kLabelPort[0]),
        MakeKey(SetUniversePort, DmxNodeParamsConst::kUniversePort[0]),
        MakeKey(SetDirectionPort, DmxNodeParamsConst::kDirectionPort[0]),
        MakeKey(SetMergeModePort, DmxNodeParamsConst::kMergeModePort[0]),
        MakeKey(SetOutputStylePort, DmxNodeParamsConst::kOutputStylePort[0]),
#if (MAX_ARRAY_SIZE > 1)
        MakeKey(SetLabelPort, DmxNodeParamsConst::kLabelPort[1]),
        MakeKey(SetUniversePort, DmxNodeParamsConst::kUniversePort[1]),
        MakeKey(SetDirectionPort, DmxNodeParamsConst::kDirectionPort[1]),
        MakeKey(SetMergeModePort, DmxNodeParamsConst::kMergeModePort[1]),
        MakeKey(SetOutputStylePort, DmxNodeParamsConst::kOutputStylePort[1]),
#endif
#if (MAX_ARRAY_SIZE > 2)
        MakeKey(SetLabelPort, DmxNodeParamsConst::kLabelPort[2]),
        MakeKey(SetUniversePort, DmxNodeParamsConst::kUniversePort[2]),
        MakeKey(SetDirectionPort, DmxNodeParamsConst::kDirectionPort[2]),
        MakeKey(SetMergeModePort, DmxNodeParamsConst::kMergeModePort[2]),
        MakeKey(SetOutputStylePort, DmxNodeParamsConst::kOutputStylePort[2]),
#endif
#if (MAX_ARRAY_SIZE == 4)
        MakeKey(SetLabelPort, DmxNodeParamsConst::kLabelPort[3]),
        MakeKey(SetUniversePort, DmxNodeParamsConst::kUniversePort[3]),
        MakeKey(SetDirectionPort, DmxNodeParamsConst::kDirectionPort[3]),
        MakeKey(SetMergeModePort, DmxNodeParamsConst::kMergeModePort[3]),
        MakeKey(SetOutputStylePort, DmxNodeParamsConst::kOutputStylePort[3]),
#endif
    };
   
    inline static common::store::DmxNode store_dmxnode_;

    friend class JsonParamsBase<DmxNodeParams>;
};
} // namespace json

#endif  // JSON_DMXNODEPARAMS_H_
