/**
 * @file dmxnodeparamsconst.h
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

#ifndef DMXNODEPARAMSCONST_H_
#define DMXNODEPARAMSCONST_H_

#include "dmxnode.h"
#include "dmxnode_nodetype.h"
#include "dmxnode_outputtype.h"

#if defined (DMXNODE_OUTPUT_DMX)
# include "dmx.h"
# define MAX_ARRAY_SIZE	DMX_MAX_PORTS
#else
# if !defined(DMXNODE_PARAM_PORTS)
#  define MAX_ARRAY_SIZE 4
# else
#  define MAX_ARRAY_SIZE DMXNODE_PARAM_PORTS
# endif
#endif

struct DmxNodeParamsConst {
	enum class FileName {
		FILE_NAME_DMXNODE,
		FILE_NAME_ARTNET,
		FILE_NAME_E131
	};
	static inline const char *FILE_NAME[] = { "dmxnode.txt", "artnet.txt", "e131.txt" };

	static inline const char PERSONALITY[] = "personality";

	static inline const char *UNIVERSE_PORT[] = {
			"universe_port_a",
#if (MAX_ARRAY_SIZE > 1)
			"universe_port_b",
#endif
#if (MAX_ARRAY_SIZE > 2)
			"universe_port_c",
#endif
#if (MAX_ARRAY_SIZE > 3)
			"universe_port_d",
#endif
#if (MAX_ARRAY_SIZE == 8)
			"universe_port_e",
			"universe_port_f",
			"universe_port_g",
			"universe_port_h"
#endif
	};

	static inline const char *DIRECTION_PORT[] = {
			"direction_port_a",
#if (MAX_ARRAY_SIZE > 1)
			"direction_port_b",
#endif
#if (MAX_ARRAY_SIZE > 2)
			"direction_port_c",
#endif
#if (MAX_ARRAY_SIZE > 3)
			"direction_port_d",
#endif
#if (MAX_ARRAY_SIZE == 8)
			"direction_port_e",
			"direction_port_f",
			"direction_port_g",
			"direction_port_h"
#endif
	};

	static inline const char *MERGE_MODE_PORT[] = {
			"merge_mode_port_a",
#if (MAX_ARRAY_SIZE > 1)
			"merge_mode_port_b",
#endif
#if (MAX_ARRAY_SIZE > 2)
			"merge_mode_port_c",
#endif
#if (MAX_ARRAY_SIZE > 3)
			"merge_mode_port_d",
#endif
#if (MAX_ARRAY_SIZE == 8)
			"merge_mode_port_e",
			"merge_mode_port_f",
			"merge_mode_port_g",
			"merge_mode_port_h"
#endif
	};

	static inline const char *OUTPUT_STYLE_PORT[] = {
			"output_style_a",
#if (MAX_ARRAY_SIZE > 1)
			"output_style_b",
#endif
#if (MAX_ARRAY_SIZE > 2)
			"output_style_c",
#endif
#if (MAX_ARRAY_SIZE > 3)
			"output_style_d",
#endif
#if (MAX_ARRAY_SIZE == 8)
			"output_style_e",
			"output_style_f",
			"output_style_g",
			"output_style_h"
#endif
	};

	static inline const char FAILSAFE[] = "failsafe";

	static inline const char NODE_NAME[] = "long_name";

	static inline const char *LABEL_PORT[] = {
			"label_port_a",
#if (MAX_ARRAY_SIZE > 1)
			"label_port_b",
#endif
#if (MAX_ARRAY_SIZE > 2)
			"label_port_c",
#endif
#if (MAX_ARRAY_SIZE > 3)
			"label_port_d",
#endif
#if (MAX_ARRAY_SIZE == 8)
			"label_port_e",
			"label_port_f",
			"label_port_g",
			"label_port_h"
#endif
	};

	/*
	 * RDM
	 */

	static inline const char DMX_START_ADDRESS[] = "dmx_start_address";
	static inline const char DMX_SLOT_INFO[] = "dmx_slot_info";

	/*
	 * Extra's
	 */

	static inline const char DISABLE_MERGE_TIMEOUT[] = "disable_merge_timeout";

	/*
	 * Art-Net
	 */
#if defined (DMXNODE_TYPE_ARTNETNODE) || defined(NODE_NODE)
	static inline const char *DESTINATION_IP_PORT[] = {
			"destination_ip_port_a",
#if (MAX_ARRAY_SIZE > 1)
			"destination_ip_port_b",
#endif
#if (MAX_ARRAY_SIZE > 2)
			"destination_ip_port_c",
#endif
#if (MAX_ARRAY_SIZE > 3)
			"destination_ip_port_d",
#endif
#if (MAX_ARRAY_SIZE == 8)
			"destination_ip_port_e",
			"destination_ip_port_f",
			"destination_ip_port_g",
			"destination_ip_port_h"
#endif
	};

#if (ARTNET_VERSION >= 4)
	static inline const char MAP_UNIVERSE0[] = "map_universe0";
	static inline const char ENABLE_RDM[] = "enable_rdm";

	static inline const char *RDM_ENABLE_PORT[] = {
				"rdm_enable_port_a",
#if (MAX_ARRAY_SIZE > 1)
				"rdm_enable_port_b",
#endif
#if (MAX_ARRAY_SIZE > 2)
				"rdm_enable_port_c",
#endif
#if (MAX_ARRAY_SIZE > 3)
				"rdm_enable_port_d",
#endif
#if (MAX_ARRAY_SIZE == 8)
				"rdm_enable_port_e",
				"rdm_enable_port_f",
				"rdm_enable_port_g",
				"rdm_enable_port_h"
#endif
	};

	static inline const char *PROTOCOL_PORT[] = {
				"protocol_port_a",
#if (MAX_ARRAY_SIZE > 1)
				"protocol_port_b",
#endif
#if (MAX_ARRAY_SIZE > 2)
				"protocol_port_c",
#endif
#if (MAX_ARRAY_SIZE > 3)
				"protocol_port_d",
#endif
#if (MAX_ARRAY_SIZE == 8)
				"protocol_port_e",
				"protocol_port_f",
				"protocol_port_g",
				"protocol_port_h"
#endif
	};
#endif
#endif

	/*
	 * sACN
	 */
#if defined (DMXNODE_TYPE_E131BRIDGE) || (ARTNET_VERSION >= 4)
	static inline const char *PRIORITY_PORT[] {
			"priority_port_a",
#if (MAX_ARRAY_SIZE > 1)
			"priority_port_b",
#endif
#if (MAX_ARRAY_SIZE > 2)
			"priority_port_c",
#endif
#if (MAX_ARRAY_SIZE > 3)
			"priority_port_d",
#endif
#if (MAX_ARRAY_SIZE == 8)
			"priority_port_e",
			"priority_port_f",
			"priority_port_g",
			"priority_port_h"
#endif
	};
#endif
};
#endif /* DMXNODEPARAMSCONST_H_ */
