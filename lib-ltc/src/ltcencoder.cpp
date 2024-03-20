/**
 * @file ltcencoder.cpp
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#pragma GCC push_options
#pragma GCC optimize ("O3")

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "ltcencoder.h"
#include "ltc.h"

#include "debug.h"

namespace ltc {
namespace encoder {
#if defined (CONFIG_LTC_USE_DAC)
static constexpr uint32_t SAMPLE_RATE =	48000;
static constexpr int16_t S_TOP = 32000;
static constexpr int16_t S_MAX = (S_TOP);
static constexpr int16_t S_MIN = (-S_TOP);

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

/*
 * Buffer size is nSampleRate / FPS where FPS is 24, 25, 29 or 30
 *
 */

static constexpr uint32_t BUFFER_SIZE = SAMPLE_RATE / 24U;

#else
static constexpr uint32_t BUFFER_SIZE = FORMAT_SIZE_BITS * 2;
#endif
}  // namespace encode
}  // namespace ltc

LtcEncoder *LtcEncoder::s_pThis;

LtcEncoder::LtcEncoder()  {
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_pLtcBits = new uint8_t[sizeof (struct ltc::encoder::FormatTemplate)];
	assert(m_pLtcBits != nullptr);

	m_pBuffer = new int16_t[ltc::encoder::BUFFER_SIZE];
	assert(m_pBuffer != nullptr);

	DEBUG_PRINTF("m_pBuffer=%p", reinterpret_cast<void *>(m_pBuffer));

	auto *p = reinterpret_cast<struct ltc::encoder::FormatTemplate*>(m_pLtcBits);

	for (uint32_t i = 0; i < ltc::encoder::FORMAT_SIZE_WORDS ;i ++) {
		p->Format.words[i] = 0;
	}

	p->Format.half_words[4] = __builtin_bswap16(ltc::encoder::SYNC_WORD_VALUE);
}

LtcEncoder::~LtcEncoder() {
	delete [] m_pBuffer;
	m_pBuffer = nullptr;

	delete [] m_pLtcBits;
	m_pLtcBits = nullptr;
}

void LtcEncoder::Encode() {
	const auto *p = reinterpret_cast<struct ltc::encoder::FormatTemplate *>(m_pLtcBits);

	auto *dst = m_pBuffer;
	uint32_t nIdx;

	uint32_t nIdxPrevious = 1; // Force rising first

	for (uint32_t nBytesIndex = 0; nBytesIndex < ltc::encoder::FORMAT_SIZE_BYTES; nBytesIndex++) {
		const auto w = p->Format.bytes[nBytesIndex];

		for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
			if (mask & w) {	// '1'
				if ((nIdxPrevious == 0) || (nIdxPrevious == 2)) {
					nIdx = 2;
				} else {
					nIdx = 3;
				}
			} else { // '0'
				if ((nIdxPrevious == 0) || (nIdxPrevious == 2)) {
					nIdx = 1;
				} else {
					nIdx = 0;
				}
			}

			nIdxPrevious = nIdx;

#if defined (CONFIG_LTC_USE_DAC)
			const auto *src = ltc::encoder::sTables[m_nType].Samples[nIdx];

			for (uint32_t i = 0; i < ltc::encoder::sTables[m_nType].nSize; i ++) {
				*dst = *src;
				dst++;
				src++;
			}
#else
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
	debug_dump(m_pLtcBits, sizeof(struct ltc::encoder::FormatTemplate));

	puts("");
	printf("0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3 3 3 3 3 4 4 4 4 4 4 4 4 4 4 5 5 5 5 5 5 5 5 5 5 6 6 6 6 6 6 6 6 6 6 7 7 7 7 7 7 7 7 7 7\n");
	printf("0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9\n");
	puts("");

	const auto *p = reinterpret_cast<struct ltc::encoder::FormatTemplate *>(m_pLtcBits);

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
	debug_dump(m_pBuffer, static_cast<uint16_t>(ltc::encoder::BUFFER_SIZE * 2));
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
	auto *p = reinterpret_cast<struct ltc::encoder::FormatTemplate*>(m_pLtcBits);

	uint8_t nTens = pLtcTimeCode->nFrames / 10;

	p->Format.bytes[0] = reverse_bits(static_cast<uint8_t>(pLtcTimeCode->nFrames - (10 * nTens)));
	p->Format.bytes[1] = reverse_bits(nTens);

	nTens = pLtcTimeCode->nSeconds / 10;

	p->Format.bytes[2] = reverse_bits(static_cast<uint8_t>(pLtcTimeCode->nSeconds - (10 * nTens)));
	p->Format.bytes[3] = reverse_bits(nTens);

	nTens = pLtcTimeCode->nMinutes / 10;

	p->Format.bytes[4] = reverse_bits(static_cast<uint8_t>(pLtcTimeCode->nMinutes - (10 * nTens)));
	p->Format.bytes[5] = reverse_bits(nTens);

	nTens = pLtcTimeCode->nHours / 10;

	p->Format.bytes[6] = reverse_bits(static_cast<uint8_t>(pLtcTimeCode->nHours - (10 * nTens)));
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

	set_polarity(pLtcTimeCode->nType, m_pLtcBits);
}

