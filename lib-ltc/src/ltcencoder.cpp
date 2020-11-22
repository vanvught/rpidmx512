/**
 * @file ltcencoder.cpp
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <stdio.h>
#include <cassert>

#include "ltcencoder.h"
#include "ltc.h"

#include "debug.h"

#define SAMPLE_RATE				48000

#define CEILING(x,y) 			(((x) + (y) - 1) / (y))

#define FORMAT_SIZE_BITS		80
#define FORMAT_SIZE_BYTES		CEILING(FORMAT_SIZE_BITS,  8)
#define FORMAT_SIZE_HALFWORDS	CEILING(FORMAT_SIZE_BYTES, 2)
#define FORMAT_SIZE_WORDS		CEILING(FORMAT_SIZE_BYTES, 4)

#define S_TOP					32000
#define S_MAX					(S_TOP)
#define S_MIN					(-S_TOP)

#define SYNC_WORD_VALUE			0x3FFD

struct TLtcFormatTemplate {
	union TLtcFormat {
		uint8_t bytes[FORMAT_SIZE_BYTES];
		uint16_t half_words[FORMAT_SIZE_HALFWORDS];
		uint32_t words[FORMAT_SIZE_WORDS];
		uint64_t data;
	} Format;
};

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

LtcEncoder *LtcEncoder::s_pThis = nullptr;

LtcEncoder::LtcEncoder() : m_nBufferSize(SAMPLE_RATE / 24) {
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_pLtcBits = new uint8_t[sizeof (struct TLtcFormatTemplate)];
	assert(m_pLtcBits != nullptr);

	m_pBuffer = new int16_t[m_nBufferSize];
	assert(m_pBuffer != nullptr);

	DEBUG_PRINTF("m_pBuffer=%p", reinterpret_cast<void *>(m_pBuffer));

	auto *p = reinterpret_cast<struct TLtcFormatTemplate*>(m_pLtcBits);

	for (uint32_t i = 0; i < FORMAT_SIZE_WORDS ;i ++) {
		p->Format.words[i] = 0;
	}

	p->Format.half_words[4] = __builtin_bswap16(SYNC_WORD_VALUE);
}

LtcEncoder::~LtcEncoder() {
	delete [] m_pBuffer;
	m_pBuffer = nullptr;

	delete [] m_pLtcBits;
	m_pLtcBits = nullptr;
}

void LtcEncoder::SetTimeCode(const struct TLtcTimeCode* pLtcTimeCode, bool nExternalClock) {
	auto *p = reinterpret_cast<struct TLtcFormatTemplate*>(m_pLtcBits);

	uint32_t nTens = pLtcTimeCode->nFrames / 10;

	p->Format.bytes[0] = ReverseBits(pLtcTimeCode->nFrames - (10 * nTens));
	p->Format.bytes[1] = ReverseBits(nTens);

	nTens = pLtcTimeCode->nSeconds / 10;

	p->Format.bytes[2] = ReverseBits(pLtcTimeCode->nSeconds - (10 * nTens));
	p->Format.bytes[3] = ReverseBits(nTens);

	nTens = pLtcTimeCode->nMinutes / 10;

	p->Format.bytes[4] = ReverseBits(pLtcTimeCode->nMinutes - (10 * nTens));
	p->Format.bytes[5] = ReverseBits(nTens);

	nTens = pLtcTimeCode->nHours / 10;

	p->Format.bytes[6] = ReverseBits(pLtcTimeCode->nHours - (10 * nTens));
	p->Format.bytes[7] = ReverseBits(nTens);

	m_nType = pLtcTimeCode->nType & 0x3;

	/* Bit 10 is set to 1 if drop frame numbering is in use;
	 * frame numbers 0 and 1 are skipped during the first second of every minute, except multiples of 10 minutes.
	 * This converts 30 frame/second time code to the 29.97 frame/second NTSC standard.
	 */

	if (pLtcTimeCode->nType == ltc::type::DF) {
		p->Format.bytes[1] |= (1 << 5);
	}

	/* Bit 58, unused in earlier versions of the specification,
	 * is now defined as "binary group flag 1" and indicates that the time code is synchronized to an external clock.
	 * If zero, the time origin is arbitrary.
	 */

	if (nExternalClock) {
		p->Format.bytes[7] |= (1 << 5);
	}

	SetPolarity(pLtcTimeCode->nType);
}

