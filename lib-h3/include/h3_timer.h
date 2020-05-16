/**
 * @file h_timer.c
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef H3_TIMER_H_
#define H3_TIMER_H_

// Timer IRQ Enable Register
#define TIMER_IRQ_EN_TMR0			(1U << 0)
#define TIMER_IRQ_EN_TMR1			(1U << 1)

// Timer IRQ Status Register
#define TIMER_IRQ_PEND_TMR0			(1U << 0)	///< Set will clear
#define TIMER_IRQ_PEND_TMR1			(1U << 1)	///< Set will clear

// Timer Control Register
#define TIMER_CTRL_EN_START			(1U << 0)	///< 0:Stop/Pause 1:Start
#define TIMER_CTRL_RELOAD			(1U << 1)
#define TIMER_CTRL_CLK_SRC_OSC24M	(1U << 2)	///< Bit 3:2
#define TIMER_CTRL_CLK_PRES_1		(0U << 4)	///< Bit 6:4
#define TIMER_CTRL_CLK_PRES_2		(1U << 4)
#define TIMER_CTRL_CLK_PRES_4		(2U << 4)
#define TIMER_CTRL_CLK_PRES_8		(3U << 4)
#define TIMER_CTRL_CLK_PRES_16		(4U << 4)
#define TIMER_CTRL_CLK_PRES_32		(5U << 4)
#define TIMER_CTRL_CLK_PRES_64		(6U << 4)
#define TIMER_CTRL_CLK_PRES_128		(7U << 4)
#define TIMER_CTRL_SINGLE_MODE		(1U << 7)	///< When interval reached , the timer will disable automatically

#ifdef __cplusplus
extern "C" {
#endif

extern void __msdelay(uint32_t);
extern void __usdelay(uint32_t);

#ifdef __cplusplus
}
#endif

#endif /* H3_TIMER_H_ */
