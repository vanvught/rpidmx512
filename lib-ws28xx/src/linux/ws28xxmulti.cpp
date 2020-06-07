/**
 * @file ws28xxmulti.cpp
 *
 */
/**
 * Stub for testing purpose only
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
#include <cassert>

#include "ws28xxmulti.h"

#include "debug.h"

#define PULSE	6

void WS28xxMulti::SetupGPIO(void) {
	// Nothing todo
}

void WS28xxMulti::SetupBuffers4x(void) {
	m_pBuffer4x = new uint32_t[m_nBufSize];
	assert(m_pBuffer4x != 0);

	m_pBlackoutBuffer4x = new uint32_t[m_nBufSize];
	assert(m_pBlackoutBuffer4x != 0);

	for (uint32_t i = 0; i < m_nBufSize; i++) {
		uint32_t d = (i & 0x1) ? (1U << PULSE) : 0;
		m_pBuffer4x[i] = d;
		m_pBlackoutBuffer4x[i] = d;
	}
}

void WS28xxMulti::Update(void) {
	Generate800kHz(m_pBuffer4x);
}

void WS28xxMulti::Blackout(void) {
	Generate800kHz(m_pBlackoutBuffer4x);
}

void WS28xxMulti::Generate800kHz(const uint32_t* pBuffer) {
	for (uint32_t k = 0; k < m_nBufSize; k = k + SINGLE_RGB) {
		printf("%.2d ", k / SINGLE_RGB);

		for (uint32_t j = 0; j < 4; j++) {

			for (uint32_t i = 0; i < 24; i++) {
				bool b = (pBuffer[k + i] & (1U << j)) == 0;
				printf("%c", '0' + (b ? 0 : 1));

				if ((i != 0) && ((i + 1) % 8 == 0)) {
					printf("|");
				}
			}
			printf(" ");
		}
		printf("\n");
	}

	printf("\n");
}

uint8_t WS28xxMulti::ReverseBits(uint8_t nBits) {
	// http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64Bits
	const uint8_t nResult = ((nBits * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
	return nResult;
}