void LtcEncoder::SetPolarity(uint32_t nType) {
	/* "Polarity correction bit" (bit 59 at 25 frame/s, bit 27 at other rates):
	 * this bit is chosen to provide an even number of 0 bits in the whole frame, including the sync code.
	 * (Since the frame is an even number of bits long, this implies an even number of 1 bits, and is thus an even parity bit.
	 * Since the sync code includes an odd number of 1 bits, it is an odd parity bit over the data.)
	 * This keeps the phase of each frame consistent, so it always starts with a rising edge at the beginning of bit 0.
	 */

	auto *p = reinterpret_cast<struct TLtcFormatTemplate*>(m_pLtcBits);

	if (nType == ltc::type::EBU) {
		uint8_t b = p->Format.bytes[7];
		b &= ~(1 << 4);
		p->Format.bytes[7] = b;
	} else {
		uint8_t b = p->Format.bytes[3];
		b &= ~(1 << 4);
		p->Format.bytes[3] = b;
	}

	bool bParityOnes = GetParity(p->Format.words[0]) ^ GetParity(p->Format.words[1]);

	if (!bParityOnes) {
		if (nType == ltc::type::EBU) {
			uint8_t b = p->Format.bytes[7];
			b |= (1 << 4);
			p->Format.bytes[7] = b;
		} else {
			uint8_t b = p->Format.bytes[3];
			b |= (1 << 4);
			p->Format.bytes[3] = b;
		}
	}
}

void LtcEncoder::Encode() {
	const struct TLtcFormatTemplate *p = reinterpret_cast<struct TLtcFormatTemplate*>(m_pLtcBits);

	int16_t *dst = m_pBuffer;
	uint32_t nIdx;

	uint32_t nIdxPrevious = 1; // Force rising first

	for (uint32_t nBytesIndex = 0; nBytesIndex < FORMAT_SIZE_BYTES; nBytesIndex++) {
		uint32_t w = p->Format.bytes[nBytesIndex];

		for (uint32_t mask = 0x80; mask != 0; mask >>= 1) {
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

			const int16_t *src = sTables[m_nType].Samples[nIdx];

			for (uint32_t i = 0; i < sTables[m_nType].nSize; i ++) {
				*dst = *src;
				dst++;
				src++;
			}
		}
	}
}

uint32_t LtcEncoder::GetBufferSize() {
	const uint32_t nSize = SAMPLE_RATE / sTables[m_nType].nFPS;

	DEBUG_PRINTF("m_nType=%d, nSize=%d", m_nType, nSize);

	return nSize;
}

void LtcEncoder::Dump() {
	debug_dump( m_pLtcBits, sizeof(struct TLtcFormatTemplate));

	printf("\n");
	printf("0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3 3 3 3 3 4 4 4 4 4 4 4 4 4 4 5 5 5 5 5 5 5 5 5 5 6 6 6 6 6 6 6 6 6 6 7 7 7 7 7 7 7 7 7 7\n");
	printf("0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9\n");
	printf("\n");

	const struct TLtcFormatTemplate *p = reinterpret_cast<struct TLtcFormatTemplate*>(m_pLtcBits);

	uint32_t nZeros = 0;
	uint32_t nOnes = 0;

	for (uint32_t nBytesIndex = 0; nBytesIndex < FORMAT_SIZE_BYTES; nBytesIndex++) {
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
	debug_dump(m_pBuffer, m_nBufferSize * 2);
}

bool LtcEncoder::GetParity(uint32_t nValue) {
	nValue ^= nValue >> 16;
	nValue ^= nValue >> 8;
	nValue ^= nValue >> 4;
	nValue &= 0xf;

	const bool bParity = (0x6996 >> nValue) & 1;

	return bParity;
}

uint8_t LtcEncoder::ReverseBits(uint8_t nBits) {
#if defined (H3)
	const auto input = static_cast<uint32_t>(nBits);
	uint32_t output;
	asm("rbit %0, %1" : "=r"(output) : "r"(input));
	return (output >> 24);
#else
	// http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64Bits
	const uint8_t nResult = ((nBits * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
	return nResult;
#endif
}
