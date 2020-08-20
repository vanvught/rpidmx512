/**
 * @file h3_lcdc_dump.c
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

#include <stdio.h>

extern int uart0_printf(const char* fmt, ...);
#define printf uart0_printf

#include "h3.h"

/*
 * The LCD0 module is used for HDMI
 */

void h3_lcdc_dump(void) {
	printf("LCD0\n");
	printf(" GCTL         %p\n", H3_LCD0->GCTL);
	printf(" GINT0        %p\n", H3_LCD0->GINT0);
	printf(" GINT1        %p\n", H3_LCD0->GINT1);
	printf(" TCON1_CTL    %p\n", H3_LCD0->TCON1_CTL);
	printf(" TCON1_BASIC0 %p\n", H3_LCD0->TCON1_BASIC0);
	printf(" TCON1_BASIC1 %p\n", H3_LCD0->TCON1_BASIC1);
	printf(" TCON1_BASIC2 %p\n", H3_LCD0->TCON1_BASIC2);
	printf(" TCON1_BASIC3 %p\n", H3_LCD0->TCON1_BASIC3);
	printf(" TCON1_BASIC4 %p\n", H3_LCD0->TCON1_BASIC4);
	printf(" TCON1_BASIC5 %p\n", H3_LCD0->TCON1_BASIC5);
}
