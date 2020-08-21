/**
 * @file h3_hdmi_phy_dump.c
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

void h3_hdmi_phy_dump(void) {
	/* enable read access to HDMI controller */
	H3_HDMI_PHY->READ_EN = 0x54524545;
	/* descramble register offsets */
	H3_HDMI_PHY->UNSCRAMBLE = 0x42494E47;

	printf("HDMI_PHY\n");
	printf(" POL        %p\n", H3_HDMI_PHY->POL);
	printf(" READ_EN    %p\n", H3_HDMI_PHY->READ_EN);
	printf(" UNSCRAMBLE %p\n", H3_HDMI_PHY->UNSCRAMBLE);
	printf(" CTRL       %p\n", H3_HDMI_PHY->CTRL);
	printf(" UNK1       %p\n", H3_HDMI_PHY->UNK1);
	printf(" UNK2       %p\n", H3_HDMI_PHY->UNK2);
	printf(" PLL        %p\n", H3_HDMI_PHY->PLL);
	printf(" CLK        %p\n", H3_HDMI_PHY->CLK);
	printf(" UNK3       %p\n", H3_HDMI_PHY->UNK3);
	printf(" STATUS     %p\n", H3_HDMI_PHY->STATUS);
}
