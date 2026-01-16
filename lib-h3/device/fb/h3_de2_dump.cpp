/**
 * @file h3_de2_dump.cpp
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

#include "h3_de2.h"
#include "h3_ccu.h"

namespace uart0 {
void Printf(const char* fmt, ...);
}

void h3_de2_dump(void) {
	uart0::Printf("DE2\n");
	uart0::Printf(" H3_CCU->PLL_DE_CTRL %p\n", H3_CCU->PLL_DE_CTRL);
	uart0::Printf(" H3_CCU->DE_CLK %p\n", H3_CCU->DE_CLK);
	uart0::Printf(" MUX0\n");
	uart0::Printf("  CLK\n");
	uart0::Printf("  GATE %p\n", H3_DE2->GATE);
	uart0::Printf("  BUS  %p\n", H3_DE2->BUS);
	uart0::Printf("  RST  %p\n", H3_DE2->RST);
	uart0::Printf("  DIV  %p\n", H3_DE2->DIV);
	uart0::Printf("  SEL  %p\n", H3_DE2->SEL);

	uart0::Printf(" GLB\n");
	uart0::Printf("  CTL     %p\n", H3_DE2_MUX0_GLB->CTL);
	uart0::Printf("  STATUS  %p\n", H3_DE2_MUX0_GLB->STATUS);
	uart0::Printf("  DBUFFER %p\n", H3_DE2_MUX0_GLB->DBUFFER);
	uart0::Printf("  SIZE    %p\n", H3_DE2_MUX0_GLB->SIZE);

	uart0::Printf(" BLD\n");
	uart0::Printf("  FCOLOR_CTL   %p\n", H3_DE2_MUX0_BLD->FCOLOR_CTL);
	uart0::Printf("  ROUTE        %p\n", H3_DE2_MUX0_BLD->ROUTE);
	uart0::Printf("  PREMULIPLY  %p\n", H3_DE2_MUX0_BLD->PREMULTIPLY);
	uart0::Printf("  BKCOLOR      %p\n", H3_DE2_MUX0_BLD->BKCOLOR);
	uart0::Printf("  OUTPUT_SIZE  %p\n", H3_DE2_MUX0_BLD->OUTPUT_SIZE);
	uart0::Printf("  BLD_MODE[0]  %p\n", H3_DE2_MUX0_BLD->BLD_MODE[0]);
	uart0::Printf("  CK_CTL       %p\n", H3_DE2_MUX0_BLD->CK_CTL);
	uart0::Printf("  CK_CFG       %p\n", H3_DE2_MUX0_BLD->CK_CFG);
	uart0::Printf("  OUT_CTL      %p\n", H3_DE2_MUX0_BLD->OUT_CTL);
	uart0::Printf("   ATTR[0]\n");
	uart0::Printf("    FCOLOR  %p\n", H3_DE2_MUX0_BLD->ATTR[0].FCOLOR);
	uart0::Printf("    INSIZE  %p\n", H3_DE2_MUX0_BLD->ATTR[0].INSIZE);
	uart0::Printf("    OFFSET  %p\n", H3_DE2_MUX0_BLD->ATTR[0].OFFSET);

	uart0::Printf(" CSC\n");
	uart0::Printf("  CTL %p\n", H3_DE2_MUX0_CSC->CTL);

	uart0::Printf(" UI\n");
	uart0::Printf("  TOP_HADDR %p\n", H3_DE2_MUX0_UI->TOP_HADDR);
	uart0::Printf("  BOT_HADDR %p\n", H3_DE2_MUX0_UI->BOT_HADDR);
	uart0::Printf("  OVL_SIZE  %p\n", H3_DE2_MUX0_UI->OVL_SIZE);
	uart0::Printf("  CFG[0]\n");
	uart0::Printf("   ATTR      %p\n", H3_DE2_MUX0_UI->CFG[0].ATTR);
	uart0::Printf("   SIZE      %p\n", H3_DE2_MUX0_UI->CFG[0].SIZE);
	uart0::Printf("   COORD     %p\n", H3_DE2_MUX0_UI->CFG[0].COORD);
	uart0::Printf("   PITCH     %p\n", H3_DE2_MUX0_UI->CFG[0].PITCH);
	uart0::Printf("   TOP_LADDR %p\n", H3_DE2_MUX0_UI->CFG[0].TOP_LADDR);
	uart0::Printf("   BOT_LADDR %p\n", H3_DE2_MUX0_UI->CFG[0].BOT_LADDR);
	uart0::Printf("   FCOLOR    %p\n", H3_DE2_MUX0_UI->CFG[0].FCOLOR);
}
