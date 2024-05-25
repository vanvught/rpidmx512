/**
 * @file ltcsender.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>

#include "ltcsender.h"
#include "ltc.h"

#include "h3_codec.h"

#include "debug.h"

LtcSender *LtcSender::s_pThis = nullptr;

LtcSender::LtcSender(uint32_t nVolume) {
	assert(s_pThis == nullptr);
	s_pThis = this;

	if ((nVolume > 1) && (nVolume < 32)) {	// TODO constexpr
		h3_codec_set_volume(static_cast<uint8_t>(nVolume));
	}
}

void LtcSender::Start() {
	h3_codec_begin();
}

void LtcSender::SetTimeCode(const struct ltc::TimeCode* pLtcSenderTimeCode, bool nExternalClock) {
	LtcEncoder::SetTimeCode(pLtcSenderTimeCode, nExternalClock);
	LtcEncoder::Get()->Encode();

	if (__builtin_expect((m_nTypePrevious != pLtcSenderTimeCode->nType), 0)) {
		m_nTypePrevious = pLtcSenderTimeCode->nType;
		h3_codec_set_buffer_length(LtcEncoder::Get()->GetBufferSize()); // This is an implicit stop
		h3_codec_start();
	}

	h3_codec_push_data(LtcEncoder::Get()->GetBufferPointer());
}
