/**
 * @file shell.cpp
 *
 */
/* Copyright (C) 2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cstdarg>

#include "shell/shell.h"

extern "C" {
extern void uart0_putc(int);
extern int uart0_getc(void);
}

using namespace shell;

const char* Shell::ReadLine(uint32_t& nLength) {
	int c;

	if (__builtin_expect(((c = uart0_getc()) != EOF), 0)) {
		if (c == 127) {		// Backspace
			if (m_nLength > 0) {
				m_nLength--;
				return nullptr;
			}
		}
		if (m_nLength < BUFLEN) {
			if ((c == '\r') || (c == '\n')){
				m_bIsEndOfLine = true;
				m_Buffer[m_nLength] = '\0';
				nLength = m_nLength;
				m_nLength = 0;
				return m_Buffer;
			} else {
				if (m_bIsEndOfLine) {
					m_bIsEndOfLine = false;
					nLength = 0;
				}
			}
			m_Buffer[m_nLength] = static_cast<char>(c);
			m_nLength++;
		}
	}

	return nullptr;
}

void Shell::Puts(const char *pString) {
	while (*pString != '\0') {
		if (*pString == '\n') {
			uart0_putc('\r');
		}
		uart0_putc(*pString++);
	}
	//TODO Add additional '\r\n' at the end?
}
