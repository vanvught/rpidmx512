/**
 * @file dmxnodeoutputtypenull.h
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

#ifndef DMXNODEOUTPUTTYPENULL_H_
#define DMXNODEOUTPUTTYPENULL_H_

#include <cstdint>

#include "dmxnode.h"

class DmxNodeOutputTypeNull {
public:
	void Start([[maybe_unused]] uint32_t port_index) {}
	void Stop([[maybe_unused]] uint32_t port_index) {}

	template <bool doUpdate>
	void SetData([[maybe_unused]] uint32_t port_index, [[maybe_unused]] const uint8_t *data, [[maybe_unused]] uint32_t length) {}

    void Sync([[maybe_unused]] uint32_t port_index) {}
    void Sync() {}

	uint32_t GetUserData() { return 0; }		///< Art-Net ArtPollReply
	uint32_t GetRefreshRate() { return 0; }		///< Art-Net ArtPollReply

	void Blackout([[maybe_unused]] bool blackout) {}
	void FullOn() {}

	bool SetDmxStartAddress([[maybe_unused]] uint16_t dmx_start_address) {
		return false;
	}

	uint16_t GetDmxStartAddress() {
		return dmxnode::kStartAddressDefault;
	}

	uint16_t GetDmxFootprint() {
		return dmxnode::kUniverseSize;
	}

	bool GetSlotInfo([[maybe_unused]] uint16_t slot_offset, dmxnode::SlotInfo &slot_info) {
		slot_info.type = 0x00; // ST_PRIMARY
		slot_info.category = 0x0001; // SD_INTENSITY
		return true;
	}
};

#endif  // DMXNODEOUTPUTTYPENULL_H_
