/**
 * @file h3_lcdc_dump.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "h3.h"

namespace uart0
{
void Printf(const char* fmt, ...);
}

/*
 * The LCD0 module is used for HDMI
 */
void h3_lcdc_dump()
{
    uart0::Printf("LCD0\n");
    uart0::Printf(" GCTL         %p\n", H3_LCD0->GCTL);
    uart0::Printf(" GINT0        %p\n", H3_LCD0->GINT0);
    uart0::Printf(" GINT1        %p\n", H3_LCD0->GINT1);
    uart0::Printf(" TCON1_CTL    %p\n", H3_LCD0->TCON1_CTL);
    uart0::Printf(" TCON1_BASIC0 %p\n", H3_LCD0->TCON1_BASIC0);
    uart0::Printf(" TCON1_BASIC1 %p\n", H3_LCD0->TCON1_BASIC1);
    uart0::Printf(" TCON1_BASIC2 %p\n", H3_LCD0->TCON1_BASIC2);
    uart0::Printf(" TCON1_BASIC3 %p\n", H3_LCD0->TCON1_BASIC3);
    uart0::Printf(" TCON1_BASIC4 %p\n", H3_LCD0->TCON1_BASIC4);
    uart0::Printf(" TCON1_BASIC5 %p\n", H3_LCD0->TCON1_BASIC5);
}
