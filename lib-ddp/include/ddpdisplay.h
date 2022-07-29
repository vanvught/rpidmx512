/**
 * @file ddpdisplay.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DDPDISPLAY_H_
#define DDPDISPLAY_H_

#include "ddp.h"
#include "ddpdisplaypixelconfiguration.h"

#include "ws28xxmulti.h"
#include "dmx.h"

namespace ddpdisplay {
namespace configuration {
namespace dmx {
static constexpr auto MAX_PORTS = 2;
}  // namespace dmx
static constexpr auto MAX_PORTS = ddpdisplay::configuration::pixel::MAX_PORTS + configuration::dmx::MAX_PORTS;
}  // namespace configuration
struct PixelPortData {
	uint16_t length;
	uint8_t data[512 * 4];
}__attribute__((packed));

struct DmxPortData {
	uint16_t length;
	uint8_t data[512];
}__attribute__((packed));

struct FrameBuffer {
	struct PixelPortData pixelPortdata[ddpdisplay::configuration::pixel::MAX_PORTS];
	struct DmxPortData dmxPortdata[configuration::dmx::MAX_PORTS];
}__attribute__((packed));
}  // namespace ddpdisplay

class DdpDisplay {
public:
	DdpDisplay(DdpDisplayPixelConfiguration& ddpPixelConfiguration);
	~DdpDisplay();

	void Start();
	void Stop();

	void Run();

	void Print();

	uint32_t GetActivePorts() {
		uint32_t nCount = 0;
		for (uint32_t nPortIndex = 0; nPortIndex < ddpdisplay::configuration::pixel::MAX_PORTS; nPortIndex++) {
			if (s_FrameBuffer.pixelPortdata[nPortIndex].length != 0) {
				nCount++;
			}
		}
		for (uint32_t nPortIndex = 0; nPortIndex < ddpdisplay::configuration::dmx::MAX_PORTS; nPortIndex++) {
			if (s_FrameBuffer.pixelPortdata[nPortIndex].length != 0) {
				nCount++;
			}
		}
		return nCount;
	}

private:
	void HandleQuery();
	void HandleData();

private:
	uint32_t m_nLedsPerPixel;
	WS28xxMulti *m_pWS28xxMulti { nullptr };
	Dmx m_Dmx;
	int32_t m_nHandle { -1 };
	uint32_t m_nFromIp;
	ddp::Packet m_Packet;

	static ddpdisplay::FrameBuffer s_FrameBuffer;
	static uint32_t s_nOffsetCompare[ddpdisplay::configuration::MAX_PORTS];
};

#endif /* DDPDISPLAY_H_ */
