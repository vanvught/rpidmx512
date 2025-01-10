/**
 * @file ltcencoder.cpp
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

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O3")
#endif

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "ltcencoder.h"
#include "ltc.h"

#include "debug.h"


namespace ltc::encoder {
#if defined (CONFIG_LTC_USE_DAC)
struct TTable {
	uint32_t nSize;
	uint32_t nFPS;
	int16_t Samples[4][26];
} static constexpr sTables[4] = {
	{ 25, 24, {
		{ S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX },
		{ S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN },
		{ S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, 0,     S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX },
		{ S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, 0,     S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN }
	} },
	{ 24, 25, {
		{ S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX },
		{ S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN },
		{ S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX },
		{ S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN }
	} },
	{ 20, 30, {
		{ S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX },
		{ S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN },
		{ S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX },
		{ S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN }

	} },
	{ 20, 30, {
		{ S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX },
		{ S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN },
		{ S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX },
		{ S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MAX, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN, S_MIN }
	} }
	};
#else
# include "arm/gd32/ltc_gpio.h"
#endif
}  // namespace encode


#if defined (CONFIG_LTC_USE_DAC)
	int16_t LtcEncoder::s_Buffer[ltc::encoder::BUFFER_SIZE];
#else
	uint32_t LtcEncoder::s_Buffer[ltc::encoder::BUFFER_SIZE];
#endif
LtcEncoder *LtcEncoder::s_pThis;

LtcEncoder::LtcEncoder()  {
	assert(s_pThis == nullptr);
	s_pThis = this;

	auto *p = reinterpret_cast<struct ltc::encoder::FormatTemplate *>(m_LtcBits);

	for (uint32_t i = 0; i < ltc::encoder::FORMAT_SIZE_WORDS ;i ++) {
		p->Format.words[i] = 0;
	}

	p->Format.half_words[4] = __builtin_bswap16(ltc::encoder::SYNC_WORD_VALUE);
}

void LtcEncoder::Encode(void *pBuffer) {
	const auto *p = reinterpret_cast<struct ltc::encoder::FormatTemplate *>(m_LtcBits);

	auto *pDst = s_Buffer;
#if defined (CONFIG_LTC_USE_DAC)
	if (pBuffer != nullptr) {
		pDst = reinterpret_cast<int16_t *>(pBuffer);
	}
#else
	if (pBuffer != nullptr) {
		pDst = reinterpret_cast<uint32_t *>(pBuffer);
	}
#endif

	uint32_t nIdx;
	uint32_t nIdxPrevious = 1; // Force rising first

#if !defined (CONFIG_LTC_USE_DAC)
	constexpr auto GPIO_SHIFT_SET = static_cast<uint32_t>(1U << LTC_OUTPUT_GPIO_PIN_OFFSET);
	constexpr auto GPIO_SHIFT_CLEAR	= static_cast<uint32_t>(GPIO_SHIFT_SET << 16);
#endif

	for (uint32_t nBytesIndex = 0; nBytesIndex < ltc::encoder::FORMAT_SIZE_BYTES; nBytesIndex++) {
		const auto w = p->Format.bytes[nBytesIndex];

		for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
			if (mask & w) {	// '1'
				if ((nIdxPrevious == 0) || (nIdxPrevious == 2)) {
					nIdx = 2;
#if !defined (CONFIG_LTC_USE_DAC)
					*pDst++ = GPIO_SHIFT_CLEAR;
					*pDst++ = GPIO_SHIFT_SET;
#endif
				} else {
					nIdx = 3;
#if !defined (CONFIG_LTC_USE_DAC)
					*pDst++ = GPIO_SHIFT_SET;
					*pDst++ = GPIO_SHIFT_CLEAR;
#endif
				}
			} else { // '0'
				if ((nIdxPrevious == 0) || (nIdxPrevious == 2)) {
					nIdx = 1;
#if !defined (CONFIG_LTC_USE_DAC)
					*pDst++ = GPIO_SHIFT_CLEAR;
					*pDst++ = GPIO_SHIFT_CLEAR;
#endif
				} else {
					nIdx = 0;
#if !defined (CONFIG_LTC_USE_DAC)
					*pDst++ = GPIO_SHIFT_SET;
					*pDst++ = GPIO_SHIFT_SET;
#endif
				}
			}

			nIdxPrevious = nIdx;

#if defined (CONFIG_LTC_USE_DAC)
			const auto *pSrc = ltc::encoder::sTables[m_nType].Samples[nIdx];

			for (uint32_t i = 0; i < ltc::encoder::sTables[m_nType].nSize; i ++) {
				*pDst = *pSrc;
				pDst++;
				pSrc++;
			}
#endif
		}
	}
}

uint32_t LtcEncoder::GetBufferSize() {
#if defined (CONFIG_LTC_USE_DAC)
	const uint32_t nSize = ltc::encoder::SAMPLE_RATE / ltc::encoder::sTables[m_nType].nFPS;
#else
	const uint32_t nSize = ltc::encoder::BUFFER_SIZE;
#endif

	DEBUG_PRINTF("m_nType=%d, nSize=%d", m_nType, nSize);

	return nSize;
}

void LtcEncoder::Dump() {
	debug_dump(m_LtcBits, sizeof(struct ltc::encoder::FormatTemplate));

	puts("");
	printf("0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3 3 3 3 3 4 4 4 4 4 4 4 4 4 4 5 5 5 5 5 5 5 5 5 5 6 6 6 6 6 6 6 6 6 6 7 7 7 7 7 7 7 7 7 7\n");
	printf("0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9\n");
	puts("");

	const auto *p = reinterpret_cast<struct ltc::encoder::FormatTemplate *>(m_LtcBits);

	uint32_t nZeros = 0;
	uint32_t nOnes = 0;

	for (uint32_t nBytesIndex = 0; nBytesIndex < ltc::encoder::FORMAT_SIZE_BYTES; nBytesIndex++) {
		uint32_t w = p->Format.bytes[nBytesIndex];

		for (uint32_t mask = 0x80; mask != 0; mask >>= 1) {
			if (mask & w) {
				nOnes++;
				printf("1 ");
			} else {
				nZeros++;
				printf("0 ");
			}
		}
	}

	printf("\n\nZero's=%d (%s), Ones's=%d\n", nZeros, (nZeros % 2 == 0) ? "even" : "odd!", nOnes);
}

void LtcEncoder::DumpBuffer() {
#if defined (CONFIG_LTC_USE_DAC)
	debug_dump(s_Buffer, static_cast<uint16_t>(ltc::encoder::BUFFER_SIZE * 2));
#else
	debug_dump(s_Buffer, static_cast<uint16_t>(ltc::encoder::BUFFER_SIZE * 4));
#endif
}

/*
 * Common
 */

