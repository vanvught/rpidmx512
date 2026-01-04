/**
 * @file e131paramsconst.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef JSON_E131PARAMSCONST_H_
#define JSON_E131PARAMSCONST_H_

#include "configurationstore.h"
#include "json/json_key.h"

#if defined(DMXNODE_OUTPUT_DMX)
#include "dmx.h"
#define MAX_ARRAY_SIZE DMX_MAX_PORTS
#else
#define MAX_ARRAY_SIZE 4
#endif

static_assert(MAX_ARRAY_SIZE <= common::store::dmxnode::kParamPorts);

namespace json
{
struct E131ParamsConst
{
    static constexpr char kFileName[] = "e131.json";

    static constexpr json::PortKey kPriorityPortA{"priority_port_a", 16, Fnv1a32("priority_port_a", 16)};
#if (MAX_ARRAY_SIZE > 1)
    static constexpr json::PortKey kPriorityPortB{"priority_port_b", 16, Fnv1a32("priority_port_b", 16)};
#endif
#if (MAX_ARRAY_SIZE > 2)
    static constexpr json::PortKey kPriorityPortC{"priority_port_c", 16, Fnv1a32("priority_port_c", 16)};
#endif
#if (MAX_ARRAY_SIZE == 4)
    static constexpr json::PortKey kPriorityPortD{"priority_port_d", 16, Fnv1a32("priority_port_d", 16)};
#endif

    static constexpr json::PortKey kPriorityPort[] = 
    {
		kPriorityPortA,
#if (MAX_ARRAY_SIZE > 1)
		kPriorityPortB,
#endif
#if (MAX_ARRAY_SIZE > 2)
    	kPriorityPortC,
#endif
#if (MAX_ARRAY_SIZE == 4)
    	kPriorityPortD,
#endif
    };
};
} // namespace json

#endif  // JSON_E131PARAMSCONST_H_
