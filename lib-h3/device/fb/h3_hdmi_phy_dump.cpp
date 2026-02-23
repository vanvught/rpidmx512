/**
 * @file h3_hdmi_phy_dump.cpp
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

void h3_hdmi_phy_dump()
{
    /* enable read access to HDMI controller */
    H3_HDMI_PHY->READ_EN = 0x54524545;
    /* descramble register offsets */
    H3_HDMI_PHY->UNSCRAMBLE = 0x42494E47;

    uart0::Printf("HDMI_PHY\n");
    uart0::Printf(" POL        %p\n", H3_HDMI_PHY->POL);
    uart0::Printf(" READ_EN    %p\n", H3_HDMI_PHY->READ_EN);
    uart0::Printf(" UNSCRAMBLE %p\n", H3_HDMI_PHY->UNSCRAMBLE);
    uart0::Printf(" CTRL       %p\n", H3_HDMI_PHY->CTRL);
    uart0::Printf(" UNK1       %p\n", H3_HDMI_PHY->UNK1);
    uart0::Printf(" UNK2       %p\n", H3_HDMI_PHY->UNK2);
    uart0::Printf(" PLL        %p\n", H3_HDMI_PHY->PLL);
    uart0::Printf(" CLK        %p\n", H3_HDMI_PHY->CLK);
    uart0::Printf(" UNK3       %p\n", H3_HDMI_PHY->UNK3);
    uart0::Printf(" STATUS     %p\n", H3_HDMI_PHY->STATUS);
}
