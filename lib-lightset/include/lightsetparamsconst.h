/**
 * @file lightsetparamsconst.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LIGHTSETPARAMSCONST_H_
#define LIGHTSETPARAMSCONST_H_

#include <cstdint>

namespace lightsetparams {
static constexpr uint32_t MAX_PORTS = 4;
}  // namespace lightsetparams

struct LightSetParamsConst {
	static inline const char PARAMS_OUTPUT[] = "output";

	static inline const char UNIVERSE_PORT[lightsetparams::MAX_PORTS][16] = {
			"universe_port_a",
			"universe_port_b",
			"universe_port_c",
			"universe_port_d"
	};

	static inline const char MERGE_MODE_PORT[lightsetparams::MAX_PORTS][18] = {
			"merge_mode_port_a",
			"merge_mode_port_b",
			"merge_mode_port_c",
			"merge_mode_port_d"
	};

	static inline const char DIRECTION[lightsetparams::MAX_PORTS][18] = {
			"direction_port_a",
			"direction_port_b",
			"direction_port_c",
			"direction_port_d"
	};

	static inline const char OUTPUT_STYLE[lightsetparams::MAX_PORTS][16] = {
			"output_style_a",
			"output_style_b",
			"output_style_c",
			"output_style_d"
	};

	static inline const char PRIORITY[lightsetparams::MAX_PORTS][16] = {
			"priority_port_a",
			"priority_port_b",
			"priority_port_c",
			"priority_port_d"
	};

	static inline const char NODE_LABEL[lightsetparams::MAX_PORTS][14] = {
			"label_port_a",
			"label_port_b",
			"label_port_c",
			"label_port_d"
	};

	static inline const char NODE_LONG_NAME[] = "long_name";

	static inline const char DMX_START_ADDRESS[] = "dmx_start_address";
	static inline const char DMX_SLOT_INFO[] = "dmx_slot_info";

	static inline const char DISABLE_MERGE_TIMEOUT[] = "disable_merge_timeout";

	static inline const char FAILSAFE[] = "failsafe";

#if defined (CONFIG_PIXELDMX_MAX_PORTS)
	static inline const char START_UNI_PORT[CONFIG_PIXELDMX_MAX_PORTS][20] = {
			"start_uni_port_1",
# if CONFIG_PIXELDMX_MAX_PORTS > 2
			"start_uni_port_2",
			"start_uni_port_3",
			"start_uni_port_4",
			"start_uni_port_5",
			"start_uni_port_6",
			"start_uni_port_7",
			"start_uni_port_8",
# endif
# if CONFIG_PIXELDMX_MAX_PORTS == 16
			"start_uni_port_9",
			"start_uni_port_10",
			"start_uni_port_11",
			"start_uni_port_12",
			"start_uni_port_13",
			"start_uni_port_14",
			"start_uni_port_15",
			"start_uni_port_16"
# endif
	};
#endif
};

#endif /* LIGHTSETPARAMSCONST_H_ */
