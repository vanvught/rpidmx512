/**
 * @file ltcoscserver.h
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTCOSCSERVER_H_
#define LTCOSCSERVER_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#if !(defined(CONFIG_LTC_DISABLE_RGB_PANEL) && defined (CONFIG_LTC_DISABLE_WS28XX))
# include "ltcdisplayrgb.h"
#else
# define LTC_NO_DISPLAY_RGB
#endif
#include "osc.h"
#include "emac/network.h"
#include "firmware/debug/debug_debug.h"

namespace ltcoscserver {
static constexpr uint32_t kPathLengthMax =128;
}  // namespace ltcoscserver

class LtcOscServer {
public:
	LtcOscServer(): port_incoming_(osc::port::DEFAULT_INCOMING) {
		DEBUG_ENTRY();

		assert(s_this == nullptr);
		s_this = this;

		path_length_ = static_cast<uint32_t>(snprintf(path_, sizeof(path_) - 1, "/%s/tc/*",  network::iface::GetHostName()) - 1);

		DEBUG_PRINTF("%d [%s]", path_length_, path_);
		DEBUG_EXIT();
	}

	void Start() {
		DEBUG_ENTRY();

		assert(handle_ == -1);
		handle_ = network::udp::Begin(port_incoming_, StaticCallbackFunction);
		assert(handle_ != -1);

		DEBUG_EXIT();
	}

	void Print() {
		puts("OSC Server");
		printf(" Port : %u\n", port_incoming_);
		printf(" Path : [%s]\n", path_);
	}

	void SetPortIncoming(uint16_t port_incoming) {
		port_incoming_ = port_incoming;
	}

	uint16_t GetPortIncoming() const {
		return port_incoming_;
	}

	void Input(const uint8_t *buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

private:
#if !defined(LTC_NO_DISPLAY_RGB)
	void SetWS28xxRGB(const uint8_t *, uint32_t, ltc::display::rgb::ColourIndex);
#endif

	void static StaticCallbackFunction(const uint8_t *buffer, uint32_t size, uint32_t from_ip, uint16_t from_port) {
		s_this->Input(buffer, size, from_ip, from_port);
	}

private:
	uint16_t port_incoming_;
	uint16_t remote_port_ { 0 };
	int32_t handle_ { -1 };
	uint32_t remote_ip_ { 0 };
	uint32_t path_length_ { 0 };
	char path_[ltcoscserver::kPathLengthMax];

	static inline LtcOscServer *s_this;
};

#endif  // LTCOSCSERVER_H_
