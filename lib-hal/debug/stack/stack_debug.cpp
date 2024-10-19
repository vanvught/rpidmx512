/**
 * @file stack_debug.cpp
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

extern unsigned char stack_low;
extern unsigned char _sp;

static constexpr uint32_t MAGIC_WORD = 0xABCDABCD;

extern "C" {
void stack_debug_init() {
	auto *pStart = reinterpret_cast<uint32_t *>(&stack_low);
	auto *pEnd = reinterpret_cast<uint32_t *>( &_sp);

	while (pStart < pEnd) {
		*pStart = MAGIC_WORD;
		pStart++;
	}
}
}

#include <cstdio>

static uint32_t s_nUsedBytesPrevious;

void stack_debug_print() {
	const auto *pStart = reinterpret_cast<uint32_t *>(&stack_low);
	const auto *pEnd = reinterpret_cast<uint32_t *>(&_sp);
	const auto nSize = pEnd - pStart;

	auto *p = pStart;

	while (p < pEnd) {
		if (*p != MAGIC_WORD) {
			break;
		}
		p++;
	}

	const auto nUsedBytes = static_cast<uint32_t>(4 * (pEnd - p));
	const auto nFreeBytes = static_cast<uint32_t>(4 * (p - pStart));
	const auto nFreePct = ((p - pStart) * 100) / nSize;

	if (s_nUsedBytesPrevious != nUsedBytes) {
		s_nUsedBytesPrevious = nUsedBytes;

		if (nFreePct == 0) {
			printf("\x1b[31m");
		} else if (nFreePct == 1) {
			printf("\x1b[33m");
		} else {
			printf("\x1b[34m");
		}

#ifndef NDEBUG
		printf("Stack: Size %uKB, [%p:%p:%p], Used: %u, Free: %u [%u]", nSize / (1024 / 4), pStart, p, pEnd, nUsedBytes, nFreeBytes, nFreePct);
#else
		printf("Stack: Size %uKB, Used: %u, Free: %u", nSize / (1024 / 4), nUsedBytes,  nFreeBytes);
#endif
		printf("\x1b[39m\n");
	}
}

#include <time.h>

static time_t s_nTimePrevious;

void stack_debug_run() {
	auto nTime = time(nullptr);
	if (nTime != s_nTimePrevious) {
		s_nTimePrevious = nTime;
		stack_debug_print();
	}
}
