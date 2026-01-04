/**
 * @file json_config_pca9685dmx.cpp
 *
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

#include <cstdint>

#include "json/json_helpers.h"
#include "json/pca9685dmxparams.h"
#include "pca9685dmx.h"
#include "json/pca9685dmxparamsconst.h"
#include "pca9685.h"

namespace json::config
{
uint32_t GetPca9685Dmx(char* buffer, uint32_t length)
{
    auto& pca9685_dmx = Pca9685Dmx::Instance();

	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
	    doc[json::Pca9685DmxParamsConst::kMode.name] = pca9685dmx::GetMode(pca9685_dmx.GetMode());
	    doc[json::Pca9685DmxParamsConst::kChannelCount.name] = pca9685_dmx.GetChannelCount();
	    doc[json::Pca9685DmxParamsConst::kDmxStartAddress.name] = pca9685_dmx.GetDmxStartAddress();
		// Led
		doc[json::Pca9685DmxParamsConst::kLedPwmFrequency.name] = pca9685_dmx.GetLedPwmFrequency();
	    doc[json::Pca9685DmxParamsConst::kUse8Bit.name] = static_cast<uint32_t>(pca9685_dmx.IsUse8Bit());
	    doc[json::Pca9685DmxParamsConst::kLedOutputInvert.name] = static_cast<uint32_t>(pca9685_dmx.GetLedOutputInvert() == pca9685::Invert::kOutputInverted);
	    doc[json::Pca9685DmxParamsConst::kLedOutputOpendrain.name] = static_cast<uint32_t>(pca9685_dmx.GetLedOutputDriver() == pca9685::Output::kDriverOpendrain);
		// Servo
	    doc[json::Pca9685DmxParamsConst::kServoLeftUs.name] = pca9685_dmx.GetServoLeftUs();
	    doc[json::Pca9685DmxParamsConst::kServoCenterUs.name] = pca9685_dmx.GetServoCenterUs();
	    doc[json::Pca9685DmxParamsConst::kServoRightUs.name] = pca9685_dmx.GetServoRightUs();
    });
}

void SetPca9685Dmx(const char* buffer, uint32_t buffer_size)
{
    ::json::Pca9685DmxParams pca9685dmx_params;
    pca9685dmx_params.Store(buffer, buffer_size);
    pca9685dmx_params.Set();
}
} // namespace json::config