/**
 * @file lightsetparamsconst.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
	static const char PARAMS_OUTPUT[];

	static const char NODE_LABEL[lightsetparams::MAX_PORTS][14];
	static const char NODE_LONG_NAME[];

	static const char UNIVERSE_PORT[lightsetparams::MAX_PORTS][16];
	static const char MERGE_MODE_PORT[lightsetparams::MAX_PORTS][18];
	static const char DIRECTION[lightsetparams::MAX_PORTS][18];
	static const char OUTPUT_STYLE[lightsetparams::MAX_PORTS][16];
	static const char PRIORITY[lightsetparams::MAX_PORTS][16];

	static const char DMX_START_ADDRESS[];
	static const char DMX_SLOT_INFO[];

	static const char DISABLE_MERGE_TIMEOUT[];

	static const char FAILSAFE[];

#if defined (CONFIG_PIXELDMX_MAX_PORTS)
	static const char START_UNI_PORT[CONFIG_PIXELDMX_MAX_PORTS][20];
#endif
};

#endif /* LIGHTSETPARAMSCONST_H_ */
