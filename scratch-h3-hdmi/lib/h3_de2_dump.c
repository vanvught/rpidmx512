/**
 * @file h3_de2_dump.c
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

#include "h3_de2.h"

#include "h3_ccu.h"

void h3_de2_dump(void) {
	printf("DE2\n");
	printf(" H3_CCU->PLL_DE_CTRL %p\n", H3_CCU->PLL_DE_CTRL);
	printf(" H3_CCU->DE_CLK %p\n", H3_CCU->DE_CLK);
	printf(" MUX0\n");
	printf("  CLK\n");
	printf("  GATE %p\n", H3_DE2->GATE);
	printf("  BUS  %p\n", H3_DE2->BUS);
	printf("  RST  %p\n", H3_DE2->RST);
	printf("  DIV  %p\n", H3_DE2->DIV);
	printf("  SEL  %p\n", H3_DE2->SEL);

	printf(" GLB\n");
	printf("  CTL     %p\n", H3_DE2_MUX0_GLB->CTL);
	printf("  STATUS  %p\n", H3_DE2_MUX0_GLB->STATUS);
	printf("  DBUFFER %p\n", H3_DE2_MUX0_GLB->DBUFFER);
	printf("  SIZE    %p\n", H3_DE2_MUX0_GLB->SIZE);

	printf(" BLD\n");
	printf("  FCOLOR_CTL   %p\n", H3_DE2_MUX0_BLD->FCOLOR_CTL);
	printf("  ROUTE        %p\n", H3_DE2_MUX0_BLD->ROUTE);
	printf("  PREMULIPLY  %p\n", H3_DE2_MUX0_BLD->PREMULTIPLY);
	printf("  BKCOLOR      %p\n", H3_DE2_MUX0_BLD->BKCOLOR);
	printf("  OUTPUT_SIZE  %p\n", H3_DE2_MUX0_BLD->OUTPUT_SIZE);
	printf("  BLD_MODE[0]  %p\n", H3_DE2_MUX0_BLD->BLD_MODE[0]);
	printf("  CK_CTL       %p\n", H3_DE2_MUX0_BLD->CK_CTL);
	printf("  CK_CFG       %p\n", H3_DE2_MUX0_BLD->CK_CFG);
	printf("  OUT_CTL      %p\n", H3_DE2_MUX0_BLD->OUT_CTL);
	printf("   ATTR[0]\n");
	printf("    FCOLOR  %p\n", H3_DE2_MUX0_BLD->ATTR[0].FCOLOR);
	printf("    INSIZE  %p\n", H3_DE2_MUX0_BLD->ATTR[0].INSIZE);
	printf("    OFFSET  %p\n", H3_DE2_MUX0_BLD->ATTR[0].OFFSET);

	printf(" CSC\n");
	printf("  CTL %p\n", H3_DE2_MUX0_CSC->CTL);

	printf(" UI\n");
	printf("  TOP_HADDR %p\n", H3_DE2_MUX0_UI->TOP_HADDR);
	printf("  BOT_HADDR %p\n", H3_DE2_MUX0_UI->BOT_HADDR);
	printf("  OVL_SIZE  %p\n", H3_DE2_MUX0_UI->OVL_SIZE);
	printf("  CFG[0]\n");
	printf("   ATTR      %p\n", H3_DE2_MUX0_UI->CFG[0].ATTR);
	printf("   SIZE      %p\n", H3_DE2_MUX0_UI->CFG[0].SIZE);
	printf("   COORD     %p\n", H3_DE2_MUX0_UI->CFG[0].COORD);
	printf("   PITCH     %p\n", H3_DE2_MUX0_UI->CFG[0].PITCH);
	printf("   TOP_LADDR %p\n", H3_DE2_MUX0_UI->CFG[0].TOP_LADDR);
	printf("   BOT_LADDR %p\n", H3_DE2_MUX0_UI->CFG[0].BOT_LADDR);
	printf("   FCOLOR    %p\n", H3_DE2_MUX0_UI->CFG[0].FCOLOR);
}
