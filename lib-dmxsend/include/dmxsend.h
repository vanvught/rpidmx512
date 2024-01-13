/**
 * @file dmxsend.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DMXSEND_H_
#define DMXSEND_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "lightset.h"
#include "lightsetdata.h"

#include "dmx.h"
#include "panel_led.h"
#include "hardware.h"

#include "debug.h"

class DmxSend final: public LightSet  {
public:
	void Start(const uint32_t nPortIndex) override;
	void Stop(const uint32_t nPortIndex) override;
	void SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate) override;
	void Sync(uint32_t const nPortIndex) override;
	void Sync(const bool doForce) override;

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle(const uint32_t nPortIndex, const lightset::OutputStyle outputStyle) override {
		Dmx::Get()->SetOutputStyle(nPortIndex, outputStyle == lightset::OutputStyle::CONSTANT ? dmx::OutputStyle::CONTINOUS : dmx::OutputStyle::DELTA);
	}

	lightset::OutputStyle GetOutputStyle(const uint32_t nPortIndex) const override {
		return Dmx::Get()->GetOutputStyle(nPortIndex) == dmx::OutputStyle::CONTINOUS ? lightset::OutputStyle::CONSTANT : lightset::OutputStyle::DELTA;
	}
#endif

	void Blackout(__attribute__((unused)) bool bBlackout) override {
		Dmx::Get()->Blackout();
	}

	void FullOn() override {
		Dmx::Get()->FullOn();
	}

	void Print() override;

private:
	uint8_t m_nStarted { 0 };
};

#endif /* DMXSEND_H_ */
