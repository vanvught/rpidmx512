/**
 * @file artnetdisplay.cpp
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>

#include "lightset.h"
#include "artnet.h"

#include "debug.h"

namespace artnet {
void display_longname([[maybe_unused]] const char *pLongName) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void display_universe_switch([[maybe_unused]]  uint32_t nPortIndex, [[maybe_unused]]  uint8_t nAddress) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void display_net_switch([[maybe_unused]]  uint8_t nAddress) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void display_subnet_switch([[maybe_unused]]  uint8_t nAddress) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void display_merge_mode([[maybe_unused]]  uint32_t nPortIndex, [[maybe_unused]]  lightset::MergeMode mergeMode) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void display_outputstyle([[maybe_unused]] const uint32_t nPortIndex, [[maybe_unused]] const lightset::OutputStyle outputStyle) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void display_port_protocol([[maybe_unused]]  uint32_t nPortIndex, [[maybe_unused]]  artnet::PortProtocol tPortProtocol) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void display_rdm_enabled([[maybe_unused]] uint32_t nPortIndex, [[maybe_unused]] bool isEnabled) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void display_failsafe([[maybe_unused]] uint8_t nFailsafe) {
	DEBUG_ENTRY
	DEBUG_EXIT
}
}  // namespace artnet
