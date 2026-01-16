/**
 * @file dmxnodeparamsconst.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */

#ifndef JSON_DMXNODEPARAMSCONST_H_
#define JSON_DMXNODEPARAMSCONST_H_

#include "configurationstore.h"
#include "json/json_key.h"

#undef MAX_ARRAY_SIZE
#if defined(DMXNODE_OUTPUT_DMX)
#include "dmx.h"
#define MAX_ARRAY_SIZE DMX_MAX_PORTS
#else
#define MAX_ARRAY_SIZE 4
#endif

static_assert(MAX_ARRAY_SIZE <= common::store::dmxnode::kParamPorts);

namespace json
{
struct DmxNodeParamsConst
{
    static constexpr char kFileName[] = "dmxnode.json";

	static constexpr json::SimpleKey kPersonality {
	    "personality",
	    11,
	    Fnv1a32("personality", 11)
	};
	
	static constexpr json::SimpleKey kNodeName {
	    "node_name",
	    9,
	    Fnv1a32("node_name", 9)
	};
	
	static constexpr json::SimpleKey kFailsafe {
	    "failsafe",
	    8,
	    Fnv1a32("failsafe", 8)
	};
		
	static constexpr json::SimpleKey kDisableMergeTimeout {
	    "disable_merge_timeout",
	    21,
	    Fnv1a32("disable_merge_timeout", 21)
	};
	
	static constexpr json::SimpleKey kDmxStartAddress {
	    "dmx_start_address",
	    17,
	    Fnv1a32("dmx_start_address", 17)
	};
	
	static constexpr json::SimpleKey kDmxSlotInfo {
	    "dmx_slot_info",
	    13,
	    Fnv1a32("dmx_slot_info", 13)
	};

    static constexpr json::PortKey kLabelPortA{"label_port_a", 12, Fnv1a32("label_port_a", 12)};
#if (MAX_ARRAY_SIZE > 1)
    static constexpr json::PortKey kLabelPortB{"label_port_b", 12, Fnv1a32("label_port_b", 12)};
#endif
#if (MAX_ARRAY_SIZE > 2)
    static constexpr json::PortKey kLabelPortC{"label_port_c", 12, Fnv1a32("label_port_c", 12)};
#endif
#if (MAX_ARRAY_SIZE == 4)
    static constexpr json::PortKey kLabelPortD{"label_port_d", 12, Fnv1a32("label_port_d", 12)};
#endif

    static constexpr json::PortKey kLabelPort[] = 
    {
        kLabelPortA,
#if (MAX_ARRAY_SIZE > 1)
        kLabelPortB,
#endif
#if (MAX_ARRAY_SIZE > 2)
        kLabelPortC,
#endif
#if (MAX_ARRAY_SIZE == 4)
        kLabelPortD,
#endif
    };

    static constexpr json::PortKey kUniversePortA{"universe_port_a", 15, Fnv1a32("universe_port_a", 15)};
#if (MAX_ARRAY_SIZE > 1)
    static constexpr json::PortKey kUniversePortB{"universe_port_b", 15, Fnv1a32("universe_port_b", 15)};
#endif
#if (MAX_ARRAY_SIZE > 2)
    static constexpr json::PortKey kUniversePortC{"universe_port_c", 15, Fnv1a32("universe_port_c", 15)};
#endif
#if (MAX_ARRAY_SIZE == 4)
    static constexpr json::PortKey kUniversePortD{"universe_port_d", 15, Fnv1a32("universe_port_d", 15)};
#endif

    static constexpr json::PortKey kUniversePort[] = 
    {
        kUniversePortA,
#if (MAX_ARRAY_SIZE > 1)
        kUniversePortB,
#endif
#if (MAX_ARRAY_SIZE > 2)
        kUniversePortC,
#endif
#if (MAX_ARRAY_SIZE == 4)
        kUniversePortD,
#endif
    };

    static constexpr json::PortKey kDirectionPortA{"direction_port_a", 16, Fnv1a32("direction_port_a", 16)};
#if (MAX_ARRAY_SIZE > 1)
    static constexpr json::PortKey kDirectionPortB{"direction_port_b", 16, Fnv1a32("direction_port_b", 16)};
#endif
#if (MAX_ARRAY_SIZE > 2)
    static constexpr json::PortKey kDirectionPortC{"direction_port_c", 16, Fnv1a32("direction_port_c", 16)};
#endif
#if (MAX_ARRAY_SIZE == 4)
    static constexpr json::PortKey kDirectionPortD{"direction_port_d", 16, Fnv1a32("direction_port_d", 16)};
#endif

    static constexpr json::PortKey kDirectionPort[] = 
    {
        kDirectionPortA,
#if (MAX_ARRAY_SIZE > 1)
        kDirectionPortB,
#endif
#if (MAX_ARRAY_SIZE > 2)
        kDirectionPortC,
#endif
#if (MAX_ARRAY_SIZE == 4)
        kDirectionPortD,
#endif
    };

    static constexpr json::PortKey kMergeModePortA{"merge_mode_port_a", 17, Fnv1a32("merge_mode_port_a", 17)};
#if (MAX_ARRAY_SIZE > 1)
    static constexpr json::PortKey kMergeModePortB{"merge_mode_port_b", 17, Fnv1a32("merge_mode_port_b", 17)};
#endif
#if (MAX_ARRAY_SIZE > 2)
    static constexpr json::PortKey kMergeModePortC{"merge_mode_port_c", 17, Fnv1a32("merge_mode_port_c", 17)};
#endif
#if (MAX_ARRAY_SIZE == 4)
    static constexpr json::PortKey kMergeModePortD{"merge_mode_port_d", 17, Fnv1a32("merge_mode_port_d", 17)};
#endif

    static constexpr json::PortKey kMergeModePort[] = 
    {
		kMergeModePortA,
#if (MAX_ARRAY_SIZE > 1)
        kMergeModePortB,
#endif
#if (MAX_ARRAY_SIZE > 2)
        kMergeModePortC,
#endif
#if (MAX_ARRAY_SIZE == 4)
        kMergeModePortD,
#endif
    };

    static constexpr json::PortKey kOutputStylePortA{"output_style_a", 14, Fnv1a32("output_style_a", 14)};
#if (MAX_ARRAY_SIZE > 1)
    static constexpr json::PortKey kOutputStylePortB{"output_style_b", 14, Fnv1a32("output_style_b", 14)};
#endif
#if (MAX_ARRAY_SIZE > 2)
    static constexpr json::PortKey kOutputStylePortC{"output_style_c", 14, Fnv1a32("output_style_c", 14)};
#endif
#if (MAX_ARRAY_SIZE == 4)
    static constexpr json::PortKey kOutputStylePortD{"output_style_d", 14, Fnv1a32("output_style_d", 14)};
#endif

    static constexpr json::PortKey kOutputStylePort[] = 
    {
        kOutputStylePortA,
#if (MAX_ARRAY_SIZE > 1)
        kOutputStylePortB,
#endif
#if (MAX_ARRAY_SIZE > 2)
        kOutputStylePortC,
#endif
#if (MAX_ARRAY_SIZE == 4)
        kOutputStylePortD,
#endif
    };
};
} // namespace json

#endif  // JSON_DMXNODEPARAMSCONST_H_
