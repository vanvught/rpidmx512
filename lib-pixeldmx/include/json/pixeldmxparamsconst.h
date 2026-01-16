/**
 * @file pixeldmxparamsconst.h
 *
 */

#ifndef JSON_PIXELDMXPARAMSCONST_H_
#define JSON_PIXELDMXPARAMSCONST_H_

#include "json/json_key.h"

namespace json
{
struct PixelDmxParamsConst
{
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
	
	static constexpr json::PortKey kStartUniPort1
	{
		"start_uni_port_1", 
		16, 
		Fnv1a32("start_uni_port_1", 16)
	};
#if (CONFIG_DMXNODE_PIXEL_MAX_PORTS > 1)
	static constexpr json::PortKey kStartUniPort2
	{
		"start_uni_port_2", 
		16, 
		Fnv1a32("start_uni_port_2", 16)
	};
	
	static constexpr json::PortKey kStartUniPort3
	{
		"start_uni_port_3", 
		16, 
		Fnv1a32("start_uni_port_3", 16)
	};
	
	static constexpr json::PortKey kStartUniPort4
	{
		"start_uni_port_4", 
		16, 
		Fnv1a32("start_uni_port_4", 16)
	};
	
	static constexpr json::PortKey kStartUniPort5
	{
		"start_uni_port_5", 
		16, 
		Fnv1a32("start_uni_port_5", 16)
	};

	static constexpr json::PortKey kStartUniPort6
	{
		"start_uni_port_6", 
		16, 
		Fnv1a32("start_uni_port_6", 16)
	};
	
	static constexpr json::PortKey kStartUniPort7
	{
		"start_uni_port_7", 
		16, 
		Fnv1a32("start_uni_port_7", 16)
	};
	
	static constexpr json::PortKey kStartUniPort8
	{
		"start_uni_port_8", 
		16, 
		Fnv1a32("start_uni_port_8", 16)
	};
#endif
#if CONFIG_DMXNODE_PIXEL_MAX_PORTS == 16
	static constexpr json::PortKey kStartUniPort9
	{
		"start_uni_port_9", 
		16, 
		Fnv1a32("start_uni_port_9", 16)
	};

	static constexpr json::PortKey kStartUniPort10
	{
		"start_uni_port_10", 
		17, 
		Fnv1a32("start_uni_port_10", 17)
	};
	
	static constexpr json::PortKey kStartUniPort11
	{
		"start_uni_port_11", 
		17, 
		Fnv1a32("start_uni_port_11", 17)
	};
	
	static constexpr json::PortKey kStartUniPort12
	{
		"start_uni_port_12", 
		17, 
		Fnv1a32("start_uni_port_12", 17)
	};
	
	static constexpr json::PortKey kStartUniPort13
	{
		"start_uni_port_13", 
		17, 
		Fnv1a32("start_uni_port_13", 17)
	};

	static constexpr json::PortKey kStartUniPort14
	{
		"start_uni_port_14", 
		17, 
		Fnv1a32("start_uni_port_14", 17)
	};
	
	static constexpr json::PortKey kStartUniPort15
	{
		"start_uni_port_15", 
		17, 
		Fnv1a32("start_uni_port_15", 17)
	};
	
	static constexpr json::PortKey kStartUniPort16
	{
		"start_uni_port_16", 
		17, 
		Fnv1a32("start_uni_port_16", 17)
	};
#endif

    static constexpr json::PortKey kStartUniPort[] = 
    {
		kStartUniPort1,
#if (CONFIG_DMXNODE_PIXEL_MAX_PORTS > 1)
		kStartUniPort2,
		kStartUniPort3,
		kStartUniPort4,
		kStartUniPort5,
		kStartUniPort6,
		kStartUniPort7,
		kStartUniPort8,
#if CONFIG_DMXNODE_PIXEL_MAX_PORTS == 16
		kStartUniPort9,
		kStartUniPort10,
		kStartUniPort11,
		kStartUniPort12,
		kStartUniPort13,
		kStartUniPort14,
		kStartUniPort15,
		kStartUniPort16
#endif
#endif
    };
};
} // namespace json

#endif  // JSON_PIXELDMXPARAMSCONST_H_
