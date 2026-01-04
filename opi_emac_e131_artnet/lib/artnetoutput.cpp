/**
 * @file artnetoutput.cpp
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "artnetoutput.h"
#include "artnetcontroller.h"
#include "dmxnodenode.h"
#include "e131bridge.h"

#include "dmxnode.h"

 #include "firmware/debug/debug_debug.h"

void SyncHandler()
{
    ArtNetController::Get()->HandleSync();
}

ArtNetOutput::ArtNetOutput() {
	DEBUG_ENTRY();
	
	DmxNodeNode::Get()->SetE131Sync(SyncHandler);

	for (uint32_t i = 0; i < dmxnode::kMaxPorts; i++) {
		universe_[i] = 0;
	}

	DEBUG_EXIT();
}

void ArtNetOutput::Start(uint32_t port_index) {
	DEBUG_ENTRY();
	DEBUG_PRINTF("port_index=%u", port_index);

	if (port_index < dmxnode::kMaxPorts) {
		uint16_t universe;

		if (E131Bridge::Get()->GetUniverse(static_cast<uint8_t>(port_index), universe, dmxnode::PortDirection::kOutput)) {
			universe_[port_index] = universe;
			DEBUG_PRINTF("universe_[%u]=%d", port_index, static_cast<int>(universe_[port_index]));
		}
	}

	DEBUG_EXIT();
}

void ArtNetOutput::Stop(uint32_t port_index) {
	DEBUG_ENTRY();

	if (port_index < dmxnode::kMaxPorts) {
		uint16_t universe;

		if (E131Bridge::Get()->GetUniverse(static_cast<uint8_t>(port_index), universe, dmxnode::PortDirection::kOutput)) {
			universe_[port_index] = 0;
			DEBUG_PRINTF("universe_[%d]=0", static_cast<int>(port_index));
		}
	}

	DEBUG_EXIT();
}

void ArtNetOutput::SetDataImpl(uint32_t port_index, const uint8_t *dmx_data, uint32_t length) {
	assert(port_index < dmxnode::kMaxPorts);

	if (universe_[port_index] != 0) {
        ArtNetController::Get()->HandleDmxOut(universe_[port_index], dmx_data, length, static_cast<uint8_t>(port_index));
    }
}

void ArtNetOutput::Sync([[maybe_unused]] uint32_t port_index) {
	//TODO (a) Implement Sync
}

void ArtNetOutput::Sync() {
	DEBUG_ENTRY();

	//TODO (a) Implement Sync

	DEBUG_EXIT();
}
