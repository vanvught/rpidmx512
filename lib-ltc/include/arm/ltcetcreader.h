/**
 * @file ltcetcreader.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARM_LTCETCREADER_H_
#define ARM_LTCETCREADER_H_

#include "ltcetc.h"
#include "midi.h"

#include "ltcoutputs.h"

#include "hardware.h"

#include "arm/platform_ltc.h"

class LtcEtcReader final : public LtcEtcHandler {
public:
	void Start();
	void Stop();

	void Run() {
		__DMB();
		if (gv_ltc_nUpdatesPerSecond != 0) {
			Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
		} else {
			LtcOutputs::Get()->ShowSysTime();
			Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
		}
	}

	void Handler(const midi::Timecode *pTimeCode) override;

private:
	struct midi::Timecode m_MidiTimeCode;
};

#endif /* ARM_LTCETCREADER_H_ */