static bool get_parity(uint32_t nValue) {
	nValue ^= nValue >> 16;
	nValue ^= nValue >> 8;
	nValue ^= nValue >> 4;
	nValue &= 0xf;

	return static_cast<bool>((0x6996 >> nValue) & 1);
}

static void set_polarity(const uint32_t nType, uint8_t *pLtcBits) {
	/* "Polarity correction bit" (bit 59 at 25 frame/s, bit 27 at other rates):
	 * this bit is chosen to provide an even number of 0 bits in the whole frame, including the sync code.
	 * (Since the frame is an even number of bits long, this implies an even number of 1 bits, and is thus an even parity bit.
	 * Since the sync code includes an odd number of 1 bits, it is an odd parity bit over the data.)
	 * This keeps the phase of each frame consistent, so it always starts with a rising edge at the beginning of bit 0.
	 */

	auto *p = reinterpret_cast<struct ltc::encoder::FormatTemplate *>(pLtcBits);

	if (nType == static_cast<uint8_t>(ltc::Type::EBU)) {
		auto b = p->Format.bytes[7];
		b &= static_cast<uint8_t>(~(1U << 4));
		p->Format.bytes[7] = b;
	} else {
		auto b = p->Format.bytes[3];
		b &= static_cast<uint8_t>(~(1U << 4));
		p->Format.bytes[3] = b;
	}

	const auto bParityOnes = get_parity(p->Format.words[0]) ^ get_parity(p->Format.words[1]);

	if (!bParityOnes) {
		if (nType == static_cast<uint8_t>(ltc::Type::EBU)) {
			auto b = p->Format.bytes[7];
			b |= (1 << 4);
			p->Format.bytes[7] = b;
		} else {
			auto b = p->Format.bytes[3];
			b |= (1 << 4);
			p->Format.bytes[3] = b;
		}
	}
}

static uint8_t reverse_bits(const uint8_t nBits) {
#if defined (PLATFORM_LTC_ARM)
	return (static_cast<uint8_t>(__RBIT(static_cast<uint32_t>(nBits)) >> 24));
#else
	// http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64Bits
	const uint8_t nResult = ((nBits * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
	return nResult;
#endif
}

void LtcEncoder::SetTimeCode(const struct ltc::TimeCode *pLtcTimeCode, bool nExternalClock) {
	auto *p = reinterpret_cast<struct ltc::encoder::FormatTemplate *>(m_LtcBits);

	uint8_t nTens = pLtcTimeCode->nFrames / 10U;

	p->Format.bytes[0] = reverse_bits(static_cast<uint8_t>(pLtcTimeCode->nFrames - (10U * nTens)));
	p->Format.bytes[1] = reverse_bits(nTens);

	nTens = pLtcTimeCode->nSeconds / 10U;

	p->Format.bytes[2] = reverse_bits(static_cast<uint8_t>(pLtcTimeCode->nSeconds - (10U * nTens)));
	p->Format.bytes[3] = reverse_bits(nTens);

	nTens = pLtcTimeCode->nMinutes / 10U;

	p->Format.bytes[4] = reverse_bits(static_cast<uint8_t>(pLtcTimeCode->nMinutes - (10U * nTens)));
	p->Format.bytes[5] = reverse_bits(nTens);

	nTens = pLtcTimeCode->nHours / 10U;

	p->Format.bytes[6] = reverse_bits(static_cast<uint8_t>(pLtcTimeCode->nHours - (10U * nTens)));
	p->Format.bytes[7] = reverse_bits(nTens);

	m_nType = pLtcTimeCode->nType & 0x3;

	/* Bit 10 is set to 1 if drop frame numbering is in use;
	 * frame numbers 0 and 1 are skipped during the first second of every minute, except multiples of 10 minutes.
	 * This converts 30 frame/second time code to the 29.97 frame/second NTSC standard.
	 */

	if (pLtcTimeCode->nType == static_cast<uint8_t>(ltc::Type::DF)) {
		p->Format.bytes[1] |= (1U << 5);
	}

	/* Bit 58, unused in earlier versions of the specification,
	 * is now defined as "binary group flag 1" and indicates that the time code is synchronized to an external clock.
	 * If zero, the time origin is arbitrary.
	 */

	if (nExternalClock) {
		p->Format.bytes[7] |= (1U << 5);
	}

	set_polarity(pLtcTimeCode->nType, m_LtcBits);
}

