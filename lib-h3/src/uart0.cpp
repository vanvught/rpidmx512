/**
 * @file uart0.cpp
 *
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "h3.h"
#include "h3_uart.h"

namespace uart0 {
void __attribute__((cold)) Init() {
    H3UartBegin(H3_UART0_BASE, 115200, H3_UART_BITS_8, H3_UART_PARITY_NONE, H3_UART_STOP_1BIT);

    while ((H3_UART0->USR & UART_USR_BUSY) == UART_USR_BUSY) {
        (void)H3_UART0->O00.RBR;
    }
}

void PutChar(int c) {
    if (c == '\n') {
        while (!(H3_UART0->LSR & UART_LSR_THRE));
        H3_UART0->O00.THR = static_cast<uint32_t>('\r');
    }

    while (!(H3_UART0->LSR & UART_LSR_THRE));

    H3_UART0->O00.THR = static_cast<uint32_t>(c);
}

void Puts(const char* s) {
    while (*s != '\0') {
        PutChar(*s++);
    }

    PutChar('\n');
}

static char s_buffer[128];

int Printf(const char* fmt, ...) {
    va_list arp;

    va_start(arp, fmt);

    int i = vsnprintf(s_buffer, sizeof(s_buffer), fmt, arp);
    s_buffer[sizeof(s_buffer) - 1] = '\0';

    va_end(arp);

    char* s = s_buffer;

    while (*s != '\0') {
        if (*s == '\n') {
            PutChar('\r');
        }
        PutChar(*s++);
    }

    return i;
}

int GetChar() {
    if (__builtin_expect(((H3_UART0->LSR & UART_LSR_DR) != UART_LSR_DR), 1)) {
        return EOF;
    }

    const auto kC = static_cast<int>(H3_UART0->O00.RBR);

#if defined(UART0_ECHO)
    putc(c);
#endif

    return kC;
}
} // namespace uart0
