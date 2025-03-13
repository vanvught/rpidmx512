/**
 * @file pixeldmxparamsconst.h
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

#ifndef PIXELDMXPARAMSCONST_H_
#define PIXELDMXPARAMSCONST_H_

struct PixelDmxParamsConst {
#if defined (CONFIG_DMXNODE_PIXEL_MAX_PORTS)
	static inline const char START_UNI_PORT[CONFIG_DMXNODE_PIXEL_MAX_PORTS][20] = {
			"start_uni_port_1",
# if CONFIG_DMXNODE_PIXEL_MAX_PORTS > 2
			"start_uni_port_2",
			"start_uni_port_3",
			"start_uni_port_4",
			"start_uni_port_5",
			"start_uni_port_6",
			"start_uni_port_7",
			"start_uni_port_8",
# endif
# if CONFIG_DMXNODE_PIXEL_MAX_PORTS == 16
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

#endif /* PIXELDMXPARAMSCONST_H_ */
