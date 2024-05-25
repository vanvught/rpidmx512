/**
 * @file dmxstatistics.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DMXSTATISTICS_H_
#define DMXSTATISTICS_H_

#include <cstdint>

namespace dmx {
struct TotalStatistics {
	struct  {
		uint32_t Sent;
		uint32_t Received;
	} Dmx;

	struct {
		struct  {
			uint32_t Good;
			uint32_t Bad;
			uint32_t DiscoveryResponse;
		} Received;
		struct  {
			uint32_t Class;
			uint32_t DiscoveryResponse;
		} Sent;
	} Rdm;
};
}  // namespace dmx

#endif /* DMXSTATISTICS_H_ */
