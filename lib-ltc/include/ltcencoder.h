/**
 * @file ltcencoder.h
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTCENCODER_H_
#define LTCENCODER_H_

#if !(defined(CONFIG_LTC_USE_DAC) || defined(CONFIG_LTC_USE_GPIO))
# error No output defined
#endif

#include <cstdint>

#include "ltc.h"


namespace ltc::encoder {
#define LTCENCODER_CEILING(x,y)		(((x) + (y) - 1) / (y))
static constexpr uint32_t FORMAT_SIZE_BITS 		= 80;
static constexpr uint32_t FORMAT_SIZE_BYTES 	= LTCENCODER_CEILING(FORMAT_SIZE_BITS, 8);
static constexpr uint32_t FORMAT_SIZE_HALFWORDS	= LTCENCODER_CEILING(FORMAT_SIZE_BYTES, 2);
static constexpr uint32_t FORMAT_SIZE_WORDS 	= LTCENCODER_CEILING(FORMAT_SIZE_BYTES, 4);
#undef LTCENCODER_CEILING

static constexpr uint16_t SYNC_WORD_VALUE = 0x3FFD;

#if defined (CONFIG_LTC_USE_DAC)
static constexpr uint32_t SAMPLE_RATE =	48000;
static constexpr int16_t S_TOP = 32000;
static constexpr int16_t S_MAX = (S_TOP);
static constexpr int16_t S_MIN = (-S_TOP);
/*
 * Buffer size is nSampleRate / FPS where FPS is 24, 25, 29 or 30
 *
 */
static constexpr uint32_t BUFFER_SIZE = SAMPLE_RATE / 24U;
#else
static constexpr uint32_t BUFFER_SIZE = FORMAT_SIZE_BITS * 2;
#endif

struct FormatTemplate {
	union Format {
		uint8_t bytes[FORMAT_SIZE_BYTES];
		uint16_t half_words[FORMAT_SIZE_HALFWORDS];
		uint32_t words[FORMAT_SIZE_WORDS];
		uint64_t data;
	} Format;
};
} // namespace ltc::encoder


class LtcEncoder {
public:
	LtcEncoder();
	~LtcEncoder();

	void SetTimeCode(const struct ltc::TimeCode *pLtcTimeCode, bool nExternalClock = true);
	void Encode(void *pBuffer = nullptr);

	void Dump();
	void DumpBuffer();

#if defined (CONFIG_LTC_USE_DAC)
	int16_t *GetBufferPointer() const {
		return s_Buffer;
	}
#else
	uint32_t *GetBufferPointer() const {
		return s_Buffer;
	}
#endif

	uint32_t GetBufferSize();

	static LtcEncoder* Get() {
		return s_pThis;
	}

private:
	uint8_t m_LtcBits[sizeof(struct ltc::encoder::FormatTemplate)];
	uint32_t m_nType { 0xFF };

#if defined (CONFIG_LTC_USE_DAC)
	static int16_t s_Buffer[ltc::encoder::BUFFER_SIZE];
#else
	static uint32_t s_Buffer[ltc::encoder::BUFFER_SIZE];
#endif
	static LtcEncoder *s_pThis;
};

#endif /* LTCENCODER_H_ */
