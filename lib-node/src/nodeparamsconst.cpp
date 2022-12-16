/**
 * @file nodeparamsconst.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "nodeparamsconst.h"
#include "nodeparams.h"
#include "node.h"

#ifdef MAX_ARRAY
# undef MAX_ARRAY
#endif

#if LIGHTSET_PORTS > 8
# define MAX_ARRAY 4
#else
# define MAX_ARRAY LIGHTSET_PORTS
#endif

static_assert(nodeparams::MAX_PORTS == MAX_ARRAY, "nodeparams::MAX_PORTS != MAX_ARRAY");

const char NodeParamsConst::FILE_NAME[static_cast<uint32_t>(node::Personality::UNKNOWN)][12] = {
		"node.txt",
		"artnet.txt",
		"e131.txt" };

const char NodeParamsConst::PERSONALITY[] = "personality";
const char NodeParamsConst::FAILSAFE[] = "failsafe";

const char NodeParamsConst::UNIVERSE_PORT[nodeparams::MAX_PORTS][16] = {
		"universe_port_a",
#if MAX_ARRAY >= 2
		"universe_port_b",
#endif
#if MAX_ARRAY >= 3
		"universe_port_c",
#endif
#if MAX_ARRAY >= 4
		"universe_port_d",
#endif
#if MAX_ARRAY >= 5
		"universe_port_e",
#endif
#if MAX_ARRAY >= 6
		"universe_port_f",
#endif
#if MAX_ARRAY >= 7
		"universe_port_g",
#endif
#if MAX_ARRAY == 8
		"universe_port_h"
#endif
};

const char NodeParamsConst::MERGE_MODE_PORT[nodeparams::MAX_PORTS][18] = {
		"merge_mode_port_a",
#if MAX_ARRAY >= 2
		"merge_mode_port_b",
#endif
#if MAX_ARRAY >= 3
		"merge_mode_port_c",
#endif
#if MAX_ARRAY >= 4
		"merge_mode_port_d",
#endif
#if MAX_ARRAY >= 5
		"merge_mode_port_e",
#endif
#if MAX_ARRAY >= 6
		"merge_mode_port_f",
#endif
#if MAX_ARRAY >= 7
		"merge_mode_port_g",
#endif
#if MAX_ARRAY == 8
		"merge_mode_port_h"
#endif
};

const char NodeParamsConst::DIRECTION[nodeparams::MAX_PORTS][18] = {
		"direction_port_a",
#if MAX_ARRAY > 1
		"direction_port_b",
#endif
#if MAX_ARRAY > 2
		"direction_port_c",
#endif
#if MAX_ARRAY > 3
		"direction_port_d",
#endif
#if MAX_ARRAY > 4
		"direction_port_e",
#endif
#if MAX_ARRAY > 5
		"direction_port_f",
#endif
#if MAX_ARRAY > 6
		"direction_port_g",
#endif
#if MAX_ARRAY > 7
		"direction_port_h"
#endif
};

/**
 * Art-Net 4
 */

const char NodeParamsConst::PROTOCOL_PORT[nodeparams::MAX_PORTS][16] = {
		"protocol_port_a",
#if MAX_ARRAY >= 2
		"protocol_port_b",
#endif
#if MAX_ARRAY >= 3
		"protocol_port_c",
#endif
#if MAX_ARRAY >= 4
		"protocol_port_d",
#endif
#if MAX_ARRAY >= 5
		"protocol_port_e",
#endif
#if MAX_ARRAY >= 6
		"protocol_port_f",
#endif
#if MAX_ARRAY >= 7
		"protocol_port_g",
#endif
#if MAX_ARRAY == 8
		"protocol_port_h"
#endif
};

const char NodeParamsConst::DESTINATION_IP_PORT[nodeparams::MAX_PORTS][24] = {
		"destination_ip_port_a",
#if MAX_ARRAY >= 2
		"destination_ip_port_b",
#endif
#if MAX_ARRAY >= 3
		"destination_ip_port_c",
#endif
#if MAX_ARRAY >= 4
		"destination_ip_port_d",
#endif
#if MAX_ARRAY >= 5
		"destination_ip_port_e",
#endif
#if MAX_ARRAY >= 6
		"destination_ip_port_f",
#endif
#if MAX_ARRAY >= 7
		"destination_ip_port_g",
#endif
#if MAX_ARRAY == 8
		"destination_ip_port_h"
#endif
};

const char NodeParamsConst::RDM_ENABLE_PORT[nodeparams::MAX_PORTS][18] = {
		"rdm_enable_port_a",
#if MAX_ARRAY >= 2
		"rdm_enable_port_b",
#endif
#if MAX_ARRAY >= 3
		"rdm_enable_port_c",
#endif
#if MAX_ARRAY >= 4
		"rdm_enable_port_d",
#endif
#if MAX_ARRAY >= 5
		"rdm_enable_port_e",
#endif
#if MAX_ARRAY >= 6
		"rdm_enable_port_f",
#endif
#if MAX_ARRAY >= 7
		"rdm_enable_port_g",
#endif
#if MAX_ARRAY == 8
		"rdm_enable_port_h"
#endif
};

const char NodeParamsConst::MAP_UNIVERSE0[] = "map_universe0";
const char NodeParamsConst::ENABLE_RDM[] = "enable_rdm";
const char NodeParamsConst::NODE_SHORT_NAME[] = "short_name";
const char NodeParamsConst::NODE_LONG_NAME[] = "long_name";

/**
 * sACN E1.31
 */

const char NodeParamsConst::PRIORITY[nodeparams::MAX_PORTS][18] {
		"priority_port_a",
#if MAX_ARRAY >= 2
		"priority_port_b",
#endif
#if MAX_ARRAY >= 3
		"priority_port_c",
#endif
#if MAX_ARRAY >= 4
		"priority_port_d",
#endif
#if MAX_ARRAY >= 5
		"priority_port_e",
#endif
#if MAX_ARRAY >= 6
		"priority_port_f",
#endif
#if MAX_ARRAY >= 7
		"priority_port_g",
#endif
#if MAX_ARRAY == 8
		"priority_port_h"
#endif
};

/**
 * Extra's
 */

const char NodeParamsConst::DISABLE_MERGE_TIMEOUT[] = "disable_merge_timeout";
