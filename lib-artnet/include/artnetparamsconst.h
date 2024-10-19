/**
 * @file artnetparamsconst.h
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

#ifndef ARTNETPARAMSCONST_H_
#define ARTNETPARAMSCONST_H_

#include "artnetparams.h"
#include "artnet.h"

struct ArtNetParamsConst {
	static inline const char FILE_NAME[] = "artnet.txt";

	static inline const char ENABLE_RDM[] = "enable_rdm";

	static inline const char DESTINATION_IP_PORT[artnet::PORTS][24] = {
			"destination_ip_port_a",
			"destination_ip_port_b",
			"destination_ip_port_c",
			"destination_ip_port_d"
	};


	static inline const char RDM_ENABLE_PORT[artnet::PORTS][18] = {
			"rdm_enable_port_a",
			"rdm_enable_port_b",
			"rdm_enable_port_c",
			"rdm_enable_port_d"
	};

	/**
	 * Art-Net 4
	 */

	static inline const char PROTOCOL_PORT[artnet::PORTS][16] = {
			"protocol_port_a",
			"protocol_port_b",
			"protocol_port_c",
			"protocol_port_d"
	};

	static inline const char MAP_UNIVERSE0[] = "map_universe0";
};

#endif /* ARTNETPARAMSCONST_H_ */
