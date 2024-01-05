/**
 * @file midimonitor.h
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

#ifndef MIDIMONITOR_H_
#define MIDIMONITOR_H_

#include <stdint.h>

#include "midi.h"
#include "midibpm.h"

class MidiMonitor {
public:
	MidiMonitor();

	void Init();

	void Run() {
		HandleMessage();
		ShowActiveSenseAndUpdatesPerSecond();
	}

private:
	void HandleMessage();
	void ShowActiveSenseAndUpdatesPerSecond();
	void HandleMtc();
	void HandleQf();
	void UpdateTimecode(uint8_t nType);

private:
	uint32_t m_nMillisPrevious { 0 };
	struct midi::Message *m_pMidiMessage { nullptr };
	uint32_t m_nInitTimestamp { 0 };
	uint8_t m_nTypePrevious { 0xFF };
	uint8_t m_nPartPrevious { 0 };
	bool m_bDirection { true };
	MidiBPM m_MidiBPM;
	uint32_t m_nBPM { 0 };
};

#endif /* MIDIMONITOR_H_ */
