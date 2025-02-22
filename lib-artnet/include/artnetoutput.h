/**
 * @file artnetoutput.h
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

#ifndef ARTNETOUTPUT_H_
#define ARTNETOUTPUT_H_

#include <cstdint>

#include "e131sync.h"

#include "dmxnode.h"

class ArtNetOutput: public E131Sync {
public:
	ArtNetOutput();

	void Handler() override;

	void Start(const uint32_t nPortIndex);
	void Stop(const uint32_t nPortIndex);

	void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true);
	void Sync(const uint32_t nPortIndex);
	void Sync();

	void Blackout([[maybe_unused]] bool bBlackout) {}
	void FullOn() {}

	bool SetDmxStartAddress([[maybe_unused]] const uint16_t nDmxStartAddress) {
		return false;
	}

	uint16_t GetDmxStartAddress() {
		return dmxnode::START_ADDRESS_DEFAULT;
	}

	uint16_t GetDmxFootprint() {
		return dmxnode::UNIVERSE_SIZE;
	}

	bool GetSlotInfo([[maybe_unused]] const uint16_t nSlotOffset, dmxnode::SlotInfo &slotInfo) {
		slotInfo.nType = 0x00; // ST_PRIMARY
		slotInfo.nCategory = 0x0001; // SD_INTENSITY
		return true;
	}

private:
	uint16_t m_nUniverse[DMXNODE_PORTS];
};

#endif /* ARTNETOUTPUT_H_ */
