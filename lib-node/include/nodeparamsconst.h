/**
 * @file nodeparamsconst.h
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

#ifndef NODEPARAMSCONST_H_
#define NODEPARAMSCONST_H_

#include <cstdint>

#include "node.h"
#include "nodeparams.h"

struct NodeParamsConst {
	static const char FILE_NAME[static_cast<uint32_t>(node::Personality::UNKNOWN)][12];

	/**
	 * Node
	 */

	static const char PERSONALITY[];
	static const char FAILSAFE[];

	static const char UNIVERSE_PORT[nodeparams::MAX_PORTS][16];
	static const char MERGE_MODE_PORT[nodeparams::MAX_PORTS][18];
	static const char DIRECTION[nodeparams::MAX_PORTS][18];

	/**
	 * Art-Net 4
	 */

	static const char PROTOCOL_PORT[nodeparams::MAX_PORTS][16];
	static const char DESTINATION_IP_PORT[nodeparams::MAX_PORTS][24];
	static const char RDM_ENABLE_PORT[nodeparams::MAX_PORTS][18];
	static const char MAP_UNIVERSE0[];
	static const char ENABLE_RDM[];
	static const char NODE_SHORT_NAME[];
	static const char NODE_LONG_NAME[];

	/**
	 * sACN E1.31
	 */

	static const char PRIORITY[nodeparams::MAX_PORTS][18];

	/**
	 * Extra's
	 */

	static const char DISABLE_MERGE_TIMEOUT[];

};

#endif /* NODEPARAMSCONST_H_ */
